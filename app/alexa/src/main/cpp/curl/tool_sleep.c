#include "tool_setup.h"
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#elif defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#ifdef HAVE_POLL_H
#include <poll.h>
#elif defined(HAVE_SYS_POLL_H)
#include <sys/poll.h>
#endif
#ifdef MSDOS
#include <dos.h>
#endif
#include "tool_sleep.h"
#include "memdebug.h"

void tool_go_sleep(long ms) {
#if defined(MSDOS)
    delay(ms);
#elif defined(WIN32)
    Sleep(ms);
#elif defined(HAVE_POLL_FINE)
    (void)poll((void *)0, 0, (int)ms);
#else
    struct timeval timeout;
    timeout.tv_sec = ms / 1000L;
    ms = ms % 1000L;
    timeout.tv_usec = (int)ms * 1000;
    select(0, NULL,  NULL, NULL, &timeout);
#endif
}