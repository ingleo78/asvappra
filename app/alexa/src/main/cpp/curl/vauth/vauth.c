#include <stdbool.h>
#include "../curl_setup.h"
#include "../curl.h"
#include "../curl_multibyte.h"
#include "../curl_printf.h"
#include "../curl_memory.h"
#include "../memdebug.h"
#include "vauth.h"

#if !defined(USE_WINDOWS_SSPI)
char *Curl_auth_build_spn(const char *service, const char *host, const char *realm) {
    char *spn = NULL;
    if(host && realm) spn = aprintf("%s/%s@%s", service, host, realm);
    else if(host) spn = aprintf("%s/%s", service, host);
    else if(realm) spn = aprintf("%s@%s", service, realm);
    return spn;
}
#else
TCHAR *Curl_auth_build_spn(const char *service, const char *host, const char *realm) {
    char *utf8_spn = NULL;
    TCHAR *tchar_spn = NULL;
    (void) realm;
    utf8_spn = aprintf("%s/%s", service, host);
    if(!utf8_spn) return NULL;
    tchar_spn = curlx_convert_UTF8_to_tchar(utf8_spn);
    if(!tchar_spn) {
        free(utf8_spn);
        return NULL;
    }
    curlx_unicodefree(utf8_spn);
    return tchar_spn;
}
#endif
bool Curl_auth_user_contains_domain(const char *user) {
    bool valid = FALSE;
    if(user && *user) {
        char *p = strpbrk(user, "\\/@");
        valid = (p != NULL && p > user && p < user + strlen(user) - 1 ? TRUE : FALSE);
    }
#if defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI)
    else valid = TRUE;
#endif
    return valid;
}