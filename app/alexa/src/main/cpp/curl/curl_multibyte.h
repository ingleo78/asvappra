#ifndef HEADER_CURL_MULTIBYTE_H
#define HEADER_CURL_MULTIBYTE_H
#include "curl_setup.h"

#if defined(WIN32)
wchar_t *curlx_convert_UTF8_to_wchar(const char *str_utf8);
char *curlx_convert_wchar_to_UTF8(const wchar_t *str_w);
#endif
#if defined(UNICODE) && defined(WIN32)
#define curlx_convert_UTF8_to_tchar(ptr) curlx_convert_UTF8_to_wchar((ptr))
#define curlx_convert_tchar_to_UTF8(ptr) curlx_convert_wchar_to_UTF8((ptr))
#define curlx_unicodefree(ptr) \
    do { \
        if(ptr) { \
            (free)(ptr); \
            (ptr) = NULL; \
        } \
    } while(0);
typedef union {
    unsigned short *tchar_ptr;
    const unsigned short *const_tchar_ptr;
    unsigned short *tbyte_ptr;
    const unsigned short *const_tbyte_ptr;
} xcharp_u;
#else
#define curlx_convert_UTF8_to_tchar(ptr) (ptr)
#define curlx_convert_tchar_to_UTF8(ptr) (ptr)
#define curlx_unicodefree(ptr) \
    do { \
        (ptr) = NULL; \
    } while(0);
typedef union {
    char *tchar_ptr;
    const char *const_tchar_ptr;
    unsigned char *tbyte_ptr;
    const unsigned char *const_tbyte_ptr;
} xcharp_u;
#endif
#endif