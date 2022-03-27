#ifndef HEADER_CURL_TOOL_CONVERT_H
#define HEADER_CURL_TOOL_CONVERT_H

#include "tool_setup.h"

#ifdef CURL_DOES_CONVERSIONS
#ifdef HAVE_ICONV
CURLcode convert_to_network(char *buffer, size_t length);
CURLcode convert_from_network(char *buffer, size_t length);
void convert_cleanup(void);
#endif
char convert_char(curl_infotype infotype, char this_char);
#endif
#if !defined(CURL_DOES_CONVERSIONS) || !defined(HAVE_ICONV)
#define convert_cleanup() Curl_nop_stmt
#endif

#endif