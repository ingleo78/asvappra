#ifndef HEADER_CURL_NON_ASCII_H
#define HEADER_CURL_NON_ASCII_H

#include "curl_setup.h"

#ifdef CURL_DOES_CONVERSIONS
#if __ANDROID_API__ > 28
#include "urldata.h"
CURLcode Curl_convert_clone(struct Curl_easy *data, const char *indata, size_t insize, char **outbuf);
void Curl_convert_init(struct Curl_easy *data);
void Curl_convert_setup(struct Curl_easy *data);
void Curl_convert_close(struct Curl_easy *data);
CURLcode Curl_convert_to_network(struct Curl_easy *data, char *buffer, size_t length);
CURLcode Curl_convert_from_network(struct Curl_easy *data, char *buffer, size_t length);
CURLcode Curl_convert_from_utf8(struct Curl_easy *data, char *buffer, size_t length);
#endif
#else
#define Curl_convert_clone(a,b,c,d) ((void)a, CURLE_OK)
#define Curl_convert_init(x) Curl_nop_stmt
#define Curl_convert_setup(x) Curl_nop_stmt
#define Curl_convert_close(x) Curl_nop_stmt
#define Curl_convert_to_network(a,b,c) ((void)a, CURLE_OK)
#define Curl_convert_from_network(a,b,c) ((void)a, CURLE_OK)
#define Curl_convert_from_utf8(a,b,c) ((void)a, CURLE_OK)
#endif

#endif