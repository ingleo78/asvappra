#ifndef foopulsertclockhfoo
#define foopulsertclockhfoo

#include "../pulse/sample.h"
#include "macro.h"

struct timeval;
struct timeval *pa_rtclock_get(struct timeval *ts);
pa_usec_t pa_rtclock_age(const struct timeval *tv);
bool pa_rtclock_hrtimer(void);
void pa_rtclock_hrtimer_enable(void);
#define PA_HRTIMER_THRESHOLD_USEC 10
#define PA_TIMEVAL_RTCLOCK ((time_t) (1LU << 30))
struct timeval* pa_rtclock_from_wallclock(struct timeval *tv);
#ifdef HAVE_CLOCK_GETTIME
struct timespec;
pa_usec_t pa_timespec_load(const struct timespec *ts);
struct timespec* pa_timespec_store(struct timespec *ts, pa_usec_t v);
#endif
struct timeval* pa_timeval_rtstore(struct timeval *tv, pa_usec_t v, bool rtclock);
#endif