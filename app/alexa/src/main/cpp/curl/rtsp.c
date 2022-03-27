#include <stdbool.h>
#include <assert.h>
#include "curl_setup.h"
#include "curl_ctype.h"

#ifndef CURL_DISABLE_RTSP
#include "urldata.h"
#include "curl.h"
#include "transfer.h"
#include "sendf.h"
#include "multiif.h"
#include "http.h"
#include "url.h"
#include "progress.h"
#include "rtsp.h"
#include "strcase.h"
#include "select.h"
#include "connect.h"
#include "strdup.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

#define RTP_PKT_CHANNEL(p)   ((int)((unsigned char)((p)[1])))
#define RTP_PKT_LENGTH(p)  ((((int)((unsigned char)((p)[2]))) << 8) | ((int)((unsigned char)((p)[3]))))

static int rtsp_getsock_do(struct connectdata *conn, curl_socket_t *socks) {
    socks[0] = conn->sock[FIRSTSOCKET];
    return GETSOCK_WRITESOCK(0);
}
static CURLcode rtp_client_write(struct connectdata *conn, char *ptr, size_t len);
const struct Curl_handler Curl_handler_rtsp = {
    "RTSP",
    rtsp_setup_connection,
    rtsp_do,
    rtsp_done,
    ZERO_NULL,
    rtsp_connect,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    rtsp_getsock_do,
    ZERO_NULL,
    ZERO_NULL,
    rtsp_disconnect,
    rtsp_rtp_readwrite,
    rtsp_conncheck,
    PORT_RTSP,
    CURLPROTO_RTSP,
    PROTOPT_NONE
};
static CURLcode rtsp_setup_connection(struct connectdata *conn) {
    struct RTSP *rtsp;
    conn->data->req.protop = rtsp = calloc(1, sizeof(struct RTSP));
    if(!rtsp) return CURLE_OUT_OF_MEMORY;
    return CURLE_OK;
}
static bool rtsp_connisdead(struct connectdata *check) {
    int sval;
    bool ret_val = TRUE;
    sval = SOCKET_READABLE(check->sock[FIRSTSOCKET], 0);
    if(sval == 0) ret_val = FALSE;
    else if(sval & CURL_CSELECT_ERR) ret_val = TRUE;
    else if(sval & CURL_CSELECT_IN) ret_val = !Curl_connalive(check);
    return ret_val;
}
static unsigned int rtsp_conncheck(struct connectdata *check, unsigned int checks_to_perform) {
    unsigned int ret_val = CONNRESULT_NONE;
    if(checks_to_perform & CONNCHECK_ISDEAD) {
        if(rtsp_connisdead(check)) ret_val |= CONNRESULT_DEAD;
    }
    return ret_val;
}
static CURLcode rtsp_connect(struct connectdata *conn, bool *done) {
    CURLcode httpStatus;
    struct Curl_easy *data = conn->data;
    httpStatus = Curl_http_connect(conn, done);
    if(data->state.rtsp_next_client_CSeq == 0) data->state.rtsp_next_client_CSeq = 1;
    if(data->state.rtsp_next_server_CSeq == 0) data->state.rtsp_next_server_CSeq = 1;
    conn->proto.rtspc.rtp_channel = -1;
    return httpStatus;
}
static CURLcode rtsp_disconnect(struct connectdata *conn, bool dead) {
    (void)dead;
    Curl_safefree(conn->proto.rtspc.rtp_buf);
    return CURLE_OK;
}
static CURLcode rtsp_done(struct connectdata *conn, CURLcode status, bool premature) {
    struct Curl_easy *data = conn->data;
    struct RTSP *rtsp = data->req.protop;
    CURLcode httpStatus;
    if(data->set.rtspreq == RTSPREQ_RECEIVE) premature = TRUE;
    httpStatus = Curl_http_done(conn, status, premature);
    if(rtsp) {
        long CSeq_sent = rtsp->CSeq_sent;
        long CSeq_recv = rtsp->CSeq_recv;
        if((data->set.rtspreq != RTSPREQ_RECEIVE) && (CSeq_sent != CSeq_recv)) {
            failf(data, "The CSeq of this request %ld did not match the response %ld", CSeq_sent, CSeq_recv);
            return CURLE_RTSP_CSEQ_ERROR;
        }
        if(data->set.rtspreq == RTSPREQ_RECEIVE && (conn->proto.rtspc.rtp_channel == -1)) {
            infof(data, "Got an RTP Receive with a CSeq of %ld\n", CSeq_recv);
        }
    }
    return httpStatus;
}
static CURLcode rtsp_do(struct connectdata *conn, bool *done) {
    struct Curl_easy *data = conn->data;
    CURLcode result = CURLE_OK;
    Curl_RtspReq rtspreq = data->set.rtspreq;
    struct RTSP *rtsp = data->req.protop;
    struct dynbuf req_buffer;
    curl_off_t postsize = 0;
    curl_off_t putsize = 0;
    const char *p_request = NULL;
    const char *p_session_id = NULL;
    const char *p_accept = NULL;
    const char *p_accept_encoding = NULL;
    const char *p_range = NULL;
    const char *p_referrer = NULL;
    const char *p_stream_uri = NULL;
    const char *p_transport = NULL;
    const char *p_uagent = NULL;
    const char *p_proxyuserpwd = NULL;
    const char *p_userpwd = NULL;
    *done = TRUE;
    rtsp->CSeq_sent = data->state.rtsp_next_client_CSeq;
    rtsp->CSeq_recv = 0;
    data->set.opt_no_body = TRUE;
    switch(rtspreq) {
        case RTSPREQ_OPTIONS: p_request = "OPTIONS"; break;
        case RTSPREQ_DESCRIBE:
            p_request = "DESCRIBE";
            data->set.opt_no_body = FALSE;
            break;
        case RTSPREQ_ANNOUNCE: p_request = "ANNOUNCE"; break;
        case RTSPREQ_SETUP: p_request = "SETUP"; break;
        case RTSPREQ_PLAY: p_request = "PLAY"; break;
        case RTSPREQ_PAUSE: p_request = "PAUSE"; break;
        case RTSPREQ_TEARDOWN: p_request = "TEARDOWN"; break;
        case RTSPREQ_GET_PARAMETER:
            p_request = "GET_PARAMETER";
            data->set.opt_no_body = FALSE;
            break;
        case RTSPREQ_SET_PARAMETER: p_request = "SET_PARAMETER"; break;
        case RTSPREQ_RECORD: p_request = "RECORD"; break;
        case RTSPREQ_RECEIVE:
            p_request = "";
            data->set.opt_no_body = FALSE;
            break;
        case RTSPREQ_LAST: failf(data, "Got invalid RTSP request: RTSPREQ_LAST"); return CURLE_BAD_FUNCTION_ARGUMENT;
        default: failf(data, "Got invalid RTSP request"); return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    if(rtspreq == RTSPREQ_RECEIVE) {
        Curl_setup_transfer(data, FIRSTSOCKET, -1, TRUE, -1);
        return result;
    }
    p_session_id = data->set.str[STRING_RTSP_SESSION_ID];
    if(!p_session_id && (rtspreq & ~(RTSPREQ_OPTIONS | RTSPREQ_DESCRIBE | RTSPREQ_SETUP))) {
        failf(data, "Refusing to issue an RTSP request [%s] without a session ID.", p_request);
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    if(data->set.str[STRING_RTSP_STREAM_URI]) p_stream_uri = data->set.str[STRING_RTSP_STREAM_URI];
    else p_stream_uri = "*";
    p_transport = Curl_checkheaders(conn, "Transport");
    if(rtspreq == RTSPREQ_SETUP && !p_transport) {
        if(data->set.str[STRING_RTSP_TRANSPORT]) {
            Curl_safefree(data->state.aptr.rtsp_transport);
            data->state.aptr.rtsp_transport = aprintf("Transport: %s\r\n", data->set.str[STRING_RTSP_TRANSPORT]);
            if(!data->state.aptr.rtsp_transport) return CURLE_OUT_OF_MEMORY;
        } else {
            failf(data, "Refusing to issue an RTSP SETUP without a Transport: header.");
            return CURLE_BAD_FUNCTION_ARGUMENT;
        }
        p_transport = data->state.aptr.rtsp_transport;
    }
    if(rtspreq == RTSPREQ_DESCRIBE) {
        p_accept = Curl_checkheaders(conn, "Accept") ? NULL : "Accept: application/sdp\r\n";
        if(!Curl_checkheaders(conn, "Accept-Encoding") && data->set.str[STRING_ENCODING]) {
            Curl_safefree(data->state.aptr.accept_encoding);
            data->state.aptr.accept_encoding = aprintf("Accept-Encoding: %s\r\n", data->set.str[STRING_ENCODING]);
            if(!data->state.aptr.accept_encoding) return CURLE_OUT_OF_MEMORY;
            p_accept_encoding = data->state.aptr.accept_encoding;
        }
    }
    if(Curl_checkheaders(conn, "User-Agent") && data->state.aptr.uagent) {
        Curl_safefree(data->state.aptr.uagent);
        data->state.aptr.uagent = NULL;
    } else if(!Curl_checkheaders(conn, "User-Agent") && data->set.str[STRING_USERAGENT]) p_uagent = data->state.aptr.uagent;
    result = Curl_http_output_auth(conn, p_request, p_stream_uri, FALSE);
    if(result) return result;
    p_proxyuserpwd = data->state.aptr.proxyuserpwd;
    p_userpwd = data->state.aptr.userpwd;
    Curl_safefree(data->state.aptr.ref);
    if(data->change.referer && !Curl_checkheaders(conn, "Referer")) data->state.aptr.ref = aprintf("Referer: %s\r\n", data->change.referer);
    else data->state.aptr.ref = NULL;
    p_referrer = data->state.aptr.ref;
    if(data->state.use_range && (rtspreq  & (RTSPREQ_PLAY | RTSPREQ_PAUSE | RTSPREQ_RECORD))) {
        if(!Curl_checkheaders(conn, "Range") && data->state.range) {
            Curl_safefree(data->state.aptr.rangeline);
            data->state.aptr.rangeline = aprintf("Range: %s\r\n", data->state.range);
            p_range = data->state.aptr.rangeline;
        }
    }
    if(Curl_checkheaders(conn, "CSeq")) {
        failf(data, "CSeq cannot be set as a custom header.");
        return CURLE_RTSP_CSEQ_ERROR;
    }
    if(Curl_checkheaders(conn, "Session")) {
    failf(data, "Session ID cannot be set as a custom header.");
    return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    Curl_dyn_init(&req_buffer, DYN_RTSP_REQ_HEADER);
    result = Curl_dyn_addf(&req_buffer, "%s %s RTSP/1.0\r\n" "CSeq: %ld\r\n", p_request, p_stream_uri, rtsp->CSeq_sent);
    if(result) return result;
    if(p_session_id) {
        result = Curl_dyn_addf(&req_buffer, "Session: %s\r\n", p_session_id);
        if(result) return result;
    }
    result = Curl_dyn_addf(&req_buffer,"%s""%s""%s""%s""%s""%s""%s""%s", p_transport ? p_transport : "",p_accept ? p_accept : "",
                           p_accept_encoding ? p_accept_encoding : "", p_range ? p_range : "", p_referrer ? p_referrer : "",
                           p_uagent ? p_uagent : "", p_proxyuserpwd ? p_proxyuserpwd : "", p_userpwd ? p_userpwd : "");
    Curl_safefree(data->state.aptr.userpwd);
    data->state.aptr.userpwd = NULL;
    if(result) return result;
    if((rtspreq == RTSPREQ_SETUP) || (rtspreq == RTSPREQ_DESCRIBE)) {
        result = Curl_add_timecondition(conn, &req_buffer);
        if(result) return result;
    }
    result = Curl_add_custom_headers(conn, FALSE, &req_buffer);
    if(result) return result;
    if(rtspreq == RTSPREQ_ANNOUNCE || rtspreq == RTSPREQ_SET_PARAMETER || rtspreq == RTSPREQ_GET_PARAMETER) {
        if(data->set.upload) {
            putsize = data->state.infilesize;
            data->state.httpreq = HTTPREQ_PUT;
        } else {
            postsize = (data->state.infilesize != -1)? data->state.infilesize: (data->set.postfields? (curl_off_t)strlen(data->set.postfields):0);
            data->state.httpreq = HTTPREQ_POST;
        }
        if(putsize > 0 || postsize > 0) {
            if(!Curl_checkheaders(conn, "Content-Length")) {
                result = Curl_dyn_addf(&req_buffer, "Content-Length: %" CURL_FORMAT_CURL_OFF_T"\r\n", (data->set.upload ? putsize : postsize));
                if(result) return result;
            }
            if(rtspreq == RTSPREQ_SET_PARAMETER || rtspreq == RTSPREQ_GET_PARAMETER) {
                if(!Curl_checkheaders(conn, "Content-Type")) {
                    result = Curl_dyn_addf(&req_buffer, "Content-Type: text/parameters\r\n");
                    if(result) return result;
                }
            }
            if(rtspreq == RTSPREQ_ANNOUNCE) {
                if(!Curl_checkheaders(conn, "Content-Type")) {
                    result = Curl_dyn_addf(&req_buffer, "Content-Type: application/sdp\r\n");
                    if(result) return result;
                }
            }
            data->state.expect100header = FALSE;
        } else if(rtspreq == RTSPREQ_GET_PARAMETER) {
            data->state.httpreq = HTTPREQ_HEAD;
            data->set.opt_no_body = TRUE;
        }
    }
    data->req.forbidchunk = TRUE;
    result = Curl_dyn_add(&req_buffer, "\r\n");
    if(result) return result;
    if(postsize > 0) {
        result = Curl_dyn_addn(&req_buffer, data->set.postfields, (size_t)postsize);
        if(result) return result;
    }
    result = Curl_buffer_send(&req_buffer, conn, &data->info.request_size, 0, FIRSTSOCKET);
    if(result) {
        failf(data, "Failed sending RTSP request");
        return result;
    }
    Curl_setup_transfer(data, FIRSTSOCKET, -1, TRUE, putsize?FIRSTSOCKET:-1);
    data->state.rtsp_next_client_CSeq++;
    if(data->req.writebytecount) {
        Curl_pgrsSetUploadCounter(data, data->req.writebytecount);
        if(Curl_pgrsUpdate(conn)) result = CURLE_ABORTED_BY_CALLBACK;
    }
    return result;
}
static CURLcode rtsp_rtp_readwrite(struct Curl_easy *data, struct connectdata *conn, ssize_t *nread, bool *readmore) {
    struct SingleRequest *k = &data->req;
    struct rtsp_conn *rtspc = &(conn->proto.rtspc);
    char *rtp;
    ssize_t rtp_dataleft;
    char *scratch;
    CURLcode result;
    if(rtspc->rtp_buf) {
        char *newptr = Curl_saferealloc(rtspc->rtp_buf,rtspc->rtp_bufsize + *nread);
        if(!newptr) {
            rtspc->rtp_buf = NULL;
            rtspc->rtp_bufsize = 0;
            return CURLE_OUT_OF_MEMORY;
        }
        rtspc->rtp_buf = newptr;
        memcpy(rtspc->rtp_buf + rtspc->rtp_bufsize, k->str, *nread);
        rtspc->rtp_bufsize += *nread;
        rtp = rtspc->rtp_buf;
        rtp_dataleft = rtspc->rtp_bufsize;
    } else {
        rtp = k->str;
        rtp_dataleft = *nread;
    }
    while((rtp_dataleft > 0) && (rtp[0] == '$')) {
        if(rtp_dataleft > 4) {
            int rtp_length;
            rtspc->rtp_channel = RTP_PKT_CHANNEL(rtp);
            rtp_length = RTP_PKT_LENGTH(rtp);
            if(rtp_dataleft < rtp_length + 4) {
                *readmore = TRUE;
                break;
            }
            DEBUGF(infof(data, "RTP write channel %d rtp_length %d\n", rtspc->rtp_channel, rtp_length));
            result = rtp_client_write(conn, &rtp[0], rtp_length + 4);
            if(result) {
                failf(data, "Got an error writing an RTP packet");
                *readmore = FALSE;
                Curl_safefree(rtspc->rtp_buf);
                rtspc->rtp_buf = NULL;
                rtspc->rtp_bufsize = 0;
                return result;
            }
            rtp_dataleft -= rtp_length + 4;
            rtp += rtp_length + 4;
            if(data->set.rtspreq == RTSPREQ_RECEIVE) k->keepon &= ~KEEP_RECV;
        } else {
          *readmore = TRUE;
          break;
        }
    }
    if(rtp_dataleft != 0 && rtp[0] == '$') {
        DEBUGF(infof(data, "RTP Rewinding %zd %s\n", rtp_dataleft, *readmore ? "(READMORE)" : ""));
        scratch = malloc(rtp_dataleft);
        if(!scratch) {
            Curl_safefree(rtspc->rtp_buf);
            rtspc->rtp_buf = NULL;
            rtspc->rtp_bufsize = 0;
            return CURLE_OUT_OF_MEMORY;
        }
        memcpy(scratch, rtp, rtp_dataleft);
        Curl_safefree(rtspc->rtp_buf);
        rtspc->rtp_buf = scratch;
        rtspc->rtp_bufsize = rtp_dataleft;
        *nread = 0;
        return CURLE_OK;
    }
    k->str += *nread - rtp_dataleft;
    if(rtp_dataleft > 0) DEBUGASSERT(k->str[0] == rtp[0]);
    DEBUGASSERT(rtp_dataleft <= *nread);
    *nread = rtp_dataleft;
    Curl_safefree(rtspc->rtp_buf);
    rtspc->rtp_buf = NULL;
    rtspc->rtp_bufsize = 0;
    return CURLE_OK;
}
static CURLcode rtp_client_write(struct connectdata *conn, char *ptr, size_t len) {
    struct Curl_easy *data = conn->data;
    size_t wrote;
    curl_write_callback writeit;
    void *user_ptr;
    if(len == 0) {
        failf(data, "Cannot write a 0 size RTP packet.");
        return CURLE_WRITE_ERROR;
    }
    if(data->set.fwrite_rtp) {
        writeit = data->set.fwrite_rtp;
        user_ptr = data->set.rtp_out;
    } else {
        writeit = data->set.fwrite_func;
        user_ptr = data->set.out;
    }
    Curl_set_in_callback(data, true);
    wrote = writeit(ptr, 1, len, user_ptr);
    Curl_set_in_callback(data, false);
    if(CURL_WRITEFUNC_PAUSE == wrote) {
        failf(data, "Cannot pause RTP");
        return CURLE_WRITE_ERROR;
    }
    if(wrote != len) {
        failf(data, "Failed writing RTP data");
        return CURLE_WRITE_ERROR;
    }
    return CURLE_OK;
}
CURLcode Curl_rtsp_parseheader(struct connectdata *conn, char *header) {
    struct Curl_easy *data = conn->data;
    long CSeq = 0;
    if(checkprefix("CSeq:", header)) {
        int nc = sscanf(&header[4], ": %ld", &CSeq);
        if(nc == 1) {
            struct RTSP *rtsp = data->req.protop;
            rtsp->CSeq_recv = CSeq;
            data->state.rtsp_CSeq_recv = CSeq;
        } else {
            failf(data, "Unable to read the CSeq header: [%s]", header);
            return CURLE_RTSP_CSEQ_ERROR;
        }
    } else if(checkprefix("Session:", header)) {
        char *start;
        start = header + 8;
        while(*start && ISSPACE(*start)) start++;
        if(!*start) failf(data, "Got a blank Session ID");
        else if(data->set.str[STRING_RTSP_SESSION_ID]) {
            if(strncmp(start, data->set.str[STRING_RTSP_SESSION_ID], strlen(data->set.str[STRING_RTSP_SESSION_ID]))  != 0) {
                failf(data, "Got RTSP Session ID Line [%s], but wanted ID [%s]", start, data->set.str[STRING_RTSP_SESSION_ID]);
                return CURLE_RTSP_SESSION_ERROR;
            }
        } else {
            char *end = start;
            while(*end && *end != ';' && !ISSPACE(*end)) end++;
            data->set.str[STRING_RTSP_SESSION_ID] = malloc(end - start + 1);
            if(data->set.str[STRING_RTSP_SESSION_ID] == NULL) return CURLE_OUT_OF_MEMORY;
            memcpy(data->set.str[STRING_RTSP_SESSION_ID], start, end - start);
            (data->set.str[STRING_RTSP_SESSION_ID])[end - start] = '\0';
        }
    }
    return CURLE_OK;
}
#endif