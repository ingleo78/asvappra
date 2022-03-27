#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_INET_SOCKET_ADDRESS_H__
#define __G_INET_SOCKET_ADDRESS_H__

#include "../gobject/gtype.h"
#include "gsocketaddress.h"

G_BEGIN_DECLS
#define G_TYPE_INET_SOCKET_ADDRESS  (g_inet_socket_address_get_type())
#define G_INET_SOCKET_ADDRESS(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_INET_SOCKET_ADDRESS, GInetSocketAddress))
#define G_INET_SOCKET_ADDRESS_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_INET_SOCKET_ADDRESS, GInetSocketAddressClass))
#define G_IS_INET_SOCKET_ADDRESS(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_INET_SOCKET_ADDRESS))
#define G_IS_INET_SOCKET_ADDRESS_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_INET_SOCKET_ADDRESS))
#define G_INET_SOCKET_ADDRESS_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_INET_SOCKET_ADDRESS, GInetSocketAddressClass))
typedef struct _GInetSocketAddressClass GInetSocketAddressClass;
typedef struct _GInetSocketAddressPrivate GInetSocketAddressPrivate;
struct _GInetSocketAddress {
  GSocketAddress parent_instance;
  GInetSocketAddressPrivate *priv;
};
struct _GInetSocketAddressClass {
  GSocketAddressClass parent_class;
};
GType g_inet_socket_address_get_type(void) G_GNUC_CONST;
GSocketAddress *g_inet_socket_address_new(GInetAddress *address, guint16 port);
GInetAddress *g_inet_socket_address_get_address(GInetSocketAddress *address);
guint16 g_inet_socket_address_get_port(GInetSocketAddress *address);
G_END_DECLS

#endif