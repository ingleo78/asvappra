#ifndef HEADER_CURL_CONNCACHE_H
#define HEADER_CURL_CONNCACHE_H

struct conncache {
    struct curl_hash hash;
    size_t num_conn;
    long next_connection_id;
    struct curltime last_cleanup;
    struct Curl_easy *closure_handle;
};
#define BUNDLE_NO_MULTIUSE -1
#define BUNDLE_UNKNOWN     0
#define BUNDLE_MULTIPLEX   2
#ifdef CURLDEBUG
#define CONNCACHE_LOCK(x) if((x)->share) { \
    Curl_share_lock((x), CURL_LOCK_DATA_CONNECT, CURL_LOCK_ACCESS_SINGLE); \
    DEBUGASSERT(!(x)->state.conncache_lock); \
    (x)->state.conncache_lock = TRUE; \
}
#define CONNCACHE_UNLOCK(x) if((x)->share) { \
    DEBUGASSERT((x)->state.conncache_lock); \
    (x)->state.conncache_lock = FALSE; \
    Curl_share_unlock((x), CURL_LOCK_DATA_CONNECT); \
}
#else
#define CONNCACHE_LOCK(x) if((x)->share)  Curl_share_lock((x), CURL_LOCK_DATA_CONNECT, CURL_LOCK_ACCESS_SINGLE)
#define CONNCACHE_UNLOCK(x) if((x)->share)  Curl_share_unlock((x), CURL_LOCK_DATA_CONNECT)
#endif
struct connectbundle {
    int multiuse;
    size_t num_connections;
    struct curl_llist conn_list;
};
int Curl_conncache_init(struct conncache *, int size);
void Curl_conncache_destroy(struct conncache *connc);
struct connectbundle *Curl_conncache_find_bundle(struct connectdata *conn, struct conncache *connc, const char **hostp);
size_t Curl_conncache_size(struct Curl_easy *data);
bool Curl_conncache_return_conn(struct Curl_easy *data, struct connectdata *conn);
CURLcode Curl_conncache_add_conn(struct conncache *connc, struct connectdata *conn) WARN_UNUSED_RESULT;
void Curl_conncache_remove_conn(struct Curl_easy *data, struct connectdata *conn, bool lock);
bool Curl_conncache_foreach(struct Curl_easy *data, struct conncache *connc, void *param, int (*func)(struct connectdata *conn, void *param));
struct connectdata * Curl_conncache_find_first_connection(struct conncache *connc);
struct connectdata * Curl_conncache_extract_bundle(struct Curl_easy *data, struct connectbundle *bundle);
struct connectdata * Curl_conncache_extract_oldest(struct Curl_easy *data);
void Curl_conncache_close_all_connections(struct conncache *connc);
void Curl_conncache_print(struct conncache *connc);
#endif