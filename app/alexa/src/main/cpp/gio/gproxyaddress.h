#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_PROXY_ADDRESS_H__
#define __G_PROXY_ADDRESS_H__

#include "ginetsocketaddress.h"

G_BEGIN_DECLS
#define G_TYPE_PROXY_ADDRESS  (g_proxy_address_get_type())
#define G_PROXY_ADDRESS(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_PROXY_ADDRESS, GProxyAddress))
#define G_PROXY_ADDRESS_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_PROXY_ADDRESS, GProxyAddressClass))
#define G_IS_PROXY_ADDRESS(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_PROXY_ADDRESS))
#define G_IS_PROXY_ADDRESS_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_PROXY_ADDRESS))
#define G_PROXY_ADDRESS_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_PROXY_ADDRESS, GProxyAddressClass))
typedef struct _GProxyAddressClass GProxyAddressClass;
typedef struct _GProxyAddressPrivate GProxyAddressPrivate;
struct _GProxyAddress {
  GInetSocketAddress parent_instance;
  GProxyAddressPrivate *priv;
};
struct _GProxyAddressClass {
  GInetSocketAddressClass parent_class;
};
GType g_proxy_address_get_type(void) G_GNUC_CONST;
GSocketAddress *g_proxy_address_new(GInetAddress *inetaddr, guint16 port, const gchar *protocol, const gchar *dest_hostname, guint16 dest_port, const gchar *username,
					     			const gchar *password);
const gchar *g_proxy_address_get_protocol(GProxyAddress *proxy);
const gchar *g_proxy_address_get_destination_hostname(GProxyAddress *proxy);
guint16 g_proxy_address_get_destination_port(GProxyAddress *proxy);
const gchar *g_proxy_address_get_username(GProxyAddress *proxy);
const gchar *g_proxy_address_get_password(GProxyAddress *proxy);
G_END_DECLS

#endif