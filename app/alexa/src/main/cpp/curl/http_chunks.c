#include <assert.h>
#include "curl_setup.h"
#include "curl_ctype.h"

#ifndef CURL_DISABLE_HTTP
#include "urldata.h"
#include "sendf.h"
#include "dynbuf.h"
#include "content_encoding.h"
#include "http.h"
#include "non-ascii.h"
#include "strtoofft.h"
#include "warnless.h"
#include "curl_memory.h"
#include "memdebug.h"
#include "http_chunks.h"

#ifdef CURL_DOES_CONVERSIONS
static bool Curl_isxdigit_ascii(char digit) {
    return (digit >= 0x30 && digit <= 0x39) || (digit >= 0x41 && digit <= 0x46) || (digit >= 0x61 && digit <= 0x66);
}
#else
#define Curl_isxdigit_ascii(x) Curl_isxdigit(x)
#endif
void Curl_httpchunk_init(struct connectdata *conn) {
    struct Curl_chunker *chunk = &conn->chunk;
    chunk->hexindex = 0;
    chunk->dataleft = 0;
    chunk->state = CHUNK_HEX;
    Curl_dyn_init(&conn->trailer, DYN_H1_TRAILER);
}
CHUNKcode Curl_httpchunk_read(struct connectdata *conn, char *datap, ssize_t datalen, ssize_t *wrotep, CURLcode *extrap) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    struct Curl_chunker *ch = &conn->chunk;
    struct SingleRequest *k = &data->req;
    size_t piece;
    curl_off_t length = (curl_off_t)datalen;
    size_t *wrote = (size_t *)wrotep;
    *wrote = 0;
    if(data->set.http_te_skip && !k->ignorebody) {
        result = Curl_client_write(conn, CLIENTWRITE_BODY, datap, datalen);
        if(result) {
            *extrap = result;
            return CHUNKE_PASSTHRU_ERROR;
        }
    }
    while(length) {
        switch(ch->state) {
            case CHUNK_HEX:
                if (Curl_isxdigit_ascii(*datap)) {
                    if (ch->hexindex < MAXNUM_SIZE) {
                        ch->hexbuffer[ch->hexindex] = *datap;
                        datap++;
                        length--;
                        ch->hexindex++;
                    } else  return CHUNKE_TOO_LONG_HEX;
                } else {
                    char *endptr;
                    if (0 == ch->hexindex) return CHUNKE_ILLEGAL_HEX;
                    ch->hexbuffer[ch->hexindex] = 0;
                    result = Curl_convert_from_network(conn->data, ch->hexbuffer, ch->hexindex);
                    if (result) return CHUNKE_ILLEGAL_HEX;
                    if (curlx_strtoofft(ch->hexbuffer, &endptr, 16, &ch->datasize)) return CHUNKE_ILLEGAL_HEX;
                    ch->state = CHUNK_LF;
                }
                break;
            case CHUNK_LF:
                if (*datap == 0x0a) {
                    if (0 == ch->datasize) ch->state = CHUNK_TRAILER; /* now check for trailers */
                    else ch->state = CHUNK_DATA;
                }
                datap++;
                length--;
                break;
            case CHUNK_DATA:
                piece = curlx_sotouz((ch->datasize >= length)?length:ch->datasize);
                if (!conn->data->set.http_te_skip && !k->ignorebody) {
                    if (!conn->data->set.http_ce_skip && k->writer_stack) result = Curl_unencode_write(conn, k->writer_stack, datap, piece);
                    else result = Curl_client_write(conn, CLIENTWRITE_BODY, datap, piece);
                    if (result) {
                        *extrap = result;
                        return CHUNKE_PASSTHRU_ERROR;
                    }
                }
                *wrote += piece;
                ch->datasize -= piece;
                datap += piece;
                length -= piece;
                if (0 == ch->datasize) ch->state = CHUNK_POSTLF;
                break;
            case CHUNK_POSTLF:
                if (*datap == 0x0a) Curl_httpchunk_init(conn);
                else if (*datap != 0x0d) return CHUNKE_BAD_CHUNK;
                datap++;
                length--;
                break;
            case CHUNK_TRAILER:
                if((*datap == 0x0d) || (*datap == 0x0a)) {
                    char *tr = Curl_dyn_ptr(&conn->trailer);
                    if(tr) {
                        size_t trlen;
                        result = Curl_dyn_add(&conn->trailer, (char *)"\x0d\x0a");
                        if(result) return CHUNKE_OUT_OF_MEMORY;
                        tr = Curl_dyn_ptr(&conn->trailer);
                        trlen = Curl_dyn_len(&conn->trailer);
                        result = Curl_convert_from_network(conn->data, tr, trlen);
                        if(result) return CHUNKE_BAD_CHUNK;
                        if(!data->set.http_te_skip) {
                            result = Curl_client_write(conn, CLIENTWRITE_HEADER, tr, trlen);
                            if(result) {
                                *extrap = result;
                                return CHUNKE_PASSTHRU_ERROR;
                            }
                        }
                        Curl_dyn_reset(&conn->trailer);
                        ch->state = CHUNK_TRAILER_CR;
                        if(*datap == 0x0a) break;
                    } else {
                        ch->state = CHUNK_TRAILER_POSTCR;
                        break;
                    }
                } else {
                    result = Curl_dyn_addn(&conn->trailer, datap, 1);
                    if (result) return CHUNKE_OUT_OF_MEMORY;
                }
                datap++;
                length--;
                break;
            case CHUNK_TRAILER_CR:
                if (*datap == 0x0a) {
                    ch->state = CHUNK_TRAILER_POSTCR;
                    datap++;
                    length--;
                } else return CHUNKE_BAD_CHUNK;
                break;
            case CHUNK_TRAILER_POSTCR:
                if ((*datap != 0x0d) && (*datap != 0x0a)) {
                    ch->state = CHUNK_TRAILER;
                    break;
                }
                if (*datap == 0x0d) {
                    datap++;
                    length--;
                }
                ch->state = CHUNK_STOP;
                break;
            case CHUNK_STOP:
                if (*datap == 0x0a) {
                    length--;
                    ch->dataleft = curlx_sotouz(length);
                    return CHUNKE_STOP;
                } else return CHUNKE_BAD_CHUNK;
        }
    }
    return CHUNKE_OK;
}
const char *Curl_chunked_strerror(CHUNKcode code) {
    switch(code) {
        case CHUNKE_TOO_LONG_HEX: return "Too long hexadecimal number";
        case CHUNKE_ILLEGAL_HEX: return "Illegal or missing hexadecimal sequence";
        case CHUNKE_BAD_CHUNK: return "Malformed encoding found";
        case CHUNKE_PASSTHRU_ERROR: DEBUGASSERT(0); return "";
        case CHUNKE_BAD_ENCODING: return "Bad content-encoding found";
        case CHUNKE_OUT_OF_MEMORY: return "Out of memory";
        default: return "OK";
    }
}
#endif