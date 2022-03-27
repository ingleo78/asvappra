#ifndef HEADER_CURL_HTTP_CHUNKS_H
#define HEADER_CURL_HTTP_CHUNKS_H

struct connectdata;
#define MAXNUM_SIZE 16
typedef enum {
    CHUNK_HEX,
    CHUNK_LF,
    CHUNK_DATA,
    CHUNK_POSTLF,
    CHUNK_STOP,
    CHUNK_TRAILER,
    CHUNK_TRAILER_CR,
    CHUNK_TRAILER_POSTCR
} ChunkyState;
typedef enum {
    CHUNKE_STOP = -1,
    CHUNKE_OK = 0,
    CHUNKE_TOO_LONG_HEX = 1,
    CHUNKE_ILLEGAL_HEX,
    CHUNKE_BAD_CHUNK,
    CHUNKE_BAD_ENCODING,
    CHUNKE_OUT_OF_MEMORY,
    CHUNKE_PASSTHRU_ERROR,
    CHUNKE_LAST
} CHUNKcode;
const char *Curl_chunked_strerror(CHUNKcode code);
struct Curl_chunker {
    char hexbuffer[ MAXNUM_SIZE + 1];
    int hexindex;
    ChunkyState state;
    curl_off_t datasize;
    size_t dataleft;
};
void Curl_httpchunk_init(struct connectdata *conn);
CHUNKcode Curl_httpchunk_read(struct connectdata *conn, char *datap, ssize_t length, ssize_t *wrote, CURLcode *passthru);
#endif