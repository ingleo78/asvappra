#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "../gobject/gobject.h"
#include "../gobject/gtype.h"
#include "config.h"
#include "gasyncinitable.h"
#include "gasyncresult.h"
#include "gsimpleasyncresult.h"

static void g_async_initable_real_init_async(GAsyncInitable *initable, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                             gpointer user_data);
static gboolean g_async_initable_real_init_finish(GAsyncInitable *initable, GAsyncResult *res, GError **error);
typedef GAsyncInitableIface GAsyncInitableInterface;
G_DEFINE_INTERFACE(GAsyncInitable, g_async_initable, G_TYPE_OBJECT)
static void g_async_initable_default_init(GAsyncInitableInterface *iface) {
  iface->init_async = g_async_initable_real_init_async;
  iface->init_finish = g_async_initable_real_init_finish;
}
void g_async_initable_init_async(GAsyncInitable *initable, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                 gpointer user_data) {
  GAsyncInitableIface *iface;
  g_return_if_fail(G_IS_ASYNC_INITABLE (initable));
  iface = G_ASYNC_INITABLE_GET_IFACE (initable);
  (* iface->init_async)(initable, io_priority, cancellable, callback, user_data);
}
gboolean g_async_initable_init_finish(GAsyncInitable *initable, GAsyncResult *res, GError **error) {
  GAsyncInitableIface *iface;
  g_return_val_if_fail(G_IS_ASYNC_INITABLE(initable), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_ASYNC_INITABLE_GET_IFACE (initable);
  return (*iface->init_finish)(initable, res, error);
}
static void async_init_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GError *error = NULL;
  if (!g_initable_init(G_INITABLE(object), cancellable, &error)) g_simple_async_result_take_error(res, error);
}
static void g_async_initable_real_init_async(GAsyncInitable *initable, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  g_return_if_fail(G_IS_INITABLE(initable));
  res = g_simple_async_result_new(G_OBJECT(initable), callback, user_data, g_async_initable_real_init_async);
  g_simple_async_result_run_in_thread(res, async_init_thread, io_priority, cancellable);
  g_object_unref(res);
}
static gboolean g_async_initable_real_init_finish(GAsyncInitable *initable, GAsyncResult *res, GError **error) {
  return TRUE;
}
void g_async_initable_new_async(GType object_type, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data,
			   				 	const gchar *first_property_name, ...) {
  va_list var_args;
  va_start(var_args, first_property_name);
  g_async_initable_new_valist_async(object_type, first_property_name, var_args, io_priority, cancellable, callback, user_data);
  va_end(var_args);
}
void g_async_initable_newv_async(GType object_type, guint n_parameters, GParameter *parameters, int io_priority, GCancellable *cancellable,
			     				 GAsyncReadyCallback callback, gpointer user_data) {
  GObject *obj;
  g_return_if_fail(G_TYPE_IS_ASYNC_INITABLE(object_type));
  obj = g_object_newv(object_type, n_parameters, parameters);
  g_async_initable_init_async(G_ASYNC_INITABLE(obj), io_priority, cancellable, callback, user_data);
}
void g_async_initable_new_valist_async(GType object_type, const gchar *first_property_name, va_list var_args, int io_priority, GCancellable *cancellable,
									   GAsyncReadyCallback callback, gpointer user_data) {
  GObject *obj;
  g_return_if_fail(G_TYPE_IS_ASYNC_INITABLE(object_type));
  obj = g_object_new_valist(object_type, first_property_name, var_args);
  g_async_initable_init_async(G_ASYNC_INITABLE(obj), io_priority, cancellable, callback, user_data);
  g_object_unref(obj);
}
GObject *g_async_initable_new_finish(GAsyncInitable *initable, GAsyncResult *res, GError **error) {
  if (g_async_initable_init_finish(initable, res, error)) return g_object_ref(initable);
  else return NULL;
}