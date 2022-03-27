#ifndef HEADER_CURL_TOOL_UTIL_H
#define HEADER_CURL_TOOL_UTIL_H

#include "tool_setup.h"

struct timeval tvnow(void);
long tvdiff(struct timeval t1, struct timeval t2);

#endif