#ifndef HEADER_CURL_MD5_H
#define HEADER_CURL_MD5_H

#ifndef CURL_DISABLE_CRYPTO_AUTH
#include "curl_hmac.h"
#define MD5_DIGEST_LEN  16
typedef void (* Curl_MD5_init_func)(void *context);
typedef void (* Curl_MD5_update_func)(void *context, const unsigned char *data, unsigned int len);
typedef void (* Curl_MD5_final_func)(unsigned char *result, void *context);
struct MD5_params {
    Curl_MD5_init_func md5_init_func;
    Curl_MD5_update_func md5_update_func;
    Curl_MD5_final_func md5_final_func;
    unsigned int md5_ctxtsize;
    unsigned int md5_resultlen;
};
struct MD5_context {
    const struct MD5_params *md5_hash;
    void *md5_hashctx;
};
extern const struct MD5_params Curl_DIGEST_MD5[1];
extern const struct HMAC_params Curl_HMAC_MD5[1];
void Curl_md5it(unsigned char *output, const unsigned char *input, const size_t len);
struct MD5_context *Curl_MD5_init(const struct MD5_params *md5params);
CURLcode Curl_MD5_update(struct MD5_context *context, const unsigned char *data, unsigned int len);
CURLcode Curl_MD5_final(struct MD5_context *context, unsigned char *result);
#endif
#endif