#include "../glib/glibintl.h"
#include "config.h"
#include "gtlsclientconnection.h"
#include "ginitable.h"
#include "gioenumtypes.h"
#include "gsocket.h"
#include "gsocketconnectable.h"
#include "gtlsbackend.h"
#include "gtlscertificate.h"

G_DEFINE_INTERFACE(GTlsClientConnection, g_tls_client_connection, G_TYPE_TLS_CONNECTION);
static void g_tls_client_connection_default_init(GTlsClientConnectionInterface *iface) {
  g_object_interface_install_property(iface, g_param_spec_flags("validation-flags", P_("Validation flags"), P_("What certificate validation to perform"),
  		  				   		    G_TYPE_TLS_CERTIFICATE_FLAGS, G_TLS_CERTIFICATE_VALIDATE_ALL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));
  g_object_interface_install_property(iface, g_param_spec_object("server-identity", P_("Server identity"), P_("GSocketConnectable identifying the server"),
							    	  G_TYPE_SOCKET_CONNECTABLE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));
  g_object_interface_install_property(iface, g_param_spec_boolean("use-ssl3", P_("Use SSL3"), P_("Use SSL 3.0 rather than trying to use "
									  "TLS 1.x"),FALSE,G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));
  g_object_interface_install_property(iface,g_param_spec_pointer("accepted-cas", P_("Accepted CAs"), P_("Distinguished names of the CAs "
									  "the server accepts certificates from"),G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
}
GIOStream *g_tls_client_connection_new(GIOStream *base_io_stream, GSocketConnectable *server_identity, GError **error) {
  GObject *conn;
  GTlsBackend *backend;
  backend = g_tls_backend_get_default();
  conn = g_initable_new(g_tls_backend_get_client_connection_type(backend),NULL, error,"base-io-stream", base_io_stream, "server-identity",
			 		    server_identity, NULL);
  return G_IO_STREAM(conn);
}
GTlsCertificateFlags g_tls_client_connection_get_validation_flags(GTlsClientConnection *conn) {
  GTlsCertificateFlags flags = 0;
  g_return_val_if_fail(G_IS_TLS_CLIENT_CONNECTION(conn), 0);
  g_object_get(G_OBJECT(conn), "validation-flags", &flags, NULL);
  return flags;
}
void g_tls_client_connection_set_validation_flags(GTlsClientConnection *conn, GTlsCertificateFlags flags) {
  g_return_if_fail(G_IS_TLS_CLIENT_CONNECTION(conn));
  g_object_set(G_OBJECT(conn), "validation-flags", flags, NULL);
}
GSocketConnectable *g_tls_client_connection_get_server_identity(GTlsClientConnection *conn) {
  GSocketConnectable *identity = NULL;
  g_return_val_if_fail(G_IS_TLS_CLIENT_CONNECTION(conn), 0);
  g_object_get(G_OBJECT(conn), "server-identity", &identity, NULL);
  if (identity) g_object_unref(identity);
  return identity;
}
void g_tls_client_connection_set_server_identity(GTlsClientConnection *conn, GSocketConnectable *identity) {
  g_return_if_fail(G_IS_TLS_CLIENT_CONNECTION(conn));
  g_object_set(G_OBJECT(conn), "server-identity", identity, NULL);
}
gboolean g_tls_client_connection_get_use_ssl3(GTlsClientConnection *conn) {
  gboolean use_ssl3 = FALSE;
  g_return_val_if_fail(G_IS_TLS_CLIENT_CONNECTION(conn), 0);
  g_object_get(G_OBJECT(conn), "use-ssl3", &use_ssl3, NULL);
  return use_ssl3;
}
void g_tls_client_connection_set_use_ssl3(GTlsClientConnection *conn, gboolean use_ssl3) {
  g_return_if_fail (G_IS_TLS_CLIENT_CONNECTION(conn));
  g_object_set(G_OBJECT(conn), "use-ssl3", use_ssl3, NULL);
}
GList *g_tls_client_connection_get_accepted_cas(GTlsClientConnection *conn) {
  GList *accepted_cas = NULL;
  g_return_val_if_fail(G_IS_TLS_CLIENT_CONNECTION(conn), NULL);
  g_object_get(G_OBJECT(conn), "accepted-cas", &accepted_cas, NULL);
  return accepted_cas;
}