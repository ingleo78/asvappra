#include "../glib/glibintl.h"
#include "config.h"
#include "gsocketconnectable.h"

typedef GSocketConnectableIface GSocketConnectableInterface;
G_DEFINE_INTERFACE(GSocketConnectable, g_socket_connectable, G_TYPE_OBJECT);
static void g_socket_connectable_default_init(GSocketConnectableInterface *iface) {}
GSocketAddressEnumerator *g_socket_connectable_enumerate(GSocketConnectable *connectable) {
  GSocketConnectableIface *iface;
  g_return_val_if_fail(G_IS_SOCKET_CONNECTABLE(connectable), NULL);
  iface = G_SOCKET_CONNECTABLE_GET_IFACE(connectable);
  return (*iface->enumerate)(connectable);
}
GSocketAddressEnumerator *g_socket_connectable_proxy_enumerate(GSocketConnectable *connectable) {
  GSocketConnectableIface *iface;
  g_return_val_if_fail(G_IS_SOCKET_CONNECTABLE(connectable), NULL);
  iface = G_SOCKET_CONNECTABLE_GET_IFACE(connectable);
  if (iface->proxy_enumerate) return (*iface->proxy_enumerate)(connectable);
  else return (*iface->enumerate)(connectable);
}