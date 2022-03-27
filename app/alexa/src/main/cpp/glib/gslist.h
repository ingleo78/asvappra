#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_SLIST_H__
#define __G_SLIST_H__

#include "gmacros.h"
#include "gmem.h"
#include "gtypes.h"

G_BEGIN_DECLS
typedef struct _GSList GSList;
struct _GSList {
  gpointer data;
  GSList *next;
};
GSList* g_slist_alloc(void) G_GNUC_WARN_UNUSED_RESULT;
void g_slist_free(GSList *list);
void g_slist_free_1(GSList *list);
#define g_slist_free1 g_slist_free_1
void g_slist_free_full(GSList *list, GDestroyNotify    free_func);
GSList* g_slist_append(GSList *list, gpointer data) G_GNUC_WARN_UNUSED_RESULT;
GSList* g_slist_prepend(GSList *list, gpointer data) G_GNUC_WARN_UNUSED_RESULT;
GSList* g_slist_insert(GSList *list, gpointer data, gint position) G_GNUC_WARN_UNUSED_RESULT;
GSList* g_slist_insert_sorted(GSList *list, gpointer data, GCompareFunc func) G_GNUC_WARN_UNUSED_RESULT;
GSList* g_slist_insert_sorted_with_data(GSList *list, gpointer data, GCompareDataFunc func, gpointer user_data) G_GNUC_WARN_UNUSED_RESULT;
GSList* g_slist_insert_before(GSList *slist, GSList *sibling, gpointer data) G_GNUC_WARN_UNUSED_RESULT;
GSList* g_slist_concat(GSList *list1, GSList *list2) G_GNUC_WARN_UNUSED_RESULT;
GSList* g_slist_remove(GSList *list, gconstpointer data) G_GNUC_WARN_UNUSED_RESULT;
GSList* g_slist_remove_all(GSList *list, gconstpointer data) G_GNUC_WARN_UNUSED_RESULT;
GSList* g_slist_remove_link(GSList *list, GSList *link_) G_GNUC_WARN_UNUSED_RESULT;
GSList* g_slist_delete_link(GSList *list, GSList *link_) G_GNUC_WARN_UNUSED_RESULT;
GSList* g_slist_reverse(GSList *list) G_GNUC_WARN_UNUSED_RESULT;
GSList* g_slist_copy(GSList *list) G_GNUC_WARN_UNUSED_RESULT;
GSList* g_slist_nth(GSList *list, guint n);
GSList* g_slist_find(GSList *list, gconstpointer data);
GSList* g_slist_find_custom(GSList *list, gconstpointer data, GCompareFunc func);
gint g_slist_position(GSList *list, GSList *llink);
gint g_slist_index(GSList *list, gconstpointer data);
GSList* g_slist_last(GSList *list);
guint g_slist_length(GSList *list);
void g_slist_foreach(GSList *list, GFunc func, gpointer user_data);
GSList* g_slist_sort(GSList *list, GCompareFunc compare_func) G_GNUC_WARN_UNUSED_RESULT;
GSList* g_slist_sort_with_data(GSList *list, GCompareDataFunc compare_func, gpointer user_data) G_GNUC_WARN_UNUSED_RESULT;
gpointer g_slist_nth_data(GSList *list, guint n);
#define g_slist_next(slist)	((slist) ? (((GSList *)(slist))->next) : NULL)
#ifndef G_DISABLE_DEPRECATED
void g_slist_push_allocator(gpointer dummy);
void g_slist_pop_allocator(void);
#endif
G_END_DECLS

#endif