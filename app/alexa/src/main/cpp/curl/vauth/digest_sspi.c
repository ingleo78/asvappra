#include "../curl_setup.h"

#if defined(USE_WINDOWS_SSPI) && !defined(CURL_DISABLE_CRYPTO_AUTH)
#include "curl/curl.h"
#include "vauth.h"
#include "digest.h"
#include "curl/urldata.h"
#include "curl/curl_base64.h"
#include "curl/warnless.h"
#include "curl/curl_multibyte.h"
#include "curl/sendf.h"
#include "curl/strdup.h"
#include "curl/strcase.h"
#include "curl/curl_memory.h"
#include "curl/memdebug.h"

bool Curl_auth_is_digest_supported(void) {
    PSecPkgInfo SecurityPackage;
    SECURITY_STATUS status;
    status = s_pSecFn->QuerySecurityPackageInfo((TCHAR *) TEXT(SP_NAME_DIGEST), &SecurityPackage);
    if(status == SEC_E_OK) s_pSecFn->FreeContextBuffer(SecurityPackage);
    return (status == SEC_E_OK ? TRUE : FALSE);
}
CURLcode Curl_auth_create_digest_md5_message(struct Curl_easy *data, const char *chlg64, const char *userp, const char *passwdp, const char *service,
                                             char **outptr, size_t *outlen) {
    CURLcode result = CURLE_OK;
    TCHAR *spn = NULL;
    size_t chlglen = 0;
    size_t token_max = 0;
    unsigned char *input_token = NULL;
    unsigned char *output_token = NULL;
    CredHandle credentials;
    CtxtHandle context;
    PSecPkgInfo SecurityPackage;
    SEC_WINNT_AUTH_IDENTITY identity;
    SEC_WINNT_AUTH_IDENTITY *p_identity;
    SecBuffer chlg_buf;
    SecBuffer resp_buf;
    SecBufferDesc chlg_desc;
    SecBufferDesc resp_desc;
    SECURITY_STATUS status;
    unsigned long attrs;
    TimeStamp expiry;
    if(strlen(chlg64) && *chlg64 != '=') {
        result = Curl_base64_decode(chlg64, &input_token, &chlglen);
        if(result) return result;
    }
    if(!input_token) {
        infof(data, "DIGEST-MD5 handshake failure (empty challenge message)\n");
        return CURLE_BAD_CONTENT_ENCODING;
    }
    status = s_pSecFn->QuerySecurityPackageInfo((TCHAR *) TEXT(SP_NAME_DIGEST), &SecurityPackage);
    if(status != SEC_E_OK) {
        free(input_token);
        failf(data, "SSPI: couldn't get auth info\n");
        return CURLE_AUTH_ERROR;
    }
    token_max = SecurityPackage->cbMaxToken;
    s_pSecFn->FreeContextBuffer(SecurityPackage);
    output_token = malloc(token_max);
    if(!output_token) {
        free(input_token);
        return CURLE_OUT_OF_MEMORY;
    }
    spn = Curl_auth_build_spn(service, data->conn->host.name, NULL);
    if(!spn) {
        free(output_token);
        free(input_token);
        return CURLE_OUT_OF_MEMORY;
    }
    if(userp && *userp) {
        result = Curl_create_sspi_identity(userp, passwdp, &identity);
        if(result) {
            free(spn);
            free(output_token);
            free(input_token);
            return result;
        }
        p_identity = &identity;
    } else p_identity = NULL;
    status = s_pSecFn->AcquireCredentialsHandle(NULL, (TCHAR*)TEXT(SP_NAME_DIGEST), SECPKG_CRED_OUTBOUND, NULL, p_identity, NULL, NULL, &credentials, &expiry);
    if(status != SEC_E_OK) {
        Curl_sspi_free_identity(p_identity);
        free(spn);
        free(output_token);
        free(input_token);
        return CURLE_LOGIN_DENIED;
    }
    chlg_desc.ulVersion = SECBUFFER_VERSION;
    chlg_desc.cBuffers  = 1;
    chlg_desc.pBuffers  = &chlg_buf;
    chlg_buf.BufferType = SECBUFFER_TOKEN;
    chlg_buf.pvBuffer   = input_token;
    chlg_buf.cbBuffer   = curlx_uztoul(chlglen);
    resp_desc.ulVersion = SECBUFFER_VERSION;
    resp_desc.cBuffers  = 1;
    resp_desc.pBuffers  = &resp_buf;
    resp_buf.BufferType = SECBUFFER_TOKEN;
    resp_buf.pvBuffer   = output_token;
    resp_buf.cbBuffer   = curlx_uztoul(token_max);
    status = s_pSecFn->InitializeSecurityContext(&credentials, NULL, spn, 0, 0, 0, &chlg_desc, 0, &context, &resp_desc, &attrs, &expiry);
    if(status == SEC_I_COMPLETE_NEEDED || status == SEC_I_COMPLETE_AND_CONTINUE) s_pSecFn->CompleteAuthToken(&credentials, &resp_desc);
    else if(status != SEC_E_OK && status != SEC_I_CONTINUE_NEEDED) {
        s_pSecFn->FreeCredentialsHandle(&credentials);
        Curl_sspi_free_identity(p_identity);
        free(spn);
        free(output_token);
        free(input_token);
        if(status == SEC_E_INSUFFICIENT_MEMORY) return CURLE_OUT_OF_MEMORY;
        return CURLE_AUTH_ERROR;
    }
    result = Curl_base64_encode(data, (char *) output_token, resp_buf.cbBuffer, outptr, outlen);
    s_pSecFn->DeleteSecurityContext(&context);
    s_pSecFn->FreeCredentialsHandle(&credentials);
    Curl_sspi_free_identity(p_identity);
    free(spn);
    free(output_token);
    free(input_token);
    return result;
}
CURLcode Curl_override_sspi_http_realm(const char *chlg, SEC_WINNT_AUTH_IDENTITY *identity) {
    xcharp_u domain, dup_domain;
    if(!identity->Domain || !identity->DomainLength) {
        for(;;) {
            char value[DIGEST_MAX_VALUE_LENGTH];
            char content[DIGEST_MAX_CONTENT_LENGTH];
            while(*chlg && ISSPACE(*chlg)) chlg++;
            if(Curl_auth_digest_get_pair(chlg, value, content, &chlg)) {
                if(strcasecompare(value, "realm")) {
                    domain.tchar_ptr = curlx_convert_UTF8_to_tchar((char *) content);
                    if(!domain.tchar_ptr) return CURLE_OUT_OF_MEMORY;
                    dup_domain.tchar_ptr = _tcsdup(domain.tchar_ptr);
                    if(!dup_domain.tchar_ptr) {
                        curlx_unicodefree(domain.tchar_ptr);
                        return CURLE_OUT_OF_MEMORY;
                    }
                    free(identity->Domain);
                    identity->Domain = dup_domain.tbyte_ptr;
                    identity->DomainLength = curlx_uztoul(_tcslen(dup_domain.tchar_ptr));
                    dup_domain.tchar_ptr = NULL;
                    curlx_unicodefree(domain.tchar_ptr);
                } else {
                    /* Unknown specifier, ignore it! */
                }
            } else break;
              while(*chlg && ISSPACE(*chlg)) chlg++;
          if(',' == *chlg) chlg++;
        }
    }
    return CURLE_OK;
}
CURLcode Curl_auth_decode_digest_http_message(const char *chlg, struct digestdata *digest) {
    size_t chlglen = strlen(chlg);
    if(digest->input_token) {
        bool stale = false;
        const char *p = chlg;
        for(;;) {
            char value[DIGEST_MAX_VALUE_LENGTH];
            char content[DIGEST_MAX_CONTENT_LENGTH];
            while(*p && ISSPACE(*p)) p++;
            if(!Curl_auth_digest_get_pair(p, value, content, &p)) break;
            if(strcasecompare(value, "stale") && strcasecompare(content, "true")) {
                stale = true;
                break;
            }
            while(*p && ISSPACE(*p)) p++;
            if(',' == *p) p++;
        }
        if(stale) Curl_auth_digest_cleanup(digest);
        else return CURLE_LOGIN_DENIED;
    }
    digest->input_token = (BYTE *) Curl_memdup(chlg, chlglen + 1);
    if(!digest->input_token) return CURLE_OUT_OF_MEMORY;
    digest->input_token_len = chlglen;
    return CURLE_OK;
}
CURLcode Curl_auth_create_digest_http_message(struct Curl_easy *data, const char *userp, const char *passwdp, const unsigned char *request,
                                              const unsigned char *uripath, struct digestdata *digest, char **outptr, size_t *outlen) {
    size_t token_max;
    char *resp;
    BYTE *output_token;
    size_t output_token_len = 0;
    PSecPkgInfo SecurityPackage;
    SecBuffer chlg_buf[5];
    SecBufferDesc chlg_desc;
    SECURITY_STATUS status;
    (void) data;
    status = s_pSecFn->QuerySecurityPackageInfo((TCHAR *) TEXT(SP_NAME_DIGEST), &SecurityPackage);
    if(status != SEC_E_OK) {
        failf(data, "SSPI: couldn't get auth info\n");
        return CURLE_AUTH_ERROR;
    }
    token_max = SecurityPackage->cbMaxToken;
    s_pSecFn->FreeContextBuffer(SecurityPackage);
    output_token = malloc(token_max);
    if(!output_token)  return CURLE_OUT_OF_MEMORY;
    if((userp && !digest->user) || (!userp && digest->user) || (passwdp && !digest->passwd) || (!passwdp && digest->passwd) ||
       (userp && digest->user && strcmp(userp, digest->user)) || (passwdp && digest->passwd && strcmp(passwdp, digest->passwd))) {
        if(digest->http_context) {
            s_pSecFn->DeleteSecurityContext(digest->http_context);
            Curl_safefree(digest->http_context);
        }
        Curl_safefree(digest->user);
        Curl_safefree(digest->passwd);
    }
    if(digest->http_context) {
        chlg_desc.ulVersion    = SECBUFFER_VERSION;
        chlg_desc.cBuffers     = 5;
        chlg_desc.pBuffers     = chlg_buf;
        chlg_buf[0].BufferType = SECBUFFER_TOKEN;
        chlg_buf[0].pvBuffer   = NULL;
        chlg_buf[0].cbBuffer   = 0;
        chlg_buf[1].BufferType = SECBUFFER_PKG_PARAMS;
        chlg_buf[1].pvBuffer   = (void *) request;
        chlg_buf[1].cbBuffer   = curlx_uztoul(strlen((const char *) request));
        chlg_buf[2].BufferType = SECBUFFER_PKG_PARAMS;
        chlg_buf[2].pvBuffer   = (void *) uripath;
        chlg_buf[2].cbBuffer   = curlx_uztoul(strlen((const char *) uripath));
        chlg_buf[3].BufferType = SECBUFFER_PKG_PARAMS;
        chlg_buf[3].pvBuffer   = NULL;
        chlg_buf[3].cbBuffer   = 0;
        chlg_buf[4].BufferType = SECBUFFER_PADDING;
        chlg_buf[4].pvBuffer   = output_token;
        chlg_buf[4].cbBuffer   = curlx_uztoul(token_max);
        status = s_pSecFn->MakeSignature(digest->http_context, 0, &chlg_desc, 0);
        if(status == SEC_E_OK) output_token_len = chlg_buf[4].cbBuffer;
        else {
            infof(data, "digest_sspi: MakeSignature failed, error 0x%08lx\n", (long)status);
            s_pSecFn->DeleteSecurityContext(digest->http_context);
            Curl_safefree(digest->http_context);
        }
    }
    if(!digest->http_context) {
        CredHandle credentials;
        SEC_WINNT_AUTH_IDENTITY identity;
        SEC_WINNT_AUTH_IDENTITY *p_identity;
        SecBuffer resp_buf;
        SecBufferDesc resp_desc;
        unsigned long attrs;
        TimeStamp expiry;
        TCHAR *spn;
        Curl_safefree(digest->user);
        Curl_safefree(digest->passwd);
        if(userp && *userp) {
            if(Curl_create_sspi_identity(userp, passwdp, &identity)) {
                free(output_token);
                return CURLE_OUT_OF_MEMORY;
            }
            if(Curl_override_sspi_http_realm((const char *) digest->input_token, &identity)) {
                free(output_token);
                return CURLE_OUT_OF_MEMORY;
            }
            p_identity = &identity;
        } else p_identity = NULL;
        if(userp) {
            digest->user = strdup(userp);
            if(!digest->user) {
                free(output_token);
                return CURLE_OUT_OF_MEMORY;
            }
        }
        if(passwdp) {
            digest->passwd = strdup(passwdp);
            if(!digest->passwd) {
                free(output_token);
                Curl_safefree(digest->user);
                return CURLE_OUT_OF_MEMORY;
            }
        }
        status = s_pSecFn->AcquireCredentialsHandle(NULL, (TCHAR*)TEXT(SP_NAME_DIGEST), SECPKG_CRED_OUTBOUND, NULL, p_identity, NULL, NULL, &credentials, &expiry);
        if(status != SEC_E_OK) {
            Curl_sspi_free_identity(p_identity);
            free(output_token);
            return CURLE_LOGIN_DENIED;
        }
        chlg_desc.ulVersion    = SECBUFFER_VERSION;
        chlg_desc.cBuffers     = 3;
        chlg_desc.pBuffers     = chlg_buf;
        chlg_buf[0].BufferType = SECBUFFER_TOKEN;
        chlg_buf[0].pvBuffer   = digest->input_token;
        chlg_buf[0].cbBuffer   = curlx_uztoul(digest->input_token_len);
        chlg_buf[1].BufferType = SECBUFFER_PKG_PARAMS;
        chlg_buf[1].pvBuffer   = (void *) request;
        chlg_buf[1].cbBuffer   = curlx_uztoul(strlen((const char *) request));
        chlg_buf[2].BufferType = SECBUFFER_PKG_PARAMS;
        chlg_buf[2].pvBuffer   = NULL;
        chlg_buf[2].cbBuffer   = 0;
        resp_desc.ulVersion = SECBUFFER_VERSION;
        resp_desc.cBuffers  = 1;
        resp_desc.pBuffers  = &resp_buf;
        resp_buf.BufferType = SECBUFFER_TOKEN;
        resp_buf.pvBuffer   = output_token;
        resp_buf.cbBuffer   = curlx_uztoul(token_max);
        spn = curlx_convert_UTF8_to_tchar((char *) uripath);
        if(!spn) {
            s_pSecFn->FreeCredentialsHandle(&credentials);
            Curl_sspi_free_identity(p_identity);
            free(output_token);
            return CURLE_OUT_OF_MEMORY;
        }
        digest->http_context = calloc(1, sizeof(CtxtHandle));
        if(!digest->http_context) return CURLE_OUT_OF_MEMORY;
        status = s_pSecFn->InitializeSecurityContext(&credentials, NULL, spn, ISC_REQ_USE_HTTP_STYLE, 0, 0, &chlg_desc, 0, digest->http_context,
                                                     &resp_desc, &attrs, &expiry);
        curlx_unicodefree(spn);
        if(status == SEC_I_COMPLETE_NEEDED || status == SEC_I_COMPLETE_AND_CONTINUE) s_pSecFn->CompleteAuthToken(&credentials, &resp_desc);
        else if(status != SEC_E_OK && status != SEC_I_CONTINUE_NEEDED) {
            s_pSecFn->FreeCredentialsHandle(&credentials);
            Curl_sspi_free_identity(p_identity);
            free(output_token);
            Curl_safefree(digest->http_context);
            if(status == SEC_E_INSUFFICIENT_MEMORY) return CURLE_OUT_OF_MEMORY;
            return CURLE_AUTH_ERROR;
        }
        output_token_len = resp_buf.cbBuffer;
        s_pSecFn->FreeCredentialsHandle(&credentials);
        Curl_sspi_free_identity(p_identity);
    }
    resp = malloc(output_token_len + 1);
    if(!resp) {
        free(output_token);
        return CURLE_OUT_OF_MEMORY;
    }
    memcpy(resp, output_token, output_token_len);
    resp[output_token_len] = 0;
    *outptr = resp;
    *outlen = output_token_len;
    free(output_token);
    return CURLE_OK;
}
void Curl_auth_digest_cleanup(struct digestdata *digest) {
    Curl_safefree(digest->input_token);
    digest->input_token_len = 0;
    if(digest->http_context) {
        s_pSecFn->DeleteSecurityContext(digest->http_context);
        Curl_safefree(digest->http_context);
    }
    Curl_safefree(digest->user);
    Curl_safefree(digest->passwd);
}
#endif