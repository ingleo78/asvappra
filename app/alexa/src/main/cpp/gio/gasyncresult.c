#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "../gobject/gobject.h"
#include "config.h"
#include "gasyncresult.h"

typedef GAsyncResultIface GAsyncResultInterface;
G_DEFINE_INTERFACE(GAsyncResult, g_async_result, G_TYPE_OBJECT);
static void g_async_result_default_init(GAsyncResultInterface *iface) {}
gpointer g_async_result_get_user_data(GAsyncResult *res) {
  GAsyncResultIface *iface;
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  iface = G_ASYNC_RESULT_GET_IFACE(res);
  return (*iface->get_user_data)(res);
}
GObject *g_async_result_get_source_object(GAsyncResult *res) {
  GAsyncResultIface *iface;
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  iface = G_ASYNC_RESULT_GET_IFACE(res);
  return (*iface->get_source_object)(res);
}