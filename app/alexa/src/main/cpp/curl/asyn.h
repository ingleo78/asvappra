#ifndef HEADER_CURL_ASYN_H
#define HEADER_CURL_ASYN_H

#include "curl_setup.h"
#include "curl_addrinfo.h"

struct addrinfo;
struct hostent;
struct Curl_easy;
struct connectdata;
struct Curl_dns_entry;
int Curl_resolver_global_init(void);
void Curl_resolver_global_cleanup(void);
CURLcode Curl_resolver_init(struct Curl_easy *easy, void **resolver);
void Curl_resolver_cleanup(void *resolver);
CURLcode Curl_resolver_duphandle(struct Curl_easy *easy, void **to, void *from);
void Curl_resolver_cancel(struct connectdata *conn);
void Curl_resolver_kill(struct connectdata *conn);
int Curl_resolver_getsock(struct connectdata *conn, curl_socket_t *sock);
CURLcode Curl_resolver_is_resolved(struct connectdata *conn, struct Curl_dns_entry **dns);
CURLcode Curl_resolver_wait_resolv(struct connectdata *conn, struct Curl_dns_entry **dnsentry);
struct Curl_addrinfo *Curl_resolver_getaddrinfo(struct connectdata *conn, const char *hostname, int port, int *waitp);
#ifndef CURLRES_ASYNCH
#define Curl_resolver_cancel(x) Curl_nop_stmt
#define Curl_resolver_kill(x) Curl_nop_stmt
#define Curl_resolver_is_resolved(x,y) CURLE_COULDNT_RESOLVE_HOST
#define Curl_resolver_wait_resolv(x,y) CURLE_COULDNT_RESOLVE_HOST
#define Curl_resolver_getsock(x,y,z) 0
#define Curl_resolver_duphandle(x,y,z) CURLE_OK
#define Curl_resolver_init(x,y) CURLE_OK
#define Curl_resolver_global_init() CURLE_OK
#define Curl_resolver_global_cleanup() Curl_nop_stmt
#define Curl_resolver_cleanup(x) Curl_nop_stmt
#else
int Curl_resolver_global_init(void);
void Curl_resolver_global_cleanup(void);
CURLcode Curl_resolver_init(struct Curl_easy *easy, void **resolver);
void Curl_resolver_cleanup(void *resolver);
void Curl_resolver_cancel(struct connectdata *conn);
int Curl_resolver_getsock(struct connectdata *conn, curl_socket_t *socks);
CURLcode Curl_resolver_is_resolved(struct connectdata *conn, struct Curl_dns_entry **dns);
CURLcode Curl_resolver_wait_resolv(struct connectdata *conn, struct Curl_dns_entry **entry);
struct Curl_addrinfo *Curl_resolver_getaddrinfo(struct connectdata *conn, const char *hostname, int port, int *waitp);
CURLcode Curl_set_dns_servers(struct Curl_easy *data, char *servers);
CURLcode Curl_set_dns_interface(struct Curl_easy *data, const char *interf);
CURLcode Curl_set_dns_local_ip4(struct Curl_easy *data, const char *local_ip4);
CURLcode Curl_set_dns_local_ip6(struct Curl_easy *data, const char *local_ip6);
#endif
#ifdef CURLRES_ASYNCH
#define Curl_resolver_asynch() 1
#else
#define Curl_resolver_asynch() 0
#endif
#endif