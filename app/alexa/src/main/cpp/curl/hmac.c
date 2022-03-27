#include "curl_setup.h"

#ifndef CURL_DISABLE_CRYPTO_AUTH
#include "curl.h"
#include "curl_hmac.h"
#include "curl_memory.h"
#include "warnless.h"
#include "memdebug.h"

static const unsigned char hmac_ipad = 0x36;
static const unsigned char hmac_opad = 0x5C;
struct HMAC_context * Curl_HMAC_init(const struct HMAC_params *hashparams, const unsigned char *key, unsigned int keylen) {
    size_t i;
    struct HMAC_context *ctxt;
    unsigned char *hkey;
    unsigned char b;
    i = sizeof(*ctxt) + 2 * hashparams->hmac_ctxtsize + hashparams->hmac_resultlen;
    ctxt = malloc(i);
    if(!ctxt) return ctxt;
    ctxt->hmac_hash = hashparams;
    ctxt->hmac_hashctxt1 = (void *) (ctxt + 1);
    ctxt->hmac_hashctxt2 = (void *) ((char *) ctxt->hmac_hashctxt1 + hashparams->hmac_ctxtsize);
    if(keylen > hashparams->hmac_maxkeylen) {
        (*hashparams->hmac_hinit)(ctxt->hmac_hashctxt1);
        (*hashparams->hmac_hupdate)(ctxt->hmac_hashctxt1, key, keylen);
        hkey = (unsigned char *) ctxt->hmac_hashctxt2 + hashparams->hmac_ctxtsize;
        (*hashparams->hmac_hfinal)(hkey, ctxt->hmac_hashctxt1);
        key = hkey;
        keylen = hashparams->hmac_resultlen;
    }
    (*hashparams->hmac_hinit)(ctxt->hmac_hashctxt1);
    (*hashparams->hmac_hinit)(ctxt->hmac_hashctxt2);
    for(i = 0; i < keylen; i++) {
        b = (unsigned char)(*key ^ hmac_ipad);
        (*hashparams->hmac_hupdate)(ctxt->hmac_hashctxt1, &b, 1);
        b = (unsigned char)(*key++ ^ hmac_opad);
        (*hashparams->hmac_hupdate)(ctxt->hmac_hashctxt2, &b, 1);
    }
    for(; i < hashparams->hmac_maxkeylen; i++) {
        (*hashparams->hmac_hupdate)(ctxt->hmac_hashctxt1, &hmac_ipad, 1);
        (*hashparams->hmac_hupdate)(ctxt->hmac_hashctxt2, &hmac_opad, 1);
    }
    return ctxt;
}
int Curl_HMAC_update(struct HMAC_context *ctxt, const unsigned char *data,unsigned int len) {
    (*ctxt->hmac_hash->hmac_hupdate)(ctxt->hmac_hashctxt1, data, len);
    return 0;
}
int Curl_HMAC_final(struct HMAC_context *ctxt, unsigned char *result) {
    const struct HMAC_params *hashparams = ctxt->hmac_hash;
    if(!result) result = (unsigned char *) ctxt->hmac_hashctxt2 + ctxt->hmac_hash->hmac_ctxtsize;
    (*hashparams->hmac_hfinal)(result, ctxt->hmac_hashctxt1);
    (*hashparams->hmac_hupdate)(ctxt->hmac_hashctxt2, result, hashparams->hmac_resultlen);
    (*hashparams->hmac_hfinal)(result, ctxt->hmac_hashctxt2);
    free((char *) ctxt);
    return 0;
}
CURLcode Curl_hmacit(const struct HMAC_params *hashparams, const unsigned char *key, const size_t keylen, const unsigned char *data, const size_t datalen,
                     unsigned char *output) {
    struct HMAC_context *ctxt = Curl_HMAC_init(hashparams, key, curlx_uztoui(keylen));
    if(!ctxt) return CURLE_OUT_OF_MEMORY;
    Curl_HMAC_update(ctxt, data, curlx_uztoui(datalen));
    Curl_HMAC_final(ctxt, output);
    return CURLE_OK;
}
#endif