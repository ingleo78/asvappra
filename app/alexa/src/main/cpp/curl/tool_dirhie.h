#ifndef HEADER_CURL_TOOL_DIRHIE_H
#define HEADER_CURL_TOOL_DIRHIE_H

#include <stdio.h>
#include "tool_setup.h"

CURLcode create_dir_hierarchy(const char *outfile, FILE *errors);

#endif