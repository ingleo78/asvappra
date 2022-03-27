#ifndef HEADER_CURL_TOOL_GETPASS_H
#define HEADER_CURL_TOOL_GETPASS_H

#include "tool_setup.h"

#ifndef HAVE_GETPASS_R
char *getpass_r(const char *prompt, char *buffer, size_t buflen);
#endif

#endif