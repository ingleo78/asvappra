#include <stdbool.h>
#include <assert.h>
#include "curl_setup.h"
#include "urldata.h"
#include "sendf.h"
#include "select.h"
#include "progress.h"
#include "speedcheck.h"
#include "pingpong.h"
#include "multiif.h"
#include "non-ascii.h"
#include "vtls/vtls.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

#ifdef USE_PINGPONG
timediff_t Curl_pp_state_timeout(struct pingpong *pp, bool disconnecting) {
    struct connectdata *conn = pp->conn;
    struct Curl_easy *data = conn->data;
    timediff_t timeout_ms;
    timediff_t response_time = (data->set.server_response_timeout) ? data->set.server_response_timeout : pp->response_time;
    timeout_ms = response_time - Curl_timediff(Curl_now(), pp->response);
    if(data->set.timeout && !disconnecting) {
        timediff_t timeout2_ms = data->set.timeout - Curl_timediff(Curl_now(), conn->now);
        timeout_ms = CURLMIN(timeout_ms, timeout2_ms);
    }
    return timeout_ms;
}
CURLcode Curl_pp_statemach(struct pingpong *pp, bool block, bool disconnecting) {
    struct connectdata *conn = pp->conn;
    curl_socket_t sock = conn->sock[FIRSTSOCKET];
    int rc;
    timediff_t interval_ms;
    timediff_t timeout_ms = Curl_pp_state_timeout(pp, disconnecting);
    struct Curl_easy *data = conn->data;
    CURLcode result = CURLE_OK;
    if(timeout_ms <= 0) {
        failf(data, "server response timeout");
        return CURLE_OPERATION_TIMEDOUT;
    }
    if(block) {
        interval_ms = 1000;
        if(timeout_ms < interval_ms) interval_ms = timeout_ms;
    } else interval_ms = 0;
    if(Curl_ssl_data_pending(conn, FIRSTSOCKET)) rc = 1;
    else if(Curl_pp_moredata(pp)) rc = 1;
    else if(!pp->sendleft && Curl_ssl_data_pending(conn, FIRSTSOCKET)) rc = 1;
    else rc = Curl_socket_check(pp->sendleft?CURL_SOCKET_BAD:sock, CURL_SOCKET_BAD, pp->sendleft?sock:CURL_SOCKET_BAD, interval_ms);
    if(block) {
        if(Curl_pgrsUpdate(conn)) result = CURLE_ABORTED_BY_CALLBACK;
        else result = Curl_speedcheck(data, Curl_now());
        if(result) return result;
    }
    if(rc == -1) {
        failf(data, "select/poll error");
        result = CURLE_OUT_OF_MEMORY;
    } else if(rc) result = pp->statemach_act(conn);
    return result;
}
void Curl_pp_init(struct pingpong *pp) {
    struct connectdata *conn = pp->conn;
    pp->nread_resp = 0;
    pp->linestart_resp = conn->data->state.buffer;
    pp->pending_resp = TRUE;
    pp->response = Curl_now();
}
CURLcode Curl_pp_vsendf(struct pingpong *pp, const char *fmt, va_list args) {
    ssize_t bytes_written;
    size_t write_len;
    char *fmt_crlf;
    char *s;
    CURLcode result;
    struct connectdata *conn = pp->conn;
    struct Curl_easy *data;
#ifdef HAVE_GSSAPI
    enum protection_level data_sec;
#endif
    DEBUGASSERT(pp->sendleft == 0);
    DEBUGASSERT(pp->sendsize == 0);
    DEBUGASSERT(pp->sendthis == NULL);
    if(!conn) return CURLE_SEND_ERROR;
    data = conn->data;
    fmt_crlf = aprintf("%s\r\n", fmt);
    if(!fmt_crlf) return CURLE_OUT_OF_MEMORY;
    s = vaprintf(fmt_crlf, args);
    free(fmt_crlf);
    if(!s) return CURLE_OUT_OF_MEMORY;
    bytes_written = 0;
    write_len = strlen(s);
    Curl_pp_init(pp);
    result = Curl_convert_to_network(data, s, write_len);
    if(result) {
        free(s);
        return result;
    }
#ifdef HAVE_GSSAPI
    conn->data_prot = PROT_CMD;
#endif
    result = Curl_write(conn, conn->sock[FIRSTSOCKET], s, write_len, &bytes_written);
#ifdef HAVE_GSSAPI
    data_sec = conn->data_prot;
    DEBUGASSERT(data_sec > PROT_NONE && data_sec < PROT_LAST);
    conn->data_prot = data_sec;
#endif
    if(result) {
        free(s);
        return result;
    }
    if(conn->data->set.verbose) Curl_debug(conn->data, CURLINFO_HEADER_OUT, s, (size_t)bytes_written);
    if(bytes_written != (ssize_t)write_len) {
        pp->sendthis = s;
        pp->sendsize = write_len;
        pp->sendleft = write_len - bytes_written;
    } else {
        free(s);
        pp->sendthis = NULL;
        pp->sendleft = pp->sendsize = 0;
        pp->response = Curl_now();
    }
    return CURLE_OK;
}
CURLcode Curl_pp_sendf(struct pingpong *pp, const char *fmt, ...) {
    CURLcode result;
    va_list ap;
    va_start(ap, fmt);
    result = Curl_pp_vsendf(pp, fmt, ap);
    va_end(ap);
    return result;
}
CURLcode Curl_pp_readresp(curl_socket_t sockfd, struct pingpong *pp, int *code, size_t *size) {
    ssize_t perline;
    bool keepon = TRUE;
    ssize_t gotbytes;
    char *ptr;
    struct connectdata *conn = pp->conn;
    struct Curl_easy *data = conn->data;
    char * const buf = data->state.buffer;
    CURLcode result = CURLE_OK;
    *code = 0;
    *size = 0;
    ptr = buf + pp->nread_resp;
    perline = (ssize_t)(ptr-pp->linestart_resp);
    while((pp->nread_resp < (size_t)data->set.buffer_size) && (keepon && !result)) {
        if(pp->cache) {
            if((ptr + pp->cache_size) > (buf + data->set.buffer_size + 1)) {
                failf(data, "cached response data too big to handle");
                return CURLE_RECV_ERROR;
            }
            memcpy(ptr, pp->cache, pp->cache_size);
            gotbytes = (ssize_t)pp->cache_size;
            free(pp->cache);
            pp->cache = NULL;
            pp->cache_size = 0;
        } else {
        #ifdef HAVE_GSSAPI
            enum protection_level prot = conn->data_prot;
            conn->data_prot = PROT_CLEAR;
        #endif
            DEBUGASSERT((ptr + data->set.buffer_size - pp->nread_resp) <= (buf + data->set.buffer_size + 1));
            result = Curl_read(conn, sockfd, ptr, data->set.buffer_size - pp->nread_resp, &gotbytes);
        #ifdef HAVE_GSSAPI
            DEBUGASSERT(prot  > PROT_NONE && prot < PROT_LAST);
            conn->data_prot = prot;
        #endif
            if(result == CURLE_AGAIN) return CURLE_OK;
            if(!result && (gotbytes > 0)) result = Curl_convert_from_network(data, ptr, gotbytes);
            if(result) keepon = FALSE;
        }
        if(!keepon);
        else if(gotbytes <= 0) {
            keepon = FALSE;
            result = CURLE_RECV_ERROR;
            failf(data, "response reading failed");
        } else {
            ssize_t i;
            ssize_t clipamount = 0;
            bool restart = FALSE;
            data->req.headerbytecount += (long)gotbytes;
            pp->nread_resp += gotbytes;
            for(i = 0; i < gotbytes; ptr++, i++) {
                perline++;
                if(*ptr == '\n') {
                #ifdef HAVE_GSSAPI
                    if(!conn->sec_complete)
                #endif
                    if(data->set.verbose) Curl_debug(data, CURLINFO_HEADER_IN, pp->linestart_resp, (size_t)perline);
                    result = Curl_client_write(conn, CLIENTWRITE_HEADER, pp->linestart_resp, perline);
                    if(result) return result;
                    if(pp->endofresp(conn, pp->linestart_resp, perline, code)) {
                        size_t n = ptr - pp->linestart_resp;
                        memmove(buf, pp->linestart_resp, n);
                        buf[n] = 0;
                        keepon = FALSE;
                        pp->linestart_resp = ptr + 1;
                        i++;
                        *size = pp->nread_resp;
                        pp->nread_resp = 0;
                        break;
                    }
                    perline = 0;
                    pp->linestart_resp = ptr + 1;
                }
            }
            if(!keepon && (i != gotbytes)) {
            clipamount = gotbytes - i;
            restart = TRUE;
            DEBUGF(infof(data, "Curl_pp_readresp_ %d bytes of trailing " "server response left\n", (int)clipamount));
            } else if(keepon) {
                if((perline == gotbytes) && (gotbytes > data->set.buffer_size/2)) {
                    infof(data, "Excessive server response line length received, " "%zd bytes. Stripping\n", gotbytes);
                    restart = TRUE;
                    clipamount = 40;
                } else if(pp->nread_resp > (size_t)data->set.buffer_size/2) {
                    clipamount = perline;
                    restart = TRUE;
                }
            } else if(i == gotbytes) restart = TRUE;
            if(clipamount) {
                pp->cache_size = clipamount;
                pp->cache = malloc(pp->cache_size);
                if(pp->cache) memcpy(pp->cache, pp->linestart_resp, pp->cache_size);
                else return CURLE_OUT_OF_MEMORY;
            }
            if(restart) {
                pp->nread_resp = 0;
                ptr = pp->linestart_resp = buf;
                perline = 0;
            }
        }
    }
    pp->pending_resp = FALSE;
    return result;
}
int Curl_pp_getsock(struct pingpong *pp, curl_socket_t *socks) {
    struct connectdata *conn = pp->conn;
    socks[0] = conn->sock[FIRSTSOCKET];
    if(pp->sendleft) return GETSOCK_WRITESOCK(0);
    return GETSOCK_READSOCK(0);
}
CURLcode Curl_pp_flushsend(struct pingpong *pp) {
    struct connectdata *conn = pp->conn;
    ssize_t written;
    curl_socket_t sock = conn->sock[FIRSTSOCKET];
    CURLcode result = Curl_write(conn, sock, pp->sendthis + pp->sendsize - pp->sendleft, pp->sendleft, &written);
    if(result) return result;
    if(written != (ssize_t)pp->sendleft) pp->sendleft -= written;
    else {
        free(pp->sendthis);
        pp->sendthis = NULL;
        pp->sendleft = pp->sendsize = 0;
        pp->response = Curl_now();
    }
    return CURLE_OK;
}
CURLcode Curl_pp_disconnect(struct pingpong *pp) {
    free(pp->cache);
    pp->cache = NULL;
    return CURLE_OK;
}
bool Curl_pp_moredata(struct pingpong *pp) {
    return (!pp->sendleft && pp->cache && pp->nread_resp < pp->cache_size) ? TRUE : FALSE;
}
#endif