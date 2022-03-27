#include "curl_setup.h"

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if (defined(HAVE_IOCTL_FIONBIO) && defined(NETWARE))
#include <sys/filio.h>
#endif
#ifdef __VMS
#include <in.h>
#include <inet.h>
#endif
#include "nonblock.h"

int curlx_nonblock(curl_socket_t sockfd, int nonblock) {
#if defined(USE_BLOCKING_SOCKETS)
    (void)sockfd;
    (void)nonblock;
    return 0;
#elif defined(HAVE_FCNTL_O_NONBLOCK)
    int flags;
    flags = sfcntl(sockfd, F_GETFL, 0);
    if(nonblock) return sfcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    return sfcntl(sockfd, F_SETFL, flags & (~O_NONBLOCK));
#elif defined(HAVE_IOCTL_FIONBIO)
    int flags = nonblock ? 1 : 0;
    return ioctl(sockfd, FIONBIO, &flags);
#elif defined(HAVE_IOCTLSOCKET_FIONBIO)
    unsigned long flags = nonblock ? 1UL : 0UL;
    return ioctlsocket(sockfd, FIONBIO, &flags);
#elif defined(HAVE_IOCTLSOCKET_CAMEL_FIONBIO)
    long flags = nonblock ? 1L : 0L;
    return IoctlSocket(sockfd, FIONBIO, (char*)&flags);
#elif defined(HAVE_SETSOCKOPT_SO_NONBLOCK)
    long b = nonblock ? 1L : 0L;
    return setsockopt(sockfd, SOL_SOCKET, SO_NONBLOCK, &b, sizeof(b));
#else
    #error "no non-blocking method was found/used/set"
#endif
}