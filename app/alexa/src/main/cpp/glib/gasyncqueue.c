#include "gasyncqueue.h"
#include "gmem.h"
#include "gqueue.h"
#include "gtestutils.h"
#include "gthread.h"

struct _GAsyncQueue {
  GMutex *mutex;
  GCond *cond;
  GQueue queue;
  GDestroyNotify item_free_func;
  guint waiting_threads;
  gint32 ref_count;
};
typedef struct {
  GCompareDataFunc func;
  gpointer         user_data;
} SortData;
GAsyncQueue*
g_async_queue_new (void) {
  GAsyncQueue* retval = g_new(GAsyncQueue, 1);
  retval->mutex = g_mutex_new();
  retval->cond = NULL;
  g_queue_init (&retval->queue);
  retval->waiting_threads = 0;
  retval->ref_count = 1;
  retval->item_free_func = NULL;
  return retval;
}
GAsyncQueue* g_async_queue_new_full(GDestroyNotify item_free_func) {
  GAsyncQueue *async_queue = g_async_queue_new();
  async_queue->item_free_func = item_free_func;
  return async_queue;
}
GAsyncQueue* g_async_queue_ref(GAsyncQueue *queue) {
  g_return_val_if_fail(queue, NULL);
  g_return_val_if_fail(g_atomic_int_get(&queue->ref_count) > 0, NULL);
  g_atomic_int_inc(&queue->ref_count);
  return queue;
}
void g_async_queue_ref_unlocked(GAsyncQueue *queue) {
  g_return_if_fail(queue);
  g_return_if_fail(g_atomic_int_get(&queue->ref_count) > 0);
  g_atomic_int_inc(&queue->ref_count);
}
void g_async_queue_unref_and_unlock(GAsyncQueue *queue) {
  g_return_if_fail(queue);
  g_return_if_fail(g_atomic_int_get(&queue->ref_count) > 0);
  g_mutex_unlock(queue->mutex);
  g_async_queue_unref(queue);
}
void g_async_queue_unref(GAsyncQueue *queue) {
  g_return_if_fail(queue);
  g_return_if_fail(g_atomic_int_get(&queue->ref_count) > 0);
  if (g_atomic_int_dec_and_test(&queue->ref_count)) {
      g_return_if_fail(queue->waiting_threads == 0);
      g_mutex_free(queue->mutex);
      if (queue->cond) g_cond_free(queue->cond);
      if (queue->item_free_func) g_queue_foreach(&queue->queue, (GFunc)queue->item_free_func, NULL);
      g_queue_clear(&queue->queue);
      g_free(queue);
  }
}
void g_async_queue_lock(GAsyncQueue *queue) {
  g_return_if_fail(queue);
  g_return_if_fail(g_atomic_int_get(&queue->ref_count) > 0);
  g_mutex_lock (queue->mutex);
}
void g_async_queue_unlock(GAsyncQueue *queue) {
  g_return_if_fail(queue);
  g_return_if_fail(g_atomic_int_get(&queue->ref_count) > 0);
  g_mutex_unlock(queue->mutex);
}
void g_async_queue_push(GAsyncQueue* queue, gpointer data) {
  g_return_if_fail(queue);
  g_return_if_fail(g_atomic_int_get(&queue->ref_count) > 0);
  g_return_if_fail(data);
  g_mutex_lock(queue->mutex);
  g_async_queue_push_unlocked(queue, data);
  g_mutex_unlock(queue->mutex);
}
void g_async_queue_push_unlocked(GAsyncQueue* queue, gpointer data) {
  g_return_if_fail(queue);
  g_return_if_fail(g_atomic_int_get(&queue->ref_count) > 0);
  g_return_if_fail(data);
  g_queue_push_head(&queue->queue, data);
  if (queue->waiting_threads > 0) g_cond_signal(queue->cond);
}
void g_async_queue_push_sorted(GAsyncQueue *queue, gpointer data, GCompareDataFunc func, gpointer user_data) {
  g_return_if_fail(queue != NULL);
  g_mutex_lock(queue->mutex);
  g_async_queue_push_sorted_unlocked(queue, data, func, user_data);
  g_mutex_unlock(queue->mutex);
}
static gint g_async_queue_invert_compare(gpointer v1, gpointer v2, SortData *sd) {
  return -sd->func(v1, v2, sd->user_data);
}
void g_async_queue_push_sorted_unlocked(GAsyncQueue *queue, gpointer data, GCompareDataFunc func, gpointer user_data) {
  SortData sd;
  g_return_if_fail(queue != NULL);
  sd.func = func;
  sd.user_data = user_data;
  g_queue_insert_sorted(&queue->queue, data, (GCompareDataFunc)g_async_queue_invert_compare, &sd);
  if (queue->waiting_threads > 0) g_cond_signal(queue->cond);
}
static gpointer g_async_queue_pop_intern_unlocked(GAsyncQueue *queue, gboolean try, GTimeVal *end_time) {
  gpointer retval;
  if (!g_queue_peek_tail_link(&queue->queue)) {
      if (try) return NULL;
      if (!queue->cond) queue->cond = g_cond_new();
      if (!end_time) {
          queue->waiting_threads++;
	      while(!g_queue_peek_tail_link(&queue->queue)) g_cond_wait(queue->cond, queue->mutex);
          queue->waiting_threads--;
      } else {
          queue->waiting_threads++;
          while(!g_queue_peek_tail_link(&queue->queue)) if (!g_cond_timed_wait(queue->cond, queue->mutex, end_time)) break;
          queue->waiting_threads--;
          if (!g_queue_peek_tail_link(&queue->queue))
	    return NULL;
      }
  }
  retval = g_queue_pop_tail(&queue->queue);
  g_assert(retval);
  return retval;
}
gpointer g_async_queue_pop(GAsyncQueue* queue) {
  gpointer retval;
  g_return_val_if_fail(queue, NULL);
  g_return_val_if_fail(g_atomic_int_get(&queue->ref_count) > 0, NULL);
  g_mutex_lock(queue->mutex);
  retval = g_async_queue_pop_intern_unlocked(queue, FALSE, NULL);
  g_mutex_unlock(queue->mutex);
  return retval;
}
gpointer g_async_queue_pop_unlocked(GAsyncQueue* queue) {
  g_return_val_if_fail(queue, NULL);
  g_return_val_if_fail(g_atomic_int_get(&queue->ref_count) > 0, NULL);
  return g_async_queue_pop_intern_unlocked(queue, FALSE, NULL);
}
gpointer g_async_queue_try_pop(GAsyncQueue* queue) {
  gpointer retval;
  g_return_val_if_fail(queue, NULL);
  g_return_val_if_fail(g_atomic_int_get(&queue->ref_count) > 0, NULL);
  g_mutex_lock(queue->mutex);
  retval = g_async_queue_pop_intern_unlocked(queue, TRUE, NULL);
  g_mutex_unlock(queue->mutex);
  return retval;
}
gpointer g_async_queue_try_pop_unlocked(GAsyncQueue* queue) {
  g_return_val_if_fail(queue, NULL);
  g_return_val_if_fail(g_atomic_int_get(&queue->ref_count) > 0, NULL);
  return g_async_queue_pop_intern_unlocked(queue, TRUE, NULL);
}
gpointer g_async_queue_timed_pop(GAsyncQueue* queue, GTimeVal *end_time) {
  gpointer retval;
  g_return_val_if_fail(queue, NULL);
  g_return_val_if_fail(g_atomic_int_get(&queue->ref_count) > 0, NULL);
  g_mutex_lock(queue->mutex);
  retval = g_async_queue_pop_intern_unlocked(queue, FALSE, end_time);
  g_mutex_unlock(queue->mutex);
  return retval;  
}
gpointer g_async_queue_timed_pop_unlocked(GAsyncQueue* queue, GTimeVal *end_time) {
  g_return_val_if_fail(queue, NULL);
  g_return_val_if_fail(g_atomic_int_get(&queue->ref_count) > 0, NULL);
  return g_async_queue_pop_intern_unlocked(queue, FALSE, end_time);
}
gint g_async_queue_length(GAsyncQueue* queue) {
  gint retval;
  g_return_val_if_fail(queue, 0);
  g_return_val_if_fail(g_atomic_int_get(&queue->ref_count) > 0, 0);
  g_mutex_lock(queue->mutex);
  retval = queue->queue.length - queue->waiting_threads;
  g_mutex_unlock(queue->mutex);
  return retval;
}
gint g_async_queue_length_unlocked(GAsyncQueue* queue) {
  g_return_val_if_fail(queue, 0);
  g_return_val_if_fail(g_atomic_int_get(&queue->ref_count) > 0, 0);
  return queue->queue.length - queue->waiting_threads;
}
void g_async_queue_sort(GAsyncQueue *queue, GCompareDataFunc func, gpointer user_data) {
  g_return_if_fail(queue != NULL);
  g_return_if_fail(func != NULL);
  g_mutex_lock(queue->mutex);
  g_async_queue_sort_unlocked(queue, func, user_data);
  g_mutex_unlock(queue->mutex);
}
void g_async_queue_sort_unlocked(GAsyncQueue *queue, GCompareDataFunc func, gpointer user_data) {
  SortData sd;
  g_return_if_fail(queue != NULL);
  g_return_if_fail(func != NULL);
  sd.func = func;
  sd.user_data = user_data;
  g_queue_sort (&queue->queue, (GCompareDataFunc)g_async_queue_invert_compare, &sd);
}
GMutex* _g_async_queue_get_mutex(GAsyncQueue* queue) {
  g_return_val_if_fail(queue, NULL);
  g_return_val_if_fail(g_atomic_int_get(&queue->ref_count) > 0, NULL);
  return queue->mutex;
}
gboolean g_async_queue_remove(GAsyncQueue *queue, gpointer item) {
  gboolean ret;
  g_return_val_if_fail(queue != NULL, FALSE);
  g_return_val_if_fail(item != NULL, FALSE);
  g_mutex_lock(&queue->mutex);
  ret = g_async_queue_remove_unlocked(queue, item);
  g_mutex_unlock(&queue->mutex);
  return ret;
}
gboolean g_async_queue_remove_unlocked(GAsyncQueue *queue, gpointer item) {
  g_return_val_if_fail(queue != NULL, FALSE);
  g_return_val_if_fail(item != NULL, FALSE);
  return g_queue_remove(&queue->queue, item);
}
void g_async_queue_push_front(GAsyncQueue *queue, gpointer item) {
  g_return_if_fail(queue != NULL);
  g_return_if_fail(item != NULL);
  g_mutex_lock(&queue->mutex);
  g_async_queue_push_front_unlocked(queue, item);
  g_mutex_unlock(&queue->mutex);
}
void g_async_queue_push_front_unlocked(GAsyncQueue *queue, gpointer item) {
  g_return_if_fail(queue != NULL);
  g_return_if_fail(item != NULL);
  g_queue_push_tail(&queue->queue, item);
  if (queue->waiting_threads > 0) g_cond_signal(&queue->cond);
}