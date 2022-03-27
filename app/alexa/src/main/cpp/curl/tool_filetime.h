#ifndef HEADER_CURL_TOOL_FILETIME_H
#define HEADER_CURL_TOOL_FILETIME_H

#include <stdio.h>
#include "tool_setup.h"

curl_off_t getfiletime(const char *filename, FILE *error_stream);
#if defined(HAVE_UTIME) || defined(HAVE_UTIMES) || (defined(WIN32) && (SIZEOF_CURL_OFF_T >= 8))
void setfiletime(curl_off_t filetime, const char *filename, FILE *error_stream);
#else
#define setfiletime(a,b,c) Curl_nop_stmt
#endif

#endif