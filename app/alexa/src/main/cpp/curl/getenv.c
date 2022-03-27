#include "curl_setup.h"
#include "curl.h"
#include "curl_memory.h"
#include "memdebug.h"

static char *GetEnv(const char *variable) {
#if defined(_WIN32_WCE) || defined(CURL_WINDOWS_APP)
    (void)variable;
    return NULL;
#elif defined(WIN32)
    char *buf = NULL;
    char *tmp;
    DWORD bufsize;
    DWORD rc = 1;
    const DWORD max = 32768;
    for( ; ; ) {
        tmp = realloc(buf, rc);
        if (!tmp) {
            free(buf);
            return NULL;
        }
        buf = tmp;
        bufsize = rc;
        rc = GetEnvironmentVariableA(variable, buf, bufsize);
        if (!rc || rc == bufsize || rc > max) {
            free(buf);
            return NULL;
        }
        if (rc < bufsize) return buf;
    }
#else
    char *env = getenv(variable);
    return (env && env[0])?strdup(env):NULL;
#endif
}
char *curl_getenv(const char *v) { return GetEnv(v); }