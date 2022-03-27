#include "curl_setup.h"

#ifdef CURLRES_SYNCH
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
#include "curl_memory.h"
#include "memdebug.h"

CURLcode Curl_set_dns_servers(struct Curl_easy *data, char *servers) {
    (void)data;
    (void)servers;
    return CURLE_NOT_BUILT_IN;
}
CURLcode Curl_set_dns_interface(struct Curl_easy *data, const char *interf) {
    (void)data;
    (void)interf;
    return CURLE_NOT_BUILT_IN;
}
CURLcode Curl_set_dns_local_ip4(struct Curl_easy *data, const char *local_ip4) {
    (void)data;
    (void)local_ip4;
    return CURLE_NOT_BUILT_IN;
}
CURLcode Curl_set_dns_local_ip6(struct Curl_easy *data, const char *local_ip6) {
    (void)data;
    (void)local_ip6;
    return CURLE_NOT_BUILT_IN;
}
#endif