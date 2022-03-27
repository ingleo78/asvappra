#ifndef HEADER_CURL_DOH_H
#define HEADER_CURL_DOH_H

#include "urldata.h"
#include "curl_addrinfo.h"

#ifndef CURL_DISABLE_DOH
struct Curl_addrinfo *Curl_doh(struct connectdata *conn, const char *hostname, int port, int *waitp);
CURLcode Curl_doh_is_resolved(struct connectdata *conn, struct Curl_dns_entry **dns);
int Curl_doh_getsock(struct connectdata *conn, curl_socket_t *socks);
typedef enum {
    DOH_OK,
    DOH_DNS_BAD_LABEL,
    DOH_DNS_OUT_OF_RANGE,
    DOH_DNS_LABEL_LOOP,
    DOH_TOO_SMALL_BUFFER,
    DOH_OUT_OF_MEM,
    DOH_DNS_RDATA_LEN,
    DOH_DNS_MALFORMAT,
    DOH_DNS_BAD_RCODE,
    DOH_DNS_UNEXPECTED_TYPE,
    DOH_DNS_UNEXPECTED_CLASS,
    DOH_NO_CONTENT,
    DOH_DNS_BAD_ID,
    DOH_DNS_NAME_TOO_LONG
} DOHcode;
typedef enum {
    DNS_TYPE_A = 1,
    DNS_TYPE_NS = 2,
    DNS_TYPE_CNAME = 5,
    DNS_TYPE_AAAA = 28,
    DNS_TYPE_DNAME = 39
} DNStype;
#define DOH_MAX_ADDR 24
#define DOH_MAX_CNAME 4
struct dohaddr {
    int type;
    union {
        unsigned char v4[4];
        unsigned char v6[16];
    } ip;
};
struct dohentry {
    struct dynbuf cname[DOH_MAX_CNAME];
    struct dohaddr addr[DOH_MAX_ADDR];
    int numaddr;
    unsigned int ttl;
    int numcname;
};
#ifdef DEBUGBUILD
DOHcode doh_encode(const char *host, DNStype dnstype, unsigned char *dnsp, size_t len, size_t *olen);
DOHcode doh_decode(const unsigned char *doh, size_t dohlen, DNStype dnstype, struct dohentry *d);
void de_init(struct dohentry *d);
void de_cleanup(struct dohentry *d);
#endif
#else
#define Curl_doh(a,b,c,d) NULL
#define Curl_doh_is_resolved(x,y) CURLE_COULDNT_RESOLVE_HOST
#endif
#endif