#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_SOCKET_CONNECTABLE_H__
#define __G_SOCKET_CONNECTABLE_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_SOCKET_CONNECTABLE  (g_socket_connectable_get_type())
#define G_SOCKET_CONNECTABLE(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_SOCKET_CONNECTABLE, GSocketConnectable))
#define G_IS_SOCKET_CONNECTABLE(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_SOCKET_CONNECTABLE))
#define G_SOCKET_CONNECTABLE_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_SOCKET_CONNECTABLE, GSocketConnectableIface))
typedef struct _GSocketConnectableIface GSocketConnectableIface;
struct _GSocketConnectableIface {
  GTypeInterface g_iface;
  GSocketAddressEnumerator *(*enumerate)(GSocketConnectable *connectable);
  GSocketAddressEnumerator *(*proxy_enumerate)(GSocketConnectable *connectable);

};
GType g_socket_connectable_get_type(void) G_GNUC_CONST;
GSocketAddressEnumerator *g_socket_connectable_enumerate(GSocketConnectable *connectable);
GSocketAddressEnumerator *g_socket_connectable_proxy_enumerate(GSocketConnectable *connectable);
G_END_DECLS

#endif