#include <stdbool.h>
#include <assert.h>
#include "curl_setup.h"
#include "http_proxy.h"

#if !defined(CURL_DISABLE_PROXY) && !defined(CURL_DISABLE_HTTP)
#include "curl.h"
#include "sendf.h"
#include "http.h"
#include "url.h"
#include "select.h"
#include "progress.h"
#include "non-ascii.h"
#include "connect.h"
#include "curlx.h"
#include "vtls/vtls.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

static CURLcode https_proxy_connect(struct connectdata *conn, int sockindex) {
#ifdef USE_SSL
    CURLcode result = CURLE_OK;
    DEBUGASSERT(conn->http_proxy.proxytype == CURLPROXY_HTTPS);
    if(!conn->bits.proxy_ssl_connected[sockindex]) {
        result = Curl_ssl_connect_nonblocking(conn, sockindex, &conn->bits.proxy_ssl_connected[sockindex]);
        if(result) connclose(conn, "TLS handshake failed");
    }
    return result;
#else
    (void) conn;
    (void) sockindex;
    return CURLE_NOT_BUILT_IN;
#endif
}
CURLcode Curl_proxy_connect(struct connectdata *conn, int sockindex) {
    struct Curl_easy *data = conn->data;
    if(conn->http_proxy.proxytype == CURLPROXY_HTTPS) {
        const CURLcode result = https_proxy_connect(conn, sockindex);
        if(result) return result;
        if(!conn->bits.proxy_ssl_connected[sockindex]) return result;
    }
    if(conn->bits.tunnel_proxy && conn->bits.httpproxy) {
    #ifndef CURL_DISABLE_PROXY
        struct HTTP http_proxy;
        void *prot_save;
        const char *hostname;
        int remote_port;
        CURLcode result;
        prot_save = conn->data->req.protop;
        memset(&http_proxy, 0, sizeof(http_proxy));
        conn->data->req.protop = &http_proxy;
        connkeep(conn, "HTTP proxy CONNECT");
        if(conn->bits.conn_to_host) hostname = conn->conn_to_host.name;
        else if(sockindex == SECONDARYSOCKET) hostname = conn->secondaryhostname;
        else hostname = conn->host.name;
        if(sockindex == SECONDARYSOCKET) remote_port = conn->secondary_port;
        else if(conn->bits.conn_to_port) remote_port = conn->conn_to_port;
        else remote_port = conn->remote_port;
        result = Curl_proxyCONNECT(conn, sockindex, hostname, remote_port);
        conn->data->req.protop = prot_save;
        if(CURLE_OK != result) return result;
        Curl_safefree(data->state.aptr.proxyuserpwd);
    #else
        return CURLE_NOT_BUILT_IN;
    #endif
    }
    return CURLE_OK;
}
bool Curl_connect_complete(struct connectdata *conn) {
    return !conn->connect_state || (conn->connect_state->tunnel_state == TUNNEL_COMPLETE);
}
bool Curl_connect_ongoing(struct connectdata *conn) {
    return conn->connect_state && (conn->connect_state->tunnel_state != TUNNEL_COMPLETE);
}
static CURLcode connect_init(struct connectdata *conn, bool reinit) {
    struct http_connect_state *s;
    if(!reinit) {
        DEBUGASSERT(!conn->connect_state);
        s = calloc(1, sizeof(struct http_connect_state));
        if(!s) return CURLE_OUT_OF_MEMORY;
        infof(conn->data, "allocate connect buffer!\n");
        conn->connect_state = s;
        Curl_dyn_init(&s->rcvbuf, DYN_PROXY_CONNECT_HEADERS);
    } else {
        DEBUGASSERT(conn->connect_state);
        s = conn->connect_state;
        Curl_dyn_reset(&s->rcvbuf);
    }
    s->tunnel_state = TUNNEL_INIT;
    s->keepon = TRUE;
    s->cl = 0;
    s->close_connection = FALSE;
    return CURLE_OK;
}
static void connect_done(struct connectdata *conn) {
    struct http_connect_state *s = conn->connect_state;
    s->tunnel_state = TUNNEL_COMPLETE;
    Curl_dyn_free(&s->rcvbuf);
    infof(conn->data, "CONNECT phase completed!\n");
}
static CURLcode CONNECT(struct connectdata *conn, int sockindex, const char *hostname, int remote_port) {
    int subversion = 0;
    struct Curl_easy *data = conn->data;
    struct SingleRequest *k = &data->req;
    CURLcode result;
    curl_socket_t tunnelsocket = conn->sock[sockindex];
    struct http_connect_state *s = conn->connect_state;
    char *linep;
    size_t perline;
    #define SELECT_OK      0
    #define SELECT_ERROR   1
    if(Curl_connect_complete(conn)) return CURLE_OK;
    conn->bits.proxy_connect_closed = FALSE;
    do {
        timediff_t check;
        if(TUNNEL_INIT == s->tunnel_state) {
            char *host_port;
            struct dynbuf req;
            infof(data, "Establish HTTP proxy tunnel to %s:%d\n", hostname, remote_port);
            free(data->req.newurl);
            data->req.newurl = NULL;
            host_port = aprintf("%s:%d", hostname, remote_port);
            if(!host_port) return CURLE_OUT_OF_MEMORY;
            Curl_dyn_init(&req, DYN_HTTP_REQUEST);
            result = Curl_http_output_auth(conn, "CONNECT", host_port, TRUE);
            free(host_port);
            if(!result) {
                char *host = NULL;
                const char *proxyconn = "";
                const char *useragent = "";
                const char *httpv = (conn->http_proxy.proxytype == CURLPROXY_HTTP_1_0) ? "1.0" : "1.1";
                bool ipv6_ip = conn->bits.ipv6_ip;
                char *hostheader;
                if (hostname != conn->host.name) ipv6_ip = (strchr(hostname, ':') != NULL);
                    aprintf("%s%s%s:%d", ipv6_ip?"[":"", hostname, ipv6_ip?"]":"", remote_port);
                if (!hostheader) {
                    Curl_dyn_free(&req);
                    return CURLE_OUT_OF_MEMORY;
                }
                if (!Curl_checkProxyheaders(conn, "Host")) {
                    host = aprintf("Host: %s\r\n", hostheader);
                    if (!host) {
                        free(hostheader);
                        Curl_dyn_free(&req);
                        return CURLE_OUT_OF_MEMORY;
                    }
                }
                if (!Curl_checkProxyheaders(conn, "Proxy-Connection")) proxyconn = "Proxy-Connection: Keep-Alive\r\n";
                if (!Curl_checkProxyheaders(conn, "User-Agent") && data->set.str[STRING_USERAGENT]) useragent = data->state.aptr.uagent;
                result = Curl_dyn_addf(&req, "CONNECT %s HTTP/%s\r\n" "%s" "%s" "%s" "%s", hostheader, httpv, host ? host : "",
                                       data->state.aptr.proxyuserpwd ? data->state.aptr.proxyuserpwd : "", useragent, proxyconn);
                if (host) free(host);
                free(hostheader);
                if (!result) result = Curl_add_custom_headers(conn, TRUE, &req);
                if (!result) result = Curl_dyn_add(&req, "\r\n");
                if (!result) result = Curl_buffer_send(&req, conn, &data->info.request_size, 0, sockindex);
                if (result) failf(data, "Failed sending CONNECT to proxy");
            }
            Curl_dyn_free(&req);
            if (result) return result;
            s->tunnel_state = TUNNEL_CONNECT;
        }
        check = Curl_timeleft(data, NULL, TRUE);
        if (check <= 0) {
            failf(data, "Proxy CONNECT aborted due to timeout");
            return CURLE_OPERATION_TIMEDOUT;
        }
        if (!Curl_conn_data_pending(conn, sockindex)) return CURLE_OK;
        int error = SELECT_OK;
        while(s->keepon) {
            ssize_t gotbytes;
            char byte;
            result = Curl_read(conn, tunnelsocket, &byte, 1, &gotbytes);
            if(result == CURLE_AGAIN)  return CURLE_OK;
            if(Curl_pgrsUpdate(conn)) return CURLE_ABORTED_BY_CALLBACK;
            if(result) {
                s->keepon = FALSE;
                break;
            } else if(gotbytes <= 0) {
                if(data->set.proxyauth && data->state.authproxy.avail) {
                    conn->bits.proxy_connect_closed = TRUE;
                    infof(data, "Proxy CONNECT connection closed\n");
                } else {
                    error = SELECT_ERROR;
                    failf(data, "Proxy CONNECT aborted");
                }
                s->keepon = FALSE;
                break;
            }
            if(s->keepon > TRUE) {
                if(s->cl) {
                    s->cl--;
                    if(s->cl <= 0) {
                        s->keepon = FALSE;
                        s->tunnel_state = TUNNEL_COMPLETE;
                        break;
                    }
                } else {
                    CHUNKcode r;
                    CURLcode extra;
                    ssize_t tookcareof = 0;
                    r = Curl_httpchunk_read(conn, &byte, 1, &tookcareof, &extra);
                    if(r == CHUNKE_STOP) {
                        infof(data, "chunk reading DONE\n");
                        s->keepon = FALSE;
                        s->tunnel_state = TUNNEL_COMPLETE;
                    }
                }
                continue;
            }
            if(Curl_dyn_addn(&s->rcvbuf, &byte, 1)) {
                failf(data, "CONNECT response too large!");
                return CURLE_RECV_ERROR;
            }
            if(byte != 0x0a)  continue;
            linep = Curl_dyn_ptr(&s->rcvbuf);
            perline = Curl_dyn_len(&s->rcvbuf);
            result = Curl_convert_from_network(data, linep, perline);
            if(result) return result;
            if(data->set.verbose) Curl_debug(data, CURLINFO_HEADER_IN, linep, perline);
            if(!data->set.suppress_connect_headers) {
                int writetype = CLIENTWRITE_HEADER;
                if(data->set.include_header) writetype |= CLIENTWRITE_BODY;
                result = Curl_client_write(conn, writetype, linep, perline);
                if(result) return result;
            }
            data->info.header_size += (long)perline;
            data->req.headerbytecount += (long)perline;
            if(('\r' == linep[0]) || ('\n' == linep[0])) {
                if((407 == k->httpcode) && !data->state.authproblem) {
                    s->keepon = 2;
                    if(s->cl) infof(data, "Ignore %" CURL_FORMAT_CURL_OFF_T " bytes of response-body\n", s->cl);
                    else if(s->chunked_encoding) {
                        CHUNKcode r;
                        CURLcode extra;
                        infof(data, "Ignore chunked response-body\n");
                        k->ignorebody = TRUE;
                        if(linep[1] == '\n') linep++;
                        r = Curl_httpchunk_read(conn, linep + 1, 1, &gotbytes, &extra);
                        if(r == CHUNKE_STOP) {
                            infof(data, "chunk reading DONE\n");
                            s->keepon = FALSE;
                            s->tunnel_state = TUNNEL_COMPLETE;
                        }
                    } else  s->keepon = FALSE;
                } else s->keepon = FALSE;
                if(!s->cl) s->tunnel_state = TUNNEL_COMPLETE;
                continue;
            }
            if((checkprefix("WWW-Authenticate:", linep) && (401 == k->httpcode)) || (checkprefix("Proxy-authenticate:", linep) && (407 == k->httpcode))) {
                bool proxy = (k->httpcode == 407) ? TRUE : FALSE;
                char *auth = Curl_copy_header_value(linep);
                if(!auth) return CURLE_OUT_OF_MEMORY;
                result = Curl_http_input_auth(conn, proxy, auth);
                free(auth);
                if(result) return result;
            } else if(checkprefix("Content-Length:", linep)) {
                if(k->httpcode/100 == 2) infof(data, "Ignoring Content-Length in CONNECT %03d response\n", k->httpcode);
                else (void)curlx_strtoofft(linep + strlen("Content-Length:"), NULL, 10, &s->cl);
            } else if(Curl_compareheader(linep, "Connection:", "close")) s->close_connection = TRUE;
            else if(checkprefix("Transfer-Encoding:", linep)) {
                if(k->httpcode/100 == 2) infof(data, "Ignoring Transfer-Encoding in " "CONNECT %03d response\n", k->httpcode);
                else if(Curl_compareheader(linep, "Transfer-Encoding:", "chunked")) {
                    infof(data, "CONNECT responded chunked\n");
                    s->chunked_encoding = TRUE;
                    Curl_httpchunk_init(conn);
                }
            } else if(Curl_compareheader(linep, "Proxy-Connection:", "close")) s->close_connection = TRUE;
            else if(2 == sscanf(linep, "HTTP/1.%d %d", &subversion, &k->httpcode)) data->info.httpproxycode = k->httpcode;
            Curl_dyn_reset(&s->rcvbuf);
        }
        if(Curl_pgrsUpdate(conn)) return CURLE_ABORTED_BY_CALLBACK;
        if(error) return CURLE_RECV_ERROR;
        if(data->info.httpproxycode/100 != 2) {
            result = Curl_http_auth_act(conn);
            if(result) return result;
            if(conn->bits.close) s->close_connection = TRUE;
        }
        if(s->close_connection && data->req.newurl) {
            Curl_closesocket(conn, conn->sock[sockindex]);
            conn->sock[sockindex] = CURL_SOCKET_BAD;
            break;
        }
        if(data->req.newurl && (TUNNEL_COMPLETE == s->tunnel_state)) connect_init(conn, TRUE);
    } while(data->req.newurl);
    if(data->info.httpproxycode/100 != 2) {
        if(s->close_connection && data->req.newurl) {
            conn->bits.proxy_connect_closed = TRUE;
            infof(data, "Connect me again please\n");
            connect_done(conn);
        } else {
            free(data->req.newurl);
            data->req.newurl = NULL;
            streamclose(conn, "proxy CONNECT failure");
            Curl_closesocket(conn, conn->sock[sockindex]);
            conn->sock[sockindex] = CURL_SOCKET_BAD;
        }
        s->tunnel_state = TUNNEL_INIT;
        if(conn->bits.proxy_connect_closed) return CURLE_OK;
        Curl_dyn_free(&s->rcvbuf);
        failf(data, "Received HTTP code %d from proxy after CONNECT", data->req.httpcode);
        return CURLE_RECV_ERROR;
    }
    s->tunnel_state = TUNNEL_COMPLETE;
    Curl_safefree(data->state.aptr.proxyuserpwd);
    data->state.aptr.proxyuserpwd = NULL;
    data->state.authproxy.done = TRUE;
    data->state.authproxy.multipass = FALSE;
    infof(data, "Proxy replied %d to CONNECT request\n", data->info.httpproxycode);
    data->req.ignorebody = FALSE;
    conn->bits.rewindaftersend = FALSE;
    Curl_dyn_free(&s->rcvbuf);
    return CURLE_OK;
}
void Curl_connect_free(struct Curl_easy *data) {
    struct connectdata *conn = data->conn;
    struct http_connect_state *s = conn->connect_state;
    if(s) {
        free(s);
        conn->connect_state = NULL;
    }
}
CURLcode Curl_proxyCONNECT(struct connectdata *conn, int sockindex, const char *hostname, int remote_port) {
    CURLcode result;
    if(!conn->connect_state) {
        result = connect_init(conn, FALSE);
        if(result) return result;
    }
    result = CONNECT(conn, sockindex, hostname, remote_port);
    if(result || Curl_connect_complete(conn)) connect_done(conn);
    return result;
}
#else
void Curl_connect_free(struct Curl_easy *data) { (void)data; }
#endif