#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gtlsserverconnection.h"
#include "ginitable.h"
#include "gioenumtypes.h"
#include "gsocket.h"
#include "gtlsbackend.h"
#include "gtlscertificate.h"

G_DEFINE_INTERFACE(GTlsServerConnection, g_tls_server_connection, G_TYPE_TLS_CONNECTION);
static void g_tls_server_connection_default_init(GTlsServerConnectionInterface *iface) {
  g_object_interface_install_property(iface, g_param_spec_enum("authentication-mode", P_("Authentication Mode"), P_("The client authentication mode"),
							  		  G_TYPE_TLS_AUTHENTICATION_MODE, G_TLS_AUTHENTICATION_NONE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
GIOStream *g_tls_server_connection_new(GIOStream *base_io_stream, GTlsCertificate *certificate, GError **error) {
  GObject *conn;
  GTlsBackend *backend;
  backend = g_tls_backend_get_default();
  conn = g_initable_new(g_tls_backend_get_server_connection_type(backend),NULL, error,"base-io-stream", base_io_stream, "certificate",
			 			certificate, NULL);
  return G_IO_STREAM(conn);
}