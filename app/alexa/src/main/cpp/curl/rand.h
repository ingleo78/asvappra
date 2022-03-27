#ifndef HEADER_CURL_RAND_H
#define HEADER_CURL_RAND_H

#include "curl_setup.h"
#include "curl.h"
#include "vtls/vtls.h"
#include "sendf.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
#include "timeval.h"

static CURLcode randit(struct Curl_easy *data, unsigned int *rnd);
CURLcode Curl_rand(struct Curl_easy *data, unsigned char *rnd, size_t num) {
    CURLcode result = CURLE_BAD_FUNCTION_ARGUMENT;
    DEBUGASSERT(num > 0);
    while(num) {
        unsigned int r;
        size_t left = num < sizeof(unsigned int) ? num : sizeof(unsigned int);
        result = randit(data, &r);
        if(result) return result;
        while(left) {
            *rnd++ = (unsigned char)(r & 0xFF);
            r >>= 8;
            --num;
            --left;
        }
    }
    return result;
}
CURLcode Curl_rand_hex(struct Curl_easy *data, unsigned char *rnd, size_t num) {
    CURLcode result = CURLE_BAD_FUNCTION_ARGUMENT;
    const char *hex = "0123456789abcdef";
    unsigned char buffer[128];
    unsigned char *bufp = buffer;
    DEBUGASSERT(num > 1);
#ifdef __clang_analyzer__
    memset(buffer, 0, sizeof(buffer));
#endif
    if((num/2 >= sizeof(buffer)) || !(num&1)) return CURLE_BAD_FUNCTION_ARGUMENT;
    num--;
    result = Curl_rand(data, buffer, num/2);
    if(result) return result;
    while(num) {
        *rnd++ = hex[(*bufp & 0xF0)>>4];
        *rnd++ = hex[*bufp & 0x0F];
        bufp++;
        num -= 2;
    }
    *rnd = 0;
    return result;
}
#endif