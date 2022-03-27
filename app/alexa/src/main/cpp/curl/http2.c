#include "curl_setup.h"

#ifdef USE_NGHTTP2
#include <nghttp2/nghttp2.h>
#include "urldata.h"
#include "http2.h"
#include "http.h"
#include "sendf.h"
#include "select.h"
#include "curl_base64.h"
#include "strcase.h"
#include "multiif.h"
#include "url.h"
#include "connect.h"
#include "strtoofft.h"
#include "strdup.h"
#include "dynbuf.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

#define H2_BUFSIZE 32768
#if (NGHTTP2_VERSION_NUM < 0x010c00)
#error too old nghttp2 version, upgrade!
#endif
#ifdef CURL_DISABLE_VERBOSE_STRINGS
#define nghttp2_session_callbacks_set_error_callback(x,y)
#endif
#if (NGHTTP2_VERSION_NUM >= 0x010c00)
#define NGHTTP2_HAS_SET_LOCAL_WINDOW_SIZE 1
#endif
#define HTTP2_HUGE_WINDOW_SIZE (32 * 1024 * 1024) /* 32 MB */
#ifdef DEBUG_HTTP2
#define H2BUGF(x) x
#else
#define H2BUGF(x) do { } while(0)
#endif
static ssize_t http2_recv(struct connectdata *conn, int sockindex, char *mem, size_t len, CURLcode *err);
static bool http2_connisdead(struct connectdata *conn);
static int h2_session_send(struct Curl_easy *data, nghttp2_session *h2);
static int h2_process_pending_input(struct connectdata *conn, struct http_conn *httpc, CURLcode *err);
void Curl_http2_init_state(struct UrlState *state) { state->stream_weight = NGHTTP2_DEFAULT_WEIGHT; }
void Curl_http2_init_userset(struct UserDefined *set) { set->stream_weight = NGHTTP2_DEFAULT_WEIGHT; }
static int http2_perform_getsock(const struct connectdata *conn, curl_socket_t *sock) {
    const struct http_conn *c = &conn->proto.httpc;
    struct SingleRequest *k = &conn->data->req;
    int bitmap = GETSOCK_BLANK;
    sock[0] = conn->sock[FIRSTSOCKET];
    bitmap |= GETSOCK_READSOCK(FIRSTSOCKET);
    if(((k->keepon & (KEEP_SEND|KEEP_SEND_PAUSE)) == KEEP_SEND) || nghttp2_session_want_write(c->h2)) bitmap |= GETSOCK_WRITESOCK(FIRSTSOCKET);
    return bitmap;
}
static int http2_getsock(struct connectdata *conn, curl_socket_t *socks) { return http2_perform_getsock(conn, socks); }
static void http2_stream_free(struct HTTP *http) {
    if(http) {
        Curl_dyn_free(&http->header_recvbuf);
        for(; http->push_headers_used > 0; --http->push_headers_used) {
            free(http->push_headers[http->push_headers_used - 1]);
        }
        free(http->push_headers);
        http->push_headers = NULL;
    }
}
static CURLcode http2_disconnect(struct connectdata *conn, bool dead_connection) {
    struct http_conn *c = &conn->proto.httpc;
    (void)dead_connection;
    H2BUGF(infof(conn->data, "HTTP/2 DISCONNECT starts now\n"));
    nghttp2_session_del(c->h2);
    Curl_safefree(c->inbuf);
    H2BUGF(infof(conn->data, "HTTP/2 DISCONNECT done\n"));
    return CURLE_OK;
}
static bool http2_connisdead(struct connectdata *conn) {
    int sval;
    bool dead = TRUE;
    if(conn->bits.close) return TRUE;
    sval = SOCKET_READABLE(conn->sock[FIRSTSOCKET], 0);
    if(sval == 0) dead = FALSE;
    else if(sval & CURL_CSELECT_ERR) dead = TRUE;
    else if(sval & CURL_CSELECT_IN) {
        dead = !Curl_connalive(conn);
        if(!dead) {
            CURLcode result;
            struct http_conn *httpc = &conn->proto.httpc;
            ssize_t nread = -1;
            if(httpc->recv_underlying) nread = ((Curl_recv *)httpc->recv_underlying)(conn, FIRSTSOCKET, httpc->inbuf, H2_BUFSIZE, &result);
            if(nread != -1) {
                infof(conn->data, "%d bytes stray data read before trying h2 connection\n", (int)nread);
                httpc->nread_inbuf = 0;
                httpc->inbuflen = nread;
                (void)h2_process_pending_input(conn, httpc, &result);
            } else dead = TRUE;
        }
    }
    return dead;
}
static unsigned int http2_conncheck(struct connectdata *check, unsigned int checks_to_perform) {
    unsigned int ret_val = CONNRESULT_NONE;
    struct http_conn *c = &check->proto.httpc;
    int rc;
    bool send_frames = false;
    if(checks_to_perform & CONNCHECK_ISDEAD) {
    if(http2_connisdead(check))
        ret_val |= CONNRESULT_DEAD;
    }
    if(checks_to_perform & CONNCHECK_KEEPALIVE) {
        struct curltime now = Curl_now();
        timediff_t elapsed = Curl_timediff(now, check->keepalive);
        if(elapsed > check->upkeep_interval_ms) {
            rc = nghttp2_submit_ping(c->h2, 0, ZERO_NULL);
            if(!rc) send_frames = true;
            else failf(check->data, "nghttp2_submit_ping() failed: %s(%d)", nghttp2_strerror(rc), rc);
            check->keepalive = now;
        }
    }
    if(send_frames) {
        rc = nghttp2_session_send(c->h2);
        if(rc) failf(check->data, "nghttp2_session_send() failed: %s(%d)", nghttp2_strerror(rc), rc);
    }
    return ret_val;
}
void Curl_http2_setup_req(struct Curl_easy *data) {
    struct HTTP *http = data->req.protop;
    http->bodystarted = FALSE;
    http->status_code = -1;
    http->pausedata = NULL;
    http->pauselen = 0;
    http->closed = FALSE;
    http->close_handled = FALSE;
    http->mem = NULL;
    http->len = 0;
    http->memlen = 0;
}
void Curl_http2_setup_conn(struct connectdata *conn) {
    conn->proto.httpc.settings.max_concurrent_streams = DEFAULT_MAX_CONCURRENT_STREAMS;
    conn->proto.httpc.error_code = NGHTTP2_NO_ERROR;
}
static const struct Curl_handler Curl_handler_http2 = {
    "HTTP",
    ZERO_NULL,
    Curl_http,
    Curl_http_done,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    http2_getsock,
    http2_getsock,
    ZERO_NULL,
    http2_perform_getsock,
    http2_disconnect,
    ZERO_NULL,
    http2_conncheck,
    PORT_HTTP,
    CURLPROTO_HTTP,
    PROTOPT_STREAM
};
static const struct Curl_handler Curl_handler_http2_ssl = {
    "HTTPS",
    ZERO_NULL,
    Curl_http,
    Curl_http_done,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    http2_getsock,
    http2_getsock,
    ZERO_NULL,
    http2_perform_getsock,
    http2_disconnect,
    ZERO_NULL,
    http2_conncheck,
    PORT_HTTP,
    CURLPROTO_HTTPS,
    PROTOPT_SSL | PROTOPT_STREAM
};
int Curl_http2_ver(char *p, size_t len) {
    nghttp2_info *h2 = nghttp2_version(0);
    return msnprintf(p, len, "nghttp2/%s", h2->version_str);
}
static ssize_t send_callback(nghttp2_session *h2, const uint8_t *data, size_t length, int flags, void *userp) {
    struct connectdata *conn = (struct connectdata *)userp;
    struct http_conn *c = &conn->proto.httpc;
    ssize_t written;
    CURLcode result = CURLE_OK;
    (void)h2;
    (void)flags;
    if (!c->send_underlying) return NGHTTP2_ERR_CALLBACK_FAILURE;
    written = ((Curl_send*)c->send_underlying)(conn, FIRSTSOCKET, data, length, &result);
    if (result == CURLE_AGAIN) return NGHTTP2_ERR_WOULDBLOCK;
    if (written == -1) {
        failf(conn->data, "Failed sending HTTP2 data");
        return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    if (!written) return NGHTTP2_ERR_WOULDBLOCK;
    return written;
}
struct curl_pushheaders {
    struct Curl_easy *data;
    const nghttp2_push_promise *frame;
};
char *curl_pushheader_bynum(struct curl_pushheaders *h, size_t num) {
    if(!h || !GOOD_EASY_HANDLE(h->data)) return NULL;
    else {
        struct HTTP *stream = h->data->req.protop;
        if(num < stream->push_headers_used) return stream->push_headers[num];
    }
    return NULL;
}
char *curl_pushheader_byname(struct curl_pushheaders *h, const char *header) {
    if (!h || !GOOD_EASY_HANDLE(h->data) || !header || !header[0] || !strcmp(header, ":") || strchr(header + 1, ':')) return NULL;
    else {
        struct HTTP *stream = h->data->req.protop;
        size_t len = strlen(header);
        size_t i;
        for (i = 0; i<stream->push_headers_used; i++) {
            if (!strncmp(header, stream->push_headers[i], len)) {
                if (stream->push_headers[i][len] != ':') continue;
                return &stream->push_headers[i][len + 1];
            }
        }
    }
    return NULL;
}
static void drained_transfer(struct Curl_easy *data, struct http_conn *httpc) {
    DEBUGASSERT(httpc->drain_total >= data->state.drain);
    httpc->drain_total -= data->state.drain;
    data->state.drain = 0;
}
static void drain_this(struct Curl_easy *data, struct http_conn *httpc) {
    data->state.drain++;
    httpc->drain_total++;
    DEBUGASSERT(httpc->drain_total >= data->state.drain);
}
static struct Curl_easy *duphandle(struct Curl_easy *data) {
    struct Curl_easy *second = curl_easy_duphandle(data);
    if (second) {
        struct HTTP *http = calloc(1, sizeof(struct HTTP));
        if (!http) (void)Curl_close(&second);
        else {
            second->req.protop = http;
            Curl_dyn_init(&http->header_recvbuf, DYN_H2_HEADERS);
            Curl_http2_setup_req(second);
            second->state.stream_weight = data->state.stream_weight;
        }
    }
    return second;
}
static int set_transfer_url(struct Curl_easy *data, struct curl_pushheaders *hp) {
    const char *v;
    CURLU *u = curl_url();
    CURLUcode uc;
    char *url;
    v = curl_pushheader_byname(hp, ":scheme");
    if (v) {
        uc = curl_url_set(u, CURLUPART_SCHEME, v, 0);
        if (uc) return 1;
    }
    v = curl_pushheader_byname(hp, ":authority");
    if (v) {
        uc = curl_url_set(u, CURLUPART_HOST, v, 0);
        if (uc) return 2;
    }
    v = curl_pushheader_byname(hp, ":path");
    if (v) {
        uc = curl_url_set(u, CURLUPART_PATH, v, 0);
        if (uc) return 3;
    }
    uc = curl_url_get(u, CURLUPART_URL, &url, 0);
    if (uc) return 4;
    curl_url_cleanup(u);
    if (data->change.url_alloc) free(data->change.url);
    data->change.url_alloc = TRUE;
    data->change.url = url;
    return 0;
}
static int push_promise(struct Curl_easy *data, struct connectdata *conn, const nghttp2_push_promise *frame) {
    int rv;
    H2BUGF(infof(data, "PUSH_PROMISE received, stream %u!\n", frame->promised_stream_id));
    if(data->multi->push_cb) {
        struct HTTP *stream;
        struct HTTP *newstream;
        struct curl_pushheaders heads;
        CURLMcode rc;
        struct http_conn *httpc;
        size_t i;
        struct Curl_easy *newhandle = duphandle(data);
        if(!newhandle) {
            infof(data, "failed to duplicate handle\n");
            rv = 1;
            goto fail;
        }
        heads.data = data;
        heads.frame = frame;
        H2BUGF(infof(data, "Got PUSH_PROMISE, ask application!\n"));
        stream = data->req.protop;
        if(!stream) {
            failf(data, "Internal NULL stream!\n");
            (void)Curl_close(&newhandle);
            rv = 1;
            goto fail;
        }
        rv = set_transfer_url(newhandle, &heads);
        if(rv) goto fail;
        Curl_set_in_callback(data, true);
        rv = data->multi->push_cb(data, newhandle, stream->push_headers_used, &heads, data->multi->push_userp);
        Curl_set_in_callback(data, false);
        for(i = 0; i<stream->push_headers_used; i++) free(stream->push_headers[i]);
        free(stream->push_headers);
        stream->push_headers = NULL;
        stream->push_headers_used = 0;
        if(rv) {
            http2_stream_free(newhandle->req.protop);
            newhandle->req.protop = NULL;
            (void)Curl_close(&newhandle);
            goto fail;
        }
        newstream = newhandle->req.protop;
        newstream->stream_id = frame->promised_stream_id;
        newhandle->req.maxdownload = -1;
        newhandle->req.size = -1;
        rc = Curl_multi_add_perform(data->multi, newhandle, conn);
        if(rc) {
            infof(data, "failed to add handle to multi\n");
            http2_stream_free(newhandle->req.protop);
            newhandle->req.protop = NULL;
            Curl_close(&newhandle);
            rv = 1;
            goto fail;
        }
        httpc = &conn->proto.httpc;
        rv = nghttp2_session_set_stream_user_data(httpc->h2, frame->promised_stream_id, newhandle);
        if(rv) {
            infof(data, "failed to set user_data for stream %d\n", frame->promised_stream_id);
            DEBUGASSERT(0);
            goto fail;
        }
    } else {
        H2BUGF(infof(data, "Got PUSH_PROMISE, ignore it!\n"));
        rv = 1;
    }
    fail:
    return rv;
}
static void multi_connchanged(struct Curl_multi *multi) { multi->recheckstate = TRUE; }
static int on_frame_recv(nghttp2_session *session, const nghttp2_frame *frame, void *userp) {
    struct connectdata *conn = (struct connectdata *)userp;
    struct http_conn *httpc = &conn->proto.httpc;
    struct Curl_easy *data_s = NULL;
    struct HTTP *stream = NULL;
    int rv;
    size_t left, ncopy;
    int32_t stream_id = frame->hd.stream_id;
    CURLcode result;
    if(!stream_id) {
        if(frame->hd.type == NGHTTP2_SETTINGS) {
          uint32_t max_conn = httpc->settings.max_concurrent_streams;
          H2BUGF(infof(conn->data, "Got SETTINGS\n"));
          httpc->settings.max_concurrent_streams = nghttp2_session_get_remote_settings(session, NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS);
          httpc->settings.enable_push = nghttp2_session_get_remote_settings(session, NGHTTP2_SETTINGS_ENABLE_PUSH);
          H2BUGF(infof(conn->data, "MAX_CONCURRENT_STREAMS == %d\n", httpc->settings.max_concurrent_streams));
          H2BUGF(infof(conn->data, "ENABLE_PUSH == %s\n", httpc->settings.enable_push?"TRUE":"false"));
          if(max_conn != httpc->settings.max_concurrent_streams) {
            infof(conn->data, "Connection state changed (MAX_CONCURRENT_STREAMS == %u)!\n", httpc->settings.max_concurrent_streams);
            multi_connchanged(conn->data->multi);
          }
        }
        return 0;
    }
    data_s = nghttp2_session_get_stream_user_data(session, stream_id);
    if(!data_s) {
        H2BUGF(infof(conn->data, "No Curl_easy associated with stream: %x\n", stream_id));
        return 0;
    }
    stream = data_s->req.protop;
    if(!stream) {
        H2BUGF(infof(data_s, "No proto pointer for stream: %x\n", stream_id));
        return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    H2BUGF(infof(data_s, "on_frame_recv() header %x stream %x\n", frame->hd.type, stream_id));
    switch(frame->hd.type) {
        case NGHTTP2_DATA:
            if(!stream->bodystarted) {
                rv = nghttp2_submit_rst_stream(session, NGHTTP2_FLAG_NONE, stream_id, NGHTTP2_PROTOCOL_ERROR);
                if(nghttp2_is_fatal(rv)) return NGHTTP2_ERR_CALLBACK_FAILURE;
            }
            break;
        case NGHTTP2_HEADERS:
            if(stream->bodystarted) break;
            if(stream->status_code == -1) return NGHTTP2_ERR_CALLBACK_FAILURE;
            if(stream->status_code / 100 != 1) {
                stream->bodystarted = TRUE;
                stream->status_code = -1;
            }
            result = Curl_dyn_add(&stream->header_recvbuf, "\r\n");
            if(result) return NGHTTP2_ERR_CALLBACK_FAILURE;
            left = Curl_dyn_len(&stream->header_recvbuf) - stream->nread_header_recvbuf;
            ncopy = CURLMIN(stream->len, left);
            memcpy(&stream->mem[stream->memlen], Curl_dyn_ptr(&stream->header_recvbuf) + stream->nread_header_recvbuf, ncopy);
            stream->nread_header_recvbuf += ncopy;
            H2BUGF(infof(data_s, "Store %zu bytes headers from stream %u at %p\n", ncopy, stream_id, stream->mem));
            stream->len -= ncopy;
            stream->memlen += ncopy;
            drain_this(data_s, httpc);
            {
              struct connectdata *conn_s = (struct connectdata *)userp;
              if(conn_s->data != data_s) Curl_expire(data_s, 0, EXPIRE_RUN_NOW);
            }
            break;
        case NGHTTP2_PUSH_PROMISE:
            rv = push_promise(data_s, conn, &frame->push_promise);
            if(rv) {
              rv = nghttp2_submit_rst_stream(session, NGHTTP2_FLAG_NONE, frame->push_promise.promised_stream_id, NGHTTP2_CANCEL);
              if(nghttp2_is_fatal(rv)) return rv;
            }
            break;
        default: H2BUGF(infof(data_s, "Got frame type %x for stream %u!\n", frame->hd.type, stream_id));
    }
    return 0;
}
static int on_data_chunk_recv(nghttp2_session *session, uint8_t flags, int32_t stream_id, const uint8_t *data, size_t len, void *userp) {
    struct HTTP *stream;
    struct Curl_easy *data_s;
    size_t nread;
    struct connectdata *conn = (struct connectdata *)userp;
    (void)session;
    (void)flags;
    (void)data;
    DEBUGASSERT(stream_id);
    data_s = nghttp2_session_get_stream_user_data(session, stream_id);
    if(!data_s) return NGHTTP2_ERR_CALLBACK_FAILURE;
    stream = data_s->req.protop;
    if(!stream) return NGHTTP2_ERR_CALLBACK_FAILURE;
    nread = CURLMIN(stream->len, len);
    memcpy(&stream->mem[stream->memlen], data, nread);
    stream->len -= nread;
    stream->memlen += nread;
    drain_this(data_s, &conn->proto.httpc);
    if(conn->data != data_s) Curl_expire(data_s, 0, EXPIRE_RUN_NOW);
    H2BUGF(infof(data_s, "%zu data received for stream %u " "(%zu left in buffer %p, total %zu)\n", nread, stream_id, stream->len, stream->mem, stream->memlen));
    if(nread < len) {
        stream->pausedata = data + nread;
        stream->pauselen = len - nread;
        H2BUGF(infof(data_s, "NGHTTP2_ERR_PAUSE - %zu bytes out of buffer" ", stream %u\n", len - nread, stream_id));
        data_s->conn->proto.httpc.pause_stream_id = stream_id;
        return NGHTTP2_ERR_PAUSE;
    }
    if(conn->data != data_s) {
        data_s->conn->proto.httpc.pause_stream_id = stream_id;
        return NGHTTP2_ERR_PAUSE;
    }
    return 0;
}
static int on_stream_close(nghttp2_session *session, int32_t stream_id, uint32_t error_code, void *userp) {
    struct Curl_easy *data_s;
    struct HTTP *stream;
    struct connectdata *conn = (struct connectdata *)userp;
    int rv;
    (void)session;
    (void)stream_id;
    if(stream_id) {
        struct http_conn *httpc;
        data_s = nghttp2_session_get_stream_user_data(session, stream_id);
        if(!data_s) return 0;
        H2BUGF(infof(data_s, "on_stream_close(), %s (err %d), stream %u\n", nghttp2_strerror(error_code), error_code, stream_id));
        stream = data_s->req.protop;
        if(!stream) return NGHTTP2_ERR_CALLBACK_FAILURE;
        stream->closed = TRUE;
        httpc = &conn->proto.httpc;
        drain_this(data_s, httpc);
        Curl_expire(data_s, 0, EXPIRE_RUN_NOW);
        httpc->error_code = error_code;
        rv = nghttp2_session_set_stream_user_data(session, stream_id, 0);
        if(rv) {
            infof(data_s, "http/2: failed to clear user_data for stream %d!\n", stream_id);
            DEBUGASSERT(0);
        }
        if(stream_id == httpc->pause_stream_id) {
            H2BUGF(infof(data_s, "Stopped the pause stream!\n"));
            httpc->pause_stream_id = 0;
        }
        H2BUGF(infof(data_s, "Removed stream %u hash!\n", stream_id));
        stream->stream_id = 0;
    }
    return 0;
}

static int on_begin_headers(nghttp2_session *session, const nghttp2_frame *frame, void *userp) {
    struct HTTP *stream;
    struct Curl_easy *data_s = NULL;
    (void)userp;
    data_s = nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
    if (!data_s) return 0;
    H2BUGF(infof(data_s, "on_begin_headers() was called\n"));
    if (frame->hd.type != NGHTTP2_HEADERS) return 0;
    stream = data_s->req.protop;
    if (!stream || !stream->bodystarted) return 0;
    return 0;
}
static int decode_status_code(const uint8_t *value, size_t len) {
    int i;
    int res;
    if (len != 3) return -1;
    res = 0;
    for (i = 0; i < 3; ++i) {
        char c = value[i];
        if (c < '0' || c > '9') return -1;
        res *= 10;
        res += c - '0';
    }
    return res;
}
static int on_header(nghttp2_session *session, const nghttp2_frame *frame, const uint8_t *name, size_t namelen, const uint8_t *value, size_t valuelen,
                     uint8_t flags, void *userp) {
    struct HTTP *stream;
    struct Curl_easy *data_s;
    int32_t stream_id = frame->hd.stream_id;
    struct connectdata *conn = (struct connectdata *)userp;
    CURLcode result;
    (void)flags;
    DEBUGASSERT(stream_id);
    data_s = nghttp2_session_get_stream_user_data(session, stream_id);
    if (!data_s) return NGHTTP2_ERR_CALLBACK_FAILURE;
    stream = data_s->req.protop;
    if (!stream) {
        failf(data_s, "Internal NULL stream! 5\n");
        return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    if (frame->hd.type == NGHTTP2_PUSH_PROMISE) {
        char *h;
        if (!strcmp(":authority", (const char *)name)) {
            int rc = 0;
            char *check = aprintf("%s:%d", conn->host.name, conn->remote_port);
            if (!check) return NGHTTP2_ERR_CALLBACK_FAILURE;
            if (!Curl_strcasecompare(check, (const char *)value) && ((conn->remote_port != conn->given->defport) ||
                !Curl_strcasecompare(conn->host.name, (const char *)value))) {
                (void)nghttp2_submit_rst_stream(session, NGHTTP2_FLAG_NONE, stream_id, NGHTTP2_PROTOCOL_ERROR);
                rc = NGHTTP2_ERR_CALLBACK_FAILURE;
            }
            free(check);
            if(rc) return rc;
        }
        if(!stream->push_headers) {
            stream->push_headers_alloc = 10;
            stream->push_headers = malloc(stream->push_headers_alloc * sizeof(char *));
            if(!stream->push_headers) return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
            stream->push_headers_used = 0;
        } else if(stream->push_headers_used == stream->push_headers_alloc) {
            char **headp;
            stream->push_headers_alloc *= 2;
            headp = Curl_saferealloc(stream->push_headers, stream->push_headers_alloc * sizeof(char *));
            if(!headp) {
                stream->push_headers = NULL;
                return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
            }
            stream->push_headers = headp;
        }
        h = aprintf("%s:%s", name, value);
        if(h) stream->push_headers[stream->push_headers_used++] = h;
        return 0;
    }
    if (stream->bodystarted) {
        struct dynbuf trail;
        H2BUGF(infof(data_s, "h2 trailer: %.*s: %.*s\n", namelen, name, valuelen, value));
        Curl_dyn_init(&trail, DYN_H2_TRAILER);
        result = Curl_dyn_addf(&trail, "%.*s: %.*s\r\n", namelen, name, valuelen, value);
        if (!result) result = Curl_client_write(conn, CLIENTWRITE_HEADER, Curl_dyn_ptr(&trail), Curl_dyn_len(&trail));
        Curl_dyn_free(&trail);
        if (result) return NGHTTP2_ERR_CALLBACK_FAILURE;
        return 0;
    }
    if (namelen == sizeof(":status") - 1 && memcmp(":status", name, namelen) == 0) {
        stream->status_code = decode_status_code(value, valuelen);
        DEBUGASSERT(stream->status_code != -1);
        result = Curl_dyn_add(&stream->header_recvbuf, "HTTP/2 ");
        if (result) return NGHTTP2_ERR_CALLBACK_FAILURE;
        result = Curl_dyn_addn(&stream->header_recvbuf, value, valuelen);
        if (result) return NGHTTP2_ERR_CALLBACK_FAILURE;
        result = Curl_dyn_add(&stream->header_recvbuf, " \r\n");
        if (result) return NGHTTP2_ERR_CALLBACK_FAILURE;
        if (conn->data != data_s) Curl_expire(data_s, 0, EXPIRE_RUN_NOW);
        H2BUGF(infof(data_s, "h2 status: HTTP/2 %03d (easy %p)\n", stream->status_code, data_s));
        return 0;
    }
    result = Curl_dyn_addn(&stream->header_recvbuf, name, namelen);
    if (result) return NGHTTP2_ERR_CALLBACK_FAILURE;
    result = Curl_dyn_add(&stream->header_recvbuf, ": ");
    if (result) return NGHTTP2_ERR_CALLBACK_FAILURE;
    result = Curl_dyn_addn(&stream->header_recvbuf, value, valuelen);
    if (result) return NGHTTP2_ERR_CALLBACK_FAILURE;
    result = Curl_dyn_add(&stream->header_recvbuf, "\r\n");
    if (result) return NGHTTP2_ERR_CALLBACK_FAILURE;
    if (conn->data != data_s) Curl_expire(data_s, 0, EXPIRE_RUN_NOW);
    H2BUGF(infof(data_s, "h2 header: %.*s: %.*s\n", namelen, name, valuelen, value));
    return 0;
}
static ssize_t data_source_read_callback(nghttp2_session *session, int32_t stream_id, uint8_t *buf, size_t length, uint32_t *data_flags,
                                         nghttp2_data_source *source, void *userp) {
    struct Curl_easy *data_s;
    struct HTTP *stream = NULL;
    size_t nread;
    (void)source;
    (void)userp;
    if (stream_id) {
        data_s = nghttp2_session_get_stream_user_data(session, stream_id);
        if(!data_s) return NGHTTP2_ERR_CALLBACK_FAILURE;
        stream = data_s->req.protop;
        if (!stream) return NGHTTP2_ERR_CALLBACK_FAILURE;
    } else return NGHTTP2_ERR_INVALID_ARGUMENT;
    nread = CURLMIN(stream->upload_len, length);
    if (nread > 0) {
        memcpy(buf, stream->upload_mem, nread);
        stream->upload_mem += nread;
        stream->upload_len -= nread;
        if (data_s->state.infilesize != -1) stream->upload_left -= nread;
    }
    if(stream->upload_left == 0) *data_flags = NGHTTP2_DATA_FLAG_EOF;
    else if(nread == 0)
    return NGHTTP2_ERR_DEFERRED;
    H2BUGF(infof(data_s, "data_source_read_callback: " "returns %zu bytes stream %u\n", nread, stream_id));
    return nread;
}
#if !defined(CURL_DISABLE_VERBOSE_STRINGS)
static int error_callback(nghttp2_session *session, const char *msg, size_t len, void *userp) {
    struct connectdata *conn = (struct connectdata *)userp;
    (void)session;
    infof(conn->data, "http2 error: %.*s\n", len, msg);
    return 0;
}
#endif
static void populate_settings(struct connectdata *conn, struct http_conn *httpc) {
    nghttp2_settings_entry *iv = httpc->local_settings;
    DEBUGASSERT(conn->data);
    iv[0].settings_id = NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS;
    iv[0].value = Curl_multi_max_concurrent_streams(conn->data->multi);
    iv[1].settings_id = NGHTTP2_SETTINGS_INITIAL_WINDOW_SIZE;
    iv[1].value = HTTP2_HUGE_WINDOW_SIZE;
    iv[2].settings_id = NGHTTP2_SETTINGS_ENABLE_PUSH;
    iv[2].value = conn->data->multi->push_cb != NULL;
    httpc->local_settings_num = 3;
}
void Curl_http2_done(struct Curl_easy *data, bool premature) {
    struct HTTP *http = data->req.protop;
    struct http_conn *httpc = &data->conn->proto.httpc;
    Curl_dyn_free(&http->header_recvbuf);
    if(http->push_headers) {
        for(; http->push_headers_used > 0; --http->push_headers_used) {
            free(http->push_headers[http->push_headers_used - 1]);
        }
        free(http->push_headers);
        http->push_headers = NULL;
    }
    if(!httpc->h2) return;
    if(premature) {
        if(!nghttp2_submit_rst_stream(httpc->h2, NGHTTP2_FLAG_NONE, http->stream_id, NGHTTP2_STREAM_CLOSED))
            (void)nghttp2_session_send(httpc->h2);
        if(http->stream_id == httpc->pause_stream_id) {
            infof(data, "stopped the pause stream!\n");
            httpc->pause_stream_id = 0;
        }
    }
    if(data->state.drain) drained_transfer(data, httpc);
    if(http->stream_id > 0) {
        int rv = nghttp2_session_set_stream_user_data(httpc->h2, http->stream_id, 0);
        if(rv) {
            infof(data, "http/2: failed to clear user_data for stream %d!\n", http->stream_id);
            DEBUGASSERT(0);
        }
        http->stream_id = 0;
    }
}
static CURLcode http2_init(struct connectdata *conn) {
    if (!conn->proto.httpc.h2) {
        int rc;
        nghttp2_session_callbacks *callbacks;
        conn->proto.httpc.inbuf = malloc(H2_BUFSIZE);
        if (conn->proto.httpc.inbuf == NULL) return CURLE_OUT_OF_MEMORY;
        rc = nghttp2_session_callbacks_new(&callbacks);
        if(rc) {
            failf(conn->data, "Couldn't initialize nghttp2 callbacks!");
            return CURLE_OUT_OF_MEMORY; /* most likely at least */
        }
        nghttp2_session_callbacks_set_send_callback(callbacks, send_callback);
        nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks, on_frame_recv);
        nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, on_data_chunk_recv);
        nghttp2_session_callbacks_set_on_stream_close_callback(callbacks, on_stream_close);
        nghttp2_session_callbacks_set_on_begin_headers_callback(callbacks, on_begin_headers);
        nghttp2_session_callbacks_set_on_header_callback(callbacks, on_header);
        nghttp2_session_callbacks_set_error_callback(callbacks, error_callback);
        rc = nghttp2_session_client_new(&conn->proto.httpc.h2, callbacks, conn);
        nghttp2_session_callbacks_del(callbacks);
        if(rc) {
            failf(conn->data, "Couldn't initialize nghttp2!");
            return CURLE_OUT_OF_MEMORY; /* most likely at least */
        }
    }
    return CURLE_OK;
}
CURLcode Curl_http2_request_upgrade(struct dynbuf *req, struct connectdata *conn) {
    CURLcode result;
    ssize_t binlen;
    char *base64;
    size_t blen;
    struct SingleRequest *k = &conn->data->req;
    uint8_t *binsettings = conn->proto.httpc.binsettings;
    struct http_conn *httpc = &conn->proto.httpc;
    populate_settings(conn, httpc);
    binlen = nghttp2_pack_settings_payload(binsettings, H2_BINSETTINGS_LEN, httpc->local_settings, httpc->local_settings_num);
    if(!binlen) {
        failf(conn->data, "nghttp2 unexpectedly failed on pack_settings_payload");
        Curl_dyn_free(req);
        return CURLE_FAILED_INIT;
    }
    conn->proto.httpc.binlen = binlen;
    result = Curl_base64url_encode(conn->data, (const char *)binsettings, binlen, &base64, &blen);
    if(result) {
        Curl_dyn_free(req);
        return result;
    }
    result = Curl_dyn_addf(req, "Connection: Upgrade, HTTP2-Settings\r\n" "Upgrade: %s\r\n" "HTTP2-Settings: %s\r\n", NGHTTP2_CLEARTEXT_PROTO_VERSION_ID, base64);
    free(base64);
    k->upgr101 = UPGR101_REQUESTED;
    return result;
}
static int should_close_session(struct http_conn *httpc) {
    return httpc->drain_total == 0 && !nghttp2_session_want_read(httpc->h2) && !nghttp2_session_want_write(httpc->h2);
}
static int h2_process_pending_input(struct connectdata *conn, struct http_conn *httpc, CURLcode *err) {
    ssize_t nread;
    char *inbuf;
    ssize_t rv;
    struct Curl_easy *data = conn->data;
    nread = httpc->inbuflen - httpc->nread_inbuf;
    inbuf = httpc->inbuf + httpc->nread_inbuf;
    rv = nghttp2_session_mem_recv(httpc->h2, (const uint8_t *)inbuf, nread);
    if(rv < 0) {
        failf(data, "h2_process_pending_input: nghttp2_session_mem_recv() returned " "%zd:%s\n", rv, nghttp2_strerror((int)rv));
        *err = CURLE_RECV_ERROR;
        return -1;
    }
    if(nread == rv) {
        H2BUGF(infof(data, "h2_process_pending_input: All data in connection buffer " "processed\n"));
        httpc->inbuflen = 0;
        httpc->nread_inbuf = 0;
    } else {
        httpc->nread_inbuf += rv;
        H2BUGF(infof(data, "h2_process_pending_input: %zu bytes left in connection " "buffer\n", httpc->inbuflen - httpc->nread_inbuf));
    }
    rv = h2_session_send(data, httpc->h2);
    if(rv != 0) {
        *err = CURLE_SEND_ERROR;
        return -1;
    }
    if(should_close_session(httpc)) {
        H2BUGF(infof(data, "h2_process_pending_input: nothing to do in this session\n"));
        if(httpc->error_code) *err = CURLE_HTTP2;
        else {
            connclose(conn, "GOAWAY received");
            *err = CURLE_OK;
        }
        return -1;
    }
    return 0;
}
CURLcode Curl_http2_done_sending(struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    if ((conn->handler == &Curl_handler_http2_ssl) || (conn->handler == &Curl_handler_http2)) {
        struct HTTP *stream = conn->data->req.protop;
        struct http_conn *httpc = &conn->proto.httpc;
        nghttp2_session *h2 = httpc->h2;
        if (stream->upload_left) {
            stream->upload_left = 0;
            (void)nghttp2_session_resume_data(h2, stream->stream_id);
            (void)h2_process_pending_input(conn, httpc, &result);
        }
        if (nghttp2_session_want_write(h2)) {
            struct Curl_easy *data = conn->data;
            struct SingleRequest *k = &data->req;
            int rv;
            H2BUGF(infof(data, "HTTP/2 still wants to send data (easy %p)\n", data));
            k->keepon |= KEEP_SEND;
            rv = h2_session_send(data, h2);
            if (rv != 0) result = CURLE_SEND_ERROR;
        }
    }
    return result;
}
static ssize_t http2_handle_stream_close(struct connectdata *conn, struct Curl_easy *data, struct HTTP *stream, CURLcode *err) {
    struct http_conn *httpc = &conn->proto.httpc;
    if(httpc->pause_stream_id == stream->stream_id) httpc->pause_stream_id = 0;
    drained_transfer(data, httpc);
    if(httpc->pause_stream_id == 0) if(h2_process_pending_input(conn, httpc, err) != 0) return -1;
    DEBUGASSERT(data->state.drain == 0);
    stream->closed = FALSE;
    if(httpc->error_code == NGHTTP2_REFUSED_STREAM) {
        H2BUGF(infof(data, "REFUSED_STREAM (%d), try again on a new connection!\n", stream->stream_id));
        connclose(conn, "REFUSED_STREAM");
        data->state.refused_stream = TRUE;
        *err = CURLE_RECV_ERROR;
        return -1;
    } else if(httpc->error_code != NGHTTP2_NO_ERROR) {
        failf(data, "HTTP/2 stream %d was not closed cleanly: %s (err %u)", stream->stream_id, nghttp2_strerror(httpc->error_code), httpc->error_code);
        *err = CURLE_HTTP2_STREAM;
        return -1;
    }
    if(!stream->bodystarted) {
        failf(data, "HTTP/2 stream %d was closed cleanly, but before getting " " all response header fields, treated as error", stream->stream_id);
        *err = CURLE_HTTP2_STREAM;
        return -1;
    }
    stream->close_handled = TRUE;
    H2BUGF(infof(data, "http2_recv returns 0, http2_handle_stream_close\n"));
    return 0;
}
static void h2_pri_spec(struct Curl_easy *data, nghttp2_priority_spec *pri_spec) {
    struct HTTP *depstream = (data->set.stream_depends_on ? data->set.stream_depends_on->req.protop : NULL);
    int32_t depstream_id = depstream? depstream->stream_id:0;
    nghttp2_priority_spec_init(pri_spec, depstream_id, data->set.stream_weight, data->set.stream_depends_e);
    data->state.stream_weight = data->set.stream_weight;
    data->state.stream_depends_e = data->set.stream_depends_e;
    data->state.stream_depends_on = data->set.stream_depends_on;
}
static int h2_session_send(struct Curl_easy *data, nghttp2_session *h2) {
    struct HTTP *stream = data->req.protop;
    if ((data->set.stream_weight != data->state.stream_weight) || (data->set.stream_depends_e != data->state.stream_depends_e) ||
       (data->set.stream_depends_on != data->state.stream_depends_on) ) {
        nghttp2_priority_spec pri_spec;
        int rv;
        h2_pri_spec(data, &pri_spec);
        H2BUGF(infof(data, "Queuing PRIORITY on stream %u (easy %p)\n", stream->stream_id, data));
        DEBUGASSERT(stream->stream_id != -1);
        rv = nghttp2_submit_priority(h2, NGHTTP2_FLAG_NONE, stream->stream_id, &pri_spec);
        if (rv) return rv;
    }
    return nghttp2_session_send(h2);
}
static ssize_t http2_recv(struct connectdata *conn, int sockindex, char *mem, size_t len, CURLcode *err) {
    CURLcode result = CURLE_OK;
    ssize_t rv;
    ssize_t nread;
    struct http_conn *httpc = &conn->proto.httpc;
    struct Curl_easy *data = conn->data;
    struct HTTP *stream = data->req.protop;
    (void)sockindex;
    if(should_close_session(httpc)) {
        H2BUGF(infof(data, "http2_recv: nothing to do in this session\n"));
        if(conn->bits.close) {
            *err = CURLE_OK;
            return 0;
        }
        *err = CURLE_HTTP2;
        return -1;
    }
    stream->upload_mem = NULL;
    stream->upload_len = 0;
    if(stream->bodystarted &&
    stream->nread_header_recvbuf < Curl_dyn_len(&stream->header_recvbuf)) {
        size_t left = Curl_dyn_len(&stream->header_recvbuf) - stream->nread_header_recvbuf;
        size_t ncopy = CURLMIN(len, left);
        memcpy(mem, Curl_dyn_ptr(&stream->header_recvbuf) + stream->nread_header_recvbuf, ncopy);
        stream->nread_header_recvbuf += ncopy;
        H2BUGF(infof(data, "http2_recv: Got %d bytes from header_recvbuf\n", (int)ncopy));
        return ncopy;
    }
    H2BUGF(infof(data, "http2_recv: easy %p (stream %u) win %u/%u\n", data, stream->stream_id, nghttp2_session_get_local_window_size(httpc->h2),
                 nghttp2_session_get_stream_local_window_size(httpc->h2, stream->stream_id)));
    if ((data->state.drain) && stream->memlen) {
        H2BUGF(infof(data, "http2_recv: DRAIN %zu bytes stream %u!! (%p => %p)\n", stream->memlen, stream->stream_id, stream->mem, mem));
        if (mem != stream->mem) {
            memmove(mem, stream->mem, stream->memlen);
            stream->len = len - stream->memlen;
            stream->mem = mem;
        }
        if (httpc->pause_stream_id == stream->stream_id && !stream->pausedata) {
            httpc->pause_stream_id = 0;
            if (h2_process_pending_input(conn, httpc, &result) != 0) {
                *err = result;
                return -1;
            }
        }
    } else if (stream->pausedata) {
        DEBUGASSERT(httpc->pause_stream_id == stream->stream_id);
        nread = CURLMIN(len, stream->pauselen);
        memcpy(mem, stream->pausedata, nread);
        stream->pausedata += nread;
        stream->pauselen -= nread;
        if (stream->pauselen == 0) {
            H2BUGF(infof(data, "Unpaused by stream %u\n", stream->stream_id));
            DEBUGASSERT(httpc->pause_stream_id == stream->stream_id);
            httpc->pause_stream_id = 0;
            stream->pausedata = NULL;
            stream->pauselen = 0;
            if (h2_process_pending_input(conn, httpc, &result) != 0) {
                *err = result;
                return -1;
            }
        }
        H2BUGF(infof(data, "http2_recv: returns unpaused %zd bytes on stream %u\n", nread, stream->stream_id));
        return nread;
    } else if (httpc->pause_stream_id) {
        if (stream->closed) return 0;
        H2BUGF(infof(data, "stream %x is paused, pause id: %x\n", stream->stream_id, httpc->pause_stream_id));
        *err = CURLE_AGAIN;
        return -1;
    } else {
        char *inbuf;
        stream->mem = mem;
        stream->len = len;
        stream->memlen = 0;
        if (httpc->inbuflen == 0) {
            nread = ((Curl_recv *)httpc->recv_underlying)(conn, FIRSTSOCKET, httpc->inbuf, H2_BUFSIZE, &result);
            if (nread == -1) {
                if (result != CURLE_AGAIN) failf(data, "Failed receiving HTTP2 data");
                else if (stream->closed) return http2_handle_stream_close(conn, data, stream, err);
                *err = result;
                return -1;
            }
            if (nread == 0) {
                H2BUGF(infof(data, "end of stream\n"));
                *err = CURLE_OK;
                return 0;
            }
            H2BUGF(infof(data, "nread=%zd\n", nread));
            httpc->inbuflen = nread;
            inbuf = httpc->inbuf;
        } else {
            nread = httpc->inbuflen - httpc->nread_inbuf;
            inbuf = httpc->inbuf + httpc->nread_inbuf;
            H2BUGF(infof(data, "Use data left in connection buffer, nread=%zd\n", nread));
        }
        rv = nghttp2_session_mem_recv(httpc->h2, (const uint8_t *)inbuf, nread);
        if (nghttp2_is_fatal((int)rv)) {
            failf(data, "nghttp2_session_mem_recv() returned %zd:%s\n", rv, nghttp2_strerror((int)rv));
            *err = CURLE_RECV_ERROR;
            return -1;
        }
        H2BUGF(infof(data, "nghttp2_session_mem_recv() returns %zd\n", rv));
        if (nread == rv) {
            H2BUGF(infof(data, "All data in connection buffer processed\n"));
            httpc->inbuflen = 0;
            httpc->nread_inbuf = 0;
        } else {
          httpc->nread_inbuf += rv;
          H2BUGF(infof(data, "%zu bytes left in connection buffer\n", httpc->inbuflen - httpc->nread_inbuf));
        }
        rv = h2_session_send(data, httpc->h2);
        if (rv != 0) {
            *err = CURLE_SEND_ERROR;
            return -1;
        }
        if (should_close_session(httpc)) {
            H2BUGF(infof(data, "http2_recv: nothing to do in this session\n"));
            *err = CURLE_HTTP2;
            return -1;
        }
    }
    if (stream->memlen) {
        ssize_t retlen = stream->memlen;
        H2BUGF(infof(data, "http2_recv: returns %zd for stream %u\n", retlen, stream->stream_id));
        stream->memlen = 0;
        if (httpc->pause_stream_id == stream->stream_id) H2BUGF(infof(data, "Data returned for PAUSED stream %u\n", stream->stream_id));
        else if (!stream->closed) drained_transfer(data, httpc);
        else Curl_expire(data, 0, EXPIRE_RUN_NOW);
        return retlen;
    }
    if (stream->closed) return 0;
    *err = CURLE_AGAIN;
    H2BUGF(infof(data, "http2_recv returns AGAIN for stream %u\n", stream->stream_id));
    return -1;
}
#define AUTHORITY_DST_IDX 3
#define HEADER_OVERFLOW(x) (x.namelen > 0xffff || x.valuelen > 0xffff - x.namelen)
static bool contains_trailers(const char *p, size_t len) {
    const char *end = p + len;
    for ( ; ; ) {
        for ( ; p != end && (*p == ' ' || *p == '\t'); ++p);
        if (p == end || (size_t)(end - p) < sizeof("trailers") - 1) return FALSE;
        if (strncasecompare("trailers", p, sizeof("trailers") - 1)) {
            p += sizeof("trailers") - 1;
            for(; p != end && (*p == ' ' || *p == '\t'); ++p);
            if(p == end || *p == ',') return TRUE;
        }
        for (; p != end && *p != ','; ++p);
        if (p == end)return FALSE;
        ++p;
    }
}
typedef enum {
    HEADERINST_FORWARD,
    HEADERINST_IGNORE,
    HEADERINST_TE_TRAILERS
} header_instruction;
static header_instruction inspect_header(const char *name, size_t namelen, const char *value, size_t valuelen) {
    switch(namelen) {
        case 2:
            if(!strncasecompare("te", name, namelen)) return HEADERINST_FORWARD;
            return contains_trailers(value, valuelen) ? HEADERINST_TE_TRAILERS : HEADERINST_IGNORE;
        case 7: return strncasecompare("upgrade", name, namelen) ? HEADERINST_IGNORE : HEADERINST_FORWARD;
        case 10: return (strncasecompare("connection", name, namelen) || strncasecompare("keep-alive", name, namelen)) ? HEADERINST_IGNORE : HEADERINST_FORWARD;
        case 16: return strncasecompare("proxy-connection", name, namelen) ? HEADERINST_IGNORE : HEADERINST_FORWARD;
        case 17: return strncasecompare("transfer-encoding", name, namelen) ? HEADERINST_IGNORE : HEADERINST_FORWARD;
        default: return HEADERINST_FORWARD;
    }
}
static ssize_t http2_send(struct connectdata *conn, int sockindex, const void *mem, size_t len, CURLcode *err) {
    int rv;
    struct http_conn *httpc = &conn->proto.httpc;
    struct HTTP *stream = conn->data->req.protop;
    nghttp2_nv *nva = NULL;
    size_t nheader;
    size_t i;
    size_t authority_idx;
    char *hdbuf = (char *)mem;
    char *end, *line_end;
    nghttp2_data_provider data_prd;
    int32_t stream_id;
    nghttp2_session *h2 = httpc->h2;
    nghttp2_priority_spec pri_spec;
    (void)sockindex;
    H2BUGF(infof(conn->data, "http2_send len=%zu\n", len));
    if (stream->stream_id != -1) {
        if (stream->close_handled) {
            infof(conn->data, "stream %d closed\n", stream->stream_id);
            *err = CURLE_HTTP2_STREAM;
            return -1;
        } else if (stream->closed) return http2_handle_stream_close(conn, conn->data, stream, err);
        stream->upload_mem = mem;
        stream->upload_len = len;
        rv = nghttp2_session_resume_data(h2, stream->stream_id);
        if (nghttp2_is_fatal(rv)) {
            *err = CURLE_SEND_ERROR;
            return -1;
        }
        rv = h2_session_send(conn->data, h2);
        if (nghttp2_is_fatal(rv)) {
            *err = CURLE_SEND_ERROR;
            return -1;
        }
        len -= stream->upload_len;
        stream->upload_mem = NULL;
        stream->upload_len = 0;
        if (should_close_session(httpc)) {
            H2BUGF(infof(conn->data, "http2_send: nothing to do in this session\n"));
            *err = CURLE_HTTP2;
            return -1;
        }
        if (stream->upload_left) nghttp2_session_resume_data(h2, stream->stream_id);
        H2BUGF(infof(conn->data, "http2_send returns %zu for stream %u\n", len, stream->stream_id));
        return len;
    }
    nheader = 0;
    for (i = 1; i < len; ++i) {
        if (hdbuf[i] == '\n' && hdbuf[i - 1] == '\r') {
            ++nheader;
            ++i;
        }
    }
    if (nheader < 2) goto fail;
    nheader += 1;
    nva = malloc(sizeof(nghttp2_nv) * nheader);
    if (nva == NULL) {
        *err = CURLE_OUT_OF_MEMORY;
        return -1;
    }
    line_end = memchr(hdbuf, '\r', len);
    if (!line_end) goto fail;
    end = memchr(hdbuf, ' ', line_end - hdbuf);
    if (!end || end == hdbuf) goto fail;
    nva[0].name = (unsigned char *)":method";
    nva[0].namelen = strlen((char *)nva[0].name);
    nva[0].value = (unsigned char *)hdbuf;
    nva[0].valuelen = (size_t)(end - hdbuf);
    nva[0].flags = NGHTTP2_NV_FLAG_NONE;
    if (HEADER_OVERFLOW(nva[0])) {
        failf(conn->data, "Failed sending HTTP request: Header overflow");
        goto fail;
    }
    hdbuf = end + 1;
    end = NULL;
    for (i = (size_t)(line_end - hdbuf); i; --i) {
        if (hdbuf[i - 1] == ' ') {
            end = &hdbuf[i - 1];
            break;
        }
    }
    if (!end || end == hdbuf) goto fail;
    nva[1].name = (unsigned char *)":path";
    nva[1].namelen = strlen((char *)nva[1].name);
    nva[1].value = (unsigned char *)hdbuf;
    nva[1].valuelen = (size_t)(end - hdbuf);
    nva[1].flags = NGHTTP2_NV_FLAG_NONE;
    if (HEADER_OVERFLOW(nva[1])) {
        failf(conn->data, "Failed sending HTTP request: Header overflow");
        goto fail;
    }
    nva[2].name = (unsigned char *)":scheme";
    nva[2].namelen = strlen((char *)nva[2].name);
    if (conn->handler->flags & PROTOPT_SSL) nva[2].value = (unsigned char *)"https";
    else nva[2].value = (unsigned char *)"http";
    nva[2].valuelen = strlen((char *)nva[2].value);
    nva[2].flags = NGHTTP2_NV_FLAG_NONE;
    if (HEADER_OVERFLOW(nva[2])) {
        failf(conn->data, "Failed sending HTTP request: Header overflow");
        goto fail;
    }
    authority_idx = 0;
    i = 3;
    while(i < nheader) {
        size_t hlen;
        hdbuf = line_end + 2;
        line_end = memchr(hdbuf, '\r', len - (hdbuf - (char *)mem));
        if (!line_end || (line_end == hdbuf)) goto fail;
        if (*hdbuf == ' ' || *hdbuf == '\t') goto fail;
        for(end = hdbuf; end < line_end && *end != ':'; ++end) ;
        if (end == hdbuf || end == line_end) goto fail;
        hlen = end - hdbuf;
        if(hlen == 4 && strncasecompare("host", hdbuf, 4)) {
            authority_idx = i;
            nva[i].name = (unsigned char *)":authority";
            nva[i].namelen = strlen((char *)nva[i].name);
        } else {
            nva[i].namelen = (size_t)(end - hdbuf);
            Curl_strntolower((char *)hdbuf, hdbuf, nva[i].namelen);
            nva[i].name = (unsigned char *)hdbuf;
        }
        hdbuf = end + 1;
        while(*hdbuf == ' ' || *hdbuf == '\t') ++hdbuf;
        end = line_end;
        switch(inspect_header((const char *)nva[i].name, nva[i].namelen, hdbuf, end - hdbuf)) {
            case HEADERINST_IGNORE: --nheader; continue;
            case HEADERINST_TE_TRAILERS:
                nva[i].value = (uint8_t*)"trailers";
                nva[i].valuelen = sizeof("trailers") - 1;
                break;
            default:
                nva[i].value = (unsigned char *)hdbuf;
                nva[i].valuelen = (size_t)(end - hdbuf);
        }
        nva[i].flags = NGHTTP2_NV_FLAG_NONE;
        if(HEADER_OVERFLOW(nva[i])) {
            failf(conn->data, "Failed sending HTTP request: Header overflow");
            goto fail;
        }
        ++i;
    }
    if (authority_idx != 0 && authority_idx != AUTHORITY_DST_IDX) {
        nghttp2_nv authority = nva[authority_idx];
        for (i = authority_idx; i > AUTHORITY_DST_IDX; --i) nva[i] = nva[i - 1];
        nva[i] = authority;
    }
    #define MAX_ACC 60000
    size_t acc = 0;
    for (i = 0; i < nheader; ++i) {
      acc += nva[i].namelen + nva[i].valuelen;
      H2BUGF(infof(conn->data, "h2 header: %.*s:%.*s\n", nva[i].namelen, nva[i].name, nva[i].valuelen, nva[i].value));
    }
    if (acc > MAX_ACC) {
      infof(conn->data, "http2_send: Warning: The cumulative length of all " "headers exceeds %zu bytes and that could cause the "
            "stream to be rejected.\n", MAX_ACC);
    }
    h2_pri_spec(conn->data, &pri_spec);
    switch(conn->data->state.httpreq) {
        case HTTPREQ_POST: case HTTPREQ_POST_FORM: case HTTPREQ_POST_MIME: case HTTPREQ_PUT:
            if (conn->data->state.infilesize != -1) stream->upload_left = conn->data->state.infilesize;
            else stream->upload_left = -1; /* unknown, but not zero */
            data_prd.read_callback = data_source_read_callback;
            data_prd.source.ptr = NULL;
            stream_id = nghttp2_submit_request(h2, &pri_spec, nva, nheader, &data_prd, conn->data);
            break;
        default: stream_id = nghttp2_submit_request(h2, &pri_spec, nva, nheader, NULL, conn->data);
    }
    Curl_safefree(nva);
    if (stream_id < 0) {
        H2BUGF(infof(conn->data, "http2_send() send error\n"));
        *err = CURLE_SEND_ERROR;
        return -1;
    }
    infof(conn->data, "Using Stream ID: %x (easy handle %p)\n", stream_id, (void *)conn->data);
    stream->stream_id = stream_id;
    rv = nghttp2_session_send(h2);
    if (rv != 0) {
        *err = CURLE_SEND_ERROR;
        return -1;
    }
    if (should_close_session(httpc)) {
        H2BUGF(infof(conn->data, "http2_send: nothing to do in this session\n"));
        *err = CURLE_HTTP2;
        return -1;
    }
    nghttp2_session_resume_data(h2, stream->stream_id);
    return len;
    fail:
    free(nva);
    *err = CURLE_SEND_ERROR;
    return -1;
}
CURLcode Curl_http2_setup(struct connectdata *conn) {
    CURLcode result;
    struct http_conn *httpc = &conn->proto.httpc;
    struct HTTP *stream = conn->data->req.protop;
    DEBUGASSERT(conn->data->state.buffer);
    stream->stream_id = -1;
    Curl_dyn_init(&stream->header_recvbuf, DYN_H2_HEADERS);
    if ((conn->handler == &Curl_handler_http2_ssl) || (conn->handler == &Curl_handler_http2)) return CURLE_OK;
    if (conn->handler->flags & PROTOPT_SSL) conn->handler = &Curl_handler_http2_ssl;
    else conn->handler = &Curl_handler_http2;
    result = http2_init(conn);
    if (result) {
        Curl_dyn_free(&stream->header_recvbuf);
        return result;
    }
    infof(conn->data, "Using HTTP2, server supports multi-use\n");
    stream->upload_left = 0;
    stream->upload_mem = NULL;
    stream->upload_len = 0;
    stream->mem = conn->data->state.buffer;
    stream->len = conn->data->set.buffer_size;
    httpc->inbuflen = 0;
    httpc->nread_inbuf = 0;
    httpc->pause_stream_id = 0;
    httpc->drain_total = 0;
    conn->bits.multiplex = TRUE;
    conn->httpversion = 20;
    conn->bundle->multiuse = BUNDLE_MULTIPLEX;
    infof(conn->data, "Connection state changed (HTTP/2 confirmed)\n");
    multi_connchanged(conn->data->multi);
    return CURLE_OK;
}
CURLcode Curl_http2_switched(struct connectdata *conn, const char *mem, size_t nread) {
    CURLcode result;
    struct http_conn *httpc = &conn->proto.httpc;
    int rv;
    ssize_t nproc;
    struct Curl_easy *data = conn->data;
    struct HTTP *stream = conn->data->req.protop;
    result = Curl_http2_setup(conn);
    if (result) return result;
    httpc->recv_underlying = conn->recv[FIRSTSOCKET];
    httpc->send_underlying = conn->send[FIRSTSOCKET];
    conn->recv[FIRSTSOCKET] = http2_recv;
    conn->send[FIRSTSOCKET] = http2_send;
    if (conn->data->req.upgr101 == UPGR101_RECEIVED) {
        stream->stream_id = 1;
        rv = nghttp2_session_upgrade(httpc->h2, httpc->binsettings, httpc->binlen, NULL);
        if (rv != 0) {
            failf(data, "nghttp2_session_upgrade() failed: %s(%d)", nghttp2_strerror(rv), rv);
            return CURLE_HTTP2;
        }
        rv = nghttp2_session_set_stream_user_data(httpc->h2, stream->stream_id, data);
        if (rv) {
            infof(data, "http/2: failed to set user_data for stream %d!\n", stream->stream_id);
            DEBUGASSERT(0);
        }
    } else {
        populate_settings(conn, httpc);
        stream->stream_id = -1;
        rv = nghttp2_submit_settings(httpc->h2, NGHTTP2_FLAG_NONE, httpc->local_settings, httpc->local_settings_num);
        if (rv != 0) {
            failf(data, "nghttp2_submit_settings() failed: %s(%d)", nghttp2_strerror(rv), rv);
            return CURLE_HTTP2;
        }
    }
    rv = nghttp2_session_set_local_window_size(httpc->h2, NGHTTP2_FLAG_NONE, 0, HTTP2_HUGE_WINDOW_SIZE);
    if (rv != 0) {
        failf(data, "nghttp2_session_set_local_window_size() failed: %s(%d)", nghttp2_strerror(rv), rv);
        return CURLE_HTTP2;
    }
    if (H2_BUFSIZE < nread) {
        failf(data, "connection buffer size is too small to store data following " "HTTP Upgrade response header: buflen=%zu, datalen=%zu", H2_BUFSIZE, nread);
        return CURLE_HTTP2;
    }
    infof(conn->data, "Copying HTTP/2 data in stream buffer to connection buffer" " after upgrade: len=%zu\n", nread);
    i f(nread) memcpy(httpc->inbuf, mem, nread);
    httpc->inbuflen = nread;
    nproc = nghttp2_session_mem_recv(httpc->h2, (const uint8_t *)httpc->inbuf, httpc->inbuflen);
    if (nghttp2_is_fatal((int)nproc)) {
        failf(data, "nghttp2_session_mem_recv() failed: %s(%d)", nghttp2_strerror((int)nproc), (int)nproc);
        return CURLE_HTTP2;
    }
    H2BUGF(infof(data, "nghttp2_session_mem_recv() returns %zd\n", nproc));
    if ((ssize_t)nread == nproc) {
        httpc->inbuflen = 0;
        httpc->nread_inbuf = 0;
    } else httpc->nread_inbuf += nproc;
    rv = h2_session_send(data, httpc->h2);
    if (rv != 0) {
        failf(data, "nghttp2_session_send() failed: %s(%d)", nghttp2_strerror(rv), rv);
        return CURLE_HTTP2;
    }
    if(should_close_session(httpc)) {
        H2BUGF(infof(data, "nghttp2_session_send(): nothing to do in this session\n"));
        return CURLE_HTTP2;
    }
    return CURLE_OK;
}
CURLcode Curl_http2_stream_pause(struct Curl_easy *data, bool pause) {
    DEBUGASSERT(data);
    DEBUGASSERT(data->conn);
    if (!data->conn->proto.httpc.h2) return CURLE_OK;
#ifdef NGHTTP2_HAS_SET_LOCAL_WINDOW_SIZE
    else {
        struct HTTP *stream = data->req.protop;
        struct http_conn *httpc = &data->conn->proto.httpc;
        uint32_t window = !pause * HTTP2_HUGE_WINDOW_SIZE;
        int rv = nghttp2_session_set_local_window_size(httpc->h2, NGHTTP2_FLAG_NONE, stream->stream_id, window);
        if (rv) {
            failf(data, "nghttp2_session_set_local_window_size() failed: %s(%d)", nghttp2_strerror(rv), rv);
            return CURLE_HTTP2;
        }
        rv = h2_session_send(data, httpc->h2);
        if (rv) return CURLE_SEND_ERROR;
        DEBUGF(infof(data, "Set HTTP/2 window size to %u for stream %u\n", window, stream->stream_id));
#ifdef DEBUGBUILD
        uint32_t window2 = nghttp2_session_get_stream_local_window_size(httpc->h2, stream->stream_id);
        DEBUGF(infof(data, "HTTP/2 window size is now %u for stream %u\n", window2, stream->stream_id));
#endif
    }
#endif
    return CURLE_OK;
}
CURLcode Curl_http2_add_child(struct Curl_easy *parent, struct Curl_easy *child, bool exclusive) {
    if(parent) {
        struct Curl_http2_dep **tail;
        struct Curl_http2_dep *dep = calloc(1, sizeof(struct Curl_http2_dep));
        if(!dep) return CURLE_OUT_OF_MEMORY;
        dep->data = child;
        if(parent->set.stream_dependents && exclusive) {
            struct Curl_http2_dep *node = parent->set.stream_dependents;
            while(node) {
                node->data->set.stream_depends_on = child;
                node = node->next;
            }
            tail = &child->set.stream_dependents;
            while(*tail) tail = &(*tail)->next;
            DEBUGASSERT(!*tail);
            *tail = parent->set.stream_dependents;
            parent->set.stream_dependents = 0;
        }
        tail = &parent->set.stream_dependents;
        while(*tail) {
            (*tail)->data->set.stream_depends_e = FALSE;
            tail = &(*tail)->next;
        }
        DEBUGASSERT(!*tail);
        *tail = dep;
    }
    child->set.stream_depends_on = parent;
    child->set.stream_depends_e = exclusive;
    return CURLE_OK;
}
void Curl_http2_remove_child(struct Curl_easy *parent, struct Curl_easy *child) {
    struct Curl_http2_dep *last = 0;
    struct Curl_http2_dep *data = parent->set.stream_dependents;
    DEBUGASSERT(child->set.stream_depends_on == parent);
    while(data && data->data != child) {
        last = data;
        data = data->next;
    }
    DEBUGASSERT(data);
    if (data) {
        if (last) last->next = data->next;
        else parent->set.stream_dependents = data->next;
        free(data);
    }
    child->set.stream_depends_on = 0;
    child->set.stream_depends_e = FALSE;
}
void Curl_http2_cleanup_dependencies(struct Curl_easy *data) {
    while(data->set.stream_dependents) {
        struct Curl_easy *tmp = data->set.stream_dependents->data;
        Curl_http2_remove_child(data, tmp);
        if (data->set.stream_depends_on) Curl_http2_add_child(data->set.stream_depends_on, tmp, FALSE);
    }
    if (data->set.stream_depends_on) Curl_http2_remove_child(data->set.stream_depends_on, data);
}
bool Curl_h2_http_1_1_error(struct connectdata *conn) {
    struct http_conn *httpc = &conn->proto.httpc;
    return (httpc->error_code == NGHTTP2_HTTP_1_1_REQUIRED);
}
#else
#include "curl.h"
char *curl_pushheader_bynum(struct curl_pushheaders *h, size_t num) {
    (void) h;
    (void) num;
    return NULL;
}
char *curl_pushheader_byname(struct curl_pushheaders *h, const char *header) {
    (void) h;
    (void) header;
    return NULL;
}
#endif