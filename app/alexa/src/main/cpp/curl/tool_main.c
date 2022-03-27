#include "tool_setup.h"
#include <sys/stat.h>
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef USE_NSS
#include <nspr.h>
#include <plarenas.h>
#endif
#define ENABLE_CURLX_PRINTF
#include "curlx.h"
#include "tool_cfgable.h"
#include "tool_convert.h"
#include "tool_doswin.h"
#include "tool_msgs.h"
#include "tool_operate.h"
#include "tool_panykey.h"
#include "tool_vms.h"
#include "tool_main.h"
#include "tool_libinfo.h"
#include "memdebug.h"

#ifdef __VMS
int vms_show = 0;
#endif
#ifdef __MINGW32__
int _CRT_glob = 0;
#endif
#ifndef UNITTESTS
static void main_checkfds(void) {
#ifdef HAVE_PIPE
    int fd[2] = { STDIN_FILENO, STDIN_FILENO };
    while(fd[0] == STDIN_FILENO || fd[0] == STDOUT_FILENO || fd[0] == STDERR_FILENO || fd[1] == STDIN_FILENO || fd[1] == STDOUT_FILENO || fd[1] == STDERR_FILENO)
        if(pipe(fd) < 0) return;
    close(fd[0]);
    close(fd[1]);
#endif
}
#ifdef CURLDEBUG
static void memory_tracking_init(void) {
    char *env;
    env = curlx_getenv("CURL_MEMDEBUG");
    if(env) {
        char fname[CURL_MT_LOGFNAME_BUFSIZE];
        if(strlen(env) >= CURL_MT_LOGFNAME_BUFSIZE) env[CURL_MT_LOGFNAME_BUFSIZE-1] = '\0';
        strcpy(fname, env);
        curl_free(env);
        curl_dbg_memdebug(fname);
    }
    env = curlx_getenv("CURL_MEMLIMIT");
    if(env) {
        char *endptr;
        long num = strtol(env, &endptr, 10);
        if((endptr != env) && (endptr == env + strlen(env)) && (num > 0)) curl_dbg_memlimit(num);
        curl_free(env);
    }
}
#else
#define memory_tracking_init() Curl_nop_stmt
#endif
static CURLcode main_init(struct GlobalConfig *config) {
    CURLcode result = CURLE_OK;
#if defined(__DJGPP__) || defined(__GO32__)
    _djstat_flags |= _STAT_INODE | _STAT_EXEC_MAGIC | _STAT_DIRSIZE;
#endif
    config->showerror = -1;
    config->errors = stderr;
    config->styled_output = TRUE;
    config->parallel_max = PARALLEL_DEFAULT;
    config->first = config->last = malloc(sizeof(struct OperationConfig));
    if(config->first) {
        result = curl_global_init(CURL_GLOBAL_DEFAULT);
        if(!result) {
            result = get_libcurl_info();
            if(!result) {
                config_init(config->first);
                config->first->global = config;
            } else {
                errorf(config, "error retrieving curl library information\n");
                free(config->first);
            }
        } else {
            errorf(config, "error initializing curl library\n");
            free(config->first);
        }
    } else {
        errorf(config, "error initializing curl\n");
        result = CURLE_FAILED_INIT;
    }
    return result;
}
static void free_globalconfig(struct GlobalConfig *config) {
    Curl_safefree(config->trace_dump);
    if(config->errors_fopened && config->errors) fclose(config->errors);
    config->errors = NULL;
    if(config->trace_fopened && config->trace_stream) fclose(config->trace_stream);
    config->trace_stream = NULL;
    Curl_safefree(config->libcurl);
}
static void main_free(struct GlobalConfig *config) {
    curl_global_cleanup();
    convert_cleanup();
    metalink_cleanup();
#ifdef USE_NSS
    if(PR_Initialized()) {
        PL_ArenaFinish();
        PR_Cleanup();
    }
#endif
    free_globalconfig(config);
    config_free(config->last);
    config->first = NULL;
    config->last = NULL;
}
#ifdef WIN32
static struct TerminalSettings {
    HANDLE hStdOut;
    DWORD dwOutputMode;
} TerminalSettings;
static void configure_terminal(void) {
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
    #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
    memset(&TerminalSettings, 0, sizeof(TerminalSettings));
    TerminalSettings.hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if((TerminalSettings.hStdOut != INVALID_HANDLE_VALUE) && (GetConsoleMode(TerminalSettings.hStdOut, &TerminalSettings.dwOutputMode))) {
        SetConsoleMode(TerminalSettings.hStdOut, TerminalSettings.dwOutputMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}
#else
#define configure_terminal()
#endif
static void restore_terminal(void) {
#ifdef WIN32
    SetConsoleMode(TerminalSettings.hStdOut, TerminalSettings.dwOutputMode);
#endif
}
#ifdef _UNICODE
int wmain(int argc, wchar_t *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    CURLcode result = CURLE_OK;
    struct GlobalConfig global;
    memset(&global, 0, sizeof(global));
#ifdef WIN32
#ifdef _tcscmp
    if(argc == 2 && !_tcscmp(argv[1], _T("--dump-module-paths"))) {
        struct curl_slist *item, *head = GetLoadedModulePaths();
        for(item = head; item; item = item->next) printf("%s\n", item->data);
        curl_slist_free_all(head);
        return head ? 0 : 1;
    }
#endif
    result = win32_init();
    if(result) {
        fprintf(stderr, "curl: (%d) Windows-specific init failed.\n", result);
        return result;
    }
#endif
    configure_terminal();
    main_checkfds();
#if defined(HAVE_SIGNAL) && defined(SIGPIPE)
    (void)signal(SIGPIPE, SIG_IGN);
#endif
    memory_tracking_init();
    result = main_init(&global);
    if(!result) {
        result = operate(&global, argc, argv);
    #ifdef __SYMBIAN32__
        if(global.showerror) tool_pressanykey();
    #endif
        main_free(&global);
    }
    restore_terminal();
#ifdef __NOVELL_LIBC__
    if(getenv("_IN_NETWARE_BASH_") == NULL) tool_pressanykey();
#endif
#ifdef __VMS
    vms_special_exit(result, vms_show);
#else
    return (int)result;
#endif
}

#endif