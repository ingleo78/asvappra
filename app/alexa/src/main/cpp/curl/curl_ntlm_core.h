#ifndef HEADER_CURL_NTLM_CORE_H
#define HEADER_CURL_NTLM_CORE_H
#include "curl_setup.h"

#if defined(USE_NTLM)
#if !defined(USE_OPENSSL) && !defined(USE_WOLFSSL) && !defined(USE_GNUTLS_NETTLE) && !defined(USE_GNUTLS) && defined(USE_NSS)
#define NTLM_NEEDS_NSS_INIT
#endif
#if !defined(USE_WINDOWS_SSPI) || defined(USE_WIN32_CRYPTO)
#if defined(USE_OPENSSL) || defined(USE_WOLFSSL)
#ifdef USE_WOLFSSL
#include <wolfssl/options.h>
#endif
#include <openssl/ssl.h>
#endif
#define USE_NTRESPONSES
#if defined(USE_NTRESPONSES) && !defined(CURL_DISABLE_CRYPTO_AUTH)
#define USE_NTLM2SESSION
#endif
#if defined(USE_NTRESPONSES) && (CURL_SIZEOF_CURL_OFF_T > 4)
#define USE_NTLM_V2
#endif
void Curl_ntlm_core_lm_resp(const unsigned char *keys, const unsigned char *plaintext, unsigned char *results);
CURLcode Curl_ntlm_core_mk_lm_hash(struct Curl_easy *data, const char *password, unsigned char *lmbuffer);
#ifdef USE_NTRESPONSES
CURLcode Curl_ntlm_core_mk_nt_hash(struct Curl_easy *data, const char *password, unsigned char *ntbuffer);
#if defined(USE_NTLM_V2) && !defined(USE_WINDOWS_SSPI)
CURLcode Curl_hmac_md5(const unsigned char *key, unsigned int keylen, const unsigned char *data, unsigned int datalen, unsigned char *output);
CURLcode Curl_ntlm_core_mk_ntlmv2_hash(const char *user, size_t userlen, const char *domain, size_t domlen, unsigned char *ntlmhash,
                                       unsigned char *ntlmv2hash);
CURLcode  Curl_ntlm_core_mk_ntlmv2_resp(unsigned char *ntlmv2hash, unsigned char *challenge_client, struct ntlmdata *ntlm,
                                        unsigned char **ntresp, unsigned int *ntresp_len);
CURLcode  Curl_ntlm_core_mk_lmv2_resp(unsigned char *ntlmv2hash, unsigned char *challenge_client, unsigned char *challenge_server,
                                      unsigned char *lmresp);
#endif
#endif
#endif
#endif
#endif