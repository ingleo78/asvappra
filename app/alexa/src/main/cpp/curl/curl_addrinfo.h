#ifndef HEADER_CURL_ADDRINFO_H
#define HEADER_CURL_ADDRINFO_H

#include "curl_setup.h"

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <netdb.h>
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef __VMS
#include <in.h>
#include <inet.h>
#include <stdlib.h>
#endif
struct Curl_addrinfo {
    int                   ai_flags;
    int                   ai_family;
    int                   ai_socktype;
    int                   ai_protocol;
    curl_socklen_t        ai_addrlen;
    char                 *ai_canonname;
    struct sockaddr      *ai_addr;
    struct Curl_addrinfo *ai_next;
};
void Curl_freeaddrinfo(struct Curl_addrinfo *cahead);
#ifdef HAVE_GETADDRINFO
int Curl_getaddrinfo_ex(const char *nodename, const char *servname, const struct addrinfo *hints, struct Curl_addrinfo **result);
#endif
struct Curl_addrinfo* Curl_he2ai(const struct hostent *he, int port);
struct Curl_addrinfo* Curl_ip2addr(int af, const void *inaddr, const char *hostname, int port);
struct Curl_addrinfo* Curl_str2addr(char *dotted, int port);
#ifdef HAVE_GETADDRINFO
int Curl_getaddrinfo_ex(const char *nodename, const char *servname, const struct addrinfo *hints, struct Curl_addrinfo **result) {
    const struct addrinfo *ai;
    struct addrinfo *aihead;
    struct Curl_addrinfo *cafirst = NULL;
    struct Curl_addrinfo *calast = NULL;
    struct Curl_addrinfo *ca;
    size_t ss_size;
    int error;
    *result = NULL;
    error = getaddrinfo(nodename, servname, hints, &aihead);
    if (error) return error;
    for (ai = aihead; ai != NULL; ai = ai->ai_next) {
        size_t namelen = ai->ai_canonname ? strlen(ai->ai_canonname) + 1 : 0;
        if (ai->ai_family == AF_INET) ss_size = sizeof(struct sockaddr_in);
#ifdef ENABLE_IPV6
            else if(ai->ai_family == AF_INET6) ss_size = sizeof(struct sockaddr_in6);
#endif
        else continue;
        if ((ai->ai_addr == NULL) || !(ai->ai_addrlen > 0)) continue;
        if ((size_t)ai->ai_addrlen < ss_size) continue;
        ca = malloc(sizeof(struct Curl_addrinfo) + ss_size + namelen);
        if (!ca) {
            error = EAI_MEMORY;
            break;
        }
        ca->ai_flags     = ai->ai_flags;
        ca->ai_family    = ai->ai_family;
        ca->ai_socktype  = ai->ai_socktype;
        ca->ai_protocol  = ai->ai_protocol;
        ca->ai_addrlen   = (curl_socklen_t)ss_size;
        ca->ai_addr      = NULL;
        ca->ai_canonname = NULL;
        ca->ai_next      = NULL;
        ca->ai_addr = (void *)((char *)ca + sizeof(struct Curl_addrinfo));
        memcpy(ca->ai_addr, ai->ai_addr, ss_size);
        if (namelen) {
            ca->ai_canonname = (void *)((char *)ca->ai_addr + ss_size);
            memcpy(ca->ai_canonname, ai->ai_canonname, namelen);
        }
        if (!cafirst) cafirst = ca;
        if (calast) calast->ai_next = ca;
        calast = ca;
    }
    if(aihead) freeaddrinfo(aihead);
    if(error) {
        Curl_freeaddrinfo(cafirst);
        cafirst = NULL;
    } else if(!cafirst) {
#ifdef EAI_NONAME
        error = EAI_NONAME;
#else
        error = EAI_NODATA;
#endif
#ifdef USE_WINSOCK
        SET_SOCKERRNO(error);
#endif
    }
    *result = cafirst;
    return error;
}
#endif
#ifdef USE_UNIX_SOCKETS
struct Curl_addrinfo *Curl_unix2addr(const char *path, bool *longpath, bool abstract);
#endif
#if defined(CURLDEBUG) && defined(HAVE_GETADDRINFO) && defined(HAVE_FREEADDRINFO)
void curl_dbg_freeaddrinfo(struct addrinfo *freethis, int line, const char *source);
#endif
#if defined(CURLDEBUG) && defined(HAVE_GETADDRINFO)
int curl_dbg_getaddrinfo(const char *hostname, const char *service, const struct addrinfo *hints, struct addrinfo **result, int line, const char *source);
#endif
#ifdef HAVE_GETADDRINFO
#ifdef USE_RESOLVE_ON_IPS
void Curl_addrinfo_set_port(struct Curl_addrinfo *addrinfo, int port);
#else
#define Curl_addrinfo_set_port(x,y)
#endif
#endif
#endif