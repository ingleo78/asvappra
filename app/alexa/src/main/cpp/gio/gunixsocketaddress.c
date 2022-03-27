#include <string.h>
#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gio.h"
#include "gunixsocketaddress.h"
#include "gnetworkingprivate.h"

G_DEFINE_TYPE(GUnixSocketAddress, g_unix_socket_address, G_TYPE_SOCKET_ADDRESS);
enum {
  PROP_0,
  PROP_PATH,
  PROP_PATH_AS_ARRAY,
  PROP_ABSTRACT,
  PROP_ADDRESS_TYPE
};
#define UNIX_PATH_MAX sizeof(((struct sockaddr_un*)0)->sun_path)
struct _GUnixSocketAddressPrivate {
  char path[UNIX_PATH_MAX];
  gsize path_len;
  GUnixSocketAddressType address_type;
};
static void g_unix_socket_address_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GUnixSocketAddress *address = G_UNIX_SOCKET_ADDRESS (object);
  const char *str;
  GByteArray *array;
  gsize len;
  switch(prop_id) {
      case PROP_PATH:
          str = g_value_get_string(value);
          if (str) {
              g_strlcpy(address->priv->path, str, sizeof(address->priv->path));
              address->priv->path_len = strlen(address->priv->path);
          }
          break;
      case PROP_PATH_AS_ARRAY:
          array = g_value_get_boxed(value);
          if (array) {
              len = MIN(array->len, UNIX_PATH_MAX-1);
              memcpy(address->priv->path, array->data, len);
              address->priv->path[len] = 0;
              address->priv->path_len = len;
          }
          break;
      case PROP_ABSTRACT:
          if (address->priv->address_type != G_UNIX_SOCKET_ADDRESS_INVALID) return;
          if (g_value_get_boolean(value)) address->priv->address_type = G_UNIX_SOCKET_ADDRESS_ABSTRACT_PADDED;
          else address->priv->address_type = G_UNIX_SOCKET_ADDRESS_PATH;
          break;
      case PROP_ADDRESS_TYPE:
          if (address->priv->address_type != G_UNIX_SOCKET_ADDRESS_INVALID) return;
          address->priv->address_type = g_value_get_enum(value);
          break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}
static void g_unix_socket_address_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GUnixSocketAddress *address = G_UNIX_SOCKET_ADDRESS(object);
  GByteArray *array;
  switch (prop_id) {
      case PROP_PATH: g_value_set_string(value, address->priv->path); break;
      case PROP_PATH_AS_ARRAY:
          array = g_byte_array_sized_new(address->priv->path_len);
          g_byte_array_append(array, (guint8 *)address->priv->path, address->priv->path_len);
          g_value_take_boxed(value, array);
	  break;
      case PROP_ABSTRACT:
          g_value_set_boolean(value, (address->priv->address_type == G_UNIX_SOCKET_ADDRESS_ABSTRACT ||
                               address->priv->address_type == G_UNIX_SOCKET_ADDRESS_ABSTRACT_PADDED));
          break;
      case PROP_ADDRESS_TYPE: g_value_set_enum(value, address->priv->address_type); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static GSocketFamily g_unix_socket_address_get_family(GSocketAddress *address) {
  g_assert(PF_UNIX == G_SOCKET_FAMILY_UNIX);
  return G_SOCKET_FAMILY_UNIX;
}
static gssize g_unix_socket_address_get_native_size(GSocketAddress *address) {
  GUnixSocketAddress *addr = G_UNIX_SOCKET_ADDRESS(address);
  switch(addr->priv->address_type) {
      case G_UNIX_SOCKET_ADDRESS_ANONYMOUS: return G_STRUCT_OFFSET(struct sockaddr_un, sun_path);
      case G_UNIX_SOCKET_ADDRESS_ABSTRACT: return G_STRUCT_OFFSET(struct sockaddr_un, sun_path) + addr->priv->path_len + 1;
      default: return sizeof(struct sockaddr_un);
  }
}
static gboolean g_unix_socket_address_to_native(GSocketAddress *address, gpointer dest, gsize destlen, GError **error) {
  GUnixSocketAddress *addr = G_UNIX_SOCKET_ADDRESS(address);
  struct sockaddr_un *sock;
  gssize socklen;
  socklen = g_unix_socket_address_get_native_size(address);
  if (destlen < socklen) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NO_SPACE, _("Not enough space for socket address"));
      return FALSE;
  }
  sock = (struct sockaddr_un*)dest;
  memset(sock, 0, socklen);
  sock->sun_family = AF_UNIX;
  switch(addr->priv->address_type) {
      case G_UNIX_SOCKET_ADDRESS_INVALID: case G_UNIX_SOCKET_ADDRESS_ANONYMOUS: break;
      case G_UNIX_SOCKET_ADDRESS_PATH: strcpy (sock->sun_path, addr->priv->path); break;
      case G_UNIX_SOCKET_ADDRESS_ABSTRACT: case G_UNIX_SOCKET_ADDRESS_ABSTRACT_PADDED:
          if (!g_unix_socket_address_abstract_names_supported()) {
              g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Abstract unix domain socket addresses not supported on this system"));
              return FALSE;
          }
          sock->sun_path[0] = 0;
          memcpy(sock->sun_path+1, addr->priv->path, addr->priv->path_len);
          break;
  }
  return TRUE;
}
static void g_unix_socket_address_class_init(GUnixSocketAddressClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GSocketAddressClass *gsocketaddress_class = G_SOCKET_ADDRESS_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GUnixSocketAddressPrivate));
  gobject_class->set_property = g_unix_socket_address_set_property;
  gobject_class->get_property = g_unix_socket_address_get_property;
  gsocketaddress_class->get_family = g_unix_socket_address_get_family;
  gsocketaddress_class->to_native = g_unix_socket_address_to_native;
  gsocketaddress_class->get_native_size = g_unix_socket_address_get_native_size;
  g_object_class_install_property(gobject_class,PROP_PATH,g_param_spec_string("path", P_("Path"), P_("UNIX socket path"),
                      NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_PATH_AS_ARRAY, g_param_spec_boxed("path-as-array", P_("Path array"), P_("UNIX socket path, as "
                                  "byte array"), G_TYPE_BYTE_ARRAY, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class,PROP_ABSTRACT,g_param_spec_boolean("abstract", P_("Abstract"), P_("Whether or "
                                  "not this is an abstract address"),FALSE,G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_ADDRESS_TYPE, g_param_spec_enum("address-type", P_("Address type"), P_("The type of UNIX socket "
                                  "address"), G_TYPE_UNIX_SOCKET_ADDRESS_TYPE, G_UNIX_SOCKET_ADDRESS_PATH, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
						          G_PARAM_STATIC_STRINGS));
}
static void g_unix_socket_address_init(GUnixSocketAddress *address) {
  address->priv = G_TYPE_INSTANCE_GET_PRIVATE(address, G_TYPE_UNIX_SOCKET_ADDRESS, GUnixSocketAddressPrivate);
  memset(address->priv->path, 0, sizeof(address->priv->path));
  address->priv->path_len = -1;
  address->priv->address_type = G_UNIX_SOCKET_ADDRESS_INVALID;
}
GSocketAddress *g_unix_socket_address_new(const gchar *path) {
  return g_object_new(G_TYPE_UNIX_SOCKET_ADDRESS, "path", path, "abstract", FALSE, NULL);
}
GSocketAddress *g_unix_socket_address_new_abstract(const gchar *path, gint path_len) {
  return g_unix_socket_address_new_with_type(path, path_len,G_UNIX_SOCKET_ADDRESS_ABSTRACT_PADDED);
}
GSocketAddress *g_unix_socket_address_new_with_type(const gchar *path, gint path_len, GUnixSocketAddressType type) {
  GSocketAddress *address;
  GByteArray *array;
  if (type == G_UNIX_SOCKET_ADDRESS_ANONYMOUS) path_len = 0;
  else if (path_len == -1) path_len = strlen(path);
  array = g_byte_array_sized_new(path_len);
  g_byte_array_append(array, (guint8*)path, path_len);
  address = g_object_new(G_TYPE_UNIX_SOCKET_ADDRESS, "path-as-array", array, "address-type", type, NULL);
  g_byte_array_unref(array);
  return address;
}
const char *g_unix_socket_address_get_path(GUnixSocketAddress *address) {
  return address->priv->path;
}
gsize g_unix_socket_address_get_path_len(GUnixSocketAddress *address) {
  return address->priv->path_len;
}
GUnixSocketAddressType g_unix_socket_address_get_address_type(GUnixSocketAddress *address) {
  return address->priv->address_type;
}
gboolean g_unix_socket_address_get_is_abstract(GUnixSocketAddress *address) {
  return (address->priv->address_type == G_UNIX_SOCKET_ADDRESS_ABSTRACT || address->priv->address_type == G_UNIX_SOCKET_ADDRESS_ABSTRACT_PADDED);
}
gboolean g_unix_socket_address_abstract_names_supported(void) {
#ifdef __linux__
  return TRUE;
#else
  return FALSE;
#endif
}