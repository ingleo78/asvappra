#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gtlsconnection.h"
#include "gcancellable.h"
#include "gioenumtypes.h"
#include "gsocket.h"
#include "gtlsbackend.h"
#include "gtlscertificate.h"
#include "gtlsclientconnection.h"

G_DEFINE_ABSTRACT_TYPE(GTlsConnection, g_tls_connection, G_TYPE_IO_STREAM);
static void g_tls_connection_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void g_tls_connection_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
enum {
  ACCEPT_CERTIFICATE,
  LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };
enum {
  PROP_0,
  PROP_BASE_IO_STREAM,
  PROP_REQUIRE_CLOSE_NOTIFY,
  PROP_REHANDSHAKE_MODE,
  PROP_USE_SYSTEM_CERTDB,
  PROP_CERTIFICATE,
  PROP_PEER_CERTIFICATE,
  PROP_PEER_CERTIFICATE_ERRORS
};
static void g_tls_connection_class_init(GTlsConnectionClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->get_property = g_tls_connection_get_property;
  gobject_class->set_property = g_tls_connection_set_property;
  g_object_class_install_property(gobject_class, PROP_BASE_IO_STREAM, g_param_spec_object("base-io-stream", P_("Base IOStream"), P_("The GIOStream "
								  "that the connection wraps"), G_TYPE_IO_STREAM, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_USE_SYSTEM_CERTDB,g_param_spec_boolean("use-system-certdb", P_("Use system "
								  "certificate database"), P_("Whether to verify peer certificates against the system certificate database"), TRUE,
							G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_REQUIRE_CLOSE_NOTIFY,g_param_spec_boolean("require-close-notify", P_("Require "
								  "close notify"), P_("Whether to require proper TLS close notification"), TRUE,G_PARAM_READWRITE |
							 	  G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_REHANDSHAKE_MODE, g_param_spec_enum("rehandshake-mode", P_("Rehandshake mode"), P_("When to allow"
								  " rehandshaking"), G_TYPE_TLS_REHANDSHAKE_MODE, G_TLS_REHANDSHAKE_SAFELY, G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
						          G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_CERTIFICATE, g_param_spec_object ("certificate", P_("Certificate"), P_("The connection's "
								  "certificate"), G_TYPE_TLS_CERTIFICATE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_PEER_CERTIFICATE, g_param_spec_object ("peer-certificate", P_("Peer Certificate"), P_("The "
								  "connection's peer's certificate"), G_TYPE_TLS_CERTIFICATE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_PEER_CERTIFICATE_ERRORS, g_param_spec_flags ("peer-certificate-errors", P_("Peer Certificate Errors"),
  					       	  P_("Errors found with the peer's certificate"), G_TYPE_TLS_CERTIFICATE_FLAGS, 0, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  signals[ACCEPT_CERTIFICATE] = g_signal_new(I_("accept-certificate"), G_TYPE_TLS_CONNECTION, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GTlsConnectionClass,
		  									 accept_certificate), g_signal_accumulator_true_handled, NULL, NULL,
                                             G_TYPE_BOOLEAN, 2, G_TYPE_TLS_CERTIFICATE, G_TYPE_TLS_CERTIFICATE_FLAGS);
}
static void g_tls_connection_init(GTlsConnection *conn) {}
static void g_tls_connection_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
}
static void g_tls_connection_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
}
void g_tls_connection_set_use_system_certdb(GTlsConnection *conn, gboolean use_system_certdb) {
  g_return_if_fail(G_IS_TLS_CONNECTION(conn));
  g_object_set(G_OBJECT(conn),"use-system-certdb", use_system_certdb, NULL);
}
gboolean g_tls_connection_get_use_system_certdb(GTlsConnection *conn) {
  gboolean use_system_certdb;
  g_return_val_if_fail(G_IS_TLS_CONNECTION(conn), TRUE);
  g_object_get(G_OBJECT(conn),"use-system-certdb", &use_system_certdb, NULL);
  return use_system_certdb;
}
void g_tls_connection_set_certificate(GTlsConnection *conn, GTlsCertificate *certificate) {
  g_return_if_fail(G_IS_TLS_CONNECTION(conn));
  g_return_if_fail(G_IS_TLS_CERTIFICATE(certificate));
  g_object_set(G_OBJECT(conn), "certificate", certificate, NULL);
}
GTlsCertificate *g_tls_connection_get_certificate(GTlsConnection *conn) {
  GTlsCertificate *certificate;
  g_return_val_if_fail(G_IS_TLS_CONNECTION(conn), NULL);
  g_object_get(G_OBJECT(conn), "certificate", &certificate, NULL);
  if (certificate) g_object_unref(certificate);
  return certificate;
}
GTlsCertificate *g_tls_connection_get_peer_certificate(GTlsConnection *conn) {
  GTlsCertificate *peer_certificate;
  g_return_val_if_fail(G_IS_TLS_CONNECTION(conn), NULL);
  g_object_get(G_OBJECT(conn), "peer-certificate", &peer_certificate, NULL);
  if (peer_certificate) g_object_unref(peer_certificate);
  return peer_certificate;
}
GTlsCertificateFlags g_tls_connection_get_peer_certificate_errors(GTlsConnection *conn) {
  GTlsCertificateFlags errors;
  g_return_val_if_fail(G_IS_TLS_CONNECTION(conn), 0);
  g_object_get(G_OBJECT(conn), "peer-certificate-errors", &errors, NULL);
  return errors;
}
void g_tls_connection_set_require_close_notify(GTlsConnection *conn, gboolean require_close_notify) {
  g_return_if_fail(G_IS_TLS_CONNECTION(conn));
  g_object_set(G_OBJECT(conn),"require-close-notify", require_close_notify, NULL);
}
gboolean g_tls_connection_get_require_close_notify(GTlsConnection *conn) {
  gboolean require_close_notify;
  g_return_val_if_fail(G_IS_TLS_CONNECTION(conn), TRUE);
  g_object_get(G_OBJECT(conn),"require-close-notify", &require_close_notify, NULL);
  return require_close_notify;
}
void g_tls_connection_set_rehandshake_mode(GTlsConnection *conn, GTlsRehandshakeMode mode) {
  g_return_if_fail(G_IS_TLS_CONNECTION(conn));
  g_object_set(G_OBJECT(conn),"rehandshake-mode", mode, NULL);
}
GTlsRehandshakeMode g_tls_connection_get_rehandshake_mode(GTlsConnection *conn) {
  GTlsRehandshakeMode mode;
  g_return_val_if_fail(G_IS_TLS_CONNECTION(conn), G_TLS_REHANDSHAKE_NEVER);
  g_object_get(G_OBJECT(conn),"rehandshake-mode", &mode, NULL);
  return mode;
}
gboolean g_tls_connection_handshake(GTlsConnection *conn, GCancellable *cancellable, GError **error) {
  g_return_val_if_fail(G_IS_TLS_CONNECTION(conn), FALSE);
  return G_TLS_CONNECTION_GET_CLASS(conn)->handshake(conn, cancellable, error);
}
void g_tls_connection_handshake_async(GTlsConnection *conn, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  g_return_if_fail(G_IS_TLS_CONNECTION(conn));
  G_TLS_CONNECTION_GET_CLASS(conn)->handshake_async(conn, io_priority, cancellable, callback, user_data);
}
gboolean g_tls_connection_handshake_finish(GTlsConnection *conn, GAsyncResult *result, GError **error) {
  g_return_val_if_fail(G_IS_TLS_CONNECTION(conn), FALSE);
  return G_TLS_CONNECTION_GET_CLASS(conn)->handshake_finish(conn, result, error);
}
GQuark g_tls_error_quark(void) {
  return g_quark_from_static_string("g-tls-error-quark");
}
gboolean g_tls_connection_emit_accept_certificate(GTlsConnection *conn, GTlsCertificate *peer_cert, GTlsCertificateFlags errors) {
  gboolean accept = FALSE;
  g_signal_emit(conn, signals[ACCEPT_CERTIFICATE], 0, peer_cert, errors, &accept);
  return accept;
}