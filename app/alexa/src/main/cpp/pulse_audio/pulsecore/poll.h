#if defined(HAVE_POLL_H)
#include <poll.h>
#else

#define POLLIN  0x001
#define POLLPRI  0x002
#define POLLOUT  0x004
#define POLLERR  0x008
#define POLLHUP  0x010
#define POLLNVAL  0x020
struct pollfd {
    int fd;
    short int events;
    short int revents;
};
#endif
#if defined(HAVE_POLL_H) && !defined(OS_IS_DARWIN)
#define pa_poll(fds,nfds,timeout) poll((fds),(nfds),(timeout))
#else
int pa_poll(struct pollfd *fds, unsigned long nfds, int timeout);
#endif