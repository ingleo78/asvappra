#include <string.h>
#include <stdlib.h>
#include "../gio/config.h"
#include "../glib/glib.h"
#include "gvaluearray.h"

#ifdef	DISABLE_MEM_POOLS
#define	GROUP_N_VALUES	(1)
#else
#define	GROUP_N_VALUES	(8)
#endif
GValue* g_value_array_get_nth(GValueArray *value_array, guint index) {
  g_return_val_if_fail(value_array != NULL, NULL);
  g_return_val_if_fail(index < value_array->n_values, NULL);
  return value_array->values + index;
}
static inline void value_array_grow(GValueArray *value_array, guint n_values, gboolean zero_init) {
  g_return_if_fail(n_values >= value_array->n_values);
  value_array->n_values = n_values;
  if (value_array->n_values > value_array->n_prealloced) {
      guint i = value_array->n_prealloced;
      value_array->n_prealloced = (value_array->n_values + GROUP_N_VALUES - 1) & ~(GROUP_N_VALUES - 1);
      value_array->values = g_renew (GValue, value_array->values, value_array->n_prealloced);
      if (!zero_init) i = value_array->n_values;
      memset(value_array->values + i, 0,(value_array->n_prealloced - i) * sizeof (value_array->values[0]));
  }
}
static inline void value_array_shrink(GValueArray *value_array) {
#ifdef  DISABLE_MEM_POOLS
  if (value_array->n_prealloced >= value_array->n_values + GROUP_N_VALUES) {
      value_array->n_prealloced = (value_array->n_values + GROUP_N_VALUES - 1) & ~(GROUP_N_VALUES - 1);
      value_array->values = g_renew(GValue, value_array->values, value_array->n_prealloced);
  }
#endif
}
GValueArray* g_value_array_new(guint n_prealloced) {
  GValueArray *value_array = g_slice_new (GValueArray);
  value_array->n_values = 0;
  value_array->n_prealloced = 0;
  value_array->values = NULL;
  value_array_grow(value_array, n_prealloced, TRUE);
  value_array->n_values = 0;
  return value_array;
}
void g_value_array_free(GValueArray *value_array) {
  guint i;
  g_return_if_fail(value_array != NULL);
  for (i = 0; i < value_array->n_values; i++) {
      GValue *value = value_array->values + i;
      if (G_VALUE_TYPE(value) != 0) g_value_unset(value);
  }
  g_free (value_array->values);
  g_slice_free(GValueArray, value_array);
}
GValueArray* g_value_array_copy(const GValueArray *value_array) {
  GValueArray *new_array;
  guint i;
  g_return_val_if_fail(value_array != NULL, NULL);
  new_array = g_slice_new(GValueArray);
  new_array->n_values = 0;
  new_array->values = NULL;
  new_array->n_prealloced = 0;
  value_array_grow(new_array, value_array->n_values, TRUE);
  for (i = 0; i < new_array->n_values; i++)
      if (G_VALUE_TYPE(value_array->values + i) != 0) {
          GValue *value = new_array->values + i;
          g_value_init(value, G_VALUE_TYPE(value_array->values + i));
          g_value_copy(value_array->values + i, value);
      }
  return new_array;
}
GValueArray* g_value_array_prepend(GValueArray  *value_array, const GValue *value) {
  g_return_val_if_fail(value_array != NULL, NULL);
  return g_value_array_insert(value_array, 0, value);
}
GValueArray* g_value_array_append(GValueArray  *value_array, const GValue *value) {
  g_return_val_if_fail(value_array != NULL, NULL);
  return g_value_array_insert(value_array, value_array->n_values, value);
}
GValueArray* g_value_array_insert(GValueArray  *value_array, guint index, const GValue *value) {
  guint i;
  g_return_val_if_fail(value_array != NULL, NULL);
  g_return_val_if_fail(index <= value_array->n_values, value_array);
  i = value_array->n_values;
  value_array_grow(value_array, value_array->n_values + 1, FALSE);
  if (index + 1 < value_array->n_values) {
      g_memmove(value_array->values + index + 1, value_array->values + index,(i - index) * sizeof(value_array->values[0]));
  }
  memset(value_array->values + index, 0, sizeof (value_array->values[0]));
  if (value) {
      g_value_init(value_array->values + index, G_VALUE_TYPE(value));
      g_value_copy(value, value_array->values + index);
  }
  return value_array;
}
GValueArray* g_value_array_remove(GValueArray *value_array, guint index) {
  g_return_val_if_fail(value_array != NULL, NULL);
  g_return_val_if_fail(index < value_array->n_values, value_array);
  if (G_VALUE_TYPE(value_array->values + index) != 0) g_value_unset(value_array->values + index);
  value_array->n_values--;
  if (index < value_array->n_values) {
      g_memmove(value_array->values + index, value_array->values + index + 1,(value_array->n_values - index) * sizeof(value_array->values[0]));
  }
  value_array_shrink (value_array);
  if (value_array->n_prealloced > value_array->n_values) memset(value_array->values + value_array->n_values, 0, sizeof(value_array->values[0]));
  return value_array;
}
GValueArray* g_value_array_sort(GValueArray *value_array, GCompareFunc compare_func) {
  g_return_val_if_fail(compare_func != NULL, NULL);
  if (value_array->n_values) qsort (value_array->values, value_array->n_values, sizeof(value_array->values[0]), compare_func);
  return value_array;
}
GValueArray* g_value_array_sort_with_data (GValueArray *value_array, GCompareDataFunc compare_func, gpointer user_data) {
  g_return_val_if_fail(value_array != NULL, NULL);
  g_return_val_if_fail(compare_func != NULL, NULL);
  if (value_array->n_values) g_qsort_with_data(value_array->values, value_array->n_values,sizeof(value_array->values[0]), compare_func, user_data);
  return value_array;
}