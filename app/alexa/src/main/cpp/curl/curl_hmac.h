#ifndef HEADER_CURL_HMAC_H
#define HEADER_CURL_HMAC_H

#ifndef CURL_DISABLE_CRYPTO_AUTH
#define HMAC_MD5_LENGTH 16
typedef void (*HMAC_hinit_func)(void *context);
typedef void (*HMAC_hupdate_func)(void *context, const unsigned char *data, unsigned int len);
typedef void (*HMAC_hfinal_func)(unsigned char *result, void *context);
struct HMAC_params {
    HMAC_hinit_func
    hmac_hinit;
    HMAC_hupdate_func hmac_hupdate;
    HMAC_hfinal_func hmac_hfinal;
    unsigned int hmac_ctxtsize;
    unsigned int hmac_maxkeylen;
    unsigned int hmac_resultlen;
};
struct HMAC_context {
    const struct HMAC_params *hmac_hash;
    void *hmac_hashctxt1;
    void *hmac_hashctxt2;
};
struct HMAC_context *Curl_HMAC_init(const struct HMAC_params *hashparams, const unsigned char *key, unsigned int keylen);
int Curl_HMAC_update(struct HMAC_context *context, const unsigned char *data, unsigned int len);
int Curl_HMAC_final(struct HMAC_context *context, unsigned char *result);
CURLcode Curl_hmacit(const struct HMAC_params *hashparams, const unsigned char *key, const size_t keylen,
                     const unsigned char *data, const size_t datalen, unsigned char *output);
#endif
#endif