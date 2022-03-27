#ifndef HEADER_CURL_MIME_H
#define HEADER_CURL_MIME_H

#include <stdio.h>
#include "curl_setup.h"

#define MIME_RAND_BOUNDARY_CHARS        16
#define MAX_ENCODED_LINE_LENGTH         76
#define ENCODING_BUFFER_SIZE            256
#define MIME_USERHEADERS_OWNER  (1 << 0)
#define MIME_BODY_ONLY          (1 << 1)
#define MIME_FAST_READ          (1 << 2)
#define FILE_CONTENTTYPE_DEFAULT        "application/octet-stream"
#define MULTIPART_CONTENTTYPE_DEFAULT   "multipart/mixed"
#define DISPOSITION_DEFAULT             "attachment"
#define size_t unsigned int
enum mimekind {
    MIMEKIND_NONE = 0,
    MIMEKIND_DATA,
    MIMEKIND_FILE,
    MIMEKIND_CALLBACK,
    MIMEKIND_MULTIPART,
    MIMEKIND_LAST
};
enum mimestate {
    MIMESTATE_BEGIN,
    MIMESTATE_CURLHEADERS,
    MIMESTATE_USERHEADERS,
    MIMESTATE_EOH,
    MIMESTATE_BODY,
    MIMESTATE_BOUNDARY1,
    MIMESTATE_BOUNDARY2,
    MIMESTATE_CONTENT,
    MIMESTATE_END,
    MIMESTATE_LAST
};
enum mimestrategy {
    MIMESTRATEGY_MAIL,
    MIMESTRATEGY_FORM,
    MIMESTRATEGY_LAST
};
struct mime_encoder {
    const char *   name;
    size_t         (*encodefunc)(char *buffer, size_t size, bool ateof, curl_mimepart *part);
    curl_off_t     (*sizefunc)(curl_mimepart *part);
};
struct mime_encoder_state {
    size_t         pos;
    size_t         bufbeg;
    size_t         bufend;
    char           buf[ENCODING_BUFFER_SIZE];
};
struct mime_state {
    enum mimestate state;
    void *ptr;
    curl_off_t offset;
};
#define MIME_BOUNDARY_LEN (24 + MIME_RAND_BOUNDARY_CHARS + 1)
struct curl_mime {
    struct Curl_easy *easy;
    curl_mimepart *parent;
    curl_mimepart *firstpart;
    curl_mimepart *lastpart;
    char boundary[MIME_BOUNDARY_LEN];
    struct mime_state state;
};
struct curl_mimepart {
    struct Curl_easy *easy;
    curl_mime *parent;
    curl_mimepart *nextpart;
    enum mimekind kind;
    char *data;
    curl_read_callback readfunc;
    curl_seek_callback seekfunc;
    curl_free_callback freefunc;
    void *arg;
    FILE *fp;
    struct curl_slist *curlheaders;
    struct curl_slist *userheaders;
    char *mimetype;
    char *filename;
    char *name;
    curl_off_t datasize;
    unsigned int flags;
    struct mime_state state;
    const struct mime_encoder *encoder;
    struct mime_encoder_state encstate;
    size_t lastreadstatus;
};
CURLcode Curl_mime_add_header(struct curl_slist **slp, const char *fmt, ...);
#if (!defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_MIME)) || !defined(CURL_DISABLE_SMTP) || !defined(CURL_DISABLE_IMAP)
void Curl_mime_initpart(struct curl_mimepart *part, struct Curl_easy *easy);
void Curl_mime_cleanpart(struct curl_mimepart *part);
CURLcode Curl_mime_duppart(struct curl_mimepart *dst, const curl_mimepart *src);
CURLcode Curl_mime_set_subparts(struct curl_mimepart *part, struct curl_mime *subparts, int take_ownership);
CURLcode Curl_mime_prepare_headers(struct curl_mimepart *part, const char *contenttype, const char *disposition, enum mimestrategy strategy);
curl_off_t Curl_mime_size(struct curl_mimepart *part);
size_t Curl_mime_read(char *buffer, size_t size, size_t nitems, void *instream);
CURLcode Curl_mime_rewind(struct curl_mimepart *part);
const char *Curl_mime_contenttype(const char *filename);
void Curl_mime_unpause(struct curl_mimepart *part);
#else
#define Curl_mime_initpart(x,y)
#define Curl_mime_cleanpart(x)
#define Curl_mime_duppart(x,y) CURLE_OK
#define Curl_mime_set_subparts(a,b,c) CURLE_NOT_BUILT_IN
#define Curl_mime_prepare_headers(a,b,c,d) CURLE_NOT_BUILT_IN
#define Curl_mime_size(x) (curl_off_t) -1
#define Curl_mime_read NULL
#define Curl_mime_rewind(x) ((void)x, CURLE_NOT_BUILT_IN)
#define Curl_mime_unpause(x)
#endif
#endif