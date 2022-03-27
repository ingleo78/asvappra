#include "curl_setup.h"
#include "curl.h"

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_IN6_H
#include <netinet/in6.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#ifdef __VMS
#include <in.h>
#include <inet.h>
#endif
#if defined(NETWARE) && defined(__NOVELL_LIBC__)
#undef  in_addr_t
#define in_addr_t unsigned long
#endif
#include <stddef.h>
#include "curl_addrinfo.h"
#include "inet_pton.h"
#include "warnless.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
#if defined(__INTEL_COMPILER) && (__INTEL_COMPILER == 910) && defined(__OPTIMIZE__) && defined(__unix__) &&  defined(__i386__)
#define vqualifier volatile
#else
#define vqualifier
#endif
void Curl_freeaddrinfo(struct Curl_addrinfo *cahead) {
    struct Curl_addrinfo *vqualifier canext;
    struct Curl_addrinfo *ca;
    for(ca = cahead; ca; ca = canext) {
        canext = ca->ai_next;
        free(ca);
    }
}
struct Curl_addrinfo* Curl_he2ai(const struct hostent *he, int port) {
    struct Curl_addrinfo *ai;
    struct Curl_addrinfo *prevai = NULL;
    struct Curl_addrinfo *firstai = NULL;
    struct sockaddr_in *addr;
#ifdef ENABLE_IPV6
    struct sockaddr_in6 *addr6;
#endif
    CURLcode result = CURLE_OK;
    int i;
    char *curr;
    if (!he) return NULL;
    DEBUGASSERT((he->h_name != NULL) && (he->h_addr_list != NULL));
    for(i = 0; (curr = he->h_addr_list[i]) != NULL; i++) {
        size_t ss_size;
        size_t namelen = strlen(he->h_name) + 1; /* include zero termination */
    #ifdef ENABLE_IPV6
        if(he->h_addrtype == AF_INET6) ss_size = sizeof(struct sockaddr_in6);
        else
    #endif
        ss_size = sizeof(struct sockaddr_in);
        ai = calloc(1, sizeof(struct Curl_addrinfo) + ss_size + namelen);
        if(!ai) {
            result = CURLE_OUT_OF_MEMORY;
            break;
        }
        ai->ai_addr = (void *)((char *)ai + sizeof(struct Curl_addrinfo));
        ai->ai_canonname = (char *)ai->ai_addr + ss_size;
        memcpy(ai->ai_canonname, he->h_name, namelen);
        if(!firstai) firstai = ai;
        if(prevai)  prevai->ai_next = ai;
        ai->ai_family = he->h_addrtype;
        ai->ai_socktype = SOCK_STREAM;
        ai->ai_addrlen = (curl_socklen_t)ss_size;
        switch(ai->ai_family) {
            case AF_INET:
                addr = (void *)ai->ai_addr;
                memcpy(&addr->sin_addr, curr, sizeof(struct in_addr));
                addr->sin_family = (CURL_SA_FAMILY_T)(he->h_addrtype);
                addr->sin_port = htons((unsigned short)port);
                break;
        #ifdef ENABLE_IPV6
            case AF_INET6:
                addr6 = (void *)ai->ai_addr;
                memcpy(&addr6->sin6_addr, curr, sizeof(struct in6_addr));
                addr6->sin6_family = (CURL_SA_FAMILY_T)(he->h_addrtype);
                addr6->sin6_port = htons((unsigned short)port);
                break;
        #endif
        }
        prevai = ai;
    }
    if(result) {
        Curl_freeaddrinfo(firstai);
        firstai = NULL;
    }
    return firstai;
}
struct namebuff {
    struct hostent hostentry;
    union {
        struct in_addr  ina4;
#ifdef ENABLE_IPV6
        struct in6_addr ina6;
#endif
    } addrentry;
    char *h_addr_list[2];
};
struct Curl_addrinfo* Curl_ip2addr(int af, const void *inaddr, const char *hostname, int port) {
    struct Curl_addrinfo *ai;
#if defined(__VMS) && defined(__INITIAL_POINTER_SIZE) && (__INITIAL_POINTER_SIZE == 64)
    #pragma pointer_size save
    #pragma pointer_size short
    #pragma message disable PTRMISMATCH
#endif
    struct hostent  *h;
    struct namebuff *buf;
    char  *addrentry;
    char  *hoststr;
    size_t addrsize;
    DEBUGASSERT(inaddr && hostname);
    buf = malloc(sizeof(struct namebuff));
    if (!buf) return NULL;
    hoststr = strdup(hostname);
    if(!hoststr) {
        free(buf);
        return NULL;
    }
    switch(af) {
        case AF_INET:
            addrsize = sizeof(struct in_addr);
            addrentry = (void *)&buf->addrentry.ina4;
            memcpy(addrentry, inaddr, sizeof(struct in_addr));
            break;
    #ifdef ENABLE_IPV6
        case AF_INET6:
            addrsize = sizeof(struct in6_addr);
            addrentry = (void *)&buf->addrentry.ina6;
            memcpy(addrentry, inaddr, sizeof(struct in6_addr));
            break;
    #endif
        default:
            free(hoststr);
            free(buf);
            return NULL;
    }
    h = &buf->hostentry;
    h->h_name = hoststr;
    h->h_aliases = NULL;
    h->h_addrtype = (short)af;
    h->h_length = (short)addrsize;
    h->h_addr_list = &buf->h_addr_list[0];
    h->h_addr_list[0] = addrentry;
    h->h_addr_list[1] = NULL;
#if defined(__VMS) && defined(__INITIAL_POINTER_SIZE) && (__INITIAL_POINTER_SIZE == 64)
    #pragma pointer_size restore
    #pragma message enable PTRMISMATCH
#endif
    ai = Curl_he2ai(h, port);
    free(hoststr);
    free(buf);
    return ai;
}
struct Curl_addrinfo *Curl_str2addr(char *address, int port) {
    struct in_addr in;
    if(Curl_inet_pton(AF_INET, address, &in) > 0) return Curl_ip2addr(AF_INET, &in, address, port);
#ifdef ENABLE_IPV6
    struct in6_addr in6;
    if(Curl_inet_pton(AF_INET6, address, &in6) > 0) return Curl_ip2addr(AF_INET6, &in6, address, port);
#endif
    return NULL;
}
#ifdef USE_UNIX_SOCKETS
struct Curl_addrinfo *Curl_unix2addr(const char *path, bool *longpath, bool abstract) {
    struct Curl_addrinfo *ai;
    struct sockaddr_un *sa_un;
    size_t path_len;
    *longpath = FALSE;
    ai = calloc(1, sizeof(struct Curl_addrinfo) + sizeof(struct sockaddr_un));
    if (!ai) return NULL;
    ai->ai_addr = (void *)((char *)ai + sizeof(struct Curl_addrinfo));
    sa_un = (void *) ai->ai_addr;
    sa_un->sun_family = AF_UNIX;
    path_len = strlen(path) + 1;
    if(path_len > sizeof(sa_un->sun_path)) {
        free(ai);
        *longpath = TRUE;
        return NULL;
    }
    ai->ai_family = AF_UNIX;
    ai->ai_socktype = SOCK_STREAM;
    ai->ai_addrlen = (curl_socklen_t)((offsetof(struct sockaddr_un, sun_path) + path_len) & 0x7FFFFFFF);
    if(abstract) memcpy(sa_un->sun_path + 1, path, path_len - 1);
    else memcpy(sa_un->sun_path, path, path_len); /* copy NUL byte */
    return ai;
}
#endif
#if defined(CURLDEBUG) && defined(HAVE_GETADDRINFO) &&  defined(HAVE_FREEADDRINFO)
void curl_dbg_freeaddrinfo(struct addrinfo *freethis, int line, const char *source) {
    curl_dbg_log("ADDR %s:%d freeaddrinfo(%p)\n", source, line, (void *)freethis);
#ifdef USE_LWIPSOCK
    lwip_freeaddrinfo(freethis);
#else
    (freeaddrinfo)(freethis);
#endif
}
#endif
#if defined(CURLDEBUG) && defined(HAVE_GETADDRINFO)
int curl_dbg_getaddrinfo(const char *hostname, const char *service, const struct addrinfo *hints, struct addrinfo **result, int line, const char *source) {
#ifdef USE_LWIPSOCK
    int res = lwip_getaddrinfo(hostname, service, hints, result);
#else
    int res = (getaddrinfo)(hostname, service, hints, result);
#endif
    if(0 == res) curl_dbg_log("ADDR %s:%d getaddrinfo() = %p\n", source, line, (void *)*result);
    else curl_dbg_log("ADDR %s:%d getaddrinfo() failed\n", source, line);
    return res;
}
#endif
#if defined(HAVE_GETADDRINFO) && defined(USE_RESOLVE_ON_IPS)
void Curl_addrinfo_set_port(struct Curl_addrinfo *addrinfo, int port) {
    struct Curl_addrinfo *ca;
    struct sockaddr_in *addr;
#ifdef ENABLE_IPV6
    struct sockaddr_in6 *addr6;
#endif
    for(ca = addrinfo; ca != NULL; ca = ca->ai_next) {
        switch(ca->ai_family) {
            case AF_INET:
                addr = (void *)ca->ai_addr; /* storage area for this info */
                addr->sin_port = htons((unsigned short)port);
                break;
        #ifdef ENABLE_IPV6
            case AF_INET6:
                addr6 = (void *)ca->ai_addr; /* storage area for this info */
                addr6->sin6_port = htons((unsigned short)port);
                break;
        #endif
            }
        }
    }
#endif