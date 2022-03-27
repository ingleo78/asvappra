#include "curl_setup.h"

#if !defined(CURL_DISABLE_HTTP_AUTH) || defined(USE_SSH) || !defined(CURL_DISABLE_LDAP) || !defined(CURL_DISABLE_DOH) || defined(USE_SSL)
#include "urldata.h"
#include "warnless.h"
#include "curl_base64.h"
#include "non-ascii.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

static const char base64[]= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char base64url[]= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static size_t decodeQuantum(unsigned char *dest, const char *src) {
    size_t padding = 0;
    const char *s, *p;
    unsigned long i, x = 0;
    for(i = 0, s = src; i < 4; i++, s++) {
        if(*s == '=') {
            x = (x << 6);
            padding++;
        } else {
            unsigned long v = 0;
            p = base64;
            while(*p && (*p != *s)) {
                v++;
                p++;
            }
            if(*p == *s) x = (x << 6) + v;
            else return 0;
        }
    }
    if(padding < 1) dest[2] = curlx_ultouc(x & 0xFFUL);
    x >>= 8;
    if(padding < 2) dest[1] = curlx_ultouc(x & 0xFFUL);
    x >>= 8; dest[0] = curlx_ultouc(x & 0xFFUL);
    return 3 - padding;
}
CURLcode Curl_base64_decode(const char *src, unsigned char **outptr, size_t *outlen) {
    size_t srclen = 0;
    size_t length = 0;
    size_t padding = 0;
    size_t i;
    size_t numQuantums;
    size_t rawlen = 0;
    unsigned char *pos;
    unsigned char *newstr;
    *outptr = NULL;
    *outlen = 0;
    srclen = strlen(src);
    if (!srclen || srclen % 4) return CURLE_BAD_CONTENT_ENCODING;
    while((src[length] != '=') && src[length]) length++;
    if (src[length] == '=') {
        padding++;
        if (src[length + 1] == '=') padding++;
    }
    if (length + padding != srclen) return CURLE_BAD_CONTENT_ENCODING;
    numQuantums = srclen / 4;
    rawlen = (numQuantums * 3) - padding;
    newstr = malloc(rawlen + 1);
    if (!newstr) return CURLE_OUT_OF_MEMORY;
    pos = newstr;
    for (i = 0; i < numQuantums; i++) {
        size_t result = decodeQuantum(pos, src);
        if (!result) {
            free(newstr);
            return CURLE_BAD_CONTENT_ENCODING;
        }
        pos += result;
        src += 4;
    }
    *pos = '\0';
    *outptr = newstr;
    *outlen = rawlen;
    return CURLE_OK;
}
static CURLcode base64_encode(const char *table64, struct Curl_easy *data, const char *inputbuff, size_t insize, char **outptr, size_t *outlen) {
    CURLcode result;
    unsigned char ibuf[3];
    unsigned char obuf[4];
    int i;
    int inputparts;
    char *output;
    char *base64data;
    char *convbuf = NULL;
    const char *indata = inputbuff;
    *outptr = NULL;
    *outlen = 0;
    if (!insize) insize = strlen(indata);
#if SIZEOF_SIZE_T == 4
    if (insize > UINT_MAX/4) return CURLE_OUT_OF_MEMORY;
#endif
    base64data = output = malloc(insize * 4 / 3 + 4);
    if (!output) return CURLE_OUT_OF_MEMORY;
    result = Curl_convert_clone(data, indata, insize, &convbuf);
    if (result) {
        free(output);
        return result;
    }
    if (convbuf) indata = convbuf;
    while(insize > 0) {
        for(i = inputparts = 0; i < 3; i++) {
            if(insize > 0) {
                inputparts++;
                ibuf[i] = (unsigned char)*indata;
                indata++;
                insize--;
            } else ibuf[i] = 0;
        }
        obuf[0] = (unsigned char)((ibuf[0] & 0xFC) >> 2);
        obuf[1] = (unsigned char)(((ibuf[0] & 0x03) << 4) | ((ibuf[1] & 0xF0) >> 4));
        obuf[2] = (unsigned char)(((ibuf[1] & 0x0F) << 2) | ((ibuf[2] & 0xC0) >> 6));
        obuf[3] = (unsigned char)(ibuf[2] & 0x3F);
        switch(inputparts) {
            case 1: msnprintf(output, 5, "%c%c==", table64[obuf[0]], table64[obuf[1]]); break;
            case 2: msnprintf(output, 5, "%c%c%c=", table64[obuf[0]], table64[obuf[1]], table64[obuf[2]]); break;
            default: msnprintf(output, 5, "%c%c%c%c", table64[obuf[0]], table64[obuf[1]], table64[obuf[2]], table64[obuf[3]]);
        }
        output += 4;
    }
    *output = '\0';
    *outptr = base64data;
    free(convbuf);
    *outlen = strlen(base64data);
    return CURLE_OK;
}
CURLcode Curl_base64_encode(struct Curl_easy *data, const char *inputbuff, size_t insize, char **outptr, size_t *outlen) {
    return base64_encode(base64, data, inputbuff, insize, outptr, outlen);
}
CURLcode Curl_base64url_encode(struct Curl_easy *data, const char *inputbuff, size_t insize, char **outptr, size_t *outlen) {
    return base64_encode(base64url, data, inputbuff, insize, outptr, outlen);
}
#endif