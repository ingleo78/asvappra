#ifndef HEADER_CURL_TOOL_CB_WRT_H
#define HEADER_CURL_TOOL_CB_WRT_H

#include "tool_setup.h"

size_t tool_write_cb(char *buffer, size_t sz, size_t nmemb, void *userdata);
bool tool_create_output_file(struct OutStruct *outs, struct OperationConfig *config);

#endif