#include <errno.h>
#include "curl_setup.h"
#include "curl_ctype.h"
#include "strtoofft.h"

#if (SIZEOF_CURL_OFF_T > SIZEOF_LONG)
#ifdef HAVE_STRTOLL
#define strtooff strtoll
#else
    #if defined(_MSC_VER) && (_MSC_VER >= 1300) && (_INTEGRAL_MAX_BITS >= 64)
        #if defined(_SAL_VERSION)
            _Check_return_ _CRTIMP __int64 __cdecl _strtoi64(_In_z_ const char *_String, _Out_opt_ _Deref_post_z_ char **_EndPtr, _In_ int _Radix);
        #else
            _CRTIMP __int64 __cdecl _strtoi64(const char *_String, char **_EndPtr, int _Radix);
        #endif
        #define strtooff _strtoi64
    #else
        #define PRIVATE_STRTOOFF 1
    #endif
#endif
#else
#define strtooff strtol
#endif
#ifdef PRIVATE_STRTOOFF
#if('9' - '0') != 9 || ('Z' - 'A') != 25 || ('z' - 'a') != 25
#define NO_RANGE_TEST
static const char valchars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
#endif
static int get_char(char c, int base);
static curl_off_t strtooff(const char *nptr, char **endptr, int base) {
    char *end;
    int is_negative = 0;
    int overflow;
    int i;
    curl_off_t value = 0;
    curl_off_t newval;
    end = (char *)nptr;
    while(ISSPACE(end[0])) end++;
    if(end[0] == '-') {
        is_negative = 1;
        end++;
    } else if(end[0] == '+') end++;
    else if(end[0] == '\0') {
        if(endptr) *endptr = end;
        return 0;
    }
    if(end[0] == '0' && end[1] == 'x') {
        if(base == 16 || base == 0) {
            end += 2;
            base = 16;
        }
    } else if(end[0] == '0') {
        if(base == 8 || base == 0) {
            end++;
            base = 8;
        }
    }
    if(base == 0) base = 10;
    value = 0;
    overflow = 0;
    for(i = get_char(end[0], base); i != -1; end++, i = get_char(end[0], base)) {
        newval = base * value + i;
        if(newval < value) {
            overflow = 1;
            break;
        } else  value = newval;
    }
    if(!overflow) {
        if(is_negative) value *= -1;
    } else {
        if(is_negative) value = CURL_OFF_T_MIN;
        else value = CURL_OFF_T_MAX;
        errno = ERANGE;
    }
    if(endptr) *endptr = end;
    return value;
}
static int get_char(char c, int base) {
#ifndef NO_RANGE_TEST
    int value = -1;
    if(c <= '9' && c >= '0') value = c - '0';
    else if(c <= 'Z' && c >= 'A') value = c - 'A' + 10;
    else if(c <= 'z' && c >= 'a') value = c - 'a' + 10;
#else
    const char *cp;
    int value;
    cp = memchr(valchars, c, 10 + 26 + 26);
    if(!cp) return -1;
    value = cp - valchars;
    if(value >= 10 + 26) value -= 26;
#endif
    if(value >= base) value = -1;
    return value;
}
#endif
CURLofft curlx_strtoofft(const char *str, char **endp, int base, curl_off_t *num) {
    char *end;
    curl_off_t number;
    errno = 0;
    *num = 0;
    while(*str && ISSPACE(*str)) str++;
    if('-' == *str) {
        if(endp) *endp = (char *)str;
        return CURL_OFFT_INVAL;
    }
    number = strtooff(str, &end, base);
    if(endp) *endp = end;
    if(errno == ERANGE) return CURL_OFFT_FLOW;
    else if(str == end) return CURL_OFFT_INVAL;
    *num = number;
    return CURL_OFFT_OK;
}