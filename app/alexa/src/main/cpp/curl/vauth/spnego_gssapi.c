#include "../curl_setup.h"
#if defined(HAVE_GSSAPI) && defined(USE_SPNEGO)
#include "../curl.h"
#include "../urldata.h"
#include "../curl_base64.h"
#include "../curl_gssapi.h"
#include "../warnless.h"
#include "../curl_multibyte.h"
#include "../sendf.h"
#include "../curl_memory.h"
#include "memdebug.h"
#include "vauth.h"

bool Curl_auth_is_spnego_supported(void) {
  return TRUE;
}
CURLcode Curl_auth_decode_spnego_message(struct Curl_easy *data, const char *user, const char *password, const char *service, const char *host,
                                         const char *chlg64, struct negotiatedata *nego) {
    CURLcode result = CURLE_OK;
    size_t chlglen = 0;
    unsigned char *chlg = NULL;
    OM_uint32 major_status;
    OM_uint32 minor_status;
    OM_uint32 unused_status;
    gss_buffer_desc spn_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
    (void) user;
    (void) password;
    if(nego->context && nego->status == GSS_S_COMPLETE) {
        Curl_auth_cleanup_spnego(nego);
        return CURLE_LOGIN_DENIED;
    }
    if(!nego->spn) {
        char *spn = Curl_auth_build_spn(service, NULL, host);
        if(!spn) return CURLE_OUT_OF_MEMORY;
        spn_token.value = spn;
        spn_token.length = strlen(spn);
        major_status = gss_import_name(&minor_status, &spn_token, GSS_C_NT_HOSTBASED_SERVICE, &nego->spn);
        if(GSS_ERROR(major_status)) {
            Curl_gss_log_error(data, "gss_import_name() failed: ", major_status, minor_status);
            free(spn);
            return CURLE_AUTH_ERROR;
        }
        free(spn);
    }
    if(chlg64 && *chlg64) {
        if(*chlg64 != '=') {
            result = Curl_base64_decode(chlg64, &chlg, &chlglen);
            if(result) return result;
        }
        if(!chlg) {
            infof(data, "SPNEGO handshake failure (empty challenge message)\n");
            return CURLE_BAD_CONTENT_ENCODING;
        }
        input_token.value = chlg;
        input_token.length = chlglen;
    }
    major_status = Curl_gss_init_sec_context(data, &minor_status, &nego->context, nego->spn, &Curl_spnego_mech_oid, GSS_C_NO_CHANNEL_BINDINGS, &input_token,
                                             &output_token, TRUE, NULL);
    Curl_safefree(input_token.value);
    nego->status = major_status;
    if(GSS_ERROR(major_status)) {
        if(output_token.value) gss_release_buffer(&unused_status, &output_token);
        Curl_gss_log_error(data, "gss_init_sec_context() failed: ", major_status, minor_status);
        return CURLE_AUTH_ERROR;
    }
    if(!output_token.value || !output_token.length) {
        if(output_token.value) gss_release_buffer(&unused_status, &output_token);
        return CURLE_AUTH_ERROR;
    }
    if(nego->output_token.length && nego->output_token.value) gss_release_buffer(&unused_status, &nego->output_token);
    nego->output_token = output_token;
    return CURLE_OK;
}
CURLcode Curl_auth_create_spnego_message(struct Curl_easy *data, struct negotiatedata *nego, char **outptr, size_t *outlen) {
    CURLcode result;
    OM_uint32 minor_status;
    result = Curl_base64_encode(data, nego->output_token.value, nego->output_token.length, outptr, outlen);
    if(result) {
        gss_release_buffer(&minor_status, &nego->output_token);
        nego->output_token.value = NULL;
        nego->output_token.length = 0;
        return result;
    }
    if(!*outptr || !*outlen) {
        gss_release_buffer(&minor_status, &nego->output_token);
        nego->output_token.value = NULL;
        nego->output_token.length = 0;
        return CURLE_REMOTE_ACCESS_DENIED;
    }
    return CURLE_OK;
}
void Curl_auth_cleanup_spnego(struct negotiatedata *nego) {
    OM_uint32 minor_status;
    if(nego->context != GSS_C_NO_CONTEXT) {
        gss_delete_sec_context(&minor_status, &nego->context, GSS_C_NO_BUFFER);
        nego->context = GSS_C_NO_CONTEXT;
    }
    if(nego->output_token.value) {
        gss_release_buffer(&minor_status, &nego->output_token);
        nego->output_token.value = NULL;
        nego->output_token.length = 0;
    }
    if(nego->spn != GSS_C_NO_NAME) {
        gss_release_name(&minor_status, &nego->spn);
        nego->spn = GSS_C_NO_NAME;
    }
    nego->status = 0;
    nego->noauthpersist = FALSE;
    nego->havenoauthpersist = FALSE;
    nego->havenegdata = FALSE;
    nego->havemultiplerequests = FALSE;
}
#endif