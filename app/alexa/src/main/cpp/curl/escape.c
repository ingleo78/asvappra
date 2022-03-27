#include <stdbool.h>
#include <assert.h>
#include "curl_setup.h"
#include "curl_ctype.h"
#include "curl.h"
#include "urldata.h"
#include "warnless.h"
#include "non-ascii.h"
#include "escape.h"
#include "strdup.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

#define DEBUGASSERT(x) assert(x)
#define TRUE 1
#define FALSE 0

bool Curl_isunreserved(unsigned char in) {
    switch(in) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case '-': case '.': case '_': case '~':
            return TRUE;
        default: break;
    }
    return FALSE;
}
char *curl_escape(const char *string, int inlength) {
    return curl_easy_escape(NULL, string, inlength);
}
char *curl_unescape(const char *string, int length) {
    return curl_easy_unescape(NULL, string, length, NULL);
}
char *curl_easy_escape(CURL *data, const char *string, int inlength) {
    size_t length;
    CURLcode result;
    struct dynbuf d;
    if (inlength < 0) return NULL;
    Curl_dyn_init(&d, CURL_MAX_INPUT_LENGTH);
    length = (inlength?(size_t)inlength:strlen(string));
    if (!length) return strdup("");
    while(length--) {
        unsigned char in = *string;
        if(Curl_isunreserved(in)) {
            if(Curl_dyn_addn(&d, &in, 1)) return NULL;
        } else {
            char encoded[4];
            result = Curl_convert_to_network(data, (char *)&in, 1);
            if(result) {
                Curl_dyn_free(&d);
                return NULL;
            }
            msnprintf(encoded, sizeof(encoded), "%%%02X", in);
            if(Curl_dyn_add(&d, encoded)) return NULL;
        }
        string++;
    }
    return Curl_dyn_ptr(&d);
}
CURLcode Curl_urldecode(struct Curl_easy *data, const char *string, size_t length, char **ostring, size_t *olen, enum urlreject ctrl) {
    size_t alloc;
    char *ns;
    size_t strindex = 0;
    unsigned long hex;
    CURLcode result = CURLE_OK;
    DEBUGASSERT(string);
    DEBUGASSERT(ctrl >= REJECT_NADA);
    alloc = (length?length:strlen(string)) + 1;
    ns = malloc(alloc);
    if (!ns) return CURLE_OUT_OF_MEMORY;
    while(--alloc > 0) {
        unsigned char in = *string;
        if (('%' == in) && (alloc > 2) && ISXDIGIT(string[1]) && ISXDIGIT(string[2])) {
            char hexstr[3];
            char *ptr;
            hexstr[0] = string[1];
            hexstr[1] = string[2];
            hexstr[2] = 0;
            hex = strtoul(hexstr, &ptr, 16);
            in = curlx_ultouc(hex);
            if(data) {
                result = Curl_convert_from_network(data, (char *)&in, 1);
                if(result) {
                    free(ns);
                    return result;
                }
            }
            string += 2;
            alloc -= 2;
        }
        if (((ctrl == REJECT_CTRL) && (in < 0x20)) ||
            ((ctrl == REJECT_ZERO) && (in == 0))) {
            free(ns);
            return CURLE_URL_MALFORMAT;
        }
        ns[strindex++] = in;
        string++;
    }
    ns[strindex] = 0;
    if (olen) *olen = strindex;
    *ostring = ns;
    return CURLE_OK;
}
char *curl_easy_unescape(CURL *data, const char *string, int length, int *olen) {
    char *str = NULL;
    if(length >= 0) {
        size_t inputlen = length;
        size_t outputlen;
        CURLcode res = Curl_urldecode(data, string, inputlen, &str, &outputlen, REJECT_NADA);
        if(res) return NULL;
        if(olen) {
            if(outputlen <= (size_t) INT_MAX) *olen = curlx_uztosi(outputlen);
            else Curl_safefree(str);
        }
    }
    return str;
}
void curl_free(void *p) { free(p); }
