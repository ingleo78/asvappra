#include <stdbool.h>
#include "tool_setup.h"
#include "strcase.h"

#define ENABLE_CURLX_PRINTF
#include "curlx.h"
#include "curl_ctype.h"
#include "tool_cfgable.h"
#include "tool_convert.h"
#include "tool_msgs.h"
#include "tool_binmode.h"
#include "tool_getparam.h"
#include "tool_paramhlp.h"
#include "tool_formparse.h"
#include "memdebug.h"

#define CONST_FREE(x)           free((void *) (x))
#define CONST_SAFEFREE(x)       Curl_safefree(*((void **) &(x)))

static struct tool_mime *tool_mime_new(struct tool_mime *parent, toolmimekind kind) {
    struct tool_mime *m = (struct tool_mime *) calloc(1, sizeof(*m));
    if(m) {
        m->kind = kind;
        m->parent = parent;
        if(parent) {
            m->prev = parent->subparts;
            parent->subparts = m;
        }
    }
    return m;
}
static struct tool_mime *tool_mime_new_parts(struct tool_mime *parent) {
    return tool_mime_new(parent, TOOLMIME_PARTS);
}
static struct tool_mime *tool_mime_new_data(struct tool_mime *parent, const char *data) {
    struct tool_mime *m = NULL;
    data = strdup(data);
    if(data) {
        m = tool_mime_new(parent, TOOLMIME_DATA);
        if(!m) CONST_FREE(data);
        else m->data = data;
    }
    return m;
}
static struct tool_mime *tool_mime_new_filedata(struct tool_mime *parent, const char *filename, bool isremotefile, CURLcode *errcode) {
    CURLcode result = CURLE_OK;
    struct tool_mime *m = NULL;
    *errcode = CURLE_OUT_OF_MEMORY;
    if(strcmp(filename, "-")) {
        filename = strdup(filename);
        if(filename) {
            m = tool_mime_new(parent, TOOLMIME_FILE);
            if(!m) CONST_FREE(filename);
            else {
                m->data = filename;
                if(!isremotefile) m->kind = TOOLMIME_FILEDATA;
               *errcode = CURLE_OK;
            }
        }
    } else {
        int fd = fileno(stdin);
        char *data = NULL;
        curl_off_t size;
        curl_off_t origin;
        struct_stat sbuf;
        set_binmode(stdin);
        origin = ftell(stdin);
        if(fd >= 0 && origin >= 0 && !fstat(fd, &sbuf) &&
    #ifdef __VMS
           sbuf.st_fab_rfm != FAB$C_VAR && sbuf.st_fab_rfm != FAB$C_VFC &&
    #endif
           S_ISREG(sbuf.st_mode)) {
            size = sbuf.st_size - origin;
            if(size < 0) size = 0;
        } else {
            size_t stdinsize = 0;
            if(file2memory(&data, &stdinsize, stdin) != PARAM_OK) return m;
            if(ferror(stdin)) {
              result = CURLE_READ_ERROR;
              Curl_safefree(data);
              data = NULL;
            } else if(!stdinsize) {
              data = strdup("");
              if(!data) return m;
            }
            size = curlx_uztoso(stdinsize);
            origin = 0;
        }
        m = tool_mime_new(parent, TOOLMIME_STDIN);
        if(!m) Curl_safefree(data);
        else {
            m->data = data;
            m->origin = origin;
            m->size = size;
            m->curpos = 0;
            if(!isremotefile)
              m->kind = TOOLMIME_STDINDATA;
            *errcode = result;
        }
    }
    return m;
}
void tool_mime_free(struct tool_mime *mime) {
    if(mime) {
        if(mime->subparts) tool_mime_free(mime->subparts);
        if(mime->prev) tool_mime_free(mime->prev);
        CONST_SAFEFREE(mime->name);
        CONST_SAFEFREE(mime->filename);
        CONST_SAFEFREE(mime->type);
        CONST_SAFEFREE(mime->encoder);
        CONST_SAFEFREE(mime->data);
        curl_slist_free_all(mime->headers);
        free(mime);
    }
}
size_t tool_mime_stdin_read(char *buffer, size_t size, size_t nitems, void *arg) {
    struct tool_mime *sip = (struct tool_mime *) arg;
    curl_off_t bytesleft;
    (void)size;
    if(sip->size >= 0) {
        if(sip->curpos >= sip->size) return 0;
        bytesleft = sip->size - sip->curpos;
        if(curlx_uztoso(nitems) > bytesleft) nitems = curlx_sotouz(bytesleft);
    }
    if(nitems) {
        if(sip->data) memcpy(buffer, sip->data + curlx_sotouz(sip->curpos), nitems);
        else {
            nitems = fread(buffer, 1, nitems, stdin);
            if(ferror(stdin)) {
                if(sip->config) {
                    warnf(sip->config, "stdin: %s\n", strerror(errno));
                    sip->config = NULL;
                }
                return CURL_READFUNC_ABORT;
            }
        }
        sip->curpos += curlx_uztoso(nitems);
    }
    return nitems;
}
int tool_mime_stdin_seek(void *instream, curl_off_t offset, int whence) {
    struct tool_mime *sip = (struct tool_mime *) instream;
    switch(whence) {
        case SEEK_CUR: offset += sip->curpos; break;
        case SEEK_END: offset += sip->size; break;
    }
    if(offset < 0) return CURL_SEEKFUNC_CANTSEEK;
    if(!sip->data) {
        if(fseek(stdin, (long) (offset + sip->origin), SEEK_SET)) return CURL_SEEKFUNC_CANTSEEK;
    }
    sip->curpos = offset;
    return CURL_SEEKFUNC_OK;
}
static CURLcode tool2curlparts(CURL *curl, struct tool_mime *m, curl_mime *mime) {
    CURLcode ret = CURLE_OK;
    curl_mimepart *part = NULL;
    curl_mime *submime = NULL;
    const char *filename = NULL;
    if(m) {
        ret = tool2curlparts(curl, m->prev, mime);
        if(!ret) {
            part = curl_mime_addpart(mime);
            if(!part) ret = CURLE_OUT_OF_MEMORY;
        }
        if(!ret) {
            filename = m->filename;
            switch(m->kind) {
                case TOOLMIME_PARTS:
                    ret = tool2curlmime(curl, m, &submime);
                    if(!ret) {
                        ret = curl_mime_subparts(part, submime);
                        if(ret) curl_mime_free(submime);
                    }
                    break;
                case TOOLMIME_DATA:
                #ifdef CURL_DOES_CONVERSIONS
                    {
                        size_t size = strlen(m->data);
                        char *cp = malloc(size + 1);
                        if(!cp) ret = CURLE_OUT_OF_MEMORY;
                        else {
                            memcpy(cp, m->data, size + 1);
                            ret = convert_to_network(cp, size);
                            if(!ret)
                              ret = curl_mime_data(part, cp, CURL_ZERO_TERMINATED);
                            free(cp);
                        }
                    }
                #else
                    ret = curl_mime_data(part, m->data, CURL_ZERO_TERMINATED);
                #endif
                    break;
                case TOOLMIME_FILE: case TOOLMIME_FILEDATA:
                    ret = curl_mime_filedata(part, m->data);
                    if(!ret && m->kind == TOOLMIME_FILEDATA && !filename) ret = curl_mime_filename(part, NULL);
                    break;
                case TOOLMIME_STDIN: if(!filename) filename = "-";
                case TOOLMIME_STDINDATA:
                    ret = curl_mime_data_cb(part, m->size, (curl_read_callback) tool_mime_stdin_read, (curl_seek_callback) tool_mime_stdin_seek, NULL, m);
                    break;
            }
        }
        if(!ret && filename) ret = curl_mime_filename(part, filename);
        if(!ret) ret = curl_mime_type(part, m->type);
        if(!ret) ret = curl_mime_headers(part, m->headers, 0);
        if(!ret) ret = curl_mime_encoder(part, m->encoder);
        if(!ret) ret = curl_mime_name(part, m->name);
    }
    return ret;
}
CURLcode tool2curlmime(CURL *curl, struct tool_mime *m, curl_mime **mime) {
    CURLcode ret = CURLE_OK;
    *mime = curl_mime_init(curl);
    if(!*mime) ret = CURLE_OUT_OF_MEMORY;
    else ret = tool2curlparts(curl, m->subparts, *mime);
    if(ret) {
        curl_mime_free(*mime);
        *mime = NULL;
    }
    return ret;
}
static char *get_param_word(char **str, char **end_pos, char endchar) {
    char *ptr = *str;
    char *word_begin = ptr;
    char *ptr2;
    char *escape = NULL;
    if(*ptr == '"') {
        ++ptr;
        while(*ptr) {
            if(*ptr == '\\') {
                if(ptr[1] == '\\' || ptr[1] == '"') {
                    if(!escape) escape = ptr;
                    ptr += 2;
                    continue;
                }
            }
            if(*ptr == '"') {
                *end_pos = ptr;
                if(escape) {
                    ptr = ptr2 = escape;
                    do {
                        if(*ptr == '\\' && (ptr[1] == '\\' || ptr[1] == '"')) ++ptr;
                        *ptr2++ = *ptr++;
                    }
                    while(ptr < *end_pos);
                    *end_pos = ptr2;
                }
                while(*ptr && *ptr != ';' && *ptr != endchar) ++ptr;
                *str = ptr;
                return word_begin + 1;
            }
            ++ptr;
        }
        ptr = word_begin;
    }
    while(*ptr && *ptr != ';' && *ptr != endchar) ++ptr;
    *str = *end_pos = ptr;
    return word_begin;
}
static int slist_append(struct curl_slist **plist, const char *data) {
    struct curl_slist *s = curl_slist_append(*plist, data);
    if(!s) return -1;
    *plist = s;
    return 0;
}
static int read_field_headers(struct OperationConfig *config, const char *filename, FILE *fp, struct curl_slist **pheaders) {
    size_t hdrlen = 0;
    size_t pos = 0;
    bool incomment = FALSE;
    int lineno = 1;
    char hdrbuf[999];
    for( ; ; ) {
        int c = getc(fp);
        if(c == EOF || (!pos && !ISSPACE(c))) {
            while(hdrlen && ISSPACE(hdrbuf[hdrlen - 1])) hdrlen--;
            if(hdrlen) {
                hdrbuf[hdrlen] = '\0';
                if(slist_append(pheaders, hdrbuf)) {
                    fprintf(config->global->errors, "Out of memory for field headers!\n");
                    return -1;
                }
                hdrlen = 0;
            }
        }
        switch(c) {
            case EOF:
                if(ferror(fp)) {
                    fprintf(config->global->errors, "Header file %s read error: %s\n", filename, strerror(errno));
                    return -1;
                }
                return 0;
            case '\r': continue;
            case '\n':
                pos = 0;
                incomment = FALSE;
                lineno++;
                continue;
            case '#':
                if(!pos) incomment = TRUE;
                break;
        }
        pos++;
        if(!incomment) {
            if(hdrlen == sizeof(hdrbuf) - 1) {
                warnf(config->global, "File %s line %d: header too long (truncated)\n", filename, lineno);
                c = ' ';
            }
            if(hdrlen <= sizeof(hdrbuf) - 1) hdrbuf[hdrlen++] = (char) c;
        }
    }
}
static int get_param_part(struct OperationConfig *config, char endchar, char **str, char **pdata, char **ptype, char **pfilename, char **pencoder,
                          struct curl_slist **pheaders) {
    char *p = *str;
    char *type = NULL;
    char *filename = NULL;
    char *encoder = NULL;
    char *endpos;
    char *tp;
    char sep;
    char type_major[128] = "";
    char type_minor[128] = "";
    char *endct = NULL;
    struct curl_slist *headers = NULL;
    if(ptype) *ptype = NULL;
    if(pfilename) *pfilename = NULL;
    if(pheaders) *pheaders = NULL;
    if(pencoder) *pencoder = NULL;
    while(ISSPACE(*p)) p++;
    tp = p;
    *pdata = get_param_word(&p, &endpos, endchar);
    if(*pdata == tp)
        while(endpos > *pdata && ISSPACE(endpos[-1])) endpos--;
    sep = *p;
    *endpos = '\0';
    while(sep == ';') {
        while(ISSPACE(*++p));
        if(!endct && checkprefix("type=", p)) {
            for(p += 5; ISSPACE(*p); p++);
            type = p;
            if(2 != sscanf(type, "%127[^/ ]/%127[^;, \n]", type_major, type_minor)) {
                warnf(config->global, "Illegally formatted content-type field!\n");
                curl_slist_free_all(headers);
                return -1;
            }
            p = type + strlen(type_major) + strlen(type_minor) + 1;
            for(endct = p; *p && *p != ';' && *p != endchar; p++)
                if(!ISSPACE(*p)) endct = p + 1;
            sep = *p;
        } else if(checkprefix("filename=", p)) {
            if(endct) {
                *endct = '\0';
                endct = NULL;
            }
            for(p += 9; ISSPACE(*p); p++);
            tp = p;
            filename = get_param_word(&p, &endpos, endchar);
            if(filename == tp)
                while(endpos > filename && ISSPACE(endpos[-1])) endpos--;
            sep = *p;
            *endpos = '\0';
        } else if(checkprefix("headers=", p)) {
            if(endct) {
                *endct = '\0';
                endct = NULL;
            }
            p += 8;
            if(*p == '@' || *p == '<') {
                char *hdrfile;
                FILE *fp;
                do {
                    p++;
                } while(ISSPACE(*p));
                tp = p;
                hdrfile = get_param_word(&p, &endpos, endchar);
                if(hdrfile == tp)
                    while(endpos > hdrfile && ISSPACE(endpos[-1])) endpos--;
                sep = *p;
                *endpos = '\0';
                fp = fopen(hdrfile, FOPEN_READTEXT);
                if(!fp) warnf(config->global, "Cannot read from %s: %s\n", hdrfile, strerror(errno));
                else {
                    int i = read_field_headers(config, hdrfile, fp, &headers);
                    fclose(fp);
                    if(i) {
                    curl_slist_free_all(headers);
                    return -1;
                    }
                }
            } else {
                char *hdr;
                while(ISSPACE(*p)) p++;
                tp = p;
                hdr = get_param_word(&p, &endpos, endchar);
                if(hdr == tp)
                    while(endpos > hdr && ISSPACE(endpos[-1])) endpos--;
                sep = *p;
                *endpos = '\0';
                if(slist_append(&headers, hdr)) {
                    fprintf(config->global->errors, "Out of memory for field header!\n");
                    curl_slist_free_all(headers);
                    return -1;
                }
            }
        } else if(checkprefix("encoder=", p)) {
            if(endct) {
                *endct = '\0';
                endct = NULL;
            }
            for(p += 8; ISSPACE(*p); p++);
            tp = p;
            encoder = get_param_word(&p, &endpos, endchar);
            if(encoder == tp)
            while(endpos > encoder && ISSPACE(endpos[-1])) endpos--;
            sep = *p;
            *endpos = '\0';
        } else if(endct) {
            for(endct = p; *p && *p != ';' && *p != endchar; p++)
            if(!ISSPACE(*p)) endct = p + 1;
            sep = *p;
        } else {
            char *unknown = get_param_word(&p, &endpos, endchar);
            sep = *p;
            *endpos = '\0';
            if(*unknown) warnf(config->global, "skip unknown form field: %s\n", unknown);
        }
    }
    if(endct) *endct = '\0';
    if(ptype) *ptype = type;
    else if(type) warnf(config->global, "Field content type not allowed here: %s\n", type);
    if(pfilename) *pfilename = filename;
    else if(filename) warnf(config->global,"Field file name not allowed here: %s\n", filename);
    if(pencoder) *pencoder = encoder;
    else if(encoder) warnf(config->global,"Field encoder not allowed here: %s\n", encoder);
    if(pheaders) *pheaders = headers;
    else if(headers) {
        warnf(config->global,"Field headers not allowed here: %s\n", headers->data);
        curl_slist_free_all(headers);
    }
    *str = p;
    return sep & 0xFF;
}
#define NULL_CHECK(ptr, init, retcode) {                                \
    (ptr) = (init);                                                     \
    if(!(ptr)) {                                                        \
        warnf(config->global, "out of memory!\n");                      \
        curl_slist_free_all(headers);                                   \
        Curl_safefree(contents);                                        \
        return retcode;                                                 \
    }                                                                   \
}
#define SET_TOOL_MIME_PTR(m, field, retcode) {                          \
    if(field) NULL_CHECK((m)->field, strdup(field), retcode);           \
}
int formparse(struct OperationConfig *config, const char *input, struct tool_mime **mimeroot, struct tool_mime **mimecurrent, bool literal_value) {
    char *name = NULL;
    char *contents = NULL;
    char *contp;
    char *data;
    char *type = NULL;
    char *filename = NULL;
    char *encoder = NULL;
    struct curl_slist *headers = NULL;
    struct tool_mime *part = NULL;
    CURLcode res;
    if(!*mimecurrent) {
        NULL_CHECK(*mimeroot, tool_mime_new_parts(NULL), 1);
        *mimecurrent = *mimeroot;
    }
    NULL_CHECK(contents, strdup(input), 2);
    contp = strchr(contents, '=');
    if(contp) {
        int sep = '\0';
        if(contp > contents) name = contents;
        *contp++ = '\0';
        if(*contp == '(' && !literal_value) {
            sep = get_param_part(config, '\0', &contp, &data, &type, NULL, NULL, &headers);
            if(sep < 0) {
            Curl_safefree(contents);
            return 3;
            }
            NULL_CHECK(part, tool_mime_new_parts(*mimecurrent), 4);
            *mimecurrent = part;
            part->headers = headers;
            headers = NULL;
            SET_TOOL_MIME_PTR(part, type, 5);
        } else if(!name && !strcmp(contp, ")") && !literal_value) {
            if(*mimecurrent == *mimeroot) {
                warnf(config->global, "no multipart to terminate!\n");
                Curl_safefree(contents);
                return 6;
            }
            *mimecurrent = (*mimecurrent)->parent;
        } else if('@' == contp[0] && !literal_value) {
            struct tool_mime *subparts = NULL;
            do {
                ++contp;
                sep = get_param_part(config, ',', &contp, &data, &type, &filename, &encoder, &headers);
                if(sep < 0) {
                    Curl_safefree(contents);
                    return 7;
                }
                if(!subparts) {
                    if(sep != ',') subparts = *mimecurrent;
                    else NULL_CHECK(subparts, tool_mime_new_parts(*mimecurrent), 8);
                }
                NULL_CHECK(part,tool_mime_new_filedata(subparts, data, TRUE, &res), 9);
                part->headers = headers;
                headers = NULL;
                part->config = config->global;
                if(res == CURLE_READ_ERROR) {
                    if(part->size > 0) {
                        warnf(config->global,"error while reading standard input\n");
                        Curl_safefree(contents);
                        return 10;
                    }
                    CONST_SAFEFREE(part->data);
                    part->data = NULL;
                    part->size = -1;
                    res = CURLE_OK;
                }
                SET_TOOL_MIME_PTR(part, filename, 11);
                SET_TOOL_MIME_PTR(part, type, 12);
                SET_TOOL_MIME_PTR(part, encoder, 13);
            } while(sep);
            part = (*mimecurrent)->subparts;
        } else {
            if(*contp == '<' && !literal_value) {
                ++contp;
                sep = get_param_part(config, '\0', &contp, &data, &type, NULL, &encoder, &headers);
                if(sep < 0) {
                    Curl_safefree(contents);
                    return 14;
                }
                NULL_CHECK(part, tool_mime_new_filedata(*mimecurrent, data, FALSE, &res), 15);
                part->headers = headers;
                headers = NULL;
                part->config = config->global;
                if(res == CURLE_READ_ERROR) {
                    if(part->size > 0) {
                    warnf(config->global,"error while reading standard input\n");
                    Curl_safefree(contents);
                    return 16;
                    }
                    CONST_SAFEFREE(part->data);
                    part->data = NULL;
                    part->size = -1;
                    res = CURLE_OK;
                }
            } else {
                if(literal_value) data = contp;
                else {
                    sep = get_param_part(config, '\0', &contp, &data, &type, &filename, &encoder, &headers);
                    if(sep < 0) {
                    Curl_safefree(contents);
                    return 17;
                    }
                }
                NULL_CHECK(part, tool_mime_new_data(*mimecurrent, data), 18);
                part->headers = headers;
                headers = NULL;
            }
            SET_TOOL_MIME_PTR(part, filename, 19);
            SET_TOOL_MIME_PTR(part, type, 20);
            SET_TOOL_MIME_PTR(part, encoder, 21);
            if(sep) {
                *contp = (char) sep;
                warnf(config->global,"garbage at end of field specification: %s\n", contp);
            }
        }
        SET_TOOL_MIME_PTR(part, name, 22);
    } else {
        warnf(config->global, "Illegally formatted input field!\n");
        Curl_safefree(contents);
        return 23;
    }
    Curl_safefree(contents);
    return 0;
}