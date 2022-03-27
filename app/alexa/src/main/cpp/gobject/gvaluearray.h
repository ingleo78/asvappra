#if defined (__GLIB_GOBJECT_H_INSIDE__) && defined (GOBJECT_COMPILATION)
//#error "Only <glib-object.h> can be included directly."
#endif

#ifndef __G_VALUE_ARRAY_H__
#define __G_VALUE_ARRAY_H__

#include "gvalue.h"

G_BEGIN_DECLS
typedef struct _GValueArray GValueArray;
struct _GValueArray {
  guint n_values;
  GValue *values;
  guint n_prealloced;
};
GValue*g_value_array_get_nth(GValueArray *value_array, guint index_);
GValueArray* g_value_array_new(guint n_prealloced);
void g_value_array_free(GValueArray	*value_array);
GValueArray* g_value_array_copy(const GValueArray *value_array);
GValueArray* g_value_array_prepend(GValueArray *value_array, const GValue *value);
GValueArray* g_value_array_append(GValueArray *value_array, const GValue *value);
GValueArray* g_value_array_insert(GValueArray *value_array, guint index_, const GValue *value);
GValueArray* g_value_array_remove(GValueArray *value_array, guint index_);
GValueArray* g_value_array_sort(GValueArray	*value_array, GCompareFunc compare_func);
GValueArray* g_value_array_sort_with_data(GValueArray *value_array, GCompareDataFunc compare_func, gpointer user_data);
G_END_DECLS
#endif /* __G_VALUE_ARRAY_H__ */