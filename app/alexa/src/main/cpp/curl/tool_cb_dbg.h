#ifndef HEADER_CURL_TOOL_CB_DBG_H
#define HEADER_CURL_TOOL_CB_DBG_H

#include "tool_setup.h"

int tool_debug_cb(CURL *handle, curl_infotype type, char *data, size_t size, void *userdata);

#endif