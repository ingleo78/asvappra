#include "curl_setup.h"
#include "socketpair.h"
#ifndef HAVE_SOCKETPAIR
#ifdef WIN32
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#else
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK 0x7f000001
#endif
#endif
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

int Curl_socketpair(int domain, int type, int protocol, curl_socket_t socks[2]) {
    union {
        struct sockaddr_in inaddr;
        struct sockaddr addr;
    } a;
    curl_socket_t listener;
    curl_socklen_t addrlen = sizeof(a.inaddr);
    int reuse = 1;
    char data[2][12];
    ssize_t dlen;
    (void)domain;
    (void)type;
    (void)protocol;
    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(listener == CURL_SOCKET_BAD) return -1;
    memset(&a, 0, sizeof(a));
    a.inaddr.sin_family = AF_INET;
    a.inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.inaddr.sin_port = 0;
    socks[0] = socks[1] = CURL_SOCKET_BAD;
    if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, (curl_socklen_t)sizeof(reuse)) == -1) goto error;
    if(bind(listener, &a.addr, sizeof(a.inaddr)) == -1) goto error;
    if(getsockname(listener, &a.addr, &addrlen) == -1) goto error;
    if(listen(listener, 1) == -1) goto error;
    socks[0] = socket(AF_INET, SOCK_STREAM, 0);
    if(socks[0] == CURL_SOCKET_BAD) goto error;
    if(connect(socks[0], &a.addr, sizeof(a.inaddr)) == -1) goto error;
    socks[1] = accept(listener, NULL, NULL);
    if(socks[1] == CURL_SOCKET_BAD) goto error;
    msnprintf(data[0], sizeof(data[0]), "%p", socks);
    dlen = strlen(data[0]);
    if(swrite(socks[0], data[0], dlen) != dlen) goto error;
    if(sread(socks[1], data[1], sizeof(data[1])) != dlen) goto error;
    if(memcmp(data[0], data[1], dlen)) goto error;
    sclose(listener);
    return 0;
    error:
    sclose(listener);
    sclose(socks[0]);
    sclose(socks[1]);
    return -1;
}

#endif