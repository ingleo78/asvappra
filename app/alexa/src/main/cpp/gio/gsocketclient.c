#include <stdlib.h>
#include <string.h>
#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gsocketclient.h"
#include "gioenumtypes.h"
#include "gsocketaddressenumerator.h"
#include "gsocketconnectable.h"
#include "gsocketconnection.h"
#include "gproxyaddressenumerator.h"
#include "gproxyaddress.h"
#include "gsimpleasyncresult.h"
#include "gcancellable.h"
#include "gioerror.h"
#include "gsocket.h"
#include "gnetworkaddress.h"
#include "gnetworkservice.h"
#include "gproxy.h"
#include "gsocketaddress.h"
#include "gtcpconnection.h"
#include "gtcpwrapperconnection.h"
#include "gtlscertificate.h"
#include "gtlsclientconnection.h"

G_DEFINE_TYPE(GSocketClient, g_socket_client, G_TYPE_OBJECT);
enum {
  PROP_NONE,
  PROP_FAMILY,
  PROP_TYPE,
  PROP_PROTOCOL,
  PROP_LOCAL_ADDRESS,
  PROP_TIMEOUT,
  PROP_ENABLE_PROXY,
  PROP_TLS,
  PROP_TLS_VALIDATION_FLAGS
};
struct _GSocketClientPrivate {
  GSocketFamily family;
  GSocketType type;
  GSocketProtocol protocol;
  GSocketAddress *local_address;
  guint timeout;
  gboolean enable_proxy;
  GHashTable *app_proxies;
  gboolean tls;
  GTlsCertificateFlags tls_validation_flags;
};
static GSocket *create_socket(GSocketClient *client, GSocketAddress *dest_address, GError **error) {
  GSocketFamily family;
  GSocket *socket;
  family = client->priv->family;
  if (family == G_SOCKET_FAMILY_INVALID && client->priv->local_address != NULL) family = g_socket_address_get_family(client->priv->local_address);
  if (family == G_SOCKET_FAMILY_INVALID) family = g_socket_address_get_family(dest_address);
  socket = g_socket_new(family, client->priv->type, client->priv->protocol, error);
  if (socket == NULL) return NULL;
  if (client->priv->local_address) {
      if (!g_socket_bind(socket, client->priv->local_address, FALSE, error)) {
	  g_object_unref(socket);
	  return NULL;
	  }
  }
  if (client->priv->timeout) g_socket_set_timeout(socket, client->priv->timeout);
  return socket;
}
gboolean can_use_proxy(GSocketClient *client) {
  GSocketClientPrivate *priv = client->priv;
  return priv->enable_proxy && priv->type == G_SOCKET_TYPE_STREAM;
}
static void g_socket_client_init(GSocketClient *client) {
  client->priv = G_TYPE_INSTANCE_GET_PRIVATE(client, G_TYPE_SOCKET_CLIENT, GSocketClientPrivate);
  client->priv->type = G_SOCKET_TYPE_STREAM;
  client->priv->app_proxies = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
}
GSocketClient *g_socket_client_new(void) {
  return g_object_new(G_TYPE_SOCKET_CLIENT, NULL);
}
static void g_socket_client_finalize(GObject *object) {
  GSocketClient *client = G_SOCKET_CLIENT(object);
  if (client->priv->local_address) g_object_unref(client->priv->local_address);
  if (G_OBJECT_CLASS(g_socket_client_parent_class)->finalize) (*G_OBJECT_CLASS(g_socket_client_parent_class)->finalize)(object);
  g_hash_table_unref(client->priv->app_proxies);
}
static void g_socket_client_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GSocketClient *client = G_SOCKET_CLIENT(object);
  switch(prop_id) {
      case PROP_FAMILY: g_value_set_enum(value, client->priv->family); break;
      case PROP_TYPE: g_value_set_enum(value, client->priv->type); break;
      case PROP_PROTOCOL: g_value_set_enum(value, client->priv->protocol); break;
      case PROP_LOCAL_ADDRESS: g_value_set_object(value, client->priv->local_address); break;
      case PROP_TIMEOUT: g_value_set_uint(value, client->priv->timeout); break;
      case PROP_ENABLE_PROXY: g_value_set_boolean(value, client->priv->enable_proxy); break;
      case PROP_TLS: g_value_set_boolean(value, g_socket_client_get_tls(client)); break;
      case PROP_TLS_VALIDATION_FLAGS: g_value_set_flags(value, g_socket_client_get_tls_validation_flags(client)); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_socket_client_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GSocketClient *client = G_SOCKET_CLIENT(object);
  switch(prop_id) {
      case PROP_FAMILY: g_socket_client_set_family(client, g_value_get_enum(value)); break;
      case PROP_TYPE: g_socket_client_set_socket_type(client, g_value_get_enum(value)); break;
      case PROP_PROTOCOL: g_socket_client_set_protocol(client, g_value_get_enum(value)); break;
      case PROP_LOCAL_ADDRESS: g_socket_client_set_local_address(client, g_value_get_object(value)); break;
      case PROP_TIMEOUT: g_socket_client_set_timeout(client, g_value_get_uint(value)); break;
      case PROP_ENABLE_PROXY: g_socket_client_set_enable_proxy(client, g_value_get_boolean(value)); break;
      case PROP_TLS: g_socket_client_set_tls(client, g_value_get_boolean(value)); break;
      case PROP_TLS_VALIDATION_FLAGS: g_socket_client_set_tls_validation_flags(client, g_value_get_flags(value)); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
GSocketFamily g_socket_client_get_family(GSocketClient *client) {
  return client->priv->family;
}
void g_socket_client_set_family(GSocketClient *client, GSocketFamily family) {
  if (client->priv->family == family) return;
  client->priv->family = family;
  g_object_notify(G_OBJECT(client), "family");
}
GSocketType g_socket_client_get_socket_type(GSocketClient *client) {
  return client->priv->type;
}
void g_socket_client_set_socket_type(GSocketClient *client, GSocketType type) {
  if (client->priv->type == type) return;
  client->priv->type = type;
  g_object_notify(G_OBJECT(client), "type");
}
GSocketProtocol g_socket_client_get_protocol(GSocketClient *client) {
  return client->priv->protocol;
}
void g_socket_client_set_protocol(GSocketClient *client, GSocketProtocol protocol) {
  if (client->priv->protocol == protocol) return;
  client->priv->protocol = protocol;
  g_object_notify(G_OBJECT(client), "protocol");
}
GSocketAddress *g_socket_client_get_local_address(GSocketClient *client) {
  return client->priv->local_address;
}
void g_socket_client_set_local_address(GSocketClient  *client, GSocketAddress *address) {
  if (address) g_object_ref(address);
  if (client->priv->local_address) g_object_unref(client->priv->local_address);
  client->priv->local_address = address;
  g_object_notify(G_OBJECT(client), "local-address");
}
guint g_socket_client_get_timeout(GSocketClient *client) {
  return client->priv->timeout;
}
void g_socket_client_set_timeout(GSocketClient *client, guint timeout) {
  if (client->priv->timeout == timeout) return;
  client->priv->timeout = timeout;
  g_object_notify(G_OBJECT(client), "timeout");
}
gboolean g_socket_client_get_enable_proxy(GSocketClient *client) {
  return client->priv->enable_proxy;
}
void g_socket_client_set_enable_proxy(GSocketClient *client, gboolean enable) {
  enable = !!enable;
  if (client->priv->enable_proxy == enable) return;
  client->priv->enable_proxy = enable;
  g_object_notify(G_OBJECT(client), "enable-proxy");
}
gboolean g_socket_client_get_tls(GSocketClient *client) {
  return client->priv->tls;
}
void g_socket_client_set_tls(GSocketClient *client, gboolean tls) {
  tls = !!tls;
  if (tls == client->priv->tls) return;
  client->priv->tls = tls;
  g_object_notify(G_OBJECT(client), "tls");
}
GTlsCertificateFlags g_socket_client_get_tls_validation_flags(GSocketClient *client) {
  return client->priv->tls_validation_flags;
}
void g_socket_client_set_tls_validation_flags(GSocketClient *client, GTlsCertificateFlags flags) {
  if (client->priv->tls_validation_flags != flags) {
      client->priv->tls_validation_flags = flags;
      g_object_notify(G_OBJECT (client), "tls-validation-flags");
  }
}
static void g_socket_client_class_init(GSocketClientClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  g_type_class_add_private(class, sizeof(GSocketClientPrivate));
  gobject_class->finalize = g_socket_client_finalize;
  gobject_class->set_property = g_socket_client_set_property;
  gobject_class->get_property = g_socket_client_get_property;
  g_object_class_install_property(gobject_class, PROP_FAMILY, g_param_spec_enum("family", P_("Socket family"), P_("The sockets address family to "
                                  "use for socket construction"), G_TYPE_SOCKET_FAMILY, G_SOCKET_FAMILY_INVALID, G_PARAM_CONSTRUCT | G_PARAM_READWRITE |
                                  G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_TYPE, g_param_spec_enum("type", P_("Socket type"), P_("The sockets type to use for socket "
                                  "construction"), G_TYPE_SOCKET_TYPE, G_SOCKET_TYPE_STREAM, G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_PROTOCOL, g_param_spec_enum("protocol", P_("Socket protocol"), P_("The protocol to use for "
                                  "socket construction, or 0 for default"), G_TYPE_SOCKET_PROTOCOL, G_SOCKET_PROTOCOL_DEFAULT, G_PARAM_CONSTRUCT | G_PARAM_READWRITE |
                                  G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_LOCAL_ADDRESS, g_param_spec_object("local-address", P_("Local address"), P_("The local address"
                                  " constructed sockets will be bound to"), G_TYPE_SOCKET_ADDRESS, G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_TIMEOUT,g_param_spec_uint("timeout", P_("Socket timeout"), P_("The I/O"
                                  " timeout for sockets, or 0 for none"),0,G_MAXUINT,0,G_PARAM_CONSTRUCT | G_PARAM_READWRITE |
                                  G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(gobject_class,PROP_ENABLE_PROXY,g_param_spec_boolean("enable-proxy", P_("Enable proxy"),
                                   P_("Enable proxy support"), TRUE,G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_TLS,g_param_spec_boolean("tls", P_("TLS"), P_("Whether to create TLS "
                                  "connections"),FALSE,G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_TLS_VALIDATION_FLAGS, g_param_spec_flags("tls-validation-flags", P_("TLS validation flags"),
						          P_("TLS validation flags to use"), G_TYPE_TLS_CERTIFICATE_FLAGS, G_TLS_CERTIFICATE_VALIDATE_ALL, G_PARAM_CONSTRUCT |
						          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
GSocketConnection *g_socket_client_connect(GSocketClient *client, GSocketConnectable *connectable, GCancellable *cancellable, GError **error) {
  GIOStream *connection = NULL;
  GSocketAddressEnumerator *enumerator = NULL;
  GError *last_error, *tmp_error;
  last_error = NULL;
  if (can_use_proxy (client)) enumerator = g_socket_connectable_proxy_enumerate(connectable);
  else enumerator = g_socket_connectable_enumerate(connectable);
  while(connection == NULL) {
      GSocketAddress *address = NULL;
      GSocket *socket;
      if (g_cancellable_is_cancelled(cancellable)) {
          g_clear_error(error);
          g_cancellable_set_error_if_cancelled(cancellable, error);
          break;
	  }
      tmp_error = NULL;
      address = g_socket_address_enumerator_next(enumerator, cancellable, &tmp_error);
      if (address == NULL) {
          if (tmp_error) {
              g_clear_error(&last_error);
              g_propagate_error(error, tmp_error);
          } else if (last_error) g_propagate_error(error, last_error);
          else g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Unknown error on connect"));
          break;
	  }
      g_clear_error(&last_error);
      socket = create_socket(client, address, &last_error);
      if (socket == NULL) {
          g_object_unref(address);
          continue;
	  }
      if (g_socket_connect(socket, address, cancellable, &last_error)) connection = (GIOStream*)g_socket_connection_factory_create_connection(socket);
      if (connection && G_IS_PROXY_ADDRESS(address) && client->priv->enable_proxy) {
          GProxyAddress *proxy_addr = G_PROXY_ADDRESS(address);
          const gchar *protocol;
          GProxy *proxy;
          protocol = g_proxy_address_get_protocol(proxy_addr);
          proxy = g_proxy_get_default_for_protocol(protocol);
          if (!G_IS_TCP_CONNECTION(connection)) {
              g_critical("Trying to proxy over non-TCP connection, this is most likely a bug in GLib IO library.");
              g_set_error_literal(&last_error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Trying to proxy over non-TCP connection is not supported."));
              g_object_unref(connection);
              connection = NULL;
          } else if (proxy) {
              GIOStream *proxy_connection;
              proxy_connection = g_proxy_connect(proxy, connection, proxy_addr, cancellable, &last_error);
              g_object_unref(connection);
              connection = proxy_connection;
              g_object_unref(proxy);
	      } else if (!g_hash_table_lookup_extended(client->priv->app_proxies, protocol, NULL, NULL)) {
              g_set_error(&last_error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Proxy protocol '%s' is not supported."), protocol);
              g_object_unref(connection);
              connection = NULL;
	      }
	  }
      if (connection && client->priv->tls) {
          GIOStream *tlsconn;
          tlsconn = g_tls_client_connection_new(connection, connectable, &last_error);
          g_object_unref(connection);
          connection = tlsconn;
          if (tlsconn) {
              g_tls_client_connection_set_validation_flags(G_TLS_CLIENT_CONNECTION(tlsconn), client->priv->tls_validation_flags);
              if (!g_tls_connection_handshake(G_TLS_CONNECTION(tlsconn), cancellable, &last_error)) {
                  g_object_unref(tlsconn);
                  connection = NULL;
              }
          }
	  }
      if (connection && !G_IS_SOCKET_CONNECTION(connection)) {
          GSocketConnection *wrapper_connection;
          wrapper_connection = g_tcp_wrapper_connection_new(connection, socket);
          g_object_unref(connection);
          connection = (GIOStream*)wrapper_connection;
	  }
      g_object_unref(socket);
      g_object_unref(address);
  }
  g_object_unref(enumerator);
  return G_SOCKET_CONNECTION(connection);
}
GSocketConnection *g_socket_client_connect_to_host(GSocketClient *client, const gchar *host_and_port, guint16 default_port, GCancellable *cancellable,
				                                   GError **error) {
  GSocketConnectable *connectable;
  GSocketConnection *connection;
  connectable = g_network_address_parse(host_and_port, default_port, error);
  if (connectable == NULL) return NULL;
  connection = g_socket_client_connect(client, connectable, cancellable, error);
  g_object_unref(connectable);
  return connection;
}
GSocketConnection *g_socket_client_connect_to_service(GSocketClient *client, const gchar *domain, const gchar *service, GCancellable *cancellable, GError **error) {
  GSocketConnectable *connectable;
  GSocketConnection *connection;
  connectable = g_network_service_new(service, "tcp", domain);
  connection = g_socket_client_connect(client, connectable, cancellable, error);
  g_object_unref(connectable);
  return connection;
}
GSocketConnection *g_socket_client_connect_to_uri(GSocketClient  *client, const gchar *uri, guint16 default_port, GCancellable *cancellable, GError **error) {
  GSocketConnectable *connectable;
  GSocketConnection *connection;
  connectable = g_network_address_parse_uri(uri, default_port, error);
  if (connectable == NULL) return NULL;
  connection = g_socket_client_connect(client, connectable, cancellable, error);
  g_object_unref(connectable);
  return connection;
}
typedef struct {
  GSimpleAsyncResult *result;
  GCancellable *cancellable;
  GSocketClient *client;
  GSocketConnectable *connectable;
  GSocketAddressEnumerator *enumerator;
  GProxyAddress *proxy_addr;
  GSocket *current_socket;
  GIOStream *connection;
  GError *last_error;
} GSocketClientAsyncConnectData;
static void g_socket_client_async_connect_complete(GSocketClientAsyncConnectData *data) {
  if (data->last_error) g_simple_async_result_take_error(data->result, data->last_error);
  else {
      g_assert(data->connection);
      if (!G_IS_SOCKET_CONNECTION(data->connection)) {
          GSocketConnection *wrapper_connection;
          wrapper_connection = g_tcp_wrapper_connection_new(data->connection, data->current_socket);
          g_object_unref(data->connection);
          data->connection = (GIOStream*)wrapper_connection;
	  }
      g_simple_async_result_set_op_res_gpointer(data->result, data->connection, g_object_unref);
  }
  g_simple_async_result_complete(data->result);
  g_object_unref(data->result);
  g_object_unref(data->connectable);
  g_object_unref(data->enumerator);
  if (data->cancellable) g_object_unref(data->cancellable);
  if (data->current_socket) g_object_unref(data->current_socket);
  if (data->proxy_addr) g_object_unref(data->proxy_addr);
  g_slice_free(GSocketClientAsyncConnectData, data);
}
static void g_socket_client_enumerator_callback(GObject *object, GAsyncResult *result, gpointer user_data);
static void set_last_error(GSocketClientAsyncConnectData *data, GError *error) {
  g_clear_error(&data->last_error);
  data->last_error = error;
}
static void enumerator_next_async(GSocketClientAsyncConnectData *data) {
  g_socket_address_enumerator_next_async(data->enumerator, data->cancellable, g_socket_client_enumerator_callback, data);
}
static void g_socket_client_tls_handshake_callback(GObject *object, GAsyncResult *result, gpointer user_data) {
  GSocketClientAsyncConnectData *data = user_data;
  if (g_tls_connection_handshake_finish(G_TLS_CONNECTION(object), result, &data->last_error)) {
      g_object_unref(data->connection);
      data->connection = G_IO_STREAM(object);
  } else {
      g_object_unref(object);
      g_object_unref(data->current_socket);
      data->current_socket = NULL;
      g_object_unref(data->connection);
      data->connection = NULL;
      enumerator_next_async(data);
  }
  g_socket_client_async_connect_complete(data);
}
static void g_socket_client_tls_handshake(GSocketClientAsyncConnectData *data) {
  GIOStream *tlsconn;
  if (!data->client->priv->tls) {
      g_socket_client_async_connect_complete(data);
      return;
  }
  tlsconn = g_tls_client_connection_new(data->connection, data->connectable, &data->last_error);
  if (tlsconn) {
      g_tls_client_connection_set_validation_flags(G_TLS_CLIENT_CONNECTION(tlsconn), data->client->priv->tls_validation_flags);
      g_tls_connection_handshake_async(G_TLS_CONNECTION(tlsconn), G_PRIORITY_DEFAULT, data->cancellable, g_socket_client_tls_handshake_callback, data);
  } else {
      g_object_unref(data->current_socket);
      data->current_socket = NULL;
      g_object_unref(data->connection);
      data->connection = NULL;
      enumerator_next_async(data);
  }
}
static void g_socket_client_proxy_connect_callback(GObject *object, GAsyncResult *result, gpointer user_data) {
  GSocketClientAsyncConnectData *data = user_data;
  g_object_unref(data->connection);
  data->connection = g_proxy_connect_finish(G_PROXY(object), result, &data->last_error);
  if (!data->connection) {
      g_object_unref(data->current_socket);
      data->current_socket = NULL;
      enumerator_next_async(data);
      return;
  }
  g_socket_client_tls_handshake(data);
}
static void g_socket_client_proxy_connect(GSocketClientAsyncConnectData *data) {
  GProxy *proxy;
  const gchar *protocol;
  if (!data->proxy_addr) {
      g_socket_client_tls_handshake(data);
      return;
  }
  protocol  = g_proxy_address_get_protocol(data->proxy_addr);
  proxy = g_proxy_get_default_for_protocol(protocol);
  if (!G_IS_TCP_CONNECTION(data->connection)) {
      g_critical("Trying to proxy over non-TCP connection, this is most likely a bug in GLib IO library.");
      g_set_error_literal(&data->last_error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Trying to proxy over non-TCP connection is not supported."));
      g_object_unref(data->connection);
      data->connection = NULL;
      g_object_unref(data->current_socket);
      data->current_socket = NULL;
      enumerator_next_async(data);
  } else if (proxy) {
      g_proxy_connect_async(proxy, data->connection, data->proxy_addr, data->cancellable, g_socket_client_proxy_connect_callback, data);
      g_object_unref(proxy);
  } else if (!g_hash_table_lookup_extended(data->client->priv->app_proxies, protocol, NULL, NULL)) {
      g_clear_error(&data->last_error);
      g_set_error(&data->last_error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Proxy protocol '%s' is not supported."), protocol);
      g_object_unref(data->current_socket);
      data->current_socket = NULL;
      g_object_unref(data->connection);
      data->connection = NULL;
      g_object_unref(data->current_socket);
      data->current_socket = NULL;
      enumerator_next_async(data);
  }
}
static void g_socket_client_socket_connected(GSocketClientAsyncConnectData *data) {
  g_socket_set_blocking(data->current_socket, TRUE);
  data->connection = (GIOStream*)g_socket_connection_factory_create_connection(data->current_socket);
  g_socket_client_proxy_connect(data);
}
static gboolean g_socket_client_socket_callback(GSocket *socket, GIOCondition condition, GSocketClientAsyncConnectData *data) {
  GError *error = NULL;
  if (g_cancellable_is_cancelled(data->cancellable)) {
      g_clear_error(&data->last_error);
      g_object_unref(data->current_socket);
      data->current_socket = NULL;
      g_cancellable_set_error_if_cancelled(data->cancellable, &data->last_error);
      g_socket_client_async_connect_complete(data);
      return FALSE;
  } else {
      if (!g_socket_check_connect_result(data->current_socket, &error)) {
          set_last_error(data, error);
          g_object_unref(data->current_socket);
          data->current_socket = NULL;
          enumerator_next_async(data);
          return FALSE;
      }
  }
  g_socket_client_socket_connected(data);
  return FALSE;
}
static void g_socket_client_enumerator_callback(GObject *object, GAsyncResult *result, gpointer user_data) {
  GSocketClientAsyncConnectData *data = user_data;
  GSocketAddress *address = NULL;
  GSocket *socket;
  GError *tmp_error = NULL;
  if (g_cancellable_is_cancelled(data->cancellable)) {
      g_clear_error(&data->last_error);
      g_cancellable_set_error_if_cancelled(data->cancellable, &data->last_error);
      g_socket_client_async_connect_complete(data);
      return;
  }
  address = g_socket_address_enumerator_next_finish(data->enumerator, result, &tmp_error);
  if (address == NULL) {
      if (tmp_error) set_last_error(data, tmp_error);
      else if (data->last_error == NULL) g_set_error_literal(&data->last_error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Unknown error on connect"));
      g_socket_client_async_connect_complete(data);
      return;
  }
  if (G_IS_PROXY_ADDRESS(address) && data->client->priv->enable_proxy) data->proxy_addr = g_object_ref(G_PROXY_ADDRESS(address));
  g_clear_error(&data->last_error);
  socket = create_socket(data->client, address, &data->last_error);
  if (socket != NULL) {
      g_socket_set_blocking(socket, FALSE);
      if (g_socket_connect(socket, address, data->cancellable, &tmp_error)) {
          data->current_socket = socket;
          g_socket_client_socket_connected(data);
          g_object_unref(address);
          return;
	  } else if (g_error_matches(tmp_error, G_IO_ERROR, G_IO_ERROR_PENDING)) {
          GSource *source;
          data->current_socket = socket;
          g_error_free(tmp_error);
          source = g_socket_create_source(socket, G_IO_OUT, data->cancellable);
          g_source_set_callback(source, (GSourceFunc) g_socket_client_socket_callback, data, NULL);
          g_source_attach(source, g_main_context_get_thread_default());
          g_source_unref(source);
          g_object_unref(address);
          return;
	  } else {
          data->last_error = tmp_error;
          g_object_unref(socket);
	  }
  }
  g_object_unref(address);
  enumerator_next_async(data);
}
void g_socket_client_connect_async(GSocketClient *client, GSocketConnectable *connectable, GCancellable *cancellable, GAsyncReadyCallback callback,
			                       gpointer user_data) {
  GSocketClientAsyncConnectData *data;
  g_return_if_fail(G_IS_SOCKET_CLIENT(client));
  data = g_slice_new0(GSocketClientAsyncConnectData);
  data->result = g_simple_async_result_new(G_OBJECT(client), callback, user_data, g_socket_client_connect_async);
  data->client = client;
  if (cancellable) data->cancellable = g_object_ref(cancellable);
  else data->cancellable = NULL;
  data->last_error = NULL;
  data->connectable = g_object_ref(connectable);
  if (can_use_proxy(client)) data->enumerator = g_socket_connectable_proxy_enumerate(connectable);
  else data->enumerator = g_socket_connectable_enumerate(connectable);
  enumerator_next_async(data);
}
void g_socket_client_connect_to_host_async(GSocketClient *client, const gchar *host_and_port, guint16 default_port, GCancellable *cancellable,
				                           GAsyncReadyCallback callback, gpointer user_data) {
  GSocketConnectable *connectable;
  GError *error;
  error = NULL;
  connectable = g_network_address_parse(host_and_port, default_port, &error);
  if (connectable == NULL) g_simple_async_report_take_gerror_in_idle(G_OBJECT(client), callback, user_data, error);
  else {
      g_socket_client_connect_async(client, connectable, cancellable, callback, user_data);
      g_object_unref(connectable);
  }
}
void g_socket_client_connect_to_service_async(GSocketClient *client, const gchar *domain, const gchar *service, GCancellable *cancellable, GAsyncReadyCallback callback,
					                          gpointer user_data) {
  GSocketConnectable *connectable;
  connectable = g_network_service_new(service, "tcp", domain);
  g_socket_client_connect_async(client, connectable, cancellable, callback, user_data);
  g_object_unref(connectable);
}
void g_socket_client_connect_to_uri_async(GSocketClient *client, const gchar *uri, guint16 default_port, GCancellable *cancellable, GAsyncReadyCallback callback,
				                          gpointer user_data) {
  GSocketConnectable *connectable;
  GError *error;
  error = NULL;
  connectable = g_network_address_parse_uri(uri, default_port, &error);
  if (connectable == NULL) g_simple_async_report_take_gerror_in_idle(G_OBJECT(client), callback, user_data, error);
  else {
      g_socket_client_connect_async(client, connectable, cancellable, callback, user_data);
      g_object_unref(connectable);
  }
}
GSocketConnection *g_socket_client_connect_finish(GSocketClient *client, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
  if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  return g_object_ref(g_simple_async_result_get_op_res_gpointer(simple));
}
GSocketConnection *g_socket_client_connect_to_host_finish(GSocketClient *client, GAsyncResult *result, GError **error) {
  return g_socket_client_connect_finish(client, result, error);
}
GSocketConnection *g_socket_client_connect_to_service_finish(GSocketClient *client, GAsyncResult *result, GError **error) {
  return g_socket_client_connect_finish(client, result, error);
}
GSocketConnection *g_socket_client_connect_to_uri_finish(GSocketClient *client, GAsyncResult *result, GError **error) {
  return g_socket_client_connect_finish(client, result, error);
}
void g_socket_client_add_application_proxy(GSocketClient *client, const gchar *protocol) {
  g_hash_table_insert(client->priv->app_proxies, g_strdup(protocol), NULL);
}