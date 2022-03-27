#include <netdb.h>
#include <stdbool.h>
#include "curl_setup.h"

#ifdef CURLRES_IPV4
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
//#include <process.h>
#endif
#include "urldata.h"
#include "sendf.h"
#include "hostip.h"
#include "hash.h"
#include "share.h"
#include "strerror.h"
#include "url.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

bool Curl_ipvalid(struct connectdata *conn) {
    if(conn->ip_version == CURL_IPRESOLVE_V6) return FALSE;
    return TRUE;
}
#ifdef CURLRES_SYNCH
struct Curl_addrinfo *Curl_getaddrinfo(struct connectdata *conn, const char *hostname, int port, int *waitp) {
    struct Curl_addrinfo *ai = NULL;
#ifdef CURL_DISABLE_VERBOSE_STRINGS
    (void)conn;
#endif
    *waitp = 0;
    ai = Curl_ipv4_resolve_r(hostname, port);
    if (!ai) infof(conn->data, "Curl_ipv4_resolve_r failed for %s\n", hostname);
    return ai;
}
#endif
#endif
#if defined(CURLRES_IPV4) && !defined(CURLRES_ARES)
struct Curl_addrinfo *Curl_ipv4_resolve_r(const char *hostname, int port) {
#if !defined(HAVE_GETADDRINFO_THREADSAFE) && defined(HAVE_GETHOSTBYNAME_R_3)
    int res;
#endif
    struct Curl_addrinfo *ai = NULL;
    struct hostent *h = NULL;
    struct hostent *buf = NULL;
#if defined(HAVE_GETADDRINFO_THREADSAFE)
    struct addrinfo hints;
    char sbuf[12];
    char *sbufptr = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (port) {
        msnprintf(sbuf, sizeof(sbuf), "%d", port);
        sbufptr = sbuf;
    }
    (void)Curl_getaddrinfo_ex(hostname, sbufptr, &hints, &ai);
#elif defined(HAVE_GETHOSTBYNAME_R)
    int h_errnop;
    buf = calloc(1, CURL_HOSTENT_SIZE);
    if (!buf) return NULL;
#if defined(HAVE_GETHOSTBYNAME_R_5)
    h = gethostbyname_r(hostname, (struct hostent*)buf, (char*)buf + sizeof(struct hostent), CURL_HOSTENT_SIZE - sizeof(struct hostent), &h_errnop);
    if (h);
    else
#elif defined(HAVE_GETHOSTBYNAME_R_6)
    (void)gethostbyname_r(hostname, (struct hostent *)buf, (char*)buf + sizeof(struct hostent), CURL_HOSTENT_SIZE - sizeof(struct hostent), &h, &h_errnop);
    if (!h)
#elif defined(HAVE_GETHOSTBYNAME_R_3)
    if (CURL_HOSTENT_SIZE >= (sizeof(struct hostent) + sizeof(struct hostent_data))) {
        res = gethostbyname_r(hostname, (struct hostent*)buf, (struct hostent_data*)((char*)buf + sizeof(struct hostent)));
        h_errnop = SOCKERRNO;
    } else res = -1;
    if (!res) h = buf;
    else
#endif
    {
        h = NULL;
        free(buf);
    }
#else
    h = gethostbyname((void *)hostname);
#endif
    if (h) {
        ai = Curl_he2ai(h, port);
        if(buf) free(buf);
    }
    return ai;
}
#endif