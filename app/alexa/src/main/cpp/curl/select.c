#include "curl_setup.h"
#include <limits.h>
#include <errno.h>
#include <stdbool.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#elif defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#if !defined(HAVE_SELECT) && !defined(HAVE_POLL_FINE)
#error "We can't compile without select() or poll() support."
#endif
#if defined(__BEOS__) && !defined(__HAIKU__)
#include <socket.h>
#endif
#ifdef MSDOS
#include <dos.h>
#endif
#ifdef __VXWORKS__
#include <strings.h>
#endif
#include "curl.h"
#include "urldata.h"
#include "connect.h"
#include "select.h"
#include "timeval.h"
#include "warnless.h"

int Curl_wait_ms(timediff_t timeout_ms) {
    int r = 0;
    if(!timeout_ms) return 0;
    if(timeout_ms < 0) return errno;
#if defined(MSDOS)
    delay(timeout_ms);
#elif defined(WIN32)
#if TIMEDIFF_T_MAX >= ULONG_MAX
    if(timeout_ms >= ULONG_MAX) timeout_ms = ULONG_MAX-1;
#endif
    Sleep((ULONG)timeout_ms);
#else
#if defined(HAVE_POLL_FINE)
#if TIMEDIFF_T_MAX > INT_MAX
    if(timeout_ms > INT_MAX) timeout_ms = INT_MAX;
#endif
    r = poll(NULL, 0, (int)timeout_ms);
#else
    struct timeval pending_tv;
    timediff_t tv_sec = timeout_ms / 1000;
    timediff_t tv_usec = (timeout_ms % 1000) * 1000;
#ifdef HAVE_SUSECONDS_T
#if TIMEDIFF_T_MAX > TIME_T_MAX
    if(tv_sec > TIME_T_MAX) tv_sec = TIME_T_MAX;
#endif
    pending_tv.tv_sec = (time_t)tv_sec;
    pending_tv.tv_usec = (suseconds_t)tv_usec;
#else
#if TIMEDIFF_T_MAX > INT_MAX
    if(tv_sec > INT_MAX) tv_sec = INT_MAX;
#endif
    pending_tv.tv_sec = (int)tv_sec;
    pending_tv.tv_usec = (int)tv_usec;
#endif
    r = select(0, NULL, NULL, NULL, &pending_tv);
#endif
#endif
    if(r) r = -1;
    return r;
}
int Curl_select(curl_socket_t maxfd, fd_set *fds_read, fd_set *fds_write, fd_set *fds_err, timediff_t timeout_ms) {
    struct timeval pending_tv;
    struct timeval *ptimeout;
    int r;
#ifdef USE_WINSOCK
    if((!fds_read || fds_read->fd_count == 0) && (!fds_write || fds_write->fd_count == 0) && (!fds_err || fds_err->fd_count == 0)) {
        r = Curl_wait_ms(timeout_ms);
        return r;
    }
#endif
    ptimeout = &pending_tv;
    if(timeout_ms < 0) ptimeout = NULL;
    else if(timeout_ms > 0) {
        timediff_t tv_sec = timeout_ms / 1000;
        timediff_t tv_usec = (timeout_ms % 1000) * 1000;
    #ifdef HAVE_SUSECONDS_T
    #if TIMEDIFF_T_MAX > TIME_T_MAX
        if(tv_sec > TIME_T_MAX) tv_sec = TIME_T_MAX;
    #endif
        pending_tv.tv_sec = (time_t)tv_sec;
        pending_tv.tv_usec = (suseconds_t)tv_usec;
    #elif defined(WIN32)
    #if TIMEDIFF_T_MAX > LONG_MAX
        if(tv_sec > LONG_MAX) tv_sec = LONG_MAX;
    #endif
        pending_tv.tv_sec = (long)tv_sec;
        pending_tv.tv_usec = (long)tv_usec;
    #else
    #if TIMEDIFF_T_MAX > INT_MAX
        if(tv_sec > INT_MAX) tv_sec = INT_MAX;
    #endif
        pending_tv.tv_sec = (int)tv_sec;
        pending_tv.tv_usec = (int)tv_usec;
    #endif
    } else {
        pending_tv.tv_sec = 0;
        pending_tv.tv_usec = 0;
    }
#ifdef USE_WINSOCK
    r = select((int)maxfd + 1, fds_read && fds_read->fd_count ? fds_read : NULL, fds_write && fds_write->fd_count ? fds_write : NULL,
               fds_err && fds_err->fd_count ? fds_err : NULL, ptimeout);
#else
    r = select((int)maxfd + 1, fds_read, fds_write, fds_err, ptimeout);
#endif
    return r;
}
#define VALID_SOCK(s) (((s) >= 0) && ((s) < FD_SETSIZE))
#define VERIFY_SOCK(x) do { \
    if(!VALID_SOCK(x)) { \
        return errno; \
    } \
} while(0)
int Curl_socket_check(curl_socket_t readfd0, curl_socket_t readfd1, curl_socket_t writefd, timediff_t timeout_ms) {
#ifdef HAVE_POLL_FINE
    struct pollfd pfd[3];
    int num;
#else
    fd_set fds_read;
    fd_set fds_write;
    fd_set fds_err;
    curl_socket_t maxfd;
#endif
    int r;
    int ret;
    if((readfd0 == CURL_SOCKET_BAD) && (readfd1 == CURL_SOCKET_BAD) && (writefd == CURL_SOCKET_BAD)) {
        r = Curl_wait_ms(timeout_ms);
        return r;
    }
#ifdef HAVE_POLL_FINE
    num = 0;
    if(readfd0 != CURL_SOCKET_BAD) {
        pfd[num].fd = readfd0;
        pfd[num].events = POLLRDNORM|POLLIN|POLLRDBAND|POLLPRI;
        pfd[num].revents = 0;
        num++;
    }
    if(readfd1 != CURL_SOCKET_BAD) {
        pfd[num].fd = readfd1;
        pfd[num].events = POLLRDNORM|POLLIN|POLLRDBAND|POLLPRI;
        pfd[num].revents = 0;
        num++;
    }
    if(writefd != CURL_SOCKET_BAD) {
        pfd[num].fd = writefd;
        pfd[num].events = POLLWRNORM|POLLOUT;
        pfd[num].revents = 0;
        num++;
    }
    r = Curl_poll(pfd, num, timeout_ms);
    if(r <= 0) return r;
    ret = 0;
    num = 0;
    if(readfd0 != CURL_SOCKET_BAD) {
        if(pfd[num].revents & (POLLRDNORM|POLLIN|POLLERR|POLLHUP)) ret |= CURL_CSELECT_IN;
        if(pfd[num].revents & (POLLRDBAND|POLLPRI|POLLNVAL)) ret |= CURL_CSELECT_ERR;
        num++;
    }
    if(readfd1 != CURL_SOCKET_BAD) {
        if(pfd[num].revents & (POLLRDNORM|POLLIN|POLLERR|POLLHUP)) ret |= CURL_CSELECT_IN2;
        if(pfd[num].revents & (POLLRDBAND|POLLPRI|POLLNVAL)) ret |= CURL_CSELECT_ERR;
        num++;
    }
    if(writefd != CURL_SOCKET_BAD) {
        if(pfd[num].revents & (POLLWRNORM|POLLOUT)) ret |= CURL_CSELECT_OUT;
        if(pfd[num].revents & (POLLERR|POLLHUP|POLLNVAL)) ret |= CURL_CSELECT_ERR;
    }
    return ret;
#else
    FD_ZERO(&fds_err);
    maxfd = (curl_socket_t)-1;
    FD_ZERO(&fds_read);
    if(readfd0 != CURL_SOCKET_BAD) {
        VERIFY_SOCK(readfd0);
        FD_SET(readfd0, &fds_read);
        FD_SET(readfd0, &fds_err);
        maxfd = readfd0;
    }
    if(readfd1 != CURL_SOCKET_BAD) {
        VERIFY_SOCK(readfd1);
        FD_SET(readfd1, &fds_read);
        FD_SET(readfd1, &fds_err);
        if(readfd1 > maxfd) maxfd = readfd1;
    }
    FD_ZERO(&fds_write);
    if(writefd != CURL_SOCKET_BAD) {
        VERIFY_SOCK(writefd);
        FD_SET(writefd, &fds_write);
        FD_SET(writefd, &fds_err);
        if(writefd > maxfd) maxfd = writefd;
    }
    r = Curl_select(maxfd, &fds_read, &fds_write, &fds_err, timeout_ms);
    if(r < 0) return -1;
    if(r == 0) return 0;
    ret = 0;
    if(readfd0 != CURL_SOCKET_BAD) {
        if(FD_ISSET(readfd0, &fds_read)) ret |= CURL_CSELECT_IN;
        if(FD_ISSET(readfd0, &fds_err)) ret |= CURL_CSELECT_ERR;
    }
    if(readfd1 != CURL_SOCKET_BAD) {
        if(FD_ISSET(readfd1, &fds_read)) ret |= CURL_CSELECT_IN2;
        if(FD_ISSET(readfd1, &fds_err)) ret |= CURL_CSELECT_ERR;
    }
    if(writefd != CURL_SOCKET_BAD) {
        if(FD_ISSET(writefd, &fds_write)) ret |= CURL_CSELECT_OUT;
        if(FD_ISSET(writefd, &fds_err)) ret |= CURL_CSELECT_ERR;
    }
    return ret;
#endif
}
int Curl_poll(struct pollfd ufds[], unsigned int nfds, timediff_t timeout_ms) {
#ifdef HAVE_POLL_FINE
    int pending_ms;
#else
    fd_set fds_read;
    fd_set fds_write;
    fd_set fds_err;
    curl_socket_t maxfd;
#endif
    bool fds_none = TRUE;
    unsigned int i;
    int r;
    if(ufds) {
        for(i = 0; i < nfds; i++) {
            if(ufds[i].fd != CURL_SOCKET_BAD) {
                fds_none = FALSE;
                break;
            }
        }
    }
    if(fds_none) {
        r = Curl_wait_ms(timeout_ms);
        return r;
    }
#ifdef HAVE_POLL_FINE
#if TIMEDIFF_T_MAX > INT_MAX
    if(timeout_ms > INT_MAX) timeout_ms = INT_MAX;
#endif
    if(timeout_ms > 0) pending_ms = (int)timeout_ms;
    else if(timeout_ms < 0) pending_ms = -1;
    else pending_ms = 0;
    r = poll(ufds, nfds, pending_ms);
    if(r < 0) return -1;
    if(r == 0) return 0;
    for(i = 0; i < nfds; i++) {
        if(ufds[i].fd == CURL_SOCKET_BAD) continue;
        if(ufds[i].revents & POLLHUP) ufds[i].revents |= POLLIN;
        if(ufds[i].revents & POLLERR) ufds[i].revents |= (POLLIN|POLLOUT);
    }
#else
    FD_ZERO(&fds_read);
    FD_ZERO(&fds_write);
    FD_ZERO(&fds_err);
    maxfd = (curl_socket_t)-1;
    for(i = 0; i < nfds; i++) {
        ufds[i].revents = 0;
        if(ufds[i].fd == CURL_SOCKET_BAD) continue;
        VERIFY_SOCK(ufds[i].fd);
        if(ufds[i].events & (POLLIN|POLLOUT | POLLPRI | POLLRDNORM | POLLWRNORM | POLLRDBAND)) {
            if(ufds[i].fd > maxfd) maxfd = ufds[i].fd;
            if(ufds[i].events & (POLLRDNORM | POLLIN)) FD_SET(ufds[i].fd, &fds_read);
            if(ufds[i].events & (POLLWRNORM | POLLOUT)) FD_SET(ufds[i].fd, &fds_write);
            if(ufds[i].events & (POLLRDBAND | POLLPRI)) FD_SET(ufds[i].fd, &fds_err);
        }
    }
    r = Curl_select(maxfd, &fds_read, &fds_write, &fds_err, timeout_ms);
    if(r < 0) return -1;
    if(r == 0) return 0;
    r = 0;
    for(i = 0; i < nfds; i++) {
        ufds[i].revents = 0;
        if(ufds[i].fd == CURL_SOCKET_BAD) continue;
        if(FD_ISSET(ufds[i].fd, &fds_read)) ufds[i].revents |= POLLIN;
        if(FD_ISSET(ufds[i].fd, &fds_write)) ufds[i].revents |= POLLOUT;
        if(FD_ISSET(ufds[i].fd, &fds_err)) ufds[i].revents |= POLLPRI;
        if(ufds[i].revents != 0) r++;
    }
#endif
    return r;
}
#ifdef TPF
int tpf_select_libcurl(int maxfds, fd_set *reads, fd_set *writes, fd_set *excepts, struct timeval *tv) {
    int rc;
    rc = tpf_select_bsd(maxfds, reads, writes, excepts, tv);
    tpf_process_signals();
    return rc;
}
#endif