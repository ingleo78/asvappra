#include <string.h>
#include "../gio/config.h"
#include "gvalue.h"
#include "gvaluecollector.h"
#include "gbsearcharray.h"

typedef struct {
  GType src_type;
  GType dest_type;
  GValueTransform func;
} TransformEntry;
static gint	transform_entries_cmp(gconstpointer bsearch_node1, gconstpointer bsearch_node2);
static GBSearchArray *transform_array = NULL;
static GBSearchConfig transform_bconfig = {
  sizeof(TransformEntry),
  transform_entries_cmp,
  G_BSEARCH_ARRAY_ALIGN_POWER2
};
void g_value_c_init(void) {
  transform_array = g_bsearch_array_create(&transform_bconfig);
}
static inline void value_meminit(GValue *value, GType value_type) {
  value->g_type = value_type;
  memset(value->data, 0, sizeof(value->data));
}
GValue* g_value_init(GValue *value, GType g_type) {
  g_return_val_if_fail(value != NULL, NULL);
  if (G_TYPE_IS_VALUE(g_type) && G_VALUE_TYPE(value) == 0) {
      GTypeValueTable *value_table = g_type_value_table_peek (g_type);
      value_meminit(value, g_type);
      value_table->value_init(value);
  } else if (G_VALUE_TYPE(value)) {
      g_warning("%s: cannot initialize GValue with type `%s', the value has already been initialized as `%s'", G_STRLOC, g_type_name(g_type),
                g_type_name(G_VALUE_TYPE(value)));
  }
  else {
      g_warning("%s: cannot initialize GValue with type `%s', %s", G_STRLOC, g_type_name(g_type), g_type_value_table_peek(g_type) ? "this type is "
                "abstract with regards to GValue use, use a more specific (derived) type" : "this type has no GTypeValueTable implementation");
  }
  return value;
}
void g_value_copy(const GValue *src_value, GValue *dest_value) {
  g_return_if_fail(G_IS_VALUE(src_value));
  g_return_if_fail(G_IS_VALUE(dest_value));
  g_return_if_fail(g_value_type_compatible (G_VALUE_TYPE(src_value), G_VALUE_TYPE(dest_value)));
  if (src_value != dest_value) {
      GType dest_type = G_VALUE_TYPE(dest_value);
      GTypeValueTable *value_table = g_type_value_table_peek(dest_type);
      if (value_table->value_free) value_table->value_free(dest_value);
      value_meminit(dest_value, dest_type);
      value_table->value_copy(src_value, dest_value);
    }
}
GValue* g_value_reset(GValue *value) {
  GTypeValueTable *value_table;
  GType g_type;
  g_return_val_if_fail(G_IS_VALUE(value), NULL);
  g_type = G_VALUE_TYPE(value);
  value_table = g_type_value_table_peek(g_type);
  if (value_table->value_free) value_table->value_free(value);
  value_meminit(value, g_type);
  value_table->value_init(value);
  return value;
}
void g_value_unset(GValue *value) {
  GTypeValueTable *value_table;
  g_return_if_fail(G_IS_VALUE(value));
  value_table = g_type_value_table_peek(G_VALUE_TYPE(value));
  if (value_table->value_free) value_table->value_free(value);
  memset(value, 0, sizeof(*value));
}
gboolean g_value_fits_pointer(const GValue *value) {
  GTypeValueTable *value_table;
  g_return_val_if_fail(G_IS_VALUE(value), FALSE);
  value_table = g_type_value_table_peek(G_VALUE_TYPE(value));
  return value_table->value_peek_pointer != NULL;
}
gpointer g_value_peek_pointer(const GValue *value) {
  GTypeValueTable *value_table;
  g_return_val_if_fail(G_IS_VALUE(value), NULL);
  value_table = g_type_value_table_peek(G_VALUE_TYPE(value));
  if (!value_table->value_peek_pointer) {
      g_return_val_if_fail(g_value_fits_pointer(value) == TRUE, NULL);
      return NULL;
  }
  return value_table->value_peek_pointer(value);
}
void g_value_set_instance(GValue *value, gpointer instance) {
  GType g_type;
  GTypeValueTable *value_table;
  GTypeCValue cvalue;
  gchar *error_msg;
  g_return_if_fail(G_IS_VALUE(value));
  if (instance) {
      g_return_if_fail(G_TYPE_CHECK_INSTANCE(instance));
      g_return_if_fail(g_value_type_compatible(G_TYPE_FROM_INSTANCE(instance), G_VALUE_TYPE(value)));
  }
  g_type = G_VALUE_TYPE(value);
  value_table = g_type_value_table_peek(g_type);
  g_return_if_fail(strcmp(value_table->collect_format, "p") == 0);
  memset(&cvalue, 0, sizeof(cvalue));
  cvalue.v_pointer = instance;
  if (value_table->value_free) value_table->value_free(value);
  value_meminit(value, g_type);
  error_msg = value_table->collect_value(value, 1, &cvalue, 0);
  if (error_msg) {
      g_warning("%s: %s", G_STRLOC, error_msg);
      g_free(error_msg);
      value_meminit(value, g_type);
      value_table->value_init(value);
    }
}
static GValueTransform transform_func_lookup(GType src_type, GType dest_type) {
  TransformEntry entry;
  entry.src_type = src_type;
  do {
      entry.dest_type = dest_type;
      do {
          TransformEntry *e;
          e = g_bsearch_array_lookup(transform_array, &transform_bconfig, &entry);
          if (e) {
              if (g_type_value_table_peek(entry.dest_type) == g_type_value_table_peek(dest_type) &&
                  g_type_value_table_peek(entry.src_type) == g_type_value_table_peek(src_type)) {
                  return e->func;
              }
          }
          entry.dest_type = g_type_parent(entry.dest_type);
	  } while(entry.dest_type);
      entry.src_type = g_type_parent(entry.src_type);
  } while(entry.src_type);
  return NULL;
}
static gint transform_entries_cmp(gconstpointer bsearch_node1, gconstpointer bsearch_node2) {
  const TransformEntry *e1 = bsearch_node1;
  const TransformEntry *e2 = bsearch_node2;
  gint cmp = G_BSEARCH_ARRAY_CMP(e1->src_type, e2->src_type);
  if (cmp) return cmp;
  else return G_BSEARCH_ARRAY_CMP(e1->dest_type, e2->dest_type);
}
void g_value_register_transform_func(GType src_type, GType dest_type, GValueTransform transform_func) {
  TransformEntry entry;
  g_return_if_fail(transform_func != NULL);
  entry.src_type = src_type;
  entry.dest_type = dest_type;
#if 0
  if (g_bsearch_array_lookup(transform_array, &transform_bconfig, &entry))
      g_warning("reregistering value transformation function(%p) for `%s' to `%s'", transform_func, g_type_name(src_type), g_type_name(dest_type));
#endif
  entry.func = transform_func;
  transform_array = g_bsearch_array_replace(transform_array, &transform_bconfig, &entry);
}
gboolean g_value_type_transformable(GType src_type, GType dest_type) {
  g_return_val_if_fail(G_TYPE_IS_VALUE(src_type), FALSE);
  g_return_val_if_fail(G_TYPE_IS_VALUE(dest_type), FALSE);
  return (g_value_type_compatible(src_type, dest_type) || transform_func_lookup(src_type, dest_type) != NULL);
}
gboolean g_value_type_compatible(GType src_type, GType dest_type) {
  g_return_val_if_fail(G_TYPE_IS_VALUE(src_type), FALSE);
  g_return_val_if_fail(G_TYPE_IS_VALUE(dest_type), FALSE);
  return (g_type_is_a(src_type, dest_type) && g_type_value_table_peek(dest_type) == g_type_value_table_peek(src_type));
}
gboolean g_value_transform(const GValue *src_value, GValue  *dest_value) {
  GType dest_type;
  g_return_val_if_fail(G_IS_VALUE(src_value), FALSE);
  g_return_val_if_fail(G_IS_VALUE(dest_value), FALSE);
  dest_type = G_VALUE_TYPE(dest_value);
  if (g_value_type_compatible(G_VALUE_TYPE(src_value), dest_type)) {
      g_value_copy(src_value, dest_value);
      return TRUE;
  } else {
      GValueTransform transform = transform_func_lookup(G_VALUE_TYPE(src_value), dest_type);
      if (transform) {
          g_value_unset(dest_value);
          value_meminit(dest_value, dest_type);
          transform(src_value, dest_value);
          return TRUE;
	  }
  }
  return FALSE;
}