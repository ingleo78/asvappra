#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gproxyresolver.h"
#include "gasyncresult.h"
#include "gcancellable.h"
#include "giomodule.h"
#include "giomodule-priv.h"
#include "gsimpleasyncresult.h"

G_DEFINE_INTERFACE(GProxyResolver, g_proxy_resolver, G_TYPE_OBJECT);
static void g_proxy_resolver_default_init(GProxyResolverInterface *iface) {}
static gpointer get_default_proxy_resolver(gpointer arg) {
  const gchar *use_this;
  GProxyResolver *resolver;
  GList *l;
  GIOExtensionPoint *ep;
  GIOExtension *extension;
  use_this = g_getenv("GIO_USE_PROXY_RESOLVER");
  _g_io_modules_ensure_loaded();
  ep = g_io_extension_point_lookup(G_PROXY_RESOLVER_EXTENSION_POINT_NAME);
  if (use_this) {
      extension = g_io_extension_point_get_extension_by_name(ep, use_this);
      if (extension) {
	      resolver = g_object_new(g_io_extension_get_type(extension), NULL);
	      if (g_proxy_resolver_is_supported(resolver)) return resolver;
          g_object_unref(resolver);
      }
  }
  for (l = g_io_extension_point_get_extensions(ep); l != NULL; l = l->next) {
      extension = l->data;
      resolver = g_object_new(g_io_extension_get_type(extension), NULL);
      if (g_proxy_resolver_is_supported(resolver)) return resolver;
      g_object_unref(resolver);
  }
  return NULL;
}
GProxyResolver *g_proxy_resolver_get_default(void) {
  static GOnce once_init = G_ONCE_INIT;
  return g_once(&once_init, get_default_proxy_resolver, NULL);
}
gboolean g_proxy_resolver_is_supported(GProxyResolver *resolver) {
  GProxyResolverInterface *iface;
  g_return_val_if_fail(G_IS_PROXY_RESOLVER(resolver), FALSE);
  iface = G_PROXY_RESOLVER_GET_IFACE(resolver);
  return (*iface->is_supported)(resolver);
}
gchar **g_proxy_resolver_lookup(GProxyResolver  *resolver, const gchar *uri, GCancellable *cancellable, GError **error) {
  GProxyResolverInterface *iface;
  g_return_val_if_fail(G_IS_PROXY_RESOLVER(resolver), NULL);
  g_return_val_if_fail(uri != NULL, NULL);
  iface = G_PROXY_RESOLVER_GET_IFACE(resolver);
  return (*iface->lookup)(resolver, uri, cancellable, error);
}
void g_proxy_resolver_lookup_async(GProxyResolver *resolver, const gchar *uri, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GProxyResolverInterface *iface;
  g_return_if_fail (G_IS_PROXY_RESOLVER (resolver));
  g_return_if_fail (uri != NULL);
  iface = G_PROXY_RESOLVER_GET_IFACE (resolver);
  (* iface->lookup_async) (resolver, uri, cancellable, callback, user_data);
}
gchar **g_proxy_resolver_lookup_finish(GProxyResolver *resolver, GAsyncResult *result, GError **error) {
  GProxyResolverInterface *iface;
  g_return_val_if_fail(G_IS_PROXY_RESOLVER(resolver), NULL);
  iface = G_PROXY_RESOLVER_GET_IFACE(resolver);
  return (*iface->lookup_finish)(resolver, result, error);
}