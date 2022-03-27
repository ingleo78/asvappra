#include "curl_setup.h"

#ifdef USE_WINDOWS_SSPI
#include "curl.h"
#include "curl_sspi.h"
#include "curl_multibyte.h"
#include "system_win32.h"
#include "warnless.h"
#include "curl_memory.h"
#include "memdebug.h"
typedef PSecurityFunctionTable (APIENTRY *INITSECURITYINTERFACE_FN)(VOID);
#ifdef UNICODE
#ifdef _WIN32_WCE
#define SECURITYENTRYPOINT L"InitSecurityInterfaceW"
#else
#define SECURITYENTRYPOINT "InitSecurityInterfaceW"
#endif
#else
#define SECURITYENTRYPOINT "InitSecurityInterfaceA"
#endif
HMODULE s_hSecDll = NULL;
PSecurityFunctionTable s_pSecFn = NULL;
CURLcode Curl_sspi_global_init(void) {
    INITSECURITYINTERFACE_FN pInitSecurityInterface;
    if(!s_hSecDll) {
        if (Curl_verify_windows_version(4, 0, PLATFORM_WINNT, VERSION_EQUAL)) s_hSecDll = Curl_load_library(TEXT("security.dll"));
        else s_hSecDll = Curl_load_library(TEXT("secur32.dll"));
        if (!s_hSecDll) return CURLE_FAILED_INIT;
        pInitSecurityInterface = CURLX_FUNCTION_CAST(INITSECURITYINTERFACE_FN, (GetProcAddress(s_hSecDll, SECURITYENTRYPOINT)));
        if (!pInitSecurityInterface) return CURLE_FAILED_INIT;
        s_pSecFn = pInitSecurityInterface();
        if(!s_pSecFn) return CURLE_FAILED_INIT;
    }
    return CURLE_OK;
}
void Curl_sspi_global_cleanup(void) {
    if (s_hSecDll) {
        FreeLibrary(s_hSecDll);
        s_hSecDll = NULL;
        s_pSecFn = NULL;
    }
}
CURLcode Curl_create_sspi_identity(const char *userp, const char *passwdp, SEC_WINNT_AUTH_IDENTITY *identity) {
    xcharp_u useranddomain;
    xcharp_u user, dup_user;
    xcharp_u domain, dup_domain;
    xcharp_u passwd, dup_passwd;
    size_t domlen = 0;
    domain.const_tchar_ptr = TEXT("");
    memset(identity, 0, sizeof(*identity));
    useranddomain.tchar_ptr = curlx_convert_UTF8_to_tchar((char *)userp);
    if (!useranddomain.tchar_ptr) return CURLE_OUT_OF_MEMORY;
    user.const_tchar_ptr = _tcschr(useranddomain.const_tchar_ptr, TEXT('\\'));
    if (!user.const_tchar_ptr) user.const_tchar_ptr = _tcschr(useranddomain.const_tchar_ptr, TEXT('/'));
    if (user.tchar_ptr) {
        domain.tchar_ptr = useranddomain.tchar_ptr;
        domlen = user.tchar_ptr - useranddomain.tchar_ptr;
        user.tchar_ptr++;
    } else {
        user.tchar_ptr = useranddomain.tchar_ptr;
        domain.const_tchar_ptr = TEXT("");
        domlen = 0;
    }
    dup_user.tchar_ptr = _tcsdup(user.tchar_ptr);
    if (!dup_user.tchar_ptr) {
        curlx_unicodefree(useranddomain.tchar_ptr);
        return CURLE_OUT_OF_MEMORY;
    }
    identity->User = dup_user.tbyte_ptr;
    identity->UserLength = curlx_uztoul(_tcslen(dup_user.tchar_ptr));
    dup_user.tchar_ptr = NULL;
    dup_domain.tchar_ptr = malloc(sizeof(TCHAR) * (domlen + 1));
    if (!dup_domain.tchar_ptr) {
        curlx_unicodefree(useranddomain.tchar_ptr);
        return CURLE_OUT_OF_MEMORY;
    }
    _tcsncpy(dup_domain.tchar_ptr, domain.tchar_ptr, domlen);
    *(dup_domain.tchar_ptr + domlen) = TEXT('\0');
    identity->Domain = dup_domain.tbyte_ptr;
    identity->DomainLength = curlx_uztoul(domlen);
    dup_domain.tchar_ptr = NULL;
    curlx_unicodefree(useranddomain.tchar_ptr);
    passwd.tchar_ptr = curlx_convert_UTF8_to_tchar((char *)passwdp);
    if (!passwd.tchar_ptr) return CURLE_OUT_OF_MEMORY;
    dup_passwd.tchar_ptr = _tcsdup(passwd.tchar_ptr);
    if (!dup_passwd.tchar_ptr) {
        curlx_unicodefree(passwd.tchar_ptr);
        return CURLE_OUT_OF_MEMORY;
    }
    identity->Password = dup_passwd.tbyte_ptr;
    identity->PasswordLength = curlx_uztoul(_tcslen(dup_passwd.tchar_ptr));
    dup_passwd.tchar_ptr = NULL;
    curlx_unicodefree(passwd.tchar_ptr);
    identity->Flags = SECFLAG_WINNT_AUTH_IDENTITY;
    return CURLE_OK;
}
void Curl_sspi_free_identity(SEC_WINNT_AUTH_IDENTITY *identity) {
    if(identity) {
        Curl_safefree(identity->User);
        Curl_safefree(identity->Password);
        Curl_safefree(identity->Domain);
    }
}
#endif