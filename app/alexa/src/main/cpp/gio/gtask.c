#include <string.h>
#include "../glib/glib-private.h"
#include "../glib/glibintl.h"
#include "../glib/gmain.h"
#include "../glib/gthreadpool.h"
#include "../glib/gstrfuncs.h"
#include "../glib/gtestutils.h"
#include "../dbus/test/test-utils-glib.h"
#include "config.h"
#include "gasyncresult.h"
#include "gcancellable.h"
#include "gio_trace.h"
#include "gtask.h"
#include "gasyncresult.h"
#include "gcancellable.h"

struct _GTask {
  GObject parent_instance;
  gpointer source_object;
  gpointer source_tag;
  gchar *name;
  gpointer task_data;
  GDestroyNotify task_data_destroy;
  GMainContext *context;
  gint64 creation_time;
  gint priority;
  GCancellable *cancellable;
  GAsyncReadyCallback callback;
  gpointer callback_data;
  GTaskThreadFunc task_func;
  GMutex lock;
  GCond cond;
  gboolean thread_cancelled;
  gboolean thread_complete : 1;
  gboolean return_on_cancel : 1;
  gboolean : 0;
  gboolean completed : 1;
  gboolean had_error : 1;
  gboolean result_set : 1;
  gboolean ever_returned : 1;
  gboolean : 0;
  gboolean check_cancellable : 1;
  gboolean synchronous : 1;
  gboolean blocking_other_task : 1;
  GError *error;
  union {
    gpointer pointer;
    gssize   size;
    gboolean boolean;
  } result;
  GDestroyNotify result_destroy;
};
#define G_TASK_IS_THREADED(task) ((task)->task_func != NULL)
struct _GTaskClass {
  GObjectClass parent_class;
};
typedef enum {
  PROP_COMPLETED = 1,
} GTaskProperty;
static void g_task_async_result_iface_init(GAsyncResultIface *iface);
static void g_task_thread_pool_init(void);
G_DEFINE_TYPE_WITH_CODE(GTask, g_task, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_ASYNC_RESULT, g_task_async_result_iface_init); g_task_thread_pool_init();)
static GThreadPool *task_pool;
static GMutex task_pool_mutex;
static GPrivate task_private = G_PRIVATE_INIT(NULL);
static GSource *task_pool_manager;
static guint64 task_wait_time;
static gint tasks_running;
#define G_TASK_POOL_SIZE 10
#define G_TASK_WAIT_TIME_BASE 100000
#define G_TASK_WAIT_TIME_MULTIPLIER 1.03
#define G_TASK_WAIT_TIME_MAX_POOL_SIZE 330
static void g_task_init(GTask *task) {
  task->check_cancellable = TRUE;
}
static void g_task_finalize(GObject *object) {
  GTask *task = G_TASK (object);
  g_clear_object(&task->source_object);
  g_clear_object(&task->cancellable);
  g_free (task->name);
  if (task->context) g_main_context_unref (task->context);
  if (task->task_data_destroy) task->task_data_destroy (task->task_data);
  if (task->result_destroy && task->result.pointer) task->result_destroy (task->result.pointer);
  if (task->error) g_error_free (task->error);
  if (G_TASK_IS_THREADED(task)) {
      g_mutex_clear(&task->lock);
      g_cond_clear(&task->cond);
  }
  G_OBJECT_CLASS(g_task_parent_class)->finalize(object);
}
GTask *g_task_new(gpointer source_object, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer callback_data) {
  GTask *task;
  GSource *source;
  task = g_object_new (G_TYPE_TASK, NULL);
  task->source_object = source_object ? g_object_ref (source_object) : NULL;
  task->cancellable = cancellable ? g_object_ref (cancellable) : NULL;
  task->callback = callback;
  task->callback_data = callback_data;
  task->context = g_main_context_ref_thread_default();
  source = g_main_current_source ();
  if (source) task->creation_time = g_source_get_time (source);
  TRACE (GIO_TASK_NEW (task, source_object, cancellable, callback, callback_data));
  return task;
}
void g_task_report_error(gpointer source_object, GAsyncReadyCallback callback, gpointer callback_data, gpointer source_tag, GError *error) {
  GTask *task;
  task = g_task_new(source_object, NULL, callback, callback_data);
  g_task_set_source_tag(task, source_tag);
  g_task_return_error(task, error);
  g_object_unref(task);
}
void g_task_report_new_error(gpointer source_object, GAsyncReadyCallback callback, gpointer callback_data, gpointer source_tag, GQuark domain, gint code,
                             const char *format, ...) {
  GError *error;
  va_list ap;
  va_start(ap, format);
  error = g_error_new_valist(domain, code, format, ap);
  va_end(ap);
  g_task_report_error(source_object, callback, callback_data, source_tag, error);
}
void g_task_set_task_data(GTask *task, gpointer task_data, GDestroyNotify task_data_destroy) {
  g_return_if_fail(G_IS_TASK (task));
  if (task->task_data_destroy) task->task_data_destroy(task->task_data);
  task->task_data = task_data;
  task->task_data_destroy = task_data_destroy;
  TRACE(GIO_TASK_SET_TASK_DATA(task, task_data, task_data_destroy));
}
void g_task_set_priority(GTask *task, gint priority) {
  g_return_if_fail (G_IS_TASK (task));
  task->priority = priority;
  TRACE (GIO_TASK_SET_PRIORITY (task, priority));
}
void g_task_set_check_cancellable(GTask *task, gboolean check_cancellable) {
  g_return_if_fail (G_IS_TASK (task));
  g_return_if_fail (check_cancellable || !task->return_on_cancel);
  task->check_cancellable = check_cancellable;
}
static void g_task_thread_complete(GTask *task);
gboolean g_task_set_return_on_cancel(GTask *task, gboolean return_on_cancel) {
  g_return_val_if_fail(G_IS_TASK (task), FALSE);
  g_return_val_if_fail(task->check_cancellable || !return_on_cancel, FALSE);
  if (!G_TASK_IS_THREADED(task)) {
      task->return_on_cancel = return_on_cancel;
      return TRUE;
  }
  g_mutex_lock(&task->lock);
  if (task->thread_cancelled) {
      if (return_on_cancel && !task->return_on_cancel) {
          g_mutex_unlock(&task->lock);
          g_task_thread_complete(task);
      } else g_mutex_unlock(&task->lock);
      return FALSE;
    }
  task->return_on_cancel = return_on_cancel;
  g_mutex_unlock(&task->lock);
  return TRUE;
}
void g_task_set_source_tag(GTask *task, gpointer source_tag) {
  g_return_if_fail(G_IS_TASK (task));
  task->source_tag = source_tag;
  TRACE(GIO_TASK_SET_SOURCE_TAG(task, source_tag));
}
void g_task_set_name(GTask *task, const gchar *name) {
  gchar *new_name;
  g_return_if_fail(G_IS_TASK(task));
  new_name = g_strdup(name);
  g_free(task->name);
  task->name = g_steal_pointer(&new_name);
}
gpointer g_task_get_source_object(GTask *task) {
  g_return_val_if_fail(G_IS_TASK (task), NULL);
  return task->source_object;
}
static GObject *g_task_ref_source_object(GAsyncResult *res) {
  GTask *task = G_TASK(res);
  if (task->source_object) return g_object_ref(task->source_object);
  else return NULL;
}
gpointer g_task_get_task_data(GTask *task) {
  g_return_val_if_fail(G_IS_TASK(task), NULL);
  return task->task_data;
}
gint g_task_get_priority(GTask *task) {
  g_return_val_if_fail(G_IS_TASK(task), G_PRIORITY_DEFAULT);
  return task->priority;
}
GMainContext *g_task_get_context(GTask *task) {
  g_return_val_if_fail(G_IS_TASK(task), NULL);
  return task->context;
}
GCancellable *g_task_get_cancellable(GTask *task) {
  g_return_val_if_fail(G_IS_TASK(task), NULL);
  return task->cancellable;
}
gboolean g_task_get_check_cancellable(GTask *task) {
  g_return_val_if_fail(G_IS_TASK(task), FALSE);
  return task->check_cancellable ? TRUE : FALSE;
}
gboolean g_task_get_return_on_cancel(GTask *task) {
  g_return_val_if_fail(G_IS_TASK(task), FALSE);
  return task->return_on_cancel ? TRUE : FALSE;
}
gpointer g_task_get_source_tag(GTask *task) {
  g_return_val_if_fail(G_IS_TASK (task), NULL);
  return task->source_tag;
}
const gchar *g_task_get_name(GTask *task) {
  g_return_val_if_fail(G_IS_TASK (task), NULL);
  return task->name;
}
static void g_task_return_now(GTask *task) {
  TRACE(GIO_TASK_BEFORE_RETURN(task, task->source_object, task->callback, task->callback_data));
  g_main_context_push_thread_default(task->context);
  if (task->callback != NULL) task->callback(task->source_object, G_ASYNC_RESULT(task), task->callback_data);
  task->completed = TRUE;
  g_object_notify(G_OBJECT(task), "completed");
  g_main_context_pop_thread_default(task->context);
}
static gboolean complete_in_idle_cb(gpointer task) {
  g_task_return_now(task);
  g_object_unref(task);
  return FALSE;
}
typedef enum {
  G_TASK_RETURN_SUCCESS,
  G_TASK_RETURN_ERROR,
  G_TASK_RETURN_FROM_THREAD
} GTaskReturnType;
static void g_task_return(GTask *task, GTaskReturnType type) {
  GSource *source;
  if (type != G_TASK_RETURN_FROM_THREAD) task->ever_returned = TRUE;
  if (type == G_TASK_RETURN_SUCCESS) task->result_set = TRUE;
  if (task->synchronous) return;
  if (G_TASK_IS_THREADED(task) && type != G_TASK_RETURN_FROM_THREAD) return;
  g_object_ref(task);
  source = g_main_current_source();
  if (source && g_source_get_context(source) == task->context) {
      if (g_source_get_time(source) > task->creation_time) {
          if (!g_cancellable_is_cancelled(task->cancellable)) {
              g_task_return_now(task);
              g_object_unref(task);
              return;
          }
      }
  }
  source = g_idle_source_new();
  g_source_set_name(source, "[gio] complete_in_idle_cb");
  g_task_attach_source(task, source, complete_in_idle_cb);
  g_source_unref(source);
}
static void task_thread_cancelled(GCancellable *cancellable, gpointer user_data);
static void g_task_thread_complete(GTask *task) {
  g_mutex_lock(&task->lock);
  if (task->thread_complete) {
      g_mutex_unlock(&task->lock);
      return;
  }
  TRACE(GIO_TASK_AFTER_RUN_IN_THREAD(task, task->thread_cancelled));
  task->thread_complete = TRUE;
  g_mutex_unlock(&task->lock);
  if (task->cancellable) g_signal_handlers_disconnect_by_func(task->cancellable, task_thread_cancelled, task);
  if (task->synchronous) g_cond_signal(&task->cond);
  else g_task_return(task, G_TASK_RETURN_FROM_THREAD);
}
static gboolean task_pool_manager_timeout(gpointer user_data) {
  g_mutex_lock(&task_pool_mutex);
  g_thread_pool_set_max_threads(task_pool, tasks_running + 1, NULL);
  g_source_set_ready_time(task_pool_manager, -1);
  g_mutex_unlock(&task_pool_mutex);
  return TRUE;
}
static void g_task_thread_setup(void) {
  g_private_set(&task_private, GUINT_TO_POINTER(TRUE));
  g_mutex_lock(&task_pool_mutex);
  tasks_running++;
  if (tasks_running == G_TASK_POOL_SIZE) task_wait_time = G_TASK_WAIT_TIME_BASE;
  else if (tasks_running > G_TASK_POOL_SIZE && tasks_running < G_TASK_WAIT_TIME_MAX_POOL_SIZE) task_wait_time *= G_TASK_WAIT_TIME_MULTIPLIER;
  if (tasks_running >= G_TASK_POOL_SIZE) g_source_set_ready_time(task_pool_manager, g_get_monotonic_time() + task_wait_time);
  g_mutex_unlock(&task_pool_mutex);
}
static void g_task_thread_cleanup(void) {
  gint tasks_pending;
  g_mutex_lock(&task_pool_mutex);
  tasks_pending = g_thread_pool_unprocessed(task_pool);
  if (tasks_running > G_TASK_POOL_SIZE) g_thread_pool_set_max_threads(task_pool, tasks_running - 1, NULL);
  else if (tasks_running + tasks_pending < G_TASK_POOL_SIZE) g_source_set_ready_time(task_pool_manager, -1);
  if (tasks_running > G_TASK_POOL_SIZE && tasks_running < G_TASK_WAIT_TIME_MAX_POOL_SIZE) task_wait_time /= G_TASK_WAIT_TIME_MULTIPLIER;
  tasks_running--;
  g_mutex_unlock(&task_pool_mutex);
  g_private_set(&task_private, GUINT_TO_POINTER(FALSE));
}
static void g_task_thread_pool_thread(gpointer thread_data, gpointer pool_data) {
  GTask *task = thread_data;
  g_task_thread_setup ();
  task->task_func(task, task->source_object, task->task_data, task->cancellable);
  g_task_thread_complete (task);
  g_object_unref (task);
  g_task_thread_cleanup ();
}
static void task_thread_cancelled(GCancellable *cancellable, gpointer user_data) {
  GTask *task = user_data;
  g_thread_pool_move_to_front(task_pool, task);
  g_mutex_lock(&task->lock);
  task->thread_cancelled = TRUE;
  if (!task->return_on_cancel) {
      g_mutex_unlock(&task->lock);
      return;
  }
  g_mutex_unlock(&task->lock);
  g_task_thread_complete(task);
}
static void task_thread_cancelled_disconnect_notify(gpointer task, GClosure *closure) {
  g_object_unref(task);
}
static void g_task_start_task_thread(GTask *task, GTaskThreadFunc task_func) {
  g_static_mutex_init(&task->lock);
  g_cond_init(&task->cond);
  g_mutex_lock(&task->lock);
  TRACE(GIO_TASK_BEFORE_RUN_IN_THREAD(task, task_func));
  task->task_func = task_func;
  if (task->cancellable) {
      if (task->return_on_cancel && g_cancellable_set_error_if_cancelled(task->cancellable, &task->error)) {
          task->thread_cancelled = task->thread_complete = TRUE;
          TRACE(GIO_TASK_AFTER_RUN_IN_THREAD(task, task->thread_cancelled));
          g_thread_pool_push(task_pool, g_object_ref (task), NULL);
          return;
      }
      g_signal_connect_data(task->cancellable, "cancelled", G_CALLBACK(task_thread_cancelled), g_object_ref(task),
                            task_thread_cancelled_disconnect_notify, 0);
  }
  if (g_private_get(&task_private)) task->blocking_other_task = TRUE;
  g_thread_pool_push(task_pool, g_object_ref(task), NULL);
}
void g_task_run_in_thread(GTask *task, GTaskThreadFunc task_func) {
  g_return_if_fail(G_IS_TASK(task));
  g_object_ref(task);
  g_task_start_task_thread(task, task_func);
  if (task->thread_complete) {
      g_mutex_unlock(&task->lock);
      g_task_return(task, G_TASK_RETURN_FROM_THREAD);
  } else g_mutex_unlock(&task->lock);
  g_object_unref(task);
}
void g_task_run_in_thread_sync(GTask *task, GTaskThreadFunc task_func) {
  g_return_if_fail(G_IS_TASK (task));
  g_object_ref(task);
  task->synchronous = TRUE;
  g_task_start_task_thread(task, task_func);
  while(!task->thread_complete) g_cond_wait(&task->cond, &task->lock);
  g_mutex_unlock(&task->lock);
  TRACE(GIO_TASK_BEFORE_RETURN(task, task->source_object, NULL, NULL));
  task->completed = TRUE;
  g_object_notify(G_OBJECT(task), "completed");
  g_object_unref(task);
}
void g_task_attach_source(GTask *task, GSource *source, GSourceFunc callback) {
  g_return_if_fail(G_IS_TASK(task));
  g_source_set_callback(source, callback, g_object_ref(task), g_object_unref);
  g_source_set_priority(source, task->priority);
  if (task->name != NULL) g_source_set_name(source, task->name);
  g_source_attach(source, task->context);
}
static gboolean g_task_propagate_error(GTask *task, GError **error) {
  gboolean error_set;
  if (task->check_cancellable && g_cancellable_set_error_if_cancelled(task->cancellable, error)) error_set = TRUE;
  else if (task->error) {
      g_propagate_error(error, task->error);
      task->error = NULL;
      task->had_error = TRUE;
      error_set = TRUE;
  } else error_set = FALSE;
  TRACE(GIO_TASK_PROPAGATE (task, error_set));
  return error_set;
}
void g_task_return_pointer(GTask *task, gpointer result, GDestroyNotify result_destroy) {
  g_return_if_fail(G_IS_TASK (task));
  g_return_if_fail(!task->ever_returned);
  task->result.pointer = result;
  task->result_destroy = result_destroy;
  g_task_return(task, G_TASK_RETURN_SUCCESS);
}
gpointer g_task_propagate_pointer(GTask *task, GError **error) {
  g_return_val_if_fail(G_IS_TASK(task), NULL);
  if (g_task_propagate_error(task, error)) return NULL;
  g_return_val_if_fail(task->result_set, NULL);
  task->result_destroy = NULL;
  task->result_set = FALSE;
  return task->result.pointer;
}
void g_task_return_int(GTask *task, gssize result) {
  g_return_if_fail(G_IS_TASK(task));
  g_return_if_fail(!task->ever_returned);
  task->result.size = result;
  g_task_return (task, G_TASK_RETURN_SUCCESS);
}
gssize g_task_propagate_int(GTask *task, GError **error) {
  g_return_val_if_fail(G_IS_TASK(task), -1);
  if (g_task_propagate_error(task, error)) return -1;
  g_return_val_if_fail(task->result_set, -1);
  task->result_set = FALSE;
  return task->result.size;
}
void g_task_return_boolean(GTask *task, gboolean result) {
  g_return_if_fail(G_IS_TASK(task));
  g_return_if_fail(!task->ever_returned);
  task->result.boolean = result;
  g_task_return (task, G_TASK_RETURN_SUCCESS);
}
gboolean g_task_propagate_boolean(GTask *task, GError **error) {
  g_return_val_if_fail(G_IS_TASK(task), FALSE);
  if (g_task_propagate_error(task, error)) return FALSE;
  g_return_val_if_fail(task->result_set, FALSE);
  task->result_set = FALSE;
  return task->result.boolean;
}
void g_task_return_error(GTask *task, GError *error) {
  g_return_if_fail(G_IS_TASK (task));
  g_return_if_fail(!task->ever_returned);
  g_return_if_fail(error != NULL);
  task->error = error;
  g_task_return(task, G_TASK_RETURN_ERROR);
}
void g_task_return_new_error(GTask *task, GQuark domain, gint code, const char *format, ...) {
  GError *error;
  va_list args;
  va_start(args, format);
  error = g_error_new_valist(domain, code, format, args);
  va_end(args);
  g_task_return_error(task, error);
}
gboolean g_task_return_error_if_cancelled(GTask *task) {
  GError *error = NULL;
  g_return_val_if_fail(G_IS_TASK(task), FALSE);
  g_return_val_if_fail(!task->ever_returned, FALSE);
  if (g_cancellable_set_error_if_cancelled(task->cancellable, &error)) {
      g_clear_error(&task->error);
      task->error = error;
      g_task_return(task, G_TASK_RETURN_ERROR);
      return TRUE;
  } else return FALSE;
}
gboolean g_task_had_error(GTask *task) {
  g_return_val_if_fail(G_IS_TASK(task), FALSE);
  if (task->error != NULL || task->had_error) return TRUE;
  if (task->check_cancellable && g_cancellable_is_cancelled(task->cancellable)) return TRUE;
  return FALSE;
}
static void value_free(gpointer value) {
  g_value_unset(value);
  g_free(value);
}
void g_task_return_value(GTask *task, GValue *result) {
  GValue *value;
  g_return_if_fail(G_IS_TASK (task));
  g_return_if_fail(!task->ever_returned);
  value = g_new0(GValue, 1);
  if (result == NULL) {
      g_value_init(value, G_TYPE_POINTER);
      g_value_set_pointer(value, NULL);
  } else {
      g_value_init(value, G_VALUE_TYPE(result));
      g_value_copy(result, value);
  }
  g_task_return_pointer(task, value, value_free);
}
gboolean g_task_propagate_value(GTask *task, GValue *value, GError **error) {
  g_return_val_if_fail(G_IS_TASK (task), FALSE);
  g_return_val_if_fail(value != NULL, FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  if (g_task_propagate_error(task, error)) return FALSE;
  g_return_val_if_fail(task->result_set, FALSE);
  g_return_val_if_fail(task->result_destroy == value_free, FALSE);
  memcpy(value, task->result.pointer, sizeof(GValue));
  g_free(task->result.pointer);
  task->result_destroy = NULL;
  task->result_set = FALSE;
  return TRUE;
}
gboolean g_task_get_completed(GTask *task) {
  g_return_val_if_fail(G_IS_TASK(task), FALSE);
  return task->completed ? TRUE : FALSE;
}
gboolean g_task_is_valid(gpointer result, gpointer source_object) {
  if (!G_IS_TASK(result)) return FALSE;
  return G_TASK(result)->source_object == source_object;
}
static gint g_task_compare_priority(gconstpointer a, gconstpointer b, gpointer user_data) {
  const GTask *ta = a;
  const GTask *tb = b;
  gboolean a_cancelled, b_cancelled;
  if (ta->blocking_other_task && !tb->blocking_other_task) return -1;
  else if (tb->blocking_other_task && !ta->blocking_other_task) return 1;
  a_cancelled = (ta->check_cancellable && g_cancellable_is_cancelled(ta->cancellable));
  b_cancelled = (tb->check_cancellable && g_cancellable_is_cancelled(tb->cancellable));
  if (a_cancelled && !b_cancelled) return -1;
  else if (b_cancelled && !a_cancelled) return 1;
  return ta->priority - tb->priority;
}
static gboolean trivial_source_dispatch(GSource *source, GSourceFunc callback, gpointer user_data) {
  return callback(user_data);
}
GSourceFuncs trivial_source_funcs = {NULL,NULL, trivial_source_dispatch,NULL };
static void g_task_thread_pool_init(void) {
  task_pool = g_thread_pool_new(g_task_thread_pool_thread, NULL, G_TASK_POOL_SIZE, FALSE, NULL);
  g_assert(task_pool != NULL);
  g_thread_pool_set_sort_function(task_pool, g_task_compare_priority, NULL);
  task_pool_manager = g_source_new(&trivial_source_funcs, sizeof (GSource));
  g_source_set_name(task_pool_manager, "GTask thread pool manager");
  g_source_set_callback(task_pool_manager, task_pool_manager_timeout, NULL, NULL);
  g_source_set_ready_time(task_pool_manager, -1);
  g_source_attach(task_pool_manager, GLIB_PRIVATE_CALL(g_get_worker_context ()));
  g_source_unref(task_pool_manager);
}
static void g_task_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GTask *task = G_TASK(object);
  switch((GTaskProperty) prop_id) {
      case PROP_COMPLETED: g_value_set_boolean(value, g_task_get_completed(task)); break;
  }
}
static void g_task_class_init(GTaskClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->get_property = g_task_get_property;
  gobject_class->finalize = g_task_finalize;
  g_object_class_install_property(gobject_class, PROP_COMPLETED,
    g_param_spec_boolean("completed", P_("Task completed"), P_("Whether the task has completed yet"),FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
}
static gpointer g_task_get_user_data(GAsyncResult *res) {
  return G_TASK(res)->callback_data;
}
static gboolean g_task_is_tagged(GAsyncResult *res, gpointer source_tag) {
  return G_TASK(res)->source_tag == source_tag;
}
static void g_task_async_result_iface_init(GAsyncResultIface *iface) {
  iface->get_user_data = g_task_get_user_data;
  iface->get_source_object = g_task_ref_source_object;
  iface->is_tagged = g_task_is_tagged;
}