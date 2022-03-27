#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_ARRAY_H__
#define __G_ARRAY_H__

#include "gmacros.h"
#include "gtypes.h"
#include "glib-basic-types.h"

G_BEGIN_DECLS
typedef struct _GArray		GArray;
typedef struct _GByteArray	GByteArray;
typedef struct _GPtrArray	GPtrArray;
typedef void (*GDestroyNotify)(gpointer data);
typedef void (*GFunc)(gpointer data, gpointer user_data);
typedef gint (*GCompareFunc)(gconstpointer  a, gconstpointer  b);
typedef gint (*GCompareDataFunc)(gconstpointer  a, gconstpointer  b, gpointer user_data);
struct _GArray {
  gchar *data;
  guint len;
};
struct _GByteArray {
  guint8 *data;
  guint	  len;
};
struct _GPtrArray {
  gpointer *pdata;
  guint	    len;
};
#define g_array_append_val(a,v)	g_array_append_vals(a, &(v), 1)
#define g_array_prepend_val(a,v) g_array_prepend_vals(a, &(v), 1)
#define g_array_insert_val(a,i,v) g_array_insert_vals(a, i, &(v), 1)
#define g_array_index(a,t,i) (((t*)(void*)(a)->data)[(i)])
GArray* g_array_new(gboolean zero_terminated, gboolean clear_, guint element_size);
GArray* g_array_sized_new(gboolean zero_terminated, gboolean clear_, guint element_size, guint reserved_size);
gchar* g_array_free(GArray *array, gboolean free_segment);
GArray *g_array_ref(GArray *array);
void g_array_unref(GArray *array);
guint g_array_get_element_size(GArray *array);
GArray* g_array_append_vals(GArray *array, gconstpointer data, guint len);
GArray* g_array_prepend_vals(GArray *array, gconstpointer data, guint len);
GArray* g_array_insert_vals(GArray *array, guint index_, gconstpointer data, guint len);
GArray* g_array_set_size(GArray *array, guint length);
GArray* g_array_remove_index(GArray *array, guint index_);
GArray* g_array_remove_index_fast(GArray *array, guint index_);
GArray* g_array_remove_range(GArray *array, guint index_, guint length);
void g_array_sort(GArray *array, GCompareFunc compare_func);
void g_array_sort_with_data(GArray *array, GCompareDataFunc compare_func, gpointer user_data);
#define  g_ptr_array_index(array,index_)((array)->pdata)[index_]
GPtrArray* g_ptr_array_new(void);
GPtrArray* g_ptr_array_new_with_free_func(GDestroyNotify element_free_func);
GPtrArray* g_ptr_array_sized_new(guint reserved_size);
gpointer* g_ptr_array_free(GPtrArray *array, gboolean free_seg);
GPtrArray* g_ptr_array_ref(GPtrArray *array);
void g_ptr_array_unref(GPtrArray *array);
void g_ptr_array_set_free_func(GPtrArray *array, GDestroyNotify element_free_func);
void g_ptr_array_set_size(GPtrArray *array, gint length);
gpointer g_ptr_array_remove_index(GPtrArray *array, guint index_);
gpointer g_ptr_array_remove_index_fast(GPtrArray *array, guint index_);
gboolean g_ptr_array_remove(GPtrArray *array, gpointer data);
gboolean g_ptr_array_remove_fast(GPtrArray *array, gpointer data);
void g_ptr_array_remove_range(GPtrArray *array, guint index_, guint length);
void g_ptr_array_add(GPtrArray *array, gpointer data);
void g_ptr_array_sort(GPtrArray *array, GCompareFunc compare_func);
void g_ptr_array_sort_with_data(GPtrArray *array, GCompareDataFunc compare_func, gpointer user_data);
void g_ptr_array_foreach(GPtrArray *array, GFunc func, gpointer user_data);
GByteArray* g_byte_array_new(void);
GByteArray* g_byte_array_sized_new(guint reserved_size);
guint8* g_byte_array_free(GByteArray *array, gboolean free_segment);
GByteArray *g_byte_array_ref(GByteArray *array);
void g_byte_array_unref(GByteArray *array);
GByteArray* g_byte_array_append(GByteArray *array, const guint8 *data, guint len);
GByteArray* g_byte_array_prepend(GByteArray *array, const guint8 *data, guint len);
GByteArray* g_byte_array_set_size(GByteArray *array, guint length);
GByteArray* g_byte_array_remove_index(GByteArray *array, guint index_);
GByteArray* g_byte_array_remove_index_fast(GByteArray *array, guint index_);
GByteArray* g_byte_array_remove_range(GByteArray *array, guint index_, guint length);
void g_byte_array_sort(GByteArray *array, GCompareFunc compare_func);
void g_byte_array_sort_with_data(GByteArray *array, GCompareDataFunc compare_func, gpointer user_data);
G_END_DECLS

#endif