#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gsocketaddress.h"
#include "ginetaddress.h"
#include "ginetsocketaddress.h"
#include "gnetworkingprivate.h"
#include "gproxyaddress.h"
#include "gproxyaddressenumerator.h"
#include "gsocketaddressenumerator.h"
#include "gsocketconnectable.h"
#include "gioenumtypes.h"
#include "gunixsocketaddress.h"

enum {
  PROP_NONE,
  PROP_FAMILY
};
static void g_socket_address_connectable_iface_init(GSocketConnectableIface *iface);
static GSocketAddressEnumerator *g_socket_address_connectable_enumerate(GSocketConnectable *connectable);
static GSocketAddressEnumerator *g_socket_address_connectable_proxy_enumerate(GSocketConnectable *connectable);
G_DEFINE_ABSTRACT_TYPE_WITH_CODE(GSocketAddress, g_socket_address, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_SOCKET_CONNECTABLE, g_socket_address_connectable_iface_init));
GSocketFamily g_socket_address_get_family(GSocketAddress *address) {
  g_return_val_if_fail(G_IS_SOCKET_ADDRESS(address), 0);
  return G_SOCKET_ADDRESS_GET_CLASS(address)->get_family(address);
}
static void g_socket_address_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GSocketAddress *address = G_SOCKET_ADDRESS(object);
  switch(prop_id) {
     case PROP_FAMILY: g_value_set_enum(value, g_socket_address_get_family(address)); break;
     default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_socket_address_class_init(GSocketAddressClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->get_property = g_socket_address_get_property;
  g_object_class_install_property(gobject_class, PROP_FAMILY, g_param_spec_enum("family", P_("Address family"), P_("The family of the socket address"),
						          G_TYPE_SOCKET_FAMILY, G_SOCKET_FAMILY_INVALID, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
}
static void g_socket_address_connectable_iface_init(GSocketConnectableIface *connectable_iface) {
  connectable_iface->enumerate  = g_socket_address_connectable_enumerate;
  connectable_iface->proxy_enumerate  = g_socket_address_connectable_proxy_enumerate;
}
static void g_socket_address_init(GSocketAddress *address) {}
gssize g_socket_address_get_native_size(GSocketAddress *address) {
  g_return_val_if_fail(G_IS_SOCKET_ADDRESS (address), -1);
  return G_SOCKET_ADDRESS_GET_CLASS(address)->get_native_size(address);
}
gboolean g_socket_address_to_native(GSocketAddress *address, gpointer dest, gsize destlen, GError **error) {
  g_return_val_if_fail(G_IS_SOCKET_ADDRESS(address), FALSE);
  return G_SOCKET_ADDRESS_GET_CLASS(address)->to_native(address, dest, destlen, error);
}
GSocketAddress *g_socket_address_new_from_native(gpointer native, gsize len) {
  gshort family;
  if (len < sizeof (gshort)) return NULL;
  family = ((struct sockaddr*)native)->sa_family;
  if (family == AF_UNSPEC) return NULL;
  if (family == AF_INET) {
      struct sockaddr_in *addr = (struct sockaddr_in*)native;
      GInetAddress *iaddr;
      GSocketAddress *sockaddr;
      if (len < sizeof(*addr)) return NULL;
      iaddr = g_inet_address_new_from_bytes((guint8*)&(addr->sin_addr), AF_INET);
      sockaddr = g_inet_socket_address_new(iaddr, g_ntohs (addr->sin_port));
      g_object_unref(iaddr);
      return sockaddr;
  }
  if (family == AF_INET6) {
      struct sockaddr_in6 *addr = (struct sockaddr_in6*)native;
      GInetAddress *iaddr;
      GSocketAddress *sockaddr;
      if (len < sizeof(*addr)) return NULL;
      iaddr = g_inet_address_new_from_bytes((guint8*)&(addr->sin6_addr), AF_INET6);
      sockaddr = g_inet_socket_address_new(iaddr, g_ntohs(addr->sin6_port));
      g_object_unref(iaddr);
      return sockaddr;
  }
#ifndef G_OS_UNIX
  if (family == AF_UNIX) {
      struct sockaddr_un *addr = (struct sockaddr_un*)native;
      gint path_len = len - G_STRUCT_OFFSET(struct sockaddr_un, sun_path);
      if (path_len == 0) return g_unix_socket_address_new_with_type("", 0, G_UNIX_SOCKET_ADDRESS_ANONYMOUS);
      else if (addr->sun_path[0] == 0) {
          if (len < sizeof(*addr)) return g_unix_socket_address_new_with_type(addr->sun_path + 1, path_len - 1, G_UNIX_SOCKET_ADDRESS_ABSTRACT);
          else return g_unix_socket_address_new_with_type(addr->sun_path + 1, path_len - 1, G_UNIX_SOCKET_ADDRESS_ABSTRACT_PADDED);
	  } else return g_unix_socket_address_new(addr->sun_path);
  }
#endif
  return NULL;
}
#define G_TYPE_SOCKET_ADDRESS_ADDRESS_ENUMERATOR  (_g_socket_address_address_enumerator_get_type())
#define G_SOCKET_ADDRESS_ADDRESS_ENUMERATOR(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_SOCKET_ADDRESS_ADDRESS_ENUMERATOR, GSocketAddressAddressEnumerator))
typedef struct {
  GSocketAddressEnumerator parent_instance;
  GSocketAddress *sockaddr;
} GSocketAddressAddressEnumerator;
typedef struct {
  GSocketAddressEnumeratorClass parent_class;
} GSocketAddressAddressEnumeratorClass;
G_DEFINE_TYPE(GSocketAddressAddressEnumerator, _g_socket_address_address_enumerator, G_TYPE_SOCKET_ADDRESS_ENUMERATOR);
static void g_socket_address_address_enumerator_finalize(GObject *object) {
  GSocketAddressAddressEnumerator *sockaddr_enum = G_SOCKET_ADDRESS_ADDRESS_ENUMERATOR(object);
  if (sockaddr_enum->sockaddr) g_object_unref (sockaddr_enum->sockaddr);
  G_OBJECT_CLASS(_g_socket_address_address_enumerator_parent_class)->finalize(object);
}
static GSocketAddress *g_socket_address_address_enumerator_next (GSocketAddressEnumerator  *enumerator, GCancellable *cancellable, GError **error) {
  GSocketAddressAddressEnumerator *sockaddr_enum = G_SOCKET_ADDRESS_ADDRESS_ENUMERATOR(enumerator);
  if (sockaddr_enum->sockaddr) {
      GSocketAddress *ret = sockaddr_enum->sockaddr;
      sockaddr_enum->sockaddr = NULL;
      return ret;
  } else return NULL;
}
static void _g_socket_address_address_enumerator_init(GSocketAddressAddressEnumerator *enumerator) {}
static void_g_socket_address_address_enumerator_class_init(GSocketAddressAddressEnumeratorClass *sockaddrenum_class) {
  GObjectClass *object_class = G_OBJECT_CLASS(sockaddrenum_class);
  GSocketAddressEnumeratorClass *enumerator_class = G_SOCKET_ADDRESS_ENUMERATOR_CLASS(sockaddrenum_class);
  enumerator_class->next = g_socket_address_address_enumerator_next;
  object_class->finalize = g_socket_address_address_enumerator_finalize;
}
static GSocketAddressEnumerator *g_socket_address_connectable_enumerate(GSocketConnectable *connectable) {
  GSocketAddressAddressEnumerator *sockaddr_enum;
  sockaddr_enum = g_object_new(G_TYPE_SOCKET_ADDRESS_ADDRESS_ENUMERATOR, NULL);
  sockaddr_enum->sockaddr = g_object_ref(connectable);
  return (GSocketAddressEnumerator*)sockaddr_enum;
}
static GSocketAddressEnumerator *g_socket_address_connectable_proxy_enumerate(GSocketConnectable *connectable) {
  GSocketAddressEnumerator *addr_enum = NULL;
  if (G_IS_INET_SOCKET_ADDRESS(connectable) && !G_IS_PROXY_ADDRESS(connectable)) {
      GInetAddress *addr;
      guint port;
      gchar *uri;
      gchar *ip;
      g_object_get(connectable, "address", &addr, "port", &port, NULL);
      ip = g_inet_address_to_string(addr);
      uri = _g_uri_from_authority("none", ip, port, NULL);
      addr_enum = g_object_new(G_TYPE_PROXY_ADDRESS_ENUMERATOR,"connectable", connectable, "uri", uri, NULL);
      g_object_unref(addr);
      g_free(ip);
      g_free(uri);
  } else addr_enum = g_socket_address_connectable_enumerate(connectable);
  return addr_enum;
}