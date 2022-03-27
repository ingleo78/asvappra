#ifndef HEADER_CURL_TOOL_LIBINFO_H
#define HEADER_CURL_TOOL_LIBINFO_H

#include "tool_setup.h"

extern curl_version_info_data *curlinfo;
extern long built_in_protos;
CURLcode get_libcurl_info(void);

#endif