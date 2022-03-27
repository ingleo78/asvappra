#ifndef HEADER_CURL_TOOL_FORMPARSE_H
#define HEADER_CURL_TOOL_FORMPARSE_H

#include "tool_setup.h"

typedef enum {
    TOOLMIME_NONE = 0,
    TOOLMIME_PARTS,
    TOOLMIME_DATA,
    TOOLMIME_FILE,
    TOOLMIME_FILEDATA,
    TOOLMIME_STDIN,
    TOOLMIME_STDINDATA
} toolmimekind;
struct tool_mime {
    toolmimekind kind;
    struct tool_mime *parent;
    struct tool_mime *prev;
    const char *data;
    const char *name;
    const char *filename;
    const char *type;
    const char *encoder;
    struct curl_slist *headers;
    struct tool_mime *subparts;
    curl_off_t origin;
    curl_off_t size;
    curl_off_t curpos;
    struct GlobalConfig *config;
};
size_t tool_mime_stdin_read(char *buffer, size_t size, size_t nitems, void *arg);
int tool_mime_stdin_seek(void *instream, curl_off_t offset, int whence);
int formparse(struct OperationConfig *config, const char *input, struct tool_mime **mimeroot, struct tool_mime **mimecurrent, bool literal_value);
CURLcode tool2curlmime(CURL *curl, struct tool_mime *m, curl_mime **mime);
void tool_mime_free(struct tool_mime *mime);

#endif