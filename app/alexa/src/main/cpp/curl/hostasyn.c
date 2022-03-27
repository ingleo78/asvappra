#include "curl_setup.h"

#ifdef CURLRES_ASYNCH
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
#include "urldata.h"
#include "sendf.h"
#include "hostip.h"
#include "hash.h"
#include "share.h"
#include "strerror.h"
#include "url.h"
#include "curl_memory.h"
#include "memdebug.h"

CURLcode Curl_addrinfo_callback(struct connectdata *conn, int status, struct Curl_addrinfo *ai) {
    struct Curl_dns_entry *dns = NULL;
    CURLcode result = CURLE_OK;
    conn->async.status = status;
    if (CURL_ASYNC_SUCCESS == status) {
        if (ai) {
            struct Curl_easy *data = conn->data;
            if (data->share) Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);
            dns = Curl_cache_addr(data, ai, conn->async.hostname, conn->async.port);
            if(data->share) Curl_share_unlock(data, CURL_LOCK_DATA_DNS);
            if (!dns) {
                Curl_freeaddrinfo(ai);
                result = CURLE_OUT_OF_MEMORY;
            }
        } else result = CURLE_OUT_OF_MEMORY;
    }
    conn->async.dns = dns;
    conn->async.done = TRUE;
    return result;
}
struct Curl_addrinfo *Curl_getaddrinfo(struct connectdata *conn, const char *hostname, int port, int *waitp) {
    return Curl_resolver_getaddrinfo(conn, hostname, port, waitp);
}
#endif