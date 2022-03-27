#include <string.h>
#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "../glib/glib-object.h"
#include "config.h"
#include "ginetsocketaddress.h"
#include "ginetaddress.h"
#include "gnetworkingprivate.h"
#include "gioerror.h"

G_DEFINE_TYPE(GInetSocketAddress, g_inet_socket_address, G_TYPE_SOCKET_ADDRESS);
enum {
  PROP_0,
  PROP_ADDRESS,
  PROP_PORT
};
struct _GInetSocketAddressPrivate {
  GInetAddress *address;
  guint16 port;
};
static void g_inet_socket_address_finalize(GObject *object) {
  GInetSocketAddress *address G_GNUC_UNUSED = G_INET_SOCKET_ADDRESS(object);
  if (G_OBJECT_CLASS(g_inet_socket_address_parent_class)->finalize) (*G_OBJECT_CLASS(g_inet_socket_address_parent_class)->finalize)(object);
}
static void g_inet_socket_address_dispose(GObject *object) {
  GInetSocketAddress *address G_GNUC_UNUSED = G_INET_SOCKET_ADDRESS(object);
  g_object_unref(address->priv->address);
  if (G_OBJECT_CLASS(g_inet_socket_address_parent_class)->dispose) (*G_OBJECT_CLASS(g_inet_socket_address_parent_class)->dispose)(object);
}
static void g_inet_socket_address_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GInetSocketAddress *address = G_INET_SOCKET_ADDRESS(object);
  switch(prop_id) {
      case PROP_ADDRESS: g_value_set_object(value, address->priv->address); break;
      case PROP_PORT: g_value_set_uint(value, address->priv->port); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_inet_socket_address_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GInetSocketAddress *address = G_INET_SOCKET_ADDRESS(object);
  switch(prop_id) {
      case PROP_ADDRESS: address->priv->address = g_object_ref(g_value_get_object(value)); break;
      case PROP_PORT: address->priv->port = (guint16)g_value_get_uint(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static GSocketFamily g_inet_socket_address_get_family(GSocketAddress *address) {
  GInetSocketAddress *addr;
  g_return_val_if_fail(G_IS_INET_SOCKET_ADDRESS(address), 0);
  addr = G_INET_SOCKET_ADDRESS(address);
  return g_inet_address_get_family(addr->priv->address);
}
static gssize g_inet_socket_address_get_native_size(GSocketAddress *address) {
  GInetSocketAddress *addr;
  GSocketFamily family;
  g_return_val_if_fail(G_IS_INET_SOCKET_ADDRESS(address), 0);
  addr = G_INET_SOCKET_ADDRESS(address);
  family = g_inet_address_get_family(addr->priv->address);
  if (family == AF_INET) return sizeof(struct sockaddr_in);
  else if (family == AF_INET6) return sizeof(struct sockaddr_in6);
  else return -1;
}
static gboolean g_inet_socket_address_to_native(GSocketAddress *address, gpointer dest, gsize destlen, GError **error) {
  GInetSocketAddress *addr;
  GSocketFamily family;
  g_return_val_if_fail(G_IS_INET_SOCKET_ADDRESS(address), FALSE);
  addr = G_INET_SOCKET_ADDRESS(address);
  family = g_inet_address_get_family(addr->priv->address);
  if (family == AF_INET) {
      struct sockaddr_in *sock = (struct sockaddr_in*)dest;
      if (destlen < sizeof(*sock)) {
          g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NO_SPACE, _("Not enough space for socket address"));
          return FALSE;
	  }
      sock->sin_family = AF_INET;
      sock->sin_port = g_htons(addr->priv->port);
      memcpy(&(sock->sin_addr.s_addr), g_inet_address_to_bytes(addr->priv->address), sizeof(sock->sin_addr));
      memset(sock->sin_zero, 0, sizeof(sock->sin_zero));
      return TRUE;
  } else if (family == AF_INET6) {
      struct sockaddr_in6 *sock = (struct sockaddr_in6*) dest;
      if (destlen < sizeof(*sock)) {
          g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NO_SPACE, _("Not enough space for socket address"));
          return FALSE;
	  }
      memset(sock, 0, sizeof(*sock));
      sock->sin6_family = AF_INET6;
      sock->sin6_port = g_htons(addr->priv->port);
      memcpy(&(sock->sin6_addr.s6_addr), g_inet_address_to_bytes (addr->priv->address), sizeof(sock->sin6_addr));
      return TRUE;
  } else {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Unsupported socket address"));
      return FALSE;
  }
}
static void g_inet_socket_address_class_init(GInetSocketAddressClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GSocketAddressClass *gsocketaddress_class = G_SOCKET_ADDRESS_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GInetSocketAddressPrivate));
  gobject_class->finalize = g_inet_socket_address_finalize;
  gobject_class->dispose = g_inet_socket_address_dispose;
  gobject_class->set_property = g_inet_socket_address_set_property;
  gobject_class->get_property = g_inet_socket_address_get_property;
  gsocketaddress_class->get_family = g_inet_socket_address_get_family;
  gsocketaddress_class->to_native = g_inet_socket_address_to_native;
  gsocketaddress_class->get_native_size = g_inet_socket_address_get_native_size;
  g_object_class_install_property(gobject_class, PROP_ADDRESS, g_param_spec_object ("address", P_("Address"), P_("The address"), G_TYPE_INET_ADDRESS,
                                  G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_PORT,g_param_spec_uint ("port", P_("Port"), P_("The port"),0,
                         65535,0,G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
static void g_inet_socket_address_init(GInetSocketAddress *address) {
  address->priv = G_TYPE_INSTANCE_GET_PRIVATE(address, G_TYPE_INET_SOCKET_ADDRESS, GInetSocketAddressPrivate);
  address->priv->address = NULL;
  address->priv->port = 0;
}
GSocketAddress *g_inet_socket_address_new(GInetAddress *address, guint16 port) {
  return g_object_new(G_TYPE_INET_SOCKET_ADDRESS, "address", address, "port", port, NULL);
}
GInetAddress *g_inet_socket_address_get_address(GInetSocketAddress *address) {
  g_return_val_if_fail(G_IS_INET_SOCKET_ADDRESS(address), NULL);
  return address->priv->address;
}
guint16 g_inet_socket_address_get_port(GInetSocketAddress *address) {
  g_return_val_if_fail(G_IS_INET_SOCKET_ADDRESS(address), 0);
  return address->priv->port;
}