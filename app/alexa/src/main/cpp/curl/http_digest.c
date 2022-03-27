#include <stdbool.h>
#include "curl_setup.h"
#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_CRYPTO_AUTH)
#include "urldata.h"
#include "strcase.h"
#include "vauth/vauth.h"
#include "http_digest.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
#include "curl_ctype.h"

CURLcode Curl_input_digest(struct connectdata *conn, bool proxy, const char *header) {
    struct Curl_easy *data = conn->data;
    struct digestdata *digest;
    if (proxy) digest = &data->state.proxydigest;
    else digest = &data->state.digest;
    if (!checkprefix("Digest", header)) return CURLE_BAD_CONTENT_ENCODING;
    header += strlen("Digest");
    while(*header && ISSPACE(*header)) header++;
    return Curl_auth_decode_digest_http_message(header, digest);
}
CURLcode Curl_output_digest(struct connectdata *conn, bool proxy, const unsigned char *request, const unsigned char *uripath) {
    CURLcode result;
    struct Curl_easy *data = conn->data;
    unsigned char *path = NULL;
    char *tmp = NULL;
    char *response;
    size_t len;
    bool have_chlg;
    char **allocuserpwd;
    const char *userp;
    const char *passwdp;
    struct digestdata *digest;
    struct auth *authp;
    if (proxy) {
    #ifdef CURL_DISABLE_PROXY
        return CURLE_NOT_BUILT_IN;
    #else
        digest = &data->state.proxydigest;
        allocuserpwd = &data->state.aptr.proxyuserpwd;
        userp = conn->http_proxy.user;
        passwdp = conn->http_proxy.passwd;
        authp = &data->state.authproxy;
    #endif
    } else {
        digest = &data->state.digest;
        allocuserpwd = &data->state.aptr.userpwd;
        userp = conn->user;
        passwdp = conn->passwd;
        authp = &data->state.authhost;
    }
    Curl_safefree(*allocuserpwd);
    if (!userp) userp = "";
    if (!passwdp) passwdp = "";
#if defined(USE_WINDOWS_SSPI)
    have_chlg = digest->input_token ? TRUE : FALSE;
#else
    have_chlg = digest->nonce ? TRUE : FALSE;
#endif
    if (!have_chlg) {
        authp->done = FALSE;
        return CURLE_OK;
    }
    if (authp->iestyle) {
        tmp = strchr((char *)uripath, '?');
        if (tmp) {
            size_t urilen = tmp - (char *)uripath;
            path = (unsigned char *) aprintf("%.*s", urilen, uripath);
        }
    }
    if (!tmp) path = (unsigned char *) strdup((char *) uripath);
    if (!path) return CURLE_OUT_OF_MEMORY;
    result = Curl_auth_create_digest_http_message(data, userp, passwdp, request, path, digest, &response, &len);
    free(path);
    if (result) return result;
    *allocuserpwd = aprintf("%sAuthorization: Digest %s\r\n", proxy ? "Proxy-" : "", response);
    free(response);
    if (!*allocuserpwd) return CURLE_OUT_OF_MEMORY;
    authp->done = TRUE;
    return CURLE_OK;
}
void Curl_http_auth_cleanup_digest(struct Curl_easy *data) {
    Curl_auth_digest_cleanup(&data->state.digest);
    Curl_auth_digest_cleanup(&data->state.proxydigest);
}
#endif