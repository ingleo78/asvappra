#ifndef HEADER_CURL_MULTIHANDLE_H
#define HEADER_CURL_MULTIHANDLE_H

#include "conncache.h"
#include "psl.h"
#include "socketpair.h"
#include "multi.h"

struct Curl_message {
    struct curl_llist_element list;
    struct CURLMsg extmsg;
};
typedef enum {
    CURLM_STATE_INIT,
    CURLM_STATE_CONNECT_PEND,
    CURLM_STATE_CONNECT,
    CURLM_STATE_WAITRESOLVE,
    CURLM_STATE_WAITCONNECT,
    CURLM_STATE_WAITPROXYCONNECT,
    CURLM_STATE_SENDPROTOCONNECT,
    CURLM_STATE_PROTOCONNECT,
    CURLM_STATE_DO,
    CURLM_STATE_DOING,
    CURLM_STATE_DO_MORE,
    CURLM_STATE_DO_DONE,
    CURLM_STATE_PERFORM,
    CURLM_STATE_TOOFAST,
    CURLM_STATE_DONE,
    CURLM_STATE_COMPLETED,
    CURLM_STATE_MSGSENT,
    CURLM_STATE_LAST
} CURLMstate;
#define MAX_SOCKSPEREASYHANDLE 5
#define GETSOCK_READABLE (0x00ff)
#define GETSOCK_WRITABLE (0xff00)
#define CURLPIPE_ANY (CURLPIPE_MULTIPLEX)
#if defined(USE_SOCKETPAIR) && !defined(USE_BLOCKING_SOCKETS)
#define ENABLE_WAKEUP
#endif
#define INITIAL_MAX_CONCURRENT_STREAMS ((1U << 31) - 1)
struct Curl_multi {
    long type;
    struct Curl_easy *easyp;
    struct Curl_easy *easylp;
    int num_easy;
    int num_alive;
    struct curl_llist msglist;
    struct curl_llist pending;
    curl_socket_callback socket_cb;
    void *socket_userp;
    curl_push_callback push_cb;
    void *push_userp;
    struct curl_hash hostcache;
#ifdef USE_LIBPSL
    struct PslCache psl;
#endif
    struct Curl_tree *timetree;
    struct curl_hash sockhash;
    struct conncache conn_cache;
    long maxconnects;
    long max_host_connections;
    long max_total_connections;
    curl_multi_timer_callback timer_cb;
    void *timer_userp;
    struct curltime timer_lastcall;
    unsigned int max_concurrent_streams;
#ifdef ENABLE_WAKEUP
    curl_socket_t wakeup_pair[2];
#endif
    bool multiplexing;
    bool recheckstate;
    bool in_callback;
    bool ipv6_works;
};
#endif