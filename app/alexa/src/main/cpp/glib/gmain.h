#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_MAIN_H__
#define __G_MAIN_H__

#include "../gio/gioenums.h"
#include "gmacros.h"
#include "gpoll.h"
#include "gtypes.h"
#include "gslist.h"
#include "gthread.h"
#include "glibconfig.h"

G_BEGIN_DECLS
typedef union _GMutex GMutex;
typedef struct _GMainContext GMainContext;
typedef struct _GMainLoop GMainLoop;
typedef struct _GSource GSource;
typedef struct _GSourcePrivate GSourcePrivate;
typedef struct _GSourceCallbackFuncs GSourceCallbackFuncs;
typedef struct _GSourceFuncs GSourceFuncs;
typedef struct _GTimeVal GTimeVal;
typedef struct _GCond GCond;
typedef void (*GDestroyNotify)(gpointer data);
typedef int (*GSourceFunc)(gpointer data);
typedef void (*GChildWatchFunc)(GPid pid, gint status, gpointer data);
struct _GSource {
  gpointer callback_data;
  GSourceCallbackFuncs *callback_funcs;
  GSourceFuncs *source_funcs;
  guint ref_count;
  GMainContext *context;
  gint priority;
  guint flags;
  guint source_id;
  GSList *poll_fds;
  GSource *prev;
  GSource *next;
  char *name;
  GSourcePrivate *priv;
};
struct _GSourceCallbackFuncs {
  void (*ref)(gpointer cb_data);
  void (*unref)(gpointer cb_data);
  void (*get)(gpointer cb_data, GSource *source, GSourceFunc *func, gpointer *data);
};
typedef void (*GSourceDummyMarshal)(void);
struct _GSourceFuncs {
  int (*prepare)(GSource *source, gint *timeout_);
  int (*check)(GSource *source);
  int (*dispatch)(GSource *source, GSourceFunc callback, gpointer user_data);
  void (*finalize)(GSource *source);
  GSourceFunc closure_callback;
  GSourceDummyMarshal closure_marshal;
};
#define G_PRIORITY_HIGH  -100
#define G_PRIORITY_DEFAULT  0
#define G_PRIORITY_HIGH_IDLE  100
#define G_PRIORITY_DEFAULT_IDLE  200
#define G_PRIORITY_LOW  300
GMainContext *g_main_context_new(void);
GMainContext *g_main_context_ref(GMainContext *context);
GMainContext *g_main_context_ref_thread_default(void);
void g_main_context_unref(GMainContext *context);
GMainContext *g_main_context_default(void);
int g_main_context_iteration(GMainContext *context, int may_block);
int g_main_context_pending(GMainContext *context);
GSource *g_main_context_find_source_by_id(GMainContext *context, guint source_id);
GSource *g_main_context_find_source_by_user_data(GMainContext *context, gpointer user_data);
GSource *g_main_context_find_source_by_funcs_user_data(GMainContext *context, GSourceFuncs *funcs, gpointer user_data);
void g_main_context_wakeup(GMainContext *context);
int g_main_context_acquire(GMainContext *context);
void g_main_context_release(GMainContext *context);
int g_main_context_is_owner(GMainContext *context);
int g_main_context_wait(GMainContext *context, GCond *cond, GMutex *mutex);
int g_main_context_prepare(GMainContext *context, gint *priority);
gint g_main_context_query(GMainContext *context, gint max_priority, gint *timeout_, GPollFD *fds, gint n_fds);
gint g_main_context_check(GMainContext *context, gint max_priority, GPollFD *fds, gint n_fds);
void g_main_context_dispatch(GMainContext *context);
void g_main_context_set_poll_func(GMainContext *context, GPollFunc func);
GPollFunc g_main_context_get_poll_func(GMainContext *context);
void g_main_context_add_poll(GMainContext *context, GPollFD *fd, gint priority);
void g_main_context_remove_poll(GMainContext *context, GPollFD *fd);
gint g_main_depth(void);
GSource *g_main_current_source(void);
void g_main_context_push_thread_default(GMainContext *context);
void g_main_context_pop_thread_default(GMainContext *context);
GMainContext *g_main_context_get_thread_default(void);
GMainLoop *g_main_loop_new(GMainContext *context, int is_running);
void g_main_loop_run(GMainLoop *loop);
void g_main_loop_quit(GMainLoop *loop);
GMainLoop *g_main_loop_ref(GMainLoop *loop);
void g_main_loop_unref(GMainLoop *loop);
int g_main_loop_is_running(GMainLoop *loop);
GMainContext *g_main_loop_get_context(GMainLoop *loop);
void g_source_set_ready_time(GSource *source, gint64 ready_time);
gint64 g_source_get_ready_time(GSource *source);
GSource *g_source_new(GSourceFuncs *source_funcs, guint struct_size);
GSource *g_source_ref(GSource *source);
void g_source_unref(GSource *source);
guint g_source_attach(GSource *source, GMainContext *context);
void g_source_destroy(GSource *source);
void g_source_set_priority(GSource *source, gint priority);
gint g_source_get_priority(GSource *source);
void g_source_set_can_recurse(GSource *source, int can_recurse);
int g_source_get_can_recurse(GSource *source);
guint g_source_get_id(GSource *source);
GMainContext *g_source_get_context(GSource *source);
void g_source_set_callback(GSource *source, GSourceFunc func, gpointer data, GDestroyNotify notify);
void g_source_set_funcs(GSource *source, GSourceFuncs *funcs);
int g_source_is_destroyed(GSource *source);
void g_source_set_name(GSource *source, const char *name);
G_CONST_RETURN char* g_source_get_name(GSource *source);
void g_source_set_name_by_id(guint tag, const char *name);
void g_source_set_callback_indirect(GSource *source, gpointer callback_data, GSourceCallbackFuncs *callback_funcs);
void g_source_add_poll(GSource *source, GPollFD *fd);
void g_source_remove_poll(GSource *source, GPollFD *fd);
void g_source_add_child_source(GSource *source, GSource *child_source);
void g_source_remove_child_source(GSource *source, GSource *child_source);
#ifndef G_DISABLE_DEPRECATED
void g_source_get_current_time(GSource *source, GTimeVal *timeval);
#endif
gint64 g_source_get_time(GSource *source);
gpointer g_source_add_unix_fd(GSource *source, gint fd, GIOCondition events);
void g_source_modify_unix_fd(GSource *source, gpointer tag, GIOCondition new_events);
void g_source_remove_unix_fd(GSource *source, gpointer tag);
GIOCondition g_source_query_unix_fd(GSource *source, gpointer tag);
GSource *g_idle_source_new(void);
GSource *g_child_watch_source_new(GPid pid);
GSource *g_timeout_source_new(guint interval);
GSource *g_timeout_source_new_seconds(guint interval);
void g_get_current_time(GTimeVal *result);
gint64 g_get_monotonic_time(void);
gint64 g_get_real_time(void);
#ifndef G_DISABLE_DEPRECATED
#define g_main_new(is_running) g_main_loop_new(NULL, is_running)
#define g_main_run(loop) g_main_loop_run(loop)
#define g_main_quit(loop) g_main_loop_quit(loop)
#define g_main_destroy(loop) g_main_loop_unref(loop)
#define g_main_is_running(loop) g_main_loop_is_running(loop)
#define g_main_iteration(may_block) g_main_context_iteration(NULL, may_block)
#define g_main_pending() g_main_context_pending(NULL)
#define g_main_set_poll_func(func) g_main_context_set_poll_func(NULL, func)
#endif
int g_source_remove(guint tag);
int g_source_remove_by_user_data(gpointer user_data);
int g_source_remove_by_funcs_user_data(GSourceFuncs *funcs, gpointer user_data);
guint g_timeout_add_full(gint priority, guint interval, GSourceFunc function, gpointer data, GDestroyNotify  notify);
guint g_timeout_add(guint interval, GSourceFunc function, gpointer data);
guint g_timeout_add_seconds_full(gint priority, guint interval, GSourceFunc function, gpointer data, GDestroyNotify notify);
guint g_timeout_add_seconds(guint interval, GSourceFunc function, gpointer data);
guint g_child_watch_add_full(gint priority, GPid pid, GChildWatchFunc function, gpointer data, GDestroyNotify notify);
guint g_child_watch_add(GPid pid, GChildWatchFunc function, gpointer data);
guint g_idle_add(GSourceFunc function, gpointer data);
guint g_idle_add_full(gint priority, GSourceFunc function, gpointer data, GDestroyNotify notify);
int g_idle_remove_by_data(gpointer data);
void g_main_context_invoke_full(GMainContext *context, gint priority, GSourceFunc function, gpointer data, GDestroyNotify notify);
void g_main_context_invoke(GMainContext *context, GSourceFunc function, gpointer data);
extern GSourceFuncs g_timeout_funcs;
extern GSourceFuncs g_child_watch_funcs;
extern GSourceFuncs g_idle_funcs;
G_END_DECLS

#endif