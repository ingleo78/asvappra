#include <string.h>
#include "ghash.h"
#include "gatomic.h"
#include "gtestutils.h"

#define HASH_TABLE_MIN_SHIFT 3
typedef struct _GHashNode      GHashNode;
struct _GHashNode {
  gpointer key;
  gpointer value;
  guint key_hash;
};
struct _GHashTable {
  gint size;
  gint mod;
  guint mask;
  gint nnodes;
  gint noccupied;
  GHashNode *nodes;
  GHashFunc hash_func;
  GEqualFunc key_equal_func;
  volatile gint ref_count;
#ifndef G_DISABLE_ASSERT
  int version;
#endif
  GDestroyNotify key_destroy_func;
  GDestroyNotify value_destroy_func;
};
typedef struct {
  GHashTable *hash_table;
  gpointer dummy1;
  gpointer dummy2;
  int position;
  gboolean dummy3;
  int version;
} RealIter;
static const gint prime_mod [] = {
  1,
  2,
  3,
  7,
  13,
  31,
  61,
  127,
  251,
  509,
  1021,
  2039,
  4093,
  8191,
  16381,
  32749,
  65521,
  131071,
  262139,
  524287,
  1048573,
  2097143,
  4194301,
  8388593,
  16777213,
  33554393,
  67108859,
  134217689,
  268435399,
  536870909,
  1073741789,
  2147483647
};
static void g_hash_table_set_shift(GHashTable *hash_table, gint shift) {
  gint i;
  guint mask = 0;
  hash_table->size = 1 << shift;
  hash_table->mod  = prime_mod[shift];
  for (i = 0; i < shift; i++) {
      mask <<= 1;
      mask |= 1;
  }
  hash_table->mask = mask;
}
static gint g_hash_table_find_closest_shift (gint n) {
  gint i;
  for (i = 0; n; i++) n >>= 1;
  return i;
}
static void g_hash_table_set_shift_from_size(GHashTable *hash_table, gint size) {
  gint shift;
  shift = g_hash_table_find_closest_shift(size);
  shift = MAX(shift, HASH_TABLE_MIN_SHIFT);
  g_hash_table_set_shift(hash_table, shift);
}
static inline guint g_hash_table_lookup_node(GHashTable *hash_table, gconstpointer key) {
  GHashNode *node;
  guint node_index;
  guint hash_value;
  guint step = 0;
  hash_value = (* hash_table->hash_func) (key);
  if (G_UNLIKELY(hash_value <= 1)) hash_value = 2;
  node_index = hash_value % hash_table->mod;
  node = &hash_table->nodes [node_index];
  while (node->key_hash) {
      if (node->key_hash == hash_value) {
          if (hash_table->key_equal_func) {
              if (hash_table->key_equal_func (node->key, key)) break;
          } else if (node->key == key) break;
      }
      step++;
      node_index += step;
      node_index &= hash_table->mask;
      node = &hash_table->nodes[node_index];
  }
  return node_index;
}
static inline guint g_hash_table_lookup_node_for_insertion(GHashTable *hash_table, gconstpointer key, guint *hash_return) {
  GHashNode *node;
  guint node_index;
  guint hash_value;
  guint first_tombstone;
  gboolean have_tombstone = FALSE;
  guint step = 0;
  hash_value = (*hash_table->hash_func)(key);
  if (G_UNLIKELY(hash_value <= 1)) hash_value = 2;
  *hash_return = hash_value;
  node_index = hash_value % hash_table->mod;
  node = &hash_table->nodes[node_index];
  while(node->key_hash) {
      if (node->key_hash == hash_value) {
          if (hash_table->key_equal_func) {
              if (hash_table->key_equal_func(node->key, key)) return node_index;
          } else if (node->key == key) return node_index;
      } else if (node->key_hash == 1 && !have_tombstone) {
          first_tombstone = node_index;
          have_tombstone = TRUE;
      }
      step++;
      node_index += step;
      node_index &= hash_table->mask;
      node = &hash_table->nodes[node_index];
  }
  if (have_tombstone) return first_tombstone;
  return node_index;
}
static void g_hash_table_remove_node(GHashTable *hash_table,
                          GHashNode *node,
                          gboolean notify) {
  if (notify && hash_table->key_destroy_func) hash_table->key_destroy_func(node->key);
  if (notify && hash_table->value_destroy_func) hash_table->value_destroy_func(node->value);
  node->key_hash = 1;
  node->key = NULL;
  node->value = NULL;
  hash_table->nnodes--;
}
static void g_hash_table_remove_all_nodes(GHashTable *hash_table, gboolean notify) {
  int i;
  for (i = 0; i < hash_table->size; i++) {
      GHashNode *node = &hash_table->nodes [i];
      if (node->key_hash > 1) {
          if (notify && hash_table->key_destroy_func) hash_table->key_destroy_func (node->key);
          if (notify && hash_table->value_destroy_func) hash_table->value_destroy_func (node->value);
      }
  }
  memset (hash_table->nodes, 0, hash_table->size * sizeof(GHashNode));
  hash_table->nnodes = 0;
  hash_table->noccupied = 0;
}
static void g_hash_table_resize(GHashTable *hash_table) {
  GHashNode *new_nodes;
  gint old_size;
  gint i;
  old_size = hash_table->size;
  g_hash_table_set_shift_from_size(hash_table, hash_table->nnodes * 2);
  new_nodes = g_new0(GHashNode, hash_table->size);
  for (i = 0; i < old_size; i++) {
      GHashNode *node = &hash_table->nodes[i];
      GHashNode *new_node;
      guint hash_val;
      guint step = 0;
      if (node->key_hash <= 1) continue;
      hash_val = node->key_hash % hash_table->mod;
      new_node = &new_nodes[hash_val];
      while(new_node->key_hash) {
          step++;
          hash_val += step;
          hash_val &= hash_table->mask;
          new_node = &new_nodes[hash_val];
      }
      *new_node = *node;
  }
  g_free(hash_table->nodes);
  hash_table->nodes = new_nodes;
  hash_table->noccupied = hash_table->nnodes;
}
static inline void g_hash_table_maybe_resize(GHashTable *hash_table) {
  gint noccupied = hash_table->noccupied;
  gint size = hash_table->size;
  if ((size > hash_table->nnodes * 4 && size > 1 << HASH_TABLE_MIN_SHIFT) || (size <= noccupied + (noccupied / 16))) g_hash_table_resize(hash_table);
}
GHashTable* g_hash_table_new(GHashFunc hash_func, GEqualFunc key_equal_func) {
  return g_hash_table_new_full(hash_func, key_equal_func, NULL, NULL);
}
GHashTable* g_hash_table_new_full(GHashFunc hash_func, GEqualFunc key_equal_func, GDestroyNotify key_destroy_func, GDestroyNotify value_destroy_func) {
  GHashTable *hash_table;
  hash_table = g_slice_new(GHashTable);
  g_hash_table_set_shift(hash_table, HASH_TABLE_MIN_SHIFT);
  hash_table->nnodes = 0;
  hash_table->noccupied = 0;
  hash_table->hash_func = hash_func ? hash_func : g_direct_hash;
  hash_table->key_equal_func = key_equal_func;
  hash_table->ref_count = 1;
#ifndef G_DISABLE_ASSERT
  hash_table->version = 0;
#endif
  hash_table->key_destroy_func = key_destroy_func;
  hash_table->value_destroy_func = value_destroy_func;
  hash_table->nodes = g_new0(GHashNode, hash_table->size);
  return hash_table;
}
void g_hash_table_iter_init(GHashTableIter *iter,GHashTable *hash_table) {
  RealIter *ri = (RealIter*)iter;
  g_return_if_fail(iter != NULL);
  g_return_if_fail(hash_table != NULL);
  ri->hash_table = hash_table;
  ri->position = -1;
#ifndef G_DISABLE_ASSERT
  ri->version = hash_table->version;
#endif
}
gboolean g_hash_table_iter_next(GHashTableIter *iter, gpointer *key, gpointer *value) {
  RealIter *ri = (RealIter*)iter;
  GHashNode *node;
  gint position;
  g_return_val_if_fail(iter != NULL, FALSE);
#ifndef G_DISABLE_ASSERT
  g_return_val_if_fail(ri->version == ri->hash_table->version, FALSE);
#endif
  g_return_val_if_fail(ri->position < ri->hash_table->size, FALSE);
  position = ri->position;
  do {
      position++;
      if (position >= ri->hash_table->size) {
          ri->position = position;
          return FALSE;
      }
      node = &ri->hash_table->nodes [position];
  } while(node->key_hash <= 1);
  if (key != NULL) *key = node->key;
  if (value != NULL) *value = node->value;
  ri->position = position;
  return TRUE;
}
GHashTable* g_hash_table_iter_get_hash_table(GHashTableIter *iter) {
  g_return_val_if_fail(iter != NULL, NULL);
  return ((RealIter*)iter)->hash_table;
}
static void iter_remove_or_steal(RealIter *ri, gboolean notify) {
  g_return_if_fail(ri != NULL);
#ifndef G_DISABLE_ASSERT
  g_return_if_fail(ri->version == ri->hash_table->version);
#endif
  g_return_if_fail(ri->position >= 0);
  g_return_if_fail(ri->position < ri->hash_table->size);
  g_hash_table_remove_node(ri->hash_table, &ri->hash_table->nodes[ri->position], notify);
#ifndef G_DISABLE_ASSERT
  ri->version++;
  ri->hash_table->version++;
#endif
}
void g_hash_table_iter_remove(GHashTableIter *iter) {
  iter_remove_or_steal((RealIter*)iter, TRUE);
}
void g_hash_table_iter_steal(GHashTableIter *iter) {
  iter_remove_or_steal((RealIter*)iter, FALSE);
}
GHashTable* g_hash_table_ref(GHashTable *hash_table) {
  g_return_val_if_fail(hash_table != NULL, NULL);
  g_return_val_if_fail(hash_table->ref_count > 0, hash_table);
  g_atomic_int_add(&hash_table->ref_count, 1);
  return hash_table;
}
void g_hash_table_unref(GHashTable *hash_table) {
  g_return_if_fail(hash_table != NULL);
  g_return_if_fail(hash_table->ref_count > 0);
  if (g_atomic_int_exchange_and_add (&hash_table->ref_count, -1) - 1 == 0) {
      g_hash_table_remove_all_nodes(hash_table, TRUE);
      g_free(hash_table->nodes);
      g_slice_free(GHashTable, hash_table);
  }
}
void g_hash_table_destroy(GHashTable *hash_table) {
  g_return_if_fail(hash_table != NULL);
  g_return_if_fail(hash_table->ref_count > 0);
  g_hash_table_remove_all(hash_table);
  g_hash_table_unref(hash_table);
}
gpointer g_hash_table_lookup(GHashTable *hash_table, gconstpointer key) {
  GHashNode *node;
  guint node_index;
  g_return_val_if_fail(hash_table != NULL, NULL);
  node_index = g_hash_table_lookup_node(hash_table, key);
  node = &hash_table->nodes[node_index];
  return node->key_hash ? node->value : NULL;
}
gboolean g_hash_table_lookup_extended(GHashTable *hash_table, gconstpointer lookup_key, gpointer *orig_key, gpointer *value) {
  GHashNode *node;
  guint node_index;
  g_return_val_if_fail(hash_table != NULL, FALSE);
  node_index = g_hash_table_lookup_node(hash_table, lookup_key);
  node = &hash_table->nodes[node_index];
  if (!node->key_hash) return FALSE;
  if (orig_key) *orig_key = node->key;
  if (value) *value = node->value;
  return TRUE;
}
static void g_hash_table_insert_internal(GHashTable *hash_table, gpointer key, gpointer value, gboolean keep_new_key) {
  GHashNode *node;
  guint node_index;
  guint key_hash;
  guint old_hash;
  g_return_if_fail(hash_table != NULL);
  g_return_if_fail(hash_table->ref_count > 0);
  node_index = g_hash_table_lookup_node_for_insertion(hash_table, key, &key_hash);
  node = &hash_table->nodes[node_index];
  old_hash = node->key_hash;
  if (old_hash > 1) {
      if (keep_new_key) {
          if (hash_table->key_destroy_func) hash_table->key_destroy_func(node->key);
          node->key = key;
      } else {
          if (hash_table->key_destroy_func) hash_table->key_destroy_func(key);
      }
      if (hash_table->value_destroy_func) hash_table->value_destroy_func (node->value);
      node->value = value;
  } else {
      node->key = key;
      node->value = value;
      node->key_hash = key_hash;
      hash_table->nnodes++;
      if (old_hash == 0) {
          hash_table->noccupied++;
          g_hash_table_maybe_resize(hash_table);
      }
#ifndef G_DISABLE_ASSERT
      hash_table->version++;
#endif
  }
}
void g_hash_table_insert(GHashTable *hash_table, gpointer key, gpointer value) {
  g_hash_table_insert_internal(hash_table, key, value, FALSE);
}
void g_hash_table_replace(GHashTable *hash_table, gpointer key, gpointer value) {
  g_hash_table_insert_internal(hash_table, key, value, TRUE);
}
static gboolean g_hash_table_remove_internal(GHashTable *hash_table, gconstpointer key, gboolean notify) {
  GHashNode *node;
  guint node_index;
  g_return_val_if_fail(hash_table != NULL, FALSE);
  node_index = g_hash_table_lookup_node (hash_table, key);
  node = &hash_table->nodes[node_index];
  if (!node->key_hash) return FALSE;
  g_hash_table_remove_node (hash_table, node, notify);
  g_hash_table_maybe_resize (hash_table);
#ifndef G_DISABLE_ASSERT
  hash_table->version++;
#endif
  return TRUE;
}
gboolean g_hash_table_remove(GHashTable *hash_table, gconstpointer key) {
  return g_hash_table_remove_internal(hash_table, key, TRUE);
}
gboolean g_hash_table_steal(GHashTable *hash_table, gconstpointer key) {
  return g_hash_table_remove_internal(hash_table, key, FALSE);
}
void g_hash_table_remove_all(GHashTable *hash_table) {
  g_return_if_fail(hash_table != NULL);
#ifndef G_DISABLE_ASSERT
  if (hash_table->nnodes != 0) hash_table->version++;
#endif
  g_hash_table_remove_all_nodes(hash_table, TRUE);
  g_hash_table_maybe_resize(hash_table);
}
void g_hash_table_steal_all(GHashTable *hash_table) {
  g_return_if_fail(hash_table != NULL);
#ifndef G_DISABLE_ASSERT
  if (hash_table->nnodes != 0) hash_table->version++;
#endif
  g_hash_table_remove_all_nodes(hash_table, FALSE);
  g_hash_table_maybe_resize(hash_table);
}
static guint g_hash_table_foreach_remove_or_steal(GHashTable *hash_table, GHRFunc func, gpointer user_data, gboolean notify) {
  guint deleted = 0;
  gint i;
  for (i = 0; i < hash_table->size; i++) {
      GHashNode *node = &hash_table->nodes[i];
      if (node->key_hash > 1 && (*func)(node->key, node->value, user_data)) {
          g_hash_table_remove_node(hash_table, node, notify);
          deleted++;
      }
  }
  g_hash_table_maybe_resize(hash_table);
#ifndef G_DISABLE_ASSERT
  if (deleted > 0) hash_table->version++;
#endif
  return deleted;
}
guint g_hash_table_foreach_remove(GHashTable *hash_table, GHRFunc func, gpointer user_data) {
  g_return_val_if_fail(hash_table != NULL, 0);
  g_return_val_if_fail(func != NULL, 0);
  return g_hash_table_foreach_remove_or_steal(hash_table, func, user_data, TRUE);
}
guint g_hash_table_foreach_steal(GHashTable *hash_table, GHRFunc func, gpointer user_data) {
  g_return_val_if_fail(hash_table != NULL, 0);
  g_return_val_if_fail(func != NULL, 0);
  return g_hash_table_foreach_remove_or_steal(hash_table, func, user_data, FALSE);
}
void g_hash_table_foreach(GHashTable *hash_table, GHFunc func, gpointer user_data) {
  gint i;
  g_return_if_fail(hash_table != NULL);
  g_return_if_fail(func != NULL);
  for (i = 0; i < hash_table->size; i++) {
      GHashNode *node = &hash_table->nodes[i];
      if (node->key_hash > 1) (*func)(node->key, node->value, user_data);
  }
}
gpointer g_hash_table_find(GHashTable *hash_table, GHRFunc predicate, gpointer user_data) {
  gint i;
  g_return_val_if_fail(hash_table != NULL, NULL);
  g_return_val_if_fail(predicate != NULL, NULL);
  for (i = 0; i < hash_table->size; i++) {
      GHashNode *node = &hash_table->nodes[i];
      if (node->key_hash > 1 && predicate(node->key, node->value, user_data)) return node->value;
  }
  return NULL;
}
guint g_hash_table_size(GHashTable *hash_table) {
  g_return_val_if_fail(hash_table != NULL, 0);
  return hash_table->nnodes;
}
GList* g_hash_table_get_keys(GHashTable *hash_table) {
  gint i;
  GList *retval;
  g_return_val_if_fail(hash_table != NULL, NULL);
  retval = NULL;
  for (i = 0; i < hash_table->size; i++) {
      GHashNode *node = &hash_table->nodes[i];
      if (node->key_hash > 1) retval = g_list_prepend(retval, node->key);
  }
  return retval;
}
GList* g_hash_table_get_values(GHashTable *hash_table) {
  gint i;
  GList *retval;
  g_return_val_if_fail(hash_table != NULL, NULL);
  retval = NULL;
  for (i = 0; i < hash_table->size; i++) {
      GHashNode *node = &hash_table->nodes[i];
      if (node->key_hash > 1) retval = g_list_prepend(retval, node->value);
  }
  return retval;
}
