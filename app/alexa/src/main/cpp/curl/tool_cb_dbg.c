#include <stdbool.h>
#include "tool_setup.h"

#define ENABLE_CURLX_PRINTF
#include "curlx.h"
#include "tool_cfgable.h"
#include "tool_convert.h"
#include "tool_msgs.h"
#include "tool_cb_dbg.h"
#include "tool_util.h"
#include "memdebug.h"

static void dump(const char *timebuf, const char *text, FILE *stream, const unsigned char *ptr, size_t size, trace tracetype, curl_infotype infotype);
int tool_debug_cb(CURL *handle, curl_infotype type, char *data, size_t size, void *userdata) {
    struct OperationConfig *operation = userdata;
    struct GlobalConfig *config = operation->global;
    FILE *output = config->errors;
    const char *text;
    struct timeval tv;
    char timebuf[20];
    time_t secs;
    (void)handle;
    if(config->tracetime) {
        struct tm *now;
        static time_t epoch_offset;
        static int    known_offset;
        tv = tvnow();
        if(!known_offset) {
            epoch_offset = time(NULL) - tv.tv_sec;
            known_offset = 1;
        }
        secs = epoch_offset + tv.tv_sec;
        now = localtime(&secs);  /* not thread safe but we don't care */
        msnprintf(timebuf, sizeof(timebuf), "%02d:%02d:%02d.%06ld ", now->tm_hour, now->tm_min, now->tm_sec, (long)tv.tv_usec);
    } else timebuf[0] = 0;
    if(!config->trace_stream) {
        if(!strcmp("-", config->trace_dump)) config->trace_stream = stdout;
        else if(!strcmp("%", config->trace_dump)) config->trace_stream = config->errors;
        else {
            config->trace_stream = fopen(config->trace_dump, FOPEN_WRITETEXT);
            config->trace_fopened = TRUE;
        }
    }
    if(config->trace_stream) output = config->trace_stream;
    if(!output) {
        warnf(config, "Failed to create/open output");
        return 0;
    }
    if(config->tracetype == TRACE_PLAIN) {
        static const char * const s_infotype[] = { "*", "<", ">", "{", "}", "{", "}" };
        static bool newl = FALSE;
        static bool traced_data = FALSE;
        switch(type) {
            case CURLINFO_HEADER_OUT:
                if(size > 0) {
                    size_t st = 0;
                    size_t i;
                    for(i = 0; i < size - 1; i++) {
                        if(data[i] == '\n') { /* LF */
                            if(!newl) fprintf(output, "%s%s ", timebuf, s_infotype[type]);
                            (void)fwrite(data + st, i - st + 1, 1, output);
                            st = i + 1;
                            newl = FALSE;
                        }
                    }
                    if(!newl) fprintf(output, "%s%s ", timebuf, s_infotype[type]);
                    (void)fwrite(data + st, i - st + 1, 1, output);
                }
                newl = (size && (data[size - 1] != '\n')) ? TRUE : FALSE;
                traced_data = FALSE;
                break;
            case CURLINFO_TEXT: case CURLINFO_HEADER_IN:
                if(!newl) fprintf(output, "%s%s ", timebuf, s_infotype[type]);
                (void)fwrite(data, size, 1, output);
                newl = (size && (data[size - 1] != '\n')) ? TRUE : FALSE;
                traced_data = FALSE;
                break;
            case CURLINFO_DATA_OUT: case CURLINFO_DATA_IN: case CURLINFO_SSL_DATA_IN: case CURLINFO_SSL_DATA_OUT:
                if(!traced_data) {
                    if(!config->isatty || ((output != stderr) && (output != stdout))) {
                        if(!newl) fprintf(output, "%s%s ", timebuf, s_infotype[type]);
                        fprintf(output, "[%zu bytes data]\n", size);
                        newl = FALSE;
                        traced_data = TRUE;
                    }
                }
                break;
            default:
                newl = FALSE;
                traced_data = FALSE;
                break;
        }
        return 0;
    }
#ifdef CURL_DOES_CONVERSIONS
    if((type == CURLINFO_HEADER_OUT) && (size > 4)) {
        size_t i;
        for(i = 0; i < size - 4; i++) {
            if(memcmp(&data[i], "\r\n\r\n", 4) == 0) {
                text = "=> Send header";
                dump(timebuf, text, output, (unsigned char *)data, i + 4, config->tracetype, type);
                data += i + 3;
                size -= i + 4;
                type = CURLINFO_DATA_OUT;
                data += 1;
                break;
            }
        }
    }
#endif
    switch(type) {
        case CURLINFO_TEXT: fprintf(output, "%s== Info: %s", timebuf, data);
        case CURLINFO_HEADER_OUT: text = "=> Send header"; break;
        case CURLINFO_DATA_OUT: text = "=> Send data"; break;
        case CURLINFO_HEADER_IN: text = "<= Recv header"; break;
        case CURLINFO_DATA_IN: text = "<= Recv data"; break;
        case CURLINFO_SSL_DATA_IN: text = "<= Recv SSL data"; break;
        case CURLINFO_SSL_DATA_OUT:
          text = "=> Send SSL data";
          break;
        default: return 0;
    }
    dump(timebuf, text, output, (unsigned char *) data, size, config->tracetype, type);
    return 0;
}
static void dump(const char *timebuf, const char *text, FILE *stream, const unsigned char *ptr, size_t size, trace tracetype, curl_infotype infotype) {
    size_t i;
    size_t c;
    unsigned int width = 0x10;
    if(tracetype == TRACE_ASCII) width = 0x40;
    fprintf(stream, "%s%s, %zu bytes (0x%zx)\n", timebuf, text, size, size);
    for(i = 0; i < size; i += width) {
        fprintf(stream, "%04zx: ", i);
        if(tracetype == TRACE_BIN) {
            for(c = 0; c < width; c++)
                if(i + c < size) fprintf(stream, "%02x ", ptr[i + c]);
                else fputs("   ", stream);
        }
        for(c = 0; (c < width) && (i + c < size); c++) {
            if((tracetype == TRACE_ASCII) && (i + c + 1 < size) && (ptr[i + c] == 0x0D) && (ptr[i + c + 1] == 0x0A)) {
                i += (c + 2 - width);
                break;
            }
        #ifdef CURL_DOES_CONVERSIONS
            if((tracetype == TRACE_ASCII) && (i + c + 1 < size) && (ptr[i + c] == '\r') && (ptr[i + c + 1] == '\n')) {
                i += (c + 2 - width);
                break;
            }
            fprintf(stream, "%c", convert_char(infotype, ptr[i + c]));
        #else
            (void)infotype;
            fprintf(stream, "%c", ((ptr[i + c] >= 0x20) && (ptr[i + c] < 0x80)) ? ptr[i + c] : UNPRINTABLE_CHAR);
        #endif
            if((tracetype == TRACE_ASCII) && (i + c + 2 < size) && (ptr[i + c + 1] == 0x0D) && (ptr[i + c + 2] == 0x0A)) {
                i += (c + 3 - width);
                break;
            }
        }
        fputc('\n', stream);
    }
    fflush(stream);
}