#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_NETWORK_ADDRESS_H__
#define __G_NETWORK_ADDRESS_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_NETWORK_ADDRESS  (g_network_address_get_type())
#define G_NETWORK_ADDRESS(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_NETWORK_ADDRESS, GNetworkAddress))
#define G_NETWORK_ADDRESS_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_NETWORK_ADDRESS, GNetworkAddressClass))
#define G_IS_NETWORK_ADDRESS(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_NETWORK_ADDRESS))
#define G_IS_NETWORK_ADDRESS_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_NETWORK_ADDRESS))
#define G_NETWORK_ADDRESS_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_NETWORK_ADDRESS, GNetworkAddressClass))
typedef struct _GNetworkAddressClass GNetworkAddressClass;
typedef struct _GNetworkAddressPrivate GNetworkAddressPrivate;
struct _GNetworkAddress {
  GObject parent_instance;
  GNetworkAddressPrivate *priv;
};
struct _GNetworkAddressClass {
  GObjectClass parent_class;
};
GType g_network_address_get_type(void) G_GNUC_CONST;
GSocketConnectable *g_network_address_new(const gchar *hostname, guint16 port);
GSocketConnectable *g_network_address_parse(const gchar *host_and_port, guint16 default_port, GError **error);
GSocketConnectable *g_network_address_parse_uri(const gchar *uri, guint16 default_port, GError **error);
const gchar *g_network_address_get_hostname(GNetworkAddress *addr);
guint16 g_network_address_get_port(GNetworkAddress *addr);
const gchar *g_network_address_get_scheme(GNetworkAddress *addr);
G_END_DECLS

#endif