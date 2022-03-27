#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gsocketconnection.h"
#include "gsocketoutputstream.h"
#include "gsocketinputstream.h"
#include "giostream.h"
#include "gsimpleasyncresult.h"
#include "gunixconnection.h"
#include "gtcpconnection.h"

G_DEFINE_TYPE(GSocketConnection, g_socket_connection, G_TYPE_IO_STREAM);
enum {
  PROP_NONE,
  PROP_SOCKET,
};
struct _GSocketConnectionPrivate {
  GSocket *socket;
  GInputStream *input_stream;
  GOutputStream *output_stream;
  gboolean in_dispose;
};
static gboolean g_socket_connection_close(GIOStream *stream, GCancellable *cancellable, GError **error);
static void g_socket_connection_close_async(GIOStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
static gboolean g_socket_connection_close_finish(GIOStream *stream, GAsyncResult *result, GError **error);
static GInputStream *g_socket_connection_get_input_stream(GIOStream *io_stream) {
  GSocketConnection *connection = G_SOCKET_CONNECTION(io_stream);
  if (connection->priv->input_stream == NULL) connection->priv->input_stream = (GInputStream*)_g_socket_input_stream_new(connection->priv->socket);
  return connection->priv->input_stream;
}
static GOutputStream *g_socket_connection_get_output_stream(GIOStream *io_stream) {
  GSocketConnection *connection = G_SOCKET_CONNECTION(io_stream);
  if (connection->priv->output_stream == NULL) connection->priv->output_stream = (GOutputStream*)_g_socket_output_stream_new(connection->priv->socket);
  return connection->priv->output_stream;
}
GSocket *g_socket_connection_get_socket(GSocketConnection *connection) {
  g_return_val_if_fail(G_IS_SOCKET_CONNECTION(connection), NULL);
  return connection->priv->socket;
}
GSocketAddress *g_socket_connection_get_local_address(GSocketConnection *connection, GError **error) {
  return g_socket_get_local_address(connection->priv->socket, error);
}
GSocketAddress *g_socket_connection_get_remote_address(GSocketConnection *connection, GError **error) {
  return g_socket_get_remote_address(connection->priv->socket, error);
}
static void g_socket_connection_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GSocketConnection *connection = G_SOCKET_CONNECTION(object);
  switch(prop_id) {
     case PROP_SOCKET: g_value_set_object(value, connection->priv->socket); break;
     default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_socket_connection_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GSocketConnection *connection = G_SOCKET_CONNECTION(object);
  switch(prop_id) {
       case PROP_SOCKET: connection->priv->socket = G_SOCKET(g_value_dup_object(value)); break;
       default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_socket_connection_constructed(GObject *object) {
  GSocketConnection *connection = G_SOCKET_CONNECTION(object);
  g_assert(connection->priv->socket != NULL);
}
static void g_socket_connection_dispose(GObject *object) {
  GSocketConnection *connection = G_SOCKET_CONNECTION(object);
  connection->priv->in_dispose = TRUE;
  G_OBJECT_CLASS(g_socket_connection_parent_class)->dispose(object);
  connection->priv->in_dispose = FALSE;
}
static void g_socket_connection_finalize(GObject *object) {
  GSocketConnection *connection = G_SOCKET_CONNECTION(object);
  if (connection->priv->input_stream) g_object_unref(connection->priv->input_stream);
  if (connection->priv->output_stream) g_object_unref(connection->priv->output_stream);
  g_object_unref(connection->priv->socket);
  G_OBJECT_CLASS(g_socket_connection_parent_class)->finalize(object);
}
static void g_socket_connection_class_init(GSocketConnectionClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GIOStreamClass *stream_class = G_IO_STREAM_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GSocketConnectionPrivate));
  gobject_class->set_property = g_socket_connection_set_property;
  gobject_class->get_property = g_socket_connection_get_property;
  gobject_class->constructed = g_socket_connection_constructed;
  gobject_class->finalize = g_socket_connection_finalize;
  gobject_class->dispose = g_socket_connection_dispose;
  stream_class->get_input_stream = g_socket_connection_get_input_stream;
  stream_class->get_output_stream = g_socket_connection_get_output_stream;
  stream_class->close_fn = g_socket_connection_close;
  stream_class->close_async = g_socket_connection_close_async;
  stream_class->close_finish = g_socket_connection_close_finish;
  g_object_class_install_property(gobject_class,PROP_SOCKET,g_param_spec_object ("socket", P_("Socket"), P_("The underlying GSocket"),
                                  G_TYPE_SOCKET,G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
static void g_socket_connection_init(GSocketConnection *connection) {
  connection->priv = G_TYPE_INSTANCE_GET_PRIVATE(connection, G_TYPE_SOCKET_CONNECTION, GSocketConnectionPrivate);
}
static gboolean g_socket_connection_close(GIOStream *stream, GCancellable *cancellable, GError **error) {
  GSocketConnection *connection = G_SOCKET_CONNECTION(stream);
  if (connection->priv->output_stream) g_output_stream_close(connection->priv->output_stream, cancellable, NULL);
  if (connection->priv->input_stream) g_input_stream_close(connection->priv->input_stream, cancellable, NULL);
  if (connection->priv->in_dispose) return TRUE;
  return g_socket_close(connection->priv->socket, error);
}
static void g_socket_connection_close_async(GIOStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  GIOStreamClass *class;
  GError *error;
  class = G_IO_STREAM_GET_CLASS(stream);
  error = NULL;
  if (class->close_fn && !class->close_fn(stream, cancellable, &error)) {
      g_simple_async_report_take_gerror_in_idle(G_OBJECT(stream), callback, user_data, error);
      return;
  }
  res = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_socket_connection_close_async);
  g_simple_async_result_complete_in_idle(res);
  g_object_unref(res);
}
static gboolean g_socket_connection_close_finish(GIOStream *stream, GAsyncResult *result, GError **error) {
  return TRUE;
}
typedef struct {
  GSocketFamily socket_family;
  GSocketType socket_type;
  int protocol;
  GType implementation;
} ConnectionFactory;
static guint connection_factory_hash(gconstpointer key) {
  const ConnectionFactory *factory = key;
  guint h;
  h = factory->socket_family ^ (factory->socket_type << 4) ^ (factory->protocol << 8);
  h = h ^ (h << 8) ^ (h << 16) ^ (h << 24);
  return h;
}
static gboolean connection_factory_equal(gconstpointer _a, gconstpointer _b) {
  const ConnectionFactory *a = _a;
  const ConnectionFactory *b = _b;
  if (a->socket_family != b->socket_family) return FALSE;
  if (a->socket_type != b->socket_type) return FALSE;
  if (a->protocol != b->protocol) return FALSE;
  return TRUE;
}
static GHashTable *connection_factories = NULL;
G_LOCK_DEFINE_STATIC(connection_factories);
void g_socket_connection_factory_register_type(GType g_type, GSocketFamily family, GSocketType type, gint protocol) {
  ConnectionFactory *factory;
  g_return_if_fail(g_type_is_a (g_type, G_TYPE_SOCKET_CONNECTION));
  G_LOCK(connection_factories);
  if (connection_factories == NULL) connection_factories = g_hash_table_new_full(connection_factory_hash, connection_factory_equal, (GDestroyNotify)g_free, NULL);
  factory = g_new0(ConnectionFactory, 1);
  factory->socket_family = family;
  factory->socket_type = type;
  factory->protocol = protocol;
  factory->implementation = g_type;
  g_hash_table_insert(connection_factories, factory, factory);
  G_UNLOCK (connection_factories);
}
static void init_builtin_types(void) {
  volatile GType a_type;
#ifdef G_OS_WIN32
  a_type = g_unix_connection_get_type();
#endif
  a_type = g_tcp_connection_get_type();
}
GType g_socket_connection_factory_lookup_type(GSocketFamily family, GSocketType type, gint protocol_id) {
  ConnectionFactory *factory, key;
  GType g_type;
  init_builtin_types();
  G_LOCK(connection_factories);
  g_type = G_TYPE_SOCKET_CONNECTION;
  if (connection_factories) {
      key.socket_family = family;
      key.socket_type = type;
      key.protocol = protocol_id;
      factory = g_hash_table_lookup(connection_factories, &key);
      if (factory) g_type = factory->implementation;
  }
  G_UNLOCK(connection_factories);
  return g_type;
}
GSocketConnection *g_socket_connection_factory_create_connection(GSocket *socket) {
  GType type;
  type = g_socket_connection_factory_lookup_type(g_socket_get_family(socket), g_socket_get_socket_type(socket), g_socket_get_protocol(socket));
  return g_object_new(type, "socket", socket, NULL);
}