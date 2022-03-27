#ifndef HEADER_CURL_TOOL_CB_REA_H
#define HEADER_CURL_TOOL_CB_REA_H

#include "tool_setup.h"

size_t tool_read_cb(void *buffer, size_t sz, size_t nmemb, void *userdata);
int tool_readbusy_cb(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

#endif