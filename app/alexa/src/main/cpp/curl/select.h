#ifndef HEADER_CURL_SELECT_H
#define HEADER_CURL_SELECT_H

#include "curl_setup.h"
#include "curl_setup_once.h"
#ifdef HAVE_POLL_H
#include <poll.h>
#elif defined(HAVE_SYS_POLL_H)
#include <sys/poll.h>
#endif
#if !defined(HAVE_STRUCT_POLLFD) && !defined(HAVE_SYS_POLL_H) && !defined(HAVE_POLL_H) && !defined(POLLIN)
#define POLLIN      0x01
#define POLLPRI     0x02
#define POLLOUT     0x04
#define POLLERR     0x08
#define POLLHUP     0x10
#define POLLNVAL    0x20
struct pollfd {
    curl_socket_t fd;
    short   events;
    short   revents;
};
#endif
#ifndef POLLRDNORM
#define POLLRDNORM POLLIN
#endif
#ifndef POLLWRNORM
#define POLLWRNORM POLLOUT
#endif
#ifndef POLLRDBAND
#define POLLRDBAND POLLPRI
#endif
#define CURL_CSELECT_IN2 (CURL_CSELECT_ERR << 1)
int Curl_select(curl_socket_t maxfd, fd_set *fds_read, fd_set *fds_write, fd_set *fds_err, timediff_t timeout_ms);
int Curl_socket_check(curl_socket_t readfd, curl_socket_t readfd2, curl_socket_t writefd, timediff_t timeout_ms);
#define SOCKET_READABLE(x,z)  Curl_socket_check(x, CURL_SOCKET_BAD, CURL_SOCKET_BAD, z)
#define SOCKET_WRITABLE(x,z)  Curl_socket_check(CURL_SOCKET_BAD, CURL_SOCKET_BAD, x, z)
int Curl_poll(struct pollfd ufds[], unsigned int nfds, timediff_t timeout_ms);
int Curl_wait_ms(timediff_t timeout_ms);
#ifdef TPF
int tpf_select_libcurl(int maxfds, fd_set* reads, fd_set* writes, fd_set* excepts, struct timeval *tv);
#endif
#if defined(USE_WINSOCK) || defined(TPF)
#define VALID_SOCK(x) 1
#define VERIFY_SOCK(x) Curl_nop_stmt
#else
#define VALID_SOCK(s) (((s) >= 0) && ((s) < FD_SETSIZE))
#define VERIFY_SOCK(x) \
    do { \
        if(!VALID_SOCK(x)) return errno; \
    } while(0);
#endif
#endif