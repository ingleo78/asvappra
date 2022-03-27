#ifndef HEADER_CURL_TOOL_SDECLS_H
#define HEADER_CURL_TOOL_SDECLS_H

#include "tool_setup.h"
#ifdef USE_METALINK
#include <metalink/metalink.h>
#endif

struct OutStruct {
    char *filename;
    bool alloc_filename;
    bool is_cd_filename;
    bool s_isreg;
    bool fopened;
    FILE *stream;
    curl_off_t bytes;
    curl_off_t init;
#ifdef USE_METALINK
    metalink_parser_context_t *metalink_parser;
#endif
};
struct InStruct {
    int fd;
    struct OperationConfig *config;
};
struct getout {
    struct getout *next;
    char *url;
    char *outfile;
    char *infile;
    int flags;
};
#define GETOUT_OUTFILE    (1<<0)
#define GETOUT_URL        (1<<1)
#define GETOUT_USEREMOTE  (1<<2)
#define GETOUT_UPLOAD     (1<<3)
#define GETOUT_NOUPLOAD   (1<<4)
#define GETOUT_METALINK   (1<<5)
typedef enum {
    TRACE_NONE,
    TRACE_BIN,
    TRACE_ASCII,
    TRACE_PLAIN
} trace;
typedef enum {
    HTTPREQ_UNSPEC,
    HTTPREQ_GET,
    HTTPREQ_HEAD,
    HTTPREQ_MIMEPOST,
    HTTPREQ_SIMPLEPOST
} HttpReq;
#include "tool_cfgable.h"

#endif