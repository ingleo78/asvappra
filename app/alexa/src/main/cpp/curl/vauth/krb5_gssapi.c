#include "../curl_setup.h"

#if defined(HAVE_GSSAPI) && defined(USE_KERBEROS5)
#include "../vauth/vauth.h"
#include "../curl.h"
#include "../curl_sasl.h"
#include "../urldata.h"
#include "../curl_base64.h"
#include "../curl_gssapi.h"
#include "../sendf.h"
#include "../curl_printf.h"
#include "../curl_memory.h"
#include "../memdebug.h"

bool Curl_auth_is_gssapi_supported(void) {
    return TRUE;
}
CURLcode Curl_auth_create_gssapi_user_message(struct Curl_easy *data, const char *userp, const char *passwdp, const char *service, const char *host,
                                              const bool mutual_auth, const char *chlg64, struct kerberos5data *krb5, char **outptr, size_t *outlen) {
    CURLcode result = CURLE_OK;
    size_t chlglen = 0;
    unsigned char *chlg = NULL;
    OM_uint32 major_status;
    OM_uint32 minor_status;
    OM_uint32 unused_status;
    gss_buffer_desc spn_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
    (void) userp;
    (void) passwdp;
    if(!krb5->spn) {
        char *spn = Curl_auth_build_spn(service, NULL, host);
        if(!spn) return CURLE_OUT_OF_MEMORY;
        spn_token.value = spn;
        spn_token.length = strlen(spn);
        major_status = gss_import_name(&minor_status, &spn_token, GSS_C_NT_HOSTBASED_SERVICE, &krb5->spn);
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
            infof(data, "GSSAPI handshake failure (empty challenge message)\n");
            return CURLE_BAD_CONTENT_ENCODING;
        }
        input_token.value = chlg;
        input_token.length = chlglen;
    }
    major_status = Curl_gss_init_sec_context(data, &minor_status, &krb5->context, krb5->spn, &Curl_krb5_mech_oid, GSS_C_NO_CHANNEL_BINDINGS, &input_token,
                                           &output_token, mutual_auth, NULL);
    free(input_token.value);
    if(GSS_ERROR(major_status)) {
        if(output_token.value) gss_release_buffer(&unused_status, &output_token);
        Curl_gss_log_error(data, "gss_init_sec_context() failed: ", major_status, minor_status);
        return CURLE_AUTH_ERROR;
    }
    if(output_token.value && output_token.length) {
        result = Curl_base64_encode(data, (char *) output_token.value, output_token.length, outptr, outlen);
        gss_release_buffer(&unused_status, &output_token);
    } else if(mutual_auth) {
        *outptr = strdup("");
        if(!*outptr) result = CURLE_OUT_OF_MEMORY;
    }
    return result;
}
CURLcode Curl_auth_create_gssapi_security_message(struct Curl_easy *data, const char *chlg64, struct kerberos5data *krb5, char **outptr,
                                                  size_t *outlen) {
  CURLcode result = CURLE_OK;
  size_t chlglen = 0;
  size_t messagelen = 0;
  unsigned char *chlg = NULL;
  unsigned char *message = NULL;
  OM_uint32 major_status;
  OM_uint32 minor_status;
  OM_uint32 unused_status;
  gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
  gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
  unsigned int indata = 0;
  unsigned int outdata = 0;
  gss_qop_t qop = GSS_C_QOP_DEFAULT;
  unsigned int sec_layer = 0;
  unsigned int max_size = 0;
  gss_name_t username = GSS_C_NO_NAME;
  gss_buffer_desc username_token;
  if(strlen(chlg64) && *chlg64 != '=') {
    result = Curl_base64_decode(chlg64, &chlg, &chlglen);
    if(result) return result;
  }
  if(!chlg) {
    infof(data, "GSSAPI handshake failure (empty security message)\n");
    return CURLE_BAD_CONTENT_ENCODING;
  }
  major_status = gss_inquire_context(&minor_status, krb5->context, &username, NULL, NULL, NULL, NULL, NULL, NULL);
  if(GSS_ERROR(major_status)) {
    Curl_gss_log_error(data, "gss_inquire_context() failed: ", major_status, minor_status);
    free(chlg);
    return CURLE_AUTH_ERROR;
  }
  major_status = gss_display_name(&minor_status, username, &username_token, NULL);
  if(GSS_ERROR(major_status)) {
    Curl_gss_log_error(data, "gss_display_name() failed: ", major_status, minor_status);
    free(chlg);
    return CURLE_AUTH_ERROR;
  }
  input_token.value = chlg;
  input_token.length = chlglen;
  major_status = gss_unwrap(&minor_status, krb5->context, &input_token, &output_token, NULL, &qop);
  if(GSS_ERROR(major_status)) {
    Curl_gss_log_error(data, "gss_unwrap() failed: ", major_status, minor_status);
    gss_release_buffer(&unused_status, &username_token);
    free(chlg);
    return CURLE_BAD_CONTENT_ENCODING;
  }
  if(output_token.length != 4) {
    infof(data, "GSSAPI handshake failure (invalid security data)\n");
    gss_release_buffer(&unused_status, &username_token);
    free(chlg);
    return CURLE_BAD_CONTENT_ENCODING;
  }
  memcpy(&indata, output_token.value, 4);
  gss_release_buffer(&unused_status, &output_token);
  free(chlg);
  sec_layer = indata & 0x000000FF;
  if(!(sec_layer & GSSAUTH_P_NONE)) {
    infof(data, "GSSAPI handshake failure (invalid security layer)\n");
    gss_release_buffer(&unused_status, &username_token);
    return CURLE_BAD_CONTENT_ENCODING;
  }
  max_size = ntohl(indata & 0xFFFFFF00);
  if(max_size > 0)  max_size = 0;
  messagelen = sizeof(outdata) + username_token.length + 1;
  message = malloc(messagelen);
  if(!message) {
    gss_release_buffer(&unused_status, &username_token);
    return CURLE_OUT_OF_MEMORY;
  }
  outdata = htonl(max_size) | sec_layer;
  memcpy(message, &outdata, sizeof(outdata));
  memcpy(message + sizeof(outdata), username_token.value, username_token.length);
  message[messagelen - 1] = '\0';
  gss_release_buffer(&unused_status, &username_token);
  input_token.value = message;
  input_token.length = messagelen;
  major_status = gss_wrap(&minor_status, krb5->context, 0, GSS_C_QOP_DEFAULT, &input_token, NULL, &output_token);
  if(GSS_ERROR(major_status)) {
    Curl_gss_log_error(data, "gss_wrap() failed: ", major_status, minor_status);
    free(message);
    return CURLE_AUTH_ERROR;
  }
  result = Curl_base64_encode(data, (char *) output_token.value, output_token.length, outptr, outlen);
  gss_release_buffer(&unused_status, &output_token);
  free(message);
  return result;
}
void Curl_auth_cleanup_gssapi(struct kerberos5data *krb5) {
  OM_uint32 minor_status;
  if(krb5->context != GSS_C_NO_CONTEXT) {
    gss_delete_sec_context(&minor_status, &krb5->context, GSS_C_NO_BUFFER);
    krb5->context = GSS_C_NO_CONTEXT;
  }
  if(krb5->spn != GSS_C_NO_NAME) {
    gss_release_name(&minor_status, &krb5->spn);
    krb5->spn = GSS_C_NO_NAME;
  }
}
#endif