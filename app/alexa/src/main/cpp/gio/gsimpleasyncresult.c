#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "../glib/glibintl.h"
#include "../gio/gioerror.h"
#include "config.h"
#include "gsimpleasyncresult.h"
#include "gasyncresult.h"
#include "gcancellable.h"
#include "gioscheduler.h"

static void g_simple_async_result_async_result_iface_init(GAsyncResultIface *iface);
struct _GSimpleAsyncResult {
  GObject parent_instance;
  GObject *source_object;
  GAsyncReadyCallback callback;
  gpointer user_data;
  GMainContext *context;
  GError *error;
  gboolean failed;
  gboolean handle_cancellation;
  gpointer source_tag;
  union {
      gpointer v_pointer;
      gboolean v_boolean;
      gssize   v_ssize;
  } op_res;
  GDestroyNotify destroy_op_res;
};
struct _GSimpleAsyncResultClass {
  GObjectClass parent_class;
};
G_DEFINE_TYPE_WITH_CODE(GSimpleAsyncResult, g_simple_async_result, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_ASYNC_RESULT, g_simple_async_result_async_result_iface_init));
static void clear_op_res(GSimpleAsyncResult *simple) {
  if (simple->destroy_op_res) simple->destroy_op_res(simple->op_res.v_pointer);
  simple->destroy_op_res = NULL;
  simple->op_res.v_ssize = 0;
}
static void g_simple_async_result_finalize(GObject *object) {
  GSimpleAsyncResult *simple;
  simple = G_SIMPLE_ASYNC_RESULT(object);
  if (simple->source_object) g_object_unref(simple->source_object);
  if (simple->context) g_main_context_unref(simple->context);
  clear_op_res(simple);
  if (simple->error) g_error_free(simple->error);
  G_OBJECT_CLASS(g_simple_async_result_parent_class)->finalize(object);
}
static void g_simple_async_result_class_init (GSimpleAsyncResultClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_simple_async_result_finalize;
}
static void g_simple_async_result_init(GSimpleAsyncResult *simple) {
  simple->handle_cancellation = TRUE;
  simple->context = g_main_context_get_thread_default();
  if (simple->context) g_main_context_ref(simple->context);
}
GSimpleAsyncResult *g_simple_async_result_new(GObject *source_object, GAsyncReadyCallback callback, gpointer user_data, gpointer source_tag) {
  GSimpleAsyncResult *simple;
  g_return_val_if_fail(!source_object || G_IS_OBJECT(source_object), NULL);
  simple = g_object_new(G_TYPE_SIMPLE_ASYNC_RESULT, NULL);
  simple->callback = callback;
  if (source_object) simple->source_object = g_object_ref(source_object);
  else simple->source_object = NULL;
  simple->user_data = user_data;
  simple->source_tag = source_tag;
  return simple;
}
GSimpleAsyncResult *g_simple_async_result_new_from_error(GObject *source_object, GAsyncReadyCallback callback, gpointer user_data, const GError *error) {
  GSimpleAsyncResult *simple;
  g_return_val_if_fail(!source_object || G_IS_OBJECT(source_object), NULL);
  simple = g_simple_async_result_new(source_object, callback, user_data, NULL);
  g_simple_async_result_set_from_error(simple, error);
  return simple;
}
GSimpleAsyncResult *g_simple_async_result_new_take_error(GObject *source_object, GAsyncReadyCallback callback, gpointer user_data, GError *error) {
  GSimpleAsyncResult *simple;
  g_return_val_if_fail(!source_object || G_IS_OBJECT(source_object), NULL);
  simple = g_simple_async_result_new(source_object, callback, user_data, NULL);
  g_simple_async_result_take_error(simple, error);
  return simple;
}
GSimpleAsyncResult *g_simple_async_result_new_error(GObject *source_object, GAsyncReadyCallback callback, gpointer user_data, GQuark domain, gint code,
                                                    const char *format, ...) {
  GSimpleAsyncResult *simple;
  va_list args;
  g_return_val_if_fail(!source_object || G_IS_OBJECT(source_object), NULL);
  g_return_val_if_fail(domain != 0, NULL);
  g_return_val_if_fail(format != NULL, NULL);
  simple = g_simple_async_result_new(source_object, callback, user_data, NULL);
  va_start(args, format);
  g_simple_async_result_set_error_va(simple, domain, code, format, args);
  va_end(args);
  return simple;
}
static gpointer g_simple_async_result_get_user_data(GAsyncResult *res) {
  return G_SIMPLE_ASYNC_RESULT(res)->user_data;
}
static GObject *g_simple_async_result_get_source_object(GAsyncResult *res) {
  if (G_SIMPLE_ASYNC_RESULT(res)->source_object) return g_object_ref(G_SIMPLE_ASYNC_RESULT(res)->source_object);
  return NULL;
}
static void g_simple_async_result_async_result_iface_init(GAsyncResultIface *iface) {
  iface->get_user_data = g_simple_async_result_get_user_data;
  iface->get_source_object = g_simple_async_result_get_source_object;
}
void g_simple_async_result_set_handle_cancellation(GSimpleAsyncResult *simple, gboolean handle_cancellation) {
  g_return_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple));
  simple->handle_cancellation = handle_cancellation;
}
gpointer g_simple_async_result_get_source_tag(GSimpleAsyncResult *simple) {
  g_return_val_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple), NULL);
  return simple->source_tag;
}
gboolean g_simple_async_result_propagate_error(GSimpleAsyncResult *simple, GError **dest) {
  g_return_val_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple), FALSE);
  if (simple->failed) {
      g_propagate_error(dest, simple->error);
      simple->error = NULL;
      return TRUE;
  }
  return FALSE;
}
void g_simple_async_result_set_op_res_gpointer(GSimpleAsyncResult *simple, gpointer op_res, GDestroyNotify destroy_op_res) {
  g_return_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple));
  clear_op_res(simple);
  simple->op_res.v_pointer = op_res;
  simple->destroy_op_res = destroy_op_res;
}
gpointer g_simple_async_result_get_op_res_gpointer(GSimpleAsyncResult *simple) {
  g_return_val_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple), NULL);
  return simple->op_res.v_pointer;
}
void g_simple_async_result_set_op_res_gssize(GSimpleAsyncResult *simple, gssize op_res) {
  g_return_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple));
  clear_op_res(simple);
  simple->op_res.v_ssize = op_res;
}
gssize g_simple_async_result_get_op_res_gssize(GSimpleAsyncResult *simple) {
  g_return_val_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple), 0);
  return simple->op_res.v_ssize;
}
void g_simple_async_result_set_op_res_gboolean(GSimpleAsyncResult *simple, gboolean op_res) {
  g_return_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple));
  clear_op_res(simple);
  simple->op_res.v_boolean = !!op_res;
}
gboolean g_simple_async_result_get_op_res_gboolean(GSimpleAsyncResult *simple) {
  g_return_val_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple), FALSE);
  return simple->op_res.v_boolean;
}
void g_simple_async_result_set_from_error(GSimpleAsyncResult *simple, const GError *error) {
  g_return_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple));
  g_return_if_fail(error != NULL);
  if (simple->error) g_error_free(simple->error);
  simple->error = g_error_copy(error);
  simple->failed = TRUE;
}
void g_simple_async_result_take_error(GSimpleAsyncResult *simple, GError *error) {
  g_return_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple));
  g_return_if_fail(error != NULL);
  if (simple->error) g_error_free(simple->error);
  simple->error = error;
  simple->failed = TRUE;
}
void g_simple_async_result_set_error_va(GSimpleAsyncResult *simple, GQuark domain, gint code, const char *format, va_list args) {
  g_return_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple));
  g_return_if_fail(domain != 0);
  g_return_if_fail(format != NULL);
  if (simple->error) g_error_free(simple->error);
  simple->error = g_error_new_valist(domain, code, format, args);
  simple->failed = TRUE;
}
void g_simple_async_result_set_error(GSimpleAsyncResult *simple, GQuark domain, gint code, const char *format, ...) {
  va_list args;
  g_return_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple));
  g_return_if_fail(domain != 0);
  g_return_if_fail(format != NULL);
  va_start(args, format);
  g_simple_async_result_set_error_va(simple, domain, code, format, args);
  va_end(args);
}
void g_simple_async_result_complete(GSimpleAsyncResult *simple) {
#ifndef G_DISABLE_CHECKS
  GSource *current_source;
  GMainContext *current_context;
#endif
  g_return_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple));
#ifndef G_DISABLE_CHECKS
  current_source = g_main_current_source();
  if (current_source && !g_source_is_destroyed(current_source)) {
      current_context = g_source_get_context(current_source);
      if (current_context == g_main_context_default()) current_context = NULL;
      if (simple->context != current_context) g_warning("g_simple_async_result_complete() called from wrong context!");
  }
#endif
  if (simple->callback) simple->callback(simple->source_object, G_ASYNC_RESULT(simple), simple->user_data);
}
static gboolean complete_in_idle_cb(gpointer data) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(data);
  g_simple_async_result_complete(simple);
  return FALSE;
}
void g_simple_async_result_complete_in_idle(GSimpleAsyncResult *simple) {
  GSource *source;
  g_return_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple));
  g_object_ref(simple);
  source = g_idle_source_new();
  g_source_set_priority(source, G_PRIORITY_DEFAULT);
  g_source_set_callback(source, complete_in_idle_cb, simple, g_object_unref);
  g_source_attach(source, simple->context);
  g_source_unref(source);
}
typedef struct {
  GSimpleAsyncResult *simple;
  GCancellable *cancellable;
  GSimpleAsyncThreadFunc func;
} RunInThreadData;
static gboolean complete_in_idle_cb_for_thread(gpointer _data) {
  RunInThreadData *data = _data;
  GSimpleAsyncResult *simple;
  simple = data->simple;
  if (simple->handle_cancellation && g_cancellable_is_cancelled(data->cancellable))
      g_simple_async_result_set_error(simple, G_IO_ERROR,G_IO_ERROR_CANCELLED,"%s", _("Operation was cancelled"));
  g_simple_async_result_complete(simple);
  if (data->cancellable) g_object_unref(data->cancellable);
  g_object_unref(data->simple);
  g_free(data);
  return FALSE;
}
static gboolean run_in_thread(GIOSchedulerJob *job, GCancellable *c, gpointer _data) {
  RunInThreadData *data = _data;
  GSimpleAsyncResult *simple = data->simple;
  GSource *source;
  if (simple->handle_cancellation && g_cancellable_is_cancelled(c)) {
    g_simple_async_result_set_error(simple, G_IO_ERROR,G_IO_ERROR_CANCELLED,"%s", _("Operation was cancelled"));
  } else data->func(simple, simple->source_object, c);
  source = g_idle_source_new();
  g_source_set_priority(source, G_PRIORITY_DEFAULT);
  g_source_set_callback(source, complete_in_idle_cb_for_thread, data, NULL);
  g_source_attach(source, simple->context);
  g_source_unref(source);
  return FALSE;
}
void g_simple_async_result_run_in_thread(GSimpleAsyncResult *simple, GSimpleAsyncThreadFunc func, int io_priority, GCancellable *cancellable) {
  RunInThreadData *data;
  g_return_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple));
  g_return_if_fail(func != NULL);
  data = g_new(RunInThreadData, 1);
  data->func = func;
  data->simple = g_object_ref(simple);
  data->cancellable = cancellable;
  if (cancellable) g_object_ref(cancellable);
  g_io_scheduler_push_job(run_in_thread, data, NULL, io_priority, cancellable);
}
gboolean g_simple_async_result_is_valid(GAsyncResult *result, GObject *source, gpointer source_tag) {
  GSimpleAsyncResult *simple;
  GObject *cmp_source;
  if (!G_IS_SIMPLE_ASYNC_RESULT(result)) return FALSE;
  simple = (GSimpleAsyncResult*)result;
  cmp_source = g_async_result_get_source_object(result);
  if (cmp_source != source) {
      if (cmp_source != NULL) g_object_unref(cmp_source);
      return FALSE;
  }
  if (cmp_source != NULL) g_object_unref(cmp_source);
  return source_tag == NULL || source_tag == g_simple_async_result_get_source_tag(simple);
}
void g_simple_async_report_error_in_idle(GObject *object, GAsyncReadyCallback callback, gpointer user_data, GQuark domain, gint code, const char *format, ...) {
  GSimpleAsyncResult *simple;
  va_list args;
  g_return_if_fail(!object || G_IS_OBJECT(object));
  g_return_if_fail(domain != 0);
  g_return_if_fail(format != NULL);
  simple = g_simple_async_result_new(object, callback, user_data, NULL);
  va_start(args, format);
  g_simple_async_result_set_error_va(simple, domain, code, format, args);
  va_end(args);
  g_simple_async_result_complete_in_idle(simple);
  g_object_unref(simple);
}
void g_simple_async_report_gerror_in_idle(GObject *object, GAsyncReadyCallback callback, gpointer user_data, const GError *error) {
  GSimpleAsyncResult *simple;
  g_return_if_fail(!object || G_IS_OBJECT(object));
  g_return_if_fail(error != NULL);
  simple = g_simple_async_result_new_from_error(object, callback, user_data, error);
  g_simple_async_result_complete_in_idle(simple);
  g_object_unref(simple);
}
void g_simple_async_report_take_gerror_in_idle(GObject *object, GAsyncReadyCallback callback, gpointer user_data, GError *error) {
  GSimpleAsyncResult *simple;
  g_return_if_fail(!object || G_IS_OBJECT(object));
  g_return_if_fail(error != NULL);
  simple = g_simple_async_result_new_take_error(object, callback, user_data, error);
  g_simple_async_result_complete_in_idle(simple);
  g_object_unref(simple);
}