#ifndef HEADER_CURL_STRTOK_H
#define HEADER_CURL_STRTOK_H

#include "curl_setup.h"

#ifndef HAVE_STRTOK_R
char *Curl_strtok_r(char *s, const char *delim, char **last);
#define strtok_r Curl_strtok_r
#else
#include <string.h>
#endif

#endif