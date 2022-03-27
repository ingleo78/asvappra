#include "../curl_setup.h"
#if !defined(CURL_DISABLE_CRYPTO_AUTH)
#include "../curl.h"
#include "../urldata.h"
#include "../vauth/vauth.h"
#include "../curl_base64.h"
#include "../curl_hmac.h"
#include "../curl_md5.h"
#include "../warnless.h"
#include "../curl_printf.h"
#include "../curl_memory.h"
#include "../memdebug.h"

CURLcode Curl_auth_decode_cram_md5_message(const char *chlg64, char **outptr, size_t *outlen) {
    CURLcode result = CURLE_OK;
    size_t chlg64len = strlen(chlg64);
    *outptr = NULL;
    *outlen = 0;
    if(chlg64len && *chlg64 != '=') result = Curl_base64_decode(chlg64, (unsigned char **) outptr, outlen);
    return result;
}
CURLcode Curl_auth_create_cram_md5_message(struct Curl_easy *data, const char *chlg, const char *userp, const char *passwdp, char **outptr, size_t *outlen) {
    CURLcode result = CURLE_OK;
    size_t chlglen = 0;
    struct HMAC_context *ctxt;
    unsigned char digest[MD5_DIGEST_LEN];
    char *response;
    if(chlg) chlglen = strlen(chlg);
    ctxt = Curl_HMAC_init(Curl_HMAC_MD5, (const unsigned char*)passwdp, curlx_uztoui(strlen(passwdp)));
    if(!ctxt) return CURLE_OUT_OF_MEMORY;
    if(chlglen > 0) Curl_HMAC_update(ctxt, (const unsigned char *) chlg, curlx_uztoui(chlglen));
    Curl_HMAC_final(ctxt, digest);
    response = aprintf("%s %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", userp, digest[0], digest[1], digest[2], digest[3], digest[4],
                       digest[5], digest[6], digest[7], digest[8], digest[9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15]);
    if(!response) return CURLE_OUT_OF_MEMORY;
    result = Curl_base64_encode(data, response, 0, outptr, outlen);
    free(response);
    return result;
}

#endif