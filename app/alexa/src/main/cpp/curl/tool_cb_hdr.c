#include "tool_setup.h"
#include "strcase.h"
#define ENABLE_CURLX_PRINTF
#include "curlx.h"
#include "curl_ctype.h"
#include "tool_cfgable.h"
#include "tool_doswin.h"
#include "tool_msgs.h"
#include "tool_cb_hdr.h"
#include "tool_cb_wrt.h"
#include "tool_operate.h"
#include "memdebug.h"

static char *parse_filename(const char *ptr, size_t len);
#ifdef WIN32
#define BOLD
#define BOLDOFF
#else
#define BOLD "\x1b[1m"
#define BOLDOFF "\x1b[0m"
#endif
size_t tool_header_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
    struct per_transfer *per = userdata;
    struct HdrCbData *hdrcbdata = &per->hdrcbdata;
    struct OutStruct *outs = &per->outs;
    struct OutStruct *heads = &per->heads;
    struct OutStruct *etag_save = &per->etag_save;
    const char *str = ptr;
    const size_t cb = size * nmemb;
    const char *end = (char *)ptr + cb;
    long protocol = 0;
    size_t failure = (size && nmemb) ? 0 : 1;
    if(!per->config) return failure;
#ifdef DEBUGBUILD
    if(size * nmemb > (size_t)CURL_MAX_HTTP_HEADER) {
        warnf(per->config->global, "Header data exceeds single call write limit!\n");
        return failure;
    }
#endif
    if(per->config->headerfile && heads->stream) {
        size_t rc = fwrite(ptr, size, nmemb, heads->stream);
        if(rc != cb) return rc;
        (void)fflush(heads->stream);
    }
    if(per->config->etag_save_file && etag_save->stream) {
        if(curl_strnequal(str, "etag:", 5)) {
            char *etag_h = NULL;
            char *first = NULL;
            char *last = NULL;
            size_t etag_length = 0;
            etag_h = ptr;
            first = memchr(etag_h, '\"', cb);
            if(!first) {
                warnf(per->config->global,"Received header etag is missing double quote/s\n");
                return failure;
            } else first++;
            last = memchr(first, '\"', cb);
            if(!last) {
                warnf(per->config->global,"Received header etag is missing double quote/s\n");
                return failure;
            }
            etag_length = (size_t)last - (size_t)first;
            fwrite(first, size, etag_length, etag_save->stream);
            fputc('\n', etag_save->stream);
        }
        (void)fflush(etag_save->stream);
    }
    curl_easy_getinfo(per->curl, CURLINFO_PROTOCOL, &protocol);
    if(hdrcbdata->honor_cd_filename && (cb > 20) && checkprefix("Content-disposition:", str) && (protocol & (CURLPROTO_HTTPS|CURLPROTO_HTTP))) {
        const char *p = str + 20;
        for( ; ; ) {
            char *filename;
            size_t len;
            while(*p && (p < end) && !ISALPHA(*p)) p++;
            if(p > end - 9) break;
            if(memcmp(p, "filename=", 9)) {
                while((p < end) && (*p != ';')) p++;
                continue;
            }
            p += 9;
            len = (ssize_t)cb - (p - str);
            filename = parse_filename(p, len);
            if(filename) {
                if(outs->stream) {
                    free(filename);
                    return failure;
                }
                outs->is_cd_filename = TRUE;
                outs->s_isreg = TRUE;
                outs->fopened = FALSE;
                outs->filename = filename;
                outs->alloc_filename = TRUE;
                hdrcbdata->honor_cd_filename = FALSE;
                if(!tool_create_output_file(outs, per->config)) return failure;
            }
            break;
        }
        if(!outs->stream && !tool_create_output_file(outs, per->config)) return failure;
    }
    if(hdrcbdata->config->show_headers && (protocol & (CURLPROTO_HTTP|CURLPROTO_HTTPS|CURLPROTO_RTSP|CURLPROTO_FILE))) {
        char *value = NULL;
        if(!outs->stream && !tool_create_output_file(outs, per->config)) return failure;
        if(hdrcbdata->global->isatty && hdrcbdata->global->styled_output) value = memchr(ptr, ':', cb);
        if(value) {
            size_t namelen = value - ptr;
            fprintf(outs->stream, BOLD "%.*s" BOLDOFF ":", namelen, ptr);
            fwrite(&value[1], cb - namelen - 1, 1, outs->stream);
        } else fwrite(ptr, cb, 1, outs->stream);
    }
    return cb;
}
static char *parse_filename(const char *ptr, size_t len) {
    char *copy;
    char *p;
    char *q;
    char  stop = '\0';
    copy = malloc(len + 1);
    if(!copy) return NULL;
    memcpy(copy, ptr, len);
    copy[len] = '\0';
    p = copy;
    if(*p == '\'' || *p == '"') {
        stop = *p;
        p++;
    } else stop = ';';
    q = strchr(p, stop);
    if(q) *q = '\0';
    q = strrchr(p, '/');
    if(q) {
        p = q + 1;
        if(!*p) {
            Curl_safefree(copy);
            return NULL;
        }
    }
    q = strrchr(p, '\\');
    if(q) {
        p = q + 1;
        if(!*p) {
            Curl_safefree(copy);
            return NULL;
        }
    }
    q = strchr(p, '\r');
    if(q) *q = '\0';
    q = strchr(p, '\n');
    if(q) *q = '\0';
    if(copy != p) memmove(copy, p, strlen(p) + 1);
#if defined(MSDOS) || defined(WIN32)
    char *sanitized;
    SANITIZEcode sc = sanitize_file_name(&sanitized, copy, 0);
    Curl_safefree(copy);
    if(sc) return NULL;
    copy = sanitized;
#endif
#ifdef DEBUGBUILD
    char *tdir = curlx_getenv("CURL_TESTDIR");
    if(tdir) {
        char buffer[512];
        msnprintf(buffer, sizeof(buffer), "%s/%s", tdir, copy);
        Curl_safefree(copy);
        copy = strdup(buffer);
        curl_free(tdir);
    }
#endif
    return copy;
}