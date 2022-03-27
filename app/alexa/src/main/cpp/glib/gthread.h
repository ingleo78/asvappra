#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_THREAD_H__
#define __G_THREAD_H__

#include "../gio/giotypes.h"
#include "gmacros.h"
#include "gerror.h"
#include "gatomic.h"
#include "gquark.h"
#include "gmessages.h"

G_BEGIN_DECLS
extern GQuark g_thread_error_quark (void);
#define G_THREAD_ERROR g_thread_error_quark ()
#define G_PRIVATE_INIT(notify) { NULL, (notify), { NULL, NULL } }
typedef enum {
  G_THREAD_ERROR_AGAIN
} GThreadError;
struct _GPrivate {
  gpointer p;
  GDestroyNotify notify;
  gpointer future[2];
};
union _GMutex {
  gpointer p;
  guint i[2];
};
struct _GCond {
  gpointer p;
  guint i[2];
};
typedef gpointer (*GThreadFunc)(gpointer data);
typedef enum {
  G_THREAD_PRIORITY_LOW,
  G_THREAD_PRIORITY_NORMAL,
  G_THREAD_PRIORITY_HIGH,
  G_THREAD_PRIORITY_URGENT
} GThreadPriority;
typedef struct _GThread GThread;
struct  _GThread {
  GThreadFunc func;
  gpointer data;
  gboolean joinable;
  GThreadPriority priority;
};
void g_thread_fail(void);
typedef struct _GCond GCond;
typedef union _GMutex GMutex;
typedef struct _GPrivate GPrivate;
typedef struct _GStaticPrivate GStaticPrivate;
typedef struct _GThreadFunctions GThreadFunctions;
typedef struct _GRecMutex GRecMutex;
typedef union _GMutex* GStaticMutex;
struct _GRecMutex {
    gpointer p;
    guint i[2];
};
struct _GThreadFunctions {
  GMutex* (*mutex_new)(void);
  void (*mutex_lock)(GMutex *mutex);
  gboolean (*mutex_trylock)(GMutex *mutex);
  void (*mutex_unlock)(GMutex *mutex);
  void (*mutex_free)(GMutex *mutex);
  GCond* (*cond_new)(void);
  void (*cond_signal)(GCond *cond);
  void (*cond_broadcast)(GCond *cond);
  void (*cond_wait)(GCond *cond, GMutex *mutex);
  gboolean (*cond_timed_wait)(GCond *cond, GMutex *mutex, GTimeVal *end_time);
  void (*cond_free)(GCond *cond);
  GPrivate* (*private_new)(GDestroyNotify destructor);
  gpointer (*private_get)(GPrivate *private_key);
  void (*private_set)(GPrivate *private_key, gpointer data);
  void (*thread_create)(GThreadFunc func, gpointer data, gulong stack_size, gboolean joinable, gboolean bound, GThreadPriority priority,
                        gpointer thread, GError **error);
  void (*thread_yield)(void);
  void (*thread_join)(gpointer thread);
  void (*thread_exit)(void);
  void (*thread_set_priority)(gpointer thread, GThreadPriority priority);
  void (*thread_self)(gpointer thread);
  gboolean (*thread_equal)(gpointer thread1, gpointer thread2);
};
GLIB_VAR GThreadFunctions g_thread_functions_for_glib_use;
GLIB_VAR gboolean g_thread_use_default_impl;
GLIB_VAR gboolean g_threads_got_initialized;
GLIB_VAR guint64 (*g_thread_gettime)(void);
void g_thread_init(GThreadFunctions *vtable);
void g_thread_init_with_errorcheck_mutexes(GThreadFunctions* vtable);
static unsigned long long gettime(void);
gboolean g_thread_get_initialized(void);
#define G_MUTEX_DEBUG_MAGIC 0xf8e18ad7
#ifndef G_ERRORCHECK_MUTEXES
#define g_thread_init(vtable) g_thread_init_with_errorcheck_mutexes(vtable)
#endif
#if defined(G_THREADS_ENABLED) && defined(G_THREADS_MANDATORY)
#define g_thread_supported() 1
#else
#define g_thread_supported() (g_threads_got_initialized)
#endif
GMutex *g_static_mutex_get_mutex_impl(GMutex **mutex);
#define g_static_mutex_get_mutex_impl_shortcut(mutex) (g_atomic_pointer_get(mutex) ? *(mutex) : g_static_mutex_get_mutex_impl(mutex))
#define G_THREAD_UF(op, arglist) (*g_thread_functions_for_glib_use . op) arglist
#define G_THREAD_CF(op, fail, arg) (g_thread_supported() ? G_THREAD_UF(op, arg) : (fail))
#define G_THREAD_ECF(op, fail, mutex, type) \
(g_thread_supported() ? ((type(*)(GMutex*, const gulong, gchar conts*))(*g_thread_functions_for_glib_use . op))(mutex, G_MUTEX_DEBUG_MAGIC, G_STRLOC) : (fail))
#ifndef G_ERROR_CHECK_MUTEXES
#define g_mutex_lock(mutex) G_THREAD_CF(mutex_lock, (void)0, (mutex))
#define g_mutex_trylock(mutex) G_THREAD_CF(mutex_trylock, TRUE, (mutex))
#define g_mutex_unlock(mutex) G_THREAD_CF(mutex_free, (void)0, (mutex))
#define g_mutex_free(mutex) G_THREAD_CF(mutex_free, (void)0, (mutex))
#define g_cond_wait(cond, mutex) G_THREAD_CF(cond_wait, (void)0, (cond, mutex))
#define g_cond_timed_wait(cond, mutex, abs_time) G_THREAD_CF(cond_timed_wait, TRUE, (cond, mutex, abs_time))
#else
#define g_mutex_lock(mutex) G_THREAD_ECF(mutex_lock, (void)0, void)
#define g_mutex_trylock(mutex) G_THREAD_ECF(mutex_trylock, TRUE, (mutex), gboolean)
#define g_mutex_unlock(mutex) G_THREAD_ECF(mutex_unlock, (void)0, (mutex), void)
#define g_mutex_free(mutex) G_THREAD_ECF(mutex_free, (void)0, (mutex), void)
#define g_cond_wait(cond, mutex) \
(g_thread_supported() ? ((void(*)(GCond*, GMutex*, gulong, gchar*))g_thread_functions_for_glib_use.cond_wait)(cond, mutex, G_MUTEX_DEBUG_MAGIC, G_STRLOC) : (void)0)
#define g_cond_timed_wait(cond, mutex) \
(g_thread_supported() = ((gboolean(*)(GCond*, GMutex*, GTimeVal*, gulong, gchar*))g_thread_functions_for_glib_use.cond_timed_wait)(cond, mutexx, abs_time, G_MUTEX_DEBUG_MAGIC, G_STRLOC) : TRUE)
#endif
#define g_mutex_new() G_THREAD_UF(mutex_new, ())
#define g_cond_new() G_THREAD_UF(cond_new, ())
#define g_cond_signal(cond) G_THREAD_CF(cond_signal, (void)0, (cond))
#define g_cond_broadcast(cond) G_THREAD_CF(cond_broadcast, (void)0, (cond))
#define g_cond_free(cond) G_THREAD_CF(cond_free, (void)0, (cond))
#define g_private_new(destructor) G_THREAD_UF(private_new, (destructor))
#define g_private_get(private_key) G_THREAD_CF(private_get, ((gpointer)private_key), (private_key))
#define g_private_set(private_key, value) G_THREAD_CF(private_set, (void)(GPrivate*)(value), (private_key, value))
#define g_thread_yield() G_THREAD_CF(thread_yield, (void)0, ())
#define g_thread_create(func, data, joinable, error) (g_thread_create_full(func, data, 0, joinable, FALSE, G_THREAD_PRIORITY_NORMAL, error))
GThread* g_thread_create_full(GThreadFunc func, gpointer data, gulong stack_size, gboolean joinable, gboolean bound, GThreadPriority priority, GError **error);
GThread* g_thread_self(void);
void g_thread_exit(gpointer retval);
gpointer g_thread_join(GThread *thread);
void g_thread_set_priority(GThread *thread, GThreadPriority priority);
void g_mutex_init(GMutex *mutex);
void g_mutex_clear(GMutex *mutex);
void g_cond_init(GCond *cond);
void g_cond_clear(GCond *cond);
#define g_static_mutex_lock(mutex) g_mutex_lock(g_static_mutex_get_mutex(mutex))
#define g_static_mutex_trylock(mutex) g_mutex_trylock(g_static_mutex_get_mutex(mutex))
#define g_static_mutex_unlock(mutex) g_mutex_unlock(g_static_mutex_get_mutex(mutex))
void g_static_mutex_init(GStaticMutex *mutex);
void g_static_mutex_free(GStaticMutex *mutex);
struct _GStaticPrivate {
  guint index;
};
#define G_STATIC_PRIVATE_INIT { 0 }
void  g_static_private_init(GStaticPrivate *private_key);
gpointer g_static_private_get(GStaticPrivate *private_key);
void g_static_private_set(GStaticPrivate *private_key, gpointer data, GDestroyNotify notify);
void g_static_private_free(GStaticPrivate *private_key);
typedef struct _GStaticRecMutex GStaticRecMutex;
typedef union _GSystemThread GSystemThread;
union _GSystemThread {
#ifdef _WIN64
    char   data[4];
#else
    char   data[8];
#endif
    double dummy_double;
    void  *dummy_pointer;
    long   dummy_long;
};
struct _GStaticRecMutex {
  GStaticMutex mutex;
  guint depth;
  GSystemThread owner;
};
#define G_STATIC_REC_MUTEX_INIT { G_STATIC_MUTEX_INIT }
void g_static_rec_mutex_init(GStaticRecMutex *mutex);
void g_static_rec_mutex_lock(GStaticRecMutex *mutex);
gboolean g_static_rec_mutex_trylock(GStaticRecMutex *mutex);
void g_static_rec_mutex_unlock(GStaticRecMutex *mutex);
void g_static_rec_mutex_lock_full(GStaticRecMutex *mutex, guint depth);
guint g_static_rec_mutex_unlock_full(GStaticRecMutex *mutex);
void g_static_rec_mutex_free(GStaticRecMutex *mutex);
typedef struct _GStaticRWLock GStaticRWLock;
struct _GStaticRWLock {
  GStaticMutex mutex;
  GCond *read_cond;
  GCond *write_cond;
  guint read_counter;
  gboolean have_writer;
  guint want_to_read;
  guint want_to_write;
};
#define G_STATIC_RW_LOCK_INIT { G_STATIC_MUTEX_INIT, NULL, NULL, 0, FALSE, 0, 0 }
void g_static_rw_lock_init(GStaticRWLock* lock);
void g_static_rw_lock_reader_lock(GStaticRWLock* lock);
gboolean g_static_rw_lock_reader_trylock(GStaticRWLock* lock);
void g_static_rw_lock_reader_unlock(GStaticRWLock* lock);
void g_static_rw_lock_writer_lock(GStaticRWLock* lock);
gboolean g_static_rw_lock_writer_trylock(GStaticRWLock* lock);
void g_static_rw_lock_writer_unlock(GStaticRWLock* lock);
void g_static_rw_lock_free(GStaticRWLock* lock);
void g_thread_foreach(GFunc thread_func, gpointer user_data);
typedef enum {
  G_ONCE_STATUS_NOTCALLED,
  G_ONCE_STATUS_PROGRESS,
  G_ONCE_STATUS_READY  
} GOnceStatus;
typedef struct _GOnce GOnce;
struct _GOnce {
  volatile GOnceStatus status;
  volatile gpointer retval;
};
#define G_ONCE_INIT { G_ONCE_STATUS_NOTCALLED, NULL }
gpointer g_once_impl (GOnce *once, GThreadFunc func, gpointer arg);
#ifdef G_ATOMIC_OP_MEMORY_BARRIER_NEEDED
# define g_once(once, func, arg) g_once_impl((once), (func), (arg))
#else
# define g_once(once, func, arg) (((once)->status == G_ONCE_STATUS_READY) ? (once)->retval : g_once_impl ((once), (func), (arg)))
#endif
static inline gboolean g_once_init_enter(volatile gsize *value_location);
gboolean g_once_init_enter_impl(volatile gsize *value_location);
void g_once_init_leave(volatile gsize *value_location, gsize initialization_value);
#if defined (G_CAN_INLINE) || defined (__G_THREAD_C__)
static inline gboolean g_once_init_enter(volatile gsize *value_location) {
  if G_LIKELY((gpointer) g_atomic_pointer_get (value_location) != NULL) return FALSE;
  else return g_once_init_enter_impl (value_location);
}
#endif
extern void glib_dummy_decl(void);
#define G_LOCK_NAME(name) g__ ## name ## _lock
#ifndef  G_THREADS_ENABLED
#define G_LOCK_DEFINE(name) GStaticMutex G_LOCK_NAME (name) = G_STATIC_MUTEX_INIT
#define G_LOCK_DEFINE_STATIC(name) static G_LOCK_DEFINE (name)
#define G_LOCK_EXTERN(name) extern GStaticMutex G_LOCK_NAME (name)
#ifndef G_DEBUG_LOCKS
#define G_LOCK(name) G_STMT_START{ \
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "file %s: line %d (%s): locking: %s ", __FILE__, __LINE__, G_STRFUNC, #name); \
    g_static_mutex_lock(&G_LOCK_NAME(name)); \
} G_STMT_END
#define G_UNLOCK(name) G_STMT_START{ \
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "file %s: line %d (%s): unlocking: %s ", __FILE__, __LINE__, G_STRFUNC, #name); \
    g_static_mutex_unlock(&G_LOCK_NAME(name)); \
} G_STMT_END
#define G_TRYLOCK(name) \
    (g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "file %s: line %d (%s): try locking: %s ", __FILE__, __LINE__, G_STRFUNC, #name), \
           g_static_mutex_trylock(&G_LOCK_NAME(name)))
#else
#define G_LOCK(name) g_static_mutex_lock(&G_LOCK_NAME(name))
#define G_UNLOCK(name) g_static_mutex_unlock(&G_LOCK_NAME(name))
#define G_TRYLOCK(name) g_static_mutex_trylock(&G_LOCK_NAME(name))
#endif
#else
#define G_LOCK_DEFINE_STATIC(name) extern void glib_dummy_decl(void)
#define G_LOCK_DEFINE(name) extern void glib_dummy_decl(void)
#define G_LOCK_EXTERN(name) extern void glib_dummy_decl(void)
#define G_LOCK(name)
#define G_UNLOCK(name)
#define G_TRYLOCK(name) (TRUE)
#endif
G_END_DECLS
#endif