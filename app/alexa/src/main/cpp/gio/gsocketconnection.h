#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_SOCKET_CONNECTION_H__
#define __G_SOCKET_CONNECTION_H__

#include "../glib/glib-object.h"
#include "gsocket.h"
#include "giostream.h"

G_BEGIN_DECLS
#define G_TYPE_SOCKET_CONNECTION   (g_socket_connection_get_type())
#define G_SOCKET_CONNECTION(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_SOCKET_CONNECTION, GSocketConnection))
#define G_SOCKET_CONNECTION_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_SOCKET_CONNECTION, GSocketConnectionClass))
#define G_IS_SOCKET_CONNECTION(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_SOCKET_CONNECTION))
#define G_IS_SOCKET_CONNECTION_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_SOCKET_CONNECTION))
#define G_SOCKET_CONNECTION_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_SOCKET_CONNECTION, GSocketConnectionClass))
typedef struct _GSocketConnectionPrivate GSocketConnectionPrivate;
typedef struct _GSocketConnectionClass GSocketConnectionClass;
struct _GSocketConnectionClass {
  GIOStreamClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
  void (*_g_reserved6)(void);
};
struct _GSocketConnection {
  GIOStream parent_instance;
  GSocketConnectionPrivate *priv;
};
GType g_socket_connection_get_type(void) G_GNUC_CONST;
GSocket *g_socket_connection_get_socket(GSocketConnection *connection);
GSocketAddress *g_socket_connection_get_local_address(GSocketConnection *connection, GError **error);
GSocketAddress *g_socket_connection_get_remote_address(GSocketConnection *connection, GError **error);
void g_socket_connection_factory_register_type(GType g_type, GSocketFamily family, GSocketType type, gint protocol);
GType g_socket_connection_factory_lookup_type(GSocketFamily family, GSocketType type, gint protocol_id);
GSocketConnection *g_socket_connection_factory_create_connection(GSocket *socket);
G_END_DECLS

#endif