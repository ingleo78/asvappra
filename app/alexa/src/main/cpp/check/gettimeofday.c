#include <errno.h>
#include "libcompat.h"

#if defined(_MSC_VER) || defined(__BORLANDC__)
#define EPOCHFILETIME (116444736000000000i64)
#else
#define EPOCHFILETIME (116444736000000000LL)
#endif
int gettimeofday(struct timeval *tv, struct timezone *tz) {
#if defined(_MSC_VER)
    union {
        __int64 ns100;
        FILETIME ft;
    } now;
    GetSystemTimeAsFileTime(&now.ft);
    tv->tv_usec = (long)((now.ns100 / 10LL) % 1000000LL);
    tv->tv_sec = (long)((now.ns100 - EPOCHFILETIME) / 10000000LL);
    return (0);
#else
    errno = ENOSYS;
    return -1;
#endif
}
