#include "../curl_setup.h"
#if !defined(CURL_DISABLE_IMAP) || !defined(CURL_DISABLE_SMTP) || !defined(CURL_DISABLE_POP3)
#include "../curl.h"
#include "../urldata.h"
#include "../curl_base64.h"
#include "../warnless.h"
#include "../curl_printf.h"
#include "../curl_memory.h"
#include "../memdebug.h"
#include "vauth.h"

CURLcode Curl_auth_create_oauth_bearer_message(struct Curl_easy *data, const char *user, const char *host, const long port, const char *bearer,
                                               char **outptr, size_t *outlen) {
    CURLcode result = CURLE_OK;
    char *oauth = NULL;
    if(port == 0 || port == 80) oauth = aprintf("n,a=%s,\1host=%s\1auth=Bearer %s\1\1", user, host, bearer);
    else oauth = aprintf("n,a=%s,\1host=%s\1port=%ld\1auth=Bearer %s\1\1", user, host, port, bearer);
    if(!oauth) return CURLE_OUT_OF_MEMORY;
    result = Curl_base64_encode(data, oauth, strlen(oauth), outptr, outlen);
    free(oauth);
    return result;
}
CURLcode Curl_auth_create_xoauth_bearer_message(struct Curl_easy *data, const char *user, const char *bearer, char **outptr, size_t *outlen) {
    CURLcode result = CURLE_OK;
    char *xoauth = aprintf("user=%s\1auth=Bearer %s\1\1", user, bearer);
    if(!xoauth) return CURLE_OUT_OF_MEMORY;
    result = Curl_base64_encode(data, xoauth, strlen(xoauth), outptr, outlen);
    free(xoauth);
    return result;
}
#endif