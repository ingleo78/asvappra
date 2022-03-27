#ifndef HEADER_CURL_TOOL_PANYKEY_H
#define HEADER_CURL_TOOL_PANYKEY_H

#include "tool_setup.h"

#if defined(__SYMBIAN32__) || defined(NETWARE)
void tool_pressanykey(void);
#else
#define tool_pressanykey() Curl_nop_stmt
#endif

#endif