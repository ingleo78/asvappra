#ifndef HEADER_CURL_IF2IP_H
#define HEADER_CURL_IF2IP_H

#include "curl_setup.h"

#define IPV6_SCOPE_GLOBAL       0
#define IPV6_SCOPE_LINKLOCAL    1
#define IPV6_SCOPE_SITELOCAL    2
#define IPV6_SCOPE_UNIQUELOCAL  3
#define IPV6_SCOPE_NODELOCAL    4

unsigned int Curl_ipv6_scope(const struct sockaddr *sa);
typedef enum {
    IF2IP_NOT_FOUND = 0,
    IF2IP_AF_NOT_SUPPORTED = 1,
    IF2IP_FOUND = 2
} if2ip_result_t;
if2ip_result_t Curl_if2ip(int af, unsigned int remote_scope, unsigned int local_scope_id, const char *interf, char *buf, int buf_size);
#ifdef __INTERIX
struct ifreq {
    #define IFNAMSIZ 16
    #define IFHWADDRLEN 6
    union {
        char ifrn_name[IFNAMSIZ];
    } ifr_ifrn;
   union {
       struct sockaddr ifru_addr;
       struct sockaddr ifru_broadaddr;
       struct sockaddr ifru_netmask;
       struct sockaddr ifru_hwaddr;
       short ifru_flags;
       int ifru_metric;
       int ifru_mtu;
   } ifr_ifru;
};
#define ifr_name ifr_ifrn.ifrn_name /* interface name */
#define ifr_addr ifr_ifru.ifru_addr /* address */
#define ifr_broadaddr ifr_ifru.ifru_broadaddr /* broadcast address */
#define ifr_netmask ifr_ifru.ifru_netmask /* interface net mask */
#define ifr_flags ifr_ifru.ifru_flags /* flags */
#define ifr_hwaddr ifr_ifru.ifru_hwaddr /* MAC address */
#define ifr_metric ifr_ifru.ifru_metric /* metric */
#define ifr_mtu ifr_ifru.ifru_mtu /* mtu */
#define SIOCGIFADDR _IOW('s', 102, struct ifreq) /* Get if addr */
#endif
#endif