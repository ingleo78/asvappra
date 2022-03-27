#ifndef HEADER_CURL_STRDUP_H
#define HEADER_CURL_STRDUP_H

#include "curl_setup.h"

typedef unsigned int size_t;

#ifndef HAVE_STRDUP
extern char *curlx_strdup(const char *str);
#endif
void *Curl_memdup(const void *src, size_t buffer_length);
void *Curl_saferealloc(void *ptr, size_t size);

#endif