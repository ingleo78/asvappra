#include "../curl_setup.h"
#if defined(USE_WINDOWS_SSPI) && defined(USE_KERBEROS5)
#include "../curl.h"
#include "../urldata.h"
#include "../curl_base64.h"
#include "../warnless.h"
#include "../curl_multibyte.h"
#include "../sendf.h"
#include "../curl_memory.h"
#include "../memdebug.h"
#include "vauth.h"

bool Curl_auth_is_gssapi_supported(void) {
    PSecPkgInfo SecurityPackage;
    SECURITY_STATUS status;
    status = s_pSecFn->QuerySecurityPackageInfo((TCHAR*)TEXT(SP_NAME_KERBEROS), &SecurityPackage);
    if(status == SEC_E_OK) s_pSecFn->FreeContextBuffer(SecurityPackage);
    return (status == SEC_E_OK ? TRUE : FALSE);
}
CURLcode Curl_auth_create_gssapi_user_message(struct Curl_easy *data, const char *userp, const char *passwdp, const char *service, const char *host,
                                              const bool mutual_auth, const char *chlg64, struct kerberos5data *krb5, char **outptr, size_t *outlen) {
    CURLcode result = CURLE_OK;
    size_t chlglen = 0;
    unsigned char *chlg = NULL;
    CtxtHandle context;
    PSecPkgInfo SecurityPackage;
    SecBuffer chlg_buf;
    SecBuffer resp_buf;
    SecBufferDesc chlg_desc;
    SecBufferDesc resp_desc;
    SECURITY_STATUS status;
    unsigned long attrs;
    TimeStamp expiry;
    if(!krb5->spn) {
        krb5->spn = Curl_auth_build_spn(service, host, NULL);
        if(!krb5->spn) return CURLE_OUT_OF_MEMORY;
    }
    if(!krb5->output_token) {
        status = s_pSecFn->QuerySecurityPackageInfo((TCHAR*)TEXT(SP_NAME_KERBEROS), &SecurityPackage);
        if(status != SEC_E_OK) {
            failf(data, "SSPI: couldn't get auth info\n");
            return CURLE_AUTH_ERROR;
        }
        krb5->token_max = SecurityPackage->cbMaxToken;
        s_pSecFn->FreeContextBuffer(SecurityPackage);
        krb5->output_token = malloc(krb5->token_max);
        if(!krb5->output_token) return CURLE_OUT_OF_MEMORY;
    }
    if(!krb5->credentials) {
        if(userp && *userp) {
          result = Curl_create_sspi_identity(userp, passwdp, &krb5->identity);
          if(result) return result;
          krb5->p_identity = &krb5->identity;
        } else krb5->p_identity = NULL;
        krb5->credentials = calloc(1, sizeof(CredHandle));
        if(!krb5->credentials) return CURLE_OUT_OF_MEMORY;
        status = s_pSecFn->AcquireCredentialsHandle(NULL, (TCHAR*)TEXT(SP_NAME_KERBEROS), SECPKG_CRED_OUTBOUND, NULL, krb5->p_identity, NULL, NULL,
                                                    krb5->credentials, &expiry);
        if(status != SEC_E_OK) return CURLE_LOGIN_DENIED;
        krb5->context = calloc(1, sizeof(CtxtHandle));
        if(!krb5->context) return CURLE_OUT_OF_MEMORY;
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
        chlg_desc.ulVersion = SECBUFFER_VERSION;
        chlg_desc.cBuffers  = 1;
        chlg_desc.pBuffers  = &chlg_buf;
        chlg_buf.BufferType = SECBUFFER_TOKEN;
        chlg_buf.pvBuffer   = chlg;
        chlg_buf.cbBuffer   = curlx_uztoul(chlglen);
    }
    resp_desc.ulVersion = SECBUFFER_VERSION;
    resp_desc.cBuffers  = 1;
    resp_desc.pBuffers  = &resp_buf;
    resp_buf.BufferType = SECBUFFER_TOKEN;
    resp_buf.pvBuffer   = krb5->output_token;
    resp_buf.cbBuffer   = curlx_uztoul(krb5->token_max);
    status = s_pSecFn->InitializeSecurityContext(krb5->credentials, chlg ? krb5->context : NULL, krb5->spn, (mutual_auth ? ISC_REQ_MUTUAL_AUTH : 0),
                                                 0, SECURITY_NATIVE_DREP, chlg ? &chlg_desc : NULL, 0, &context, &resp_desc, &attrs, &expiry);
    free(chlg);
    if(status == SEC_E_INSUFFICIENT_MEMORY)  return CURLE_OUT_OF_MEMORY;
    if(status != SEC_E_OK && status != SEC_I_CONTINUE_NEEDED) return CURLE_AUTH_ERROR;
    if(memcmp(&context, krb5->context, sizeof(context))) {
        s_pSecFn->DeleteSecurityContext(krb5->context);
        memcpy(krb5->context, &context, sizeof(context));
    }
    if(resp_buf.cbBuffer) result = Curl_base64_encode(data, (char *) resp_buf.pvBuffer, resp_buf.cbBuffer, outptr, outlen);
    else if(mutual_auth) {
        *outptr = strdup("");
        if(!*outptr) result = CURLE_OUT_OF_MEMORY;
    }
    return result;
}
CURLcode Curl_auth_create_gssapi_security_message(struct Curl_easy *data, const char *chlg64, struct kerberos5data *krb5, char **outptr, size_t *outlen) {
    CURLcode result = CURLE_OK;
    size_t offset = 0;
    size_t chlglen = 0;
    size_t messagelen = 0;
    size_t appdatalen = 0;
    unsigned char *chlg = NULL;
    unsigned char *trailer = NULL;
    unsigned char *message = NULL;
    unsigned char *padding = NULL;
    unsigned char *appdata = NULL;
    SecBuffer input_buf[2];
    SecBuffer wrap_buf[3];
    SecBufferDesc input_desc;
    SecBufferDesc wrap_desc;
    unsigned long indata = 0;
    unsigned long outdata = 0;
    unsigned long qop = 0;
    unsigned long sec_layer = 0;
    unsigned long max_size = 0;
    SecPkgContext_Sizes sizes;
    SecPkgCredentials_Names names;
    SECURITY_STATUS status;
    char *user_name;
    if(strlen(chlg64) && *chlg64 != '=') {
        result = Curl_base64_decode(chlg64, &chlg, &chlglen);
        if(result) return result;
    }
    if(!chlg) {
        infof(data, "GSSAPI handshake failure (empty security message)\n");
        return CURLE_BAD_CONTENT_ENCODING;
    }
    status = s_pSecFn->QueryContextAttributes(krb5->context, SECPKG_ATTR_SIZES, &sizes);
    if(status != SEC_E_OK) {
        free(chlg);
        if(status == SEC_E_INSUFFICIENT_MEMORY) return CURLE_OUT_OF_MEMORY;
        return CURLE_AUTH_ERROR;
    }
    status = s_pSecFn->QueryCredentialsAttributes(krb5->credentials, SECPKG_CRED_ATTR_NAMES, &names);
    if(status != SEC_E_OK) {
        free(chlg);
        if(status == SEC_E_INSUFFICIENT_MEMORY) return CURLE_OUT_OF_MEMORY;
        return CURLE_AUTH_ERROR;
    }
    input_desc.ulVersion = SECBUFFER_VERSION;
    input_desc.cBuffers = 2;
    input_desc.pBuffers = input_buf;
    input_buf[0].BufferType = SECBUFFER_STREAM;
    input_buf[0].pvBuffer = chlg;
    input_buf[0].cbBuffer = curlx_uztoul(chlglen);
    input_buf[1].BufferType = SECBUFFER_DATA;
    input_buf[1].pvBuffer = NULL;
    input_buf[1].cbBuffer = 0;
    status = s_pSecFn->DecryptMessage(krb5->context, &input_desc, 0, &qop);
    if(status != SEC_E_OK) {
        infof(data, "GSSAPI handshake failure (empty security message)\n");
        free(chlg);
        return CURLE_BAD_CONTENT_ENCODING;
    }
    if(input_buf[1].cbBuffer != 4) {
        infof(data, "GSSAPI handshake failure (invalid security data)\n");
        free(chlg);
        return CURLE_BAD_CONTENT_ENCODING;
    }
    memcpy(&indata, input_buf[1].pvBuffer, 4);
    s_pSecFn->FreeContextBuffer(input_buf[1].pvBuffer);
    free(chlg);
    sec_layer = indata & 0x000000FF;
    if(!(sec_layer & KERB_WRAP_NO_ENCRYPT)) {
        infof(data, "GSSAPI handshake failure (invalid security layer)\n");
        return CURLE_BAD_CONTENT_ENCODING;
    }
    max_size = ntohl(indata & 0xFFFFFF00);
    if(max_size > 0)  max_size = 0;
    trailer = malloc(sizes.cbSecurityTrailer);
    if(!trailer) return CURLE_OUT_OF_MEMORY;
    user_name = curlx_convert_tchar_to_UTF8(names.sUserName);
    if(!user_name) {
        free(trailer);
        return CURLE_OUT_OF_MEMORY;
    }
    messagelen = sizeof(outdata) + strlen(user_name) + 1;
    message = malloc(messagelen);
    if(!message) {
        free(trailer);
        curlx_unicodefree(user_name);
        return CURLE_OUT_OF_MEMORY;
    }
    outdata = htonl(max_size) | sec_layer;
    memcpy(message, &outdata, sizeof(outdata));
    strcpy((char *) message + sizeof(outdata), user_name);
    curlx_unicodefree(user_name);
    padding = malloc(sizes.cbBlockSize);
    if(!padding) {
        free(message);
        free(trailer);
        return CURLE_OUT_OF_MEMORY;
    }
    wrap_desc.ulVersion    = SECBUFFER_VERSION;
    wrap_desc.cBuffers     = 3;
    wrap_desc.pBuffers     = wrap_buf;
    wrap_buf[0].BufferType = SECBUFFER_TOKEN;
    wrap_buf[0].pvBuffer   = trailer;
    wrap_buf[0].cbBuffer   = sizes.cbSecurityTrailer;
    wrap_buf[1].BufferType = SECBUFFER_DATA;
    wrap_buf[1].pvBuffer   = message;
    wrap_buf[1].cbBuffer   = curlx_uztoul(messagelen);
    wrap_buf[2].BufferType = SECBUFFER_PADDING;
    wrap_buf[2].pvBuffer   = padding;
    wrap_buf[2].cbBuffer   = sizes.cbBlockSize;
    status = s_pSecFn->EncryptMessage(krb5->context, KERB_WRAP_NO_ENCRYPT, &wrap_desc, 0);
    if(status != SEC_E_OK) {
        free(padding);
        free(message);
        free(trailer);
        if(status == SEC_E_INSUFFICIENT_MEMORY) return CURLE_OUT_OF_MEMORY;
        return CURLE_AUTH_ERROR;
    }
    appdatalen = wrap_buf[0].cbBuffer + wrap_buf[1].cbBuffer + wrap_buf[2].cbBuffer;
    appdata = malloc(appdatalen);
    if(!appdata) {
        free(padding);
        free(message);
        free(trailer);
        return CURLE_OUT_OF_MEMORY;
    }
    memcpy(appdata, wrap_buf[0].pvBuffer, wrap_buf[0].cbBuffer);
    offset += wrap_buf[0].cbBuffer;
    memcpy(appdata + offset, wrap_buf[1].pvBuffer, wrap_buf[1].cbBuffer);
    offset += wrap_buf[1].cbBuffer;
    memcpy(appdata + offset, wrap_buf[2].pvBuffer, wrap_buf[2].cbBuffer);
    result = Curl_base64_encode(data, (char *) appdata, appdatalen, outptr, outlen);
    free(appdata);
    free(padding);
    free(message);
    free(trailer);
    return result;
}
void Curl_auth_cleanup_gssapi(struct kerberos5data *krb5) {
    if(krb5->context) {
        s_pSecFn->DeleteSecurityContext(krb5->context);
        free(krb5->context);
        krb5->context = NULL;
    }
    if(krb5->credentials) {
        s_pSecFn->FreeCredentialsHandle(krb5->credentials);
        free(krb5->credentials);
        krb5->credentials = NULL;
    }
    Curl_sspi_free_identity(krb5->p_identity);
    krb5->p_identity = NULL;
    Curl_safefree(krb5->spn);
    Curl_safefree(krb5->output_token);
    krb5->token_max = 0;
}
#endif