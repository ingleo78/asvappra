#include "../curl_setup.h"
#if defined(USE_WINDOWS_SSPI) && defined(USE_SPNEGO)
#include "../curl.h"
#include "../urldata.h"
#include "../curl_base64.h"
#include "../warnless.h"
#include "../curl_multibyte.h"
#include "../sendf.h"
#include "../strerror.h"
#include "../curl_memory.h"
#include "../memdebug.h"
#include "vauth.h"

bool Curl_auth_is_spnego_supported(void) {
    PSecPkgInfo SecurityPackage;
    SECURITY_STATUS status;
    status = s_pSecFn->QuerySecurityPackageInfo((TCHAR*)TEXT(SP_NAME_NEGOTIATE), &SecurityPackage);
    if(status == SEC_E_OK) s_pSecFn->FreeContextBuffer(SecurityPackage);
    return (status == SEC_E_OK ? TRUE : FALSE);
}
CURLcode Curl_auth_decode_spnego_message(struct Curl_easy *data, const char *user, const char *password, const char *service, const char *host,
                                         const char *chlg64, struct negotiatedata *nego) {
    CURLcode result = CURLE_OK;
    size_t chlglen = 0;
    unsigned char *chlg = NULL;
    PSecPkgInfo SecurityPackage;
    SecBuffer chlg_buf[2];
    SecBuffer resp_buf;
    SecBufferDesc chlg_desc;
    SecBufferDesc resp_desc;
    unsigned long attrs;
    TimeStamp expiry;
#if defined(CURL_DISABLE_VERBOSE_STRINGS)
    (void) data;
#endif
    if(nego->context && nego->status == SEC_E_OK) {
        Curl_auth_cleanup_spnego(nego);
        return CURLE_LOGIN_DENIED;
    }
    if(!nego->spn) {
        nego->spn = Curl_auth_build_spn(service, host, NULL);
        if(!nego->spn) return CURLE_OUT_OF_MEMORY;
    }
    if(!nego->output_token) {
        nego->status = s_pSecFn->QuerySecurityPackageInfo((TCHAR*)TEXT(SP_NAME_NEGOTIATE), &SecurityPackage);
        if(nego->status != SEC_E_OK) {
            failf(data, "SSPI: couldn't get auth info\n");
            return CURLE_AUTH_ERROR;
        }
        nego->token_max = SecurityPackage->cbMaxToken;
        s_pSecFn->FreeContextBuffer(SecurityPackage);
        nego->output_token = malloc(nego->token_max);
        if(!nego->output_token) return CURLE_OUT_OF_MEMORY;
    }
    if(!nego->credentials) {
        if(user && *user) {
            result = Curl_create_sspi_identity(user, password, &nego->identity);
            if(result) return result;
            nego->p_identity = &nego->identity;
        } else nego->p_identity = NULL;
        nego->credentials = calloc(1, sizeof(CredHandle));
        if(!nego->credentials) return CURLE_OUT_OF_MEMORY;
        nego->status = s_pSecFn->AcquireCredentialsHandle(NULL, (TCHAR*)TEXT(SP_NAME_NEGOTIATE), SECPKG_CRED_OUTBOUND, NULL, nego->p_identity, NULL, NULL,
                                                          nego->credentials, &expiry);
        if(nego->status != SEC_E_OK) return CURLE_AUTH_ERROR;
        nego->context = calloc(1, sizeof(CtxtHandle));
        if(!nego->context) return CURLE_OUT_OF_MEMORY;
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
        chlg_desc.ulVersion    = SECBUFFER_VERSION;
        chlg_desc.cBuffers     = 1;
        chlg_desc.pBuffers     = &chlg_buf[0];
        chlg_buf[0].BufferType = SECBUFFER_TOKEN;
        chlg_buf[0].pvBuffer   = chlg;
        chlg_buf[0].cbBuffer   = curlx_uztoul(chlglen);
    #ifdef SECPKG_ATTR_ENDPOINT_BINDINGS
        if(nego->sslContext) {
            SEC_CHANNEL_BINDINGS channelBindings;
            SecPkgContext_Bindings pkgBindings;
            pkgBindings.Bindings = &channelBindings;
            nego->status = s_pSecFn->QueryContextAttributes(nego->sslContext, SECPKG_ATTR_ENDPOINT_BINDINGS, &pkgBindings);
            if(nego->status == SEC_E_OK) {
                chlg_desc.cBuffers++;
                chlg_buf[1].BufferType = SECBUFFER_CHANNEL_BINDINGS;
                chlg_buf[1].cbBuffer   = pkgBindings.BindingsLength;
                chlg_buf[1].pvBuffer   = pkgBindings.Bindings;
            }
        }
    #endif
    }
    resp_desc.ulVersion = SECBUFFER_VERSION;
    resp_desc.cBuffers  = 1;
    resp_desc.pBuffers  = &resp_buf;
    resp_buf.BufferType = SECBUFFER_TOKEN;
    resp_buf.pvBuffer   = nego->output_token;
    resp_buf.cbBuffer   = curlx_uztoul(nego->token_max);
    nego->status = s_pSecFn->InitializeSecurityContext(nego->credentials, chlg ? nego->context : NULL, nego->spn, ISC_REQ_CONFIDENTIALITY, 0, SECURITY_NATIVE_DREP,
                                                       chlg ? &chlg_desc : NULL, 0, nego->context, &resp_desc, &attrs, &expiry);
    free(chlg);
    if(GSS_ERROR(nego->status)) {
        char buffer[STRERROR_LEN];
        failf(data, "InitializeSecurityContext failed: %s", Curl_sspi_strerror(nego->status, buffer, sizeof(buffer)));
        if(nego->status == (DWORD)SEC_E_INSUFFICIENT_MEMORY) return CURLE_OUT_OF_MEMORY;
        return CURLE_AUTH_ERROR;
    }
    if(nego->status == SEC_I_COMPLETE_NEEDED || nego->status == SEC_I_COMPLETE_AND_CONTINUE) {
        nego->status = s_pSecFn->CompleteAuthToken(nego->context, &resp_desc);
        if(GSS_ERROR(nego->status)) {
            char buffer[STRERROR_LEN];
            failf(data, "CompleteAuthToken failed: %s", Curl_sspi_strerror(nego->status, buffer, sizeof(buffer)));
            if(nego->status == (DWORD)SEC_E_INSUFFICIENT_MEMORY) return CURLE_OUT_OF_MEMORY;
            return CURLE_AUTH_ERROR;
        }
    }
    nego->output_token_length = resp_buf.cbBuffer;
    return result;
}
CURLcode Curl_auth_create_spnego_message(struct Curl_easy *data, struct negotiatedata *nego, char **outptr, size_t *outlen) {
    CURLcode result;
    result = Curl_base64_encode(data, (const char*)nego->output_token, nego->output_token_length, outptr, outlen);
    if(result) return result;
    if(!*outptr || !*outlen) {
        free(*outptr);
        return CURLE_REMOTE_ACCESS_DENIED;
    }
    return CURLE_OK;
}
void Curl_auth_cleanup_spnego(struct negotiatedata *nego) {
    if(nego->context) {
        s_pSecFn->DeleteSecurityContext(nego->context);
        free(nego->context);
        nego->context = NULL;
    }
    if(nego->credentials) {
        s_pSecFn->FreeCredentialsHandle(nego->credentials);
        free(nego->credentials);
        nego->credentials = NULL;
    }
    Curl_sspi_free_identity(nego->p_identity);
    nego->p_identity = NULL;
    Curl_safefree(nego->spn);
    Curl_safefree(nego->output_token);
    nego->status = 0;
    nego->token_max = 0;
    nego->noauthpersist = FALSE;
    nego->havenoauthpersist = FALSE;
    nego->havenegdata = FALSE;
    nego->havemultiplerequests = FALSE;
}
#endif