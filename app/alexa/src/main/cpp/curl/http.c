#include <assert.h>
#include <stdbool.h>
#include "curl_setup.h"
#include "curl_ctype.h"

#ifndef CURL_DISABLE_HTTP
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
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include "urldata.h"
#include "curl.h"
#include "transfer.h"
#include "sendf.h"
#include "formdata.h"
#include "mime.h"
#include "progress.h"
#include "curl_base64.h"
#include "cookie.h"
#include "vauth/vauth.h"
#include "vtls/vtls.h"
#include "http_digest.h"
#include "http_ntlm.h"
#include "curl_ntlm_wb.h"
#include "http_negotiate.h"
#include "url.h"
#include "share.h"
#include "hostip.h"
#include "http.h"
#include "select.h"
#include "parsedate.h"
#include "strtoofft.h"
#include "multiif.h"
#include "strcase.h"
#include "content_encoding.h"
#include "http_proxy.h"
#include "warnless.h"
#include "non-ascii.h"
#include "http2.h"
#include "connect.h"
#include "strdup.h"
#include "altsvc.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

static int http_getsock_do(struct connectdata *conn, curl_socket_t *socks);
static int http_should_fail(struct connectdata *conn);
#ifndef CURL_DISABLE_PROXY
static CURLcode add_haproxy_protocol_header(struct connectdata *conn);
#endif
#ifdef USE_SSL
static CURLcode https_connecting(struct connectdata *conn, bool *done);
static int https_getsock(struct connectdata *conn, curl_socket_t *socks);
#else
#define https_connecting(x,y) CURLE_COULDNT_CONNECT
#endif
static CURLcode http_setup_conn(struct connectdata *conn);
const struct Curl_handler Curl_handler_http = {
    "HTTP",
    http_setup_conn,
    Curl_http,
    Curl_http_done,
    ZERO_NULL,
    Curl_http_connect,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    http_getsock_do,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    PORT_HTTP,
    CURLPROTO_HTTP,
    PROTOPT_CREDSPERREQUEST | PROTOPT_USERPWDCTRL
};
#ifdef USE_SSL
const struct Curl_handler Curl_handler_https = {
    "HTTPS",
    http_setup_conn,
    Curl_http,
    Curl_http_done,
    ZERO_NULL,
    Curl_http_connect,
    https_connecting,
    ZERO_NULL,
    https_getsock,
    http_getsock_do,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    ZERO_NULL,
    PORT_HTTPS,
    CURLPROTO_HTTPS,
    PROTOPT_SSL | PROTOPT_CREDSPERREQUEST | PROTOPT_ALPN_NPN | PROTOPT_USERPWDCTRL
};
#endif
static CURLcode http_setup_conn(struct connectdata *conn) {
    struct HTTP *http;
    struct Curl_easy *data = conn->data;
    DEBUGASSERT(data->req.protop == NULL);
    http = calloc(1, sizeof(struct HTTP));
    if (!http) return CURLE_OUT_OF_MEMORY;
    Curl_mime_initpart(&http->form, conn->data);
    data->req.protop = http;
    if (data->set.httpversion == CURL_HTTP_VERSION_3) {
        if (conn->handler->flags & PROTOPT_SSL) conn->transport = TRNSPRT_QUIC;
        else {
            failf(data, "HTTP/3 requested for non-HTTPS URL");
            return CURLE_URL_MALFORMAT;
        }
    } else {
        if (!CONN_INUSE(conn))  Curl_http2_setup_conn(conn);
        Curl_http2_setup_req(data);
    }
    return CURLE_OK;
}
#ifndef CURL_DISABLE_PROXY
char *Curl_checkProxyheaders(const struct connectdata *conn, const char *thisheader) {
    struct curl_slist *head;
    size_t thislen = strlen(thisheader);
    struct Curl_easy *data = conn->data;
    for(head = (conn->bits.proxy && data->set.sep_headers) ? data->set.proxyheaders : data->set.headers; head; head = head->next) {
    if(strncasecompare(head->data, thisheader, thislen) &&
        Curl_headersep(head->data[thislen]))
        return head->data;
    }
    return NULL;
}
#else
#define Curl_checkProxyheaders(x,y) NULL
#endif
char *Curl_copy_header_value(const char *header) {
    const char *start;
    const char *end;
    char *value;
    size_t len;
    while(*header && (*header != ':')) ++header;
    if(*header) ++header;
    start = header;
    while(*start && ISSPACE(*start)) start++;
    end = strchr(start, '\r');
    if(!end) end = strchr(start, '\n');
    if(!end) end = strchr(start, '\0');
    if(!end) return NULL;
    while((end > start) && ISSPACE(*end)) end--;
    len = end - start + 1;
    value = malloc(len + 1);
    if(!value) return NULL;
    memcpy(value, start, len);
    value[len] = 0;
    return value;
}
#ifndef CURL_DISABLE_HTTP_AUTH
static CURLcode http_output_basic(struct connectdata *conn, bool proxy) {
    size_t size = 0;
    char *authorization = NULL;
    struct Curl_easy *data = conn->data;
    char **userp;
    const char *user;
    const char *pwd;
    CURLcode result;
    char *out;
    if(proxy) {
    #ifndef CURL_DISABLE_PROXY
        userp = &data->state.aptr.proxyuserpwd;
        user = conn->http_proxy.user;
        pwd = conn->http_proxy.passwd;
    #else
        return CURLE_NOT_BUILT_IN;
    #endif
    } else {
        userp = &data->state.aptr.userpwd;
        user = conn->user;
        pwd = conn->passwd;
    }
    out = aprintf("%s:%s", user, pwd ? pwd : "");
    if(!out) return CURLE_OUT_OF_MEMORY;
    result = Curl_base64_encode(data, out, strlen(out), &authorization, &size);
    if(result) goto fail;
    if(!authorization) {
        result = CURLE_REMOTE_ACCESS_DENIED;
        goto fail;
    }
    free(*userp);
    *userp = aprintf("%sAuthorization: Basic %s\r\n", proxy ? "Proxy-" : "", authorization);
    free(authorization);
    if(!*userp) {
        result = CURLE_OUT_OF_MEMORY;
        goto fail;
    }
    fail:
    free(out);
    return result;
}
static CURLcode http_output_bearer(struct connectdata *conn) {
    char **userp;
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    userp = &data->state.aptr.userpwd;
    free(*userp);
    *userp = aprintf("Authorization: Bearer %s\r\n", conn->data->set.str[STRING_BEARER]);
    if(!*userp) {
        result = CURLE_OUT_OF_MEMORY;
        goto fail;
    }
    fail:
    return result;
}
#endif
static bool pickoneauth(struct auth *pick, unsigned long mask) {
    bool picked;
    unsigned long avail = pick->avail & pick->want & mask;
    picked = TRUE;
    if(avail & CURLAUTH_NEGOTIATE) pick->picked = CURLAUTH_NEGOTIATE;
    else if(avail & CURLAUTH_BEARER) pick->picked = CURLAUTH_BEARER;
    else if(avail & CURLAUTH_DIGEST) pick->picked = CURLAUTH_DIGEST;
    else if(avail & CURLAUTH_NTLM) pick->picked = CURLAUTH_NTLM;
    else if(avail & CURLAUTH_NTLM_WB) pick->picked = CURLAUTH_NTLM_WB;
    else if(avail & CURLAUTH_BASIC) pick->picked = CURLAUTH_BASIC;
    else {
        pick->picked = CURLAUTH_PICKNONE;
        picked = FALSE;
    }
    pick->avail = CURLAUTH_NONE;
    return picked;
}
static CURLcode http_perhapsrewind(struct connectdata *conn) {
    struct Curl_easy *data = conn->data;
    struct HTTP *http = data->req.protop;
    curl_off_t bytessent;
    curl_off_t expectsend = -1;
    if(!http) return CURLE_OK;
    switch(data->state.httpreq) {
        case HTTPREQ_GET: case HTTPREQ_HEAD: return CURLE_OK;
    }
    bytessent = data->req.writebytecount;
    if(conn->bits.authneg) expectsend = 0;
    else if(!conn->bits.protoconnstart) expectsend = 0;
    else {
        switch(data->state.httpreq) {
            case HTTPREQ_POST: case HTTPREQ_PUT:
                if(data->state.infilesize != -1) expectsend = data->state.infilesize;
                break;
            case HTTPREQ_POST_FORM: case HTTPREQ_POST_MIME: expectsend = http->postsize;
        }
    }
    conn->bits.rewindaftersend = FALSE;
    if((expectsend == -1) || (expectsend > bytessent)) {
    #if defined(USE_NTLM)
        if((data->state.authproxy.picked == CURLAUTH_NTLM) || (data->state.authhost.picked == CURLAUTH_NTLM) ||
           (data->state.authproxy.picked == CURLAUTH_NTLM_WB) || (data->state.authhost.picked == CURLAUTH_NTLM_WB)) {
            if(((expectsend - bytessent) < 2000) || (conn->http_ntlm_state != NTLMSTATE_NONE) || (conn->proxy_ntlm_state != NTLMSTATE_NONE)) {
                if(!conn->bits.authneg && (conn->writesockfd != CURL_SOCKET_BAD)) {
                    conn->bits.rewindaftersend = TRUE;
                    infof(data, "Rewind stream after send\n");
                }
                return CURLE_OK;
            }
            if(conn->bits.close) return CURLE_OK;
            infof(data, "NTLM send, close instead of sending %" CURL_FORMAT_CURL_OFF_T " bytes\n", (curl_off_t)(expectsend - bytessent));
        }
    #endif
    #if defined(USE_SPNEGO)
        if((data->state.authproxy.picked == CURLAUTH_NEGOTIATE) || (data->state.authhost.picked == CURLAUTH_NEGOTIATE)) {
            if(((expectsend - bytessent) < 2000) || (conn->http_negotiate_state != GSS_AUTHNONE) || (conn->proxy_negotiate_state != GSS_AUTHNONE)) {
                if(!conn->bits.authneg && (conn->writesockfd != CURL_SOCKET_BAD)) {
                    conn->bits.rewindaftersend = TRUE;
                    infof(data, "Rewind stream after send\n");
                }
                return CURLE_OK;
            }
            if(conn->bits.close) return CURLE_OK;
            infof(data, "NEGOTIATE send, close instead of sending %" CURL_FORMAT_CURL_OFF_T " bytes\n", (curl_off_t)(expectsend - bytessent));
        }
    #endif
        streamclose(conn, "Mid-auth HTTP and much data left to send");
        data->req.size = 0;
    }
    if(bytessent) return Curl_readrewind(conn);
    return CURLE_OK;
}
CURLcode Curl_http_auth_act(struct connectdata *conn) {
    struct Curl_easy *data = conn->data;
    bool pickhost = FALSE;
    bool pickproxy = FALSE;
    CURLcode result = CURLE_OK;
    unsigned long authmask = ~0ul;
    if(!data->set.str[STRING_BEARER]) authmask &= (unsigned long)~CURLAUTH_BEARER;
    if(100 <= data->req.httpcode && 199 >= data->req.httpcode) return CURLE_OK;
    if(data->state.authproblem) return data->set.http_fail_on_error?CURLE_HTTP_RETURNED_ERROR:CURLE_OK;
    if((conn->bits.user_passwd || data->set.str[STRING_BEARER]) && ((data->req.httpcode == 401) || (conn->bits.authneg && data->req.httpcode < 300))) {
        pickhost = pickoneauth(&data->state.authhost, authmask);
        if(!pickhost) data->state.authproblem = TRUE;
        if(data->state.authhost.picked == CURLAUTH_NTLM &&
            conn->httpversion > 11) {
            infof(data, "Forcing HTTP/1.1 for NTLM");
            connclose(conn, "Force HTTP/1.1 connection");
            conn->data->set.httpversion = CURL_HTTP_VERSION_1_1;
        }
    }
#ifndef CURL_DISABLE_PROXY
    if (conn->bits.proxy_user_passwd && ((data->req.httpcode == 407) || (conn->bits.authneg && data->req.httpcode < 300))) {
        pickproxy = pickoneauth(&data->state.authproxy, authmask & ~CURLAUTH_BEARER);
        if (!pickproxy) data->state.authproblem = TRUE;
    }
#endif
    if (pickhost || pickproxy) {
        if ((data->state.httpreq != HTTPREQ_GET) && (data->state.httpreq != HTTPREQ_HEAD) && !conn->bits.rewindaftersend) {
            result = http_perhapsrewind(conn);
            if (result) return result;
        }
        Curl_safefree(data->req.newurl);
        data->req.newurl = strdup(data->change.url);
        if (!data->req.newurl) return CURLE_OUT_OF_MEMORY;
    } else if ((data->req.httpcode < 300) && (!data->state.authhost.done) && conn->bits.authneg) {
        if ((data->state.httpreq != HTTPREQ_GET) && (data->state.httpreq != HTTPREQ_HEAD)) {
            data->req.newurl = strdup(data->change.url);
            if (!data->req.newurl) return CURLE_OUT_OF_MEMORY;
            data->state.authhost.done = TRUE;
        }
    }
    if (http_should_fail(conn)) {
        failf(data, "The requested URL returned error: %d", data->req.httpcode);
        result = CURLE_HTTP_RETURNED_ERROR;
    }
    return result;
}
#ifndef CURL_DISABLE_HTTP_AUTH
static CURLcode output_auth_headers(struct connectdata *conn, struct auth *authstatus, const char *request, const char *path, bool proxy) {
    const char *auth = NULL;
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
#ifdef CURL_DISABLE_CRYPTO_AUTH
    (void)request;
    (void)path;
#endif
#ifdef USE_SPNEGO
    if(authstatus->picked == CURLAUTH_NEGOTIATE) {
        auth = "Negotiate";
        result = Curl_output_negotiate(conn, proxy);
        if(result) return result;
    } else
#endif
#ifdef USE_NTLM
    if(authstatus->picked == CURLAUTH_NTLM) {
        auth = "NTLM";
        result = Curl_output_ntlm(conn, proxy);
        if(result) return result;
    } else
#endif
#if defined(USE_NTLM) && defined(NTLM_WB_ENABLED)
    if(authstatus->picked == CURLAUTH_NTLM_WB) {
        auth = "NTLM_WB";
        result = Curl_output_ntlm_wb(conn, proxy);
        if(result) return result;
    } else
#endif
#ifndef CURL_DISABLE_CRYPTO_AUTH
    if (authstatus->picked == CURLAUTH_DIGEST) {
        auth = "Digest";
        result = Curl_output_digest(conn, proxy, (const unsigned char*)request, (const unsigned char*)path);
        if (result) return result;
    } else
#endif
    if (authstatus->picked == CURLAUTH_BASIC) {
        if (
        #ifndef CURL_DISABLE_PROXY
           (proxy && conn->bits.proxy_user_passwd && !Curl_checkProxyheaders(conn, "Proxy-authorization")) ||
        #endif
           (!proxy && conn->bits.user_passwd && !Curl_checkheaders(conn, "Authorization"))) {
            auth = "Basic";
            result = http_output_basic(conn, proxy);
            if (result) return result;
        }
        authstatus->done = TRUE;
    }
    if(authstatus->picked == CURLAUTH_BEARER) {
        if((!proxy && data->set.str[STRING_BEARER] && !Curl_checkheaders(conn, "Authorization:"))) {
            auth = "Bearer";
            result = http_output_bearer(conn);
            if(result) return result;
        }
        authstatus->done = TRUE;
    }
    if(auth) {
    #ifndef CURL_DISABLE_PROXY
        infof(data, "%s auth using %s with user '%s'\n", proxy ? "Proxy" : "Server", auth, proxy ? (conn->http_proxy.user ? conn->http_proxy.user : "") :
              (conn->user ? conn->user : ""));
    #else
        infof(data, "Server auth using %s with user '%s'\n", auth, conn->user ? conn->user : "");
    #endif
        authstatus->multipass = (!authstatus->done) ? TRUE : FALSE;
    } else authstatus->multipass = FALSE;
    return CURLE_OK;
}
CURLcode Curl_http_output_auth(struct connectdata *conn, const char *request, const char *path, bool proxytunnel) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct auth *authhost;
    struct auth *authproxy;
    DEBUGASSERT(data);
    authhost = &data->state.authhost;
    authproxy = &data->state.authproxy;
    if (
    #ifndef CURL_DISABLE_PROXY
       (conn->bits.httpproxy && conn->bits.proxy_user_passwd) ||
    #endif
       conn->bits.user_passwd || data->set.str[STRING_BEARER]);
    else {
        authhost->done = TRUE;
        authproxy->done = TRUE;
        return CURLE_OK;
    }
    if (authhost->want && !authhost->picked) authhost->picked = authhost->want;
    if (authproxy->want && !authproxy->picked) authproxy->picked = authproxy->want;
#ifndef CURL_DISABLE_PROXY
    if (conn->bits.httpproxy && (conn->bits.tunnel_proxy == (bit)proxytunnel)) {
        result = output_auth_headers(conn, authproxy, request, path, TRUE);
        if (result) return result;
    } else
#else
    (void)proxytunnel;
#endif
    authproxy->done = TRUE;
    if(!data->state.this_is_a_follow || conn->bits.netrc || !data->state.first_host || data->set.allow_auth_to_other_hosts ||
       strcasecompare(data->state.first_host, conn->host.name)) {
        result = output_auth_headers(conn, authhost, request, path, FALSE);
    } else authhost->done = TRUE;
    return result;
}
#else
CURLcode Curl_http_output_auth(struct connectdata *conn, const char *request, const char *path, bool proxytunnel) {
    (void)conn;
    (void)request;
    (void)path;
    (void)proxytunnel;
    return CURLE_OK;
}
#endif
CURLcode Curl_http_input_auth(struct connectdata *conn, bool proxy, const char *auth) {
    struct Curl_easy *data = conn->data;
#ifdef USE_SPNEGO
    curlnegotiate *negstate = proxy ? &conn->proxy_negotiate_state : &conn->http_negotiate_state;
#endif
    unsigned long *availp;
    struct auth *authp;
    if(proxy) {
        availp = &data->info.proxyauthavail;
        authp = &data->state.authproxy;
    } else {
        availp = &data->info.httpauthavail;
        authp = &data->state.authhost;
    }
    while(*auth) {
    #ifdef USE_SPNEGO
        if(checkprefix("Negotiate", auth)) {
            if((authp->avail & CURLAUTH_NEGOTIATE) || Curl_auth_is_spnego_supported()) {
                *availp |= CURLAUTH_NEGOTIATE;
                authp->avail |= CURLAUTH_NEGOTIATE;
                if(authp->picked == CURLAUTH_NEGOTIATE) {
                    CURLcode result = Curl_input_negotiate(conn, proxy, auth);
                    if(!result) {
                        DEBUGASSERT(!data->req.newurl);
                        data->req.newurl = strdup(data->change.url);
                        if(!data->req.newurl) return CURLE_OUT_OF_MEMORY;
                        data->state.authproblem = FALSE;
                        *negstate = GSS_AUTHRECV;
                    } else data->state.authproblem = TRUE;
                }
            }
        } else
    #endif
    #ifdef USE_NTLM
        if (checkprefix("NTLM", auth)) {
            if ((authp->avail & CURLAUTH_NTLM) || (authp->avail & CURLAUTH_NTLM_WB) || Curl_auth_is_ntlm_supported()) {
                *availp |= CURLAUTH_NTLM;
                authp->avail |= CURLAUTH_NTLM;
                if (authp->picked == CURLAUTH_NTLM || authp->picked == CURLAUTH_NTLM_WB) {
                    CURLcode result = Curl_input_ntlm(conn, proxy, auth);
                    if (!result) {
                        data->state.authproblem = FALSE;
                    #ifdef NTLM_WB_ENABLED
                        if (authp->picked == CURLAUTH_NTLM_WB) {
                            *availp &= ~CURLAUTH_NTLM;
                            authp->avail &= ~CURLAUTH_NTLM;
                            *availp |= CURLAUTH_NTLM_WB;
                            authp->avail |= CURLAUTH_NTLM_WB;
                            result = Curl_input_ntlm_wb(conn, proxy, auth);
                            if (result) {
                                infof(data, "Authentication problem. Ignoring this.\n");
                                data->state.authproblem = TRUE;
                            }
                        }
                    #endif
                    } else {
                      infof(data, "Authentication problem. Ignoring this.\n");
                      data->state.authproblem = TRUE;
                    }
                }
            }
        } else
    #endif
    #ifndef CURL_DISABLE_CRYPTO_AUTH
        if(checkprefix("Digest", auth)) {
            if((authp->avail & CURLAUTH_DIGEST) != 0) infof(data, "Ignoring duplicate digest auth header.\n");
            else if(Curl_auth_is_digest_supported()) {
                CURLcode result;
                *availp |= CURLAUTH_DIGEST;
                authp->avail |= CURLAUTH_DIGEST;
                result = Curl_input_digest(conn, proxy, auth);
                if(result) {
                    infof(data, "Authentication problem. Ignoring this.\n");
                    data->state.authproblem = TRUE;
                }
            }
        } else
    #endif
        if(checkprefix("Basic", auth)) {
            *availp |= CURLAUTH_BASIC;
            authp->avail |= CURLAUTH_BASIC;
            if(authp->picked == CURLAUTH_BASIC) {
                authp->avail = CURLAUTH_NONE;
                infof(data, "Authentication problem. Ignoring this.\n");
                data->state.authproblem = TRUE;
            }
        } else if(checkprefix("Bearer", auth)) {
            *availp |= CURLAUTH_BEARER;
            authp->avail |= CURLAUTH_BEARER;
            if(authp->picked == CURLAUTH_BEARER) {
                authp->avail = CURLAUTH_NONE;
                infof(data, "Authentication problem. Ignoring this.\n");
                data->state.authproblem = TRUE;
            }
        }
        while(*auth && *auth != ',') auth++;
        if(*auth == ',') auth++;
        while(*auth && ISSPACE(*auth)) auth++;
    }
    return CURLE_OK;
}
static int http_should_fail(struct connectdata *conn) {
    struct Curl_easy *data;
    int httpcode;
    DEBUGASSERT(conn);
    data = conn->data;
    DEBUGASSERT(data);
    httpcode = data->req.httpcode;
    if(!data->set.http_fail_on_error) return 0;
    if(httpcode < 400) return 0;
    if((httpcode != 401) && (httpcode != 407)) return 1;
    DEBUGASSERT((httpcode == 401) || (httpcode == 407));
    if((httpcode == 401) && !conn->bits.user_passwd) return TRUE;
#ifndef CURL_DISABLE_PROXY
    if((httpcode == 407) && !conn->bits.proxy_user_passwd) return TRUE;
#endif
    return data->state.authproblem;
}
static size_t readmoredata(char *buffer, size_t size, size_t nitems, void *userp) {
    struct connectdata *conn = (struct connectdata *)userp;
    struct HTTP *http = conn->data->req.protop;
    size_t fullsize = size * nitems;
    if(!http->postsize) return 0;
    conn->data->req.forbidchunk = (http->sending == HTTPSEND_REQUEST) ? TRUE : FALSE;
    if(http->postsize <= (curl_off_t)fullsize) {
        memcpy(buffer, http->postdata, (size_t)http->postsize);
        fullsize = (size_t)http->postsize;
        if(http->backup.postsize) {
            http->postdata = http->backup.postdata;
            http->postsize = http->backup.postsize;
            conn->data->state.fread_func = http->backup.fread_func;
            conn->data->state.in = http->backup.fread_in;
            http->sending++;
            http->backup.postsize = 0;
        } else http->postsize = 0;
        return fullsize;
    }
    memcpy(buffer, http->postdata, fullsize);
    http->postdata += fullsize;
    http->postsize -= fullsize;
    return fullsize;
}
CURLcode Curl_buffer_send(struct dynbuf *in, struct connectdata *conn, curl_off_t *bytes_written, size_t included_body_bytes, int socketindex) {
    ssize_t amount;
    CURLcode result;
    char *ptr;
    size_t size;
    struct Curl_easy *data = conn->data;
    struct HTTP *http = data->req.protop;
    size_t sendsize;
    curl_socket_t sockfd;
    size_t headersize;
    DEBUGASSERT(socketindex <= SECONDARYSOCKET);
    sockfd = conn->sock[socketindex];
    ptr = Curl_dyn_ptr(in);
    size = Curl_dyn_len(in);
    headersize = size - included_body_bytes;
    DEBUGASSERT(size > included_body_bytes);
    result = Curl_convert_to_network(data, ptr, headersize);
    if (result) {
        Curl_dyn_free(in);
        return result;
    }
    if ((conn->handler->flags & PROTOPT_SSL
#ifndef CURL_DISABLE_PROXY
        || conn->http_proxy.proxytype == CURLPROXY_HTTPS
#endif
       ) && conn->httpversion != 20) {
        sendsize = CURLMIN(size, CURL_MAX_WRITE_SIZE);
        result = Curl_get_upload_buffer(data);
        if(result) {
            Curl_dyn_free(in);
            return result;
        }
        memcpy(data->state.ulbuf, ptr, sendsize);
        ptr = data->state.ulbuf;
    } else {
    #ifdef CURLDEBUG
        char *p = getenv("CURL_SMALLREQSEND");
        if(p) {
            size_t altsize = (size_t)strtoul(p, NULL, 10);
            if(altsize) sendsize = CURLMIN(size, altsize);
            else sendsize = size;
        } else
    #endif
        sendsize = size;
    }
    result = Curl_write(conn, sockfd, ptr, sendsize, &amount);
    if (!result) {
        size_t headlen = (size_t)amount>headersize ? headersize : (size_t)amount;
        size_t bodylen = amount - headlen;
        if (data->set.verbose) {
            Curl_debug(data, CURLINFO_HEADER_OUT, ptr, headlen);
            if (bodylen) Curl_debug(data, CURLINFO_DATA_OUT,ptr + headlen, bodylen);
        }
        *bytes_written += (long)amount;
        if(http) {
            data->req.writebytecount += bodylen;
            Curl_pgrsSetUploadCounter(data, data->req.writebytecount);
            if((size_t)amount != size) {
                size -= amount;
                ptr = Curl_dyn_ptr(in) + amount;
                http->backup.fread_func = data->state.fread_func;
                http->backup.fread_in = data->state.in;
                http->backup.postdata = http->postdata;
                http->backup.postsize = http->postsize;
                data->state.fread_func = (curl_read_callback)readmoredata;
                data->state.in = (void *)conn;
                http->postdata = ptr;
                http->postsize = (curl_off_t)size;
                http->send_buffer = *in;
                http->sending = HTTPSEND_REQUEST;
                return CURLE_OK;
            }
            http->sending = HTTPSEND_BODY;
        } else {
            if((size_t)amount != size) return CURLE_SEND_ERROR;
        }
    }
    Curl_dyn_free(in);
    return result;
}
bool Curl_compareheader(const char *headerline, const char *header, const char *content) {
    size_t hlen = strlen(header);
    size_t clen;
    size_t len;
    const char *start;
    const char *end;
    if(!strncasecompare(headerline, header, hlen)) return FALSE;
    start = &headerline[hlen];
    while(*start && ISSPACE(*start)) start++;
    end = strchr(start, '\r');
    if(!end) {
        end = strchr(start, '\n');
        if(!end) end = strchr(start, '\0');
    }
    len = end-start;
    clen = strlen(content);
    for(; len >= clen; len--, start++) {
        if(strncasecompare(start, content, clen)) return TRUE;
    }
    return FALSE;
}
CURLcode Curl_http_connect(struct connectdata *conn, bool *done) {
    CURLcode result;
    connkeep(conn, "HTTP default");
#ifndef CURL_DISABLE_PROXY
    result = Curl_proxy_connect(conn, FIRSTSOCKET);
    if(result) return result;
    if(conn->bits.proxy_connect_closed) return CURLE_OK;
    if(CONNECT_FIRSTSOCKET_PROXY_SSL()) return CURLE_OK;
    if(Curl_connect_ongoing(conn)) return CURLE_OK;
    if(conn->data->set.haproxyprotocol) {
        result = add_haproxy_protocol_header(conn);
        if(result) return result;
    }
#endif
    if(conn->given->protocol & CURLPROTO_HTTPS) {
        result = https_connecting(conn, done);
        if(result) return result;
    } else *done = TRUE;
    return CURLE_OK;
}
static int http_getsock_do(struct connectdata *conn, curl_socket_t *socks) {
    socks[0] = conn->sock[FIRSTSOCKET];
    return GETSOCK_WRITESOCK(0);
}
#ifndef CURL_DISABLE_PROXY
static CURLcode add_haproxy_protocol_header(struct connectdata *conn) {
    char proxy_header[128];
    struct dynbuf req;
    CURLcode result;
    char tcp_version[5];
    if(conn->bits.ipv6) strcpy(tcp_version, "TCP6");
    else strcpy(tcp_version, "TCP4");
    msnprintf(proxy_header, sizeof(proxy_header), "PROXY %s %s %s %li %li\r\n", tcp_version, conn->data->info.conn_local_ip,
              conn->data->info.conn_primary_ip, conn->data->info.conn_local_port, conn->data->info.conn_primary_port);
    Curl_dyn_init(&req, DYN_HAXPROXY);
    result = Curl_dyn_add(&req, proxy_header);
    if(result) return result;
    result = Curl_buffer_send(&req, conn, &conn->data->info.request_size,0, FIRSTSOCKET);
    return result;
}
#endif
#ifdef USE_SSL
static CURLcode https_connecting(struct connectdata *conn, bool *done) {
    CURLcode result;
    DEBUGASSERT((conn) && (conn->handler->flags & PROTOPT_SSL));
#ifdef ENABLE_QUIC
    if(conn->transport == TRNSPRT_QUIC) {
        *done = TRUE;
        return CURLE_OK;
    }
#endif
    result = Curl_ssl_connect_nonblocking(conn, FIRSTSOCKET, done);
    if(result) connclose(conn, "Failed HTTPS connection");
    return result;
}
static int https_getsock(struct connectdata *conn, curl_socket_t *socks) {
    if(conn->handler->flags & PROTOPT_SSL) return Curl_ssl_getsock(conn, socks);
    return GETSOCK_BLANK;
}
#endif
CURLcode Curl_http_done(struct connectdata *conn, CURLcode status, bool premature) {
    struct Curl_easy *data = conn->data;
    struct HTTP *http = data->req.protop;
    data->state.authhost.multipass = FALSE;
    data->state.authproxy.multipass = FALSE;
    Curl_unencode_cleanup(conn);
    conn->seek_func = data->set.seek_func;
    conn->seek_client = data->set.seek_client;
    if (!http) return CURLE_OK;
    Curl_dyn_free(&http->send_buffer);
    Curl_mime_cleanpart(&http->form);
    Curl_dyn_reset(&data->state.headerb);
    if (status) return status;
    if (!premature && !conn->bits.retry && !data->set.connect_only && (data->req.bytecount + data->req.headerbytecount - data->req.deductheadercount) <= 0) {
        failf(data, "Empty reply from server");
        return CURLE_GOT_NOTHING;
    }
    return CURLE_OK;
}
static bool use_http_1_1plus(const struct Curl_easy *data, const struct connectdata *conn) {
    if ((data->state.httpversion == 10) || (conn->httpversion == 10)) return FALSE;
    if ((data->set.httpversion == CURL_HTTP_VERSION_1_0) && (conn->httpversion <= 10)) return FALSE;
    return ((data->set.httpversion == CURL_HTTP_VERSION_NONE) || (data->set.httpversion >= CURL_HTTP_VERSION_1_1));
}
static const char *get_http_string(const struct Curl_easy *data, const struct connectdata *conn) {
#ifdef ENABLE_QUIC
    if((data->set.httpversion == CURL_HTTP_VERSION_3) || (conn->httpversion == 30)) return "3";
#endif
#ifdef USE_NGHTTP2
    if(conn->proto.httpc.h2) return "2";
#endif
    if(use_http_1_1plus(data, conn)) return "1.1";
    return "1.0";
}
static CURLcode expect100(struct Curl_easy *data, struct connectdata *conn, struct dynbuf *req) {
    CURLcode result = CURLE_OK;
    data->state.expect100header = FALSE;
    if (!data->state.disableexpect && use_http_1_1plus(data, conn) && (conn->httpversion < 20)) {
        const char *ptr = Curl_checkheaders(conn, "Expect");
        if (ptr) data->state.expect100header = Curl_compareheader(ptr, "Expect:", "100-continue");
        else {
            result = Curl_dyn_add(req, "Expect: 100-continue\r\n");
            if (!result) data->state.expect100header = TRUE;
        }
    }
    return result;
}
enum proxy_use {
    HEADER_SERVER,
    HEADER_PROXY,
    HEADER_CONNECT
};
CURLcode Curl_http_compile_trailers(struct curl_slist *trailers, struct dynbuf *b, struct Curl_easy *handle) {
    char *ptr = NULL;
    CURLcode result = CURLE_OK;
    const char *endofline_native = NULL;
    const char *endofline_network = NULL;
    if(
#ifdef CURL_DO_LINEEND_CONV
       (handle->set.prefer_ascii) ||
#endif
       (handle->set.crlf)) {
        endofline_native  = "\n";
        endofline_network = "\x0a";
    } else {
        endofline_native  = "\r\n";
        endofline_network = "\x0d\x0a";
    }
    while(trailers) {
        ptr = strchr(trailers->data, ':');
        if (ptr && *(ptr + 1) == ' ') {
            result = Curl_dyn_add(b, trailers->data);
            if (result) return result;
            result = Curl_dyn_add(b, endofline_native);
            if (result) return result;
        } else infof(handle, "Malformatted trailing header ! Skipping trailer.");
        trailers = trailers->next;
    }
    result = Curl_dyn_add(b, endofline_network);
    return result;
}
CURLcode Curl_add_custom_headers(struct connectdata *conn, bool is_connect, struct dynbuf *req) {
    char *ptr;
    struct curl_slist *h[2];
    struct curl_slist *headers;
    int numlists = 1;
    struct Curl_easy *data = conn->data;
    int i;
#ifndef CURL_DISABLE_PROXY
    enum proxy_use proxy;
    if(is_connect) proxy = HEADER_CONNECT;
    else proxy = conn->bits.httpproxy && !conn->bits.tunnel_proxy ? HEADER_PROXY : HEADER_SERVER;
    switch(proxy) {
        case HEADER_SERVER: h[0] = data->set.headers; break;
        case HEADER_PROXY:
            h[0] = data->set.headers;
            if(data->set.sep_headers) {
                h[1] = data->set.proxyheaders;
                numlists++;
            }
            break;
        case HEADER_CONNECT:
            if(data->set.sep_headers) h[0] = data->set.proxyheaders;
            else h[0] = data->set.headers;
            break;
    }
#else
    (void)is_connect;
    h[0] = data->set.headers;
#endif
    for(i = 0; i < numlists; i++) {
        headers = h[i];
        while(headers) {
            char *semicolonp = NULL;
            ptr = strchr(headers->data, ':');
            if(!ptr) {
                char *optr;
                ptr = strchr(headers->data, ';');
                if(ptr) {
                    optr = ptr;
                    ptr++;
                    while(*ptr && ISSPACE(*ptr)) ptr++;
                    if(*ptr) optr = NULL;
                    else {
                        if(*(--ptr) == ';') {
                            semicolonp = strdup(headers->data);
                            if(!semicolonp) {
                                Curl_dyn_free(req);
                                return CURLE_OUT_OF_MEMORY;
                            }
                            semicolonp[ptr - headers->data] = ':';
                            optr = &semicolonp [ptr - headers->data];
                        }
                    }
                    ptr = optr;
                }
            }
            if(ptr) {
                while(*ptr && ISSPACE(*ptr)) ptr++;
                if(*ptr || semicolonp) {
                    CURLcode result = CURLE_OK;
                    char *compare = semicolonp ? semicolonp : headers->data;
                    if(data->state.aptr.host && checkprefix("Host:", compare));
                    else if(data->state.httpreq == HTTPREQ_POST_FORM && checkprefix("Content-Type:", compare));
                    else if(data->state.httpreq == HTTPREQ_POST_MIME &&checkprefix("Content-Type:", compare));
                    else if(conn->bits.authneg && checkprefix("Content-Length:", compare));
                    else if(data->state.aptr.te && checkprefix("Connection:", compare));
                    else if((conn->httpversion >= 20) && checkprefix("Transfer-Encoding:", compare));
                    else if((checkprefix("Authorization:", compare) || checkprefix("Cookie:", compare)) &&
                            (data->state.this_is_a_follow && data->state.first_host && !data->set.allow_auth_to_other_hosts &&
                            !strcasecompare(data->state.first_host, conn->host.name)));
                    else result = Curl_dyn_addf(req, "%s\r\n", compare);
                    if(semicolonp) free(semicolonp);
                    if(result) return result;
                }
            }
            headers = headers->next;
        }
    }
    return CURLE_OK;
}
#ifndef CURL_DISABLE_PARSEDATE
CURLcode Curl_add_timecondition(const struct connectdata *conn, struct dynbuf *req) {
    struct Curl_easy *data = conn->data;
    const struct tm *tm;
    struct tm keeptime;
    CURLcode result;
    char datestr[80];
    const char *condp;
    if(data->set.timecondition == CURL_TIMECOND_NONE) return CURLE_OK;
    result = Curl_gmtime(data->set.timevalue, &keeptime);
    if(result) {
        failf(data, "Invalid TIMEVALUE");
        return result;
    }
    tm = &keeptime;
    switch(data->set.timecondition) {
        case CURL_TIMECOND_IFMODSINCE: condp = "If-Modified-Since"; break;
        case CURL_TIMECOND_IFUNMODSINCE: condp = "If-Unmodified-Since"; break;
        case CURL_TIMECOND_LASTMOD: condp = "Last-Modified"; break;
        default: return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    if(Curl_checkheaders(conn, condp)) return CURLE_OK;
    msnprintf(datestr, sizeof(datestr), "%s: %s, %02d %s %4d %02d:%02d:%02d GMT\r\n", condp, Curl_wkday[tm->tm_wday?tm->tm_wday-1:6], tm->tm_mday,
              Curl_month[tm->tm_mon], tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
    result = Curl_dyn_add(req, datestr);
    return result;
}
#else
CURLcode Curl_add_timecondition(const struct connectdata *conn, struct dynbuf *req) {
    (void)conn;
    (void)req;
    return CURLE_OK;
}
#endif
CURLcode Curl_http(struct connectdata *conn, bool *done) {
    struct Curl_easy *data = conn->data;
    CURLcode result = CURLE_OK;
    struct HTTP *http;
    const char *path = data->state.up.path;
    const char *query = data->state.up.query;
    bool paste_ftp_userpwd = FALSE;
    char ftp_typecode[sizeof("/;type=?")] = "";
    const char *host = conn->host.name;
    const char *te = "";
    const char *ptr;
    const char *request;
    Curl_HttpReq httpreq = data->state.httpreq;
#if !defined(CURL_DISABLE_COOKIES)
    char *addcookies = NULL;
#endif
    curl_off_t included_body = 0;
    const char *httpstring;
    struct dynbuf req;
    curl_off_t postsize = 0;
    char *altused = NULL;
    *done = TRUE;
    if(conn->transport != TRNSPRT_QUIC) {
        if(conn->httpversion < 20) {
            switch(conn->negnpn) {
                case CURL_HTTP_VERSION_2:
                    conn->httpversion = 20;
                    result = Curl_http2_switched(conn, NULL, 0);
                    if(result) return result;
                    break;
                case CURL_HTTP_VERSION_1_1: break;
                default:
                #ifdef USE_NGHTTP2
                    if(conn->data->set.httpversion == CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE) {
                    #ifndef CURL_DISABLE_PROXY
                        if(conn->bits.httpproxy && !conn->bits.tunnel_proxy) {
                            infof(data, "Ignoring HTTP/2 prior knowledge due to proxy\n");
                            break;
                        }
                    #endif
                        DEBUGF(infof(data, "HTTP/2 over clean TCP\n"));
                        conn->httpversion = 20;
                        result = Curl_http2_switched(conn, NULL, 0);
                        if(result) return result;
                    }
                #endif
                    break;
            }
        } else {
            result = Curl_http2_setup(conn);
            if(result) return result;
        }
    }
    http = data->req.protop;
    DEBUGASSERT(http);
    if (!data->state.this_is_a_follow) {
        free(data->state.first_host);
        data->state.first_host = strdup(conn->host.name);
        if (!data->state.first_host) return CURLE_OUT_OF_MEMORY;
        data->state.first_remote_port = conn->remote_port;
    }
    if((conn->handler->protocol&(PROTO_FAMILY_HTTP|CURLPROTO_FTP)) && data->set.upload) httpreq = HTTPREQ_PUT;
    if(data->set.str[STRING_CUSTOMREQUEST]) request = data->set.str[STRING_CUSTOMREQUEST];
    else {
        if(data->set.opt_no_body) request = "HEAD";
        else {
            DEBUGASSERT((httpreq > HTTPREQ_NONE) && (httpreq < HTTPREQ_LAST));
            switch(httpreq) {
                case HTTPREQ_POST: case HTTPREQ_POST_FORM: case HTTPREQ_POST_MIME: request = "POST"; break;
                case HTTPREQ_PUT: request = "PUT"; break;
                case HTTPREQ_OPTIONS: request = "OPTIONS"; break;
                case HTTPREQ_GET: request = "GET"; break;
                case HTTPREQ_HEAD: request = "HEAD"; break;
            }
        }
    }
    if (Curl_checkheaders(conn, "User-Agent")) {
        free(data->state.aptr.uagent);
        data->state.aptr.uagent = NULL;
    }
    char *pq = NULL;
    if (query && *query) {
        pq = aprintf("%s?%s", path, query);
        if (!pq) return CURLE_OUT_OF_MEMORY;
    }
    result = Curl_http_output_auth(conn, request, (pq ? pq : path), FALSE);
    free(pq);
    if (result) return result;
    if (((data->state.authhost.multipass && !data->state.authhost.done) || (data->state.authproxy.multipass && !data->state.authproxy.done)) &&
       (httpreq != HTTPREQ_GET) && (httpreq != HTTPREQ_HEAD)) {
        conn->bits.authneg = TRUE;
    } else conn->bits.authneg = FALSE;
    Curl_safefree(data->state.aptr.ref);
    if (data->change.referer && !Curl_checkheaders(conn, "Referer")) {
        data->state.aptr.ref = aprintf("Referer: %s\r\n", data->change.referer);
        if (!data->state.aptr.ref) return CURLE_OUT_OF_MEMORY;
    } else data->state.aptr.ref = NULL;
#if !defined(CURL_DISABLE_COOKIES)
    if (data->set.str[STRING_COOKIE] && !Curl_checkheaders(conn, "Cookie")) addcookies = data->set.str[STRING_COOKIE];
#endif
    if(!Curl_checkheaders(conn, "Accept-Encoding") && data->set.str[STRING_ENCODING]) {
        Curl_safefree(data->state.aptr.accept_encoding);
        data->state.aptr.accept_encoding = aprintf("Accept-Encoding: %s\r\n", data->set.str[STRING_ENCODING]);
        if(!data->state.aptr.accept_encoding) return CURLE_OUT_OF_MEMORY;
    } else {
        Curl_safefree(data->state.aptr.accept_encoding);
        data->state.aptr.accept_encoding = NULL;
    }
#ifdef HAVE_LIBZ
    if (!Curl_checkheaders(conn, "TE") && data->set.http_transfer_encoding) {
        char *cptr = Curl_checkheaders(conn, "Connection");
        #define TE_HEADER "TE: gzip\r\n"
        Curl_safefree(data->state.aptr.te);
        if (cptr) {
            cptr = Curl_copy_header_value(cptr);
            if (!cptr) return CURLE_OUT_OF_MEMORY;
        }
        data->state.aptr.te = aprintf("Connection: %s%sTE\r\n" TE_HEADER, cptr ? cptr : "", (cptr && *cptr) ? ", " : "");
        free(cptr);
        if (!data->state.aptr.te) return CURLE_OUT_OF_MEMORY;
    }
#endif
    switch(httpreq) {
        case HTTPREQ_POST_MIME: http->sendit = &data->set.mimepost; break;
        case HTTPREQ_POST_FORM:
            Curl_mime_cleanpart(&http->form);
            result = Curl_getformdata(data, &http->form, data->set.httppost, data->state.fread_func);
            if(result) return result;
            http->sendit = &http->form;
            break;
        default: http->sendit = NULL;
    }
#ifndef CURL_DISABLE_MIME
    if(http->sendit) {
        const char *cthdr = Curl_checkheaders(conn, "Content-Type");
        http->sendit->flags |= MIME_BODY_ONLY;
        if(cthdr) for(cthdr += 13; *cthdr == ' '; cthdr++);
        else if(http->sendit->kind == MIMEKIND_MULTIPART) cthdr = "multipart/form-data";
        curl_mime_headers(http->sendit, data->set.headers, 0);
        result = Curl_mime_prepare_headers(http->sendit, cthdr,NULL, MIMESTRATEGY_FORM);
        curl_mime_headers(http->sendit, NULL, 0);
        if (!result) result = Curl_mime_rewind(http->sendit);
        if (result) return result;
        http->postsize = Curl_mime_size(http->sendit);
    }
#endif
    ptr = Curl_checkheaders(conn, "Transfer-Encoding");
    if (ptr) data->req.upload_chunky = Curl_compareheader(ptr, "Transfer-Encoding:", "chunked");
    else {
        if ((conn->handler->protocol & PROTO_FAMILY_HTTP) && (((httpreq == HTTPREQ_POST_MIME || httpreq == HTTPREQ_POST_FORM) &&
           http->postsize < 0) || ((data->set.upload || httpreq == HTTPREQ_POST) && data->state.infilesize == -1))) {
            if (conn->bits.authneg);
            else if (use_http_1_1plus(data, conn)) if(conn->httpversion < 20) data->req.upload_chunky = TRUE;
            else {
                failf(data, "Chunky upload is not supported by HTTP 1.0");
                return CURLE_UPLOAD_FAILED;
            }
        } else data->req.upload_chunky = FALSE;
        if (data->req.upload_chunky) te = "Transfer-Encoding: chunked\r\n";
    }
    Curl_safefree(data->state.aptr.host);
    ptr = Curl_checkheaders(conn, "Host");
    if (ptr && (!data->state.this_is_a_follow || strcasecompare(data->state.first_host, conn->host.name))) {
    #if !defined(CURL_DISABLE_COOKIES)
        char *cookiehost = Curl_copy_header_value(ptr);
        if (!cookiehost) return CURLE_OUT_OF_MEMORY;
        if (!*cookiehost) free(cookiehost);
        else {
            if(*cookiehost == '[') {
                char *closingbracket;
                memmove(cookiehost, cookiehost + 1, strlen(cookiehost) - 1);
                closingbracket = strchr(cookiehost, ']');
                if (closingbracket) *closingbracket = 0;
            } else {
                int startsearch = 0;
                char *colon = strchr(cookiehost + startsearch, ':');
                if (colon) *colon = 0;
            }
          Curl_safefree(data->state.aptr.cookiehost);
          data->state.aptr.cookiehost = cookiehost;
        }
    #endif
        if(strcmp("Host:", ptr)) {
          data->state.aptr.host = aprintf("Host:%s\r\n", &ptr[5]);
          if(!data->state.aptr.host) return CURLE_OUT_OF_MEMORY;
        } else data->state.aptr.host = NULL;
    } else {
        if(((conn->given->protocol&CURLPROTO_HTTPS) && (conn->remote_port == PORT_HTTPS)) || ((conn->given->protocol&CURLPROTO_HTTP) &&
            (conn->remote_port == PORT_HTTP)))
            data->state.aptr.host = aprintf("Host: %s%s%s\r\n", conn->bits.ipv6_ip?"[":"", host, conn->bits.ipv6_ip?"]":"");
        else data->state.aptr.host = aprintf("Host: %s%s%s:%d\r\n", conn->bits.ipv6_ip?"[":"", host, conn->bits.ipv6_ip?"]":"", conn->remote_port);
        if(!data->state.aptr.host) return CURLE_OUT_OF_MEMORY;
    }
#ifndef CURL_DISABLE_PROXY
    if(conn->bits.httpproxy && !conn->bits.tunnel_proxy) {
        CURLUcode uc;
        CURLU *h = curl_url_dup(data->state.uh);
        if(!h) return CURLE_OUT_OF_MEMORY;
        if(conn->host.dispname != conn->host.name) {
            uc = curl_url_set(h, CURLUPART_HOST, conn->host.name, 0);
            if(uc) {
                curl_url_cleanup(h);
                return CURLE_OUT_OF_MEMORY;
            }
        }
        uc = curl_url_set(h, CURLUPART_FRAGMENT, NULL, 0);
        if (uc) {
            curl_url_cleanup(h);
            return CURLE_OUT_OF_MEMORY;
        }
        if (strcasecompare("http", data->state.up.scheme)) {
            uc = curl_url_set(h, CURLUPART_USER, NULL, 0);
            if (uc) {
                curl_url_cleanup(h);
                return CURLE_OUT_OF_MEMORY;
            }
            uc = curl_url_set(h, CURLUPART_PASSWORD, NULL, 0);
            if (uc) {
                curl_url_cleanup(h);
                return CURLE_OUT_OF_MEMORY;
            }
        }
        uc = curl_url_get(h, CURLUPART_URL, &data->set.str[STRING_TEMP_URL], 0);
        if (uc) {
            curl_url_cleanup(h);
            return CURLE_OUT_OF_MEMORY;
        }
        curl_url_cleanup(h);
        if (strcasecompare("ftp", data->state.up.scheme)) {
            if (data->set.proxy_transfer_mode) {
                char *type = strstr(path, ";type=");
                    if (type && type[6] && type[7] == 0) {
                    switch(Curl_raw_toupper(type[6])) {
                        case 'A': case 'D': case 'I': break;
                        default: type = NULL;
                    }
                }
                if (!type) {
                    char *p = ftp_typecode;
                    if (!*data->state.up.path && path[strlen(path) - 1] != '/') *p++ = '/';
                    msnprintf(p, sizeof(ftp_typecode) - 1, ";type=%c", data->set.prefer_ascii ? 'a' : 'i');
                }
            }
            if(conn->bits.user_passwd) paste_ftp_userpwd = TRUE;
        }
    }
#endif
    http->p_accept = Curl_checkheaders(conn, "Accept")?NULL:"Accept: */*\r\n";
    if ((HTTPREQ_POST == httpreq || HTTPREQ_PUT == httpreq) && data->state.resume_from) {
        if (data->state.resume_from < 0) data->state.resume_from = 0;
        if (data->state.resume_from && !data->state.this_is_a_follow) {
            int seekerr = CURL_SEEKFUNC_CANTSEEK;
            if (conn->seek_func) {
                Curl_set_in_callback(data, true);
                seekerr = conn->seek_func(conn->seek_client, data->state.resume_from, SEEK_SET);
                Curl_set_in_callback(data, false);
            }
            if (seekerr != CURL_SEEKFUNC_OK) {
                curl_off_t passed = 0;
                if (seekerr != CURL_SEEKFUNC_CANTSEEK) {
                    failf(data, "Could not seek stream");
                    return CURLE_READ_ERROR;
                }
                do {
                    size_t readthisamountnow =
                    (data->state.resume_from - passed > data->set.buffer_size) ?
                    (size_t)data->set.buffer_size :
                    curlx_sotouz(data->state.resume_from - passed);
                    size_t actuallyread = data->state.fread_func(data->state.buffer, 1, readthisamountnow, data->state.in);
                    passed += actuallyread;
                    if ((actuallyread == 0) || (actuallyread > readthisamountnow)) {
                        failf(data, "Could only read %" CURL_FORMAT_CURL_OFF_T " bytes from the input", passed);
                        return CURLE_READ_ERROR;
                    }
                } while(passed < data->state.resume_from);
            }
            if(data->state.infilesize>0) {
                data->state.infilesize -= data->state.resume_from;
                if(data->state.infilesize <= 0) {
                    failf(data, "File already completely uploaded");
                    return CURLE_PARTIAL_FILE;
                }
            }
        }
    }
    if(data->state.use_range) {
        if(((httpreq == HTTPREQ_GET) || (httpreq == HTTPREQ_HEAD)) && !Curl_checkheaders(conn, "Range")) {
            free(data->state.aptr.rangeline);
            data->state.aptr.rangeline = aprintf("Range: bytes=%s\r\n", data->state.range);
        } else if((httpreq == HTTPREQ_POST || httpreq == HTTPREQ_PUT) && !Curl_checkheaders(conn, "Content-Range")) {
            free(data->state.aptr.rangeline);
            if(data->set.set_resume_from < 0) {
                data->state.aptr.rangeline = aprintf("Content-Range: bytes 0-%" CURL_FORMAT_CURL_OFF_T "/%" CURL_FORMAT_CURL_OFF_T "\r\n",
                                                     data->state.infilesize - 1, data->state.infilesize);
            } else if(data->state.resume_from) {
                curl_off_t total_expected_size = data->state.resume_from + data->state.infilesize;
                data->state.aptr.rangeline = aprintf("Content-Range: bytes %s%" CURL_FORMAT_CURL_OFF_T "/%" CURL_FORMAT_CURL_OFF_T "\r\n",
                                                     data->state.range, total_expected_size-1, total_expected_size);
            } else {
                data->state.aptr.rangeline = aprintf("Content-Range: bytes %s/%" CURL_FORMAT_CURL_OFF_T "\r\n", data->state.range, data->state.infilesize);
            }
            if(!data->state.aptr.rangeline) return CURLE_OUT_OF_MEMORY;
        }
    }
    httpstring = get_http_string(data, conn);
    Curl_dyn_init(&req, DYN_HTTP_REQUEST);
    result = Curl_dyn_addf(&req, "%s ", request);
    if(result) return result;
    if(data->set.str[STRING_TARGET]) {
        path = data->set.str[STRING_TARGET];
        query = NULL;
    }
#ifndef CURL_DISABLE_PROXY
    if(conn->bits.httpproxy && !conn->bits.tunnel_proxy) {
        char *url = data->set.str[STRING_TEMP_URL];
        result = Curl_dyn_add(&req, url);
        Curl_safefree(data->set.str[STRING_TEMP_URL]);
    } else
#endif
    if(paste_ftp_userpwd) result = Curl_dyn_addf(&req, "ftp://%s:%s@%s", conn->user, conn->passwd, path + sizeof("ftp://") - 1);
    else {
        result = Curl_dyn_add(&req, path);
        if(result) return result;
        if(query) result = Curl_dyn_addf(&req, "?%s", query);
    }
    if(result) return result;
#ifdef USE_ALTSVC
    if(conn->bits.altused && !Curl_checkheaders(conn, "Alt-Used")) {
        altused = aprintf("Alt-Used: %s:%d\r\n", conn->conn_to_host.name, conn->conn_to_port);
        if(!altused) {
            Curl_dyn_free(&req);
            return CURLE_OUT_OF_MEMORY;
        }
    }
#endif
    result = Curl_dyn_addf(&req, "%s HTTP/%s\r\n%s%s%s%s%s%s%s%s%s%s%s%s", ftp_typecode, httpstring, (data->state.aptr.host ? data->state.aptr.host : ""),
                           data->state.aptr.proxyuserpwd ? data->state.aptr.proxyuserpwd : "", data->state.aptr.userpwd ? data->state.aptr.userpwd : "",
                           (data->state.use_range && data->state.aptr.rangeline) ? data->state.aptr.rangeline : "",
                           (data->set.str[STRING_USERAGENT] && *data->set.str[STRING_USERAGENT] && data->state.aptr.uagent) ? data->state.aptr.uagent : "",
                           http->p_accept ? http->p_accept : "", data->state.aptr.te ? data->state.aptr.te : "", (data->set.str[STRING_ENCODING] &&
                           *data->set.str[STRING_ENCODING] && data->state.aptr.accept_encoding) ? data->state.aptr.accept_encoding : "",
                           (data->change.referer && data->state.aptr.ref) ? data->state.aptr.ref : "",
                       #ifndef CURL_DISABLE_PROXY
                           (conn->bits.httpproxy && !conn->bits.tunnel_proxy && !Curl_checkProxyheaders(conn, "Proxy-Connection")) ?
                           "Proxy-Connection: Keep-Alive\r\n" : "",
                       #else
                           "",
                       #endif
                           te, altused ? altused : "");
    Curl_safefree(data->state.aptr.userpwd);
    Curl_safefree(data->state.aptr.proxyuserpwd);
    free(altused);
    if(result) return result;
    if(!(conn->handler->flags&PROTOPT_SSL) && conn->httpversion != 20 && (data->set.httpversion == CURL_HTTP_VERSION_2)) {
        result = Curl_http2_request_upgrade(&req, conn);
        if(result) return result;
    }
#if !defined(CURL_DISABLE_COOKIES)
    if(data->cookies || addcookies) {
        struct Cookie *co = NULL;
        int count = 0;
        if(data->cookies && data->state.cookie_engine) {
            Curl_share_lock(data, CURL_LOCK_DATA_COOKIE, CURL_LOCK_ACCESS_SINGLE);
            co = Curl_cookie_getlist(data->cookies, data->state.aptr.cookiehost ? data->state.aptr.cookiehost:host, data->state.up.path,
                                     (conn->handler->protocol&CURLPROTO_HTTPS) ? TRUE : FALSE);
            Curl_share_unlock(data, CURL_LOCK_DATA_COOKIE);
        }
        if(co) {
            struct Cookie *store = co;
            while(co) {
                if(co->value) {
                    if(0 == count) {
                        result = Curl_dyn_add(&req, "Cookie: ");
                        if(result) break;
                    }
                    result = Curl_dyn_addf(&req, "%s%s=%s", count ?"; " : "", co->name, co->value);
                    if(result) break;
                    count++;
                }
                co = co->next;
            }
            Curl_cookie_freelist(store);
        }
        if(addcookies && !result) {
            if(!count) result = Curl_dyn_add(&req, "Cookie: ");
            if(!result) {
                result = Curl_dyn_addf(&req, "%s%s", count?"; ":"", addcookies);
                count++;
            }
        }
        if(count && !result) result = Curl_dyn_add(&req, "\r\n");
        if(result) return result;
    }
#endif
    result = Curl_add_timecondition(conn, &req);
    if (result) return result;
    result = Curl_add_custom_headers(conn, FALSE, &req);
    if (result) return result;
    http->postdata = NULL;
    Curl_pgrsSetUploadSize(data, -1);
    switch(httpreq) {
        case HTTPREQ_PUT:
            if (conn->bits.authneg) postsize = 0;
            else postsize = data->state.infilesize;
            if ((postsize != -1) && !data->req.upload_chunky && (conn->bits.authneg || !Curl_checkheaders(conn, "Content-Length"))) {
                result = Curl_dyn_addf(&req, "Content-Length: %" CURL_FORMAT_CURL_OFF_T "\r\n", postsize);
                if (result) return result;
            }
            if (postsize != 0) {
                result = expect100(data, conn, &req);
                if (result) return result;
            }
            result = Curl_dyn_add(&req, "\r\n");
            if (result) return result;
            Curl_pgrsSetUploadSize(data, postsize);
            result = Curl_buffer_send(&req, conn, &data->info.request_size, 0, FIRSTSOCKET);
            if (result) failf(data, "Failed sending PUT request");
            else Curl_setup_transfer(data, FIRSTSOCKET, -1, TRUE, postsize?FIRSTSOCKET:-1);
            if (result) return result;
            break;
        case HTTPREQ_POST_FORM: case HTTPREQ_POST_MIME:
            if (conn->bits.authneg) {
                result = Curl_dyn_add(&req, "Content-Length: 0\r\n\r\n");
                if (result) return result;
                result = Curl_buffer_send(&req, conn, &data->info.request_size, 0, FIRSTSOCKET);
                if(result) failf(data, "Failed sending POST request");
                else Curl_setup_transfer(data, FIRSTSOCKET, -1, TRUE, -1);
                break;
            }
            data->state.infilesize = postsize = http->postsize;
            if(postsize != -1 && !data->req.upload_chunky && (conn->bits.authneg || !Curl_checkheaders(conn, "Content-Length"))) {
              result = Curl_dyn_addf(&req,"Content-Length: %" CURL_FORMAT_CURL_OFF_T "\r\n", postsize);
              if(result) return result;
            }
        #ifndef CURL_DISABLE_MIME
            {
                struct curl_slist *hdr;
                for (hdr = http->sendit->curlheaders; hdr; hdr = hdr->next) {
                    result = Curl_dyn_addf(&req, "%s\r\n", hdr->data);
                    if (result) return result;
                }
            }
        #endif
            ptr = Curl_checkheaders(conn, "Expect");
            if (ptr) data->state.expect100header = Curl_compareheader(ptr, "Expect:", "100-continue");
            else if (postsize > EXPECT_100_THRESHOLD || postsize < 0) {
                result = expect100(data, conn, &req);
                if (result) return result;
            } else data->state.expect100header = FALSE;
            result = Curl_dyn_add(&req, "\r\n");
            if (result) return result;
            Curl_pgrsSetUploadSize(data, postsize);
            data->state.fread_func = (curl_read_callback) Curl_mime_read;
            data->state.in = (void *) http->sendit;
            http->sending = HTTPSEND_BODY;
            result = Curl_buffer_send(&req, conn, &data->info.request_size, 0, FIRSTSOCKET);
            if (result) failf(data, "Failed sending POST request");
            else Curl_setup_transfer(data, FIRSTSOCKET, -1, TRUE, postsize?FIRSTSOCKET:-1);
            if (result) return result;
            break;
        case HTTPREQ_POST:
            if (conn->bits.authneg) postsize = 0;
            else postsize = data->state.infilesize;
            if ((postsize != -1) && !data->req.upload_chunky && (conn->bits.authneg || !Curl_checkheaders(conn, "Content-Length"))) {
                result = Curl_dyn_addf(&req, "Content-Length: %" CURL_FORMAT_CURL_OFF_T "\r\n", postsize);
                if(result) return result;
            }
            if (!Curl_checkheaders(conn, "Content-Type")) {
                result = Curl_dyn_add(&req, "Content-Type: application/x-www-form-urlencoded\r\n");
                if(result) return result;
            }
            ptr = Curl_checkheaders(conn, "Expect");
            if (ptr) data->state.expect100header = Curl_compareheader(ptr, "Expect:", "100-continue");
            else if (postsize > EXPECT_100_THRESHOLD || postsize < 0) {
                result = expect100(data, conn, &req);
                if (result) return result;
            } else data->state.expect100header = FALSE;
            if(data->set.postfields) {
                if (conn->httpversion != 20 && !data->state.expect100header && (postsize < MAX_INITIAL_POST_SIZE)) {
                    result = Curl_dyn_add(&req, "\r\n");
                    if (result) return result;
                    if (!data->req.upload_chunky) {
                        result = Curl_dyn_addn(&req, data->set.postfields, (size_t)postsize);
                        included_body = postsize;
                    } else {
                        if (postsize) {
                            result = Curl_dyn_addf(&req, "%x\r\n", (int)postsize);
                            if (!result) {
                                result = Curl_dyn_addn(&req, data->set.postfields, (size_t)postsize);
                                if(!result) result = Curl_dyn_add(&req, "\r\n");
                                included_body = postsize + 2;
                            }
                        }
                        if (!result) result = Curl_dyn_add(&req, "\x30\x0d\x0a\x0d\x0a");
                        included_body += 5;
                    }
                    if (result) return result;
                    Curl_pgrsSetUploadSize(data, postsize);
                } else {
                    http->postsize = postsize;
                    http->postdata = data->set.postfields;
                    http->sending = HTTPSEND_BODY;
                    data->state.fread_func = (curl_read_callback)readmoredata;
                    data->state.in = (void *)conn;
                    Curl_pgrsSetUploadSize(data, http->postsize);
                    result = Curl_dyn_add(&req, "\r\n");
                    if(result) return result;
                }
            } else {
                result = Curl_dyn_add(&req, "\r\n");
                if (result) return result;
                if (data->req.upload_chunky && conn->bits.authneg) {
                    result = Curl_dyn_add(&req, (char*)"\x30\x0d\x0a\x0d\x0a");
                    if (result) return result;
                } else if (data->state.infilesize) {
                    Curl_pgrsSetUploadSize(data, postsize?postsize:-1);
                    if (!conn->bits.authneg) {
                        http->postdata = (char *)&http->postdata;
                        http->postsize = postsize;
                    }
                }
            }
            result = Curl_buffer_send(&req, conn, &data->info.request_size, (size_t)included_body, FIRSTSOCKET);
            if (result) failf(data, "Failed sending HTTP POST request");
            else Curl_setup_transfer(data, FIRSTSOCKET, -1, TRUE, http->postdata?FIRSTSOCKET:-1);
            break;
        default:
            result = Curl_dyn_add(&req, "\r\n");
            if (result) return result;
            result = Curl_buffer_send(&req, conn, &data->info.request_size, 0, FIRSTSOCKET);
            if (result) failf(data, "Failed sending HTTP request");
            else Curl_setup_transfer(data, FIRSTSOCKET, -1, TRUE, -1);
    }
    if (result) return result;
    if (!postsize && (http->sending != HTTPSEND_REQUEST)) data->req.upload_done = TRUE;
    if (data->req.writebytecount) {
        Curl_pgrsSetUploadCounter(data, data->req.writebytecount);
        if (Curl_pgrsUpdate(conn)) result = CURLE_ABORTED_BY_CALLBACK;
        if (data->req.writebytecount >= postsize) {
            infof(data, "upload completely sent off: %" CURL_FORMAT_CURL_OFF_T " out of %" CURL_FORMAT_CURL_OFF_T " bytes\n", data->req.writebytecount, postsize);
            data->req.upload_done = TRUE;
            data->req.keepon &= ~KEEP_SEND;
            data->req.exp100 = EXP100_SEND_DATA;
            Curl_expire_done(data, EXPIRE_100_TIMEOUT);
        }
    }
    if((conn->httpversion == 20) && data->req.upload_chunky) data->req.upload_chunky = FALSE;
    return result;
}
typedef enum {
    STATUS_UNKNOWN,
    STATUS_DONE,
    STATUS_BAD
} statusline;
static bool checkprefixmax(const char *prefix, const char *buffer, size_t len) {
    size_t ch = CURLMIN(strlen(prefix), len);
    return curl_strnequal(prefix, buffer, ch);
}
static statusline checkhttpprefix(struct Curl_easy *data, const char *s, size_t len) {
    struct curl_slist *head = data->set.http200aliases;
    statusline rc = STATUS_BAD;
    statusline onmatch = len >= 5? STATUS_DONE : STATUS_UNKNOWN;
#ifdef CURL_DOES_CONVERSIONS
    char *scratch = strdup(s);
    if(NULL == scratch) {
        failf(data, "Failed to allocate memory for conversion!");
        return FALSE;
    }
    if(CURLE_OK != Curl_convert_from_network(data, scratch, strlen(s) + 1)) {
        free(scratch);
        return FALSE;
    }
    s = scratch;
#endif
    while(head) {
        if(checkprefixmax(head->data, s, len)) {
            rc = onmatch;
            break;
        }
        head = head->next;
    }
    if((rc != STATUS_DONE) && (checkprefixmax("HTTP/", s, len))) rc = onmatch;
#ifdef CURL_DOES_CONVERSIONS
    free(scratch);
#endif
    return rc;
}
#ifndef CURL_DISABLE_RTSP
static statusline checkrtspprefix(struct Curl_easy *data, const char *s, size_t len) {
    statusline result = STATUS_BAD;
    statusline onmatch = len >= 5 ? STATUS_DONE : STATUS_UNKNOWN;
#ifdef CURL_DOES_CONVERSIONS
    char *scratch = strdup(s);
    if (NULL == scratch) {
        failf(data, "Failed to allocate memory for conversion!");
        return FALSE;
    }
    if (CURLE_OK != Curl_convert_from_network(data, scratch, strlen(s) + 1)) result = FALSE;
    else if (checkprefixmax("RTSP/", scratch, len)) result = onmatch;
    free(scratch);
#else
    (void)data;
    if (checkprefixmax("RTSP/", s, len)) result = onmatch;
#endif
    return result;
}
#endif
static statusline checkprotoprefix(struct Curl_easy *data, struct connectdata *conn, const char *s, size_t len) {
#ifndef CURL_DISABLE_RTSP
    if(conn->handler->protocol & CURLPROTO_RTSP) return checkrtspprefix(data, s, len);
#else
    (void)conn;
#endif
    return checkhttpprefix(data, s, len);
}
static void print_http_error(struct Curl_easy *data) {
    struct SingleRequest *k = &data->req;
    char *beg = Curl_dyn_ptr(&data->state.headerb);
    if(!strncmp(beg, "HTTP", 4)) {
        beg = strchr(beg, ' ');
        if(beg && *++beg) {
            char end_char = '\r';
            char *end = strchr(beg, end_char);
            if(!end) {
                end_char = '\n';
                end = strchr(beg, end_char);
            }
            if(end) {
                *end = '\0';
                failf(data, "The requested URL returned error: %s", beg);
                *end = end_char;
                return;
            }
        }
    }
    failf(data, "The requested URL returned error: %d", k->httpcode);
}
CURLcode Curl_http_readwrite_headers(struct Curl_easy *data, struct connectdata *conn, ssize_t *nread, bool *stop_reading) {
    CURLcode result;
    struct SingleRequest *k = &data->req;
    ssize_t onread = *nread;
    char *ostr = k->str;
    char *headp;
    char *str_start;
    char *end_ptr;
    do {
        size_t rest_length;
        size_t full_length;
        int writetype;
        str_start = k->str;
        end_ptr = memchr(str_start, 0x0a, *nread);
        if (!end_ptr) {
            result = Curl_dyn_addn(&data->state.headerb, str_start, *nread);
            if (result) return result;
            if (!k->headerline) {
                statusline st = checkprotoprefix(data, conn, Curl_dyn_ptr(&data->state.headerb), Curl_dyn_len(&data->state.headerb));
                if (st == STATUS_BAD) {
                    k->header = FALSE;
                    k->badheader = HEADER_ALLBAD;
                    streamclose(conn, "bad HTTP: No end-of-message indicator");
                    if (!data->set.http09_allowed) {
                        failf(data, "Received HTTP/0.9 when not allowed\n");
                        return CURLE_UNSUPPORTED_PROTOCOL;
                    }
                    break;
                }
            }
            break;
        }
        rest_length = (end_ptr - k->str) + 1;
        *nread -= (ssize_t)rest_length;
        k->str = end_ptr + 1;
        full_length = k->str - str_start;
        result = Curl_dyn_addn(&data->state.headerb, str_start, full_length);
        if (result) return result;
        if (!k->headerline) {
            statusline st = checkprotoprefix(data, conn, Curl_dyn_ptr(&data->state.headerb), Curl_dyn_len(&data->state.headerb));
            if (st == STATUS_BAD) {
                streamclose(conn, "bad HTTP: No end-of-message indicator");
                if (!data->set.http09_allowed) {
                    failf(data, "Received HTTP/0.9 when not allowed\n");
                    return CURLE_UNSUPPORTED_PROTOCOL;
                }
                k->header = FALSE;
                if (*nread) k->badheader = HEADER_PARTHEADER;
                else {
                    k->badheader = HEADER_ALLBAD;
                    *nread = onread;
                    k->str = ostr;
                    return CURLE_OK;
                }
                break;
            }
        }
        headp = Curl_dyn_ptr(&data->state.headerb);
        if ((0x0a == *headp) || (0x0d == *headp)) {
            size_t headerlen;
        #ifdef CURL_DOES_CONVERSIONS
            if (0x0d == *headp) {
                *headp = '\r';
                headp++;
            }
            if (0x0a == *headp) {
                *headp = '\n';
                headp++;
            }
        #else
            if ('\r' == *headp) headp++;
            if ('\n' == *headp) headp++;
        #endif
            if (100 <= k->httpcode && 199 >= k->httpcode) {
                switch(k->httpcode) {
                    case 100:
                        k->header = TRUE;
                        k->headerline = 0;
                        if (k->exp100 > EXP100_SEND_DATA) {
                            k->exp100 = EXP100_SEND_DATA;
                            k->keepon |= KEEP_SEND;
                            Curl_expire_done(data, EXPIRE_100_TIMEOUT);
                        }
                        break;
                    case 101:
                        if (k->upgr101 == UPGR101_REQUESTED) {
                            infof(data, "Received 101\n");
                            k->upgr101 = UPGR101_RECEIVED;
                            k->header = TRUE;
                            k->headerline = 0;
                            result = Curl_http2_switched(conn, k->str, *nread);
                            if (result) return result;
                            *nread = 0;
                        } else  k->header = FALSE;
                        break;
                    default:
                        k->header = TRUE;
                        k->headerline = 0;
                        break;
                }
            } else {
                k->header = FALSE;
                if ((k->size == -1) && !k->chunk && !conn->bits.close && (conn->httpversion == 11) && !(conn->handler->protocol & CURLPROTO_RTSP) &&
                   data->state.httpreq != HTTPREQ_HEAD) {
                    infof(data, "no chunk, no close, no size. Assume close to " "signal end\n");
                    streamclose(conn, "HTTP: No end-of-message indicator");
                }
            }
        #if defined(USE_NTLM)
            if (conn->bits.close && (((data->req.httpcode == 401) && (conn->http_ntlm_state == NTLMSTATE_TYPE2)) ||
               ((data->req.httpcode == 407) && (conn->proxy_ntlm_state == NTLMSTATE_TYPE2)))) {
                infof(data, "Connection closure while negotiating auth (HTTP 1.0?)\n");
                data->state.authproblem = TRUE;
            }
        #endif
        #if defined(USE_SPNEGO)
            if (conn->bits.close && (((data->req.httpcode == 401) && (conn->http_negotiate_state == GSS_AUTHRECV)) ||
               ((data->req.httpcode == 407) && (conn->proxy_negotiate_state == GSS_AUTHRECV)))) {
                infof(data, "Connection closure while negotiating auth (HTTP 1.0?)\n");
                data->state.authproblem = TRUE;
            }
            if ((conn->http_negotiate_state == GSS_AUTHDONE) && (data->req.httpcode != 401)) conn->http_negotiate_state = GSS_AUTHSUCC;
            if ((conn->proxy_negotiate_state == GSS_AUTHDONE) && (data->req.httpcode != 407)) conn->proxy_negotiate_state = GSS_AUTHSUCC;
        #endif
            if (http_should_fail(conn)) {
                failf(data, "The requested URL returned error: %d", k->httpcode);
                return CURLE_HTTP_RETURNED_ERROR;
            }
            writetype = CLIENTWRITE_HEADER;
            if (data->set.include_header) writetype |= CLIENTWRITE_BODY;
            headerlen = Curl_dyn_len(&data->state.headerb);
            result = Curl_client_write(conn, writetype, Curl_dyn_ptr(&data->state.headerb), headerlen);
            if (result) return result;
            data->info.header_size += (long)headerlen;
            data->req.headerbytecount += (long)headerlen;
            data->req.deductheadercount = (100 <= k->httpcode && 199 >= k->httpcode) ? data->req.headerbytecount : 0;
            result = Curl_http_auth_act(conn);
            if (result) return result;
            if (k->httpcode >= 300) {
                if ((!conn->bits.authneg) && !conn->bits.close && !conn->bits.rewindaftersend) {
                    switch(data->state.httpreq) {
                        case HTTPREQ_PUT: case HTTPREQ_POST: case HTTPREQ_POST_FORM: case HTTPREQ_POST_MIME:
                            Curl_expire_done(data, EXPIRE_100_TIMEOUT);
                            if (!k->upload_done) {
                                if ((k->httpcode == 417) && data->state.expect100header) {
                                    infof(data, "Got 417 while waiting for a 100\n");
                                    data->state.disableexpect = TRUE;
                                    DEBUGASSERT(!data->req.newurl);
                                    data->req.newurl = strdup(conn->data->change.url);
                                    Curl_done_sending(conn, k);
                                } else if (data->set.http_keep_sending_on_error) {
                                    infof(data, "HTTP error before end of send, keep sending\n");
                                    if (k->exp100 > EXP100_SEND_DATA) {
                                        k->exp100 = EXP100_SEND_DATA;
                                        k->keepon |= KEEP_SEND;
                                    }
                                } else {
                                    infof(data, "HTTP error before end of send, stop sending\n");
                                    streamclose(conn, "Stop sending data before everything sent");
                                    result = Curl_done_sending(conn, k);
                                    if (result) return result;
                                    k->upload_done = TRUE;
                                    if (data->state.expect100header) k->exp100 = EXP100_FAILED;
                                }
                            }
                            break;
                    }
                }
                if (conn->bits.rewindaftersend) {
                    infof(data, "Keep sending data to get tossed away!\n");
                    k->keepon |= KEEP_SEND;
                }
            }
            if (!k->header) {
                if (data->set.opt_no_body) *stop_reading = TRUE;
            #ifndef CURL_DISABLE_RTSP
                else if ((conn->handler->protocol & CURLPROTO_RTSP) && (data->set.rtspreq == RTSPREQ_DESCRIBE) && (k->size <= -1)) *stop_reading = TRUE;
            #endif
                else {
                    if (k->chunk) k->maxdownload = k->size = -1;
                }
                if (-1 != k->size) {
                    Curl_pgrsSetDownloadSize(data, k->size);
                    k->maxdownload = k->size;
                }
                if (0 == k->maxdownload
                #if defined(USE_NGHTTP2)
                   && !((conn->handler->protocol & PROTO_FAMILY_HTTP) && conn->httpversion == 20)
                #endif
                   ) *stop_reading = TRUE;
                if (*stop_reading) k->keepon &= ~KEEP_RECV;
                if (data->set.verbose) Curl_debug(data, CURLINFO_HEADER_IN, str_start, headerlen);
                break;
            }
            Curl_dyn_reset(&data->state.headerb);
            continue;
        }
        if (!k->headerline++) {
            int httpversion_major;
            int rtspversion_major;
            int nc = 0;
        #ifdef CURL_DOES_CONVERSIONS
            #define HEADER1 scratch
            #define SCRATCHSIZE 21
            CURLcode res;
            char scratch[SCRATCHSIZE + 1];
            strncpy(&scratch[0], headp, SCRATCHSIZE);
            scratch[SCRATCHSIZE] = 0;
            res = Curl_convert_from_network(data, &scratch[0], SCRATCHSIZE);
            if(res) return res;
        #else
            #define HEADER1 headp
        #endif
            if (conn->handler->protocol & PROTO_FAMILY_HTTP) {
                char separator;
                char twoorthree[2];
                nc = sscanf(HEADER1," HTTP/%1d.%1d%c%3d", &httpversion_major, &conn->httpversion, &separator, &k->httpcode);
                if (nc == 1 && httpversion_major >= 2 && 2 == sscanf(HEADER1, " HTTP/%1[23] %d", twoorthree, &k->httpcode)) {
                    conn->httpversion = 0;
                    nc = 4;
                    separator = ' ';
                }
                if ((nc == 4) && (' ' == separator)) {
                    conn->httpversion += 10 * httpversion_major;
                    if (k->upgr101 == UPGR101_RECEIVED) {
                        if (conn->httpversion != 20) infof(data, "Lying server, not serving HTTP/2\n");
                    }
                    if (conn->httpversion < 20) {
                        conn->bundle->multiuse = BUNDLE_NO_MULTIUSE;
                        infof(data, "Mark bundle as not supporting multiuse\n");
                    }
                } else if (!nc) {
                    nc = sscanf(HEADER1, " HTTP %3d", &k->httpcode);
                    conn->httpversion = 10;
                    if (!nc) {
                        statusline check = checkhttpprefix(data, Curl_dyn_ptr(&data->state.headerb), Curl_dyn_len(&data->state.headerb));
                        if (check == STATUS_DONE) {
                            nc = 1;
                            k->httpcode = 200;
                            conn->httpversion = 10;
                        }
                    }
                } else {
                    failf(data, "Unsupported HTTP version in response");
                    return CURLE_UNSUPPORTED_PROTOCOL;
                }
            } else if (conn->handler->protocol & CURLPROTO_RTSP) {
                char separator;
                nc = sscanf(HEADER1," RTSP/%1d.%1d%c%3d", &rtspversion_major, &conn->rtspversion, &separator, &k->httpcode);
                if ((nc == 4) && (' ' == separator)) {
                    conn->rtspversion += 10 * rtspversion_major;
                    conn->httpversion = 11;
                } else nc = 0;
            }
            if (nc) {
                data->info.httpcode = k->httpcode;
                data->info.httpversion = conn->httpversion;
                if (!data->state.httpversion || data->state.httpversion > conn->httpversion) data->state.httpversion = conn->httpversion;
                if (data->state.resume_from && data->state.httpreq == HTTPREQ_GET && k->httpcode == 416) k->ignorebody = TRUE;
                else if (data->set.http_fail_on_error && (k->httpcode >= 400) && ((k->httpcode != 401) || !conn->bits.user_passwd)
                    #ifndef CURL_DISABLE_PROXY
                        && ((k->httpcode != 407) || !conn->bits.proxy_user_passwd)
                    #endif
                       ) {
                    print_http_error(data);
                    return CURLE_HTTP_RETURNED_ERROR;
                }
                if (conn->httpversion == 10) {
                    infof(data, "HTTP 1.0, assume close after body\n");
                    connclose(conn, "HTTP/1.0 close after body");
                } else if (conn->httpversion == 20 || (k->upgr101 == UPGR101_REQUESTED && k->httpcode == 101)) {
                    DEBUGF(infof(data, "HTTP/2 found, allow multiplexing\n"));
                    conn->bundle->multiuse = BUNDLE_MULTIPLEX;
                } else if (conn->httpversion >= 11 && !conn->bits.close) DEBUGF(infof(data, "HTTP 1.1 or later with persistent connection\n"));
                k->http_bodyless = k->httpcode >= 100 && k->httpcode < 200;
                switch(k->httpcode) {
                    case 304: if (data->set.timecondition) data->info.timecond = TRUE;
                    case 204:
                        k->size = 0;
                        k->maxdownload = 0;
                        k->http_bodyless = TRUE;
                }
            } else {
                k->header = FALSE;
                break;
            }
        }
        result = Curl_convert_from_network(data, headp, strlen(headp));
        if (result) return result;
        if (!k->http_bodyless && !data->set.ignorecl && checkprefix("Content-Length:", headp)) {
            curl_off_t contentlength;
            CURLofft offt = curlx_strtoofft(headp + 15, NULL, 10, &contentlength);
            if (offt == CURL_OFFT_OK) {
            if (data->set.max_filesize && contentlength > data->set.max_filesize) {
                failf(data, "Maximum file size exceeded");
                return CURLE_FILESIZE_EXCEEDED;
            }
            k->size = contentlength;
            k->maxdownload = k->size;
            Curl_pgrsSetDownloadSize(data, k->size);
            } else if (offt == CURL_OFFT_FLOW) {
                if(data->set.max_filesize) {
                    failf(data, "Maximum file size exceeded");
                    return CURLE_FILESIZE_EXCEEDED;
                }
                streamclose(conn, "overflow content-length");
                infof(data, "Overflow Content-Length: value!\n");
            } else {
                failf(data, "Invalid Content-Length: value");
                return CURLE_WEIRD_SERVER_REPLY;
            }
        } else if (checkprefix("Content-Type:", headp)) {
            char *contenttype = Curl_copy_header_value(headp);
            if (!contenttype) return CURLE_OUT_OF_MEMORY;
            if (!*contenttype) free(contenttype);
            else {
                Curl_safefree(data->info.contenttype);
                data->info.contenttype = contenttype;
            }
        }
    #ifndef CURL_DISABLE_PROXY
        else if ((conn->httpversion == 10) && conn->bits.httpproxy && Curl_compareheader(headp, "Proxy-Connection:", "keep-alive")) {
            connkeep(conn, "Proxy-Connection keep-alive"); /* don't close */
            infof(data, "HTTP/1.0 proxy connection set to keep alive!\n");
        } else if ((conn->httpversion == 11) && conn->bits.httpproxy && Curl_compareheader(headp, "Proxy-Connection:", "close")) {
            connclose(conn, "Proxy-Connection: asked to close after done");
            infof(data, "HTTP/1.1 proxy connection set close!\n");
        }
    #endif
        else if ((conn->httpversion == 10) && Curl_compareheader(headp, "Connection:", "keep-alive")) {
            connkeep(conn, "Connection keep-alive");
            infof(data, "HTTP/1.0 connection set to keep alive!\n");
        } else if (Curl_compareheader(headp, "Connection:", "close")) streamclose(conn, "Connection: close used");
        else if (!k->http_bodyless && checkprefix("Transfer-Encoding:", headp)) {
            result = Curl_build_unencoding_stack(conn, headp + 18, TRUE);
            if (result) return result;
        } else if (!k->http_bodyless && checkprefix("Content-Encoding:", headp) && data->set.str[STRING_ENCODING]) {
            result = Curl_build_unencoding_stack(conn, headp + 17, FALSE);
            if(result) return result;
        } else if (checkprefix("Retry-After:", headp)) {
            curl_off_t retry_after = 0;
            time_t date = Curl_getdate_capped(&headp[12]);
            if (-1 == date) (void)curlx_strtoofft(&headp[12], NULL, 10, &retry_after);
            else retry_after = date - time(NULL);
            data->info.retry_after = retry_after;
        } else if (!k->http_bodyless && checkprefix("Content-Range:", headp)) {
            char *ptr = headp + 14;
            while(*ptr && !ISDIGIT(*ptr) && *ptr != '*') ptr++;
            if(ISDIGIT(*ptr)) {
                if (!curlx_strtoofft(ptr, NULL, 10, &k->offset)) {
                    if (data->state.resume_from == k->offset) k->content_range = TRUE;
                }
            } else data->state.resume_from = 0;
        }
    #if !defined(CURL_DISABLE_COOKIES)
        else if (data->cookies && data->state.cookie_engine && checkprefix("Set-Cookie:", headp)) {
            Curl_share_lock(data, CURL_LOCK_DATA_COOKIE, CURL_LOCK_ACCESS_SINGLE);
            Curl_cookie_add(data, data->cookies, TRUE, FALSE, headp + 11, data->state.aptr.cookiehost? data->state.aptr.cookiehost :
                            conn->host.name, data->state.up.path, (conn->handler->protocol&CURLPROTO_HTTPS) ? TRUE : FALSE);
            Curl_share_unlock(data, CURL_LOCK_DATA_COOKIE);
        }
    #endif
        else if (!k->http_bodyless && checkprefix("Last-Modified:", headp) && (data->set.timecondition || data->set.get_filetime) ) {
            k->timeofdoc = Curl_getdate_capped(headp + strlen("Last-Modified:"));
            if (data->set.get_filetime) data->info.filetime = k->timeofdoc;
        } else if ((checkprefix("WWW-Authenticate:", headp) && (401 == k->httpcode)) || (checkprefix("Proxy-authenticate:", headp) && (407 == k->httpcode))) {
            bool proxy = (k->httpcode == 407) ? TRUE : FALSE;
            char *auth = Curl_copy_header_value(headp);
            if (!auth) return CURLE_OUT_OF_MEMORY;
            result = Curl_http_input_auth(conn, proxy, auth);
            free(auth);
            if ( result) return result;
        }
    #ifdef USE_SPNEGO
        else if (checkprefix("Persistent-Auth", headp)) {
            struct negotiatedata *negdata = &conn->negotiate;
            struct auth *authp = &data->state.authhost;
            if (authp->picked == CURLAUTH_NEGOTIATE) {
                char *persistentauth = Curl_copy_header_value(headp);
                if (!persistentauth) return CURLE_OUT_OF_MEMORY;
                negdata->noauthpersist = checkprefix("false", persistentauth) ? TRUE : FALSE;
                negdata->havenoauthpersist = TRUE;
                infof(data, "Negotiate: noauthpersist -> %d, header part: %s", negdata->noauthpersist, persistentauth);
                free(persistentauth);
            }
        }
    #endif
        else if ((k->httpcode >= 300 && k->httpcode < 400) && checkprefix("Location:", headp) && !data->req.location) {
            char *location = Curl_copy_header_value(headp);
            if (!location) return CURLE_OUT_OF_MEMORY;
            if (!*location) free(location);
            else {
                data->req.location = location;
                if (data->set.http_follow_location) {
                    DEBUGASSERT(!data->req.newurl);
                    data->req.newurl = strdup(data->req.location);
                    if (!data->req.newurl) return CURLE_OUT_OF_MEMORY;
                    result = http_perhapsrewind(conn);
                    if (result) return result;
                }
            }
        }
    #ifdef USE_ALTSVC
        else if(data->asi && checkprefix("Alt-Svc:", headp) && ((conn->handler->flags & PROTOPT_SSL) ||
            #ifdef CURLDEBUG
                getenv("CURL_ALTSVC_HTTP")
            #else
                0
            #endif
                )) {
            enum alpnid id = (conn->httpversion == 20) ? ALPN_h2 : ALPN_h1;
            result = Curl_altsvc_parse(data, data->asi, &headp[strlen("Alt-Svc:")], id, conn->host.name, curlx_uitous(conn->remote_port));
            if(result) return result;
        }
    #endif
        else if (conn->handler->protocol & CURLPROTO_RTSP) {
            result = Curl_rtsp_parseheader(conn, headp);
            if (result) return result;
        }
        writetype = CLIENTWRITE_HEADER;
        if (data->set.include_header) writetype |= CLIENTWRITE_BODY;
        if (data->set.verbose) Curl_debug(data, CURLINFO_HEADER_IN, headp, Curl_dyn_len(&data->state.headerb));
        result = Curl_client_write(conn, writetype, headp, Curl_dyn_len(&data->state.headerb));
        if (result) return result;
        data->info.header_size += Curl_dyn_len(&data->state.headerb);
        data->req.headerbytecount += Curl_dyn_len(&data->state.headerb);
        Curl_dyn_reset(&data->state.headerb);
    } while(*k->str);
    return CURLE_OK;
}
#endif