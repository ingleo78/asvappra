#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_TCP_CONNECTION_H__
#define __G_TCP_CONNECTION_H__

#include "gsocketconnection.h"

G_BEGIN_DECLS
#define G_TYPE_TCP_CONNECTION   (g_tcp_connection_get_type ())
#define G_TCP_CONNECTION(inst)  (G_TYPE_CHECK_INSTANCE_CAST ((inst), G_TYPE_TCP_CONNECTION, GTcpConnection))
#define G_TCP_CONNECTION_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST ((class), G_TYPE_TCP_CONNECTION, GTcpConnectionClass))
#define G_IS_TCP_CONNECTION(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), G_TYPE_TCP_CONNECTION))
#define G_IS_TCP_CONNECTION_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), G_TYPE_TCP_CONNECTION))
#define G_TCP_CONNECTION_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), G_TYPE_TCP_CONNECTION, GTcpConnectionClass))
typedef struct _GTcpConnectionPrivate GTcpConnectionPrivate;
typedef struct _GTcpConnectionClass GTcpConnectionClass;
struct _GTcpConnectionClass {
  GSocketConnectionClass parent_class;
};
struct _GTcpConnection {
  GSocketConnection parent_instance;
  GTcpConnectionPrivate *priv;
};
GType g_tcp_connection_get_type(void) G_GNUC_CONST;
void g_tcp_connection_set_graceful_disconnect(GTcpConnection *connection, gboolean graceful_disconnect);
gboolean g_tcp_connection_get_graceful_disconnect(GTcpConnection *connection);
G_END_DECLS

#endif