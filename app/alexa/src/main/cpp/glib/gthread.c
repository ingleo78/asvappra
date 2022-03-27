#define G_IMPLEMENT_INLINES 1
#define __G_THREAD_C__

#include "glibconfig.h"
#include "gthread.h"
#include "gthreadprivate.h"
#include <unistd.h>
#ifdef G_OS_WIN32
#include <sys/time.h>
#include <time.h>
#else
//#include <windows.h>
#endif
#include <string.h>
#include <malloc.h>
#include "glib.h"
#include "garray.h"
#include "gslist.h"
#include "gtestutils.h"
#include "gtimer.h"

GQuark g_thread_error_quark(void) {
  return g_quark_from_static_string("g_thread_error");
}
struct  _GRealThread {
  GThread thread;
  void* private_data;
  struct _GRealThread *next;
  void* retval;
  GSystemThread system_thread;
};
typedef struct _GRealThread GRealThread;
typedef struct _GStaticPrivateNode GStaticPrivateNode;
struct _GStaticPrivateNode {
  void* data;
  GDestroyNotify destroy;
};
static void g_thread_cleanup(gpointer data);
unsigned long long (*g_thread_gettime)(void) = gettime;
static GSystemThread zero_thread;
gboolean g_thread_use_default_impl = (int)TRUE;
gboolean g_threads_got_initialized = FALSE;
GThreadFunctions g_thread_functions_for_glib_use = {
  (GMutex*(*)())g_thread_fail, NULL, NULL, NULL, NULL, (GCond*(*)())g_thread_fail, NULL,
  NULL, NULL, NULL, NULL, (GPrivate*(*)(GDestroyNotify))g_thread_fail,NULL, NULL,
  (void(*)(GThreadFunc, gpointer, gulong, gboolean, gboolean, GThreadPriority, gpointer, GError**))g_thread_fail, NULL, NULL,
  NULL, NULL, NULL,NULL
};
static GMutex *g_once_mutex = NULL;
static GCond *g_once_cond = NULL;
static GPrivate *g_thread_specific_private = NULL;
static GRealThread *g_thread_all_threads = NULL;
static GSList *g_thread_free_indeces = NULL;
static GSList *g_once_init_list = NULL;
G_LOCK_DEFINE_STATIC (g_thread);
#ifdef G_THREADS_ENABLED
void g_thread_init_glib(void) {
  GRealThread* main_thread = (GRealThread*) g_thread_self();
  g_once_mutex = g_mutex_new();
  g_once_cond = g_cond_new();
  _g_mem_thread_init_noprivate_nomessage();
  g_threads_got_initialized = TRUE;
  g_thread_specific_private = g_private_new(g_thread_cleanup);
  g_private_set(g_thread_specific_private, main_thread);
  G_THREAD_UF(thread_self, (&main_thread->system_thread));
  _g_slice_thread_init_nomessage();
  _g_messages_thread_init_nomessage();
  _g_atomic_thread_init();
  _g_convert_thread_init();
  _g_rand_thread_init();
  _g_main_thread_init();
  _g_utils_thread_init();
  _g_futex_thread_init();
#ifdef G_OS_WIN32
  _g_win32_thread_init();
#endif
}
#endif
gpointer g_once_impl(GOnce *once, GThreadFunc func, gpointer arg) {
  g_mutex_lock(g_once_mutex);
  while (once->status == G_ONCE_STATUS_PROGRESS) g_cond_wait(g_once_cond, g_once_mutex);
  if (once->status != G_ONCE_STATUS_READY) {
      once->status = G_ONCE_STATUS_PROGRESS;
      g_mutex_unlock(g_once_mutex);
      once->retval = func (arg);
      g_mutex_lock(g_once_mutex);
      once->status = G_ONCE_STATUS_READY;
      g_cond_broadcast(g_once_cond);
  }
  g_mutex_unlock(g_once_mutex);
  return once->retval;
}
gboolean g_once_init_enter_impl(volatile gsize *value_location) {
  gboolean need_init = FALSE;
  g_mutex_lock(g_once_mutex);
  if (g_atomic_pointer_get(value_location) == NULL) {
      if (!g_slist_find(g_once_init_list, (void*)value_location)) {
          need_init = TRUE;
          g_once_init_list = g_slist_prepend(g_once_init_list, (void*)value_location);
      } else {
          do {
              g_cond_wait(g_once_cond, g_once_mutex);
          }while (g_slist_find(g_once_init_list, (void*)value_location));
      }
  }
  g_mutex_unlock(g_once_mutex);
  return need_init;
}
void g_once_init_leave(volatile gsize *value_location, gsize initialization_value) {
  g_return_if_fail(g_atomic_pointer_get (value_location) == NULL);
  g_return_if_fail(initialization_value != 0);
  g_return_if_fail(g_once_init_list != NULL);
  g_atomic_pointer_set((void**)value_location, (void*)initialization_value);
  g_mutex_lock(g_once_mutex);
  g_once_init_list = g_slist_remove(g_once_init_list, (void*)value_location);
  g_cond_broadcast(g_once_cond);
  g_mutex_unlock(g_once_mutex);
}
void g_static_mutex_init(GStaticMutex *mutex) {
  static const GStaticMutex init_mutex = G_STATIC_MUTEX_INIT;
  g_return_if_fail(mutex);
  *mutex = init_mutex;
}
GMutex *g_static_mutex_get_mutex_impl(GMutex** mutex) {
  if (!g_thread_supported()) return NULL;
  g_assert(g_once_mutex);
  g_mutex_lock(g_once_mutex);
  if (!(*mutex)) g_atomic_pointer_set(mutex, g_mutex_new());
  g_mutex_unlock(g_once_mutex);
  return *mutex;
}
void g_static_mutex_free(GStaticMutex* mutex) {
  GMutex **runtime_mutex;
  g_return_if_fail(mutex);
  runtime_mutex = ((GMutex**)mutex);
  if (*runtime_mutex) g_mutex_free(*runtime_mutex);
  *runtime_mutex = NULL;
}
void g_static_rec_mutex_init(GStaticRecMutex *mutex) {
  static const GStaticRecMutex init_mutex = G_STATIC_REC_MUTEX_INIT;
  g_return_if_fail(mutex);
  *mutex = init_mutex;
}
void g_static_rec_mutex_lock(GStaticRecMutex* mutex) {
  GSystemThread self;
  g_return_if_fail(mutex);
  if (!g_thread_supported()) return;
  G_THREAD_UF(thread_self, (&self));
  if (g_system_thread_equal(self, mutex->owner)) {
      mutex->depth++;
      return;
  }
  g_static_mutex_lock(&mutex->mutex);
  g_system_thread_assign(mutex->owner, self);
  mutex->depth = 1;
}
gboolean g_static_rec_mutex_trylock (GStaticRecMutex* mutex) {
  GSystemThread self;
  g_return_val_if_fail(mutex, FALSE);
  if (!g_thread_supported()) return TRUE;
  G_THREAD_UF (thread_self, (&self));
  if (g_system_thread_equal(self, mutex->owner)) {
      mutex->depth++;
      return TRUE;
  }
  if (!g_static_mutex_trylock(&mutex->mutex)) return FALSE;
  g_system_thread_assign (mutex->owner, self);
  mutex->depth = 1;
  return TRUE;
}
void g_static_rec_mutex_unlock(GStaticRecMutex* mutex) {
  g_return_if_fail(mutex);
  if (!g_thread_supported()) return;
  if (mutex->depth > 1) {
      mutex->depth--;
      return;
  }
  g_system_thread_assign(mutex->owner, zero_thread);
  g_static_mutex_unlock(&mutex->mutex);
}
void g_static_rec_mutex_lock_full(GStaticRecMutex *mutex, guint depth) {
  GSystemThread self;
  g_return_if_fail(mutex);
  if (!g_thread_supported()) return;
  if (depth == 0) return;
  G_THREAD_UF(thread_self, (&self));
  if (g_system_thread_equal(self, mutex->owner)) {
      mutex->depth += depth;
      return;
  }
  g_static_mutex_lock(&mutex->mutex);
  g_system_thread_assign(mutex->owner, self);
  mutex->depth = depth;
}
guint g_static_rec_mutex_unlock_full(GStaticRecMutex *mutex) {
  guint depth;
  g_return_val_if_fail(mutex, 0);
  if (!g_thread_supported()) return 1;
  depth = mutex->depth;
  g_system_thread_assign(mutex->owner, zero_thread);
  mutex->depth = 0;
  g_static_mutex_unlock(&mutex->mutex);
  return depth;
}
void g_static_rec_mutex_free(GStaticRecMutex *mutex) {
  g_return_if_fail(mutex);
  g_static_mutex_free(&mutex->mutex);
}
void g_static_private_init(GStaticPrivate *private_key) {
  private_key->index = 0;
}
gpointer g_static_private_get(GStaticPrivate *private_key) {
  GRealThread *self = (GRealThread*)g_thread_self();
  GArray *array;
  array = self->private_data;
  if (!array) return NULL;
  if (!private_key->index) return NULL;
  else if (private_key->index <= array->len) return g_array_index(array, GStaticPrivateNode,private_key->index - 1).data;
  else return NULL;
}
void g_static_private_set(GStaticPrivate *private_key, gpointer data, GDestroyNotify  notify) {
  GRealThread *self = (GRealThread*)g_thread_self();
  GArray *array;
  static guint next_index = 0;
  GStaticPrivateNode *node;
  array = self->private_data;
  if (!array) {
      array = g_array_new(FALSE, TRUE, sizeof(GStaticPrivateNode));
      self->private_data = array;
  }
  if (!private_key->index) {
      G_LOCK(g_thread);
      if (!private_key->index) {
          if (g_thread_free_indeces) {
              private_key->index = GPOINTER_TO_UINT(g_thread_free_indeces->data);
              g_thread_free_indeces = g_slist_delete_link(g_thread_free_indeces, g_thread_free_indeces);
          } else private_key->index = ++next_index;
	  }
      G_UNLOCK(g_thread);
  }
  if (private_key->index > array->len) g_array_set_size(array, private_key->index);
  node = &g_array_index(array, GStaticPrivateNode, private_key->index - 1);
  if (node->destroy) {
      void* ddata = node->data;
      GDestroyNotify ddestroy = node->destroy;
      node->data = data;
      node->destroy = notify;
      ddestroy(ddata);
  } else {
      node->data = data;
      node->destroy = notify;
  }
}
void g_static_private_free(GStaticPrivate *private_key) {
  guint idx = private_key->index;
  GRealThread *thread;
  if (!idx) return;
  private_key->index = 0;
  G_LOCK(g_thread);
  thread = g_thread_all_threads;
  while(thread) {
      GArray *array = thread->private_data;
      thread = thread->next;
      if (array && idx <= array->len) {
          GStaticPrivateNode *node = &g_array_index(array, GStaticPrivateNode,idx - 1);
          void* ddata = node->data;
          GDestroyNotify ddestroy = node->destroy;
          node->data = NULL;
          node->destroy = NULL;
          if (ddestroy) {
              G_UNLOCK(g_thread);
              ddestroy(ddata);
              G_LOCK(g_thread);
          }
	  }
  }
  g_thread_free_indeces = g_slist_prepend(g_thread_free_indeces, GUINT_TO_POINTER(idx));
  G_UNLOCK(g_thread);
}
static void g_thread_cleanup(gpointer data) {
  if (data) {
      GRealThread* thread = data;
      if (thread->private_data) {
          GArray* array = thread->private_data;
          guint i;
          for (i = 0; i < array->len; i++ ) {
              GStaticPrivateNode *node = &g_array_index(array, GStaticPrivateNode, i);
              if (node->destroy) node->destroy(node->data);
          }
          g_array_free (array, TRUE);
	  }
      if (!thread->thread.joinable) {
          GRealThread *t, *p;
          G_LOCK(g_thread);
          for (t = g_thread_all_threads, p = NULL; t; p = t, t = t->next) {
              if (t == thread) {
              if (p) p->next = t->next;
              else g_thread_all_threads = t->next;
              break;
              }
          }
          G_UNLOCK(g_thread);
          g_system_thread_assign(thread->system_thread, zero_thread);
          g_free(thread);
	  }
  }
}
void g_thread_fail(void) {
  g_error("The thread system is not yet initialized.");
}
#define G_NSEC_PER_SEC 1000000000
static unsigned long long gettime(void) {
#ifndef G_OS_WIN32
  guint64 v;
  v = time(NULL);
  v -= G_GINT64_CONSTANT(116444736000000000);
  v *= 100;
  return v;
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (guint64)tv.tv_sec * G_NSEC_PER_SEC + tv.tv_usec * (G_NSEC_PER_SEC / G_USEC_PER_SEC);
#endif
}
static gpointer g_thread_create_proxy(gpointer data) {
  GRealThread* thread = data;
  g_assert(data);
  g_private_set(g_thread_specific_private, data);
  G_LOCK(g_thread);
  G_UNLOCK(g_thread);
  thread->retval = thread->thread.func(thread->thread.data);
  return NULL;
}
GThread* g_thread_create_full(GThreadFunc func, gpointer data, gulong stack_size, gboolean joinable, gboolean bound, GThreadPriority priority, GError **error) {
  GRealThread* result;
  GError *local_error = NULL;
  g_return_val_if_fail(func, NULL);
  g_return_val_if_fail(priority >= G_THREAD_PRIORITY_LOW, NULL);
  g_return_val_if_fail(priority <= G_THREAD_PRIORITY_URGENT, NULL);
  result = g_new0(GRealThread, 1);
  result->thread.joinable = joinable;
  result->thread.priority = priority;
  result->thread.func = func;
  result->thread.data = data;
  result->private_data = NULL;
  G_LOCK(g_thread);
  G_THREAD_UF(thread_create, (g_thread_create_proxy, result, stack_size, joinable, bound, priority, &result->system_thread, &local_error));
  if (!local_error) {
      result->next = g_thread_all_threads;
      g_thread_all_threads = result;
  }
  G_UNLOCK(g_thread);
  if (local_error) {
      g_propagate_error(error, local_error);
      g_free(result);
      return NULL;
  }
  return (GThread*) result;
}
void g_thread_exit(gpointer retval) {
  GRealThread* real = (GRealThread*)g_thread_self();
  real->retval = retval;
  G_THREAD_CF(thread_exit, (void)0, ());
}
gpointer g_thread_join(GThread* thread) {
  GRealThread* real = (GRealThread*)thread;
  GRealThread *p, *t;
  void* retval;
  g_return_val_if_fail(thread, NULL);
  g_return_val_if_fail(thread->joinable, NULL);
  g_return_val_if_fail(!g_system_thread_equal(real->system_thread, zero_thread), NULL);
  G_THREAD_UF(thread_join, (&real->system_thread));
  retval = real->retval;
  G_LOCK(g_thread);
  for (t = g_thread_all_threads, p = NULL; t; p = t, t = t->next) {
      if (t == (GRealThread*) thread) {
          if (p) p->next = t->next;
          else g_thread_all_threads = t->next;
          break;
	  }
  }
  G_UNLOCK(g_thread);
  thread->joinable = 0;
  g_system_thread_assign(real->system_thread, zero_thread);
  g_free(thread);
  return retval;
}
void g_thread_set_priority(GThread* thread, GThreadPriority priority) {
  GRealThread* real = (GRealThread*)thread;
  g_return_if_fail(thread);
  g_return_if_fail(!g_system_thread_equal(real->system_thread, zero_thread));
  g_return_if_fail(priority >= G_THREAD_PRIORITY_LOW);
  g_return_if_fail(priority <= G_THREAD_PRIORITY_URGENT);
  thread->priority = priority;
  G_THREAD_CF(thread_set_priority, (void)0,(&real->system_thread, priority));
}
void g_mutex_init(GMutex *mutex) {
    g_static_mutex_init(mutex);
}
void g_mutex_clear(GMutex *mutex) {
    g_static_mutex_free(mutex);
}
void g_cond_init(GCond *cond) {
    g_cond_new();
}
void g_cond_clear(GCond *cond) {
    g_cond_free(cond);
}
GThread* g_thread_self(void) {
  GRealThread* thread = g_private_get(g_thread_specific_private);
  if (!thread) {
      thread = g_new0(GRealThread, 1);
      thread->thread.joinable = FALSE;
      thread->thread.priority = G_THREAD_PRIORITY_NORMAL;
      thread->thread.func = NULL;
      thread->thread.data = NULL;
      thread->private_data = NULL;
      if (g_thread_supported()) G_THREAD_UF(thread_self, (&thread->system_thread));
      g_private_set(g_thread_specific_private, thread);
      G_LOCK(g_thread);
      thread->next = g_thread_all_threads;
      g_thread_all_threads = thread;
      G_UNLOCK(g_thread);
  }
  return (GThread*)thread;
}
void g_static_rw_lock_init(GStaticRWLock* lock) {
  static const GStaticRWLock init_lock = G_STATIC_RW_LOCK_INIT;
  g_return_if_fail(lock);
  *lock = init_lock;
}
inline static void g_static_rw_lock_wait(GCond** cond, GStaticMutex* mutex) {
  if (!*cond) *cond = g_cond_new();
  g_cond_wait(*cond, g_static_mutex_get_mutex (mutex));
}
inline static void g_static_rw_lock_signal(GStaticRWLock* lock){
  if (lock->want_to_write && lock->write_cond) { g_cond_signal(lock->write_cond); }
  else if (lock->want_to_read && lock->read_cond) { g_cond_broadcast(lock->read_cond); }
}
void g_static_rw_lock_reader_lock(GStaticRWLock* lock) {
  g_return_if_fail(lock);
  if (!g_threads_got_initialized) return;
  g_static_mutex_lock(&lock->mutex);
  lock->want_to_read++;
  while(lock->have_writer || lock->want_to_write) g_static_rw_lock_wait(&lock->read_cond, &lock->mutex);
  lock->want_to_read--;
  lock->read_counter++;
  g_static_mutex_unlock(&lock->mutex);
}
gboolean g_static_rw_lock_reader_trylock(GStaticRWLock* lock) {
  gboolean ret_val = FALSE;
  g_return_val_if_fail(lock, FALSE);
  if (!g_threads_got_initialized) return TRUE;
  g_static_mutex_lock(&lock->mutex);
  if (!lock->have_writer && !lock->want_to_write) {
      lock->read_counter++;
      ret_val = TRUE;
  }
  g_static_mutex_unlock(&lock->mutex);
  return ret_val;
}
void g_static_rw_lock_reader_unlock(GStaticRWLock* lock) {
  g_return_if_fail(lock);
  if (!g_threads_got_initialized) return;
  g_static_mutex_lock(&lock->mutex);
  lock->read_counter--;
  if (lock->read_counter == 0) g_static_rw_lock_signal(lock);
  g_static_mutex_unlock(&lock->mutex);
}
void g_static_rw_lock_writer_lock(GStaticRWLock* lock) {
  g_return_if_fail(lock);
  if (!g_threads_got_initialized) return;
  g_static_mutex_lock(&lock->mutex);
  lock->want_to_write++;
  while(lock->have_writer || lock->read_counter) g_static_rw_lock_wait(&lock->write_cond, &lock->mutex);
  lock->want_to_write--;
  lock->have_writer = TRUE;
  g_static_mutex_unlock(&lock->mutex);
}
gboolean g_static_rw_lock_writer_trylock(GStaticRWLock* lock) {
  gboolean ret_val = FALSE;
  g_return_val_if_fail(lock, FALSE);
  if (!g_threads_got_initialized) return TRUE;
  g_static_mutex_lock (&lock->mutex);
  if (!lock->have_writer && !lock->read_counter) {
      lock->have_writer = TRUE;
      ret_val = TRUE;
  }
  g_static_mutex_unlock(&lock->mutex);
  return ret_val;
}
void g_static_rw_lock_writer_unlock(GStaticRWLock* lock) {
  g_return_if_fail (lock);
  if (!g_threads_got_initialized) return;
  g_static_mutex_lock(&lock->mutex);
  lock->have_writer = FALSE;
  g_static_rw_lock_signal(lock);
  g_static_mutex_unlock(&lock->mutex);
}
void g_static_rw_lock_free(GStaticRWLock* lock) {
  g_return_if_fail (lock);
  if (lock->read_cond) {
      g_cond_free(lock->read_cond);
      lock->read_cond = NULL;
  }
  if (lock->write_cond) {
      g_cond_free(lock->write_cond);
      lock->write_cond = NULL;
  }
  g_static_mutex_free (&lock->mutex);
}
void g_thread_foreach(GFunc thread_func, gpointer user_data) {
  GSList *slist = NULL;
  GRealThread *thread;
  g_return_if_fail(thread_func != NULL);
  G_LOCK(g_thread);
  for (thread = g_thread_all_threads; thread; thread = thread->next) slist = g_slist_prepend(slist, thread);
  G_UNLOCK(g_thread);
  while (slist) {
      GSList *node = slist;
      slist = node->next;
      G_LOCK(g_thread);
      for (thread = g_thread_all_threads; thread; thread = thread->next) if (thread == node->data) break;
      G_UNLOCK (g_thread);
      if (thread) thread_func(thread, user_data);
      g_slist_free_1(node);
    }
}
gboolean g_thread_get_initialized() {
  return g_threads_got_initialized;
}