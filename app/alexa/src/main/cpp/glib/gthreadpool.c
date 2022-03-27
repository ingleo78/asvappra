#include "gthreadpool.h"
#include "gasyncqueue.h"
#include "gmain.h"
#include "gtestutils.h"
#include "gtimer.h"

typedef struct _GRealThreadPool GRealThreadPool;
struct _GRealThreadPool {
  GThreadPool pool;
  GAsyncQueue* queue;
  GCond* cond;
  gint max_threads;
  gint num_threads;
  int running;
  int immediate;
  int waiting;
  GCompareDataFunc sort_func;
  gpointer sort_user_data;
};
static const gpointer wakeup_thread_marker = (gpointer) &g_thread_pool_new;
static gint wakeup_thread_serial = 0;
static GAsyncQueue *unused_thread_queue = NULL;
static gint unused_threads = 0;
static gint max_unused_threads = 0;
static gint kill_unused_threads = 0;
static guint max_idle_time = 0;
static void g_thread_pool_queue_push_unlocked (GRealThreadPool *pool, gpointer data);
static void g_thread_pool_free_internal(GRealThreadPool *pool);
static gpointer g_thread_pool_thread_proxy(gpointer data);
static void g_thread_pool_start_thread(GRealThreadPool *pool, GError **error);
static void g_thread_pool_wakeup_and_stop_all(GRealThreadPool *pool);
static GRealThreadPool* g_thread_pool_wait_for_new_pool(void);
static gpointer g_thread_pool_wait_for_new_task(GRealThreadPool *pool);
static void g_thread_pool_queue_push_unlocked(GRealThreadPool *pool, gpointer data) {
  if (pool->sort_func) g_async_queue_push_sorted_unlocked(pool->queue, data, pool->sort_func, pool->sort_user_data);
  else g_async_queue_push_unlocked(pool->queue, data);
}
static GRealThreadPool* g_thread_pool_wait_for_new_pool(void) {
  GRealThreadPool *pool;
  gint local_wakeup_thread_serial;
  guint local_max_unused_threads;
  gint local_max_idle_time;
  gint last_wakeup_thread_serial;
  int have_relayed_thread_marker = FALSE;
  local_max_unused_threads = g_atomic_int_get(&max_unused_threads);
  local_max_idle_time = g_atomic_int_get(&max_idle_time);
  last_wakeup_thread_serial = g_atomic_int_get(&wakeup_thread_serial);
  g_atomic_int_inc(&unused_threads);
  do {
      if (g_atomic_int_get (&unused_threads) >= local_max_unused_threads) pool = NULL;
      else if (local_max_idle_time > 0) {
          GTimeVal end_time;
          g_get_current_time (&end_time);
          g_time_val_add (&end_time, local_max_idle_time * 1000);
          DEBUG_MSG(("thread %p waiting in global pool for %f seconds.", g_thread_self (), local_max_idle_time / 1000.0));
          pool = g_async_queue_timed_pop (unused_thread_queue, &end_time);
	  } else {
          DEBUG_MSG(("thread %p waiting in global pool.", g_thread_self()));
          pool = g_async_queue_pop (unused_thread_queue);
	  }
      if (pool == wakeup_thread_marker) {
          local_wakeup_thread_serial = g_atomic_int_get (&wakeup_thread_serial);
          if (last_wakeup_thread_serial == local_wakeup_thread_serial) {
              if (!have_relayed_thread_marker) {
                  DEBUG_MSG(("thread %p relaying wakeup message to waiting thread with lower serial.", g_thread_self ()));
                  g_async_queue_push (unused_thread_queue, wakeup_thread_marker);
                  have_relayed_thread_marker = TRUE;
                  g_usleep (100);
              }
          } else {
              if (g_atomic_int_exchange_and_add (&kill_unused_threads, -1) > 0) {
                  pool = NULL;
                  break;
              }
              DEBUG_MSG(("thread %p updating to new limits.",g_thread_self()));
              local_max_unused_threads = g_atomic_int_get (&max_unused_threads);
              local_max_idle_time = g_atomic_int_get (&max_idle_time);
              last_wakeup_thread_serial = local_wakeup_thread_serial;

              have_relayed_thread_marker = FALSE;
          }
	  }
  } while(pool == wakeup_thread_marker);
  g_atomic_int_add(&unused_threads, -1);
  return pool;
}
void g_thread_pool_free(GThreadPool *pool, gboolean immediate, gboolean wait_) {
    GRealThreadPool *real;
    real = (GRealThreadPool*)pool;
    g_return_if_fail(real);
    g_return_if_fail(real->running);
    g_return_if_fail(immediate || real->max_threads != 0 || g_async_queue_length (real->queue) == 0);
    g_async_queue_lock(real->queue);
    real->running = FALSE;
    real->immediate = immediate;
    real->waiting = wait_;
    if (wait_) {
        while(g_async_queue_length_unlocked(real->queue) != (gint)-real->num_threads && !(immediate && real->num_threads == 0))
            g_cond_wait(&real->cond, _g_async_queue_get_mutex(real->queue));
    }
    if (immediate || g_async_queue_length_unlocked(real->queue) == (gint)-real->num_threads) {
        if (real->num_threads == 0) {
            g_async_queue_unlock(real->queue);
            g_thread_pool_free_internal(real);
            return;
        }
        g_thread_pool_wakeup_and_stop_all(real);
    }
    real->waiting = FALSE;
    g_async_queue_unlock(real->queue);
}
guint g_thread_pool_unprocessed(GThreadPool *pool) {
    GRealThreadPool *real;
    gint unprocessed;
    real = (GRealThreadPool*)pool;
    g_return_val_if_fail(real, 0);
    g_return_val_if_fail(real->running, 0);
    unprocessed = g_async_queue_length (real->queue);
    return MAX(unprocessed, 0);

}
gboolean g_thread_pool_move_to_front(GThreadPool *pool, gpointer data) {
    GRealThreadPool *real = (GRealThreadPool*)pool;
    gboolean found;
    g_async_queue_lock(real->queue);
    found = g_async_queue_remove_unlocked(real->queue, data);
    if (found) g_async_queue_push_front_unlocked(real->queue, data);
    g_async_queue_unlock(real->queue);
    return found;
}
static gpointer g_thread_pool_wait_for_new_task(GRealThreadPool *pool) {
  gpointer task = NULL;
  if (pool->running || (!pool->immediate && g_async_queue_length_unlocked (pool->queue) > 0)) {
      if (pool->num_threads > pool->max_threads && pool->max_threads != -1) {
          DEBUG_MSG(("superfluous thread %p in pool %p.", g_thread_self(), pool));
	  } else if (pool->pool.exclusive) {
          task = g_async_queue_pop_unlocked (pool->queue);
          DEBUG_MSG(("thread %p in exclusive pool %p waits for task (%d running, %d unprocessed).", g_thread_self(), pool, pool->num_threads,
                    g_async_queue_length_unlocked (pool->queue)));
	  } else {
          GTimeVal end_time;
          g_get_current_time(&end_time);
          g_time_val_add(&end_time, G_USEC_PER_SEC / 2);
          DEBUG_MSG(("thread %p in pool %p waits for up to a 1/2 second for task (%d running, %d unprocessed).", g_thread_self(), pool, pool->num_threads,
                    g_async_queue_length_unlocked (pool->queue)));
          task = g_async_queue_timed_pop_unlocked (pool->queue, &end_time);
	  }
  } else {
      DEBUG_MSG(("pool %p not active, thread %p will go to global pool (running: %s, immediate: %s, len: %d).", pool, g_thread_self(),
		        pool->running ? "true" : "false", pool->immediate ? "true" : "false", g_async_queue_length_unlocked (pool->queue)));
  }
  return task;
}
static gpointer g_thread_pool_thread_proxy(gpointer data) {
  GRealThreadPool *pool;
  pool = data;
  DEBUG_MSG (("thread %p started for pool %p.", g_thread_self(), pool));
  g_async_queue_lock(pool->queue);
  while(TRUE) {
      gpointer task;
      task = g_thread_pool_wait_for_new_task(pool);
      if (task) {
          if (pool->running || !pool->immediate) {
              g_async_queue_unlock(pool->queue);
              DEBUG_MSG(("thread %p in pool %p calling func.", g_thread_self(), pool));
              pool->pool.func(task, pool->pool.user_data);
              g_async_queue_lock(pool->queue);
          }
	  } else {
          int free_pool = FALSE;
          DEBUG_MSG (("thread %p leaving pool %p for global pool.", g_thread_self(), pool));
          pool->num_threads--;
          if (!pool->running) {
              if (!pool->waiting) {
                  if (pool->num_threads == 0)free_pool = TRUE;
                  else {
                      if (g_async_queue_length_unlocked (pool->queue) == -pool->num_threads) g_thread_pool_wakeup_and_stop_all(pool);
                  }
              } else if (pool->immediate || g_async_queue_length_unlocked (pool->queue) <= 0) {
                  g_cond_broadcast(pool->cond);
              }
          }
          g_async_queue_unlock(pool->queue);
          if (free_pool) g_thread_pool_free_internal(pool);
          if ((pool = g_thread_pool_wait_for_new_pool()) == NULL) break;
          g_async_queue_lock(pool->queue);
          DEBUG_MSG(("thread %p entering pool %p from global pool.", g_thread_self (), pool));
	  }
  }
  return NULL;
}
static void g_thread_pool_start_thread(GRealThreadPool *pool, GError **error) {
  int success = FALSE;
  if (pool->num_threads >= pool->max_threads && pool->max_threads != -1) return;
  g_async_queue_lock(unused_thread_queue);
  if (g_async_queue_length_unlocked(unused_thread_queue) < 0) {
      g_async_queue_push_unlocked(unused_thread_queue, pool);
      success = TRUE;
  }
  g_async_queue_unlock(unused_thread_queue);
  if (!success) {
      GError *local_error = NULL;
      g_thread_create(g_thread_pool_thread_proxy, pool, FALSE, &local_error);
      if (local_error) {
          g_propagate_error(error, local_error);
          return;
	  }
  }
  pool->num_threads++;
}
GThreadPool* g_thread_pool_new(GFunc func, gpointer user_data, gint max_threads, gint exclusive, GError **error) {
  GRealThreadPool *retval;
  G_LOCK_DEFINE_STATIC(init);
  g_return_val_if_fail(func, NULL);
  g_return_val_if_fail(!exclusive || max_threads != -1, NULL);
  g_return_val_if_fail(max_threads >= -1, NULL);
  g_return_val_if_fail(g_thread_supported (), NULL);
  retval = g_new(GRealThreadPool, 1);
  retval->pool.func = func;
  retval->pool.user_data = user_data;
  retval->pool.exclusive = exclusive;
  retval->queue = g_async_queue_new();
  retval->cond = NULL;
  retval->max_threads = max_threads;
  retval->num_threads = 0;
  retval->running = TRUE;
  retval->sort_func = NULL;
  retval->sort_user_data = NULL;
  G_LOCK(init);
  if (!unused_thread_queue) unused_thread_queue = g_async_queue_new();
  G_UNLOCK(init);
  if (retval->pool.exclusive) {
      g_async_queue_lock (retval->queue);
      while (retval->num_threads < retval->max_threads) {
          GError *local_error = NULL;
          g_thread_pool_start_thread(retval, &local_error);
          if (local_error) {
              g_propagate_error(error, local_error);
              break;
          }
	  }
      g_async_queue_unlock(retval->queue);
  }
  return (GThreadPool*)retval;
}
void g_thread_pool_push(GThreadPool *pool, gpointer data, GError **error) {
  GRealThreadPool *real;
  real = (GRealThreadPool*)pool;
  g_return_if_fail(real);
  g_return_if_fail(real->running);
  g_async_queue_lock(real->queue);
  if (g_async_queue_length_unlocked(real->queue) >= 0) g_thread_pool_start_thread(real, error);
  g_thread_pool_queue_push_unlocked(real, data);
  g_async_queue_unlock(real->queue);
}
void g_thread_pool_set_max_threads(GThreadPool *pool, gint max_threads, GError **error) {
  GRealThreadPool *real;
  gint to_start;
  real = (GRealThreadPool*)pool;
  g_return_if_fail(real);
  g_return_if_fail(real->running);
  g_return_if_fail(!real->pool.exclusive || max_threads != -1);
  g_return_if_fail(max_threads >= -1);
  g_async_queue_lock(real->queue);
  real->max_threads = max_threads;
  if (pool->exclusive) to_start = real->max_threads - real->num_threads;
  else to_start = g_async_queue_length_unlocked(real->queue);
  for ( ; to_start > 0; to_start--) {
      GError *local_error = NULL;
      g_thread_pool_start_thread(real, &local_error);
      if (local_error) {
          g_propagate_error(error, local_error);
          break;
	  }
  }
  g_async_queue_unlock(real->queue);
}
gint g_thread_pool_get_max_threads(GThreadPool *pool) {
  GRealThreadPool *real;
  gint retval;
  real = (GRealThreadPool*)pool;
  g_return_val_if_fail(real, 0);
  g_return_val_if_fail(real->running, 0);
  g_async_queue_lock(real->queue);
  retval = real->max_threads;
  g_async_queue_unlock(real->queue);
  return retval;
}
guint g_thread_pool_get_num_threads(GThreadPool *pool) {
  GRealThreadPool *real;
  guint retval;
  real = (GRealThreadPool*)pool;
  g_return_val_if_fail(real, 0);
  g_return_val_if_fail(real->running, 0);
  g_async_queue_lock(real->queue);
  retval = real->num_threads;
  g_async_queue_unlock(real->queue);
  return retval;
}
static void g_thread_pool_free_internal(GRealThreadPool* pool) {
  g_return_if_fail(pool);
  g_return_if_fail(pool->running == FALSE);
  g_return_if_fail(pool->num_threads == 0);
  g_async_queue_unref(pool->queue);
  if (pool->cond) g_cond_free(pool->cond);
  g_free(pool);
}
static void g_thread_pool_wakeup_and_stop_all(GRealThreadPool* pool) {
  guint i;
  g_return_if_fail(pool);
  g_return_if_fail(pool->running == FALSE);
  g_return_if_fail(pool->num_threads != 0);
  pool->immediate = TRUE;
  for (i = 0; i < pool->num_threads; i++) g_thread_pool_queue_push_unlocked(pool, GUINT_TO_POINTER(1));
}
void g_thread_pool_set_max_unused_threads(gint max_threads) {
  g_return_if_fail(max_threads >= -1);
  g_atomic_int_set(&max_unused_threads, max_threads);
  if (max_threads != -1) {
      max_threads -= g_atomic_int_get(&unused_threads);
      if (max_threads < 0) {
          g_atomic_int_set(&kill_unused_threads, -max_threads);
          g_atomic_int_inc(&wakeup_thread_serial);
          g_async_queue_lock(unused_thread_queue);
          do {
              g_async_queue_push_unlocked(unused_thread_queue, wakeup_thread_marker);
          } while(++max_threads);
          g_async_queue_unlock(unused_thread_queue);
	  }
  }
}
gint g_thread_pool_get_max_unused_threads(void) {
  return g_atomic_int_get(&max_unused_threads);
}
guint g_thread_pool_get_num_unused_threads(void) {
  return g_atomic_int_get(&unused_threads);
}
void g_thread_pool_stop_unused_threads(void) {
  guint oldval;
  oldval = g_thread_pool_get_max_unused_threads();
  g_thread_pool_set_max_unused_threads(0);
  g_thread_pool_set_max_unused_threads(oldval);
}
void g_thread_pool_set_sort_function(GThreadPool *pool, GCompareDataFunc func, gpointer user_data) {
  GRealThreadPool *real;
  real = (GRealThreadPool*)pool;
  g_return_if_fail(real);
  g_return_if_fail(real->running);
  g_async_queue_lock(real->queue);
  real->sort_func = func;
  real->sort_user_data = user_data;
  if (func) g_async_queue_sort_unlocked(real->queue, real->sort_func, real->sort_user_data);
  g_async_queue_unlock(real->queue);
}
void g_thread_pool_set_max_idle_time(guint interval) {
  guint i;
  g_atomic_int_set(&max_idle_time, interval);
  i = g_atomic_int_get(&unused_threads);
  if (i > 0) {
      g_atomic_int_inc(&wakeup_thread_serial);
      g_async_queue_lock(unused_thread_queue);
      do {
          g_async_queue_push_unlocked(unused_thread_queue, wakeup_thread_marker);
	  } while(--i);
      g_async_queue_unlock(unused_thread_queue);
  }
}
guint g_thread_pool_get_max_idle_time(void) {
  return g_atomic_int_get(&max_idle_time);
}