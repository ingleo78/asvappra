#ifndef HEADER_CURL_TOOL_BNAME_H
#define HEADER_CURL_TOOL_BNAME_H

#include "tool_setup.h"

#ifndef HAVE_BASENAME
char *tool_basename(char *path);
#define basename(x) tool_basename((x))
#endif
#endif