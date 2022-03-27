#include "tool_setup.h"
#include "tool_util.h"
#include "memdebug.h"

#if defined(WIN32) && !defined(MSDOS)
extern LARGE_INTEGER tool_freq;
extern bool tool_isVistaOrGreater;
struct timeval tvnow(void) {
    struct timeval now;
    if(tool_isVistaOrGreater) {
        LARGE_INTEGER count;
        QueryPerformanceCounter(&count);
        now.tv_sec = (long)(count.QuadPart / tool_freq.QuadPart);
        now.tv_usec = (long)((count.QuadPart % tool_freq.QuadPart) * 1000000 / tool_freq.QuadPart);
    } else {
        #if defined(_MSC_VER)
        #pragma warning(push)
        #pragma warning(disable:28159)
        #endif
        DWORD milliseconds = GetTickCount();
        #if defined(_MSC_VER)
        #pragma warning(pop)
        #endif
        now.tv_sec = (long)(milliseconds / 1000);
        now.tv_usec = (long)((milliseconds % 1000) * 1000);
    }
    return now;
}
#elif defined(HAVE_CLOCK_GETTIME_MONOTONIC)
struct timeval tvnow(void) {
    struct timeval now;
    struct timespec tsnow;
    if(0 == clock_gettime(CLOCK_MONOTONIC, &tsnow)) {
        now.tv_sec = tsnow.tv_sec;
        now.tv_usec = tsnow.tv_nsec / 1000;
    }
#ifdef HAVE_GETTIMEOFDAY
    else (void)gettimeofday(&now, NULL);
#else
    else {
        now.tv_sec = (long)time(NULL);
        now.tv_usec = 0;
    }
#endif
    return now;
}
#elif defined(HAVE_GETTIMEOFDAY)
struct timeval tvnow(void) {
    struct timeval now;
    (void)gettimeofday(&now, NULL);
    return now;
}
#else
struct timeval tvnow(void) {
    struct timeval now;
    now.tv_sec = (long)time(NULL);
    now.tv_usec = 0;
    return now;
}
#endif
long tvdiff(struct timeval newer, struct timeval older) {
  return (long)(newer.tv_sec-older.tv_sec) * 1000 + (long)(newer.tv_usec-older.tv_usec) / 1000;
}