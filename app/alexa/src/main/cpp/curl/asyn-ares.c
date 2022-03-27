#include "curl_setup.h"

#ifdef CURLRES_ARES
#include <limits.h>
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
#ifndef HAVE_PROCESS_H
#include <process.h>
#endif
#if (defined(NETWARE) && defined(__NOVELL_LIBC__))
#undef in_addr_t
#define in_addr_t unsigned long
#endif
#include "urldata.h"
#include "sendf.h"
#include "hostip.h"
#include "hash.h"
#include "share.h"
#include "strerror.h"
#include "url.h"
#include "multiif.h"
#include "inet_pton.h"
#include "connect.h"
#include "select.h"
#include "progress.h"
#if defined(CURL_STATICLIB) && !defined(CARES_STATICLIB) && (defined(WIN32) || defined(__SYMBIAN32__))
#define CARES_STATICLIB
#endif
//#include <ares.h>
//#include <ares_version.h>
#if ARES_VERSION >= 0x010500
#define HAVE_CARES_CALLBACK_TIMEOUTS 1
#endif
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
struct ResolverResults {
    int num_pending;
    struct Curl_addrinfo *temp_ai;
    int last_status;
    struct curltime happy_eyeballs_dns_time;
};
#define HAPPY_EYEBALLS_DNS_TIMEOUT 5000
int Curl_resolver_global_init(void) {
    #ifdef CARES_HAVE_ARES_LIBRARY_INIT
    if (ares_library_init(ARES_LIB_INIT_ALL)) return CURLE_FAILED_INIT;
    #endif
    return CURLE_OK;
}
void Curl_resolver_global_cleanup(void) {
#ifdef CARES_HAVE_ARES_LIBRARY_CLEANUP
    ares_library_cleanup();
#endif
}
static void Curl_ares_sock_state_cb(void *data, ares_socket_t socket_fd, int readable, int writable) {
    struct Curl_easy *easy = data;
    if (!readable && !writable) {
        DEBUGASSERT(easy);
        Curl_multi_closed(easy, socket_fd);
    }
}
CURLcode Curl_resolver_init(struct Curl_easy *easy, void **resolver) {
    int status;
    struct ares_options options;
    int optmask = ARES_OPT_SOCK_STATE_CB;
    options.sock_state_cb = Curl_ares_sock_state_cb;
    options.sock_state_cb_data = easy;
    status = ares_init_options((ares_channel*)resolver, &options, optmask);
    if (status != ARES_SUCCESS) {
        if(status == ARES_ENOMEM) return CURLE_OUT_OF_MEMORY;
        else return CURLE_FAILED_INIT;
    }
    return CURLE_OK;
}
void Curl_resolver_cleanup(void *resolver) { ares_destroy((ares_channel)resolver); }
CURLcode Curl_resolver_duphandle(struct Curl_easy *easy, void **to, void *from) {
    (void)from;
    return Curl_resolver_init(easy, to);
}
static void destroy_async_data(struct Curl_async *async);
void Curl_resolver_cancel(struct connectdata *conn) {
    if(conn->data && conn->data->state.resolver) ares_cancel((ares_channel)conn->data->state.resolver);
    destroy_async_data(&conn->async);
}
void Curl_resolver_kill(struct connectdata *conn) { Curl_resolver_cancel(conn); }
static void destroy_async_data(struct Curl_async *async) {
    free(async->hostname);
    if (async->os_specific) {
        struct ResolverResults *res = (struct ResolverResults *)async->os_specific;
        if (res) {
            if (res->temp_ai) {
                Curl_freeaddrinfo(res->temp_ai);
                res->temp_ai = NULL;
            }
            free(res);
        }
        async->os_specific = NULL;
    }
    async->hostname = NULL;
}
int Curl_resolver_getsock(struct connectdata *conn, curl_socket_t *socks) {
    struct timeval maxtime;
    struct timeval timebuf;
    struct timeval *timeout;
    long milli;
    int max = ares_getsock((ares_channel)conn->data->state.resolver, (ares_socket_t *)socks, MAX_SOCKSPEREASYHANDLE);
    maxtime.tv_sec = CURL_TIMEOUT_RESOLVE;
    maxtime.tv_usec = 0;
    timeout = ares_timeout((ares_channel)conn->data->state.resolver, &maxtime, &timebuf);
    milli = (timeout->tv_sec * 1000) + (timeout->tv_usec/1000);
    if (milli == 0) milli += 10;
    Curl_expire(conn->data, milli, EXPIRE_ASYNC_NAME);
    return max;
}
static int waitperform(struct connectdata *conn, timediff_t timeout_ms) {
    struct Curl_easy *data = conn->data;
    int nfds;
    int bitmask;
    ares_socket_t socks[ARES_GETSOCK_MAXNUM];
    struct pollfd pfd[ARES_GETSOCK_MAXNUM];
    int i;
    int num = 0;
    bitmask = ares_getsock((ares_channel)data->state.resolver, socks, ARES_GETSOCK_MAXNUM);
    for(i = 0; i < ARES_GETSOCK_MAXNUM; i++) {
        pfd[i].events = 0;
        pfd[i].revents = 0;
        if (ARES_GETSOCK_READABLE(bitmask, i)) {
            pfd[i].fd = socks[i];
            pfd[i].events |= POLLRDNORM|POLLIN;
        }
        if (ARES_GETSOCK_WRITABLE(bitmask, i)) {
            pfd[i].fd = socks[i];
            pfd[i].events |= POLLWRNORM|POLLOUT;
        }
        if(pfd[i].events != 0) num++;
        else break;
    }
    if(num) nfds = Curl_poll(pfd, num, timeout_ms);
    else nfds = 0;
    if(!nfds) ares_process_fd((ares_channel)data->state.resolver, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
    else {
        for(i = 0; i < num; i++)
            ares_process_fd((ares_channel)data->state.resolver, (pfd[i].revents & (POLLRDNORM|POLLIN))? pfd[i].fd:ARES_SOCKET_BAD,
                            (pfd[i].revents & (POLLWRNORM|POLLOUT))? pfd[i].fd:ARES_SOCKET_BAD);
    }
    return nfds;
}
CURLcode Curl_resolver_is_resolved(struct connectdata *conn, struct Curl_dns_entry **dns) {
    struct Curl_easy *data = conn->data;
    struct ResolverResults *res = (struct ResolverResults *)conn->async.os_specific;
    CURLcode result = CURLE_OK;
    DEBUGASSERT(dns);
    *dns = NULL;
    waitperform(conn, 0);
    if (res && res->num_pending && (res->happy_eyeballs_dns_time.tv_sec || res->happy_eyeballs_dns_time.tv_usec) &&
        (Curl_timediff(Curl_now(), res->happy_eyeballs_dns_time) >= HAPPY_EYEBALLS_DNS_TIMEOUT)) {
        memset(&res->happy_eyeballs_dns_time, 0, sizeof(res->happy_eyeballs_dns_time))
        ares_cancel((ares_channel)data->state.resolver);
        DEBUGASSERT(res->num_pending == 0);
    }
    if(res && !res->num_pending) {
        (void)Curl_addrinfo_callback(conn, res->last_status, res->temp_ai);
        res->temp_ai = NULL;
        if (!conn->async.dns) {
            failf(data, "Could not resolve: %s (%s)", conn->async.hostname, ares_strerror(conn->async.status));
            result = conn->bits.proxy ? CURLE_COULDNT_RESOLVE_PROXY : CURLE_COULDNT_RESOLVE_HOST;
        } else *dns = conn->async.dns;
        destroy_async_data(&conn->async);
    }
    return result;
}
CURLcode Curl_resolver_wait_resolv(struct connectdata *conn, struct Curl_dns_entry **entry) {
    CURLcode result = CURLE_OK;
    struct Curl_easy *data = conn->data;
    timediff_t timeout;
    struct curltime now = Curl_now();
    DEBUGASSERT(entry);
    *entry = NULL;
    timeout = Curl_timeleft(data, &now, TRUE);
    if (timeout < 0) {
        connclose(conn, "Timed out before name resolve started");
        return CURLE_OPERATION_TIMEDOUT;
    }
    if (!timeout) timeout = CURL_TIMEOUT_RESOLVE * 1000;
    while(!result) {
        struct timeval *tvp, tv, store;
        int itimeout;
        timediff_t timeout_ms;
    #if TIMEDIFF_T_MAX > INT_MAX
        itimeout = (timeout > INT_MAX) ? INT_MAX : (int)timeout;
    #else
        itimeout = (int)timeout;
        #endif
        store.tv_sec = itimeout/1000;
        store.tv_usec = (itimeout%1000)*1000;
        tvp = ares_timeout((ares_channel)data->state.resolver, &store, &tv);
        if (!tvp->tv_sec) timeout_ms = (timediff_t)(tvp->tv_usec/1000);
        else timeout_ms = 1000;
        waitperform(conn, timeout_ms);
        result = Curl_resolver_is_resolved(conn, entry);
        if (result || conn->async.done) break;
        if (Curl_pgrsUpdate(conn)) result = CURLE_ABORTED_BY_CALLBACK;
        else {
            struct curltime now2 = Curl_now();
            timediff_t timediff = Curl_timediff(now2, now);
            if (timediff <= 0) timeout -= 1;
            else if (timediff > timeout) timeout = -1;
            else timeout -= timediff;
            now = now2;
        }
        if (timeout < 0) result = CURLE_OPERATION_TIMEDOUT;
    }
    if (result) ares_cancel((ares_channel)data->state.resolver);
    if (entry) *entry = conn->async.dns;
    if (result) connclose(conn, "c-ares resolve failed");
    return result;
}
static void compound_results(struct ResolverResults *res, struct Curl_addrinfo *ai) {
    struct Curl_addrinfo *ai_tail;
    if (!ai) return;
    ai_tail = ai;
    while(ai_tail->ai_next) ai_tail = ai_tail->ai_next;
    ai_tail->ai_next = res->temp_ai;
    res->temp_ai = ai;
}
static void query_completed_cb(void *arg, int status,
                                   #ifdef HAVE_CARES_CALLBACK_TIMEOUTS
                                       int timeouts,
                                   #endif
                                   struct hostent *hostent) {
    struct connectdata *conn = (struct connectdata *)arg;
    struct ResolverResults *res;
#ifdef HAVE_CARES_CALLBACK_TIMEOUTS
    (void)timeouts;
#endif
    if (ARES_EDESTRUCTION == status) return;
    res = (struct ResolverResults *)conn->async.os_specific;
    if (res) {
        res->num_pending--;
        if (CURL_ASYNC_SUCCESS == status) {
            struct Curl_addrinfo *ai = Curl_he2ai(hostent, conn->async.port);
            if (ai) compound_results(res, ai);
        }
        if (res->last_status != ARES_SUCCESS) res->last_status = status;
        if (res->num_pending && (status == ARES_SUCCESS || status == ARES_ENOTFOUND)) {
            DEBUGASSERT(res->num_pending == 1);
            res->happy_eyeballs_dns_time = Curl_now();
            Curl_expire(conn->data, HAPPY_EYEBALLS_DNS_TIMEOUT, EXPIRE_HAPPY_EYEBALLS_DNS);
        }
    }
}
struct Curl_addrinfo *Curl_resolver_getaddrinfo(struct connectdata *conn, const char *hostname, int port, int *waitp) {
    char *bufp;
    struct Curl_easy *data = conn->data;
    int family = PF_INET;
    *waitp = 0;
#ifdef ENABLE_IPV6
    switch(conn->ip_version) {
        case CURL_IPRESOLVE_V4: family = PF_INET; break;
        case CURL_IPRESOLVE_V6: family = PF_INET6; break;
        default:
        #if ARES_VERSION >= 0x010601
            family = PF_UNSPEC;
        #endif
    }
#endif
    bufp = strdup(hostname);
    if (bufp) {
        struct ResolverResults *res = NULL;
        free(conn->async.hostname);
        conn->async.hostname = bufp;
        conn->async.port = port;
        conn->async.done = FALSE;
        conn->async.status = 0;
        conn->async.dns = NULL;
        res = calloc(sizeof(struct ResolverResults), 1);
        if (!res) {
            free(conn->async.hostname);
            conn->async.hostname = NULL;
            return NULL;
        }
        conn->async.os_specific = res;
        res->last_status = ARES_ENOTFOUND;
    #ifdef ENABLE_IPV6
        if(family == PF_UNSPEC) {
            if(Curl_ipv6works(conn)) {
                res->num_pending = 2;
                ares_gethostbyname((ares_channel)data->state.resolver, hostname, PF_INET, query_completed_cb, conn);
                ares_gethostbyname((ares_channel)data->state.resolver, hostname, PF_INET6, query_completed_cb, conn);
            } else {
                res->num_pending = 1;
                ares_gethostbyname((ares_channel)data->state.resolver, hostname, PF_INET, query_completed_cb, conn);
            }
        } else {
            res->num_pending = 1;
            ares_gethostbyname((ares_channel)data->state.resolver, hostname, family, query_completed_cb, conn);
        }
        *waitp = 1;
    #endif
    }
    return NULL;
}
CURLcode Curl_set_dns_servers(struct Curl_easy *data, char *servers) {
    CURLcode result = CURLE_NOT_BUILT_IN;
    int ares_result;
    if (!(servers && servers[0])) return CURLE_OK;
#if (ARES_VERSION >= 0x010704)
#if (ARES_VERSION >= 0x010b00)
    ares_result = ares_set_servers_ports_csv(data->state.resolver, servers);
#else
    ares_result = ares_set_servers_csv(data->state.resolver, servers);
#endif
    switch(ares_result) {
        case ARES_SUCCESS: result = CURLE_OK; break;
        case ARES_ENOMEM: result = CURLE_OUT_OF_MEMORY; break;
        case ARES_ENOTINITIALIZED: case ARES_ENODATA: case ARES_EBADSTR: default:
            result = CURLE_BAD_FUNCTION_ARGUMENT;
            break;
    }
#else
    (void)data;
    (void)(ares_result);
#endif
    return result;
}
CURLcode Curl_set_dns_interface(struct Curl_easy *data, const char *interf) {
#if (ARES_VERSION >= 0x010704)
    if(!interf) interf = "";
    ares_set_local_dev((ares_channel)data->state.resolver, interf);
    return CURLE_OK;
#else
    (void)data;
    (void)interf;
    return CURLE_NOT_BUILT_IN;
#endif
}
CURLcode Curl_set_dns_local_ip4(struct Curl_easy *data, const char *local_ip4) {
#if (ARES_VERSION >= 0x010704)
    struct in_addr a4;
    if( (!local_ip4) || (local_ip4[0] == 0)) a4.s_addr = 0;
    else {
        if(Curl_inet_pton(AF_INET, local_ip4, &a4) != 1) return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    ares_set_local_ip4((ares_channel)data->state.resolver, ntohl(a4.s_addr));
    return CURLE_OK;
#else
    (void)data;
    (void)local_ip4;
    return CURLE_NOT_BUILT_IN;
#endif
}
CURLcode Curl_set_dns_local_ip6(struct Curl_easy *data, const char *local_ip6) {
#if (ARES_VERSION >= 0x010704) && defined(ENABLE_IPV6)
    unsigned char a6[INET6_ADDRSTRLEN];
    if ((!local_ip6) || (local_ip6[0] == 0)) memset(a6, 0, sizeof(a6));
    else if(Curl_inet_pton(AF_INET6, local_ip6, a6) != 1) return CURLE_BAD_FUNCTION_ARGUMENT;
    ares_set_local_ip6((ares_channel)data->state.resolver, a6);
    return CURLE_OK;
#else
    (void)data;
    (void)local_ip6;
    return CURLE_NOT_BUILT_IN;
#endif
}
#endif