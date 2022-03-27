#include <assert.h>
#include <stdbool.h>
#include "curl_setup.h"

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_IN6_H
#include <netinet/in6.h>
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
#ifdef HAVE_SETJMP_H
#include <setjmp.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_PROCESS_H
//#include <process.h>
#endif
#include "urldata.h"
#include "sendf.h"
#include "hostip.h"
#include "hash.h"
#include "rand.h"
#include "share.h"
#include "strerror.h"
#include "url.h"
#include "inet_ntop.h"
#include "inet_pton.h"
#include "multiif.h"
#include "doh.h"
#include "warnless.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

#if defined(CURLRES_SYNCH) && defined(HAVE_ALARM) && defined(SIGALRM) && defined(HAVE_SIGSETJMP)
#define USE_ALARM_TIMEOUT
#endif
#define MAX_HOSTCACHE_LEN (255 + 7)
static void freednsentry(void *freethis);
int Curl_num_addresses(const struct Curl_addrinfo *addr) {
    int i = 0;
    while(addr) {
        addr = addr->ai_next;
        i++;
    }
    return i;
}
void Curl_printable_address(const struct Curl_addrinfo *ai, char *buf, size_t bufsize) {
    DEBUGASSERT(bufsize);
    buf[0] = 0;
    switch(ai->ai_family) {
        case AF_INET: {
                const struct sockaddr_in *sa4 = (const void *)ai->ai_addr;
                const struct in_addr *ipaddr4 = &sa4->sin_addr;
                (void)Curl_inet_ntop(ai->ai_family, (const void *)ipaddr4, buf, bufsize);
                break;
            }
    #ifdef ENABLE_IPV6
        case AF_INET6: {
                const struct sockaddr_in6 *sa6 = (const void *)ai->ai_addr;
                const struct in6_addr *ipaddr6 = &sa6->sin6_addr;
                (void)Curl_inet_ntop(ai->ai_family, (const void *)ipaddr6, buf, bufsize);
                break;
            }
    #endif
    }
}
static void create_hostcache_id(const char *name, int port, char *ptr, size_t buflen) {
    size_t len = strlen(name);
    if (len > (buflen - 7)) len = buflen - 7;
    while(len--) *ptr++ = (char)TOLOWER(*name++);
    msnprintf(ptr, 7, ":%u", port);
}
struct hostcache_prune_data {
    long cache_timeout;
    time_t now;
};
static int hostcache_timestamp_remove(void *datap, void *hc) {
    struct hostcache_prune_data *data = (struct hostcache_prune_data *) datap;
    struct Curl_dns_entry *c = (struct Curl_dns_entry *) hc;
    return (0 != c->timestamp) && (data->now - c->timestamp >= data->cache_timeout);
}
static void hostcache_prune(struct curl_hash *hostcache, long cache_timeout, time_t now) {
    struct hostcache_prune_data user;
    user.cache_timeout = cache_timeout;
    user.now = now;
    Curl_hash_clean_with_criterium(hostcache, (void*)&user, hostcache_timestamp_remove);
}
void Curl_hostcache_prune(struct Curl_easy *data) {
    time_t now;
    if ((data->set.dns_cache_timeout == -1) || !data->dns.hostcache) return;
    if (data->share) Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);
    time(&now);
    hostcache_prune(data->dns.hostcache, data->set.dns_cache_timeout, now);
    if (data->share) Curl_share_unlock(data, CURL_LOCK_DATA_DNS);
}
#ifdef HAVE_SIGSETJMP
sigjmp_buf curl_jmpenv;
#endif
static struct Curl_dns_entry* fetch_addr(struct connectdata *conn, const char *hostname, int port) {
    struct Curl_dns_entry *dns = NULL;
    size_t entry_len;
    struct Curl_easy *data = conn->data;
    char entry_id[MAX_HOSTCACHE_LEN];
    create_hostcache_id(hostname, port, entry_id, sizeof(entry_id));
    entry_len = strlen(entry_id);
    dns = Curl_hash_pick(data->dns.hostcache, entry_id, entry_len + 1);
    if(!dns && data->change.wildcard_resolve) {
        create_hostcache_id("*", port, entry_id, sizeof(entry_id));
        entry_len = strlen(entry_id);
        dns = Curl_hash_pick(data->dns.hostcache, entry_id, entry_len + 1);
    }
    if(dns && (data->set.dns_cache_timeout != -1)) {
        struct hostcache_prune_data user;
        time(&user.now);
        user.cache_timeout = data->set.dns_cache_timeout;
        if(hostcache_timestamp_remove(&user, dns)) {
            infof(data, "Hostname in DNS cache was stale, zapped\n");
            dns = NULL;
            Curl_hash_delete(data->dns.hostcache, entry_id, entry_len + 1);
        }
    }
    return dns;
}
struct Curl_dns_entry* Curl_fetch_addr(struct connectdata *conn, const char *hostname, int port) {
    struct Curl_easy *data = conn->data;
    struct Curl_dns_entry *dns = NULL;
    if (data->share) Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);
    dns = fetch_addr(conn, hostname, port);
    if (dns) dns->inuse++;
    if (data->share) Curl_share_unlock(data, CURL_LOCK_DATA_DNS);
    return dns;
}
#ifndef CURL_DISABLE_SHUFFLE_DNS
UNITTEST CURLcode Curl_shuffle_addr(struct Curl_easy *data, struct Curl_addrinfo **addr);
UNITTEST CURLcode Curl_shuffle_addr(struct Curl_easy *data, struct Curl_addrinfo **addr) {
    CURLcode result = CURLE_OK;
    const int num_addrs = Curl_num_addresses(*addr);
    if(num_addrs > 1) {
        struct Curl_addrinfo **nodes;
        infof(data, "Shuffling %i addresses", num_addrs);
        nodes = malloc(num_addrs*sizeof(*nodes));
        if (nodes) {
            int i;
            unsigned int *rnd;
            const size_t rnd_size = num_addrs * sizeof(*rnd);
            nodes[0] = *addr;
            for(i = 1; i < num_addrs; i++) nodes[i] = nodes[i-1]->ai_next;
            rnd = malloc(rnd_size);
            if(rnd) {
                if(Curl_rand(data, (unsigned char *)rnd, rnd_size) == CURLE_OK) {
                    struct Curl_addrinfo *swap_tmp;
                    for(i = num_addrs - 1; i > 0; i--) {
                        swap_tmp = nodes[rnd[i] % (i + 1)];
                        nodes[rnd[i] % (i + 1)] = nodes[i];
                        nodes[i] = swap_tmp;
                    }
                    for(i = 1; i < num_addrs; i++) nodes[i-1]->ai_next = nodes[i];
                    nodes[num_addrs-1]->ai_next = NULL;
                    *addr = nodes[0];
                }
                free(rnd);
            } else result = CURLE_OUT_OF_MEMORY;
            free(nodes);
        } else result = CURLE_OUT_OF_MEMORY;
    }
    return result;
}
#endif
struct Curl_dns_entry* Curl_cache_addr(struct Curl_easy *data, struct Curl_addrinfo *addr, const char *hostname, int port) {
    char entry_id[MAX_HOSTCACHE_LEN];
    size_t entry_len;
    struct Curl_dns_entry *dns;
    struct Curl_dns_entry *dns2;
#ifndef CURL_DISABLE_SHUFFLE_DNS
    if(data->set.dns_shuffle_addresses) {
        CURLcode result = Curl_shuffle_addr(data, &addr);
        if(result) return NULL;
    }
#endif
    dns = calloc(1, sizeof(struct Curl_dns_entry));
    if(!dns) return NULL;
    create_hostcache_id(hostname, port, entry_id, sizeof(entry_id));
    entry_len = strlen(entry_id);
    dns->inuse = 1;
    dns->addr = addr;
    time(&dns->timestamp);
    if(dns->timestamp == 0) dns->timestamp = 1;
    dns2 = Curl_hash_add(data->dns.hostcache, entry_id, entry_len + 1, (void *)dns);
    if(!dns2) {
        free(dns);
        return NULL;
    }
    dns = dns2;
    dns->inuse++;
    return dns;
}
enum resolve_t Curl_resolv(struct connectdata *conn, const char *hostname, int port, bool allowDOH, struct Curl_dns_entry **entry) {
    struct Curl_dns_entry *dns = NULL;
    struct Curl_easy *data = conn->data;
    CURLcode result;
    enum resolve_t rc = CURLRESOLV_ERROR;
    *entry = NULL;
    conn->bits.doh = FALSE;
    if (data->share) Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);
    dns = fetch_addr(conn, hostname, port);
    if (dns) {
        infof(data, "Hostname %s was found in DNS cache\n", hostname);
        dns->inuse++;
        rc = CURLRESOLV_RESOLVED;
    }
    if (data->share) Curl_share_unlock(data, CURL_LOCK_DATA_DNS);
    if (!dns) {
        struct Curl_addrinfo *addr = NULL;
        int respwait = 0;
        struct in_addr in;
    #ifndef USE_RESOLVE_ON_IPS
        const
    #endif
        bool ipnum = FALSE;
        if (data->set.resolver_start) {
            int st;
            Curl_set_in_callback(data, true);
            st = data->set.resolver_start(data->state.resolver, NULL, data->set.resolver_start_client);
            Curl_set_in_callback(data, false);
            if (st) return CURLRESOLV_ERROR;
        }
    #ifndef USE_RESOLVE_ON_IPS
        if (Curl_inet_pton(AF_INET, hostname, &in) > 0) addr = Curl_ip2addr(AF_INET, &in, hostname, port);
    #ifdef ENABLE_IPV6
        if (!addr) {
            struct in6_addr in6;
            if (Curl_inet_pton(AF_INET6, hostname, &in6) > 0) addr = Curl_ip2addr(AF_INET6, &in6, hostname, port);
        }
    #endif
    #else
        if (Curl_inet_pton(AF_INET, hostname, &in) > 0) ipnum = TRUE;
    #ifdef ENABLE_IPV6
        else {
            struct in6_addr in6;
            if (Curl_inet_pton(AF_INET6, hostname, &in6) > 0) ipnum = TRUE;
        }
    #endif
    #endif
        if (!addr) {
            if (!Curl_ipvalid(conn)) return CURLRESOLV_ERROR;
            if (allowDOH && data->set.doh && !ipnum) addr = Curl_doh(conn, hostname, port, &respwait);
            else {
                addr = Curl_getaddrinfo(conn,
    #ifdef DEBUGBUILD
                                        (data->set.str[STRING_DEVICE] && !strcmp(data->set.str[STRING_DEVICE], "LocalHost")) ? "localhost" :
    #endif
                                        hostname, port, &respwait);
            }
        }
        if (!addr) {
            if (respwait) {
                result = Curl_resolv_check(conn, &dns);
                if (result)  return CURLRESOLV_ERROR;
                if (dns) rc = CURLRESOLV_RESOLVED;
                else rc = CURLRESOLV_PENDING;
            }
        } else {
            if (data->share) Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);
            dns = Curl_cache_addr(data, addr, hostname, port);
            if (data->share) Curl_share_unlock(data, CURL_LOCK_DATA_DNS);
            if (!dns) Curl_freeaddrinfo(addr);
            else rc = CURLRESOLV_RESOLVED;
        }
    }
    *entry = dns;
    return rc;
}
#ifdef USE_ALARM_TIMEOUT
static RETSIGTYPE alarmfunc(int sig) {
    (void)sig;
    siglongjmp(curl_jmpenv, 1);
}
#endif
enum resolve_t Curl_resolv_timeout(struct connectdata *conn, const char *hostname, int port, struct Curl_dns_entry **entry, timediff_t timeoutms) {
#ifdef USE_ALARM_TIMEOUT
#ifdef HAVE_SIGACTION
    struct sigaction keep_sigact;
    volatile bool keep_copysig = FALSE;
    struct sigaction sigact;
#else
#ifdef HAVE_SIGNAL
    void (*keep_sigact)(int);
#endif
#endif
    volatile long timeout;
    volatile unsigned int prev_alarm = 0;
    struct Curl_easy *data = conn->data;
#endif
    enum resolve_t rc;
    *entry = NULL;
    if(timeoutms < 0) return CURLRESOLV_TIMEDOUT;
#ifdef USE_ALARM_TIMEOUT
    if(data->set.no_signal) timeout = 0;
    else timeout = (timeoutms > LONG_MAX) ? LONG_MAX : (long)timeoutms;
    if(!timeout) return Curl_resolv(conn, hostname, port, TRUE, entry);
    if(timeout < 1000) {
        failf(data, "remaining timeout of %ld too small to resolve via SIGALRM method", timeout);
        return CURLRESOLV_TIMEDOUT;
    }
    if(sigsetjmp(curl_jmpenv, 1)) {
        failf(data, "name lookup timed out");
        rc = CURLRESOLV_ERROR;
        goto clean_up;
    } else {
    #ifdef HAVE_SIGACTION
        sigaction(SIGALRM, NULL, &sigact);
        keep_sigact = sigact;
        keep_copysig = TRUE;
        sigact.sa_handler = alarmfunc;
    #ifdef SA_RESTART
        sigact.sa_flags &= ~SA_RESTART;
    #endif
        sigaction(SIGALRM, &sigact, NULL);
    #else
    #ifdef HAVE_SIGNAL
        keep_sigact = signal(SIGALRM, alarmfunc);
    #endif
    #endif
        prev_alarm = alarm(curlx_sltoui(timeout/1000L));
    }
#else
#ifndef CURLRES_ASYNCH
    if(timeoutms) infof(conn->data, "timeout on name lookup is not supported\n");
#else
    (void)timeoutms;
#endif
#endif
    rc = Curl_resolv(conn, hostname, port, TRUE, entry);
#ifdef USE_ALARM_TIMEOUT
    clean_up:
    if(!prev_alarm) alarm(0);
#ifdef HAVE_SIGACTION
    if(keep_copysig) sigaction(SIGALRM, &keep_sigact, NULL);
#else
#ifdef HAVE_SIGNAL
    signal(SIGALRM, keep_sigact);
#endif
#endif
    if(prev_alarm) {
        timediff_t elapsed_secs = Curl_timediff(Curl_now(), conn->created) / 1000;
        unsigned long alarm_set = (unsigned long)(prev_alarm - elapsed_secs);
        if(!alarm_set || ((alarm_set >= 0x80000000) && (prev_alarm < 0x80000000)) ) {
            alarm(1);
            rc = CURLRESOLV_TIMEDOUT;
            failf(data, "Previous alarm fired off!");
        } else alarm((unsigned int)alarm_set);
    }
#endif
    return rc;
}
void Curl_resolv_unlock(struct Curl_easy *data, struct Curl_dns_entry *dns) {
    if(data && data->share) Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);
    freednsentry(dns);
    if(data && data->share) Curl_share_unlock(data, CURL_LOCK_DATA_DNS);
}
static void freednsentry(void *freethis) {
    struct Curl_dns_entry *dns = (struct Curl_dns_entry *) freethis;
    DEBUGASSERT(dns && (dns->inuse>0));
    dns->inuse--;
    if (dns->inuse == 0) {
        Curl_freeaddrinfo(dns->addr);
        free(dns);
    }
}
int Curl_mk_dnscache(struct curl_hash *hash) {
    return Curl_hash_init(hash, 7, Curl_hash_str, Curl_str_key_compare, freednsentry);
}
void Curl_hostcache_clean(struct Curl_easy *data, struct curl_hash *hash) {
    if (data && data->share) Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);
    Curl_hash_clean(hash);
    if (data && data->share) Curl_share_unlock(data, CURL_LOCK_DATA_DNS);
}
CURLcode Curl_loadhostpairs(struct Curl_easy *data) {
    struct curl_slist *hostp;
    char hostname[256];
    int port = 0;
    data->change.wildcard_resolve = false;
    for (hostp = data->change.resolve; hostp; hostp = hostp->next) {
        char entry_id[MAX_HOSTCACHE_LEN];
        if(!hostp->data) continue;
        if(hostp->data[0] == '-') {
            size_t entry_len;
            if(2 != sscanf(hostp->data + 1, "%255[^:]:%d", hostname, &port)) {
                infof(data, "Couldn't parse CURLOPT_RESOLVE removal entry '%s'!\n", hostp->data);
                continue;
            }
            create_hostcache_id(hostname, port, entry_id, sizeof(entry_id));
            entry_len = strlen(entry_id);
            if(data->share) Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);
            Curl_hash_delete(data->dns.hostcache, entry_id, entry_len + 1);
            if(data->share) Curl_share_unlock(data, CURL_LOCK_DATA_DNS);
        } else {
            struct Curl_dns_entry *dns;
            struct Curl_addrinfo *head = NULL, *tail = NULL;
            size_t entry_len;
            char address[64];
        #if !defined(CURL_DISABLE_VERBOSE_STRINGS)
            char *addresses = NULL;
        #endif
            char *addr_begin;
            char *addr_end;
            char *port_ptr;
            char *end_ptr;
            char *host_end;
            unsigned long tmp_port;
            bool error = true;
            host_end = strchr(hostp->data, ':');
            if (!host_end || ((host_end - hostp->data) >= (ptrdiff_t)sizeof(hostname))) goto err;
            memcpy(hostname, hostp->data, host_end - hostp->data);
            hostname[host_end - hostp->data] = '\0';
            port_ptr = host_end + 1;
            tmp_port = strtoul(port_ptr, &end_ptr, 10);
            if (tmp_port > USHRT_MAX || end_ptr == port_ptr || *end_ptr != ':') goto err;
            port = (int)tmp_port;
        #if !defined(CURL_DISABLE_VERBOSE_STRINGS)
            addresses = end_ptr + 1;
        #endif
            while(*end_ptr) {
                size_t alen;
                struct Curl_addrinfo *ai;
                addr_begin = end_ptr + 1;
                addr_end = strchr(addr_begin, ',');
                if (!addr_end) addr_end = addr_begin + strlen(addr_begin);
                end_ptr = addr_end;
                if (*addr_begin == '[') {
                    if (addr_end == addr_begin || *(addr_end - 1) != ']') goto err;
                    ++addr_begin;
                    --addr_end;
                }
                alen = addr_end - addr_begin;
                if (!alen) continue;
                if (alen >= sizeof(address)) goto err;
                memcpy(address, addr_begin, alen);
                address[alen] = '\0';
            #ifndef ENABLE_IPV6
                if (strchr(address, ':')) {
                    infof(data, "Ignoring resolve address '%s', missing IPv6 support.\n", address);
                    continue;
                }
            #endif
                ai = Curl_str2addr(address, port);
                if (!ai) {
                    infof(data, "Resolve address '%s' found illegal!\n", address);
                    goto err;
                }
                if (tail) {
                    tail->ai_next = ai;
                    tail = tail->ai_next;
                } else head = tail = ai;
            }
            if(!head) goto err;
            error = false;
            err:
            if(error) {
                infof(data, "Couldn't parse CURLOPT_RESOLVE entry '%s'!\n", hostp->data);
                Curl_freeaddrinfo(head);
                continue;
            }
            create_hostcache_id(hostname, port, entry_id, sizeof(entry_id));
            entry_len = strlen(entry_id);
            if(data->share) Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);
            dns = Curl_hash_pick(data->dns.hostcache, entry_id, entry_len + 1);
            if(dns) {
                infof(data, "RESOLVE %s:%d is - old addresses discarded!\n", hostname, port);
                Curl_hash_delete(data->dns.hostcache, entry_id, entry_len + 1);
            }
            dns = Curl_cache_addr(data, head, hostname, port);
            if(dns) {
                dns->timestamp = 0;
                dns->inuse--;
            }
            if(data->share) Curl_share_unlock(data, CURL_LOCK_DATA_DNS);
            if(!dns) {
                Curl_freeaddrinfo(head);
                return CURLE_OUT_OF_MEMORY;
            }
            infof(data, "Added %s:%d:%s to DNS cache\n", hostname, port, addresses);
            if(hostname[0] == '*' && hostname[1] == '\0') {
                infof(data, "RESOLVE %s:%d is wildcard, enabling wildcard checks\n", hostname, port);
                data->change.wildcard_resolve = true;
            }
        }
    }
    data->change.resolve = NULL;
    return CURLE_OK;
}
CURLcode Curl_resolv_check(struct connectdata *conn, struct Curl_dns_entry **dns) {
#if defined(CURL_DISABLE_DOH) && !defined(CURLRES_ASYNCH)
    (void)dns;
#endif
    if(conn->bits.doh) return Curl_doh_is_resolved(conn, dns);
    return Curl_resolver_is_resolved(conn, dns);
}
int Curl_resolv_getsock(struct connectdata *conn, curl_socket_t *socks) {
#ifdef CURLRES_ASYNCH
    if (conn->bits.doh) return GETSOCK_BLANK;
    return Curl_resolver_getsock(conn, socks);
#else
    (void)conn;
    (void)socks;
    return GETSOCK_BLANK;
#endif
}
CURLcode Curl_once_resolved(struct connectdata *conn, bool *protocol_done) {
    CURLcode result;
    if(conn->async.dns) {
        conn->dns_entry = conn->async.dns;
        conn->async.dns = NULL;
    }
    result = Curl_setup_conn(conn, protocol_done);
    if(result) {
        struct Curl_easy *data = conn->data;
        DEBUGASSERT(data);
        Curl_detach_connnection(data);
        Curl_conncache_remove_conn(data, conn, TRUE);
        Curl_disconnect(data, conn, TRUE);
    }
    return result;
}