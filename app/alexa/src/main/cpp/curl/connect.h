#ifndef HEADER_CURL_CONNECT_H
#define HEADER_CURL_CONNECT_H

#include <netinet/in.h>
#include "curl_setup.h"
#include "nonblock.h"
#include "sockaddr.h"
#include "timeval.h"
#include "multi.h"

CURLcode Curl_is_connected(struct connectdata *conn, int sockindex, bool *connected);
CURLcode Curl_connecthost(struct connectdata *conn, const struct Curl_dns_entry *host);
timediff_t Curl_timeleft(struct Curl_easy *data, struct curltime *nowp, bool duringconnect);
#define DEFAULT_CONNECT_TIMEOUT 300000
curl_socket_t Curl_getconnectinfo(struct Curl_easy *data, struct connectdata **connp);
bool Curl_addr2string(struct sockaddr *sa, curl_socklen_t salen, char *addr, long *port);
bool Curl_connalive(struct connectdata *conn);
#ifdef USE_WINSOCK
    void Curl_sndbufset(curl_socket_t sockfd);
#else
    #define Curl_sndbufset(y) Curl_nop_stmt
#endif
void Curl_updateconninfo(struct connectdata *conn, curl_socket_t sockfd);
void Curl_persistconninfo(struct connectdata *conn);
int Curl_closesocket(struct connectdata *conn, curl_socket_t sock);
struct Curl_sockaddr_ex {
    int family;
    int socktype;
    int protocol;
    unsigned int addrlen;
    union {
        struct sockaddr addr;
        struct Curl_sockaddr_storage buff;
    } _sa_ex_u;
};
#define sa_addr _sa_ex_u.addr
CURLcode Curl_socket(struct connectdata *conn, const struct Curl_addrinfo *ai, struct Curl_sockaddr_ex *addr, curl_socket_t *sockfd);
#define CONNCTRL_KEEP 0
#define CONNCTRL_CONNECTION 1
#define CONNCTRL_STREAM 2
void Curl_conncontrol(struct connectdata *conn, int closeit
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
    , const char *reason
#endif
    );
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
#define streamclose(x,y) Curl_conncontrol(x, CONNCTRL_STREAM, y)
#define connclose(x,y) Curl_conncontrol(x, CONNCTRL_CONNECTION, y)
#define connkeep(x,y) Curl_conncontrol(x, CONNCTRL_KEEP, y)
#else
#define streamclose(x,y) Curl_conncontrol(x, CONNCTRL_STREAM)
#define connclose(x,y) Curl_conncontrol(x, CONNCTRL_CONNECTION)
#define connkeep(x,y) Curl_conncontrol(x, CONNCTRL_KEEP)
#endif
bool Curl_conn_data_pending(struct connectdata *conn, int sockindex);
#endif