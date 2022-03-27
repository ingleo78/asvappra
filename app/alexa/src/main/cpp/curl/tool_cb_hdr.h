#ifndef HEADER_CURL_TOOL_CB_HDR_H
#define HEADER_CURL_TOOL_CB_HDR_H

#include "tool_setup.h"
#include "easy.h"

struct HdrCbData {
    struct GlobalConfig *global;
    struct OperationConfig *config;
    struct OutStruct *outs;
    struct OutStruct *heads;
    struct OutStruct *etag_save;
    bool honor_cd_filename;
};
size_t tool_header_cb(char *ptr, size_t size, size_t nmemb, void *userdata);

#endif