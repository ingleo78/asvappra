#include "../glib/glibintl.h"
#include "config.h"
#include "gtcpwrapperconnection.h"
#include "gtcpconnection.h"

G_DEFINE_TYPE(GTcpWrapperConnection, g_tcp_wrapper_connection, G_TYPE_TCP_CONNECTION);
enum {
  PROP_NONE,
  PROP_BASE_IO_STREAM
};
struct _GTcpWrapperConnectionPrivate {
  GIOStream *base_io_stream;
};
static GInputStream *g_tcp_wrapper_connection_get_input_stream(GIOStream *io_stream) {
  GTcpWrapperConnection *connection = G_TCP_WRAPPER_CONNECTION(io_stream);
  return g_io_stream_get_input_stream(connection->priv->base_io_stream);
}
static GOutputStream *g_tcp_wrapper_connection_get_output_stream(GIOStream *io_stream) {
  GTcpWrapperConnection *connection = G_TCP_WRAPPER_CONNECTION(io_stream);
  return g_io_stream_get_output_stream(connection->priv->base_io_stream);
}
static void g_tcp_wrapper_connection_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GTcpWrapperConnection *connection = G_TCP_WRAPPER_CONNECTION(object);
  switch(prop_id) {
     case PROP_BASE_IO_STREAM: g_value_set_object(value, connection->priv->base_io_stream); break;
     default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_tcp_wrapper_connection_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GTcpWrapperConnection *connection = G_TCP_WRAPPER_CONNECTION(object);
  switch(prop_id) {
      case PROP_BASE_IO_STREAM: connection->priv->base_io_stream = g_value_dup_object(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_tcp_wrapper_connection_finalize(GObject *object) {
  GTcpWrapperConnection *connection = G_TCP_WRAPPER_CONNECTION(object);
  if (connection->priv->base_io_stream) g_object_unref(connection->priv->base_io_stream);
  G_OBJECT_CLASS(g_tcp_wrapper_connection_parent_class)->finalize(object);
}
static void g_tcp_wrapper_connection_class_init(GTcpWrapperConnectionClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GIOStreamClass *stream_class = G_IO_STREAM_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GTcpWrapperConnectionPrivate));
  gobject_class->set_property = g_tcp_wrapper_connection_set_property;
  gobject_class->get_property = g_tcp_wrapper_connection_get_property;
  gobject_class->finalize = g_tcp_wrapper_connection_finalize;
  stream_class->get_input_stream = g_tcp_wrapper_connection_get_input_stream;
  stream_class->get_output_stream = g_tcp_wrapper_connection_get_output_stream;
  g_object_class_install_property(gobject_class,PROP_BASE_IO_STREAM,g_param_spec_object("base-io-stream", P_("Base IO Stream"),
			                      P_("The wrapped GIOStream"), G_TYPE_IO_STREAM,G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
static void g_tcp_wrapper_connection_init(GTcpWrapperConnection *connection) {
  connection->priv = G_TYPE_INSTANCE_GET_PRIVATE(connection, G_TYPE_TCP_WRAPPER_CONNECTION, GTcpWrapperConnectionPrivate);
}
GSocketConnection *g_tcp_wrapper_connection_new(GIOStream *base_io_stream, GSocket *socket) {
  g_return_val_if_fail(G_IS_IO_STREAM(base_io_stream), NULL);
  g_return_val_if_fail(G_IS_SOCKET(socket), NULL);
  g_return_val_if_fail(g_socket_get_family(socket) == G_SOCKET_FAMILY_IPV4 || g_socket_get_family(socket) == G_SOCKET_FAMILY_IPV6, NULL);
  g_return_val_if_fail(g_socket_get_socket_type(socket) == G_SOCKET_TYPE_STREAM, NULL);
  return g_object_new(G_TYPE_TCP_WRAPPER_CONNECTION,"base-io-stream", base_io_stream, "socket", socket, NULL);
}
GIOStream *g_tcp_wrapper_connection_get_base_io_stream(GTcpWrapperConnection *conn) {
  g_return_val_if_fail(G_IS_TCP_WRAPPER_CONNECTION(conn), NULL);
  return conn->priv->base_io_stream;
}