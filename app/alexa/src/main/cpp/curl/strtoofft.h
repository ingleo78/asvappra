#ifndef HEADER_CURL_STRTOOFFT_H
#define HEADER_CURL_STRTOOFFT_H

#include "curl_setup.h"

typedef enum {
    CURL_OFFT_OK,
    CURL_OFFT_FLOW,
    CURL_OFFT_INVAL
} CURLofft;
CURLofft curlx_strtoofft(const char *str, char **endp, int base, curl_off_t *num);

#endif