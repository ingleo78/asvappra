#include "ntlm.h"

#if defined(USE_WINDOWS_SSPI) && defined(USE_NTLM)
#include "../curl.h"
#include "../vauth/vauth.h"
#include "urldata.h"
#include "curl_base64.h"
#include "curl_ntlm_core.h"
#include "warnless.h"
#include "curl_multibyte.h"
#include "sendf.h"
#include "curl_memory.h"
#include "memdebug.h"
bool Curl_auth_is_ntlm_supported(void) {
  PSecPkgInfo SecurityPackage;
  SECURITY_STATUS status;
  status = s_pSecFn->QuerySecurityPackageInfo((TCHAR *) TEXT(SP_NAME_NTLM), &SecurityPackage);
  if(status == SEC_E_OK) s_pSecFn->FreeContextBuffer(SecurityPackage);
  return (status == SEC_E_OK ? TRUE : FALSE);
}
CURLcode Curl_auth_create_ntlm_type1_message(struct Curl_easy *data, const char *userp, const char *passwdp, const char *service,
                                             const char *host, struct ntlmdata *ntlm, char **outptr, size_t *outlen) {
  PSecPkgInfo SecurityPackage;
  SecBuffer type_1_buf;
  SecBufferDesc type_1_desc;
  SECURITY_STATUS status;
  unsigned long attrs;
  TimeStamp expiry;
  Curl_auth_cleanup_ntlm(ntlm);
  status = s_pSecFn->QuerySecurityPackageInfo((TCHAR *) TEXT(SP_NAME_NTLM), &SecurityPackage);
  if(status != SEC_E_OK) {
      failf(data, "SSPI: couldn't get auth info\n");
      return CURLE_AUTH_ERROR;
  }
  ntlm->token_max = SecurityPackage->cbMaxToken;
  s_pSecFn->FreeContextBuffer(SecurityPackage);
  ntlm->output_token = malloc(ntlm->token_max);
  if(!ntlm->output_token) return CURLE_OUT_OF_MEMORY;
  if(userp && *userp) {
      CURLcode result;
      result = Curl_create_sspi_identity(userp, passwdp, &ntlm->identity);
      if(result) return result;
      ntlm->p_identity = &ntlm->identity;
  } else ntlm->p_identity = NULL;
  ntlm->credentials = calloc(1, sizeof(CredHandle));
  if(!ntlm->credentials)
    return CURLE_OUT_OF_MEMORY;
  status = s_pSecFn->AcquireCredentialsHandle(NULL, (TCHAR*)TEXT(SP_NAME_NTLM), SECPKG_CRED_OUTBOUND, NULL, ntlm->p_identity, NULL, NULL,
                                              ntlm->credentials, &expiry);
  if(status != SEC_E_OK) return CURLE_LOGIN_DENIED;
  ntlm->context = calloc(1, sizeof(CtxtHandle));
  if(!ntlm->context) return CURLE_OUT_OF_MEMORY;
  ntlm->spn = Curl_auth_build_spn(service, host, NULL);
  if(!ntlm->spn) return CURLE_OUT_OF_MEMORY;
  type_1_desc.ulVersion = SECBUFFER_VERSION;
  type_1_desc.cBuffers  = 1;
  type_1_desc.pBuffers  = &type_1_buf;
  type_1_buf.BufferType = SECBUFFER_TOKEN;
  type_1_buf.pvBuffer   = ntlm->output_token;
  type_1_buf.cbBuffer   = curlx_uztoul(ntlm->token_max);
  status = s_pSecFn->InitializeSecurityContext(ntlm->credentials, NULL, ntlm->spn, 0, 0, SECURITY_NETWORK_DREP, NULL, 0, ntlm->context,
                                               &type_1_desc, &attrs, &expiry);
  if(status == SEC_I_COMPLETE_NEEDED || status == SEC_I_COMPLETE_AND_CONTINUE) s_pSecFn->CompleteAuthToken(ntlm->context, &type_1_desc);
  else if(status == SEC_E_INSUFFICIENT_MEMORY) return CURLE_OUT_OF_MEMORY;
  else if(status != SEC_E_OK && status != SEC_I_CONTINUE_NEEDED) return CURLE_AUTH_ERROR;
  return Curl_base64_encode(data, (char*)ntlm->output_token, type_1_buf.cbBuffer, outptr, outlen);
}
CURLcode Curl_auth_decode_ntlm_type2_message(struct Curl_easy *data, const char *type2msg, struct ntlmdata *ntlm) {
  CURLcode result = CURLE_OK;
  unsigned char *type2 = NULL;
  size_t type2_len = 0;
#if defined(CURL_DISABLE_VERBOSE_STRINGS)
  (void) data;
#endif
  if(strlen(type2msg) && *type2msg != '=') {
      result = Curl_base64_decode(type2msg, &type2, &type2_len);
      if(result) return result;
  }
  if(!type2) {
      infof(data, "NTLM handshake failure (empty type-2 message)\n");
      return CURLE_BAD_CONTENT_ENCODING;
  }
  ntlm->input_token = type2;
  ntlm->input_token_len = type2_len;
  return result;
}
CURLcode Curl_auth_create_ntlm_type3_message(struct Curl_easy *data, const char *userp, const char *passwdp, struct ntlmdata *ntlm,
                                             char **outptr, size_t *outlen) {
  CURLcode result = CURLE_OK;
  SecBuffer type_2_bufs[2];
  SecBuffer type_3_buf;
  SecBufferDesc type_2_desc;
  SecBufferDesc type_3_desc;
  SECURITY_STATUS status;
  unsigned long attrs;
  TimeStamp expiry;
  (void) passwdp;
  (void) userp;
  type_2_desc.ulVersion = SECBUFFER_VERSION;
  type_2_desc.cBuffers = 1;
  type_2_desc.pBuffers = &type_2_bufs[0];
  type_2_bufs[0].BufferType = SECBUFFER_TOKEN;
  type_2_bufs[0].pvBuffer = ntlm->input_token;
  type_2_bufs[0].cbBuffer = curlx_uztoul(ntlm->input_token_len);
#ifdef SECPKG_ATTR_ENDPOINT_BINDINGS
  if(ntlm->sslContext) {
      SEC_CHANNEL_BINDINGS channelBindings;
      SecPkgContext_Bindings pkgBindings;
      pkgBindings.Bindings = &channelBindings;
      status = s_pSecFn->QueryContextAttributes(ntlm->sslContext, SECPKG_ATTR_ENDPOINT_BINDINGS, &pkgBindings);
      if(status == SEC_E_OK) {
          type_2_desc.cBuffers++;
          type_2_bufs[1].BufferType = SECBUFFER_CHANNEL_BINDINGS;
          type_2_bufs[1].cbBuffer = pkgBindings.BindingsLength;
          type_2_bufs[1].pvBuffer = pkgBindings.Bindings;
      }
  }
#endif
  type_3_desc.ulVersion = SECBUFFER_VERSION;
  type_3_desc.cBuffers = 1;
  type_3_desc.pBuffers = &type_3_buf;
  type_3_buf.BufferType = SECBUFFER_TOKEN;
  type_3_buf.pvBuffer = ntlm->output_token;
  status = s_pSecFn->InitializeSecurityContext(ntlm->credentials, ntlm->context, ntlm->spn, 0, 0, SECURITY_NETWORK_DREP, &type_2_desc,
                                               0, ntlm->context, &type_3_desc, &attrs, &expiry);
  if(status != SEC_E_OK) {
      infof(data, "NTLM handshake failure (type-3 message): Status=%x\n", status);
      if(status == SEC_E_INSUFFICIENT_MEMORY) return CURLE_OUT_OF_MEMORY;
      return CURLE_AUTH_ERROR;
  }
  result = Curl_base64_encode(data, (char *) ntlm->output_token, type_3_buf.cbBuffer, outptr, outlen);
  Curl_auth_cleanup_ntlm(ntlm);
  return result;
}
void Curl_auth_cleanup_ntlm(struct ntlmdata *ntlm) {
  if(ntlm->context) {
      s_pSecFn->DeleteSecurityContext(ntlm->context);
      free(ntlm->context);
      ntlm->context = NULL;
  }
  if(ntlm->credentials) {
      s_pSecFn->FreeCredentialsHandle(ntlm->credentials);
      free(ntlm->credentials);
      ntlm->credentials = NULL;
  }
  Curl_sspi_free_identity(ntlm->p_identity);
  ntlm->p_identity = NULL;
  Curl_safefree(ntlm->input_token);
  Curl_safefree(ntlm->output_token);
  ntlm->token_max = 0;
  Curl_safefree(ntlm->spn);
}
#endif