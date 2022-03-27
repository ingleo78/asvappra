#include "curl_setup.h"
#include "curl.h"
#include "dotdot.h"
#include "curl_memory.h"
#include "memdebug.h"

char *Curl_dedotdotify(const char *input) {
    size_t inlen = strlen(input);
    char *clone;
    size_t clen = inlen;
    char *out = malloc(inlen + 1);
    char *outptr;
    char *orgclone;
    char *queryp;
    if (!out) return NULL;
    *out = 0;
    clone = strdup(input);
    if (!clone) {
        free(out);
        return NULL;
    }
    orgclone = clone;
    outptr = out;
    if (!*clone) {
        free(out);
        return clone;
    }
    queryp = strchr(clone, '?');
    if (queryp) *queryp = 0;
    do {
        if (!strncmp("./", clone, 2)) {
            clone += 2;
            clen -= 2;
        } else if (!strncmp("../", clone, 3)) {
            clone += 3;
            clen -= 3;
        } else if (!strncmp("/./", clone, 3)) {
            clone += 2;
            clen -= 2;
        } else if (!strcmp("/.", clone)) {
            clone[1]='/';
            clone++;
            clen -= 1;
        } else if (!strncmp("/../", clone, 4)) {
            clone += 3;
            clen -= 3;
            while(outptr > out) {
                outptr--;
                if(*outptr == '/') break;
            }
            *outptr = 0;
        } else if(!strcmp("/..", clone)) {
            clone[2]='/';
            clone += 2;
            clen -= 2;
            while(outptr > out) {
                outptr--;
                if (*outptr == '/') break;
            }
            *outptr = 0;
        } else if (!strcmp(".", clone) || !strcmp("..", clone)) {
            *clone = 0;
            *out = 0;
        } else {
            do {
                *outptr++ = *clone++;
                clen--;
            } while(*clone && (*clone != '/'));
            *outptr = 0;
        }
    } while(*clone);
    if (queryp) {
        size_t qlen;
        size_t oindex = queryp - orgclone;
        qlen = strlen(&input[oindex]);
        memcpy(outptr, &input[oindex], qlen + 1);
    }
    free(orgclone);
    return out;
}