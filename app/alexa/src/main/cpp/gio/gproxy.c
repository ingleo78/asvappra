#include "../glib/glibintl.h"
#include "config.h"
#include "gproxy.h"
#include "giomodule.h"
#include "giomodule-priv.h"

G_DEFINE_INTERFACE(GProxy, g_proxy, G_TYPE_OBJECT);
static void g_proxy_default_init(GProxyInterface *iface) {}
GProxy *g_proxy_get_default_for_protocol(const gchar *protocol) {
  GIOExtensionPoint *ep;
  GIOExtension *extension;
  _g_io_modules_ensure_loaded();
  ep = g_io_extension_point_lookup(G_PROXY_EXTENSION_POINT_NAME);
  extension = g_io_extension_point_get_extension_by_name(ep, protocol);
  if (extension) return g_object_new(g_io_extension_get_type(extension), NULL);
  return NULL;
}
GIOStream *g_proxy_connect(GProxy *proxy, GIOStream *connection, GProxyAddress *proxy_address, GCancellable *cancellable, GError **error) {
  GProxyInterface *iface;
  g_return_val_if_fail(G_IS_PROXY(proxy), NULL);
  iface = G_PROXY_GET_IFACE(proxy);
  return (*iface->connect)(proxy, connection, proxy_address, cancellable, error);
}
void g_proxy_connect_async(GProxy *proxy, GIOStream *connection, GProxyAddress *proxy_address, GCancellable *cancellable, GAsyncReadyCallback callback,
		                   gpointer user_data) {
  GProxyInterface *iface;
  g_return_if_fail(G_IS_PROXY(proxy));
  iface = G_PROXY_GET_IFACE(proxy);
  (*iface->connect_async)(proxy, connection, proxy_address, cancellable, callback, user_data);
}
GIOStream *g_proxy_connect_finish(GProxy *proxy, GAsyncResult *result, GError **error) {
  GProxyInterface *iface;
  g_return_val_if_fail(G_IS_PROXY (proxy), NULL);
  iface = G_PROXY_GET_IFACE(proxy);
  return (*iface->connect_finish)(proxy, result, error);
}
gboolean g_proxy_supports_hostname(GProxy *proxy) {
  GProxyInterface *iface;
  g_return_val_if_fail(G_IS_PROXY(proxy), FALSE);
  iface = G_PROXY_GET_IFACE(proxy);
  return (*iface->supports_hostname)(proxy);
}