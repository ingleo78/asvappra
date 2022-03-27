#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre_internal.h"

BOOL _pcre_is_newline(USPTR ptr, int type, USPTR endptr, int *lenptr, BOOL utf8) {
    int c;
    if (utf8) { GETCHAR(c, ptr); } else c = *ptr;
    if (type == NLTYPE_ANYCRLF)
        switch(c) {
            case 0x000a: *lenptr = 1; return TRUE;
            case 0x000d: *lenptr = (ptr < endptr - 1 && ptr[1] == 0x0a) ? 2 : 1; return TRUE;
            default: return FALSE;
        }
    else
        switch(c) {
            case 0x000a: case 0x000b: case 0x000c: *lenptr = 1; return TRUE;
            case 0x000d: *lenptr = (ptr < endptr - 1 && ptr[1] == 0x0a) ? 2 : 1; return TRUE;
            case 0x0085: *lenptr = utf8? 2 : 1; return TRUE;
            case 0x2028: case 0x2029: *lenptr = 3; return TRUE;
            default: return FALSE;
        }
}
BOOL _pcre_was_newline(USPTR ptr, int type, USPTR startptr, int *lenptr, BOOL utf8) {
    int c;
    ptr--;
    #ifdef SUPPORT_UTF8
    if (utf8) {
        BACKCHAR(ptr);
        GETCHAR(c, ptr);
    } else c = *ptr;
    #else
    c = *ptr;
    #endif
    if (type == NLTYPE_ANYCRLF)
        switch(c) {
            case 0x000a: *lenptr = (ptr > startptr && ptr[-1] == 0x0d) ? 2 : 1; return TRUE;
            case 0x000d: *lenptr = 1; return TRUE;
            default: return FALSE;
        }
    else
        switch(c) {
            case 0x000a: *lenptr = (ptr > startptr && ptr[-1] == 0x0d) ? 2 : 1; return TRUE;
            case 0x000b: case 0x000c: case 0x000d: *lenptr = 1; return TRUE;
            case 0x0085: *lenptr = utf8? 2 : 1; return TRUE;
            case 0x2028: case 0x2029: *lenptr = 3; return TRUE;
            default: return FALSE;
        }
}