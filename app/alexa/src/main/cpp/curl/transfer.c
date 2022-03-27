#include <stdbool.h>
#include <assert.h>
#include "curl_setup.h"
#include "strtoofft.h"
#include "typecheck-gcc.h"
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#elif defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#ifndef HAVE_SOCKET
#error "We can't compile without socket() support!"
#endif
#include "urldata.h"
#include "curl.h"
#include "netrc.h"
#include "content_encoding.h"
#include "hostip.h"
#include "transfer.h"
#include "sendf.h"
#include "speedcheck.h"
#include "progress.h"
#include "http.h"
#include "url.h"
#include "getinfo.h"
#include "vtls/vtls.h"
#include "select.h"
#include "multiif.h"
#include "connect.h"
#include "non-ascii.h"
#include "http2.h"
#include "mime.h"
#include "strcase.h"
#include "urlapi-int.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

#if !defined(CURL_DISABLE_HTTP) || !defined(CURL_DISABLE_SMTP) || !defined(CURL_DISABLE_IMAP)
char *Curl_checkheaders(const struct connectdata *conn, const char *thisheader) {
    struct curl_slist *head;
    size_t thislen = strlen(thisheader);
    struct Curl_easy *data = conn->data;
    for(head = data->set.headers; head; head = head->next) {
        if(strncasecompare(head->data, thisheader, thislen) && Curl_headersep(head->data[thislen]) ) return head->data;
    }
    return NULL;
}
#endif
CURLcode Curl_get_upload_buffer(struct Curl_easy *data) {
    if(!data->state.ulbuf) {
        data->state.ulbuf = malloc(data->set.upload_buffer_size);
        if(!data->state.ulbuf) return CURLE_OUT_OF_MEMORY;
    }
    return CURLE_OK;
}
#ifndef CURL_DISABLE_HTTP
static size_t Curl_trailers_read(char *buffer, size_t size, size_t nitems, void *raw) {
    struct Curl_easy *data = (struct Curl_easy *)raw;
    struct dynbuf *trailers_buf = &data->state.trailers_buf;
    size_t bytes_left = Curl_dyn_len(trailers_buf) - data->state.trailers_bytes_sent;
    size_t to_copy = (size*nitems < bytes_left) ? size*nitems : bytes_left;
    if(to_copy) {
        memcpy(buffer,Curl_dyn_ptr(trailers_buf) + data->state.trailers_bytes_sent, to_copy);
        data->state.trailers_bytes_sent += to_copy;
    }
    return to_copy;
}
static size_t Curl_trailers_left(void *raw) {
    struct Curl_easy *data = (struct Curl_easy *)raw;
    struct dynbuf *trailers_buf = &data->state.trailers_buf;
    return Curl_dyn_len(trailers_buf) - data->state.trailers_bytes_sent;
}
#endif
CURLcode Curl_fillreadbuffer(struct connectdata *conn, size_t bytes, size_t *nreadp) {
    struct Curl_easy *data = conn->data;
    size_t buffersize = bytes;
    size_t nread;
    curl_read_callback readfunc = NULL;
    void *extra_data = NULL;
#ifdef CURL_DOES_CONVERSIONS
    bool sending_http_headers = FALSE;
    if(conn->handler->protocol&(PROTO_FAMILY_HTTP|CURLPROTO_RTSP)) {
        const struct HTTP *http = data->req.protop;
        if(http->sending == HTTPSEND_REQUEST) sending_http_headers = TRUE;
    }
#endif
#ifndef CURL_DISABLE_HTTP
    if(data->state.trailers_state == TRAILERS_INITIALIZED) {
        struct curl_slist *trailers = NULL;
        CURLcode result;
        int trailers_ret_code;
        infof(data, "Moving trailers state machine from initialized to sending.\n");
        data->state.trailers_state = TRAILERS_SENDING;
        Curl_dyn_init(&data->state.trailers_buf, DYN_TRAILERS);
        data->state.trailers_bytes_sent = 0;
        Curl_set_in_callback(data, true);
        trailers_ret_code = data->set.trailer_callback(&trailers, data->set.trailer_data);
        Curl_set_in_callback(data, false);
        if(trailers_ret_code == CURL_TRAILERFUNC_OK) result = Curl_http_compile_trailers(trailers, &data->state.trailers_buf, data);
        else {
            failf(data, "operation aborted by trailing headers callback");
            *nreadp = 0;
            result = CURLE_ABORTED_BY_CALLBACK;
        }
        if(result) {
            Curl_dyn_free(&data->state.trailers_buf);
            curl_slist_free_all(trailers);
            return result;
        }
        infof(data, "Successfully compiled trailers.\r\n");
        curl_slist_free_all(trailers);
    }
#endif
    if(data->req.upload_chunky && data->state.trailers_state == TRAILERS_NONE) {
        buffersize -= (8 + 2 + 2);
        data->req.upload_fromhere += (8 + 2);
    }
#ifndef CURL_DISABLE_HTTP
    if(data->state.trailers_state == TRAILERS_SENDING) {
        readfunc = Curl_trailers_read;
        extra_data = (void *)data;
    } else
#endif
    {
        readfunc = data->state.fread_func;
        extra_data = data->state.in;
    }
    Curl_set_in_callback(data, true);
    nread = readfunc(data->req.upload_fromhere, 1, buffersize, extra_data);
    Curl_set_in_callback(data, false);
    if(nread == CURL_READFUNC_ABORT) {
        failf(data, "operation aborted by callback");
        *nreadp = 0;
        return CURLE_ABORTED_BY_CALLBACK;
    }
    if(nread == CURL_READFUNC_PAUSE) {
        struct SingleRequest *k = &data->req;
        if(conn->handler->flags & PROTOPT_NONETWORK) {
            failf(data, "Read callback asked for PAUSE when not supported!");
            return CURLE_READ_ERROR;
        }
        k->keepon |= KEEP_SEND_PAUSE;
        if(data->req.upload_chunky)  data->req.upload_fromhere -= (8 + 2);
        *nreadp = 0;
        return CURLE_OK;
    } else if(nread > buffersize) {
        *nreadp = 0;
        failf(data, "read function returned funny value");
        return CURLE_READ_ERROR;
    }
    if(!data->req.forbidchunk && data->req.upload_chunky) {
        bool added_crlf = FALSE;
        int hexlen = 0;
        const char *endofline_native;
        const char *endofline_network;
        if(
        #ifdef CURL_DO_LINEEND_CONV
           (data->set.prefer_ascii) ||
        #endif
           (data->set.crlf)) {
            endofline_native  = "\n";
            endofline_network = "\x0a";
        } else {
            endofline_native  = "\r\n";
            endofline_network = "\x0d\x0a";
        }
        if(data->state.trailers_state != TRAILERS_SENDING) {
            char hexbuffer[11] = "";
            hexlen = msnprintf(hexbuffer, sizeof(hexbuffer), "%zx%s", nread, endofline_native);
            data->req.upload_fromhere -= hexlen;
            nread += hexlen;
            memcpy(data->req.upload_fromhere, hexbuffer, hexlen);
        #ifndef CURL_DISABLE_HTTP
            if((nread-hexlen) == 0 && data->set.trailer_callback != NULL && data->state.trailers_state == TRAILERS_NONE) {
                data->state.trailers_state = TRAILERS_INITIALIZED;
            } else
        #endif
            {
              memcpy(data->req.upload_fromhere + nread,
                     endofline_network,
                     strlen(endofline_network));
              added_crlf = TRUE;
            }
        }
    #ifdef CURL_DOES_CONVERSIONS
        CURLcode result;
        size_t length;
        if(data->set.prefer_ascii) length = nread;
        else length = hexlen;
        if(length) {
            result = Curl_convert_to_network(data, data->req.upload_fromhere, length);
            if(result) return result;
        }
    #endif
    #ifndef CURL_DISABLE_HTTP
        if(data->state.trailers_state == TRAILERS_SENDING && !Curl_trailers_left(data)) {
            Curl_dyn_free(&data->state.trailers_buf);
            data->state.trailers_state = TRAILERS_DONE;
            data->set.trailer_data = NULL;
            data->set.trailer_callback = NULL;
            data->req.upload_done = TRUE;
            infof(data, "Signaling end of chunked upload after trailers.\n");
        } else
    #endif
        if((nread - hexlen) == 0 && data->state.trailers_state != TRAILERS_INITIALIZED) {
            data->req.upload_done = TRUE;
            infof(data, "Signaling end of chunked upload via terminating chunk.\n");
        }
        if(added_crlf) nread += strlen(endofline_network);
    }
#ifdef CURL_DOES_CONVERSIONS
    else if((data->set.prefer_ascii) && (!sending_http_headers)) {
        CURLcode result;
        result = Curl_convert_to_network(data, data->req.upload_fromhere, nread);
        if(result) return result;
    }
#endif
    *nreadp = nread;
    return CURLE_OK;
}
CURLcode Curl_readrewind(struct connectdata *conn) {
    struct Curl_easy *data = conn->data;
    curl_mimepart *mimepart = &data->set.mimepost;
    conn->bits.rewindaftersend = FALSE;
    data->req.keepon &= ~KEEP_SEND;
    if(conn->handler->protocol & PROTO_FAMILY_HTTP) {
        struct HTTP *http = data->req.protop;
        if(http->sendit) mimepart = http->sendit;
    }
    if(data->set.postfields);
    else if(data->state.httpreq == HTTPREQ_POST_MIME || data->state.httpreq == HTTPREQ_POST_FORM) {
        if(Curl_mime_rewind(mimepart)) {
            failf(data, "Cannot rewind mime/post data");
            return CURLE_SEND_FAIL_REWIND;
        }
    } else {
        if(data->set.seek_func) {
            int err;
            Curl_set_in_callback(data, true);
            err = (data->set.seek_func)(data->set.seek_client, 0, SEEK_SET);
            Curl_set_in_callback(data, false);
            if(err) {
              failf(data, "seek callback returned error %d", (int)err);
              return CURLE_SEND_FAIL_REWIND;
            }
        } else if(data->set.ioctl_func) {
            curlioerr err;
            Curl_set_in_callback(data, true);
            err = (data->set.ioctl_func)(data, CURLIOCMD_RESTARTREAD, data->set.ioctl_client);
            Curl_set_in_callback(data, false);
            infof(data, "the ioctl callback returned %d\n", (int)err);
            if(err) {
              failf(data, "ioctl callback returned error %d", (int)err);
              return CURLE_SEND_FAIL_REWIND;
            }
        } else {
            if(data->state.fread_func == (curl_read_callback)fread) {
                if(-1 != fseek(data->state.in, 0, SEEK_SET)) return CURLE_OK;
            }
            failf(data, "necessary data rewind wasn't possible");
            return CURLE_SEND_FAIL_REWIND;
        }
    }
    return CURLE_OK;
}
static int data_pending(const struct Curl_easy *data) {
    struct connectdata *conn = data->conn;
    return conn->handler->protocol & (CURLPROTO_SCP | CURLPROTO_SFTP) ||
        #if defined(USE_NGHTTP2)
           Curl_ssl_data_pending(conn, FIRSTSOCKET) || ((conn->handler->protocol&PROTO_FAMILY_HTTP) && conn->httpversion >= 20);
        #elif defined(ENABLE_QUIC)
           Curl_ssl_data_pending(conn, FIRSTSOCKET) || Curl_quic_data_pending(data);
        #else
           Curl_ssl_data_pending(conn, FIRSTSOCKET);
        #endif
}
bool Curl_meets_timecondition(struct Curl_easy *data, time_t timeofdoc) {
    if((timeofdoc == 0) || (data->set.timevalue == 0)) return TRUE;
    switch(data->set.timecondition) {
        case CURL_TIMECOND_IFUNMODSINCE:
            if(timeofdoc >= data->set.timevalue) {
                infof(data, "The requested document is not old enough\n");
                data->info.timecond = TRUE;
                return FALSE;
            }
            break;
        case CURL_TIMECOND_IFMODSINCE: default:
            if(timeofdoc <= data->set.timevalue) {
                infof(data, "The requested document is not new enough\n");
                data->info.timecond = TRUE;
                return FALSE;
            }
            break;

    }
    return TRUE;
}
static CURLcode readwrite_data(struct Curl_easy *data, struct connectdata *conn, struct SingleRequest *k, int *didwhat, bool *done, bool *comeback) {
    CURLcode result = CURLE_OK;
    ssize_t nread;
    size_t excess = 0;
    bool readmore = FALSE;
    int maxloops = 100;
    char *buf = data->state.buffer;
    DEBUGASSERT(buf);
    *done = FALSE;
    *comeback = FALSE;
    do {
        bool is_empty_data = FALSE;
        size_t buffersize = data->set.buffer_size;
        size_t bytestoread = buffersize;
    #ifdef USE_NGHTTP2
        bool is_http2 = ((conn->handler->protocol & PROTO_FAMILY_HTTP) && (conn->httpversion == 20));
    #endif
        if(
        #ifdef USE_NGHTTP2
           !is_http2 &&
        #endif
           k->size != -1 && !k->header) {
            curl_off_t totalleft = k->size - k->bytecount;
            if(totalleft < (curl_off_t)bytestoread) bytestoread = (size_t)totalleft;
        }
        if(bytestoread) {
            result = Curl_read(conn, conn->sockfd, buf, bytestoread, &nread);
            if(CURLE_AGAIN == result) break;
            if(result>0) return result;
        } else {
            DEBUGF(infof(data, "readwrite_data: we're done!\n"));
            nread = 0;
        }
        if(!k->bytecount) {
            Curl_pgrsTime(data, TIMER_STARTTRANSFER);
            if(k->exp100 > EXP100_SEND_DATA) k->start100 = Curl_now();
        }
        *didwhat |= KEEP_RECV;
        is_empty_data = ((nread == 0) && (k->bodywrites == 0)) ? TRUE : FALSE;
        if(0 < nread || is_empty_data) buf[nread] = 0;
        else {
        #ifdef USE_NGHTTP2
            if(is_http2 && !nread) DEBUGF(infof(data, "nread == 0, stream closed, bailing\n"));
            else
        #endif
            DEBUGF(infof(data, "nread <= 0, server closed connection, bailing\n"));
            k->keepon &= ~KEEP_RECV;
            break;
        }
        k->str = buf;
        if(conn->handler->readwrite) {
            result = conn->handler->readwrite(data, conn, &nread, &readmore);
            if(result) return result;
            if(readmore) break;
        }
    #ifndef CURL_DISABLE_HTTP
        if(k->header) {
            bool stop_reading = FALSE;
            result = Curl_http_readwrite_headers(data, conn, &nread, &stop_reading);
            if(result) return result;
            if(conn->handler->readwrite && (k->maxdownload <= 0 && nread > 0)) {
                result = conn->handler->readwrite(data, conn, &nread, &readmore);
                if(result) return result;
                if(readmore) break;
            }
            if(stop_reading) {
                if(nread > 0) infof(data, "Excess found: excess = %zd url = %s (zero-length body)\n", nread, data->state.up.path);
                break;
            }
        }
    #endif
        if(!k->header && (nread > 0 || is_empty_data)) {
            if(data->set.opt_no_body) {
                streamclose(conn, "ignoring body");
                *done = TRUE;
                return CURLE_WEIRD_SERVER_REPLY;
            }
        #ifndef CURL_DISABLE_HTTP
            if(0 == k->bodywrites && !is_empty_data) {
                if(conn->handler->protocol&(PROTO_FAMILY_HTTP|CURLPROTO_RTSP)) {
                    if(data->req.newurl) {
                        if(conn->bits.close) {
                            k->keepon &= ~KEEP_RECV;
                            *done = TRUE;
                            return CURLE_OK;
                        }
                        k->ignorebody = TRUE;
                        infof(data, "Ignoring the response-body\n");
                    }
                    if(data->state.resume_from && !k->content_range && (data->state.httpreq == HTTPREQ_GET) && !k->ignorebody) {
                        if(k->size == data->state.resume_from) {
                            infof(data, "The entire document is already downloaded");
                            connclose(conn, "already downloaded");
                            k->keepon &= ~KEEP_RECV;
                            *done = TRUE;
                            return CURLE_OK;
                        }
                        failf(data, "HTTP server doesn't seem to support byte ranges. Cannot resume.");
                        return CURLE_RANGE_ERROR;
                    }
                    if(data->set.timecondition && !data->state.range) {
                        if(!Curl_meets_timecondition(data, k->timeofdoc)) {
                            *done = TRUE;
                            data->info.httpcode = 304;
                            infof(data, "Simulate a HTTP 304 response!\n");
                            connclose(conn, "Simulated 304 handling");
                            return CURLE_OK;
                        }
                    }
                }
            }
        #endif
            k->bodywrites++;
            if(data->set.verbose) {
                if(k->badheader) {
                    Curl_debug(data, CURLINFO_DATA_IN, Curl_dyn_ptr(&data->state.headerb), Curl_dyn_len(&data->state.headerb));
                    if(k->badheader == HEADER_PARTHEADER) Curl_debug(data, CURLINFO_DATA_IN, k->str, (size_t)nread);
                } else Curl_debug(data, CURLINFO_DATA_IN, k->str, (size_t)nread);
            }
        #ifndef CURL_DISABLE_HTTP
            if(k->chunk) {
                CURLcode extra;
                CHUNKcode res = Curl_httpchunk_read(conn, k->str, nread, &nread, &extra);
                if(CHUNKE_OK < res) {
                    if(CHUNKE_PASSTHRU_ERROR == res) {
                        failf(data, "Failed reading the chunked-encoded stream");
                        return extra;
                    }
                    failf(data, "%s in chunked-encoding", Curl_chunked_strerror(res));
                    return CURLE_RECV_ERROR;
                }
                if(CHUNKE_STOP == res) {
                    size_t dataleft;
                    k->keepon &= ~KEEP_RECV;
                    dataleft = conn->chunk.dataleft;
                    if(dataleft != 0) infof(conn->data, "Leftovers after chunking: %zu bytes\n", dataleft);
                }
            }
        #endif
            if((k->badheader == HEADER_PARTHEADER) && !k->ignorebody) {
                size_t headlen = Curl_dyn_len(&data->state.headerb);
                DEBUGF(infof(data, "Increasing bytecount by %zu\n", headlen));
                k->bytecount += headlen;
            }
            if((-1 != k->maxdownload) && (k->bytecount + nread >= k->maxdownload)) {
                excess = (size_t)(k->bytecount + nread - k->maxdownload);
                if(excess > 0 && !k->ignorebody) {
                    infof(data, "Excess found in a read: excess = %zu, size = %" CURL_FORMAT_CURL_OFF_T ", maxdownload = %" CURL_FORMAT_CURL_OFF_T
                          ", bytecount = %" CURL_FORMAT_CURL_OFF_T "\n", excess, k->size, k->maxdownload, k->bytecount);
                    connclose(conn, "excess found in a read");
                }
                nread = (ssize_t) (k->maxdownload - k->bytecount);
                if(nread < 0) nread = 0;
                k->keepon &= ~KEEP_RECV;
            }
            k->bytecount += nread;
            Curl_pgrsSetDownloadCounter(data, k->bytecount);
            if(!k->chunk && (nread || k->badheader || is_empty_data)) {
                if(k->badheader && !k->ignorebody) {
                    size_t headlen = Curl_dyn_len(&data->state.headerb);
                    if(k->maxdownload == -1 || (curl_off_t)headlen <= k->maxdownload) {
                        result = Curl_client_write(conn, CLIENTWRITE_BODY, Curl_dyn_ptr(&data->state.headerb), headlen);
                    } else result = Curl_client_write(conn, CLIENTWRITE_BODY, Curl_dyn_ptr(&data->state.headerb), (size_t)k->maxdownload);
                    if(result) return result;
                }
                if(k->badheader < HEADER_ALLBAD) {
                    if(conn->data->set.http_ce_skip || !k->writer_stack) {
                        if(!k->ignorebody) {
                        #ifndef CURL_DISABLE_POP3
                            if(conn->handler->protocol & PROTO_FAMILY_POP3) result = Curl_pop3_write(conn, k->str, nread);
                            else
                        #endif
                            result = Curl_client_write(conn, CLIENTWRITE_BODY, k->str, nread);
                        }
                    } else if(!k->ignorebody)
                    result = Curl_unencode_write(conn, k->writer_stack, k->str, nread);
                }
                k->badheader = HEADER_NORMAL;
                if(result) return result;
            }
        }
        if(conn->handler->readwrite && excess) {
            k->str += nread;
            if(&k->str[excess] > &buf[data->set.buffer_size]) excess = &buf[data->set.buffer_size] - k->str;
            nread = (ssize_t)excess;
            result = conn->handler->readwrite(data, conn, &nread, &readmore);
            if(result) return result;
            if(readmore) k->keepon |= KEEP_RECV;
            break;
        }
        if(is_empty_data) k->keepon &= ~KEEP_RECV;
        if(k->keepon & KEEP_RECV_PAUSE) break;
    } while(data_pending(data) && maxloops--);
    if(maxloops <= 0) {
        conn->cselect_bits = CURL_CSELECT_IN;
        *comeback = TRUE;
    }
    if(((k->keepon & (KEEP_RECV|KEEP_SEND)) == KEEP_SEND) && conn->bits.close) {
        infof(data, "we are done reading and this is set to close, stop send\n");
        k->keepon &= ~KEEP_SEND;
    }
    return CURLE_OK;
}
CURLcode Curl_done_sending(struct connectdata *conn, struct SingleRequest *k) {
    k->keepon &= ~KEEP_SEND;
    Curl_http2_done_sending(conn);
    Curl_quic_done_sending(conn);
    if(conn->bits.rewindaftersend) {
        CURLcode result = Curl_readrewind(conn);
        if(result) return result;
    }
    return CURLE_OK;
}
#if defined(WIN32) && !defined(USE_LWIPSOCK)
#ifndef SIO_IDEAL_SEND_BACKLOG_QUERY
#define SIO_IDEAL_SEND_BACKLOG_QUERY 0x4004747B
#endif
static void win_update_buffer_size(curl_socket_t sockfd) {
    int result;
    ULONG ideal;
    DWORD ideallen;
    result = WSAIoctl(sockfd, SIO_IDEAL_SEND_BACKLOG_QUERY, 0, 0, &ideal, sizeof(ideal), &ideallen, 0, 0);
    if(result == 0) setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char *)&ideal, sizeof(ideal));
}
#else
#define win_update_buffer_size(x)
#endif
static CURLcode readwrite_upload(struct Curl_easy *data, struct connectdata *conn, int *didwhat) {
    ssize_t i, si;
    ssize_t bytes_written;
    CURLcode result;
    ssize_t nread;
    bool sending_http_headers = FALSE;
    struct SingleRequest *k = &data->req;
    if((k->bytecount == 0) && (k->writebytecount == 0)) Curl_pgrsTime(data, TIMER_STARTTRANSFER);
    *didwhat |= KEEP_SEND;
    do {
        if(0 == k->upload_present) {
            result = Curl_get_upload_buffer(data);
            if(result) return result;
            k->upload_fromhere = data->state.ulbuf;
            if(!k->upload_done) {
                size_t fillcount;
                struct HTTP *http = k->protop;
                if((k->exp100 == EXP100_SENDING_REQUEST) && (http->sending == HTTPSEND_BODY)) {
                    k->exp100 = EXP100_AWAITING_CONTINUE;
                    k->keepon &= ~KEEP_SEND;
                    k->start100 = Curl_now();
                    *didwhat &= ~KEEP_SEND;
                    Curl_expire(data, data->set.expect_100_timeout, EXPIRE_100_TIMEOUT);
                    break;
                }
                if(conn->handler->protocol&(PROTO_FAMILY_HTTP|CURLPROTO_RTSP)) {
                    if(http->sending == HTTPSEND_REQUEST) sending_http_headers = TRUE;
                    else sending_http_headers = FALSE;
                }
                result = Curl_fillreadbuffer(conn, data->set.upload_buffer_size, &fillcount);
                if(result) return result;
                nread = fillcount;
            } else nread = 0;
            if(!nread && (k->keepon & KEEP_SEND_PAUSE)) break;
            if(nread <= 0) {
                result = Curl_done_sending(conn, k);
                if(result) return result;
                break;
            }
            k->upload_present = nread;
            if((!sending_http_headers) && (
            #ifdef CURL_DO_LINEEND_CONV
               (data->set.prefer_ascii) ||
            #endif
               (data->set.crlf))) {
                if(!data->state.scratch) {
                    data->state.scratch = malloc(2 * data->set.upload_buffer_size);
                    if(!data->state.scratch) {
                        failf(data, "Failed to alloc scratch buffer!");
                        return CURLE_OUT_OF_MEMORY;
                    }
                }
                for(i = 0, si = 0; i < nread; i++, si++) {
                    if(k->upload_fromhere[i] == 0x0a) {
                        data->state.scratch[si++] = 0x0d;
                        data->state.scratch[si] = 0x0a;
                        if(!data->set.crlf) {
                            if(data->state.infilesize != -1)
                            data->state.infilesize++;
                        }
                    } else data->state.scratch[si] = k->upload_fromhere[i];
                }
                if(si != nread) {
                    nread = si;
                    k->upload_fromhere = data->state.scratch;
                    k->upload_present = nread;
                }
            }
        #ifndef CURL_DISABLE_SMTP
            if(conn->handler->protocol & PROTO_FAMILY_SMTP) {
                result = Curl_smtp_escape_eob(conn, nread);
                if(result) return result;
            }
        #endif
        } else {
          /* We have a partial buffer left from a previous "round". Use
             that instead of reading more data */
        }
        result = Curl_write(conn, conn->writesockfd, k->upload_fromhere, k->upload_present, &bytes_written);
        if(result) return result;
        if(data->set.verbose) Curl_debug(data, CURLINFO_DATA_OUT, k->upload_fromhere, (size_t)bytes_written);
        k->writebytecount += bytes_written;
        Curl_pgrsSetUploadCounter(data, k->writebytecount);
        if((!k->upload_chunky || k->forbidchunk) && (k->writebytecount == data->state.infilesize)) {
            k->upload_done = TRUE;
            infof(data, "We are completely uploaded and fine\n");
        }
        if(k->upload_present != bytes_written) {
            k->upload_present -= bytes_written;
            k->upload_fromhere += bytes_written;
        } else {
            result = Curl_get_upload_buffer(data);
            if(result) return result;
            k->upload_fromhere = data->state.ulbuf;
            k->upload_present = 0;
            if(k->upload_done) {
                result = Curl_done_sending(conn, k);
                if(result) return result;
            }
        }
    } while(0);
    return CURLE_OK;
}
CURLcode Curl_readwrite(struct connectdata *conn, struct Curl_easy *data, bool *done, bool *comeback) {
    struct SingleRequest *k = &data->req;
    CURLcode result;
    int didwhat = 0;
    curl_socket_t fd_read;
    curl_socket_t fd_write;
    int select_res = conn->cselect_bits;
    conn->cselect_bits = 0;
    if((k->keepon & KEEP_RECVBITS) == KEEP_RECV) fd_read = conn->sockfd;
    else fd_read = CURL_SOCKET_BAD;
    if((k->keepon & KEEP_SENDBITS) == KEEP_SEND) fd_write = conn->writesockfd;
    else fd_write = CURL_SOCKET_BAD;
    if(conn->data->state.drain) {
        select_res |= CURL_CSELECT_IN;
        DEBUGF(infof(data, "Curl_readwrite: forcibly told to drain data\n"));
    }
    if(!select_res) select_res = Curl_socket_check(fd_read, CURL_SOCKET_BAD, fd_write, 0);
    if(select_res == CURL_CSELECT_ERR) {
        failf(data, "select/poll returned error");
        return CURLE_SEND_ERROR;
    }
    if((k->keepon & KEEP_RECV) && (select_res & CURL_CSELECT_IN)) {
        result = readwrite_data(data, conn, k, &didwhat, done, comeback);
        if(result || *done) return result;
    }
    if((k->keepon & KEEP_SEND) && (select_res & CURL_CSELECT_OUT)) {
        result = readwrite_upload(data, conn, &didwhat);
        if(result) return result;
    }
    k->now = Curl_now();
    if(didwhat);
    else {
        if(k->exp100 == EXP100_AWAITING_CONTINUE) {
            timediff_t ms = Curl_timediff(k->now, k->start100);
            if(ms >= data->set.expect_100_timeout) {
                k->exp100 = EXP100_SEND_DATA;
                k->keepon |= KEEP_SEND;
                Curl_expire_done(data, EXPIRE_100_TIMEOUT);
                infof(data, "Done waiting for 100-continue\n");
            }
        }
    }
    if(Curl_pgrsUpdate(conn)) result = CURLE_ABORTED_BY_CALLBACK;
    else result = Curl_speedcheck(data, k->now);
    if(result) return result;
    if(k->keepon) {
        if(0 > Curl_timeleft(data, &k->now, FALSE)) {
            if(k->size != -1) {
                failf(data, "Operation timed out after %" CURL_FORMAT_TIMEDIFF_T " milliseconds with %" CURL_FORMAT_CURL_OFF_T " out of %"
                      CURL_FORMAT_CURL_OFF_T " bytes received", Curl_timediff(k->now, data->progress.t_startsingle), k->bytecount, k->size);
            } else {
                failf(data, "Operation timed out after %" CURL_FORMAT_TIMEDIFF_T " milliseconds with %" CURL_FORMAT_CURL_OFF_T " bytes received",
                      Curl_timediff(k->now, data->progress.t_startsingle), k->bytecount);
            }
            return CURLE_OPERATION_TIMEDOUT;
        }
    } else {
        if(!(data->set.opt_no_body) && (k->size != -1) && (k->bytecount != k->size) &&
        #ifdef CURL_DO_LINEEND_CONV
           (k->bytecount != (k->size + data->state.crlf_conversions)) &&
        #endif
           !k->newurl) {
            failf(data, "transfer closed with %" CURL_FORMAT_CURL_OFF_T " bytes remaining to read", k->size - k->bytecount);
            return CURLE_PARTIAL_FILE;
        }
        if(!(data->set.opt_no_body) && k->chunk && (conn->chunk.state != CHUNK_STOP)) {
            failf(data, "transfer closed with outstanding read data remaining");
            return CURLE_PARTIAL_FILE;
        }
        if(Curl_pgrsUpdate(conn)) return CURLE_ABORTED_BY_CALLBACK;
    }
    *done = (0 == (k->keepon&(KEEP_RECV | KEEP_SEND | KEEP_RECV_PAUSE | KEEP_SEND_PAUSE))) ? TRUE : FALSE;
    return CURLE_OK;
}
int Curl_single_getsock(const struct connectdata *conn, curl_socket_t *sock) {
    const struct Curl_easy *data = conn->data;
    int bitmap = GETSOCK_BLANK;
    unsigned sockindex = 0;
    if(conn->handler->perform_getsock) return conn->handler->perform_getsock(conn, sock);
    if((data->req.keepon & KEEP_RECVBITS) == KEEP_RECV) {
        DEBUGASSERT(conn->sockfd != CURL_SOCKET_BAD);
        bitmap |= GETSOCK_READSOCK(sockindex);
        sock[sockindex] = conn->sockfd;
    }
    if((data->req.keepon & KEEP_SENDBITS) == KEEP_SEND) {
        if((conn->sockfd != conn->writesockfd) || bitmap == GETSOCK_BLANK) {
            if(bitmap != GETSOCK_BLANK) sockindex++;
            DEBUGASSERT(conn->writesockfd != CURL_SOCKET_BAD);
            sock[sockindex] = conn->writesockfd;
        }
        bitmap |= GETSOCK_WRITESOCK(sockindex);
    }
    return bitmap;
}
void Curl_init_CONNECT(struct Curl_easy *data) {
    data->state.fread_func = data->set.fread_func_set;
    data->state.in = data->set.in_set;
}
CURLcode Curl_pretransfer(struct Curl_easy *data) {
    CURLcode result;
    if(!data->change.url && !data->set.uh) {
        failf(data, "No URL set!");
        return CURLE_URL_MALFORMAT;
    }
    if(data->change.url_alloc) {
        Curl_safefree(data->change.url);
        data->change.url_alloc = FALSE;
    }
    if(!data->change.url && data->set.uh) {
        CURLUcode uc;
        uc = curl_url_get(data->set.uh,CURLUPART_URL, &data->set.str[STRING_SET_URL], 0);
        if(uc) {
            failf(data, "No URL set!");
            return CURLE_URL_MALFORMAT;
        }
    }
    data->state.httpreq = data->set.method;
    data->change.url = data->set.str[STRING_SET_URL];
    result = Curl_ssl_initsessions(data, data->set.general_ssl.max_ssl_sessions);
    if(result) return result;
    data->state.wildcardmatch = data->set.wildcard_enabled;
    data->set.followlocation = 0;
    data->state.this_is_a_follow = FALSE;
    data->state.errorbuf = FALSE;
    data->state.httpversion = 0;
    data->state.authproblem = FALSE;
    data->state.authhost.want = data->set.httpauth;
    data->state.authproxy.want = data->set.proxyauth;
    Curl_safefree(data->info.wouldredirect);
    if(data->state.httpreq == HTTPREQ_PUT) data->state.infilesize = data->set.filesize;
    else if((data->state.httpreq != HTTPREQ_GET) && (data->state.httpreq != HTTPREQ_HEAD)) {
        data->state.infilesize = data->set.postfieldsize;
        if(data->set.postfields && (data->state.infilesize == -1)) data->state.infilesize = (curl_off_t)strlen(data->set.postfields);
    } else data->state.infilesize = 0;
    if(data->change.cookielist) Curl_cookie_loadfiles(data);
    if(data->change.resolve) result = Curl_loadhostpairs(data);
    if(!result) {
        data->state.allow_port = TRUE;
    #if defined(HAVE_SIGNAL) && defined(SIGPIPE) && !defined(HAVE_MSG_NOSIGNAL)
        if(!data->set.no_signal) data->state.prev_signal = signal(SIGPIPE, SIG_IGN);
    #endif
        Curl_initinfo(data);
        Curl_pgrsResetTransferSizes(data);
        Curl_pgrsStartNow(data);
        data->state.authhost.picked &= data->state.authhost.want;
        data->state.authproxy.picked &= data->state.authproxy.want;
    #ifndef CURL_DISABLE_FTP
        if(data->state.wildcardmatch) {
            struct WildcardData *wc = &data->wildcard;
            if(wc->state < CURLWC_INIT) {
                result = Curl_wildcard_init(wc);
                if(result) return CURLE_OUT_OF_MEMORY;
            }
        }
    #endif
        Curl_http2_init_state(&data->state);
    }
    return result;
}
CURLcode Curl_posttransfer(struct Curl_easy *data) {
#if defined(HAVE_SIGNAL) && defined(SIGPIPE) && !defined(HAVE_MSG_NOSIGNAL)
    if(!data->set.no_signal) signal(SIGPIPE, data->state.prev_signal);
#else
    (void)data;
#endif
    return CURLE_OK;
}
CURLcode Curl_follow(struct Curl_easy *data, char *newurl, followtype type) {
#ifdef CURL_DISABLE_HTTP
    (void)data;
    (void)newurl;
    (void)type;
    return CURLE_TOO_MANY_REDIRECTS;
#else
    bool disallowport = FALSE;
    bool reachedmax = FALSE;
    CURLUcode uc;
    if(type == FOLLOW_REDIR) {
        if((data->set.maxredirs != -1) && (data->set.followlocation >= data->set.maxredirs)) {
            reachedmax = TRUE;
            type = FOLLOW_FAKE;
        } else {
            data->state.this_is_a_follow = TRUE;
            data->set.followlocation++;
            if(data->set.http_auto_referer) {
                if(data->change.referer_alloc) {
                    Curl_safefree(data->change.referer);
                    data->change.referer_alloc = FALSE;
                }
                data->change.referer = strdup(data->change.url);
                if(!data->change.referer) return CURLE_OUT_OF_MEMORY;
                data->change.referer_alloc = TRUE;
            }
        }
    }
    if(Curl_is_absolute_url(newurl, NULL, MAX_SCHEME_LEN)) disallowport = TRUE;
    DEBUGASSERT(data->state.uh);
    uc = curl_url_set(data->state.uh, CURLUPART_URL, newurl,(type == FOLLOW_FAKE) ? CURLU_NON_SUPPORT_SCHEME :
                      ((type == FOLLOW_REDIR) ? CURLU_URLENCODE : 0) );
    if(uc) {
        if(type != FOLLOW_FAKE) return Curl_uc_to_curlcode(uc);
        newurl = strdup(newurl);
        if(!newurl) return CURLE_OUT_OF_MEMORY;
    } else {
        uc = curl_url_get(data->state.uh, CURLUPART_URL, &newurl, 0);
        if(uc) return Curl_uc_to_curlcode(uc);
    }
    if(type == FOLLOW_FAKE) {
    data->info.wouldredirect = newurl;
    if(reachedmax) {
      failf(data, "Maximum (%ld) redirects followed", data->set.maxredirs);
      return CURLE_TOO_MANY_REDIRECTS;
    }
    return CURLE_OK;
    }
    if(disallowport) data->state.allow_port = FALSE;
    if(data->change.url_alloc) Curl_safefree(data->change.url);
    data->change.url = newurl;
    data->change.url_alloc = TRUE;
    infof(data, "Issue another request to this URL: '%s'\n", data->change.url);
    switch(data->info.httpcode) {
        case 301:
            if((data->state.httpreq == HTTPREQ_POST || data->state.httpreq == HTTPREQ_POST_FORM || data->state.httpreq == HTTPREQ_POST_MIME)
               && !(data->set.keep_post & CURL_REDIR_POST_301)) {
                infof(data, "Switch from POST to GET\n");
                data->state.httpreq = HTTPREQ_GET;
            }
            break;
        case 302:
            if((data->state.httpreq == HTTPREQ_POST || data->state.httpreq == HTTPREQ_POST_FORM || data->state.httpreq == HTTPREQ_POST_MIME)
               && !(data->set.keep_post & CURL_REDIR_POST_302)) {
                infof(data, "Switch from POST to GET\n");
                data->state.httpreq = HTTPREQ_GET;
            }
            break;
        case 303:
            if(data->state.httpreq != HTTPREQ_GET && ((data->state.httpreq != HTTPREQ_POST && data->state.httpreq != HTTPREQ_POST_FORM &&
               data->state.httpreq != HTTPREQ_POST_MIME) || !(data->set.keep_post & CURL_REDIR_POST_303))) {
                data->state.httpreq = HTTPREQ_GET;
                data->set.upload = false;
                infof(data, "Switch to %s\n", data->set.opt_no_body?"HEAD":"GET");
            }
            break;
        case 304: case 305: default: break;
    }
    Curl_pgrsTime(data, TIMER_REDIRECT);
    Curl_pgrsResetTransferSizes(data);
    return CURLE_OK;
#endif
}
CURLcode Curl_retry_request(struct connectdata *conn, char **url) {
    struct Curl_easy *data = conn->data;
    bool retry = FALSE;
    *url = NULL;
    if(data->set.upload && !(conn->handler->protocol&(PROTO_FAMILY_HTTP|CURLPROTO_RTSP))) return CURLE_OK;
    if((data->req.bytecount + data->req.headerbytecount == 0) && conn->bits.reuse && (!data->set.opt_no_body || (conn->handler->protocol & PROTO_FAMILY_HTTP)) &&
       (data->set.rtspreq != RTSPREQ_RECEIVE)) {
        retry = TRUE;
    } else if(data->state.refused_stream && (data->req.bytecount + data->req.headerbytecount == 0) ) {
        infof(conn->data, "REFUSED_STREAM, retrying a fresh connect\n");
        data->state.refused_stream = FALSE; /* clear again */
        retry = TRUE;
    }
    if(retry) {
        #define CONN_MAX_RETRIES 5
        if(conn->retrycount++ >= CONN_MAX_RETRIES) {
            failf(data, "Connection died, tried %d times before giving up", CONN_MAX_RETRIES);
            return CURLE_SEND_ERROR;
        }
        infof(conn->data, "Connection died, retrying a fresh connect\n");
        *url = strdup(conn->data->change.url);
        if(!*url) return CURLE_OUT_OF_MEMORY;
        connclose(conn, "retry");
        conn->bits.retry = TRUE;
        if(conn->handler->protocol&PROTO_FAMILY_HTTP) {
            if(data->req.writebytecount) {
                CURLcode result = Curl_readrewind(conn);
                if(result) {
                    Curl_safefree(*url);
                    return result;
                }
            }
        }
    }
    return CURLE_OK;
}
void
Curl_setup_transfer(struct Curl_easy *data, int sockindex, curl_off_t size, bool getheader, int writesockindex) {
    struct SingleRequest *k = &data->req;
    struct connectdata *conn = data->conn;
    struct HTTP *http = data->req.protop;
    bool httpsending = ((conn->handler->protocol&PROTO_FAMILY_HTTP) && (http->sending == HTTPSEND_REQUEST));
    DEBUGASSERT(conn != NULL);
    DEBUGASSERT((sockindex <= 1) && (sockindex >= -1));
    if(conn->bits.multiplex || conn->httpversion == 20 || httpsending) {
        conn->sockfd = sockindex == -1 ? ((writesockindex == -1 ? CURL_SOCKET_BAD : conn->sock[writesockindex])) : conn->sock[sockindex];
        conn->writesockfd = conn->sockfd;
        if(httpsending) writesockindex = FIRSTSOCKET;
    } else {
        conn->sockfd = sockindex == -1 ? CURL_SOCKET_BAD : conn->sock[sockindex];
        conn->writesockfd = writesockindex == -1 ? CURL_SOCKET_BAD:conn->sock[writesockindex];
    }
    k->getheader = getheader;
    k->size = size;
    if(!k->getheader) {
        k->header = FALSE;
        if(size > 0) Curl_pgrsSetDownloadSize(data, size);
    }
    if(k->getheader || !data->set.opt_no_body) {
        if(sockindex != -1) k->keepon |= KEEP_RECV;
        if(writesockindex != -1) {
            if((data->state.expect100header) && (conn->handler->protocol&PROTO_FAMILY_HTTP) && (http->sending == HTTPSEND_BODY)) {
                k->exp100 = EXP100_AWAITING_CONTINUE;
                k->start100 = Curl_now();
                Curl_expire(data, data->set.expect_100_timeout, EXPIRE_100_TIMEOUT);
            } else {
                if(data->state.expect100header) k->exp100 = EXP100_SENDING_REQUEST;
                k->keepon |= KEEP_SEND;
            }
        }
    }
}