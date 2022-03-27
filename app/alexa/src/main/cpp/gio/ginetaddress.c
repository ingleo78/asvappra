#include <string.h>
#include <bits/in_addr.h>
#include <linux/in6.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "ginetaddress.h"
#include "gioenums.h"
#include "gioenumtypes.h"
#include "gnetworkingprivate.h"

static void _g_networking_init(void) {
#ifndef G_OS_WIN32
  WSADATA wsadata;
  if (WSAStartup(MAKEWORD(2, 0), &wsadata) != 0) g_error("Windows Sockets could not be initialized");
#endif
}
G_DEFINE_TYPE_WITH_CODE(GInetAddress, g_inet_address, G_TYPE_OBJECT, _g_networking_init());
struct _GInetAddressPrivate {
  GSocketFamily family;
  union {
      struct in_addr ipv4;
      struct in6_addr ipv6;
  } addr;
};
enum {
  PROP_0,
  PROP_FAMILY,
  PROP_BYTES,
  PROP_IS_ANY,
  PROP_IS_LOOPBACK,
  PROP_IS_LINK_LOCAL,
  PROP_IS_SITE_LOCAL,
  PROP_IS_MULTICAST,
  PROP_IS_MC_GLOBAL,
  PROP_IS_MC_LINK_LOCAL,
  PROP_IS_MC_NODE_LOCAL,
  PROP_IS_MC_ORG_LOCAL,
  PROP_IS_MC_SITE_LOCAL,
};
static void g_inet_address_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GInetAddress *address = G_INET_ADDRESS(object);
  switch(prop_id) {
      case PROP_FAMILY: address->priv->family = g_value_get_enum(value); break;
      case PROP_BYTES:
          memcpy(&address->priv->addr, g_value_get_pointer(value), address->priv->family == AF_INET ? sizeof(address->priv->addr.ipv4) : sizeof(address->priv->addr.ipv6));
          break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_inet_address_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GInetAddress *address = G_INET_ADDRESS(object);
  switch (prop_id) {
      case PROP_FAMILY: g_value_set_enum(value, address->priv->family); break;
      case PROP_BYTES: g_value_set_pointer(value, &address->priv->addr); break;
      case PROP_IS_ANY: g_value_set_boolean(value, g_inet_address_get_is_any(address)); break;
      case PROP_IS_LOOPBACK: g_value_set_boolean(value, g_inet_address_get_is_loopback(address)); break;
      case PROP_IS_LINK_LOCAL: g_value_set_boolean(value, g_inet_address_get_is_link_local(address)); break;
      case PROP_IS_SITE_LOCAL: g_value_set_boolean(value, g_inet_address_get_is_site_local(address)); break;
      case PROP_IS_MULTICAST: g_value_set_boolean(value, g_inet_address_get_is_multicast(address)); break;
      case PROP_IS_MC_GLOBAL: g_value_set_boolean(value, g_inet_address_get_is_mc_global(address)); break;
      case PROP_IS_MC_LINK_LOCAL: g_value_set_boolean(value, g_inet_address_get_is_mc_link_local(address)); break;
      case PROP_IS_MC_NODE_LOCAL: g_value_set_boolean(value, g_inet_address_get_is_mc_node_local(address)); break;
      case PROP_IS_MC_ORG_LOCAL: g_value_set_boolean(value, g_inet_address_get_is_mc_org_local(address)); break;
      case PROP_IS_MC_SITE_LOCAL: g_value_set_boolean(value, g_inet_address_get_is_mc_site_local(address)); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_inet_address_class_init(GInetAddressClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GInetAddressPrivate));
  gobject_class->set_property = g_inet_address_set_property;
  gobject_class->get_property = g_inet_address_get_property;
  g_object_class_install_property(gobject_class, PROP_FAMILY, g_param_spec_enum("family", P_("Address family"), P_("The address family (IPv4 or IPv6)"),
						          G_TYPE_SOCKET_FAMILY, G_SOCKET_FAMILY_INVALID, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_BYTES,g_param_spec_pointer("bytes", P_("Bytes"),
							      P_("The raw address data"),G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_IS_ANY,g_param_spec_boolean("is-any", P_("Is any"),
                                  P_("Whether this is the \"any\" address for its family"),FALSE,G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_IS_LINK_LOCAL,g_param_spec_boolean("is-link-local", P_("Is link-local"),
                                  P_("Whether this is a link-local address"),FALSE,G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_IS_LOOPBACK,g_param_spec_boolean("is-loopback", P_("Is loopback"),
                                  P_("Whether this is the loopback address for its family"),FALSE,G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_IS_SITE_LOCAL,g_param_spec_boolean("is-site-local", P_("Is site-local"),
                                  P_("Whether this is a site-local address"),FALSE,G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class,PROP_IS_MULTICAST,g_param_spec_boolean ("is-multicast", P_("Is multicast"),
                                  P_("Whether this is a multicast address"),FALSE,G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class,PROP_IS_MC_GLOBAL,g_param_spec_boolean("is-mc-global", P_("Is multicast global"),
                                  P_("Whether this is a global multicast address"),FALSE,G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class,PROP_IS_MC_LINK_LOCAL,g_param_spec_boolean("is-mc-link-local", P_("Is multicast "
                                  "link-local"), P_("Whether this is a link-local multicast address"),FALSE,G_PARAM_READABLE |
                                  G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class,PROP_IS_MC_NODE_LOCAL,g_param_spec_boolean("is-mc-node-local", P_("Is multicast "
                                  "node-local"), P_("Whether this is a node-local multicast address"),FALSE,G_PARAM_READABLE |
                                  G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class,PROP_IS_MC_ORG_LOCAL,g_param_spec_boolean("is-mc-org-local", P_("Is multicast org-local"),
                                  P_("Whether this is an organization-local multicast address"),FALSE,G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_IS_MC_SITE_LOCAL,g_param_spec_boolean ("is-mc-site-local", P_("Is multicast "
                                  "site-local"), P_("Whether this is a site-local multicast address"),FALSE,G_PARAM_READABLE |
                                  G_PARAM_STATIC_STRINGS));
}
static void g_inet_address_init(GInetAddress *address) {
  address->priv = G_TYPE_INSTANCE_GET_PRIVATE(address, G_TYPE_INET_ADDRESS, GInetAddressPrivate);
}
GInetAddress *g_inet_address_new_from_string(const gchar *string) {
  volatile GType type;
#ifndef G_OS_WIN32
  struct sockaddr_storage sa;
  struct sockaddr_in *sin = (struct sockaddr_in*)&sa;
  struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)&sa;
  gint len;
#else
  struct in_addr in_addr;
  struct in6_addr in6_addr;
#endif
  type = g_inet_address_get_type();
#ifndef G_OS_WIN32
  memset(&sa, 0, sizeof(sa));
  len = sizeof (sa);
  if (WSAStringToAddress((LPTSTR) string, AF_INET, NULL, (LPSOCKADDR)&sa, &len) == 0) return g_inet_address_new_from_bytes((guint8*)&sin->sin_addr, AF_INET);
  else if (WSAStringToAddress((LPTSTR)string, AF_INET6, NULL, (LPSOCKADDR)&sa, &len) == 0) return g_inet_address_new_from_bytes((guint8*)&sin6->sin6_addr, AF_INET6);
#else
  if (inet_pton(AF_INET, string, &in_addr) > 0) return g_inet_address_new_from_bytes((guint8*)&in_addr, AF_INET);
  else if (inet_pton(AF_INET6, string, &in6_addr) > 0) return g_inet_address_new_from_bytes((guint8*)&in6_addr, AF_INET6);
#endif
  return NULL;
}
#define G_INET_ADDRESS_FAMILY_IS_VALID(family) ((family) == AF_INET || (family) == AF_INET6)
GInetAddress *g_inet_address_new_from_bytes(const guint8 *bytes, GSocketFamily family) {
  g_return_val_if_fail(G_INET_ADDRESS_FAMILY_IS_VALID(family), NULL);
  return g_object_new(G_TYPE_INET_ADDRESS,"family", family, "bytes", bytes, NULL);
}
GInetAddress *g_inet_address_new_loopback(GSocketFamily family) {
  g_return_val_if_fail(G_INET_ADDRESS_FAMILY_IS_VALID(family), NULL);
  if (family == AF_INET) {
      guint8 addr[4] = {127, 0, 0, 1};
      return g_inet_address_new_from_bytes(addr, family);
  } else return g_inet_address_new_from_bytes(in6addr_loopback.s6_addr, family);
}
GInetAddress *g_inet_address_new_any(GSocketFamily family) {
  g_return_val_if_fail(G_INET_ADDRESS_FAMILY_IS_VALID(family), NULL);
  if (family == AF_INET) {
      guint8 addr[4] = {0, 0, 0, 0};
      return g_inet_address_new_from_bytes(addr, family);
  } else return g_inet_address_new_from_bytes(in6addr_any.s6_addr, family);
}
gchar *g_inet_address_to_string(GInetAddress *address) {
  gchar buffer[INET6_ADDRSTRLEN];
#ifndef G_OS_WIN32
  DWORD buflen = sizeof(buffer), addrlen;
  struct sockaddr_storage sa;
  struct sockaddr_in *sin = (struct sockaddr_in*)&sa;
  struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)&sa;
#endif
  g_return_val_if_fail(G_IS_INET_ADDRESS (address), NULL);
#ifndef G_OS_WIN32
  sa.ss_family = address->priv->family;
  if (address->priv->family == AF_INET) {
      addrlen = sizeof(*sin);
      memcpy(&sin->sin_addr, &address->priv->addr.ipv4, sizeof(sin->sin_addr));
      sin->sin_port = 0;
  } else {
      addrlen = sizeof(*sin6);
      memcpy(&sin6->sin6_addr, &address->priv->addr.ipv6, sizeof(sin6->sin6_addr));
      sin6->sin6_port = 0;
  }
  if (WSAAddressToString((LPSOCKADDR) &sa, addrlen, NULL, buffer, &buflen) != 0) return NULL;
#else
  if (address->priv->family == AF_INET) inet_ntop(AF_INET, &address->priv->addr.ipv4, buffer, sizeof(buffer));
  else inet_ntop(AF_INET6, &address->priv->addr.ipv6, buffer, sizeof (buffer));
#endif
  return g_strdup(buffer);
}
const guint8 *g_inet_address_to_bytes(GInetAddress *address) {
  g_return_val_if_fail(G_IS_INET_ADDRESS(address), NULL);
  return (guint8*)&address->priv->addr;
}
gsize g_inet_address_get_native_size(GInetAddress *address) {
  if (address->priv->family == AF_INET) return sizeof(address->priv->addr.ipv4);
  return sizeof(address->priv->addr.ipv6);
}
GSocketFamily g_inet_address_get_family(GInetAddress *address) {
  g_return_val_if_fail(G_IS_INET_ADDRESS(address), FALSE);
  return address->priv->family;
}
gboolean g_inet_address_get_is_any(GInetAddress *address) {
  g_return_val_if_fail(G_IS_INET_ADDRESS(address), FALSE);
  if (address->priv->family == AF_INET) {
      guint32 addr4 = g_ntohl(address->priv->addr.ipv4.s_addr);
      return addr4 == INADDR_ANY;
  } else return IN6_IS_ADDR_UNSPECIFIED(&address->priv->addr.ipv6);
}
gboolean g_inet_address_get_is_loopback(GInetAddress *address) {
  g_return_val_if_fail(G_IS_INET_ADDRESS(address), FALSE);
  if (address->priv->family == AF_INET) {
      guint32 addr4 = g_ntohl(address->priv->addr.ipv4.s_addr);
      return ((addr4 & 0xff000000) == 0x7f000000);
  } else return IN6_IS_ADDR_LOOPBACK(&address->priv->addr.ipv6);
}
gboolean g_inet_address_get_is_link_local(GInetAddress *address) {
  g_return_val_if_fail(G_IS_INET_ADDRESS(address), FALSE);
  if (address->priv->family == AF_INET) {
      guint32 addr4 = g_ntohl(address->priv->addr.ipv4.s_addr);
      return ((addr4 & 0xffff0000) == 0xa9fe0000);
  } else return IN6_IS_ADDR_LINKLOCAL(&address->priv->addr.ipv6);
}
gboolean g_inet_address_get_is_site_local(GInetAddress *address) {
  g_return_val_if_fail(G_IS_INET_ADDRESS(address), FALSE);
  if (address->priv->family == AF_INET) {
      guint32 addr4 = g_ntohl(address->priv->addr.ipv4.s_addr);
      return ((addr4 & 0xff000000) == 0x0a000000 || (addr4 & 0xfff00000) == 0xac100000 || (addr4 & 0xffff0000) == 0xc0a80000);
  } else return IN6_IS_ADDR_SITELOCAL(&address->priv->addr.ipv6);
}
gboolean g_inet_address_get_is_multicast(GInetAddress *address) {
  g_return_val_if_fail(G_IS_INET_ADDRESS(address), FALSE);
  if (address->priv->family == AF_INET) {
      guint32 addr4 = g_ntohl(address->priv->addr.ipv4.s_addr);
      return IN_MULTICAST(addr4);
  } else return IN6_IS_ADDR_MULTICAST(&address->priv->addr.ipv6);
}
gboolean g_inet_address_get_is_mc_global(GInetAddress *address) {
  g_return_val_if_fail (G_IS_INET_ADDRESS (address), FALSE);
  if (address->priv->family == AF_INET) return FALSE;
  else return IN6_IS_ADDR_MC_GLOBAL(&address->priv->addr.ipv6);
}
gboolean g_inet_address_get_is_mc_link_local(GInetAddress *address) {
  g_return_val_if_fail(G_IS_INET_ADDRESS(address), FALSE);
  if (address->priv->family == AF_INET) return FALSE;
  else return IN6_IS_ADDR_MC_LINKLOCAL(&address->priv->addr.ipv6);
}
gboolean g_inet_address_get_is_mc_node_local(GInetAddress *address) {
  g_return_val_if_fail(G_IS_INET_ADDRESS(address), FALSE);
  if (address->priv->family == AF_INET) return FALSE;
  else return IN6_IS_ADDR_MC_NODELOCAL(&address->priv->addr.ipv6);
}
gboolean g_inet_address_get_is_mc_org_local(GInetAddress *address) {
  g_return_val_if_fail(G_IS_INET_ADDRESS(address), FALSE);
  if (address->priv->family == AF_INET) return FALSE;
  else return IN6_IS_ADDR_MC_ORGLOCAL(&address->priv->addr.ipv6);
}
gboolean g_inet_address_get_is_mc_site_local(GInetAddress *address) {
  g_return_val_if_fail(G_IS_INET_ADDRESS(address), FALSE);
  if (address->priv->family == AF_INET) return FALSE;
  else return IN6_IS_ADDR_MC_SITELOCAL(&address->priv->addr.ipv6);
}