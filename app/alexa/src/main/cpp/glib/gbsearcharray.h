#ifndef __G_BSEARCH_ARRAY_H__
#define __G_BSEARCH_ARRAY_H__

#include <string.h>
#include "glib.h"
#include "gmacros.h"
#include "gtypes.h"

G_BEGIN_DECLS
#define G_BSEARCH_ARRAY_CMP(v1,v2) ((v1) > (v2) ? +1 : (v1) == (v2) ? 0 : -1)
typedef gint (*GBSearchCompareFunc)(gconstpointer bsearch_node1, gconstpointer bsearch_node2);
typedef enum {
  G_BSEARCH_ARRAY_ALIGN_POWER2  = 1 << 0,
  G_BSEARCH_ARRAY_AUTO_SHRINK  = 1 << 1
} GBSearchArrayFlags;
typedef struct {
  guint sizeof_node;
  GBSearchCompareFunc cmp_nodes;
  guint flags;
} GBSearchConfig;
typedef union {
  guint n_nodes;
  gpointer alignment_dummy1;
  glong alignment_dummy2;
  gdouble alignment_dummy3;
} GBSearchArray;
static inline GBSearchArray* g_bsearch_array_create(const GBSearchConfig *bconfig);
static inline gpointer g_bsearch_array_get_nth(GBSearchArray *barray, const GBSearchConfig *bconfig, guint nth);
static inline guint g_bsearch_array_get_index(GBSearchArray *barray, const GBSearchConfig *bconfig, gconstpointer node_in_array);
static inline GBSearchArray* g_bsearch_array_remove(GBSearchArray *barray, const GBSearchConfig *bconfig, guint index_);
static inline GBSearchArray* g_bsearch_array_grow(GBSearchArray *barray, const GBSearchConfig *bconfig, guint index);
static inline GBSearchArray* g_bsearch_array_insert(GBSearchArray *barray, const GBSearchConfig *bconfig, gconstpointer key_node);
static inline GBSearchArray* g_bsearch_array_replace(GBSearchArray *barray, const GBSearchConfig *bconfig, gconstpointer key_node);
static inline void g_bsearch_array_free(GBSearchArray *barray, const GBSearchConfig *bconfig);
#define g_bsearch_array_get_n_nodes(barray)  (((GBSearchArray*) (barray))->n_nodes)
#define g_bsearch_array_lookup(barray, bconfig, key_node)  g_bsearch_array_lookup_fuzzy((barray), (bconfig), (key_node), 0)
#define g_bsearch_array_lookup_sibling(barray, bconfig, key_node)  g_bsearch_array_lookup_fuzzy((barray), (bconfig), (key_node), 1)
#define g_bsearch_array_lookup_insertion(barray, bconfig, key_node)  g_bsearch_array_lookup_fuzzy((barray), (bconfig), (key_node), 2)
#ifdef  DISABLE_MEM_POOLS
#define G_BSEARCH_UPPER_POWER2(n) (n)
#else
#define G_BSEARCH_UPPER_POWER2(n)  ((n) ? 1 << g_bit_storage((n) - 1) : 0)
#endif
#define G_BSEARCH_ARRAY_NODES(barray)  (((guint8*)(barray)) + sizeof(GBSearchArray))
static inline GBSearchArray* g_bsearch_array_create(const GBSearchConfig *bconfig) {
  GBSearchArray *barray;
  guint size;
  g_return_val_if_fail(bconfig != NULL, NULL);
  size = sizeof(GBSearchArray) + bconfig->sizeof_node;
  if (bconfig->flags & G_BSEARCH_ARRAY_ALIGN_POWER2) size = G_BSEARCH_UPPER_POWER2(size);
  barray = (GBSearchArray *)g_malloc(size);
  memset(barray, 0, sizeof(GBSearchArray));
  return barray;
}
static inline gpointer g_bsearch_array_lookup_fuzzy(GBSearchArray *barray, const GBSearchConfig *bconfig, gconstpointer key_node, const guint sibling_or_after);
static inline gpointer g_bsearch_array_lookup_fuzzy(GBSearchArray *barray, const GBSearchConfig *bconfig, gconstpointer key_node, const guint sibling_or_after) {
  GBSearchCompareFunc cmp_nodes = bconfig->cmp_nodes;
  guint8 *check = NULL, *nodes = G_BSEARCH_ARRAY_NODES(barray);
  guint n_nodes = barray->n_nodes, offs = 0;
  guint sizeof_node = bconfig->sizeof_node;
  gint cmp = 0;
  while(offs < n_nodes) {
      guint i = (offs + n_nodes) >> 1;
      check = nodes + i * sizeof_node;
      cmp = cmp_nodes(key_node, check);
      if (cmp == 0) return sibling_or_after > 1 ? NULL : check;
      else if (cmp < 0) n_nodes = i;
      else offs = i + 1;
  }
  return G_LIKELY(!sibling_or_after) ? NULL : (sibling_or_after > 1 && cmp > 0) ? check + sizeof_node : check;
}
static inline gpointer g_bsearch_array_get_nth(GBSearchArray *barray, const GBSearchConfig *bconfig, guint nth) {
  return (G_LIKELY(nth < barray->n_nodes) ? G_BSEARCH_ARRAY_NODES(barray) + nth * bconfig->sizeof_node : NULL);
}
static inline guint g_bsearch_array_get_index(GBSearchArray *barray, const GBSearchConfig *bconfig, gconstpointer node_in_array) {
  guint distance = ((guint8*)node_in_array) - G_BSEARCH_ARRAY_NODES(barray);
  g_return_val_if_fail(node_in_array != NULL, barray->n_nodes);
  distance /= bconfig->sizeof_node;
  return MIN (distance, barray->n_nodes + 1);
}
static inline GBSearchArray* g_bsearch_array_grow(GBSearchArray *barray, const GBSearchConfig *bconfig, guint index_) {
  guint old_size = barray->n_nodes * bconfig->sizeof_node;
  guint new_size = old_size + bconfig->sizeof_node;
  guint8 *node;
  g_return_val_if_fail(index_ <= barray->n_nodes, NULL);
  if (G_UNLIKELY(bconfig->flags & G_BSEARCH_ARRAY_ALIGN_POWER2)) {
      new_size = G_BSEARCH_UPPER_POWER2(sizeof(GBSearchArray) + new_size);
      old_size = G_BSEARCH_UPPER_POWER2(sizeof(GBSearchArray) + old_size);
      if (old_size != new_size) barray = (GBSearchArray*) g_realloc(barray, new_size);
  } else barray = (GBSearchArray*)g_realloc(barray, sizeof(GBSearchArray) + new_size);
  node = G_BSEARCH_ARRAY_NODES(barray) + index_ * bconfig->sizeof_node;
  g_memmove(node + bconfig->sizeof_node, node, (barray->n_nodes - index_) * bconfig->sizeof_node);
  barray->n_nodes += 1;
  return barray;
}
static inline GBSearchArray* g_bsearch_array_insert(GBSearchArray *barray, const GBSearchConfig *bconfig, gconstpointer key_node) {
  guint8 *node;
  if (G_UNLIKELY(!barray->n_nodes)) {
      barray = g_bsearch_array_grow(barray, bconfig, 0);
      node = G_BSEARCH_ARRAY_NODES(barray);
  } else {
      node = (guint8*)g_bsearch_array_lookup_insertion(barray, bconfig, key_node);
      if (G_LIKELY(node)) {
          guint index_ = g_bsearch_array_get_index(barray, bconfig, node);
          barray = g_bsearch_array_grow(barray, bconfig, index_);
          node = G_BSEARCH_ARRAY_NODES(barray) + index_ * bconfig->sizeof_node;
      } else return barray;
  }
  memcpy(node, key_node, bconfig->sizeof_node);
  return barray;
}
static inline GBSearchArray* g_bsearch_array_replace(GBSearchArray *barray, const GBSearchConfig *bconfig, gconstpointer key_node) {
  guint8 *node = (guint8*)g_bsearch_array_lookup(barray, bconfig, key_node);
  if (G_LIKELY(node)) memcpy(node, key_node, bconfig->sizeof_node);
  else barray = g_bsearch_array_insert(barray, bconfig, key_node);
  return barray;
}
static inline GBSearchArray* g_bsearch_array_remove(GBSearchArray *barray, const GBSearchConfig *bconfig, guint index_) {
  guint8 *node;
  g_return_val_if_fail(index_ < barray->n_nodes, NULL);
  barray->n_nodes -= 1;
  node = G_BSEARCH_ARRAY_NODES (barray) + index_ * bconfig->sizeof_node;
  g_memmove(node, node + bconfig->sizeof_node, (barray->n_nodes - index_) * bconfig->sizeof_node);
  if (G_UNLIKELY(bconfig->flags & G_BSEARCH_ARRAY_AUTO_SHRINK)) {
      guint new_size = barray->n_nodes * bconfig->sizeof_node;
      guint old_size = new_size + bconfig->sizeof_node;
      if (G_UNLIKELY(bconfig->flags & G_BSEARCH_ARRAY_ALIGN_POWER2)) {
          new_size = G_BSEARCH_UPPER_POWER2(sizeof(GBSearchArray) + new_size);
          old_size = G_BSEARCH_UPPER_POWER2(sizeof(GBSearchArray) + old_size);
          if (old_size != new_size) barray = (GBSearchArray*)g_realloc(barray, new_size);
      } else barray = (GBSearchArray*)g_realloc(barray, sizeof(GBSearchArray) + new_size);
  }
  return barray;
}
static inline void g_bsearch_array_free(GBSearchArray *barray, const GBSearchConfig *bconfig) {
  g_return_if_fail(barray != NULL);
  g_free(barray);
}
G_END_DECLS

#endif