#include "libcompat.h"

#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <CoreServices/CoreServices.h>
#include <unistd.h>
#endif
#define NANOSECONDS_PER_SECOND 1000000000

int clock_gettime(clockid_t clk_id CK_ATTRIBUTE_UNUSED, struct timespec *ts) {
#ifdef __APPLE__
    static mach_timebase_info_data_t sTimebaseInfo;
    uint64_t rawTime;
    uint64_t nanos;
    rawTime = mach_absolute_time();
    if(sTimebaseInfo.denom == 0) (void)mach_timebase_info(&sTimebaseInfo);
    nanos = rawTime * sTimebaseInfo.numer / sTimebaseInfo.denom;
    ts->tv_sec = nanos / NANOSECONDS_PER_SECOND;
    ts->tv_nsec = nanos - (ts->tv_sec * NANOSECONDS_PER_SECOND);
#else
    ts->tv_sec = 0;
    ts->tv_nsec = 0;
#endif
    return 0;
}