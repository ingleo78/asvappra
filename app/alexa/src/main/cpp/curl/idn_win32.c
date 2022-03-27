#include "curl_setup.h"

#ifdef USE_WIN32_IDN
#include "curl_multibyte.h"
#include "curl_memory.h"
#include "warnless.h"
#include "memdebug.h"
#ifdef WANT_IDN_PROTOTYPES
#if defined(_SAL_VERSION)
WINNORMALIZEAPI int WINAPI IdnToAscii(_In_  DWORD dwFlags, _In_reads_(cchUnicodeChar) LPCWSTR lpUnicodeCharStr, _In_ int cchUnicodeChar,
                                      _Out_writes_opt_(cchASCIIChar) LPWSTR   lpASCIICharStr, _In_ int cchASCIIChar);
WINNORMALIZEAPI int WINAPI IdnToUnicode(_In_ DWORD dwFlags, _In_reads_(cchASCIIChar) LPCWSTR lpASCIICharStr, _In_ int cchASCIIChar,
                                        _Out_writes_opt_(cchUnicodeChar) LPWSTR lpUnicodeCharStr, _In_ int cchUnicodeChar);
#else
WINBASEAPI int WINAPI IdnToAscii(DWORD dwFlags, const WCHAR *lpUnicodeCharStr, int cchUnicodeChar, WCHAR *lpASCIICharStr, int cchASCIIChar);
WINBASEAPI int WINAPI IdnToUnicode(DWORD dwFlags, const WCHAR *lpASCIICharStr, int cchASCIIChar, WCHAR *lpUnicodeCharStr, int cchUnicodeChar);
#endif
#endif
#define IDN_MAX_LENGTH 255
bool curl_win32_idn_to_ascii(const char *in, char **out);
bool curl_win32_ascii_to_idn(const char *in, char **out);
bool curl_win32_idn_to_ascii(const char *in, char **out) {
    bool success = FALSE;
    wchar_t *in_w = curlx_convert_UTF8_to_wchar(in);
    if (in_w) {
        wchar_t punycode[IDN_MAX_LENGTH];
        int chars = IdnToAscii(0, in_w, -1, punycode, IDN_MAX_LENGTH);
        free(in_w);
        if(chars) {
            *out = curlx_convert_wchar_to_UTF8(punycode);
            if(*out) success = TRUE;
        }
    }
    return success;
}
bool curl_win32_ascii_to_idn(const char *in, char **out) {
    bool success = FALSE;
    wchar_t *in_w = curlx_convert_UTF8_to_wchar(in);
    if (in_w) {
        size_t in_len = wcslen(in_w) + 1;
        wchar_t unicode[IDN_MAX_LENGTH];
        int chars = IdnToUnicode(0, in_w, curlx_uztosi(in_len), unicode, IDN_MAX_LENGTH);
        free(in_w);
        if (chars) {
            *out = curlx_convert_wchar_to_UTF8(unicode);
            if (*out) success = TRUE;
        }
    }
    return success;
}
#endif