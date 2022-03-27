#ifndef HEADER_CURL_TOOL_PROGRESS_H
#define HEADER_CURL_TOOL_PROGRESS_H

#include "tool_setup.h"

int xferinfo_cb(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
bool progress_meter(struct GlobalConfig *global, struct timeval *start, bool final);
void progress_finalize(struct per_transfer *per);
extern curl_off_t all_xfers;
#endif