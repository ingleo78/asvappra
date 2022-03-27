#ifndef HEADER_CURL_HOSTIP_H
#define HEADER_CURL_HOSTIP_H

#include "curl_setup.h"
#include "hash.h"
#include "curl_addrinfo.h"
#include "timeval.h"
#include "asyn.h"
#ifdef HAVE_SETJMP_H
#include <setjmp.h>
#endif
#ifdef NETWARE
#undef in_addr_t
#define in_addr_t unsigned long
#endif

#define CURL_HOSTENT_SIZE 9000
#define CURL_TIMEOUT_RESOLVE 300
#define CURL_ASYNC_SUCCESS CURLE_OK

struct addrinfo;
struct hostent;
struct Curl_easy;
struct connectdata;
struct curl_hash *Curl_global_host_cache_init(void);
struct Curl_dns_entry {
    struct Curl_addrinfo *addr;
    time_t timestamp;
    long inuse;
};
enum resolve_t {
    CURLRESOLV_TIMEDOUT = -2,
    CURLRESOLV_ERROR    = -1,
    CURLRESOLV_RESOLVED =  0,
    CURLRESOLV_PENDING  =  1
};
enum resolve_t Curl_resolv(struct connectdata *conn, const char *hostname, int port, bool allowDOH, struct Curl_dns_entry **dnsentry);
enum resolve_t Curl_resolv_timeout(struct connectdata *conn, const char *hostname, int port, struct Curl_dns_entry **dnsentry, timediff_t timeoutms);
#ifdef CURLRES_IPV6
bool Curl_ipv6works(struct connectdata *conn);
#else
#define Curl_ipv6works(x) FALSE
#endif
bool Curl_ipvalid(struct connectdata *conn);
struct Curl_addrinfo *Curl_getaddrinfo(struct connectdata *conn, const char *hostname, int port, int *waitp);
void Curl_resolv_unlock(struct Curl_easy *data, struct Curl_dns_entry *dns);
int Curl_mk_dnscache(struct curl_hash *hash);
void Curl_hostcache_prune(struct Curl_easy *data);
int Curl_num_addresses(const struct Curl_addrinfo *addr);
#if defined(CURLDEBUG) && defined(HAVE_GETNAMEINFO)
int curl_dogetnameinfo(GETNAMEINFO_QUAL_ARG1 GETNAMEINFO_TYPE_ARG1 sa, GETNAMEINFO_TYPE_ARG2 salen, char *host, GETNAMEINFO_TYPE_ARG46 hostlen,
                       char *serv, GETNAMEINFO_TYPE_ARG46 servlen, GETNAMEINFO_TYPE_ARG7 flags, int line, const char *source);
#endif
struct Curl_addrinfo *Curl_ipv4_resolve_r(const char *hostname, int port);
CURLcode Curl_once_resolved(struct connectdata *conn, bool *protocol_connect);
CURLcode Curl_addrinfo_callback(struct connectdata *conn, int status, struct Curl_addrinfo *ai);
void Curl_printable_address(const struct Curl_addrinfo *ip, char *buf, size_t bufsize);
struct Curl_dns_entry* Curl_fetch_addr(struct connectdata *conn, const char *hostname, int port);
struct Curl_dns_entry* Curl_cache_addr(struct Curl_easy *data, struct Curl_addrinfo *addr, const char *hostname, int port);
#ifndef INADDR_NONE
#define CURL_INADDR_NONE (in_addr_t) ~0
#else
#define CURL_INADDR_NONE INADDR_NONE
#endif
#ifdef HAVE_SIGSETJMP
extern sigjmp_buf curl_jmpenv;
#endif
CURLcode Curl_set_dns_servers(struct Curl_easy *data, char *servers);
CURLcode Curl_set_dns_interface(struct Curl_easy *data, const char *interf);
CURLcode Curl_set_dns_local_ip4(struct Curl_easy *data, const char *local_ip4);
CURLcode Curl_set_dns_local_ip6(struct Curl_easy *data, const char *local_ip6);
void Curl_hostcache_clean(struct Curl_easy *data, struct curl_hash *hash);
CURLcode Curl_loadhostpairs(struct Curl_easy *data);
CURLcode Curl_resolv_check(struct connectdata *conn, struct Curl_dns_entry **dns);
int Curl_resolv_getsock(struct connectdata *conn, curl_socket_t *socks);

#endif