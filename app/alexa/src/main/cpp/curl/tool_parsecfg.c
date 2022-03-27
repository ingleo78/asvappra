#include <stdbool.h>
#include "tool_setup.h"
#include "curlx.h"
#include "curl_ctype.h"
#include "tool_cfgable.h"
#include "tool_getparam.h"
#include "tool_helpers.h"
#include "tool_homedir.h"
#include "tool_msgs.h"
#include "tool_parsecfg.h"
#include "memdebug.h"

#define DIR_CHAR "/"
#define ENABLE_CURLX_PRINTF
#define ISSEP(x,dash) (!dash && (((x) == '=') || ((x) == ':')))

static const char *unslashquote(const char *line, char *param);
static char *my_get_line(FILE *fp);

#ifdef WIN32
static FILE *execpath(const char *filename) {
    char filebuffer[512];
    unsigned long len = GetModuleFileNameA(0, filebuffer, sizeof(filebuffer));
    if(len > 0 && len < sizeof(filebuffer)) {
        char *lastdirchar = strrchr(filebuffer, '\\');
        if(lastdirchar) {
            size_t remaining;
            *lastdirchar = 0;
            remaining = sizeof(filebuffer) - strlen(filebuffer);
            if(strlen(filename) < remaining - 1) {
                msnprintf(lastdirchar, remaining, "%s%s", DIR_CHAR, filename);
                return fopen(filebuffer, FOPEN_READTEXT);
            }
        }
    }
    return NULL;
}
#endif
int parseconfig(const char *filename, struct GlobalConfig *global) {
    FILE *file = NULL;
    bool usedarg = FALSE;
    int rc = 0;
    struct OperationConfig *operation = global->last;
    char *pathalloc = NULL;
    if(!filename || !*filename) {
        char *home = homedir();
    #ifndef WIN32
        if(home) {
            pathalloc = curl_maprintf("%s%s.curlrc", home, DIR_CHAR);
            if(!pathalloc) {
                free(home);
                return 1;
            }
            filename = pathalloc;
        }
    #else
        if(home) {
            int i = 0;
            char prefix = '.';
            do {
                curl_free(pathalloc);
                pathalloc = curl_maprintf("%s%s%ccurlrc", home, DIR_CHAR, prefix);
                if(!pathalloc) {
                    free(home);
                    return 1;
                }
                file = fopen(pathalloc, FOPEN_READTEXT);
                if(file) {
                    filename = pathalloc;
                    break;
                }
                prefix = '_';
            } while(++i < 2);
        }
        if(!filename) {
            file = execpath(".curlrc");
            if(!file) file = execpath("_curlrc");
        }
    #endif
        Curl_safefree(home);
    }
    if(!file && filename) {
        if(strcmp(filename, "-")) file = fopen(filename, FOPEN_READTEXT);
        else file = stdin;
    }
    if(file) {
        char *line;
        char *aline;
        char *option;
        char *param;
        int lineno = 0;
        bool dashed_option;
        while(NULL != (aline = my_get_line(file))) {
            int res;
            bool alloced_param = FALSE;
            lineno++;
            line = aline;
            while(*line && ISSPACE(*line)) line++;
            switch(*line) {
                case '#': case '/': case '\r': case '\n': case '*': case '\0': Curl_safefree(aline); continue;
            }
            option = line;
            dashed_option = option[0]=='-' ? TRUE : FALSE;
            while(*line && !ISSPACE(*line) && !ISSEP(*line, dashed_option)) line++;
            if(*line) *line++ = '\0';
        #ifdef DEBUG_CONFIG
            fprintf(stderr, "GOT: %s\n", option);
        #endif
            while(*line && (ISSPACE(*line) || ISSEP(*line, dashed_option))) line++;
            if(*line == '\"') {
                line++;
                param = malloc(strlen(line) + 1);
                if(!param) {
                    Curl_safefree(aline);
                    rc = 1;
                    break;
                }
                alloced_param = TRUE;
                (void)unslashquote(line, param);
            } else {
                param = line;
                while(*line && !ISSPACE(*line)) line++;
                if(*line) {
                    *line = '\0';
                    line++;
                    while(*line && ISSPACE(*line)) line++;
                    switch(*line) {
                    case '\0': case '\r': case '\n': case '#': break;
                    default:
                        warnf(operation->global, "%s:%d: warning: '%s' uses unquoted white space in the line that may cause side-effects!\n",
                              filename, lineno, option);
                    }
                }
                if(!*param) param = NULL;
            }
        #ifdef DEBUG_CONFIG
            fprintf(stderr, "PARAM: \"%s\"\n",(param ? param : "(null)"));
        #endif
            res = getparameter(option, param, &usedarg, global, operation);
            operation = global->last;
            if(!res && param && *param && !usedarg) res = PARAM_GOT_EXTRA_PARAMETER;
            if(res == PARAM_NEXT_OPERATION) {
                if(operation->url_list && operation->url_list->url) {
                    operation->next = malloc(sizeof(struct OperationConfig));
                    if(operation->next) {
                        config_init(operation->next);
                        operation->next->global = global;
                        global->last = operation->next;
                        operation->next->prev = operation;
                        operation = operation->next;
                    } else res = PARAM_NO_MEM;
                }
            }
            if(res != PARAM_OK && res != PARAM_NEXT_OPERATION) {
                if(!strcmp(filename, "-")) filename = "<stdin>";
                if(res != PARAM_HELP_REQUESTED && res != PARAM_MANUAL_REQUESTED && res != PARAM_VERSION_INFO_REQUESTED && res != PARAM_ENGINES_REQUESTED) {
                    const char *reason = param2text(res);
                    warnf(operation->global, "%s:%d: warning: '%s' %s\n", filename, lineno, option, reason);
                }
            }
            if(alloced_param) Curl_safefree(param);
            Curl_safefree(aline);
        }
        if(file != stdin) fclose(file);
    } else rc = 1;
    curl_free(pathalloc);
    return rc;
}
static const char *unslashquote(const char *line, char *param) {
    while(*line && (*line != '\"')) {
        if(*line == '\\') {
            char out;
            line++;
            switch(out = *line) {
                case '\0': continue;
                case 't': out = '\t'; break;
                case 'n': out = '\n'; break;
                case 'r': out = '\r'; break;
                case 'v': out = '\v'; break;
            }
            *param++ = out;
            line++;
        } else *param++ = *line++;
    }
    *param = '\0';
    return line;
}
static char *my_get_line(FILE *fp) {
    char buf[4096];
    char *nl = NULL;
    char *line = NULL;
    do {
        if(NULL == fgets(buf, sizeof(buf), fp)) break;
        if(!line) {
            line = strdup(buf);
            if(!line) return NULL;
        } else {
            char *ptr;
            size_t linelen = strlen(line);
            ptr = realloc(line, linelen + strlen(buf) + 1);
            if(!ptr) {
                Curl_safefree(line);
                return NULL;
            }
            line = ptr;
            strcpy(&line[linelen], buf);
        }
        nl = strchr(line, '\n');
    } while(!nl);
    if(nl) *nl = '\0';
    return line;
}