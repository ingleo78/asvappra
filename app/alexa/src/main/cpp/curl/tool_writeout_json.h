#ifndef HEADER_CURL_TOOL_WRITEOUT_JSON_H
#define HEADER_CURL_TOOL_WRITEOUT_JSON_H

#include "tool_setup.h"
#include "tool_writeout.h"

void ourWriteOutJSON(const struct writeoutvar mappings[], CURL *curl, struct OutStruct *outs, FILE *stream);

#endif