#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_TLS_CLIENT_CONNECTION_H__
#define __G_TLS_CLIENT_CONNECTION_H__

#include "../gobject/gobject.h"
#include "giotypes.h"
#include "gtlsconnection.h"

G_BEGIN_DECLS
#define G_TYPE_TLS_CLIENT_CONNECTION   (g_tls_client_connection_get_type())
#define G_TLS_CLIENT_CONNECTION(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_TLS_CLIENT_CONNECTION, GTlsClientConnection))
#define G_IS_TLS_CLIENT_CONNECTION(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_TLS_CLIENT_CONNECTION))
#define G_TLS_CLIENT_CONNECTION_GET_INTERFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE((inst), G_TYPE_TLS_CLIENT_CONNECTION, GTlsClientConnectionInterface))
typedef struct _GTlsClientConnectionInterface GTlsClientConnectionInterface;
struct _GTlsClientConnectionInterface {
  GTypeInterface g_iface;
};
GType g_tls_client_connection_get_type(void) G_GNUC_CONST;
GIOStream *g_tls_client_connection_new(GIOStream *base_io_stream, GSocketConnectable *server_identity, GError **error);
GTlsCertificateFlags g_tls_client_connection_get_validation_flags(GTlsClientConnection *conn);
void g_tls_client_connection_set_validation_flags(GTlsClientConnection *conn, GTlsCertificateFlags flags);
GSocketConnectable *g_tls_client_connection_get_server_identity(GTlsClientConnection *conn);
void g_tls_client_connection_set_server_identity(GTlsClientConnection *conn, GSocketConnectable *identity);
gboolean g_tls_client_connection_get_use_ssl3(GTlsClientConnection *conn);
void g_tls_client_connection_set_use_ssl3(GTlsClientConnection *conn, gboolean use_ssl3);
GList *g_tls_client_connection_get_accepted_cas(GTlsClientConnection *conn);
G_END_DECLS

#endif