#ifndef __G_UNIX_SOCKET_ADDRESS_H__
#define __G_UNIX_SOCKET_ADDRESS_H__

G_BEGIN_DECLS
#define G_TYPE_UNIX_SOCKET_ADDRESS  (g_unix_socket_address_get_type())
#define G_UNIX_SOCKET_ADDRESS(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_UNIX_SOCKET_ADDRESS, GUnixSocketAddress))
#define G_UNIX_SOCKET_ADDRESS_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_UNIX_SOCKET_ADDRESS, GUnixSocketAddressClass))
#define G_IS_UNIX_SOCKET_ADDRESS(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_UNIX_SOCKET_ADDRESS))
#define G_IS_UNIX_SOCKET_ADDRESS_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_UNIX_SOCKET_ADDRESS))
#define G_UNIX_SOCKET_ADDRESS_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_UNIX_SOCKET_ADDRESS, GUnixSocketAddressClass))
typedef struct _GUnixSocketAddress GUnixSocketAddress;
typedef struct _GUnixSocketAddressClass GUnixSocketAddressClass;
typedef struct _GUnixSocketAddressPrivate GUnixSocketAddressPrivate;
struct _GUnixSocketAddress {
  GSocketAddress parent_instance;
  GUnixSocketAddressPrivate *priv;
};
struct _GUnixSocketAddressClass {
  GSocketAddressClass parent_class;
};
GType g_unix_socket_address_get_type(void) G_GNUC_CONST;
GSocketAddress *g_unix_socket_address_new(const gchar *path);
#ifndef G_DISABLE_DEPRECATED
GSocketAddress *g_unix_socket_address_new_abstract(const gchar *path, gint path_len);
#endif
GSocketAddress *g_unix_socket_address_new_with_type(const gchar *path, gint path_len, GUnixSocketAddressType type);
const char *g_unix_socket_address_get_path(GUnixSocketAddress *address);
gsize g_unix_socket_address_get_path_len(GUnixSocketAddress *address);
GUnixSocketAddressType g_unix_socket_address_get_address_type(GUnixSocketAddress *address);
#ifndef G_DISABLE_DEPRECATED
gboolean g_unix_socket_address_get_is_abstract(GUnixSocketAddress *address);
#endif
gboolean g_unix_socket_address_abstract_names_supported(void);
G_END_DECLS

#endif