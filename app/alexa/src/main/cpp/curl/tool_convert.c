#include "tool_setup.h"
#ifdef CURL_DOES_CONVERSIONS
#ifdef HAVE_ICONV
#include <iconv.h>
#endif
#include "tool_convert.h"
#include "memdebug.h"

#ifdef HAVE_ICONV
static iconv_t inbound_cd  = (iconv_t)-1;
static iconv_t outbound_cd = (iconv_t)-1;
#ifndef CURL_ICONV_CODESET_OF_NETWORK
#define CURL_ICONV_CODESET_OF_NETWORK "ISO8859-1"
#endif

CURLcode convert_to_network(char *buffer, size_t length) {
    char *input_ptr, *output_ptr;
    size_t res, in_bytes, out_bytes;
    if(outbound_cd == (iconv_t)-1) {
        outbound_cd = iconv_open(CURL_ICONV_CODESET_OF_NETWORK, CURL_ICONV_CODESET_OF_HOST);
        if(outbound_cd == (iconv_t)-1) return CURLE_CONV_FAILED;
    }
    input_ptr = output_ptr = buffer;
    in_bytes = out_bytes = length;
    res = iconv(outbound_cd, &input_ptr,  &in_bytes, &output_ptr, &out_bytes);
    if((res == (size_t)-1) || (in_bytes != 0)) return CURLE_CONV_FAILED;
    return CURLE_OK;
}
CURLcode convert_from_network(char *buffer, size_t length) {
    char *input_ptr, *output_ptr;
    size_t res, in_bytes, out_bytes;
    if(inbound_cd == (iconv_t)-1) {
        inbound_cd = iconv_open(CURL_ICONV_CODESET_OF_HOST, CURL_ICONV_CODESET_OF_NETWORK);
        if(inbound_cd == (iconv_t)-1) return CURLE_CONV_FAILED;
    }
    input_ptr = output_ptr = buffer;
    in_bytes = out_bytes = length;
    res = iconv(inbound_cd, &input_ptr,  &in_bytes, &output_ptr, &out_bytes);
    if((res == (size_t)-1) || (in_bytes != 0)) return CURLE_CONV_FAILED;
    return CURLE_OK;
}
void convert_cleanup(void) {
    if(inbound_cd != (iconv_t)-1) (void)iconv_close(inbound_cd);
    if(outbound_cd != (iconv_t)-1) (void)iconv_close(outbound_cd);
}
#endif
char convert_char(curl_infotype infotype, char this_char) {
    switch(infotype) {
        case CURLINFO_DATA_IN: case CURLINFO_DATA_OUT: case CURLINFO_SSL_DATA_IN: case CURLINFO_SSL_DATA_OUT:
            if(this_char < 0x20 || this_char >= 0x7f) return UNPRINTABLE_CHAR;
            (void)convert_from_network(&this_char, 1);
        default:
            if(ISPRINT(this_char) &&  (this_char != '\t') &&  (this_char != '\r') &&  (this_char != '\n')) return this_char;
            break;
    }
    return UNPRINTABLE_CHAR;
}
#endif