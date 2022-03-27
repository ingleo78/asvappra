#include "curl_setup.h"

#if defined(HAVE_GSSAPI) && !defined(CURL_DISABLE_PROXY)
#include "curl_gssapi.h"
#include "urldata.h"
#include "sendf.h"
#include "connect.h"
#include "timeval.h"
#include "socks.h"
#include "warnless.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

static gss_ctx_id_t gss_context = GSS_C_NO_CONTEXT;
static int check_gss_err(struct Curl_easy *data, OM_uint32 major_status, OM_uint32 minor_status, const char *function) {
    if(GSS_ERROR(major_status)) {
        OM_uint32 maj_stat, min_stat;
        OM_uint32 msg_ctx = 0;
        gss_buffer_desc status_string;
        char buf[1024];
        size_t len;
        len = 0;
        msg_ctx = 0;
        while(!msg_ctx) {
            maj_stat = gss_display_status(&min_stat, major_status, GSS_C_GSS_CODE, GSS_C_NULL_OID, &msg_ctx, &status_string);
            if(maj_stat == GSS_S_COMPLETE) {
            if(sizeof(buf) > len + status_string.length + 1) {
                strcpy(buf + len, (char *) status_string.value);
                len += status_string.length;
            }
            gss_release_buffer(&min_stat, &status_string);
            break;
            }
            gss_release_buffer(&min_stat, &status_string);
        }
        if(sizeof(buf) > len + 3) {
            strcpy(buf + len, ".\n");
            len += 2;
        }
        msg_ctx = 0;
        while(!msg_ctx) {
            maj_stat = gss_display_status(&min_stat, minor_status, GSS_C_MECH_CODE, GSS_C_NULL_OID, &msg_ctx, &status_string);
            if(maj_stat == GSS_S_COMPLETE) {
            if(sizeof(buf) > len + status_string.length)
              strcpy(buf + len, (char *) status_string.value);
            gss_release_buffer(&min_stat, &status_string);
            break;
            }
            gss_release_buffer(&min_stat, &status_string);
        }
        failf(data, "GSS-API error: %s failed:\n%s", function, buf);
        return 1;
    }
    return 0;
}
CURLcode Curl_SOCKS5_gssapi_negotiate(int sockindex, struct connectdata *conn) {
    struct Curl_easy *data = conn->data;
    curl_socket_t sock = conn->sock[sockindex];
    CURLcode code;
    ssize_t actualread;
    ssize_t written;
    int result;
    OM_uint32 gss_major_status, gss_minor_status, gss_status;
    OM_uint32 gss_ret_flags;
    int gss_conf_state, gss_enc;
    gss_buffer_desc  service = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc  gss_send_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc  gss_recv_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc  gss_w_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc *gss_token = GSS_C_NO_BUFFER;
    gss_name_t       server = GSS_C_NO_NAME;
    gss_name_t       gss_client_name = GSS_C_NO_NAME;
    unsigned short   us_length;
    char             *user = NULL;
    unsigned char socksreq[4];
    const char *serviceptr = data->set.str[STRING_PROXY_SERVICE_NAME] ? data->set.str[STRING_PROXY_SERVICE_NAME] : "rcmd";
    const size_t serviceptr_length = strlen(serviceptr);
    if(strchr(serviceptr, '/')) {
        service.length = serviceptr_length;
        service.value = malloc(service.length);
        if(!service.value) return CURLE_OUT_OF_MEMORY;
        memcpy(service.value, serviceptr, service.length);
        gss_major_status = gss_import_name(&gss_minor_status, &service, (gss_OID) GSS_C_NULL_OID, &server);
    } else {
        service.value = malloc(serviceptr_length + strlen(conn->socks_proxy.host.name) + 2);
        if(!service.value) return CURLE_OUT_OF_MEMORY;
        service.length = serviceptr_length + strlen(conn->socks_proxy.host.name) + 1;
        msnprintf(service.value, service.length + 1, "%s@%s", serviceptr, conn->socks_proxy.host.name);
        gss_major_status = gss_import_name(&gss_minor_status, &service, GSS_C_NT_HOSTBASED_SERVICE, &server);
    }
    gss_release_buffer(&gss_status, &service);
    if(check_gss_err(data, gss_major_status, gss_minor_status, "gss_import_name()")) {
        failf(data, "Failed to create service name.");
        gss_release_name(&gss_status, &server);
        return CURLE_COULDNT_CONNECT;
    }
    (void)curlx_nonblock(sock, FALSE);
    for( ; ; ) {
        gss_major_status = Curl_gss_init_sec_context(data, &gss_minor_status, &gss_context, server, &Curl_krb5_mech_oid, NULL, gss_token, &gss_send_token, TRUE,
                                                     &gss_ret_flags);
        if(gss_token != GSS_C_NO_BUFFER) gss_release_buffer(&gss_status, &gss_recv_token);
        if(check_gss_err(data, gss_major_status, gss_minor_status, "gss_init_sec_context")) {
            gss_release_name(&gss_status, &server);
            gss_release_buffer(&gss_status, &gss_recv_token);
            gss_release_buffer(&gss_status, &gss_send_token);
            gss_delete_sec_context(&gss_status, &gss_context, NULL);
            failf(data, "Failed to initial GSS-API token.");
            return CURLE_COULDNT_CONNECT;
        }
        if(gss_send_token.length != 0) {
            socksreq[0] = 1;
            socksreq[1] = 1;
            us_length = htons((short)gss_send_token.length);
            memcpy(socksreq + 2, &us_length, sizeof(short));
            code = Curl_write_plain(conn, sock, (char *)socksreq, 4, &written);
            if(code || (4 != written)) {
                failf(data, "Failed to send GSS-API authentication request.");
                gss_release_name(&gss_status, &server);
                gss_release_buffer(&gss_status, &gss_recv_token);
                gss_release_buffer(&gss_status, &gss_send_token);
                gss_delete_sec_context(&gss_status, &gss_context, NULL);
                return CURLE_COULDNT_CONNECT;
            }
            code = Curl_write_plain(conn, sock, (char *)gss_send_token.value, gss_send_token.length, &written);
            if(code || ((ssize_t)gss_send_token.length != written)) {
                failf(data, "Failed to send GSS-API authentication token.");
                gss_release_name(&gss_status, &server);
                gss_release_buffer(&gss_status, &gss_recv_token);
                gss_release_buffer(&gss_status, &gss_send_token);
                gss_delete_sec_context(&gss_status, &gss_context, NULL);
                return CURLE_COULDNT_CONNECT;
            }
        }
        gss_release_buffer(&gss_status, &gss_send_token);
        gss_release_buffer(&gss_status, &gss_recv_token);
        if(gss_major_status != GSS_S_CONTINUE_NEEDED) break;
        result = Curl_blockread_all(conn, sock, (char *)socksreq, 4, &actualread);
        if(result || (actualread != 4)) {
            failf(data, "Failed to receive GSS-API authentication response.");
            gss_release_name(&gss_status, &server);
            gss_delete_sec_context(&gss_status, &gss_context, NULL);
            return CURLE_COULDNT_CONNECT;
        }
        if(socksreq[1] == 255) {
            failf(data, "User was rejected by the SOCKS5 server (%d %d).", socksreq[0], socksreq[1]);
            gss_release_name(&gss_status, &server);
            gss_delete_sec_context(&gss_status, &gss_context, NULL);
            return CURLE_COULDNT_CONNECT;
        }
        if(socksreq[1] != 1) {
            failf(data, "Invalid GSS-API authentication response type (%d %d).", socksreq[0], socksreq[1]);
            gss_release_name(&gss_status, &server);
            gss_delete_sec_context(&gss_status, &gss_context, NULL);
            return CURLE_COULDNT_CONNECT;
        }
        memcpy(&us_length, socksreq + 2, sizeof(short));
        us_length = ntohs(us_length);
        gss_recv_token.length = us_length;
        gss_recv_token.value = malloc(us_length);
        if(!gss_recv_token.value) {
            failf(data, "Could not allocate memory for GSS-API authentication " "response token.");
            gss_release_name(&gss_status, &server);
            gss_delete_sec_context(&gss_status, &gss_context, NULL);
            return CURLE_OUT_OF_MEMORY;
        }
        result = Curl_blockread_all(conn, sock, (char *)gss_recv_token.value, gss_recv_token.length, &actualread);
        if(result || (actualread != us_length)) {
            failf(data, "Failed to receive GSS-API authentication token.");
            gss_release_name(&gss_status, &server);
            gss_release_buffer(&gss_status, &gss_recv_token);
            gss_delete_sec_context(&gss_status, &gss_context, NULL);
            return CURLE_COULDNT_CONNECT;
        }
        gss_token = &gss_recv_token;
    }
    gss_release_name(&gss_status, &server);
    gss_major_status = gss_inquire_context(&gss_minor_status, gss_context, &gss_client_name, NULL, NULL, NULL, NULL, NULL, NULL);
    if(check_gss_err(data, gss_major_status, gss_minor_status, "gss_inquire_context")) {
        gss_delete_sec_context(&gss_status, &gss_context, NULL);
        gss_release_name(&gss_status, &gss_client_name);
        failf(data, "Failed to determine user name.");
        return CURLE_COULDNT_CONNECT;
    }
    gss_major_status = gss_display_name(&gss_minor_status, gss_client_name, &gss_send_token, NULL);
    if(check_gss_err(data, gss_major_status, gss_minor_status, "gss_display_name")) {
        gss_delete_sec_context(&gss_status, &gss_context, NULL);
        gss_release_name(&gss_status, &gss_client_name);
        gss_release_buffer(&gss_status, &gss_send_token);
        failf(data, "Failed to determine user name.");
        return CURLE_COULDNT_CONNECT;
    }
    user = malloc(gss_send_token.length + 1);
    if(!user) {
        gss_delete_sec_context(&gss_status, &gss_context, NULL);
        gss_release_name(&gss_status, &gss_client_name);
        gss_release_buffer(&gss_status, &gss_send_token);
        return CURLE_OUT_OF_MEMORY;
    }
    memcpy(user, gss_send_token.value, gss_send_token.length);
    user[gss_send_token.length] = '\0';
    gss_release_name(&gss_status, &gss_client_name);
    gss_release_buffer(&gss_status, &gss_send_token);
    infof(data, "SOCKS5 server authenticated user %s with GSS-API.\n",user);
    free(user);
    user = NULL;
    socksreq[0] = 1;
    socksreq[1] = 2;
    gss_enc = 0;
    if(gss_ret_flags & GSS_C_CONF_FLAG) gss_enc = 2;
    else if(gss_ret_flags & GSS_C_INTEG_FLAG) gss_enc = 1;
    infof(data, "SOCKS5 server supports GSS-API %s data protection.\n", (gss_enc == 0) ? "no" : ((gss_enc==1)?"integrity":"confidentiality"));
    gss_enc = 0;
    if(data->set.socks5_gssapi_nec) {
        us_length = htons((short)1);
        memcpy(socksreq + 2, &us_length, sizeof(short));
    } else {
        gss_send_token.length = 1;
        gss_send_token.value = malloc(1);
        if(!gss_send_token.value) {
            gss_delete_sec_context(&gss_status, &gss_context, NULL);
            return CURLE_OUT_OF_MEMORY;
        }
        memcpy(gss_send_token.value, &gss_enc, 1);
        gss_major_status = gss_wrap(&gss_minor_status, gss_context, 0, GSS_C_QOP_DEFAULT, &gss_send_token, &gss_conf_state, &gss_w_token);
        if(check_gss_err(data, gss_major_status, gss_minor_status, "gss_wrap")) {
            gss_release_buffer(&gss_status, &gss_send_token);
            gss_release_buffer(&gss_status, &gss_w_token);
            gss_delete_sec_context(&gss_status, &gss_context, NULL);
            failf(data, "Failed to wrap GSS-API encryption value into token.");
            return CURLE_COULDNT_CONNECT;
        }
        gss_release_buffer(&gss_status, &gss_send_token);
        us_length = htons((short)gss_w_token.length);
        memcpy(socksreq + 2, &us_length, sizeof(short));
    }
    code = Curl_write_plain(conn, sock, (char *)socksreq, 4, &written);
    if(code  || (4 != written)) {
        failf(data, "Failed to send GSS-API encryption request.");
        gss_release_buffer(&gss_status, &gss_w_token);
        gss_delete_sec_context(&gss_status, &gss_context, NULL);
        return CURLE_COULDNT_CONNECT;
    }
    if(data->set.socks5_gssapi_nec) {
        memcpy(socksreq, &gss_enc, 1);
        code = Curl_write_plain(conn, sock, socksreq, 1, &written);
        if(code || ( 1 != written)) {
            failf(data, "Failed to send GSS-API encryption type.");
            gss_delete_sec_context(&gss_status, &gss_context, NULL);
            return CURLE_COULDNT_CONNECT;
        }
    } else {
        code = Curl_write_plain(conn, sock, (char *)gss_w_token.value, gss_w_token.length, &written);
        if(code || ((ssize_t)gss_w_token.length != written)) {
            failf(data, "Failed to send GSS-API encryption type.");
            gss_release_buffer(&gss_status, &gss_w_token);
            gss_delete_sec_context(&gss_status, &gss_context, NULL);
            return CURLE_COULDNT_CONNECT;
        }
        gss_release_buffer(&gss_status, &gss_w_token);
    }
    result = Curl_blockread_all(conn, sock, (char *)socksreq, 4, &actualread);
    if(result || (actualread != 4)) {
        failf(data, "Failed to receive GSS-API encryption response.");
        gss_delete_sec_context(&gss_status, &gss_context, NULL);
        return CURLE_COULDNT_CONNECT;
    }
    if(socksreq[1] == 255) {
        failf(data, "User was rejected by the SOCKS5 server (%d %d).", socksreq[0], socksreq[1]);
        gss_delete_sec_context(&gss_status, &gss_context, NULL);
        return CURLE_COULDNT_CONNECT;
    }
    if(socksreq[1] != 2) {
        failf(data, "Invalid GSS-API encryption response type (%d %d).", socksreq[0], socksreq[1]);
        gss_delete_sec_context(&gss_status, &gss_context, NULL);
        return CURLE_COULDNT_CONNECT;
    }
    memcpy(&us_length, socksreq + 2, sizeof(short));
    us_length = ntohs(us_length);
    gss_recv_token.length = us_length;
    gss_recv_token.value = malloc(gss_recv_token.length);
    if(!gss_recv_token.value) {
        gss_delete_sec_context(&gss_status, &gss_context, NULL);
        return CURLE_OUT_OF_MEMORY;
    }
    result = Curl_blockread_all(conn, sock, (char *)gss_recv_token.value, gss_recv_token.length, &actualread);
    if(result || (actualread != us_length)) {
        failf(data, "Failed to receive GSS-API encryptrion type.");
        gss_release_buffer(&gss_status, &gss_recv_token);
        gss_delete_sec_context(&gss_status, &gss_context, NULL);
        return CURLE_COULDNT_CONNECT;
    }
    if(!data->set.socks5_gssapi_nec) {
        gss_major_status = gss_unwrap(&gss_minor_status, gss_context, &gss_recv_token, &gss_w_token, 0, GSS_C_QOP_DEFAULT);
        if(check_gss_err(data, gss_major_status, gss_minor_status, "gss_unwrap")) {
            gss_release_buffer(&gss_status, &gss_recv_token);
            gss_release_buffer(&gss_status, &gss_w_token);
            gss_delete_sec_context(&gss_status, &gss_context, NULL);
            failf(data, "Failed to unwrap GSS-API encryption value into token.");
            return CURLE_COULDNT_CONNECT;
        }
        gss_release_buffer(&gss_status, &gss_recv_token);
        if(gss_w_token.length != 1) {
            failf(data, "Invalid GSS-API encryption response length (%d).", gss_w_token.length);
            gss_release_buffer(&gss_status, &gss_w_token);
            gss_delete_sec_context(&gss_status, &gss_context, NULL);
            return CURLE_COULDNT_CONNECT;
        }
        memcpy(socksreq, gss_w_token.value, gss_w_token.length);
        gss_release_buffer(&gss_status, &gss_w_token);
    } else {
        if(gss_recv_token.length != 1) {
            failf(data, "Invalid GSS-API encryption response length (%d).", gss_recv_token.length);
            gss_release_buffer(&gss_status, &gss_recv_token);
            gss_delete_sec_context(&gss_status, &gss_context, NULL);
            return CURLE_COULDNT_CONNECT;
        }
        memcpy(socksreq, gss_recv_token.value, gss_recv_token.length);
        gss_release_buffer(&gss_status, &gss_recv_token);
    }
    (void)curlx_nonblock(sock, TRUE);
    infof(data, "SOCKS5 access with%s protection granted.\n", (socksreq[0] == 0) ? "out GSS-API data" : ((socksreq[0] == 1) ? " GSS-API integrity" :
          " GSS-API confidentiality"));
    conn->socks5_gssapi_enctype = socksreq[0];
    if(socksreq[0] == 0) gss_delete_sec_context(&gss_status, &gss_context, NULL);
    return CURLE_OK;
}
#endif