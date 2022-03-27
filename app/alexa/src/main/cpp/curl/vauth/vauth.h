#ifndef HEADER_CURL_VAUTH_H
#define HEADER_CURL_VAUTH_H

#include "../curl.h"
#include "../urldata.h"

#if !defined(CURL_DISABLE_CRYPTO_AUTH)
struct digestdata;
#endif
#if defined(USE_NTLM)
struct ntlmdata;
#endif
#if defined(USE_KERBEROS5)
struct kerberos5data;
#endif
#if (defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI)) && defined(USE_SPNEGO)
struct negotiatedata;
#endif
#if defined(USE_WINDOWS_SSPI)
#define GSS_ERROR(status) ((status) & 0x80000000)
#endif
#if !defined(USE_WINDOWS_SSPI)
char *Curl_auth_build_spn(const char *service, const char *host, const char *realm);
#else
TCHAR *Curl_auth_build_spn(const char *service, const char *host, const char *realm);
#endif
bool Curl_auth_user_contains_domain(const char *user);
CURLcode Curl_auth_create_plain_message(struct Curl_easy *data, const char *authzid, const char *authcid, const char *passwd, char **outptr, size_t *outlen);
CURLcode Curl_auth_create_login_message(struct Curl_easy *data, const char *valuep, char **outptr, size_t *outlen);
CURLcode Curl_auth_create_external_message(struct Curl_easy *data, const char *user, char **outptr, size_t *outlen);
#if !defined(CURL_DISABLE_CRYPTO_AUTH)
CURLcode Curl_auth_decode_cram_md5_message(const char *chlg64, char **outptr, size_t *outlen);
CURLcode Curl_auth_create_cram_md5_message(struct Curl_easy *data, const char *chlg, const char *userp, const char *passwdp, char **outptr, size_t *outlen);
bool Curl_auth_is_digest_supported(void);
CURLcode Curl_auth_create_digest_md5_message(struct Curl_easy *data, const char *chlg64, const char *userp, const char *passwdp, const char *service,
                                             char **outptr, size_t *outlen);
CURLcode Curl_auth_decode_digest_http_message(const char *chlg, struct digestdata *digest);
CURLcode Curl_auth_create_digest_http_message(struct Curl_easy *data, const char *userp, const char *passwdp, const unsigned char *request,
                                              const unsigned char *uri, struct digestdata *digest, char **outptr, size_t *outlen);
void Curl_auth_digest_cleanup(struct digestdata *digest);
#endif
#if defined(USE_NTLM)
bool Curl_auth_is_ntlm_supported(void);
CURLcode Curl_auth_create_ntlm_type1_message(struct Curl_easy *data, const char *userp, const char *passwdp, const char *service, const char *host,
                                             struct ntlmdata *ntlm, char **outptr, size_t *outlen);
CURLcode Curl_auth_decode_ntlm_type2_message(struct Curl_easy *data, const char *type2msg, struct ntlmdata *ntlm);
CURLcode Curl_auth_create_ntlm_type3_message(struct Curl_easy *data, const char *userp, const char *passwdp, struct ntlmdata *ntlm, char **outptr, size_t *outlen);
void Curl_auth_cleanup_ntlm(struct ntlmdata *ntlm);
#endif
CURLcode Curl_auth_create_oauth_bearer_message(struct Curl_easy *data, const char *user, const char *host, const long port, const char *bearer,
                                               char **outptr, size_t *outlen);
CURLcode Curl_auth_create_xoauth_bearer_message(struct Curl_easy *data, const char *user, const char *bearer, char **outptr, size_t *outlen);
#if defined(USE_KERBEROS5)
bool Curl_auth_is_gssapi_supported(void);
CURLcode Curl_auth_create_gssapi_user_message(struct Curl_easy *data, const char *userp, const char *passwdp, const char *service, const char *host,
                                              const bool mutual, const char *chlg64, struct kerberos5data *krb5, char **outptr, size_t *outlen);
CURLcode Curl_auth_create_gssapi_security_message(struct Curl_easy *data, const char *input, struct kerberos5data *krb5, char **outptr, size_t *outlen);
void Curl_auth_cleanup_gssapi(struct kerberos5data *krb5);
#endif
#if defined(USE_SPNEGO)
bool Curl_auth_is_spnego_supported(void);
CURLcode Curl_auth_decode_spnego_message(struct Curl_easy *data, const char *user, const char *passwood, const char *service, const char *host,
                                         const char *chlg64, struct negotiatedata *nego);
CURLcode Curl_auth_create_spnego_message(struct Curl_easy *data, struct negotiatedata *nego, char **outptr, size_t *outlen);
void Curl_auth_cleanup_spnego(struct negotiatedata *nego);
#endif
#endif