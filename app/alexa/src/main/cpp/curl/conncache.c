#include <android/log.h>
#include <stdbool.h>
#include "curl_setup.h"
#include "curl.h"
#include "easy.h"
#include "urldata.h"
#include "url.h"
#include "progress.h"
#include "multiif.h"
#include "sendf.h"
#include "conncache.h"
#include "share.h"
#include "sigpipe.h"
#include "connect.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

#define HASHKEY_SIZE 128

static void conn_llist_dtor(void *user, void *element) {
    struct connectdata *conn = element;
    (void)user;
    conn->bundle = NULL;
}
static CURLcode bundle_create(struct connectbundle **bundlep) {
    DEBUGASSERT(*bundlep == NULL);
    *bundlep = malloc(sizeof(struct connectbundle));
    if(!*bundlep) return CURLE_OUT_OF_MEMORY;
    (*bundlep)->num_connections = 0;
    (*bundlep)->multiuse = BUNDLE_UNKNOWN;
    Curl_llist_init(&(*bundlep)->conn_list, (curl_llist_dtor) conn_llist_dtor);
    return CURLE_OK;
}
static void bundle_destroy(struct connectbundle *bundle) {
    if(!bundle) return;
    Curl_llist_destroy(&bundle->conn_list, NULL);
    free(bundle);
}
static void bundle_add_conn(struct connectbundle *bundle, struct connectdata *conn) {
    Curl_llist_insert_next(&bundle->conn_list, bundle->conn_list.tail, conn, &conn->bundle_node);
    conn->bundle = bundle;
    bundle->num_connections++;
}
static int bundle_remove_conn(struct connectbundle *bundle, struct connectdata *conn) {
    struct curl_llist_element *curr;
    curr = bundle->conn_list.head;
    while(curr) {
        if(curr->ptr == conn) {
            Curl_llist_remove(&bundle->conn_list, curr, NULL);
            bundle->num_connections--;
            conn->bundle = NULL;
            return 1;
        }
        curr = curr->next;
    }
    DEBUGASSERT(0);
    return 0;
}
static void free_bundle_hash_entry(void *freethis) {
    struct connectbundle *b = (struct connectbundle *) freethis;
    bundle_destroy(b);
}
int Curl_conncache_init(struct conncache *connc, int size) {
    int rc;
    connc->closure_handle = curl_easy_init();
    if (!connc->closure_handle) return 1;
    rc = Curl_hash_init(&connc->hash, size, Curl_hash_str, Curl_str_key_compare, free_bundle_hash_entry);
    if (rc) Curl_close(&connc->closure_handle);
    else connc->closure_handle->state.conn_cache = connc;
    return rc;
}
void Curl_conncache_destroy(struct conncache *connc) {
  if(connc) Curl_hash_destroy(&connc->hash);
}
static void hashkey(struct connectdata *conn, char *buf, size_t len, const char **hostp) {
    const char *hostname;
    long port = conn->remote_port;
    #ifndef CURL_DISABLE_PROXY
        if (conn->bits.httpproxy && !conn->bits.tunnel_proxy) {
            hostname = conn->http_proxy.host.name;
            port = conn->port;
        }
      else if(conn->bits.conn_to_host) hostname = conn->conn_to_host.name;
      else hostname = conn->host.name;
    #endif
    if(hostp) *hostp = hostname;
    msnprintf(buf, len, "%ld%s", port, hostname);
}
size_t Curl_conncache_size(struct Curl_easy *data) {
    size_t num;
    CONNCACHE_LOCK(data);
    num = data->state.conn_cache->num_conn;
    CONNCACHE_UNLOCK(data);
    return num;
}
struct connectbundle *Curl_conncache_find_bundle(struct connectdata *conn, struct conncache *connc, const char **hostp) {
    struct connectbundle *bundle = NULL;
    CONNCACHE_LOCK(conn->data);
    if (connc) {
        char key[HASHKEY_SIZE];
        hashkey(conn, key, sizeof(key), hostp);
        bundle = Curl_hash_pick(&connc->hash, key, strlen(key));
    }
    return bundle;
}
static bool conncache_add_bundle(struct conncache *connc, char *key, struct connectbundle *bundle)
{
  void *p = Curl_hash_add(&connc->hash, key, strlen(key), bundle);
  return p ? 1 : 0;
}
static void conncache_remove_bundle(struct conncache *connc, struct connectbundle *bundle) {
    struct curl_hash_iterator iter;
    struct curl_hash_element *he;
    if (!connc) return;
    Curl_hash_start_iterate(&connc->hash, &iter);
    he = Curl_hash_next_element(&iter);
    while(he) {
        if (he->ptr == bundle) {
          Curl_hash_delete(&connc->hash, he->key, he->key_len);
          return;
        }
        he = Curl_hash_next_element(&iter);
    }
}
CURLcode Curl_conncache_add_conn(struct conncache *connc, struct connectdata *conn) {
    CURLcode result = CURLE_OK;
    struct connectbundle *bundle = NULL;
    struct Curl_easy *data = conn->data;
    bundle = Curl_conncache_find_bundle(conn, data->state.conn_cache, NULL);
    if (!bundle) {
        int rc;
        char key[HASHKEY_SIZE];
        result = bundle_create(&bundle);
        if (result) goto unlock;
        hashkey(conn, key, sizeof(key), NULL);
        rc = conncache_add_bundle(data->state.conn_cache, key, bundle);
        if (!rc) {
            bundle_destroy(bundle);
            result = CURLE_OUT_OF_MEMORY;
            goto unlock;
        }
    }
    bundle_add_conn(bundle, conn);
    conn->connection_id = connc->next_connection_id++;
    connc->num_conn++;
    DEBUGF(infof(conn->data, "Added connection %ld. " "The cache now contains %zu members\n", conn->connection_id, connc->num_conn));
    unlock:
    CONNCACHE_UNLOCK(data);
    return result;
}
void Curl_conncache_remove_conn(struct Curl_easy *data, struct connectdata *conn, bool lock) {
    struct connectbundle *bundle = conn->bundle;
    struct conncache *connc = data->state.conn_cache;
    if (bundle) {
        if (lock) CONNCACHE_LOCK(data);
        bundle_remove_conn(bundle, conn);
        if (bundle->num_connections == 0) conncache_remove_bundle(connc, bundle);
        conn->bundle = NULL;
        if (connc) {
            connc->num_conn--;
            DEBUGF(infof(data, "The cache now contains %zu members\n", connc->num_conn));
        }
        if (lock) CONNCACHE_UNLOCK(data);
    }
}
bool Curl_conncache_foreach(struct Curl_easy *data, struct conncache *connc, void *param, int (*func)(struct connectdata *conn, void *param)) {
    struct curl_hash_iterator iter;
    struct curl_llist_element *curr;
    struct curl_hash_element *he;
    if (!connc) return 0;
    CONNCACHE_LOCK(data);
    Curl_hash_start_iterate(&connc->hash, &iter);
    he = Curl_hash_next_element(&iter);
    while(he) {
        struct connectbundle *bundle;
        bundle = he->ptr;
        he = Curl_hash_next_element(&iter);
        curr = bundle->conn_list.head;
        while(curr) {
            struct connectdata *conn = curr->ptr;
            curr = curr->next;
            if (1 == func(conn, param)) {
                CONNCACHE_UNLOCK(data);
                return 1;
            }
        }
    }
    CONNCACHE_UNLOCK(data);
    return 0;
}
static struct connectdata* conncache_find_first_connection(struct conncache *connc) {
    struct curl_hash_iterator iter;
    struct curl_hash_element *he;
    struct connectbundle *bundle;
    Curl_hash_start_iterate(&connc->hash, &iter);
    he = Curl_hash_next_element(&iter);
    while(he) {
        struct curl_llist_element *curr;
        bundle = he->ptr;
        curr = bundle->conn_list.head;
        if (curr) return curr->ptr;
        he = Curl_hash_next_element(&iter);
    }
    return NULL;
}
bool Curl_conncache_return_conn(struct Curl_easy *data, struct connectdata *conn) {
    size_t maxconnects = (data->multi->maxconnects < 0) ? data->multi->num_easy * 4: data->multi->maxconnects;
    struct connectdata *conn_candidate = NULL;
    conn->lastused = Curl_now();
    if (maxconnects > 0 && Curl_conncache_size(data) > maxconnects) {
        infof(data, "Connection cache is full, closing the oldest one.\n");
        conn_candidate = Curl_conncache_extract_oldest(data);
        if (conn_candidate) (void)Curl_disconnect(data, conn_candidate, 0);
    }
    return (conn_candidate == conn) ? 0 : 1;
}
struct connectdata* Curl_conncache_extract_bundle(struct Curl_easy *data, struct connectbundle *bundle) {
    struct curl_llist_element *curr;
    timediff_t highscore = -1;
    timediff_t score;
    struct curltime now;
    struct connectdata *conn_candidate = NULL;
    struct connectdata *conn;
    (void)data;
    now = Curl_now();
    curr = bundle->conn_list.head;
    while(curr) {
        conn = curr->ptr;
        if (!CONN_INUSE(conn) && !conn->data) {
            score = Curl_timediff(now, conn->lastused);
            if (score > highscore) {
                highscore = score;
                conn_candidate = conn;
            }
        }
        curr = curr->next;
    }
    if (conn_candidate) {
        bundle_remove_conn(bundle, conn_candidate);
        data->state.conn_cache->num_conn--;
        DEBUGF(infof(data, "The cache now contains %zu members\n", data->state.conn_cache->num_conn));
        conn_candidate->data = data;
    }
    return conn_candidate;
}
struct connectdata * Curl_conncache_extract_oldest(struct Curl_easy *data) {
    struct conncache *connc = data->state.conn_cache;
    struct curl_hash_iterator iter;
    struct curl_llist_element *curr;
    struct curl_hash_element *he;
    timediff_t highscore =- 1;
    timediff_t score;
    struct curltime now;
    struct connectdata *conn_candidate = NULL;
    struct connectbundle *bundle;
    struct connectbundle *bundle_candidate = NULL;
    now = Curl_now();
    CONNCACHE_LOCK(data);
    Curl_hash_start_iterate(&connc->hash, &iter);
    he = Curl_hash_next_element(&iter);
    while(he) {
        struct connectdata *conn;
        bundle = he->ptr;
        curr = bundle->conn_list.head;
        while(curr) {
            conn = curr->ptr;
            if (!CONN_INUSE(conn) && !conn->data && !conn->bits.close && !conn->bits.connect_only) {
                score = Curl_timediff(now, conn->lastused);
                if(score > highscore) {
                    highscore = score;
                    conn_candidate = conn;
                    bundle_candidate = bundle;
                }
            }
            curr = curr->next;
        }
        he = Curl_hash_next_element(&iter);
    }
    if(conn_candidate) {
        bundle_remove_conn(bundle_candidate, conn_candidate);
        connc->num_conn--;
        DEBUGF(infof(data, "The cache now contains %zu members\n", connc->num_conn));
        conn_candidate->data = data;
    }
    CONNCACHE_UNLOCK(data);
    return conn_candidate;
}
void Curl_conncache_close_all_connections(struct conncache *connc) {
    struct connectdata *conn;
    char buffer[READBUFFER_MIN + 1];
    if (!connc->closure_handle) return;
    connc->closure_handle->state.buffer = buffer;
    connc->closure_handle->set.buffer_size = READBUFFER_MIN;
    conn = conncache_find_first_connection(connc);
    while(conn) {
        SIGPIPE_VARIABLE(pipe_st);
        conn->data = connc->closure_handle;
        sigpipe_ignore(conn->data, &pipe_st);
        connclose(conn, "kill all");
        Curl_conncache_remove_conn(conn->data, conn, TRUE);
        (void)Curl_disconnect(connc->closure_handle, conn, FALSE);
        sigpipe_restore(&pipe_st);
        conn = conncache_find_first_connection(connc);
    }
    connc->closure_handle->state.buffer = NULL;
    if (connc->closure_handle) {
        SIGPIPE_VARIABLE(pipe_st);
        sigpipe_ignore(connc->closure_handle, &pipe_st);
        Curl_hostcache_clean(connc->closure_handle, connc->closure_handle->dns.hostcache);
        Curl_close(&connc->closure_handle);
        sigpipe_restore(&pipe_st);
    }
}
#if 0
void Curl_conncache_print(struct conncache *connc) {
    struct curl_hash_iterator iter;
    struct curl_llist_element *curr;
    struct curl_hash_element *he;
    if (!connc) return;
    android_log_print(ANDROID_LOG_INFO, "error", "%s =Bundle cache=", stderr);
    Curl_hash_start_iterate(connc->hash, &iter);
    he = Curl_hash_next_element(&iter);
    while(he) {
    struct connectbundle *bundle;
    struct connectdata *conn;
    bundle = he->ptr;
    android_log_print(ANDROID_LOG_INFO, "error", "%s - %s", stderror, he->key);
    curr = bundle->conn_list->head;
    while(curr) {
        conn = curr->ptr;
        android_log_print(ANDROID_LOG_INFO, "error", "%s [%p %d]", stderr, (void *)conn, conn->inuse);
        curr = curr->next;
    }
    android_log_print(ANDROID_LOG_INFO, "error", "%s", stderr);
    he = Curl_hash_next_element(&iter);
  }
}
#endif
