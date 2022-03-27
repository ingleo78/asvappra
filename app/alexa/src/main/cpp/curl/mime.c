#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include "curl_setup.h"
#include "curl.h"
#include "mime.h"
#include "non-ascii.h"
#include "warnless.h"
#include "urldata.h"
#include "sendf.h"

#if (!defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_MIME)) || !defined(CURL_DISABLE_SMTP) || !defined(CURL_DISABLE_IMAP)
#if defined(HAVE_LIBGEN_H) && defined(HAVE_BASENAME)
#include <libgen.h>
#endif
#include "rand.h"
#include "slist.h"
#include "strcase.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
#ifdef WIN32
#ifndef R_OK
#define R_OK 4
#endif
#endif
#define READ_ERROR                      ((size_t) -1)
#define STOP_FILLING                    ((size_t) -2)
static size_t mime_subparts_read(char *buffer, size_t size, size_t nitems, void *instream, bool *hasread);
static size_t encoder_nop_read(char *buffer, size_t size, bool ateof, curl_mimepart *part);
static curl_off_t encoder_nop_size(curl_mimepart *part);
static size_t encoder_7bit_read(char *buffer, size_t size, bool ateof, curl_mimepart *part);
static size_t encoder_base64_read(char *buffer, size_t size, bool ateof, curl_mimepart *part);
static curl_off_t encoder_base64_size(curl_mimepart *part);
static size_t encoder_qp_read(char *buffer, size_t size, bool ateof, curl_mimepart *part);
static curl_off_t encoder_qp_size(curl_mimepart *part);
static const struct mime_encoder encoders[] = {
    {"binary", encoder_nop_read, encoder_nop_size},
    {"8bit", encoder_nop_read, encoder_nop_size},
    {"7bit", encoder_7bit_read, encoder_nop_size},
    {"base64", encoder_base64_read, encoder_base64_size},
    {"quoted-printable", encoder_qp_read, encoder_qp_size},
    {ZERO_NULL, ZERO_NULL, ZERO_NULL}
};
static const char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#define QP_OK           1
#define QP_SP           2
#define QP_CR           3
#define QP_LF           4
static const unsigned char qp_class[] = {
    0,     0,     0,     0,     0,     0,     0,     0,
    0,     QP_SP, QP_LF, 0,     0,     QP_CR, 0,     0,
    0,     0,     0,     0,     0,     0,     0,     0,
    0,     0,     0,     0,     0,     0,     0,     0,
    QP_SP, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK,
    QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK,
    QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK,
    QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, 0    , QP_OK, QP_OK,
    QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK,
    QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK,
    QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK,
    QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK,
    QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK,
    QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK,
    QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK,
    QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, QP_OK, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static const char aschex[] = "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x41\x42\x43\x44\x45\x46";
#ifndef __VMS
#define filesize(name, stat_data) (stat_data.st_size)
#define fopen_read fopen
#else
#include <fabdef.h>
curl_off_t VmsRealFileSize(const char *name, const struct_stat *stat_buf) {
    char buffer[8192];
    curl_off_t count;
    int ret_stat;
    FILE * file;
    file = fopen(name, FOPEN_READTEXT);
    if(file == NULL) return 0;
    count = 0;
    ret_stat = 1;
    while(ret_stat > 0) {
        ret_stat = fread(buffer, 1, sizeof(buffer), file);
        if(ret_stat != 0) count += ret_stat;
    }
    fclose(file);
    return count;
}
static curl_off_t VmsSpecialSize(const char *name, const struct_stat *stat_buf) {
    switch(stat_buf->st_fab_rfm) {
        case FAB$C_VAR: case FAB$C_VFC:
            return VmsRealFileSize(name, stat_buf);
            break;
        default: return stat_buf->st_size;
    }
}
#define filesize(name, stat_data) VmsSpecialSize(name, &stat_data)
static FILE * vmsfopenread(const char *file, const char *mode) {
    struct_stat statbuf;
    int result;
    result = stat(file, &statbuf);
    switch(statbuf.st_fab_rfm) {
        case FAB$C_VAR: case FAB$C_VFC: case FAB$C_STMCR:
            return fopen(file, FOPEN_READTEXT);
            break;
        default: return fopen(file, FOPEN_READTEXT, "rfm=stmlf", "ctx=stm");
    }
}
#define fopen_read vmsfopenread
#endif
#ifndef HAVE_BASENAME
static char *Curl_basename(char *path) {
    char *s1;
    char *s2;
    s1 = strrchr(path, '/');
    s2 = strrchr(path, '\\');
    if(s1 && s2) path = (s1 > s2? s1 : s2) + 1;
    else if(s1) path = s1 + 1;
    else if(s2) path = s2 + 1;
    return path;
}
#define basename(x)  Curl_basename((x))
#endif
static void mimesetstate(struct mime_state *state, enum mimestate tok, void *ptr) {
    state->state = tok;
    state->ptr = ptr;
    state->offset = 0;
}
static char *escape_string(const char *src) {
    size_t bytecount = 0;
    size_t i;
    char *dst;
    for(i = 0; src[i]; i++)
        if(src[i] == '"' || src[i] == '\\') bytecount++;
    bytecount += i;
    dst = malloc(bytecount + 1);
    if(!dst) return NULL;
    for(i = 0; *src; src++) {
        if(*src == '"' || *src == '\\') dst[i++] = '\\';
        dst[i++] = *src;
    }
    dst[i] = '\0';
    return dst;
}
static char *match_header(struct curl_slist *hdr, const char *lbl, size_t len) {
    char *value = NULL;
    if(strncasecompare(hdr->data, lbl, len) && hdr->data[len] == ':')
        for(value = hdr->data + len + 1; *value == ' '; value++);
    return value;
}
static char *search_header(struct curl_slist *hdrlist, const char *hdr) {
    size_t len = strlen(hdr);
    char *value = NULL;
    for(; !value && hdrlist; hdrlist = hdrlist->next)
        value = match_header(hdrlist, hdr, len);
    return value;
}
static char *strippath(const char *fullfile) {
    char *filename;
    char *base;
    filename = strdup(fullfile);
    if(!filename) return NULL;
    base = strdup(basename(filename));
    free(filename);
    return base;
}
static void cleanup_encoder_state(struct mime_encoder_state *p) {
    p->pos = 0;
    p->bufbeg = 0;
    p->bufend = 0;
}
static size_t encoder_nop_read(char *buffer, size_t size, bool ateof, struct curl_mimepart *part) {
    struct mime_encoder_state *st = &part->encstate;
    size_t insize = st->bufend - st->bufbeg;
    (void) ateof;
    if(!size) return STOP_FILLING;
    if(size > insize) size = insize;
    if(size) memcpy(buffer, st->buf + st->bufbeg, size);
    st->bufbeg += size;
    return size;
}
static curl_off_t encoder_nop_size(curl_mimepart *part) {
    return part->datasize;
}
static size_t encoder_7bit_read(char *buffer, size_t size, bool ateof, curl_mimepart *part) {
    struct mime_encoder_state *st = &part->encstate;
    size_t cursize = st->bufend - st->bufbeg;
    (void) ateof;
    if(!size) return STOP_FILLING;
    if(size > cursize) size = cursize;
    for(cursize = 0; cursize < size; cursize++) {
        *buffer = st->buf[st->bufbeg];
        if(*buffer++ & 0x80) return cursize? cursize: READ_ERROR;
        st->bufbeg++;
    }
    return cursize;
}
static size_t encoder_base64_read(char *buffer, size_t size, bool ateof, curl_mimepart *part) {
    struct mime_encoder_state *st = &part->encstate;
    size_t cursize = 0;
    int i;
    char *ptr = buffer;
    while(st->bufbeg < st->bufend) {
        if(st->pos > MAX_ENCODED_LINE_LENGTH - 4) {
            if(size < 2) {
              if(!cursize) return STOP_FILLING;
              break;
            }
            *ptr++ = '\r';
            *ptr++ = '\n';
            st->pos = 0;
            cursize += 2;
            size -= 2;
        }
        if(size < 4) {
            if(!cursize) return STOP_FILLING;
            break;
        }
        if(st->bufend - st->bufbeg < 3) break;
        i = st->buf[st->bufbeg++] & 0xFF;
        i = (i << 8) | (st->buf[st->bufbeg++] & 0xFF);
        i = (i << 8) | (st->buf[st->bufbeg++] & 0xFF);
        *ptr++ = base64[(i >> 18) & 0x3F];
        *ptr++ = base64[(i >> 12) & 0x3F];
        *ptr++ = base64[(i >> 6) & 0x3F];
        *ptr++ = base64[i & 0x3F];
        cursize += 4;
        st->pos += 4;
        size -= 4;
    }
    if(ateof) {
        if(size < 4) if(!cursize) return STOP_FILLING;
        else {
            ptr[2] = ptr[3] = '=';
            i = 0;
            switch(st->bufend - st->bufbeg) {
                case 2: i = (st->buf[st->bufbeg + 1] & 0xFF) << 8;
                case 1:
                    i |= (st->buf[st->bufbeg] & 0xFF) << 16;
                    ptr[0] = base64[(i >> 18) & 0x3F];
                    ptr[1] = base64[(i >> 12) & 0x3F];
                    if(++st->bufbeg != st->bufend) {
                        ptr[2] = base64[(i >> 6) & 0x3F];
                        st->bufbeg++;
                    }
                    cursize += 4;
                    st->pos += 4;
                    break;
            }
        }
    }
#ifdef CURL_DOES_CONVERSIONS
    if(part->easy && cursize) {
        CURLcode result = Curl_convert_to_network(part->easy, buffer, cursize);
        if(result) return READ_ERROR;
    }
#endif
    return cursize;
}
static curl_off_t encoder_base64_size(curl_mimepart *part) {
    curl_off_t size = part->datasize;
    if(size <= 0) return size;
    size = 4 * (1 + (size - 1) / 3);
    return size + 2 * ((size - 1) / MAX_ENCODED_LINE_LENGTH);
}
static int qp_lookahead_eol(struct mime_encoder_state *st, int ateof, size_t n) {
    n += st->bufbeg;
    if(n >= st->bufend && ateof) return 1;
    if(n + 2 > st->bufend) return ateof ? 0: -1;
    if(qp_class[st->buf[n] & 0xFF] == QP_CR && qp_class[st->buf[n + 1] & 0xFF] == QP_LF) return 1;
    return 0;
}
static size_t encoder_qp_read(char *buffer, size_t size, bool ateof, curl_mimepart *part) {
    struct mime_encoder_state *st = &part->encstate;
    char *ptr = buffer;
    size_t cursize = 0;
    int softlinebreak;
    char buf[4];
    while(st->bufbeg < st->bufend) {
        size_t len = 1;
        size_t consumed = 1;
        int i = st->buf[st->bufbeg];
        buf[0] = (char) i;
        buf[1] = aschex[(i >> 4) & 0xF];
        buf[2] = aschex[i & 0xF];
        switch(qp_class[st->buf[st->bufbeg] & 0xFF]) {
            case QP_OK:  break;
            case QP_SP:
                switch(qp_lookahead_eol(st, ateof, 1)) {
                    case -1: return cursize;
                    case 0: break;
                    default:
                        buf[0] = '\x3D';
                        len = 3;
                        break;
                }
                break;
            case QP_CR:
                switch(qp_lookahead_eol(st, ateof, 0)) {
                    case -1: return cursize;
                    case 1:
                        buf[len++] = '\x0A';
                        consumed = 2;
                        break;
                    default:
                        buf[0] = '\x3D';
                        len = 3;
                        break;
                }
                break;
            default:
                buf[0] = '\x3D';
                len = 3;
                break;
        }
        if(buf[len - 1] != '\x0A') {
            softlinebreak = st->pos + len > MAX_ENCODED_LINE_LENGTH;
            if(!softlinebreak && st->pos + len == MAX_ENCODED_LINE_LENGTH) {
                switch(qp_lookahead_eol(st, ateof, consumed)) {
                    case -1: return cursize;
                    case 0:
                        softlinebreak = 1;
                        break;
                }
            }
            if(softlinebreak) {
                strcpy(buf, "\x3D\x0D\x0A");
                len = 3;
                consumed = 0;
            }
        }
        if(len > size) {
            if(!cursize) return STOP_FILLING;
            break;
        }
        memcpy(ptr, buf, len);
        cursize += len;
        ptr += len;
        size -= len;
        st->pos += len;
        if(buf[len - 1] == '\x0A') st->pos = 0;
        st->bufbeg += consumed;
    }
    return cursize;
}
static curl_off_t encoder_qp_size(curl_mimepart *part) { return part->datasize ? -1 : 0; }
static size_t mime_mem_read(char *buffer, size_t size, size_t nitems, void *instream) {
    curl_mimepart *part = (curl_mimepart *) instream;
    size_t sz = curlx_sotouz(part->datasize - part->state.offset);
    (void) size;
    if(!nitems) return STOP_FILLING;
    if(sz > nitems) sz = nitems;
    if(sz) memcpy(buffer, part->data + curlx_sotouz(part->state.offset), sz);
    return sz;
}
static int mime_mem_seek(void *instream, curl_off_t offset, int whence) {
    curl_mimepart *part = (curl_mimepart *) instream;
    switch(whence) {
        case SEEK_CUR: offset += part->state.offset; break;
        case SEEK_END: offset += part->datasize; break;
    }
    if(offset < 0 || offset > part->datasize) return CURL_SEEKFUNC_FAIL;
    part->state.offset = offset;
    return CURL_SEEKFUNC_OK;
}
static void mime_mem_free(void *ptr) { Curl_safefree(((curl_mimepart *) ptr)->data); }
static int mime_open_file(curl_mimepart *part) {
    if(part->fp) return 0;
    part->fp = fopen_read(part->data, "rb");
    return part->fp? 0: -1;
}
static size_t mime_file_read(char *buffer, size_t size, size_t nitems, void *instream) {
    curl_mimepart *part = (curl_mimepart *) instream;
    if(!nitems) return STOP_FILLING;
    if(mime_open_file(part)) return READ_ERROR;
    return fread(buffer, size, nitems, part->fp);
}
static int mime_file_seek(void *instream, curl_off_t offset, int whence) {
    curl_mimepart *part = (curl_mimepart *) instream;
    if(whence == SEEK_SET && !offset && !part->fp) return CURL_SEEKFUNC_OK;
    if(mime_open_file(part)) return CURL_SEEKFUNC_FAIL;
    return fseek(part->fp, (long) offset, whence) ? CURL_SEEKFUNC_CANTSEEK : CURL_SEEKFUNC_OK;
}
static void mime_file_free(void *ptr) {
    curl_mimepart *part = (curl_mimepart *) ptr;
    if(part->fp) {
        fclose(part->fp);
        part->fp = NULL;
    }
    Curl_safefree(part->data);
    part->data = NULL;
}
static size_t readback_bytes(struct mime_state *state, char *buffer, size_t bufsize, const char *bytes, size_t numbytes, const char *trail) {
    size_t sz;
    size_t offset = curlx_sotouz(state->offset);
    if(numbytes > offset) {
        sz = numbytes - offset;
        bytes += offset;
    } else {
        size_t tsz = strlen(trail);
        sz = offset - numbytes;
        if(sz >= tsz) return 0;
        bytes = trail + sz;
        sz = tsz - sz;
    }
    if(sz > bufsize) sz = bufsize;
    memcpy(buffer, bytes, sz);
    state->offset += sz;
    return sz;
}
static size_t read_part_content(curl_mimepart *part, char *buffer, size_t bufsize, bool *hasread) {
    size_t sz = 0;
    switch(part->lastreadstatus) {
        case 0: case CURL_READFUNC_ABORT: case CURL_READFUNC_PAUSE: case READ_ERROR: return part->lastreadstatus;
    }
    if(part->datasize != (curl_off_t) -1 && part->state.offset >= part->datasize) {
        /* sz is already zero. */
    } else {
        switch(part->kind) {
        case MIMEKIND_MULTIPART: sz = mime_subparts_read(buffer, 1, bufsize, part->arg, hasread); break;
        case MIMEKIND_FILE: if(part->fp && feof(part->fp)) break;
        default:
            if(part->readfunc) {
                if(!(part->flags & MIME_FAST_READ)) {
                    if(*hasread) return STOP_FILLING;
                    *hasread = TRUE;
                }
                sz = part->readfunc(buffer, 1, bufsize, part->arg);
            }
            break;
        }
    }
    switch(sz) {
        case STOP_FILLING: break;
        case 0: case CURL_READFUNC_ABORT: case CURL_READFUNC_PAUSE: case READ_ERROR: part->lastreadstatus = sz; break;
        default:
            part->state.offset += sz;
            part->lastreadstatus = sz;
            break;
    }
    return sz;
}
static size_t read_encoded_part_content(curl_mimepart *part, char *buffer, size_t bufsize, bool *hasread) {
    struct mime_encoder_state *st = &part->encstate;
    size_t cursize = 0;
    size_t sz;
    bool ateof = FALSE;
    for( ; ; ) {
        if(st->bufbeg < st->bufend || ateof) {
            sz = part->encoder->encodefunc(buffer, bufsize, ateof, part);
            switch(sz) {
                case 0:
                if(ateof) return cursize;
                break;
                case READ_ERROR: case STOP_FILLING: return cursize ? cursize : sz;
                default:
                    cursize += sz;
                    buffer += sz;
                    bufsize -= sz;
                    continue;
            }
        }
        if(st->bufbeg) {
            size_t len = st->bufend - st->bufbeg;
            if(len) memmove(st->buf, st->buf + st->bufbeg, len);
            st->bufbeg = 0;
            st->bufend = len;
        }
        if(st->bufend >= sizeof(st->buf)) return cursize ? cursize : READ_ERROR;
        sz = read_part_content(part, st->buf + st->bufend,sizeof(st->buf) - st->bufend, hasread);
        switch(sz) {
            case 0: ateof = TRUE; break;
            case CURL_READFUNC_ABORT: case CURL_READFUNC_PAUSE: case READ_ERROR: case STOP_FILLING: return cursize ? cursize : sz;
            default: st->bufend += sz; break;
        }
    }
}
static size_t readback_part(curl_mimepart *part, char *buffer, size_t bufsize, bool *hasread) {
    size_t cursize = 0;
#ifdef CURL_DOES_CONVERSIONS
    char *convbuf = buffer;
#endif
    while(bufsize) {
        size_t sz = 0;
        struct curl_slist *hdr = (struct curl_slist *) part->state.ptr;
        switch(part->state.state) {
            case MIMESTATE_BEGIN:
                mimesetstate(&part->state,(part->flags & MIME_BODY_ONLY) ? MIMESTATE_BODY : MIMESTATE_CURLHEADERS, part->curlheaders);
                break;
            case MIMESTATE_USERHEADERS:
                if(!hdr) {
                    mimesetstate(&part->state, MIMESTATE_EOH, NULL);
                    break;
                }
                if(match_header(hdr, "Content-Type", 12)) {
                    mimesetstate(&part->state, MIMESTATE_USERHEADERS, hdr->next);
                    break;
                }
            case MIMESTATE_CURLHEADERS:
                if(!hdr) mimesetstate(&part->state, MIMESTATE_USERHEADERS, part->userheaders);
                else {
                    sz = readback_bytes(&part->state, buffer, bufsize, hdr->data, strlen(hdr->data), "\r\n");
                    if(!sz) mimesetstate(&part->state, part->state.state, hdr->next);
                }
                break;
            case MIMESTATE_EOH:
                sz = readback_bytes(&part->state, buffer, bufsize, "\r\n", 2, "");
                if(!sz) mimesetstate(&part->state, MIMESTATE_BODY, NULL);
                break;
            case MIMESTATE_BODY:
            #ifdef CURL_DOES_CONVERSIONS
                if(part->easy && convbuf < buffer) {
                    CURLcode result = Curl_convert_to_network(part->easy, convbuf, buffer - convbuf);
                    if(result) return READ_ERROR;
                    convbuf = buffer;
                }
            #endif
                cleanup_encoder_state(&part->encstate);
                mimesetstate(&part->state, MIMESTATE_CONTENT, NULL);
                break;
            case MIMESTATE_CONTENT:
                if(part->encoder) sz = read_encoded_part_content(part, buffer, bufsize, hasread);
                else sz = read_part_content(part, buffer, bufsize, hasread);
                switch(sz) {
                case 0:
                mimesetstate(&part->state, MIMESTATE_END, NULL);
                if(part->kind == MIMEKIND_FILE && part->fp) {
                  fclose(part->fp);
                  part->fp = NULL;
                }
                case CURL_READFUNC_ABORT: case CURL_READFUNC_PAUSE: case READ_ERROR: case STOP_FILLING: return cursize? cursize: sz;
                }
                break;
            case MIMESTATE_END: return cursize;
            default: break;
        }
        cursize += sz;
        buffer += sz;
        bufsize -= sz;
    }
#ifdef CURL_DOES_CONVERSIONS
    if(part->easy && convbuf < buffer &&
        part->state.state < MIMESTATE_BODY) {
        CURLcode result = Curl_convert_to_network(part->easy, convbuf, buffer - convbuf);
        if(result) return READ_ERROR;
    }
#endif
  return cursize;
}
static size_t mime_subparts_read(char *buffer, size_t size, size_t nitems, void *instream, bool *hasread) {
    curl_mime *mime = (curl_mime *) instream;
    size_t cursize = 0;
#ifdef CURL_DOES_CONVERSIONS
    char *convbuf = buffer;
#endif
    (void)size;
    while(nitems) {
        size_t sz = 0;
        curl_mimepart *part = mime->state.ptr;
        switch(mime->state.state) {
            case MIMESTATE_BEGIN: case MIMESTATE_BODY:
            #ifdef CURL_DOES_CONVERSIONS
                convbuf = buffer;
            #endif
                mimesetstate(&mime->state, MIMESTATE_BOUNDARY1, mime->firstpart);
                mime->state.offset += 2;
                break;
            case MIMESTATE_BOUNDARY1:
                sz = readback_bytes(&mime->state, buffer, nitems, "\r\n--", 4, "");
                if(!sz) mimesetstate(&mime->state, MIMESTATE_BOUNDARY2, part);
                break;
            case MIMESTATE_BOUNDARY2:
                sz = readback_bytes(&mime->state, buffer, nitems, mime->boundary, strlen(mime->boundary), part? "\r\n": "--\r\n");
                if(!sz) {
                #ifdef CURL_DOES_CONVERSIONS
                    if(mime->easy && convbuf < buffer) {
                        CURLcode result = Curl_convert_to_network(mime->easy, convbuf, buffer - convbuf);
                          if(result) return READ_ERROR;
                      convbuf = buffer;
                    }
                #endif
                    mimesetstate(&mime->state, MIMESTATE_CONTENT, part);
                }
                break;
            case MIMESTATE_CONTENT:
                if(!part) {
                    mimesetstate(&mime->state, MIMESTATE_END, NULL);
                    break;
                }
                sz = readback_part(part, buffer, nitems, hasread);
                switch(sz) {
                    case CURL_READFUNC_ABORT: case CURL_READFUNC_PAUSE: case READ_ERROR: case STOP_FILLING: return cursize? cursize: sz;
                    case 0:
                    #ifdef CURL_DOES_CONVERSIONS
                        convbuf = buffer;
                    #endif
                        mimesetstate(&mime->state, MIMESTATE_BOUNDARY1, part->nextpart);
                        break;
                }
                break;
            case MIMESTATE_END: return cursize;
            default: break;
        }
        cursize += sz;
        buffer += sz;
        nitems -= sz;
    }
#ifdef CURL_DOES_CONVERSIONS
    if(mime->easy && convbuf < buffer && mime->state.state <= MIMESTATE_CONTENT) {
        CURLcode result = Curl_convert_to_network(mime->easy, convbuf, buffer - convbuf);
        if(result) return READ_ERROR;
    }
#endif
    return cursize;
}
static int mime_part_rewind(curl_mimepart *part) {
    int res = CURL_SEEKFUNC_OK;
    enum mimestate targetstate = MIMESTATE_BEGIN;
    if(part->flags & MIME_BODY_ONLY) targetstate = MIMESTATE_BODY;
    cleanup_encoder_state(&part->encstate);
    if(part->state.state > targetstate) {
        res = CURL_SEEKFUNC_CANTSEEK;
        if(part->seekfunc) {
            res = part->seekfunc(part->arg, (curl_off_t) 0, SEEK_SET);
            switch(res) {
                case CURL_SEEKFUNC_OK: case CURL_SEEKFUNC_FAIL: case CURL_SEEKFUNC_CANTSEEK: break;
                case -1: res = CURL_SEEKFUNC_CANTSEEK; break;
                default: res = CURL_SEEKFUNC_FAIL; break;
            }
        }
    }
    if(res == CURL_SEEKFUNC_OK) mimesetstate(&part->state, targetstate, NULL);
    part->lastreadstatus = 1;
    return res;
}
static int mime_subparts_seek(void *instream, curl_off_t offset, int whence) {
    curl_mime *mime = (curl_mime *) instream;
    curl_mimepart *part;
    int result = CURL_SEEKFUNC_OK;
    if(whence != SEEK_SET || offset) return CURL_SEEKFUNC_CANTSEEK;
    if(mime->state.state == MIMESTATE_BEGIN) return CURL_SEEKFUNC_OK;
    for(part = mime->firstpart; part; part = part->nextpart) {
        int res = mime_part_rewind(part);
        if(res != CURL_SEEKFUNC_OK) result = res;
    }
    if(result == CURL_SEEKFUNC_OK) mimesetstate(&mime->state, MIMESTATE_BEGIN, NULL);
    return result;
}
static void cleanup_part_content(curl_mimepart *part) {
    if(part->freefunc) part->freefunc(part->arg);
    part->readfunc = NULL;
    part->seekfunc = NULL;
    part->freefunc = NULL;
    part->arg = (void*)part;
    part->data = NULL;
    part->fp = NULL;
    part->datasize = (curl_off_t)0;
    cleanup_encoder_state(&part->encstate);
    part->kind = MIMEKIND_NONE;
    part->flags &= ~MIME_FAST_READ;
    part->lastreadstatus = 1;
}
static void mime_subparts_free(void *ptr) {
    curl_mime *mime = (curl_mime *) ptr;
    if(mime && mime->parent) {
        mime->parent->freefunc = NULL;
        cleanup_part_content(mime->parent);
    }
    curl_mime_free(mime);
}
static void mime_subparts_unbind(void *ptr) {
    curl_mime *mime = (curl_mime *) ptr;
    if(mime && mime->parent) {
        mime->parent->freefunc = NULL;
        cleanup_part_content(mime->parent);
        mime->parent = NULL;
    }
}
void Curl_mime_cleanpart(curl_mimepart *part) {
    cleanup_part_content(part);
    curl_slist_free_all(part->curlheaders);
    if(part->flags & MIME_USERHEADERS_OWNER) curl_slist_free_all(part->userheaders);
    Curl_safefree(part->mimetype);
    Curl_safefree(part->name);
    Curl_safefree(part->filename);
    Curl_mime_initpart(part, part->easy);
}
void curl_mime_free(curl_mime *mime) {
    curl_mimepart *part;
    if(mime) {
        mime_subparts_unbind(mime);
        while(mime->firstpart) {
            part = mime->firstpart;
            mime->firstpart = part->nextpart;
            Curl_mime_cleanpart(part);
            free(part);
        }
        free(mime);
    }
}
CURLcode Curl_mime_duppart(curl_mimepart *dst, const curl_mimepart *src) {
    curl_mime *mime;
    curl_mimepart *d;
    const curl_mimepart *s;
    CURLcode res = CURLE_OK;
    DEBUGASSERT(dst);
    switch(src->kind) {
        case MIMEKIND_NONE: break;
        case MIMEKIND_DATA: res = curl_mime_data(dst, src->data, (size_t) src->datasize); break;
        case MIMEKIND_FILE:
            res = curl_mime_filedata(dst, src->data);
            if(res == CURLE_READ_ERROR) res = CURLE_OK;
            break;
        case MIMEKIND_CALLBACK:
            res = curl_mime_data_cb(dst, src->datasize, src->readfunc, src->seekfunc, src->freefunc, src->arg);
            break;
        case MIMEKIND_MULTIPART:
            mime = curl_mime_init(dst->easy);
            res = mime? curl_mime_subparts(dst, mime): CURLE_OUT_OF_MEMORY;
            for(s = ((curl_mime *) src->arg)->firstpart; !res && s; s = s->nextpart) {
                d = curl_mime_addpart(mime);
                res = d? Curl_mime_duppart(d, s): CURLE_OUT_OF_MEMORY;
            }
            break;
        default: res = CURLE_BAD_FUNCTION_ARGUMENT; break;
    }
    if(!res && src->userheaders) {
        struct curl_slist *hdrs = Curl_slist_duplicate(src->userheaders);
        if(!hdrs) res = CURLE_OUT_OF_MEMORY;
        else {
            res = curl_mime_headers(dst, hdrs, TRUE);
            if(res) curl_slist_free_all(hdrs);
        }
    }
    if(!res) {
        dst->encoder = src->encoder;
        res = curl_mime_type(dst, src->mimetype);
    }
    if(!res) res = curl_mime_name(dst, src->name);
    if(!res) res = curl_mime_filename(dst, src->filename);
    if(res) Curl_mime_cleanpart(dst);
    return res;
}
curl_mime *curl_mime_init(CURL *easy) {
    curl_mime *mime;
    mime = (curl_mime *) malloc(sizeof(*mime));
    if(mime) {
        mime->easy = easy;
        mime->parent = NULL;
        mime->firstpart = NULL;
        mime->lastpart = NULL;
        memset(mime->boundary, '-', 24);
        if(Curl_rand_hex(easy, (unsigned char *) &mime->boundary[24],MIME_RAND_BOUNDARY_CHARS + 1)) {
            free(mime);
            return NULL;
        }
        mimesetstate(&mime->state, MIMESTATE_BEGIN, NULL);
    }
    return mime;
}
void Curl_mime_initpart(curl_mimepart *part, struct Curl_easy *easy) {
    memset((char *) part, 0, sizeof(*part));
    part->easy = easy;
    part->lastreadstatus = 1;
    mimesetstate(&part->state, MIMESTATE_BEGIN, NULL);
}
curl_mimepart *curl_mime_addpart(curl_mime *mime) {
    curl_mimepart *part;
    if(!mime) return NULL;
    part = (curl_mimepart *) malloc(sizeof(*part));
    if(part) {
        Curl_mime_initpart(part, mime->easy);
        part->parent = mime;
        if(mime->lastpart) mime->lastpart->nextpart = part;
        else mime->firstpart = part;
        mime->lastpart = part;
    }
    return part;
}
CURLcode curl_mime_name(curl_mimepart *part, const char *name) {
    if(!part) return CURLE_BAD_FUNCTION_ARGUMENT;
    Curl_safefree(part->name);
    part->name = NULL;
    if(name) {
        part->name = strdup(name);
        if(!part->name) return CURLE_OUT_OF_MEMORY;
    }
    return CURLE_OK;
}
CURLcode curl_mime_filename(curl_mimepart *part, const char *filename) {
    if(!part) return CURLE_BAD_FUNCTION_ARGUMENT;
    Curl_safefree(part->filename);
    part->filename = NULL;
    if(filename) {
        part->filename = strdup(filename);
        if(!part->filename) return CURLE_OUT_OF_MEMORY;
    }
    return CURLE_OK;
}
CURLcode curl_mime_data(curl_mimepart *part, const char *data, size_t datasize) {
    if(!part) return CURLE_BAD_FUNCTION_ARGUMENT;
    cleanup_part_content(part);
    if(data) {
        if(datasize == CURL_ZERO_TERMINATED) datasize = strlen(data);
        part->data = malloc(datasize + 1);
        if(!part->data) return CURLE_OUT_OF_MEMORY;
        part->datasize = datasize;
        if(datasize) memcpy(part->data, data, datasize);
        part->data[datasize] = '\0';
        part->readfunc = mime_mem_read;
        part->seekfunc = mime_mem_seek;
        part->freefunc = mime_mem_free;
        part->flags |= MIME_FAST_READ;
        part->kind = MIMEKIND_DATA;
    }
    return CURLE_OK;
}
CURLcode curl_mime_filedata(curl_mimepart *part, const char *filename) {
    CURLcode result = CURLE_OK;
    if(!part) return CURLE_BAD_FUNCTION_ARGUMENT;
    cleanup_part_content(part);
    if(filename) {
        char *base;
        struct_stat sbuf;
        if(stat(filename, &sbuf) || access(filename, R_OK)) result = CURLE_READ_ERROR;
        part->data = strdup(filename);
        if(!part->data) result = CURLE_OUT_OF_MEMORY;
        part->datasize = -1;
        if(!result && S_ISREG(sbuf.st_mode)) {
            part->datasize = filesize(filename, sbuf);
            part->seekfunc = mime_file_seek;
        }
        part->readfunc = mime_file_read;
        part->freefunc = mime_file_free;
        part->kind = MIMEKIND_FILE;
        base = strippath(filename);
        if(!base) result = CURLE_OUT_OF_MEMORY;
        else {
            CURLcode res = curl_mime_filename(part, base);
            if(res) result = res;
            free(base);
        }
    }
    return result;
}
CURLcode curl_mime_type(curl_mimepart *part, const char *mimetype) {
    if(!part) return CURLE_BAD_FUNCTION_ARGUMENT;
    Curl_safefree(part->mimetype);
    part->mimetype = NULL;
    if(mimetype) {
        part->mimetype = strdup(mimetype);
        if(!part->mimetype) return CURLE_OUT_OF_MEMORY;
    }
    return CURLE_OK;
}
CURLcode curl_mime_encoder(curl_mimepart *part, const char *encoding) {
    CURLcode result = CURLE_BAD_FUNCTION_ARGUMENT;
    const struct mime_encoder *mep;
    if(!part) return result;
    part->encoder = NULL;
    if(!encoding) return CURLE_OK;
    for(mep = encoders; mep->name; mep++)
        if(strcasecompare(encoding, mep->name)) {
            part->encoder = mep;
            result = CURLE_OK;
        }
    return result;
}
CURLcode curl_mime_headers(curl_mimepart *part, struct curl_slist *headers, int take_ownership) {
    if(!part) return CURLE_BAD_FUNCTION_ARGUMENT;
    if(part->flags & MIME_USERHEADERS_OWNER) {
        if(part->userheaders != headers) curl_slist_free_all(part->userheaders);
        part->flags &= ~MIME_USERHEADERS_OWNER;
    }
    part->userheaders = headers;
    if(headers && take_ownership) part->flags |= MIME_USERHEADERS_OWNER;
    return CURLE_OK;
}
CURLcode curl_mime_data_cb(curl_mimepart *part, curl_off_t datasize, curl_read_callback readfunc, curl_seek_callback seekfunc,
                           curl_free_callback freefunc, void *arg) {
    if(!part) return CURLE_BAD_FUNCTION_ARGUMENT;
    cleanup_part_content(part);
    if(readfunc) {
        part->readfunc = readfunc;
        part->seekfunc = seekfunc;
        part->freefunc = freefunc;
        part->arg = arg;
        part->datasize = datasize;
        part->kind = MIMEKIND_CALLBACK;
    }
    return CURLE_OK;
}
CURLcode Curl_mime_set_subparts(curl_mimepart *part, curl_mime *subparts, int take_ownership) {
    curl_mime *root;
    if(!part) return CURLE_BAD_FUNCTION_ARGUMENT;
    if(part->kind == MIMEKIND_MULTIPART && part->arg == subparts) return CURLE_OK;
    cleanup_part_content(part);
    if(subparts) {
        if(part->easy && subparts->easy && part->easy != subparts->easy) return CURLE_BAD_FUNCTION_ARGUMENT;
        if(subparts->parent) return CURLE_BAD_FUNCTION_ARGUMENT;
        root = part->parent;
        if(root) {
            while(root->parent && root->parent->parent) root = root->parent->parent;
            if(subparts == root) {
                if(part->easy) failf(part->easy, "Can't add itself as a subpart!");
                return CURLE_BAD_FUNCTION_ARGUMENT;
            }
        }
        subparts->parent = part;
        part->seekfunc = mime_subparts_seek;
        part->freefunc = take_ownership? mime_subparts_free: mime_subparts_unbind;
        part->arg = subparts;
        part->datasize = -1;
        part->kind = MIMEKIND_MULTIPART;
    }
    return CURLE_OK;
}
CURLcode curl_mime_subparts(curl_mimepart *part, curl_mime *subparts) {
    return Curl_mime_set_subparts(part, subparts, TRUE);
}
size_t Curl_mime_read(char *buffer, size_t size, size_t nitems, void *instream) {
    curl_mimepart *part = (curl_mimepart *) instream;
    size_t ret;
    bool hasread;
    (void)size;
    do {
        hasread = FALSE;
        ret = readback_part(part, buffer, nitems, &hasread);
    } while(ret == STOP_FILLING);
    return ret;
}
CURLcode Curl_mime_rewind(curl_mimepart *part) {
    return mime_part_rewind(part) == CURL_SEEKFUNC_OK ? CURLE_OK : CURLE_SEND_FAIL_REWIND;
}
static size_t slist_size(struct curl_slist *s, size_t overhead, const char *skip) {
    size_t size = 0;
    size_t skiplen = skip? strlen(skip): 0;
    for(; s; s = s->next)
        if(!skip || !match_header(s, skip, skiplen)) size += strlen(s->data) + overhead;
    return size;
}
static curl_off_t multipart_size(curl_mime *mime) {
    curl_off_t size;
    size_t boundarysize;
    curl_mimepart *part;
    if(!mime) return 0;
    boundarysize = 4 + strlen(mime->boundary) + 2;
    size = boundarysize;
    for(part = mime->firstpart; part; part = part->nextpart) {
        curl_off_t sz = Curl_mime_size(part);
        if(sz < 0) size = sz;
        if(size >= 0) size += boundarysize + sz;
    }
    return size;
}
curl_off_t Curl_mime_size(curl_mimepart *part) {
    curl_off_t size;
    if(part->kind == MIMEKIND_MULTIPART) part->datasize = multipart_size(part->arg);
    size = part->datasize;
    if(part->encoder) size = part->encoder->sizefunc(part);
    if(size >= 0 && !(part->flags & MIME_BODY_ONLY)) {
        size += slist_size(part->curlheaders, 2, NULL);
        size += slist_size(part->userheaders, 2, "Content-Type");
        size += 2;
    }
    return size;
}
CURLcode Curl_mime_add_header(struct curl_slist **slp, const char *fmt, ...) {
    struct curl_slist *hdr = NULL;
    char *s = NULL;
    va_list ap;
    va_start(ap, fmt);
    s = curl_mvaprintf(fmt, ap);
    va_end(ap);
    if(s) {
        hdr = Curl_slist_append_nodup(*slp, s);
        if(hdr) *slp = hdr;
        else free(s);
    }
    return hdr ? CURLE_OK : CURLE_OUT_OF_MEMORY;
}
static CURLcode add_content_type(struct curl_slist **slp, const char *type, const char *boundary) {
    return Curl_mime_add_header(slp, "Content-Type: %s%s%s", type, boundary ? "; boundary=" : "", boundary ? boundary : "");
}
const char *Curl_mime_contenttype(const char *filename) {
    struct ContentType {
        const char *extension;
        const char *type;
    };
    static const struct ContentType ctts[] = {
        {".gif",  "image/gif"},
        {".jpg",  "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png",  "image/png"},
        {".svg",  "image/svg+xml"},
        {".txt",  "text/plain"},
        {".htm",  "text/html"},
        {".html", "text/html"},
        {".pdf",  "application/pdf"},
        {".xml",  "application/xml"}
    };
    if(filename) {
        size_t len1 = strlen(filename);
        const char *nameend = filename + len1;
        unsigned int i;
        for(i = 0; i < sizeof(ctts) / sizeof(ctts[0]); i++) {
            size_t len2 = strlen(ctts[i].extension);
            if(len1 >= len2 && strcasecompare(nameend - len2, ctts[i].extension))
                return ctts[i].type;
        }
    }
    return NULL;
}
static bool content_type_match(const char *contenttype, const char *target) {
    size_t len = strlen(target);
    if(contenttype && strncasecompare(contenttype, target, len))
    switch(contenttype[len]) {
        case '\0': case '\t': case '\r': case '\n': case ' ': case ';': return TRUE;
    }
    return FALSE;
}
CURLcode Curl_mime_prepare_headers(curl_mimepart *part, const char *contenttype, const char *disposition, enum mimestrategy strategy) {
    curl_mime *mime = NULL;
    const char *boundary = NULL;
    char *customct;
    const char *cte = NULL;
    CURLcode ret = CURLE_OK;
    curl_slist_free_all(part->curlheaders);
    part->curlheaders = NULL;
    if(part->state.state == MIMESTATE_CURLHEADERS) mimesetstate(&part->state, MIMESTATE_CURLHEADERS, NULL);
    customct = part->mimetype;
    if(!customct) customct = search_header(part->userheaders, "Content-Type");
    if(customct) contenttype = customct;
    if(!contenttype) {
        switch(part->kind) {
            case MIMEKIND_MULTIPART: contenttype = MULTIPART_CONTENTTYPE_DEFAULT; break;
            case MIMEKIND_FILE:
                contenttype = Curl_mime_contenttype(part->filename);
                if(!contenttype) contenttype = Curl_mime_contenttype(part->data);
                if(!contenttype && part->filename) contenttype = FILE_CONTENTTYPE_DEFAULT;
                break;
            default: contenttype = Curl_mime_contenttype(part->filename);
        }
    }
    if(part->kind == MIMEKIND_MULTIPART) {
        mime = (curl_mime *) part->arg;
        if(mime) boundary = mime->boundary;
    } else if(contenttype && !customct && content_type_match(contenttype, "text/plain"))
    if(strategy == MIMESTRATEGY_MAIL || !part->filename) contenttype = NULL;
    if(!search_header(part->userheaders, "Content-Disposition")) {
        if(!disposition)
            if(part->filename || part->name || (contenttype && !strncasecompare(contenttype, "multipart/", 10)))
                disposition = DISPOSITION_DEFAULT;
        if(disposition && curl_strequal(disposition, "attachment") && !part->name && !part->filename) disposition = NULL;
        if(disposition) {
            char *name = NULL;
            char *filename = NULL;
            if(part->name) {
                name = escape_string(part->name);
                if(!name) ret = CURLE_OUT_OF_MEMORY;
            }
            if(!ret && part->filename) {
                filename = escape_string(part->filename);
                if(!filename) ret = CURLE_OUT_OF_MEMORY;
            }
            if(!ret) ret = Curl_mime_add_header(&part->curlheaders,"Content-Disposition: %s%s%s%s%s%s%s", disposition, name ? "; name=\"" : "",
                                                name ? name : "", name ? "\"" : "", filename ? "; filename=\"" : "", filename ? filename : "",
                                                filename ? "\"" : "");
            Curl_safefree(name);
            Curl_safefree(filename);
            if(ret) return ret;
        }
    }
    if(contenttype) {
        ret = add_content_type(&part->curlheaders, contenttype, boundary);
        if(ret) return ret;
    }
    if(!search_header(part->userheaders, "Content-Transfer-Encoding")) {
        if(part->encoder) cte = part->encoder->name;
        else if(contenttype && strategy == MIMESTRATEGY_MAIL && part->kind != MIMEKIND_MULTIPART) cte = "8bit";
        if(cte) {
            ret = Curl_mime_add_header(&part->curlheaders,"Content-Transfer-Encoding: %s", cte);
            if(ret) return ret;
        }
    }
    if(part->state.state == MIMESTATE_CURLHEADERS) mimesetstate(&part->state, MIMESTATE_CURLHEADERS, part->curlheaders);
    if(part->kind == MIMEKIND_MULTIPART && mime) {
        curl_mimepart *subpart;
        disposition = NULL;
        if(content_type_match(contenttype, "multipart/form-data")) disposition = "form-data";
        for(subpart = mime->firstpart; subpart; subpart = subpart->nextpart) {
            ret = Curl_mime_prepare_headers(subpart, NULL, disposition, strategy);
            if(ret) return ret;
        }
    }
    return ret;
}
void Curl_mime_unpause(curl_mimepart *part) {
    if(part) {
        if(part->lastreadstatus == CURL_READFUNC_PAUSE) part->lastreadstatus = 1;
        if(part->kind == MIMEKIND_MULTIPART) {
            curl_mime *mime = (curl_mime *) part->arg;
            if(mime) {
                curl_mimepart *subpart;
                for(subpart = mime->firstpart; subpart; subpart = subpart->nextpart) Curl_mime_unpause(subpart);
            }
        }
    }
}
#else
curl_mime *curl_mime_init(CURL *easy) {
    (void) easy;
    return NULL;
}
void curl_mime_free(curl_mime *mime) { (void) mime; }
curl_mimepart *curl_mime_addpart(curl_mime *mime) {
    (void) mime;
    return NULL;
}
CURLcode curl_mime_name(curl_mimepart *part, const char *name) {
    (void) part;
    (void) name;
    return CURLE_NOT_BUILT_IN;
}
CURLcode curl_mime_filename(curl_mimepart *part, const char *filename) {
    (void) part;
    (void) filename;
    return CURLE_NOT_BUILT_IN;
}
CURLcode curl_mime_type(curl_mimepart *part, const char *mimetype) {
    (void) part;
    (void) mimetype;
    return CURLE_NOT_BUILT_IN;
}
CURLcode curl_mime_encoder(curl_mimepart *part, const char *encoding) {
    (void) part;
    (void) encoding;
    return CURLE_NOT_BUILT_IN;
}
CURLcode curl_mime_data(curl_mimepart *part, const char *data, size_t datasize) {
    (void) part;
    (void) data;
    (void) datasize;
    return CURLE_NOT_BUILT_IN;
}
CURLcode curl_mime_filedata(curl_mimepart *part, const char *filename) {
    (void) part;
    (void) filename;
    return CURLE_NOT_BUILT_IN;
}
CURLcode curl_mime_data_cb(curl_mimepart *part, curl_off_t datasize, curl_read_callback readfunc, curl_seek_callback seekfunc, curl_free_callback freefunc,
                           void *arg) {
    (void) part;
    (void) datasize;
    (void) readfunc;
    (void) seekfunc;
    (void) freefunc;
    (void) arg;
    return CURLE_NOT_BUILT_IN;
}
CURLcode curl_mime_subparts(curl_mimepart *part, curl_mime *subparts) {
    (void) part;
    (void) subparts;
    return CURLE_NOT_BUILT_IN;
}
CURLcode curl_mime_headers(curl_mimepart *part, struct curl_slist *headers, int take_ownership) {
    (void) part;
    (void) headers;
    (void) take_ownership;
    return CURLE_NOT_BUILT_IN;
}
CURLcode Curl_mime_add_header(struct curl_slist **slp, const char *fmt, ...) {
    (void)slp;
    (void)fmt;
    return CURLE_NOT_BUILT_IN;
}
#endif