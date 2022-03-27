#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_THREADPOOL_H__
#define __G_THREADPOOL_H__

#include "gthread.h"

G_BEGIN_DECLS
#define DEBUG_MSG(args...)
typedef struct _GThreadPool GThreadPool;
struct _GThreadPool {
  GFunc func;
  gpointer user_data;
  gboolean exclusive;
};
GThreadPool* g_thread_pool_new(GFunc func, gpointer user_data, gint max_threads, gboolean exclusive, GError **error);
void g_thread_pool_free(GThreadPool *pool, gboolean immediate, gboolean wait_);
guint g_thread_pool_unprocessed(GThreadPool *pool);
gboolean g_thread_pool_move_to_front(GThreadPool *pool, gpointer data);
void g_thread_pool_push(GThreadPool *pool, gpointer data, GError **error);
void g_thread_pool_set_max_threads(GThreadPool *pool, gint max_threads, GError **error);
gint g_thread_pool_get_max_threads(GThreadPool *pool);
guint g_thread_pool_get_num_threads(GThreadPool *pool);
guint g_thread_pool_unprocessed(GThreadPool *pool);
void g_thread_pool_free(GThreadPool *pool, gboolean immediate, gboolean wait_);
void g_thread_pool_set_max_unused_threads(gint max_threads);
gint g_thread_pool_get_max_unused_threads(void);
guint g_thread_pool_get_num_unused_threads(void);
void g_thread_pool_stop_unused_threads(void);
void g_thread_pool_set_sort_function(GThreadPool *pool, GCompareDataFunc func, gpointer user_data);
void g_thread_pool_set_max_idle_time(guint interval);
guint g_thread_pool_get_max_idle_time(void);
G_END_DECLS

#endif
