#ifndef HEADER_CURL_CONTENT_ENCODING_H
#define HEADER_CURL_CONTENT_ENCODING_H

#include "curl_setup.h"

struct contenc_writer {
    const struct content_encoding *handler;
    struct contenc_writer *downstream;
    void *params;
};
struct content_encoding {
    const char *name;
    const char *alias;
    CURLcode (*init_writer)(struct connectdata *conn, struct contenc_writer *writer);
    CURLcode (*unencode_write)(struct connectdata *conn, struct contenc_writer *writer, const char *buf, size_t nbytes);
    void (*close_writer)(struct connectdata *conn, struct contenc_writer *writer);
    size_t paramsize;
};
CURLcode Curl_build_unencoding_stack(struct connectdata *conn, const char *enclist, int maybechunked);
CURLcode Curl_unencode_write(struct connectdata *conn, struct contenc_writer *writer, const char *buf, size_t nbytes);
void Curl_unencode_cleanup(struct connectdata *conn);
char *Curl_all_content_encodings(void);
#endif