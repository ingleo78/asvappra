#include "gcache.h"
#include "ghash.h"
#include "gtestutils.h"

typedef struct _GCacheNode  GCacheNode;
struct _GCacheNode {
  gpointer value;
  gint ref_count;
};
struct _GCache {
  GCacheNewFunc value_new_func;
  GCacheDestroyFunc value_destroy_func;
  GCacheDupFunc key_dup_func;
  GCacheDestroyFunc key_destroy_func;
  GHashTable *key_table;
  GHashTable *value_table;
};
static inline GCacheNode* g_cache_node_new(gpointer value) {
  GCacheNode *node = g_slice_new(GCacheNode);
  node->value = value;
  node->ref_count = 1;
  return node;
}
static inline void g_cache_node_destroy(GCacheNode *node) {
  g_slice_free(GCacheNode, node);
}
GCache* g_cache_new(GCacheNewFunc value_new_func, GCacheDestroyFunc value_destroy_func, GCacheDupFunc key_dup_func, GCacheDestroyFunc key_destroy_func,
                    GHashFunc hash_key_func, GHashFunc hash_value_func, GEqualFunc key_equal_func) {
  GCache *cache;
  g_return_val_if_fail(value_new_func != NULL, NULL);
  g_return_val_if_fail(value_destroy_func != NULL, NULL);
  g_return_val_if_fail(key_dup_func != NULL, NULL);
  g_return_val_if_fail(key_destroy_func != NULL, NULL);
  g_return_val_if_fail(hash_key_func != NULL, NULL);
  g_return_val_if_fail(hash_value_func != NULL, NULL);
  g_return_val_if_fail(key_equal_func != NULL, NULL);
  cache = g_slice_new(GCache);
  cache->value_new_func = value_new_func;
  cache->value_destroy_func = value_destroy_func;
  cache->key_dup_func = key_dup_func;
  cache->key_destroy_func = key_destroy_func;
  cache->key_table = g_hash_table_new(hash_key_func, key_equal_func);
  cache->value_table = g_hash_table_new(hash_value_func, NULL);
  return cache;
}
void g_cache_destroy(GCache *cache) {
  g_return_if_fail(cache != NULL);
  g_hash_table_destroy(cache->key_table);
  g_hash_table_destroy(cache->value_table);
  g_slice_free(GCache, cache);
}
gpointer g_cache_insert(GCache *cache, gpointer key) {
  GCacheNode *node;
  gpointer value;
  g_return_val_if_fail(cache != NULL, NULL);
  node = g_hash_table_lookup(cache->key_table, key);
  if (node) {
      node->ref_count += 1;
      return node->value;
  }
  key = (*cache->key_dup_func)(key);
  value = (*cache->value_new_func)(key);
  node = g_cache_node_new(value);
  g_hash_table_insert(cache->key_table, key, node);
  g_hash_table_insert(cache->value_table, value, key);
  return node->value;
}
void g_cache_remove(GCache *cache, gconstpointer value) {
  GCacheNode *node;
  gpointer key;
  g_return_if_fail(cache != NULL);
  key = g_hash_table_lookup(cache->value_table, value);
  node = g_hash_table_lookup(cache->key_table, key);
  g_return_if_fail(node != NULL);
  node->ref_count -= 1;
  if (node->ref_count == 0) {
      g_hash_table_remove(cache->value_table, value);
      g_hash_table_remove(cache->key_table, key);
      (*cache->key_destroy_func)(key);
      (*cache->value_destroy_func)(node->value);
      g_cache_node_destroy(node);
  }
}
void g_cache_key_foreach(GCache *cache, GHFunc func, gpointer user_data) {
  g_return_if_fail(cache != NULL);
  g_return_if_fail(func != NULL);
  g_hash_table_foreach(cache->value_table, func, user_data);
}
void g_cache_value_foreach(GCache *cache, GHFunc func, gpointer user_data) {
  g_return_if_fail(cache != NULL);
  g_return_if_fail(func != NULL);
  g_hash_table_foreach(cache->key_table, func, user_data);
}