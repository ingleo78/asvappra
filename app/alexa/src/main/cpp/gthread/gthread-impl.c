#include "../glib/glib.h"
#include "../glib/gthread.h"
#include "../glib/gthreadprivate.h"

#ifdef G_THREADS_ENABLED
static GSystemThread zero_thread;
static gboolean thread_system_already_initialized = FALSE;
static gint g_thread_priority_map [G_THREAD_PRIORITY_URGENT + 1];
#ifndef PRIORITY_LOW_VALUE
# define PRIORITY_LOW_VALUE 0
#endif
#ifndef PRIORITY_URGENT_VALUE
# define PRIORITY_URGENT_VALUE 0
#endif
#ifndef PRIORITY_NORMAL_VALUE
# define PRIORITY_NORMAL_VALUE	((PRIORITY_LOW_VALUE * 6 + PRIORITY_URGENT_VALUE * 4) / 10)
#endif
#ifndef PRIORITY_HIGH_VALUE
# define PRIORITY_HIGH_VALUE   ((PRIORITY_NORMAL_VALUE + PRIORITY_URGENT_VALUE * 2) / 3)
#endif
void g_mem_init (void);
void g_messages_init (void);
void g_convert_init (void);
void g_rand_init (void);
void g_main_thread_init (void);
struct _GMutexDebugInfo {
  gchar *location;
  GSystemThread owner;
};
#define GMutexDebugInfo struct _GMutexDebugInfo
#define G_MUTEX_SIZE 0
#define G_MUTEX_DEBUG_INFO(mutex) 	(((GMutexDebugInfo*)(((char*)mutex)+G_MUTEX_SIZE)))
static GMutex* g_mutex_new_errorcheck_impl(void) {
  g_thread_functions_for_glib_use.mutex_new();
  GMutex *retval = g_thread_functions_for_glib_use.mutex_new();
  GMutexDebugInfo *info;
  retval = g_realloc(retval, G_MUTEX_SIZE + sizeof(GMutexDebugInfo));
  info = G_MUTEX_DEBUG_INFO(retval);
  g_system_thread_assign(info->owner, zero_thread);
  info->location = "invalid";
  return retval;
}
static void g_mutex_lock_errorcheck_impl(GMutex *mutex, const gulong magic, gchar* const location) {
  GMutexDebugInfo *info = G_MUTEX_DEBUG_INFO (mutex);
  gchar *loc = (magic == G_MUTEX_DEBUG_MAGIC) ? location : "unknown";
  GSystemThread self;
  g_thread_functions_for_glib_use.thread_self (&self);
  if (g_system_thread_equal (info->owner, self))
      g_error ("Trying to recursively lock a mutex at '%s', previously locked at '%s'", loc, info->location);
  g_thread_functions_for_glib_use.mutex_lock(mutex);
  g_system_thread_assign (info->owner, self);
  info->location = loc;
}
static gboolean g_mutex_trylock_errorcheck_impl(GMutex *mutex, const gulong magic, gchar* const location) {
  GMutexDebugInfo *info = G_MUTEX_DEBUG_INFO (mutex);
  gchar *loc = (magic == G_MUTEX_DEBUG_MAGIC) ? location : "unknown";
  GSystemThread self;
  g_thread_functions_for_glib_use.thread_self(&self);
  if (g_system_thread_equal(info->owner, self)) g_error("Trying to recursivly lock a mutex at '%s', previously locked at '%s'", loc, info->location);
  if (!g_thread_functions_for_glib_use.mutex_trylock(mutex)) return FALSE;
  g_system_thread_assign(info->owner, self);
  info->location = loc;
  return TRUE;
}
static void g_mutex_unlock_errorcheck_impl(GMutex *mutex, const gulong magic, gchar* const location) {
  GMutexDebugInfo *info = G_MUTEX_DEBUG_INFO(mutex);
  gchar *loc = (magic == G_MUTEX_DEBUG_MAGIC) ? location : "unknown";
  GSystemThread self;
  g_thread_functions_for_glib_use.thread_self(&self);
  if (g_system_thread_equal(info->owner, zero_thread)) g_error("Trying to unlock an unlocked mutex at '%s'", loc);
  if (!g_system_thread_equal (info->owner, self)) {
      g_warning("Trying to unlock a mutex at '%s', previously locked by a different thread at '%s'", loc, info->location);
  }
  g_system_thread_assign(info->owner, zero_thread);
  info->location = NULL;
  g_thread_functions_for_glib_use.mutex_unlock(mutex);
}
static void g_mutex_free_errorcheck_impl(GMutex *mutex, const gulong magic, gchar* const location) {
  GMutexDebugInfo *info = G_MUTEX_DEBUG_INFO(mutex);
  gchar *loc = (magic == G_MUTEX_DEBUG_MAGIC) ? location : "unknown";
  if (info && !g_system_thread_equal(info->owner, zero_thread))
      g_error("Trying to free a locked mutex at '%s', which was previously locked at '%s'", loc, info->location);
  g_thread_functions_for_glib_use.mutex_free(mutex);
}
static void g_cond_wait_errorcheck_impl(GCond *cond, GMutex *mutex, const gulong magic, gchar* const location) {
  GMutexDebugInfo *info = G_MUTEX_DEBUG_INFO(mutex);
  gchar *loc = (magic == G_MUTEX_DEBUG_MAGIC) ? location : "unknown";
  GSystemThread self;
  g_thread_functions_for_glib_use.thread_self(&self);
  if (g_system_thread_equal(info->owner, zero_thread)) g_error("Trying to use an unlocked mutex in g_cond_wait() at '%s'", loc);
  if (!g_system_thread_equal(info->owner, self))g_error("Trying to use a mutex locked by another thread in g_cond_wait() at '%s'", loc);
  g_system_thread_assign(info->owner, zero_thread);
  loc = info->location;
  g_thread_functions_for_glib_use.cond_wait(cond, mutex);
  g_system_thread_assign(info->owner, self);
  info->location = loc;
}
static gboolean g_cond_timed_wait_errorcheck_impl(GCond *cond, GMutex *mutex, GTimeVal *end_time, const gulong magic, gchar* const location) {
  GMutexDebugInfo *info = G_MUTEX_DEBUG_INFO(mutex);
  gchar *loc = (magic == G_MUTEX_DEBUG_MAGIC) ? location : "unknown";
  gboolean retval;
  GSystemThread self;
  g_thread_functions_for_glib_use.thread_self(&self);
  if (g_system_thread_equal(info->owner, zero_thread)) g_error ("Trying to use an unlocked mutex in g_cond_timed_wait() at '%s'", loc);
  if (!g_system_thread_equal(info->owner, self)) g_error("Trying to use a mutex locked by another thread in g_cond_timed_wait() at '%s'", loc);
  g_system_thread_assign(info->owner, zero_thread);
  loc = info->location;
  retval = g_thread_functions_for_glib_use.cond_timed_wait(cond, mutex, end_time);
  g_system_thread_assign(info->owner, self);
  info->location = loc;
  return retval;
}
#undef g_thread_init
void g_thread_init_with_errorcheck_mutexes(GThreadFunctions* init) {
  GThreadFunctions errorcheck_functions;
  if (init) g_error("Errorcheck mutexes can only be used for native thread implementations. Sorry." );
#ifdef HAVE_G_THREAD_IMPL_INIT
  g_thread_impl_init();
#endif
  errorcheck_functions = g_thread_functions_for_glib_use;
  errorcheck_functions.mutex_new = g_mutex_new_errorcheck_impl;
  errorcheck_functions.mutex_lock = (void (*)(GMutex*))g_mutex_lock_errorcheck_impl;
  errorcheck_functions.mutex_trylock = (gboolean (*)(GMutex*))g_mutex_trylock_errorcheck_impl;
  errorcheck_functions.mutex_unlock = (void (*)(GMutex*))g_mutex_unlock_errorcheck_impl;
  errorcheck_functions.mutex_free = (void (*)(GMutex*))g_mutex_free_errorcheck_impl;
  errorcheck_functions.cond_wait = (void (*)(GCond*, GMutex*))g_cond_wait_errorcheck_impl;
  errorcheck_functions.cond_timed_wait = (gboolean (*)(GCond*, GMutex*, GTimeVal*))g_cond_timed_wait_errorcheck_impl;
  g_thread_init(&errorcheck_functions);
}
void g_thread_init(GThreadFunctions* init) {
  gboolean supported;
  if (thread_system_already_initialized) {
      if (init != NULL) g_warning("GThread system already initialized, ignoring custom thread implementation.");
      return;
  }
  thread_system_already_initialized = TRUE;
  if (init == NULL) {
  #ifdef HAVE_G_THREAD_IMPL_INIT
      g_thread_impl_init();
  #endif
      init = &g_thread_functions_for_glib_use;
  } else g_thread_use_default_impl = FALSE;
  g_thread_functions_for_glib_use = *init;
  if (g_thread_gettime) g_thread_gettime = gettime;
  supported = (init->mutex_new && init->mutex_lock && init->mutex_trylock && init->mutex_unlock && init->mutex_free && init->cond_new && init->cond_signal &&
	           init->cond_broadcast && init->cond_wait && init->cond_timed_wait && init->cond_free && init->private_new && init->private_get && init->private_set &&
	           init->thread_create && init->thread_yield && init->thread_join && init->thread_exit && init->thread_set_priority && init->thread_self);
  if (!supported) {
      if (g_thread_use_default_impl) g_error("Threads are not supported on this platform.");
      else g_error("The supplied thread function vector is invalid.");
  }
  g_thread_priority_map [G_THREAD_PRIORITY_LOW] = PRIORITY_LOW_VALUE;
  g_thread_priority_map [G_THREAD_PRIORITY_NORMAL] = PRIORITY_NORMAL_VALUE;
  g_thread_priority_map [G_THREAD_PRIORITY_HIGH] = PRIORITY_HIGH_VALUE;
  g_thread_priority_map [G_THREAD_PRIORITY_URGENT] = PRIORITY_URGENT_VALUE;
  g_thread_init_glib ();
}
#else
void g_thread_init(GThreadFunctions* init) {
  g_error ("GLib thread support is disabled.");
}
void g_thread_init_with_errorcheck_mutexes(GThreadFunctions* init) {
  g_error ("GLib thread support is disabled.");
}
#endif