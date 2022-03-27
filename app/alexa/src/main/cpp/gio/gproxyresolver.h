#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_PROXY_RESOLVER_H__
#define __G_PROXY_RESOLVER_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_PROXY_RESOLVER  (g_proxy_resolver_get_type())
#define G_PROXY_RESOLVER(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_PROXY_RESOLVER, GProxyResolver))
#define G_IS_PROXY_RESOLVER(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_PROXY_RESOLVER))
#define G_PROXY_RESOLVER_GET_IFACE(o)  (G_TYPE_INSTANCE_GET_INTERFACE((o), G_TYPE_PROXY_RESOLVER, GProxyResolverInterface))
#define G_PROXY_RESOLVER_EXTENSION_POINT_NAME  "gio-proxy-resolver"
typedef struct _GProxyResolverInterface GProxyResolverInterface;
struct _GProxyResolverInterface {
  GTypeInterface g_iface;
  gboolean (*is_supported)(GProxyResolver *resolver);
  gchar	**(*lookup)(GProxyResolver *resolver, const gchar *uri, GCancellable *cancellable, GError **error);
  void (*lookup_async)(GProxyResolver *resolver, const gchar *uri, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gchar	**(*lookup_finish)(GProxyResolver *resolver, GAsyncResult *result, GError **error);
};
GType g_proxy_resolver_get_type(void) G_GNUC_CONST;
GProxyResolver *g_proxy_resolver_get_default(void);
gboolean g_proxy_resolver_is_supported(GProxyResolver *resolver);
gchar **g_proxy_resolver_lookup(GProxyResolver *resolver, const gchar *uri, GCancellable *cancellable, GError **error);
void g_proxy_resolver_lookup_async(GProxyResolver *resolver, const gchar *uri, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gchar **g_proxy_resolver_lookup_finish(GProxyResolver *resolver, GAsyncResult *result, GError **error);
G_END_DECLS

#endif