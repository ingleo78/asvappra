#include "curl_setup.h"
#include "lber.h"

#if defined(HAVE_GSSAPI) && !defined(CURL_DISABLE_FTP)
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#include "urldata.h"
#include "curl_base64.h"
#include "ftp.h"
#include "curl_gssapi.h"
#include "sendf.h"
#include "curl_sec.h"
#include "warnless.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

static int krb5_init(void *app_data) {
    gss_ctx_id_t *context = app_data;
    *context = GSS_C_NO_CONTEXT;
    return 0;
}
static int krb5_check_prot(void *app_data, int level) {
    (void)app_data;
    if(level == PROT_CONFIDENTIAL) return -1;
    return 0;
}
static int
krb5_decode(void *app_data, void *buf, int len, int level UNUSED_PARAM, struct connectdata *conn UNUSED_PARAM) {
    gss_ctx_id_t *context = app_data;
    OM_uint32 maj, min;
    gss_buffer_desc enc, dec;
    (void)level;
    (void)conn;
    enc.value = buf;
    enc.length = len;
    maj = gss_unwrap(&min, *context, &enc, &dec, NULL, NULL);
    if(maj != GSS_S_COMPLETE) {
        if(len >= 4) strcpy(buf, "599 ");
        return -1;
    }
    memcpy(buf, dec.value, dec.length);
    len = curlx_uztosi(dec.length);
    gss_release_buffer(&min, &dec);
    return len;
}
static int krb5_overhead(void *app_data, int level, int len) {
    (void)app_data;
    (void)level;
    (void)len;
    return 0;
}
static int krb5_encode(void *app_data, const void *from, int length, int level, void **to) {
    gss_ctx_id_t *context = app_data;
    gss_buffer_desc dec, enc;
    OM_uint32 maj, min;
    int state;
    int len;
    dec.value = (void *)from;
    dec.length = length;
    maj = gss_wrap(&min, *context, level == PROT_PRIVATE, GSS_C_QOP_DEFAULT, &dec, &state, &enc);
    if(maj != GSS_S_COMPLETE) return -1;
    *to = malloc(enc.length);
    if(!*to) return -1;
    memcpy(*to, enc.value, enc.length);
    len = curlx_uztosi(enc.length);
    gss_release_buffer(&min, &enc);
    return len;
}
static int krb5_auth(void *app_data, struct connectdata *conn) {
    int ret = AUTH_OK;
    char *p;
    const char *host = conn->host.name;
    ssize_t nread;
    curl_socklen_t l = sizeof(conn->local_addr);
    struct Curl_easy *data = conn->data;
    CURLcode result;
    const char *service = data->set.str[STRING_SERVICE_NAME] ? data->set.str[STRING_SERVICE_NAME] : "ftp";
    const char *srv_host = "host";
    gss_buffer_desc input_buffer, output_buffer, _gssresp, *gssresp;
    OM_uint32 maj, min;
    gss_name_t gssname;
    gss_ctx_id_t *context = app_data;
    struct gss_channel_bindings_struct chan;
    size_t base64_sz = 0;
    struct sockaddr_in **remote_addr = (struct sockaddr_in **)&conn->ip_addr->ai_addr;
    char *stringp;
    if(getsockname(conn->sock[FIRSTSOCKET], (struct sockaddr *)&conn->local_addr, &l) < 0) perror("getsockname()");
    chan.initiator_addrtype = GSS_C_AF_INET;
    chan.initiator_address.length = l - 4;
    chan.initiator_address.value = &conn->local_addr.sin_addr.s_addr;
    chan.acceptor_addrtype = GSS_C_AF_INET;
    chan.acceptor_address.length = l - 4;
    chan.acceptor_address.value = &(*remote_addr)->sin_addr.s_addr;
    chan.application_data.length = 0;
    chan.application_data.value = NULL;
    for( ; ; ) {
        if(service == srv_host) {
            result = Curl_ftpsend(conn, "AUTH GSSAPI");
            if(result) return -2;
            if(Curl_GetFTPResponse(&nread, conn, NULL)) return -1;
            if(data->state.buffer[0] != '3') return -1;
        }
        stringp = aprintf("%s@%s", service, host);
        if(!stringp) return -2;
        input_buffer.value = stringp;
        input_buffer.length = strlen(stringp);
        maj = gss_import_name(&min, &input_buffer, GSS_C_NT_HOSTBASED_SERVICE, &gssname);
        free(stringp);
        if(maj != GSS_S_COMPLETE) {
            gss_release_name(&min, &gssname);
            if(service == srv_host) {
                failf(data, "Error importing service name %s@%s", service, host);
                return AUTH_ERROR;
            }
            service = srv_host;
            continue;
        }
        gss_display_name(&min, gssname, &output_buffer, NULL);
        Curl_infof(data, "Trying against %s\n", output_buffer.value);
        gssresp = GSS_C_NO_BUFFER;
        *context = GSS_C_NO_CONTEXT;
        do {
            gss_release_buffer(&min, &output_buffer);
            ret = AUTH_OK;
            maj = Curl_gss_init_sec_context(data, &min, context, gssname, &Curl_krb5_mech_oid, &chan, gssresp, &output_buffer, TRUE, NULL);
            if(gssresp) {
                free(_gssresp.value);
                gssresp = NULL;
            }
            if(GSS_ERROR(maj)) {
                Curl_infof(data, "Error creating security context\n");
                ret = AUTH_ERROR;
                break;
            }
            if(output_buffer.length != 0) {
                char *cmd;
                result = Curl_base64_encode(data, (char *)output_buffer.value, output_buffer.length, &p, &base64_sz);
                if(result) {
                    Curl_infof(data, "base64-encoding: %s\n", curl_easy_strerror(result));
                    ret = AUTH_ERROR;
                    break;
                }
                cmd = aprintf("ADAT %s", p);
                if(cmd) result = Curl_ftpsend(conn, cmd);
                else result = CURLE_OUT_OF_MEMORY;
                free(p);
                free(cmd);
                if(result) {
                    ret = -2;
                    break;
                }
                if(Curl_GetFTPResponse(&nread, conn, NULL)) {
                    ret = -1;
                    break;
                }
                if(data->state.buffer[0] != '2' && data->state.buffer[0] != '3') {
                    Curl_infof(data, "Server didn't accept auth data\n");
                    ret = AUTH_ERROR;
                    break;
                }
                _gssresp.value = NULL;
                p = data->state.buffer + 4;
                p = strstr(p, "ADAT=");
                if(p) {
                    result = Curl_base64_decode(p + 5, (unsigned char **)&_gssresp.value, &_gssresp.length);
                    if(result) {
                        failf(data, "base64-decoding: %s", curl_easy_strerror(result));
                        ret = AUTH_CONTINUE;
                        break;
                    }
                }
                gssresp = &_gssresp;
            }
        } while(maj == GSS_S_CONTINUE_NEEDED);
        gss_release_name(&min, &gssname);
        gss_release_buffer(&min, &output_buffer);
        if(gssresp) free(_gssresp.value);
        if(ret == AUTH_OK || service == srv_host) return ret;
        service = srv_host;
    }
    return ret;
}
static void krb5_end(void *app_data) {
    OM_uint32 min;
    gss_ctx_id_t *context = app_data;
    if(*context != GSS_C_NO_CONTEXT) {
        OM_uint32 maj = gss_delete_sec_context(&min, context, GSS_C_NO_BUFFER);
        (void)maj;
        DEBUGASSERT(maj == GSS_S_COMPLETE);
    }
}
struct Curl_sec_client_mech Curl_krb5_client_mech = {
    "GSSAPI",
    sizeof(gss_ctx_id_t),
    krb5_init,
    krb5_auth,
    krb5_end,
    krb5_check_prot,
    krb5_overhead,
    krb5_encode,
    krb5_decode
};
#endif