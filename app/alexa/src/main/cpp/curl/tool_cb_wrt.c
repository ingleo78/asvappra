#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include "tool_setup.h"
#define ENABLE_CURLX_PRINTF
#include "curlx.h"
#include "tool_cfgable.h"
#include "tool_msgs.h"
#include "tool_cb_wrt.h"
#include "tool_operate.h"
#include "memdebug.h"

bool tool_create_output_file(struct OutStruct *outs, struct OperationConfig *config) {
    struct GlobalConfig *global;
    FILE *file;
    DEBUGASSERT(outs);
    DEBUGASSERT(config);
    global = config->global;
    if(!outs->filename || !*outs->filename) {
        warnf(global, "Remote filename has no length!\n");
        return FALSE;
    }
    if(outs->is_cd_filename) {
        file = fopen(outs->filename, "rb");
        if(file) {
            fclose(file);
            warnf(global, "Refusing to overwrite %s: %s\n", outs->filename, strerror(EEXIST));
            return FALSE;
        }
    }
    file = fopen(outs->filename, "wb");
    if(!file) {
        warnf(global, "Failed to create the file %s: %s\n", outs->filename, strerror(errno));
        return FALSE;
    }
    outs->s_isreg = TRUE;
    outs->fopened = TRUE;
    outs->stream = file;
    outs->bytes = 0;
    outs->init = 0;
    return TRUE;
}
size_t tool_write_cb(char *buffer, size_t sz, size_t nmemb, void *userdata) {
    size_t rc;
    struct per_transfer *per = userdata;
    struct OutStruct *outs = &per->outs;
    struct OperationConfig *config = per->config;
    size_t bytes = sz * nmemb;
    bool is_tty = config->global->isatty;
#ifdef WIN32
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    intptr_t fhnd;
#endif
    const size_t failure = bytes ? 0 : 1;
#ifdef DEBUGBUILD
    char *tty = curlx_getenv("CURL_ISATTY");
    if(tty) {
        is_tty = TRUE;
        curl_free(tty);
    }
    if(config->show_headers) {
        if(bytes > (size_t)CURL_MAX_HTTP_HEADER) {
            warnf(config->global, "Header data size exceeds single call write limit!\n");
            return failure;
        }
    } else {
        if(bytes > (size_t)CURL_MAX_WRITE_SIZE) {
            warnf(config->global, "Data size exceeds single call write limit!\n");
            return failure;
        }
    }
    bool check_fails = FALSE;
    if(outs->filename) {
        if(!*outs->filename) check_fails = TRUE;
        if(!outs->s_isreg) check_fails = TRUE;
        if(outs->fopened && !outs->stream) check_fails = TRUE;
        if(!outs->fopened && outs->stream) check_fails = TRUE;
        if(!outs->fopened && outs->bytes) check_fails = TRUE;
    }
    else {
        if(!outs->stream || outs->s_isreg || outs->fopened) check_fails = TRUE;
        if(outs->alloc_filename || outs->is_cd_filename || outs->init) check_fails = TRUE;
    }
    if(check_fails) {
        warnf(config->global, "Invalid output struct data for write callback\n");
        return failure;
    }
#endif
    if(!outs->stream && !tool_create_output_file(outs, per->config)) return failure;
    if(is_tty && (outs->bytes < 2000) && !config->terminal_binary_ok) {
        if(memchr(buffer, 0, bytes)) {
            warnf(config->global, "Binary output can mess up your terminal. Use \"--output -\" to tell curl to output it to your terminal "
                  "anyway, or consider \"--output <FILE>\" to save to a file.\n");
            config->synthetic_error = ERR_BINARY_TERMINAL;
            return failure;
        }
    }
#ifdef WIN32
    fhnd = _get_osfhandle(fileno(outs->stream));
    if(isatty(fileno(outs->stream)) && GetConsoleScreenBufferInfo((HANDLE)fhnd, &console_info)) {
        DWORD in_len = (DWORD)(sz * nmemb);
        wchar_t* wc_buf;
        DWORD wc_len;
        wc_len = MultiByteToWideChar(CP_UTF8, 0, buffer, in_len,  NULL, 0);
        wc_buf = (wchar_t*) malloc(wc_len * sizeof(wchar_t));
        if(!wc_buf) return failure;
        wc_len = MultiByteToWideChar(CP_UTF8, 0, buffer, in_len, wc_buf, wc_len);
        if(!wc_len) {
            free(wc_buf);
            return failure;
        }
        if(!WriteConsoleW((HANDLE) fhnd, wc_buf, wc_len, &wc_len, NULL)) {
            free(wc_buf);
            return failure;
        }
        free(wc_buf);
        rc = bytes;
    } else
#endif
    rc = fwrite(buffer, sz, nmemb, outs->stream);
    if(bytes == rc) outs->bytes += bytes;
    if(config->readbusy) {
        config->readbusy = FALSE;
        curl_easy_pause(per->curl, CURLPAUSE_CONT);
    }
    if(config->nobuffer) {
        int res = fflush(outs->stream);
        if(res) return failure;
    }
    return rc;
}