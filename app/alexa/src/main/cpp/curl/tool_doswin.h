#ifndef HEADER_CURL_TOOL_DOSWIN_H
#define HEADER_CURL_TOOL_DOSWIN_H

#include "tool_setup.h"

#if defined(MSDOS) || defined(WIN32)
#define SANITIZE_ALLOW_COLONS    (1<<0)
#define SANITIZE_ALLOW_PATH      (1<<1)
#define SANITIZE_ALLOW_RESERVED  (1<<2)
#define SANITIZE_ALLOW_TRUNCATE  (1<<3)
typedef enum {
    SANITIZE_ERR_OK = 0,
    SANITIZE_ERR_INVALID_PATH,
    SANITIZE_ERR_BAD_ARGUMENT,
    SANITIZE_ERR_OUT_OF_MEMORY,
    SANITIZE_ERR_LAST
} SANITIZEcode;
SANITIZEcode sanitize_file_name(char **const sanitized, const char *file_name, int flags);
#ifdef UNITTESTS
SANITIZEcode truncate_dryrun(const char *path, const size_t truncate_pos);
SANITIZEcode msdosify(char **const sanitized, const char *file_name, int flags);
SANITIZEcode rename_if_reserved_dos_device_name(char **const sanitized, const char *file_name, int flags);
#endif
#if defined(MSDOS) && (defined(__DJGPP__) || defined(__GO32__))
char **__crt0_glob_function(char *arg);
#endif
#ifdef WIN32
CURLcode FindWin32CACert(struct OperationConfig *config, curl_sslbackend backend, const char *bundle_file);
struct curl_slist *GetLoadedModulePaths(void);
CURLcode win32_init(void);
#endif
#endif
#endif