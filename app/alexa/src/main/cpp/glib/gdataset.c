#include <string.h>
#include "gdataset.h"
#include "gdatasetprivate.h"
#include "ghash.h"
#include "gquark.h"
#include "gstrfuncs.h"
#include "gtestutils.h"
#include "gthread.h"
#include "glib_trace.h"

#define	G_QUARK_BLOCK_SIZE (512)
#define G_DATALIST_GET_POINTER(datalist)  ((GData*)((gsize) g_atomic_pointer_get (datalist) & ~(gsize) G_DATALIST_FLAGS_MASK))
#define G_DATALIST_SET_POINTER(datalist, pointer)                                         \
G_STMT_START {                                                                            \
    gpointer _oldv;                                                                       \
    gpointer _newv;                                                                       \
    do {                                                                                  \
        _oldv = (void*)g_atomic_pointer_get(datalist);                                           \
        _newv = (void*)(((gsize) _oldv & G_DATALIST_FLAGS_MASK) | (gsize)(gpointer)pointer);    \
    } while(!g_atomic_pointer_compare_and_exchange((void**)datalist, _oldv, _newv));     \
} G_STMT_END
typedef struct _GDataset GDataset;
struct _GData {
  GData *next;
  GQuark id;
  gpointer data;
  GDestroyNotify destroy_func;
};
struct _GDataset {
  gconstpointer location;
  GData *datalist;
};
static inline GDataset*	g_dataset_lookup(gconstpointer dataset_location);
static inline void	g_datalist_clear_i(GData **datalist);
static void	g_dataset_destroy_internal(GDataset *dataset);
static inline gpointer g_data_set_internal(GData **datalist, GQuark key_id, gpointer data, GDestroyNotify destroy_func, GDataset *dataset);
static void	g_data_initialize(void);
static inline GQuark g_quark_new(gchar *string);
G_LOCK_DEFINE_STATIC (g_dataset_global);
static GHashTable *g_dataset_location_ht = NULL;
static GDataset *g_dataset_cached = NULL;
G_LOCK_DEFINE_STATIC (g_quark_global);
static GHashTable *g_quark_ht = NULL;
static gchar **g_quarks = NULL;
static GQuark g_quark_seq_id = 0;
static inline void g_datalist_clear_i(GData **datalist) {
  register GData *list;
  list = G_DATALIST_GET_POINTER(datalist);
  datalist = NULL;
  G_DATALIST_SET_POINTER(datalist, NULL);
  while(list) {
      register GData *prev;
      prev = list;
      list = prev->next;
      if (prev->destroy_func) {
          G_UNLOCK(g_dataset_global);
          prev->destroy_func(prev->data);
          G_LOCK(g_dataset_global);
	  }
      g_slice_free(GData, prev);
  }
}
void g_datalist_clear(GData **datalist) {
  g_return_if_fail(datalist != NULL);
  G_LOCK(g_dataset_global);
  if (!g_dataset_location_ht) g_data_initialize();
  while(G_DATALIST_GET_POINTER(datalist)) g_datalist_clear_i(datalist);
  G_UNLOCK(g_dataset_global);
}
static inline GDataset* g_dataset_lookup(gconstpointer dataset_location) {
  register GDataset *dataset;
  if (g_dataset_cached && g_dataset_cached->location == dataset_location) return g_dataset_cached;
  dataset = g_hash_table_lookup(g_dataset_location_ht, dataset_location);
  if (dataset) g_dataset_cached = dataset;
  return dataset;
}
static void g_dataset_destroy_internal(GDataset *dataset) {
  register gconstpointer dataset_location;
  dataset_location = dataset->location;
  while(dataset) {
      if (!dataset->datalist) {
          if (dataset == g_dataset_cached) g_dataset_cached = NULL;
          g_hash_table_remove(g_dataset_location_ht, dataset_location);
          g_slice_free(GDataset, dataset);
          break;
	  }
      g_datalist_clear_i(&dataset->datalist);
      dataset = g_dataset_lookup(dataset_location);
  }
}
void g_dataset_destroy(gconstpointer dataset_location) {
  g_return_if_fail(dataset_location != NULL);
  G_LOCK(g_dataset_global);
  if (g_dataset_location_ht) {
      register GDataset *dataset;
      dataset = g_dataset_lookup(dataset_location);
      if (dataset) g_dataset_destroy_internal(dataset);
  }
  G_UNLOCK(g_dataset_global);
}
static inline gpointer g_data_set_internal(GData **datalist, GQuark key_id, gpointer data, GDestroyNotify destroy_func, GDataset *dataset) {
  register GData *list;
  list = G_DATALIST_GET_POINTER (datalist);
  if (!data) {
      register GData *prev;
      prev = NULL;
      while(list) {
          if (list->id == key_id) {
              gpointer ret_data = NULL;
              if (prev) prev->next = list->next;
              else {
                  G_DATALIST_SET_POINTER(datalist, list->next);
                  if (!list->next && dataset) g_dataset_destroy_internal (dataset);
              }
              if (list->destroy_func && !destroy_func) {
                  G_UNLOCK(g_dataset_global);
                  list->destroy_func(list->data);
                  G_LOCK(g_dataset_global);
              } else ret_data = list->data;
              g_slice_free(GData, list);
              return ret_data;
          }
          prev = list;
          list = list->next;
	  }
  } else {
      while(list) {
          if (list->id == key_id) {
              if (!list->destroy_func) {
                  list->data = data;
                  list->destroy_func = destroy_func;
              } else {
                  register GDestroyNotify dfunc;
                  register gpointer ddata;
                  dfunc = list->destroy_func;
                  ddata = list->data;
                  list->data = data;
                  list->destroy_func = destroy_func;
                  G_UNLOCK(g_dataset_global);
                  dfunc(ddata);
                  G_LOCK(g_dataset_global);
              }
              return NULL;
          }
          list = list->next;
	  }
      list = g_slice_new(GData);
      list->next = G_DATALIST_GET_POINTER(datalist);
      list->id = key_id;
      list->data = data;
      list->destroy_func = destroy_func;
      datalist = list;
      G_DATALIST_SET_POINTER(datalist, list);
  }
  return NULL;
}
void g_dataset_id_set_data_full(gconstpointer dataset_location, GQuark key_id, gpointer data, GDestroyNotify destroy_func) {
  register GDataset *dataset;
  g_return_if_fail(dataset_location != NULL);
  if (!data) g_return_if_fail(destroy_func == NULL);
  if (!key_id) {
      if (data) {
          g_return_if_fail(key_id > 0);
      } else return;
  }
  G_LOCK(g_dataset_global);
  if (!g_dataset_location_ht) g_data_initialize();
  dataset = g_dataset_lookup(dataset_location);
  if (!dataset) {
      dataset = g_slice_new(GDataset);
      dataset->location = dataset_location;
      g_datalist_init(&dataset->datalist);
      g_hash_table_insert(g_dataset_location_ht, (gpointer)dataset->location, dataset);
  }
  g_data_set_internal(&dataset->datalist, key_id, data, destroy_func, dataset);
  G_UNLOCK(g_dataset_global);
}
void g_datalist_id_set_data_full(GData **datalist, GQuark key_id, gpointer data, GDestroyNotify destroy_func) {
  g_return_if_fail(datalist != NULL);
  if (!data) g_return_if_fail(destroy_func == NULL);
  if (!key_id) {
      if (data) {
          g_return_if_fail(key_id > 0);
      } else return;
  }
  G_LOCK(g_dataset_global);
  if (!g_dataset_location_ht) g_data_initialize();
  g_data_set_internal(datalist, key_id, data, destroy_func, NULL);
  G_UNLOCK(g_dataset_global);
}
gpointer g_dataset_id_remove_no_notify(gconstpointer dataset_location, GQuark key_id) {
  gpointer ret_data = NULL;
  g_return_val_if_fail(dataset_location != NULL, NULL);
  G_LOCK(g_dataset_global);
  if (key_id && g_dataset_location_ht) {
      GDataset *dataset;
      dataset = g_dataset_lookup(dataset_location);
      if (dataset) ret_data = g_data_set_internal(&dataset->datalist, key_id, NULL, (GDestroyNotify)42, dataset);
  }
  G_UNLOCK(g_dataset_global);
  return ret_data;
}
gpointer g_datalist_id_remove_no_notify(GData **datalist, GQuark key_id) {
  gpointer ret_data = NULL;
  g_return_val_if_fail(datalist != NULL, NULL);
  G_LOCK(g_dataset_global);
  if (key_id && g_dataset_location_ht) ret_data = g_data_set_internal(datalist, key_id, NULL, (GDestroyNotify)42, NULL);
  G_UNLOCK(g_dataset_global);
  return ret_data;
}
gpointer g_dataset_id_get_data(gconstpointer dataset_location, GQuark key_id) {
  g_return_val_if_fail(dataset_location != NULL, NULL);
  G_LOCK(g_dataset_global);
  if (key_id && g_dataset_location_ht) {
      register GDataset *dataset;
      dataset = g_dataset_lookup(dataset_location);
      if (dataset) {
	  register GData *list;
          for (list = dataset->datalist; list; list = list->next)
              if (list->id == key_id) {
                  G_UNLOCK(g_dataset_global);
                  return list->data;
              }
	  }
  }
  G_UNLOCK(g_dataset_global);
  return NULL;
}
gpointer g_datalist_id_get_data(GData **datalist, GQuark key_id) {
  gpointer data = NULL;
  g_return_val_if_fail(datalist != NULL, NULL);
  if (key_id) {
      register GData *list;
      G_LOCK(g_dataset_global);
      for (list = G_DATALIST_GET_POINTER(datalist); list; list = list->next)
	      if (list->id == key_id) {
              data = list->data;
              break;
          }
      G_UNLOCK(g_dataset_global);
  }
  return data;
}
void g_dataset_foreach(gconstpointer dataset_location, GDataForeachFunc func, gpointer user_data) {
  register GDataset *dataset;
  g_return_if_fail(dataset_location != NULL);
  g_return_if_fail(func != NULL);
  G_LOCK(g_dataset_global);
  if (g_dataset_location_ht) {
      dataset = g_dataset_lookup(dataset_location);
      G_UNLOCK(g_dataset_global);
      if (dataset) {
          register GData *list, *next;
          for (list = dataset->datalist; list; list = next) {
              next = list->next;
              func(list->id, list->data, user_data);
          }
	  }
  } else {
      G_UNLOCK(g_dataset_global);
  }
}
void g_datalist_foreach(GData **datalist, GDataForeachFunc func, gpointer user_data) {
  register GData *list, *next;
  g_return_if_fail(datalist != NULL);
  g_return_if_fail(func != NULL);
  for (list = G_DATALIST_GET_POINTER(datalist); list; list = next) {
      next = list->next;
      func(list->id, list->data, user_data);
  }
}
void g_datalist_init(GData **datalist) {
  g_return_if_fail(datalist != NULL);
  g_atomic_pointer_set(datalist, NULL);
}
void g_datalist_set_flags(GData **datalist, guint flags) {
  gpointer oldvalue;
  g_return_if_fail(datalist != NULL);
  g_return_if_fail((flags & ~G_DATALIST_FLAGS_MASK) == 0);
  do {
      oldvalue = g_atomic_pointer_get(datalist);
  } while(!g_atomic_pointer_compare_and_exchange((void**)datalist, oldvalue, (gpointer)((gsize)oldvalue | flags)));
}
void g_datalist_unset_flags(GData **datalist, guint flags) {
  gpointer oldvalue;
  g_return_if_fail(datalist != NULL);
  g_return_if_fail((flags & ~G_DATALIST_FLAGS_MASK) == 0);
  do {
      oldvalue = g_atomic_pointer_get(datalist);
  } while(!g_atomic_pointer_compare_and_exchange((void**)datalist, oldvalue, (gpointer)((gsize)oldvalue & ~(gsize)flags)));
}
guint g_datalist_get_flags(GData **datalist) {
  g_return_val_if_fail(datalist != NULL, 0);
  return G_DATALIST_GET_FLAGS(datalist);
}
static void g_data_initialize(void) {
  g_return_if_fail(g_dataset_location_ht == NULL);
  g_dataset_location_ht = g_hash_table_new(g_direct_hash, NULL);
  g_dataset_cached = NULL;
}
GQuark g_quark_try_string(const gchar *string) {
  GQuark quark = 0;
  if (string == NULL) return 0;
  G_LOCK(g_quark_global);
  if (g_quark_ht) quark = GPOINTER_TO_UINT(g_hash_table_lookup(g_quark_ht, string));
  G_UNLOCK(g_quark_global);
  return quark;
}
#define QUARK_STRING_BLOCK_SIZE (4096 - sizeof (gsize))
static char *quark_block = NULL;
static int quark_block_offset = 0;
static char* quark_strdup(const gchar *string) {
  gchar *copy;
  gsize len;
  len = strlen(string) + 1;
  if (len > QUARK_STRING_BLOCK_SIZE / 2) return g_strdup(string);
  if (quark_block == NULL || QUARK_STRING_BLOCK_SIZE - quark_block_offset < len) {
      quark_block = g_malloc(QUARK_STRING_BLOCK_SIZE);
      quark_block_offset = 0;
  }
  copy = quark_block + quark_block_offset;
  memcpy(copy, string, len);
  quark_block_offset += len;
  return copy;
}
static inline GQuark g_quark_from_string_internal(const gchar *string, gboolean duplicate) {
  GQuark quark = 0;
  if (g_quark_ht) quark = GPOINTER_TO_UINT(g_hash_table_lookup(g_quark_ht, string));
  if (!quark) {
      quark = g_quark_new(duplicate ? quark_strdup (string) : (gchar *)string);
      TRACE(GLIB_QUARK_NEW(string, quark));
  }
  return quark;
}
GQuark g_quark_from_string(const gchar *string) {
  GQuark quark;
  if (!string) return 0;
  G_LOCK(g_quark_global);
  quark = g_quark_from_string_internal(string, TRUE);
  G_UNLOCK(g_quark_global);
  return quark;
}
GQuark g_quark_from_static_string(const gchar *string) {
  GQuark quark;
  if (!string) return 0;
  G_LOCK(g_quark_global);
  quark = g_quark_from_string_internal(string, FALSE);
  G_UNLOCK(g_quark_global);
  return quark;
}
G_CONST_RETURN gchar* g_quark_to_string(GQuark quark) {
  gchar* result = NULL;
  G_LOCK(g_quark_global);
  if (quark < g_quark_seq_id) result = g_quarks[quark];
  G_UNLOCK(g_quark_global);
  return result;
}
static inline GQuark g_quark_new(gchar *string) {
  GQuark quark;
  if (g_quark_seq_id % G_QUARK_BLOCK_SIZE == 0) g_quarks = g_renew(gchar*, g_quarks, g_quark_seq_id + G_QUARK_BLOCK_SIZE);
  if (!g_quark_ht) {
      g_assert(g_quark_seq_id == 0);
      g_quark_ht = g_hash_table_new(g_str_hash, g_str_equal);
      g_quarks[g_quark_seq_id++] = NULL;
  }
  quark = g_quark_seq_id++;
  g_quarks[quark] = string;
  g_hash_table_insert(g_quark_ht, string, GUINT_TO_POINTER(quark));
  return quark;
}
G_CONST_RETURN gchar* g_intern_string(const gchar *string) {
  const gchar *result;
  GQuark quark;
  if (!string) return NULL;
  G_LOCK(g_quark_global);
  quark = g_quark_from_string_internal(string, TRUE);
  result = g_quarks[quark];
  G_UNLOCK(g_quark_global);
  return result;
}
G_CONST_RETURN gchar* g_intern_static_string(const gchar *string) {
  GQuark quark;
  const gchar *result;
  if (!string) return NULL;
  G_LOCK(g_quark_global);
  quark = g_quark_from_string_internal(string, FALSE);
  result = g_quarks[quark];
  G_UNLOCK(g_quark_global);
  return result;
}