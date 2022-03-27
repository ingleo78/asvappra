#include "curl_setup.h"

#ifdef CURLRES_IPV6
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef __VMS
#include <in.h>
#include <inet.h>
#endif
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif
#include <android/log.h>
#include "urldata.h"
#include "sendf.h"
#include "hostip.h"
#include "hash.h"
#include "share.h"
#include "strerror.h"
#include "url.h"
#include "inet_pton.h"
#include "connect.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

bool Curl_ipv6works(struct connectdata *conn) {
    if(conn) {
        DEBUGASSERT(conn);
        DEBUGASSERT(conn->data);
        DEBUGASSERT(conn->data->multi);
        return conn->data->multi->ipv6_works;
    } else {
        int ipv6_works = -1;
        curl_socket_t s = socket(PF_INET6, SOCK_DGRAM, 0);
        if(s == CURL_SOCKET_BAD) ipv6_works = 0;
        else {
            ipv6_works = 1;
            Curl_closesocket(NULL, s);
        }
        return (ipv6_works>0)?TRUE:FALSE;
    }
}
bool Curl_ipvalid(struct connectdata *conn) {
    if (conn->ip_version == CURL_IPRESOLVE_V6) return Curl_ipv6works(conn);
    return TRUE;
}
#if defined(CURLRES_SYNCH)
#ifdef DEBUG_ADDRINFO
static void dump_addrinfo(struct connectdata *conn, const struct Curl_addrinfo *ai) {
    printf("dump_addrinfo:\n");
    for( ; ai; ai = ai->ai_next) {
        char buf[INET6_ADDRSTRLEN];
        printf("    fam %2d, CNAME %s, ", ai->ai_family, ai->ai_canonname ? ai->ai_canonname : "<none>");
        Curl_printable_address(ai, buf, sizeof(buf));
        printf("%s\n", buf);
    }
}
#else
#define dump_addrinfo(x,y) Curl_nop_stmt
#endif
struct Curl_addrinfo *Curl_getaddrinfo(struct connectdata *conn, const char *hostname, int port, int *waitp) {
    struct addrinfo hints;
    struct Curl_addrinfo *res;
    int error;
    char sbuf[12];
    char *sbufptr = NULL;
#ifndef USE_RESOLVE_ON_IPS
    char addrbuf[128];
#endif
    int pf;
#if !defined(CURL_DISABLE_VERBOSE_STRINGS)
    struct Curl_easy *data = conn->data;
#endif
    *waitp = 0;
    switch(conn->ip_version) {
        case CURL_IPRESOLVE_V4: pf = PF_INET; break;
        case CURL_IPRESOLVE_V6: pf = PF_INET6; break;
        default: pf = PF_UNSPEC; break;
    }
    if ((pf != PF_INET) && !Curl_ipv6works(conn)) pf = PF_INET;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = pf;
    hints.ai_socktype = (conn->transport == TRNSPRT_TCP) ? SOCK_STREAM : SOCK_DGRAM;
#ifndef USE_RESOLVE_ON_IPS
    if ((1 == Curl_inet_pton(AF_INET, hostname, addrbuf)) || (1 == Curl_inet_pton(AF_INET6, hostname, addrbuf))) hints.ai_flags = AI_NUMERICHOST;
#endif
    if (port) {
        msnprintf(sbuf, sizeof(sbuf), "%d", port);
        sbufptr = sbuf;
    }
    error = Curl_getaddrinfo_ex(hostname, sbufptr, &hints, &res);
    if (error) {
        infof(data, "getaddrinfo(3) failed for %s:%d\n", hostname, port);
        return NULL;
    }
    if (port) Curl_addrinfo_set_port(res, port);
    dump_addrinfo(conn, res);
    return res;
}
#endif
#endif