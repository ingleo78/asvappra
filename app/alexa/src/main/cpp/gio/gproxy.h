#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_PROXY_H__
#define __G_PROXY_H__

#include "../gobject/gtype.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_PROXY  (g_proxy_get_type())
#define G_PROXY(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_PROXY, GProxy))
#define G_IS_PROXY(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_PROXY))
#define G_PROXY_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_PROXY, GProxyInterface))
#define G_PROXY_EXTENSION_POINT_NAME  "gio-proxy"
typedef struct _GProxyInterface GProxyInterface;
struct _GProxyInterface {
  GTypeInterface g_iface;
  GIOStream *(*connect)(GProxy *proxy, GIOStream *connection, GProxyAddress *proxy_address, GCancellable *cancellable, GError **error);
  void (*connect_async)(GProxy *proxy, GIOStream *connection, GProxyAddress *proxy_address, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  GIOStream *(*connect_finish)(GProxy *proxy, GAsyncResult *result, GError **error);
  gboolean (*supports_hostname)(GProxy *proxy);
};
GType g_proxy_get_type(void) G_GNUC_CONST;
GProxy *g_proxy_get_default_for_protocol(const gchar *protocol);
GIOStream *g_proxy_connect(GProxy *proxy, GIOStream *connection, GProxyAddress *proxy_address, GCancellable *cancellable, GError **error);
void g_proxy_connect_async(GProxy *proxy, GIOStream *connection, GProxyAddress *proxy_address, GCancellable *cancellable, GAsyncReadyCallback callback,
				      	   gpointer user_data);
GIOStream *g_proxy_connect_finish(GProxy *proxy, GAsyncResult *result, GError **error);
gboolean g_proxy_supports_hostname(GProxy *proxy);
G_END_DECLS

#endif