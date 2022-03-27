#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_TCP_WRAPPER_CONNECTION_H__
#define __G_TCP_WRAPPER_CONNECTION_H__

#include "gtcpconnection.h"

G_BEGIN_DECLS
#define G_TYPE_TCP_WRAPPER_CONNECTION  (g_tcp_wrapper_connection_get_type())
#define G_TCP_WRAPPER_CONNECTION(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_TCP_WRAPPER_CONNECTION, GTcpWrapperConnection))
#define G_TCP_WRAPPER_CONNECTION_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_TCP_WRAPPER_CONNECTION, GTcpWrapperConnectionClass))
#define G_IS_TCP_WRAPPER_CONNECTION(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_TCP_WRAPPER_CONNECTION))
#define G_IS_TCP_WRAPPER_CONNECTION_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_TCP_WRAPPER_CONNECTION))
#define G_TCP_WRAPPER_CONNECTION_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_TCP_WRAPPER_CONNECTION, GTcpWrapperConnectionClass))
typedef struct _GTcpWrapperConnectionPrivate GTcpWrapperConnectionPrivate;
typedef struct _GTcpWrapperConnectionClass GTcpWrapperConnectionClass;
struct _GTcpWrapperConnectionClass {
  GTcpConnectionClass parent_class;
};
struct _GTcpWrapperConnection {
  GTcpConnection parent_instance;
  GTcpWrapperConnectionPrivate *priv;
};
GType g_tcp_wrapper_connection_get_type(void) G_GNUC_CONST;
GSocketConnection *g_tcp_wrapper_connection_new(GIOStream *base_io_stream, GSocket *socket);
GIOStream *g_tcp_wrapper_connection_get_base_io_stream(GTcpWrapperConnection *conn);
G_END_DECLS

#endif