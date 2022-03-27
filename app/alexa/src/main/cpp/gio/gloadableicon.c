#include "../glib/glibintl.h"
#include "config.h"
#include "gasyncresult.h"
#include "gsimpleasyncresult.h"
#include "gicon.h"
#include "gloadableicon.h"

static void g_loadable_icon_real_load_async(GLoadableIcon *icon, int size, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
static GInputStream *g_loadable_icon_real_load_finish(GLoadableIcon *icon, GAsyncResult *res, char **type, GError **error);
typedef GLoadableIconIface GLoadableIconInterface;
G_DEFINE_INTERFACE(GLoadableIcon, g_loadable_icon, G_TYPE_ICON);
static void g_loadable_icon_default_init(GLoadableIconIface *iface) {
  iface->load_async = g_loadable_icon_real_load_async;
  iface->load_finish = g_loadable_icon_real_load_finish;
}
GInputStream *g_loadable_icon_load(GLoadableIcon *icon, int size, char **type, GCancellable *cancellable, GError **error) {
  GLoadableIconIface *iface;
  g_return_val_if_fail(G_IS_LOADABLE_ICON(icon), NULL);
  iface = G_LOADABLE_ICON_GET_IFACE(icon);
  return (*iface->load)(icon, size, type, cancellable, error);
}
void g_loadable_icon_load_async(GLoadableIcon *icon, int size, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GLoadableIconIface *iface;
  g_return_if_fail(G_IS_LOADABLE_ICON(icon));
  iface = G_LOADABLE_ICON_GET_IFACE(icon);
  (*iface->load_async)(icon, size, cancellable, callback, user_data);
}
GInputStream *g_loadable_icon_load_finish(GLoadableIcon *icon, GAsyncResult *res, char **type, GError **error) {
  GLoadableIconIface *iface;
  g_return_val_if_fail(G_IS_LOADABLE_ICON(icon), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_LOADABLE_ICON_GET_IFACE(icon);
  return (*iface->load_finish)(icon, res, type, error);
}
typedef struct {
  int size;
  char *type;
  GInputStream *stream;
} LoadData;
static void load_data_free(LoadData *data) {
  if (data->stream) g_object_unref(data->stream);
  g_free(data->type);
  g_free(data);
}
static void load_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GLoadableIconIface *iface;
  GInputStream *stream;
  LoadData *data;
  GError *error = NULL;
  char *type = NULL;
  data = g_simple_async_result_get_op_res_gpointer(res);
  iface = G_LOADABLE_ICON_GET_IFACE(object);
  stream = iface->load(G_LOADABLE_ICON(object), data->size, &type, cancellable, &error);
  if (stream == NULL) g_simple_async_result_take_error(res, error);
  else {
      data->stream = stream;
      data->type = type;
  }
}
static void g_loadable_icon_real_load_async(GLoadableIcon *icon, int size, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  LoadData *data;
  res = g_simple_async_result_new(G_OBJECT(icon), callback, user_data, g_loadable_icon_real_load_async);
  data = g_new0(LoadData, 1);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)load_data_free);
  g_simple_async_result_run_in_thread(res, load_async_thread, 0, cancellable);
  g_object_unref(res);
}
static GInputStream *g_loadable_icon_real_load_finish(GLoadableIcon *icon, GAsyncResult *res, char **type, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  LoadData *data;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_loadable_icon_real_load_async);
  data = g_simple_async_result_get_op_res_gpointer(simple);
  if (type) {
      *type = data->type;
      data->type = NULL;
  }
  return g_object_ref(data->stream);
}