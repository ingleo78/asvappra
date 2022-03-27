#include "curl_setup.h"
#ifndef HAVE_STRTOK_R
#include <stddef.h>
#include "strtok.h"

char* Curl_strtok_r(char *ptr, const char *sep, char **end) {
    if(!ptr) ptr = *end;
    while(*ptr && strchr(sep, *ptr)) ++ptr;
    if(*ptr) {
        char *start = ptr;
        *end = start + 1;
        while(**end && !strchr(sep, **end)) ++*end;
        if(**end) {
            **end = '\0';
            ++*end;
        }
        return start;
    }
    return NULL;
}

#endif