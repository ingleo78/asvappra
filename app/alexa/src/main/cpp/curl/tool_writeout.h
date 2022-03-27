#ifndef HEADER_CURL_TOOL_WRITEOUT_H
#define HEADER_CURL_TOOL_WRITEOUT_H

#include "tool_setup.h"
#include "easy.h"

typedef enum {
    VAR_NONE,
    VAR_TOTAL_TIME,
    VAR_NAMELOOKUP_TIME,
    VAR_CONNECT_TIME,
    VAR_APPCONNECT_TIME,
    VAR_PRETRANSFER_TIME,
    VAR_STARTTRANSFER_TIME,
    VAR_SIZE_DOWNLOAD,
    VAR_SIZE_UPLOAD,
    VAR_SPEED_DOWNLOAD,
    VAR_SPEED_UPLOAD,
    VAR_HTTP_CODE,
    VAR_HTTP_CODE_PROXY,
    VAR_HEADER_SIZE,
    VAR_REQUEST_SIZE,
    VAR_EFFECTIVE_URL,
    VAR_CONTENT_TYPE,
    VAR_NUM_CONNECTS,
    VAR_REDIRECT_TIME,
    VAR_REDIRECT_COUNT,
    VAR_FTP_ENTRY_PATH,
    VAR_REDIRECT_URL,
    VAR_SSL_VERIFY_RESULT,
    VAR_PROXY_SSL_VERIFY_RESULT,
    VAR_EFFECTIVE_FILENAME,
    VAR_PRIMARY_IP,
    VAR_PRIMARY_PORT,
    VAR_LOCAL_IP,
    VAR_LOCAL_PORT,
    VAR_HTTP_VERSION,
    VAR_SCHEME,
    VAR_STDOUT,
    VAR_STDERR,
    VAR_JSON,
    VAR_NUM_OF_VARS
} writeoutid;
typedef enum {
    JSON_NONE,
    JSON_STRING,
    JSON_LONG,
    JSON_OFFSET,
    JSON_TIME,
    JSON_VERSION,
    JSON_FILENAME
} jsontype;
struct writeoutvar {
    const char *name;
    writeoutid id;
    int is_ctrl;
    CURLINFO cinfo;
    jsontype jsontype;
};
void ourWriteOut(CURL *curl, struct OutStruct *outs, const char *writeinfo);

#endif