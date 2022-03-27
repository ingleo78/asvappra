#include "timeval.h"

#if defined(WIN32) && !defined(MSDOS)
extern LARGE_INTEGER Curl_freq;
extern bool Curl_isVistaOrGreater;
struct curltime Curl_now(void) {
    struct curltime now;
    if(Curl_isVistaOrGreater) {
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    now.tv_sec = (time_t)(count.QuadPart / Curl_freq.QuadPart);
    now.tv_usec = (int)((count.QuadPart % Curl_freq.QuadPart) * 1000000 / Curl_freq.QuadPart);
    } else {
    #if defined(_MSC_VER)
        #pragma warning(push)
        #pragma warning(disable:28159)
    #endif
        DWORD milliseconds = GetTickCount();
    #if defined(_MSC_VER)
        #pragma warning(pop)
    #endif
        now.tv_sec = milliseconds / 1000;
        now.tv_usec = (milliseconds % 1000) * 1000;
    }
    return now;
}
#elif defined(HAVE_CLOCK_GETTIME_MONOTONIC)
struct curltime Curl_now(void) {
#ifdef HAVE_GETTIMEOFDAY
    struct timeval now;
#endif
    struct curltime cnow;
    struct timespec tsnow;
#if defined(__APPLE__) && (HAVE_BUILTIN_AVAILABLE == 1)
    bool have_clock_gettime = FALSE;
    if(__builtin_available(macOS 10.12, iOS 10, tvOS 10, watchOS 3, *)) have_clock_gettime = TRUE;
#endif
    if(
#if defined(__APPLE__) && (HAVE_BUILTIN_AVAILABLE == 1)
       have_clock_gettime &&
#endif
       (0 == clock_gettime(CLOCK_MONOTONIC, &tsnow))) {
        cnow.tv_sec = tsnow.tv_sec;
        cnow.tv_usec = (unsigned int)(tsnow.tv_nsec / 1000);
    }
#ifdef HAVE_GETTIMEOFDAY
    else {
        (void)gettimeofday(&now, NULL);
        cnow.tv_sec = now.tv_sec;
        cnow.tv_usec = (unsigned int)now.tv_usec;
    }
#else
    else {
        cnow.tv_sec = time(NULL);
        cnow.tv_usec = 0;
    }
#endif
    return cnow;
}
#elif defined(HAVE_MACH_ABSOLUTE_TIME)
#include <stdint.h>
#include <mach/mach_time.h>
struct curltime Curl_now(void) {
    static mach_timebase_info_data_t timebase;
    struct curltime cnow;
    uint64_t usecs;
    if(0 == timebase.denom) (void)mach_timebase_info(&timebase);
    usecs = mach_absolute_time();
    usecs *= timebase.numer;
    usecs /= timebase.denom;
    usecs /= 1000;
    cnow.tv_sec = usecs / 1000000;
    cnow.tv_usec = (int)(usecs % 1000000);
    return cnow;
}
#elif defined(HAVE_GETTIMEOFDAY)
struct curltime Curl_now(void) {
    struct timeval now;
    struct curltime ret;
    (void)gettimeofday(&now, NULL);
    ret.tv_sec = now.tv_sec;
    ret.tv_usec = (int)now.tv_usec;
    return ret;
}
#else
struct curltime Curl_now(void) {
    struct curltime now;
    now.tv_sec = time(NULL);
    now.tv_usec = 0;
    return now;
}
#endif
timediff_t Curl_timediff(struct curltime newer, struct curltime older) {
    timediff_t diff = (timediff_t)newer.tv_sec-older.tv_sec;
    if(diff >= (TIMEDIFF_T_MAX/1000)) return TIMEDIFF_T_MAX;
    else if(diff <= (TIMEDIFF_T_MIN/1000)) return TIMEDIFF_T_MIN;
    return diff * 1000 + (newer.tv_usec-older.tv_usec)/1000;
}
timediff_t Curl_timediff_us(struct curltime newer, struct curltime older) {
    timediff_t diff = (timediff_t)newer.tv_sec-older.tv_sec;
    if(diff >= (TIMEDIFF_T_MAX/1000000)) return TIMEDIFF_T_MAX;
    else if(diff <= (TIMEDIFF_T_MIN/1000000)) return TIMEDIFF_T_MIN;
    return diff * 1000000 + newer.tv_usec-older.tv_usec;
}