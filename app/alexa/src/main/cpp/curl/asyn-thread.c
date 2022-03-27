#include "curl_setup.h"
#include "socketpair.h"

#ifdef CURLRES_THREADED
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
#if defined(USE_THREADS_POSIX)
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#elif defined(USE_THREADS_WIN32)
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif
#endif
#if (defined(NETWARE) && defined(__NOVELL_LIBC__))
#undef in_addr_t
#define in_addr_t unsigned long
#endif
#ifdef HAVE_GETADDRINFO
#define RESOLVER_ENOMEM  EAI_MEMORY
#else
#define RESOLVER_ENOMEM  ENOMEM
#endif
#include "urldata.h"
#include "sendf.h"
#include "hostip.h"
#include "hash.h"
#include "share.h"
#include "strerror.h"
#include "url.h"
#include "multiif.h"
#include "inet_ntop.h"
#include "curl_threads.h"
#include "connect.h"
#include "socketpair.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
struct resdata {
    struct curltime start;
};
int Curl_resolver_global_init(void) { return CURLE_OK; }
void Curl_resolver_global_cleanup(void) { }
CURLcode Curl_resolver_init(struct Curl_easy *easy, void **resolver) {
    (void)easy;
    *resolver = calloc(1, sizeof(struct resdata));
    if (!*resolver) return CURLE_OUT_OF_MEMORY;
    return CURLE_OK;
}
void Curl_resolver_cleanup(void *resolver) { free(resolver); }
CURLcode Curl_resolver_duphandle(struct Curl_easy *easy, void **to, void *from) {
    (void)from;
    return Curl_resolver_init(easy, to);
}
static void destroy_async_data(struct Curl_async *);
void Curl_resolver_cancel(struct connectdata *conn) { destroy_async_data(&conn->async); }
static bool init_resolve_thread(struct connectdata *conn, const char *hostname, int port, const struct addrinfo *hints);
struct thread_sync_data {
    curl_mutex_t *mtx;
    int done;
    char *hostname;
    int port;
#ifdef USE_SOCKETPAIR
    struct connectdata *conn;
    curl_socket_t sock_pair[2];
#endif
    int sock_error;
    struct Curl_addrinfo *res;
#ifdef HAVE_GETADDRINFO
    struct addrinfo hints;
#endif
    struct thread_data *td;
};
struct thread_data {
    curl_thread_t thread_hnd;
    unsigned int poll_interval;
    timediff_t interval_end;
    struct thread_sync_data tsd;
};
static struct thread_sync_data *conn_thread_sync_data(struct connectdata *conn) {
    return &(((struct thread_data *)conn->async.os_specific)->tsd);
}
static void destroy_thread_sync_data(struct thread_sync_data *tsd) {
    if(tsd->mtx) {
        Curl_mutex_destroy(tsd->mtx);
        free(tsd->mtx);
    }
    free(tsd->hostname);
    if(tsd->res) Curl_freeaddrinfo(tsd->res);
#ifdef USE_SOCKETPAIR
    if(tsd->sock_pair[1] != CURL_SOCKET_BAD) sclose(tsd->sock_pair[1]);
#endif
    memset(tsd, 0, sizeof(*tsd));
}
static int init_thread_sync_data(struct thread_data *td, const char *hostname, int port, const struct addrinfo *hints) {
    struct thread_sync_data *tsd = &td->tsd;
    memset(tsd, 0, sizeof(*tsd));
    tsd->td = td;
    tsd->port = port;
    tsd->done = 1;
#ifdef HAVE_GETADDRINFO
    DEBUGASSERT(hints);
    tsd->hints = *hints;
#else
    (void) hints;
#endif
    tsd->mtx = malloc(sizeof(curl_mutex_t));
    if (tsd->mtx == NULL) goto err_exit;
    Curl_mutex_init(tsd->mtx);
#ifdef USE_SOCKETPAIR
    if(Curl_socketpair(AF_UNIX, SOCK_STREAM, 0, &tsd->sock_pair[0]) < 0) {
        tsd->sock_pair[0] = CURL_SOCKET_BAD;
        tsd->sock_pair[1] = CURL_SOCKET_BAD;
        goto err_exit;
    }
#endif
    tsd->sock_error = CURL_ASYNC_SUCCESS;
    tsd->hostname = strdup(hostname);
    if(!tsd->hostname) goto err_exit;
    return 1;
    err_exit:
    destroy_thread_sync_data(tsd);
    return 0;
}
static int getaddrinfo_complete(struct connectdata *conn) {
    struct thread_sync_data *tsd = conn_thread_sync_data(conn);
    int rc;
    rc = Curl_addrinfo_callback(conn, tsd->sock_error, tsd->res);
    tsd->res = NULL;
    return rc;
}
#ifdef HAVE_GETADDRINFO
static unsigned int CURL_STDCALL getaddrinfo_thread(void *arg) {
    struct thread_sync_data *tsd = (struct thread_sync_data*)arg;
    struct thread_data *td = tsd->td;
    char service[12];
    int rc;
#ifdef USE_SOCKETPAIR
    char buf[1];
#endif
    msnprintf(service, sizeof(service), "%d", tsd->port);
    rc = Curl_getaddrinfo_ex(tsd->hostname, service, &tsd->hints, &tsd->res);
    if (rc != 0) {
        tsd->sock_error = SOCKERRNO?SOCKERRNO:rc;
        if (tsd->sock_error == 0) tsd->sock_error = RESOLVER_ENOMEM;
    } else Curl_addrinfo_set_port(tsd->res, tsd->port);
    Curl_mutex_acquire(tsd->mtx);
    if(tsd->done) {
        Curl_mutex_release(tsd->mtx);
        destroy_thread_sync_data(tsd);
        free(td);
    } else {
    #ifdef USE_SOCKETPAIR
        if(tsd->sock_pair[1] != CURL_SOCKET_BAD) {
            buf[0] = 1;
            if (swrite(tsd->sock_pair[1],  buf, sizeof(buf)) < 0) tsd->sock_error = SOCKERRNO;
        }
    #endif
        tsd->done = 1;
        Curl_mutex_release(tsd->mtx);
    }
    return 0;
}
#else
static unsigned int CURL_STDCALL gethostbyname_thread(void *arg) {
    struct thread_sync_data *tsd = (struct thread_sync_data *)arg;
    struct thread_data *td = tsd->td;
    tsd->res = Curl_ipv4_resolve_r(tsd->hostname, tsd->port);
    if(!tsd->res) {
        tsd->sock_error = SOCKERRNO;
        if (tsd->sock_error == 0) tsd->sock_error = RESOLVER_ENOMEM;
    }
    Curl_mutex_acquire(tsd->mtx);
    if (tsd->done) {
        Curl_mutex_release(tsd->mtx);
        destroy_thread_sync_data(tsd);
        free(td);
    } else {
        tsd->done = 1;
        Curl_mutex_release(tsd->mtx);
    }
    return 0;
}
#endif
static void destroy_async_data(struct Curl_async *async) {
    if (async->os_specific) {
        struct thread_data *td = (struct thread_data*) async->os_specific;
        int done;
    #ifdef USE_SOCKETPAIR
        curl_socket_t sock_rd = td->tsd.sock_pair[0];
        struct connectdata *conn = td->tsd.conn;
    #endif
        Curl_mutex_acquire(td->tsd.mtx);
        done = td->tsd.done;
        td->tsd.done = 1;
        Curl_mutex_release(td->tsd.mtx);
        if (!done) Curl_thread_destroy(td->thread_hnd);
        else {
            if (td->thread_hnd != curl_thread_t_null) Curl_thread_join(&td->thread_hnd);
            destroy_thread_sync_data(&td->tsd);
            free(async->os_specific);
        }
    #ifdef USE_SOCKETPAIR
        if (conn) Curl_multi_closed(conn->data, sock_rd);
        sclose(sock_rd);
    #endif
    }
    async->os_specific = NULL;
    free(async->hostname);
    async->hostname = NULL;
}
static bool init_resolve_thread(struct connectdata *conn, const char *hostname, int port, const struct addrinfo *hints) {
    struct thread_data *td = calloc(1, sizeof(struct thread_data));
    int err = ENOMEM;
    conn->async.os_specific = (void *)td;
    if (!td) goto errno_exit;
    conn->async.port = port;
    conn->async.done = FALSE;
    conn->async.status = 0;
    conn->async.dns = NULL;
    td->thread_hnd = curl_thread_t_null;
    if(!init_thread_sync_data(td, hostname, port, hints)) {
        conn->async.os_specific = NULL;
        free(td);
        goto errno_exit;
    }
    free(conn->async.hostname);
    conn->async.hostname = strdup(hostname);
    if (!conn->async.hostname) goto err_exit;
    td->tsd.done = 0;
#ifdef HAVE_GETADDRINFO
    td->thread_hnd = Curl_thread_create(getaddrinfo_thread, &td->tsd);
#else
    td->thread_hnd = Curl_thread_create(gethostbyname_thread, &td->tsd);
#endif
    if(!td->thread_hnd) {
        td->tsd.done = 1;
        err = errno;
        goto err_exit;
    }
    return TRUE;
    err_exit:
    destroy_async_data(&conn->async);
    errno_exit:
    errno = err;
    return FALSE;
}
static CURLcode resolver_error(struct connectdata *conn) {
    const char *host_or_proxy;
    CURLcode result;
#ifndef CURL_DISABLE_PROXY
    if(conn->bits.httpproxy) {
        host_or_proxy = "proxy";
        result = CURLE_COULDNT_RESOLVE_PROXY;
    } else {
        host_or_proxy = "host";
        result = CURLE_COULDNT_RESOLVE_HOST;
    }
#endif
    failf(conn->data, "Could not resolve %s: %s", host_or_proxy, conn->async.hostname);
    return result;
}
static CURLcode thread_wait_resolv(struct connectdata *conn, struct Curl_dns_entry **entry, bool report) {
    struct thread_data   *td = (struct thread_data*) conn->async.os_specific;
    CURLcode result = CURLE_OK;
    DEBUGASSERT(conn && td);
    DEBUGASSERT(td->thread_hnd != curl_thread_t_null);
    if (Curl_thread_join(&td->thread_hnd)) if(entry) result = getaddrinfo_complete(conn);
    else DEBUGASSERT(0);
    conn->async.done = TRUE;
    if (entry) *entry = conn->async.dns;
    if (!conn->async.dns && report) result = resolver_error(conn);
    destroy_async_data(&conn->async);
    if (!conn->async.dns && report) connclose(conn, "asynch resolve failed");
    return result;
}
void Curl_resolver_kill(struct connectdata *conn) {
    struct thread_data *td = (struct thread_data*) conn->async.os_specific;
    if (td && td->thread_hnd != curl_thread_t_null) (void)thread_wait_resolv(conn, NULL, FALSE);
    else Curl_resolver_cancel(conn);
}
CURLcode Curl_resolver_wait_resolv(struct connectdata *conn, struct Curl_dns_entry **entry) { return thread_wait_resolv(conn, entry, TRUE); }
CURLcode Curl_resolver_is_resolved(struct connectdata *conn, struct Curl_dns_entry **entry) {
    struct Curl_easy *data = conn->data;
    struct thread_data   *td = (struct thread_data*) conn->async.os_specific;
    int done = 0;
    DEBUGASSERT(entry);
    *entry = NULL;
    if (!td) {
        DEBUGASSERT(td);
        return CURLE_COULDNT_RESOLVE_HOST;
    }
    Curl_mutex_acquire(td->tsd.mtx);
    done = td->tsd.done;
    Curl_mutex_release(td->tsd.mtx);
    if (done) {
        getaddrinfo_complete(conn);
        if (!conn->async.dns) {
            CURLcode result = resolver_error(conn);
            destroy_async_data(&conn->async);
            return result;
        }
        destroy_async_data(&conn->async);
        *entry = conn->async.dns;
    } else {
        timediff_t elapsed = Curl_timediff(Curl_now(), data->progress.t_startsingle);
        if (elapsed < 0) elapsed = 0;
        if (td->poll_interval == 0) td->poll_interval = 1;
        else if(elapsed >= td->interval_end) td->poll_interval *= 2;
        if(td->poll_interval > 250) td->poll_interval = 250;
        td->interval_end = elapsed + td->poll_interval;
        Curl_expire(conn->data, td->poll_interval, EXPIRE_ASYNC_NAME);
    }
    return CURLE_OK;
}
int Curl_resolver_getsock(struct connectdata *conn, curl_socket_t *socks) {
    int ret_val = 0;
    timediff_t milli;
    timediff_t ms;
    struct Curl_easy *data = conn->data;
    struct resdata *reslv = (struct resdata *)data->state.resolver;
#ifdef USE_SOCKETPAIR
    struct thread_data *td = (struct thread_data*)conn->async.os_specific;
#else
    (void)socks;
#endif
#ifdef USE_SOCKETPAIR
    if (td) {
        socks[0] = td->tsd.sock_pair[0];
        DEBUGASSERT(td->tsd.conn == conn || !td->tsd.conn);
        td->tsd.conn = conn;
        ret_val = GETSOCK_READSOCK(0);
    } else {
#endif
    ms = Curl_timediff(Curl_now(), reslv->start);
    if(ms < 3) milli = 0;
    else if(ms <= 50) milli = ms/3;
    else if(ms <= 250) milli = 50;
    else milli = 200;
    Curl_expire(data, milli, EXPIRE_ASYNC_NAME);
#ifdef USE_SOCKETPAIR
    }
#endif
    return ret_val;
}
#ifndef HAVE_GETADDRINFO
struct Curl_addrinfo *Curl_resolver_getaddrinfo(struct connectdata *conn, const char *hostname, int port, int *waitp) {
    struct Curl_easy *data = conn->data;
    struct resdata *reslv = (struct resdata *)data->state.resolver;
    *waitp = 0;
    reslv->start = Curl_now();
    if(init_resolve_thread(conn, hostname, port, NULL)) {
        *waitp = 1;
        return NULL;
    }
    failf(conn->data, "getaddrinfo() thread failed\n");
    return NULL;
}
#else
struct Curl_addrinfo *Curl_resolver_getaddrinfo(struct connectdata *conn, const char *hostname, int port, int *waitp) {
    struct addrinfo hints;
    int pf = PF_INET;
    struct Curl_easy *data = conn->data;
    struct resdata *reslv = (struct resdata *)data->state.resolver;
    *waitp = 0;
#ifdef CURLRES_IPV6
    switch(conn->ip_version) {
        case CURL_IPRESOLVE_V4: pf = PF_INET; break;
        case CURL_IPRESOLVE_V6: pf = PF_INET6; break;
        default: pf = PF_UNSPEC;
    }
    if ((pf != PF_INET) && !Curl_ipv6works(conn)) pf = PF_INET;
#endif
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = pf;
    hints.ai_socktype = (conn->transport == TRNSPRT_TCP) ? SOCK_STREAM : SOCK_DGRAM;
    reslv->start = Curl_now();
    if(init_resolve_thread(conn, hostname, port, &hints)) {
        *waitp = 1;
        return NULL;
    }
    failf(data, "getaddrinfo() thread failed to start\n");
    return NULL;
}
#endif
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