#include "tool_setup.h"
#define ENABLE_CURLX_PRINTF
#include "curlx.h"
#include "curl_ctype.h"
#include "tool_cfgable.h"
#include "tool_msgs.h"
#include "memdebug.h"

#define WARN_PREFIX "Warning: "
#define NOTE_PREFIX "Note: "
#define ERROR_PREFIX "curl: "

static void voutf(struct GlobalConfig *config, const char *prefix, const char *fmt, va_list ap) {
    size_t width = (79 - strlen(prefix));
    if(!config->mute) {
        size_t len;
        char *ptr;
        char *print_buffer;
        print_buffer = curlx_mvaprintf(fmt, ap);
        if(!print_buffer) return;
        len = strlen(print_buffer);
        ptr = print_buffer;
        while(len > 0) {
            fputs(prefix, config->errors);
            if(len > width) {
                size_t cut = width-1;
                while(!ISSPACE(ptr[cut]) && cut) cut--;
                if(0 == cut) cut = width-1;
                (void)fwrite(ptr, cut + 1, 1, config->errors);
                fputs("\n", config->errors);
                ptr += cut + 1; /* skip the space too */
                len -= cut + 1;
            } else {
                fputs(ptr, config->errors);
                len = 0;
            }
        }
        curl_free(print_buffer);
    }
}
void notef(struct GlobalConfig *config, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    if(config->tracetype) voutf(config, NOTE_PREFIX, fmt, ap);
    va_end(ap);
}
void warnf(struct GlobalConfig *config, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    voutf(config, WARN_PREFIX, fmt, ap);
    va_end(ap);
}
void helpf(FILE *errors, const char *fmt, ...) {
    if(fmt) {
        va_list ap;
        va_start(ap, fmt);
        fputs("curl: ", errors);
        vfprintf(errors, fmt, ap);
        va_end(ap);
    }
    fprintf(errors, "curl: try 'curl --help' "
        #ifdef USE_MANUAL
           "or 'curl --manual' "
        #endif
           "for more information\n");
}
void errorf(struct GlobalConfig *config, const char *fmt, ...) {
    if(!config->mute) {
        va_list ap;
        va_start(ap, fmt);
        voutf(config, ERROR_PREFIX, fmt, ap);
        va_end(ap);
    }
}