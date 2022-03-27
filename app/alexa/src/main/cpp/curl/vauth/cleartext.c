#include "../curl_setup.h"
#if !defined(CURL_DISABLE_IMAP) || !defined(CURL_DISABLE_SMTP) || !defined(CURL_DISABLE_POP3)
#include "../curl.h"
#include "../urldata.h"
#include "../vauth/vauth.h"
#include "../curl_base64.h"
#include "../curl_md5.h"
#include "../warnless.h"
#include "../strtok.h"
#include "../sendf.h"
#include "../curl_printf.h"
#include "../curl_memory.h"
#include "../memdebug.h"

CURLcode Curl_auth_create_plain_message(struct Curl_easy *data, const char *authzid, const char *authcid, const char *passwd, char **outptr, size_t *outlen) {
    CURLcode result;
    char *plainauth;
    size_t zlen;
    size_t clen;
    size_t plen;
    size_t plainlen;
    *outlen = 0;
    *outptr = NULL;
    zlen = (authzid == NULL ? 0 : strlen(authzid));
    clen = strlen(authcid);
    plen = strlen(passwd);
    if((zlen > SIZE_T_MAX/4) || (clen > SIZE_T_MAX/4) || (plen > (SIZE_T_MAX/2 - 2))) return CURLE_OUT_OF_MEMORY;
    plainlen = zlen + clen + plen + 2;
    plainauth = malloc(plainlen);
    if(!plainauth) return CURLE_OUT_OF_MEMORY;
    if(zlen != 0) memcpy(plainauth, authzid, zlen);
    plainauth[zlen] = '\0';
    memcpy(plainauth + zlen + 1, authcid, clen);
    plainauth[zlen + clen + 1] = '\0';
    memcpy(plainauth + zlen + clen + 2, passwd, plen);
    result = Curl_base64_encode(data, plainauth, plainlen, outptr, outlen);
    free(plainauth);
    return result;
}
CURLcode Curl_auth_create_login_message(struct Curl_easy *data, const char *valuep, char **outptr, size_t *outlen) {
    size_t vlen = strlen(valuep);
    if(!vlen) {
        *outptr = strdup("=");
        if(*outptr) {
            *outlen = (size_t) 1;
            return CURLE_OK;
        }
        *outlen = 0;
        return CURLE_OUT_OF_MEMORY;
    }
    return Curl_base64_encode(data, valuep, vlen, outptr, outlen);
}
CURLcode Curl_auth_create_external_message(struct Curl_easy *data, const char *user, char **outptr, size_t *outlen) {
    return Curl_auth_create_login_message(data, user, outptr, outlen);
}

#endif