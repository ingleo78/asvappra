#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_ASYNCQUEUE_H__
#define __G_ASYNCQUEUE_H__

#include "gmacros.h"
#include "gtypes.h"
#include "gthread.h"

G_BEGIN_DECLS
typedef struct _GAsyncQueue GAsyncQueue;
GAsyncQueue* g_async_queue_new(void);
GAsyncQueue* g_async_queue_new_full(GDestroyNotify item_free_func);
void g_async_queue_lock(GAsyncQueue *queue);
void g_async_queue_unlock(GAsyncQueue *queue);
GAsyncQueue* g_async_queue_ref(GAsyncQueue *queue);
void g_async_queue_unref(GAsyncQueue *queue);
#ifndef G_DISABLE_DEPRECATED
void g_async_queue_ref_unlocked(GAsyncQueue *queue);
void g_async_queue_unref_and_unlock(GAsyncQueue *queue);
#endif
void g_async_queue_push(GAsyncQueue *queue, gpointer data);
void g_async_queue_push_unlocked(GAsyncQueue *queue, gpointer data);
void g_async_queue_push_sorted(GAsyncQueue *queue, gpointer data, GCompareDataFunc func, gpointer user_data);
void g_async_queue_push_sorted_unlocked(GAsyncQueue *queue, gpointer data, GCompareDataFunc func, gpointer user_data);
gpointer g_async_queue_pop(GAsyncQueue *queue);
gpointer g_async_queue_pop_unlocked(GAsyncQueue *queue);
gpointer g_async_queue_try_pop(GAsyncQueue *queue);
gpointer g_async_queue_try_pop_unlocked(GAsyncQueue *queue);
gpointer g_async_queue_timed_pop(GAsyncQueue *queue, GTimeVal *end_time);
gpointer g_async_queue_timed_pop_unlocked(GAsyncQueue *queue, GTimeVal *end_time);
gint g_async_queue_length(GAsyncQueue *queue);
gint g_async_queue_length_unlocked(GAsyncQueue *queue);
void g_async_queue_sort(GAsyncQueue *queue, GCompareDataFunc func, gpointer user_data);
void g_async_queue_sort_unlocked(GAsyncQueue *queue, GCompareDataFunc func, gpointer user_data);
gboolean g_async_queue_remove(GAsyncQueue *queue, gpointer item);
gboolean g_async_queue_remove_unlocked(GAsyncQueue *queue, gpointer item);
void g_async_queue_push_front(GAsyncQueue *queue, gpointer item);
void g_async_queue_push_front_unlocked(GAsyncQueue *queue, gpointer item);
gpointer g_async_queue_timed_pop(GAsyncQueue *queue, GTimeVal *end_time);
gpointer g_async_queue_timed_pop_unlocked(GAsyncQueue *queue, GTimeVal *end_time);
GMutex* _g_async_queue_get_mutex(GAsyncQueue *queue);
G_END_DECLS
#endif