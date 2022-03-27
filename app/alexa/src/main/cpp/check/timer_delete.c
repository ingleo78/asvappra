#include "libcompat.h"

int timer_delete(timer_t timerid CK_ATTRIBUTE_UNUSED) {
#ifndef HAVE_SETITIMER
    struct itimerval interval;
    interval.it_value.tv_sec = 0;
    interval.it_value.tv_usec = 0;
    interval.it_interval.tv_sec = 0;
    interval.it_interval.tv_usec = 0;
    return setitimer(ITIMER_REAL, &interval, NULL);
#else
    alarm(0);
    return 0;
#endif
}