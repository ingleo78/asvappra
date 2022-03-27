#include "pcre_internal.h"

PCRE_EXP_DEFN int PCRE_CALL_CONVENTION pcre_config(int what, void *where) {
    switch(what) {
        case PCRE_CONFIG_UTF8:
        #ifdef SUPPORT_UTF8
            *((int *)where) = 1;
        #else
            *((int *)where) = 0;
        #endif
            break;
        case PCRE_CONFIG_UNICODE_PROPERTIES:
        #ifdef SUPPORT_UCP
            *((int *)where) = 1;
        #else
            *((int *)where) = 0;
        #endif
            break;
        //case PCRE_CONFIG_NEWLINE: *((int *)where) = NEWLINE; break;
        case PCRE_CONFIG_BSR:
        #ifdef BSR_ANYCRLF
            *((int *)where) = 1;
        #else
            *((int *)where) = 0;
        #endif
            break;
        /*case PCRE_CONFIG_LINK_SIZE: *((int *)where) = LINK_SIZE; break;
        case PCRE_CONFIG_POSIX_MALLOC_THRESHOLD: *((int *)where) = POSIX_MALLOC_THRESHOLD; break;
        case PCRE_CONFIG_MATCH_LIMIT: *((unsigned long int *)where) = MATCH_LIMIT; break;
        case PCRE_CONFIG_MATCH_LIMIT_RECURSION: *((unsigned long int *)where) = MATCH_LIMIT_RECURSION; break;*/
        case PCRE_CONFIG_STACKRECURSE:
        #ifdef NO_RECURSE
            *((int *)where) = 0;
        #else
            *((int *)where) = 1;
        #endif
            break;
        default: return PCRE_ERROR_BADOPTION;
    }
    return 0;
}
