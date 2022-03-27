#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gsocketlistener.h"
#include "gsimpleasyncresult.h"
#include "gcancellable.h"
#include "gsocketaddress.h"
#include "ginetaddress.h"
#include "gioerror.h"
#include "gsocket.h"
#include "gsocketconnection.h"
#include "ginetsocketaddress.h"

G_DEFINE_TYPE (GSocketListener, g_socket_listener, G_TYPE_OBJECT);
enum {
  PROP_0,
  PROP_LISTEN_BACKLOG
};
static GQuark source_quark = 0;
struct _GSocketListenerPrivate {
  GPtrArray *sockets;
  GMainContext *main_context;
  int listen_backlog;
  guint closed : 1;
};
static void g_socket_listener_finalize(GObject *object) {
  GSocketListener *listener = G_SOCKET_LISTENER(object);
  if (listener->priv->main_context) g_main_context_unref(listener->priv->main_context);
  if (!listener->priv->closed) g_socket_listener_close(listener);
  g_ptr_array_free(listener->priv->sockets, TRUE);
  G_OBJECT_CLASS(g_socket_listener_parent_class)->finalize(object);
}
static void g_socket_listener_get_property(GObject *object,guint prop_id, GValue *value, GParamSpec *pspec) {
  GSocketListener *listener = G_SOCKET_LISTENER(object);
  switch(prop_id) {
      case PROP_LISTEN_BACKLOG: g_value_set_int(value, listener->priv->listen_backlog); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_socket_listener_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GSocketListener *listener = G_SOCKET_LISTENER(object);
  switch(prop_id) {
      case PROP_LISTEN_BACKLOG: g_socket_listener_set_backlog(listener, g_value_get_int(value)); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_socket_listener_class_init(GSocketListenerClass *klass) {
  GObjectClass *gobject_class G_GNUC_UNUSED = G_OBJECT_CLASS(klass);
  g_type_class_add_private (klass, sizeof(GSocketListenerPrivate));
  gobject_class->finalize = g_socket_listener_finalize;
  gobject_class->set_property = g_socket_listener_set_property;
  gobject_class->get_property = g_socket_listener_get_property;
  g_object_class_install_property(gobject_class, PROP_LISTEN_BACKLOG,g_param_spec_int("listen-backlog", P_("Listen backlog"),
                                  P_("outstanding connections in the listen queue"),0,2000,10,G_PARAM_CONSTRUCT |
                                  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  source_quark = g_quark_from_static_string("g-socket-listener-source");
}
static void g_socket_listener_init(GSocketListener *listener) {
  listener->priv = G_TYPE_INSTANCE_GET_PRIVATE(listener, G_TYPE_SOCKET_LISTENER, GSocketListenerPrivate);
  listener->priv->sockets = g_ptr_array_new_with_free_func((GDestroyNotify)g_object_unref);
  listener->priv->listen_backlog = 10;
}
GSocketListener *g_socket_listener_new(void) {
  return g_object_new(G_TYPE_SOCKET_LISTENER, NULL);
}
static gboolean check_listener(GSocketListener *listener, GError **error) {
  if (listener->priv->closed) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_CLOSED, _("Listener is already closed"));
      return FALSE;
  }
  return TRUE;
}
gboolean g_socket_listener_add_socket(GSocketListener *listener, GSocket *socket, GObject *source_object, GError **error) {
  if (!check_listener(listener, error)) return FALSE;
  if (g_socket_is_closed(socket)) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Added socket is closed"));
      return FALSE;
  }
  g_object_ref(socket);
  g_ptr_array_add(listener->priv->sockets, socket);
  if (source_object) g_object_set_qdata_full(G_OBJECT(socket), source_quark, g_object_ref(source_object), g_object_unref);
  if (G_SOCKET_LISTENER_GET_CLASS(listener)->changed) G_SOCKET_LISTENER_GET_CLASS(listener)->changed (listener);
  return TRUE;
}
gboolean g_socket_listener_add_address(GSocketListener *listener, GSocketAddress *address, GSocketType type, GSocketProtocol protocol, GObject *source_object,
                                       GSocketAddress **effective_address, GError **error) {
  GSocketAddress *local_address;
  GSocketFamily family;
  GSocket *socket;
  if (!check_listener(listener, error)) return FALSE;
  family = g_socket_address_get_family(address);
  socket = g_socket_new(family, type, protocol, error);
  if (socket == NULL) return FALSE;
  g_socket_set_listen_backlog(socket, listener->priv->listen_backlog);
  if (!g_socket_bind(socket, address, TRUE, error) || !g_socket_listen(socket, error)) {
      g_object_unref(socket);
      return FALSE;
  }
  local_address = NULL;
  if (effective_address) {
      local_address = g_socket_get_local_address(socket, error);
      if (local_address == NULL) {
	  g_object_unref(socket);
	  return FALSE;
	  }
  }
  if (!g_socket_listener_add_socket(listener, socket, source_object, error)) {
      if (local_address) g_object_unref(local_address);
      g_object_unref(socket);
      return FALSE;
  }
  if (effective_address) *effective_address = local_address;
  g_object_unref(socket);
  return TRUE;
}
gboolean g_socket_listener_add_inet_port(GSocketListener *listener, guint16 port, GObject *source_object, GError **error) {
  gboolean need_ipv4_socket = TRUE;
  GSocket *socket4 = NULL;
  GSocket *socket6;
  g_return_val_if_fail(listener != NULL, FALSE);
  g_return_val_if_fail(port != 0, FALSE);
  if (!check_listener(listener, error)) return FALSE;
  socket6 = g_socket_new(G_SOCKET_FAMILY_IPV6,G_SOCKET_TYPE_STREAM,G_SOCKET_PROTOCOL_DEFAULT,NULL);
  if (socket6 != NULL) {
      GInetAddress *inet_address;
      GSocketAddress *address;
      gboolean result;
      inet_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV6);
      address = g_inet_socket_address_new(inet_address, port);
      g_object_unref(inet_address);
      g_socket_set_listen_backlog(socket6, listener->priv->listen_backlog);
      result = g_socket_bind(socket6, address, TRUE, error) && g_socket_listen(socket6, error);
      g_object_unref(address);
      if (!result) {
          g_object_unref(socket6);
          return FALSE;
      }
      if (source_object) g_object_set_qdata_full(G_OBJECT(socket6), source_quark, g_object_ref(source_object), g_object_unref);
      if (g_socket_speaks_ipv4(socket6)) need_ipv4_socket = FALSE;
  }
  if (need_ipv4_socket) {
      socket4 = g_socket_new(G_SOCKET_FAMILY_IPV4,G_SOCKET_TYPE_STREAM,G_SOCKET_PROTOCOL_DEFAULT, error);
      if (socket4 != NULL) {
          GInetAddress *inet_address;
          GSocketAddress *address;
          gboolean result;
          inet_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
          address = g_inet_socket_address_new(inet_address, port);
          g_object_unref(inet_address);
          g_socket_set_listen_backlog(socket4, listener->priv->listen_backlog);
          result = g_socket_bind(socket4, address, TRUE, error) && g_socket_listen(socket4, error);
          g_object_unref(address);
          if (!result) {
              g_object_unref(socket4);
              if (socket6 != NULL) g_object_unref(socket6);
              return FALSE;
          }
          if (source_object) g_object_set_qdata_full(G_OBJECT(socket4), source_quark, g_object_ref(source_object), g_object_unref);
      } else {
          if (socket6 != NULL) g_clear_error(error);
          else return FALSE;
      }
  }
  g_assert(socket6 != NULL || socket4 != NULL);
  if (socket6 != NULL) g_ptr_array_add(listener->priv->sockets, socket6);
  if (socket4 != NULL) g_ptr_array_add(listener->priv->sockets, socket4);
  if (G_SOCKET_LISTENER_GET_CLASS(listener)->changed) G_SOCKET_LISTENER_GET_CLASS(listener)->changed(listener);
  return TRUE;
}
static GList *add_sources(GSocketListener *listener, GSocketSourceFunc callback, gpointer callback_data, GCancellable *cancellable, GMainContext *context) {
  GSocket *socket;
  GSource *source;
  GList *sources;
  int i;
  sources = NULL;
  for (i = 0; i < listener->priv->sockets->len; i++) {
      socket = listener->priv->sockets->pdata[i];
      source = g_socket_create_source(socket, G_IO_IN, cancellable);
      g_source_set_callback(source, (GSourceFunc)callback, callback_data, NULL);
      g_source_attach(source, context);
      sources = g_list_prepend(sources, source);
  }
  return sources;
}
static void free_sources(GList *sources) {
  GSource *source;
  while(sources != NULL) {
      source = sources->data;
      sources = g_list_delete_link(sources, sources);
      g_source_destroy(source);
      g_source_unref(source);
  }
}
struct AcceptData {
  GMainLoop *loop;
  GSocket *socket;
};
static gboolean accept_callback(GSocket *socket, GIOCondition condition, gpointer user_data) {
  struct AcceptData *data = user_data;
  data->socket = socket;
  g_main_loop_quit(data->loop);
  return TRUE;
}
GSocket *g_socket_listener_accept_socket(GSocketListener *listener, GObject **source_object, GCancellable *cancellable, GError **error) {
  GSocket *accept_socket, *socket;
  g_return_val_if_fail(G_IS_SOCKET_LISTENER(listener), NULL);
  if (!check_listener(listener, error)) return NULL;
  if (listener->priv->sockets->len == 1) {
      accept_socket = listener->priv->sockets->pdata[0];
      if (!g_socket_condition_wait(accept_socket, G_IO_IN, cancellable, error)) return NULL;
  } else {
      GList *sources;
      struct AcceptData data;
      GMainLoop *loop;
      if (listener->priv->main_context == NULL) listener->priv->main_context = g_main_context_new();
      loop = g_main_loop_new(listener->priv->main_context, FALSE);
      data.loop = loop;
      sources = add_sources(listener, accept_callback, &data, cancellable, listener->priv->main_context);
      g_main_loop_run(loop);
      accept_socket = data.socket;
      free_sources(sources);
      g_main_loop_unref(loop);
  }
  if (!(socket = g_socket_accept(accept_socket, cancellable, error))) return NULL;
  if (source_object) *source_object = g_object_get_qdata(G_OBJECT (accept_socket), source_quark);
  return socket;
}
GSocketConnection *g_socket_listener_accept(GSocketListener *listener, GObject **source_object, GCancellable *cancellable, GError **error) {
  GSocketConnection *connection;
  GSocket *socket;
  socket = g_socket_listener_accept_socket(listener, source_object, cancellable, error);
  if (socket == NULL) return NULL;
  connection = g_socket_connection_factory_create_connection(socket);
  g_object_unref(socket);
  return connection;
}
struct AcceptAsyncData {
  GSimpleAsyncResult *simple;
  GCancellable *cancellable;
  GList *sources;
};
static gboolean accept_ready(GSocket *accept_socket, GIOCondition condition, gpointer _data) {
  struct AcceptAsyncData *data = _data;
  GError *error = NULL;
  GSocket *socket;
  GObject *source_object;
  socket = g_socket_accept(accept_socket, data->cancellable, &error);
  if (socket) {
      g_simple_async_result_set_op_res_gpointer(data->simple, socket, g_object_unref);
      source_object = g_object_get_qdata(G_OBJECT(accept_socket), source_quark);
      if (source_object) g_object_set_qdata_full(G_OBJECT(data->simple), source_quark, g_object_ref(source_object), g_object_unref);
  } else g_simple_async_result_take_error(data->simple, error);
  g_simple_async_result_complete_in_idle(data->simple);
  g_object_unref(data->simple);
  free_sources(data->sources);
  g_free(data);
  return FALSE;
}
void g_socket_listener_accept_socket_async(GSocketListener *listener, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  struct AcceptAsyncData *data;
  GError *error = NULL;
  if (!check_listener(listener, &error)) {
      g_simple_async_report_take_gerror_in_idle(G_OBJECT(listener), callback, user_data, error);
      return;
  }
  data = g_new0(struct AcceptAsyncData, 1);
  data->simple = g_simple_async_result_new(G_OBJECT(listener), callback, user_data, g_socket_listener_accept_socket_async);
  data->cancellable = cancellable;
  data->sources = add_sources(listener, accept_ready, data, cancellable, g_main_context_get_thread_default());
}
GSocket *g_socket_listener_accept_socket_finish(GSocketListener *listener, GAsyncResult *result, GObject **source_object, GError **error) {
  GSocket *socket;
  GSimpleAsyncResult *simple;
  g_return_val_if_fail(G_IS_SOCKET_LISTENER(listener), NULL);
  simple = G_SIMPLE_ASYNC_RESULT(result);
  if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_socket_listener_accept_socket_async);
  socket = g_simple_async_result_get_op_res_gpointer(simple);
  if (source_object) *source_object = g_object_get_qdata(G_OBJECT(result), source_quark);
  return g_object_ref(socket);
}
void g_socket_listener_accept_async(GSocketListener *listener, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  g_socket_listener_accept_socket_async(listener, cancellable, callback, user_data);
}
GSocketConnection *g_socket_listener_accept_finish(GSocketListener *listener, GAsyncResult *result, GObject **source_object, GError **error) {
  GSocket *socket;
  GSocketConnection *connection;
  socket = g_socket_listener_accept_socket_finish(listener, result, source_object, error);
  if (socket == NULL) return NULL;
  connection = g_socket_connection_factory_create_connection(socket);
  g_object_unref(socket);
  return connection;
}
void g_socket_listener_set_backlog(GSocketListener *listener, int listen_backlog) {
  GSocket *socket;
  int i;
  if (listener->priv->closed) return;
  listener->priv->listen_backlog = listen_backlog;
  for (i = 0; i < listener->priv->sockets->len; i++) {
      socket = listener->priv->sockets->pdata[i];
      g_socket_set_listen_backlog(socket, listen_backlog);
  }
}
void g_socket_listener_close(GSocketListener *listener) {
  GSocket *socket;
  int i;
  g_return_if_fail(G_IS_SOCKET_LISTENER(listener));
  if (listener->priv->closed) return;
  for (i = 0; i < listener->priv->sockets->len; i++) {
      socket = listener->priv->sockets->pdata[i];
      g_socket_close(socket, NULL);
  }
  listener->priv->closed = TRUE;
}
guint16 g_socket_listener_add_any_inet_port(GSocketListener *listener, GObject *source_object, GError **error) {
  GSList *sockets_to_close = NULL;
  guint16 candidate_port = 0;
  GSocket *socket6 = NULL;
  GSocket *socket4 = NULL;
  gint attempts = 37;
  while(attempts--) {
      GInetAddress *inet_address;
      GSocketAddress *address;
      gboolean result;
      g_assert(socket6 == NULL);
      socket6 = g_socket_new(G_SOCKET_FAMILY_IPV6,G_SOCKET_TYPE_STREAM,G_SOCKET_PROTOCOL_DEFAULT,NULL);
      if (socket6 != NULL) {
          inet_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV6);
          address = g_inet_socket_address_new(inet_address, 0);
          g_object_unref(inet_address);
          result = g_socket_bind(socket6, address, TRUE, error);
          g_object_unref(address);
          if (!result || !(address = g_socket_get_local_address(socket6, error))) {
              g_object_unref(socket6);
              socket6 = NULL;
              break;
          }
          g_assert(G_IS_INET_SOCKET_ADDRESS(address));
          candidate_port = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(address));
          g_assert(candidate_port != 0);
          g_object_unref(address);
          if (g_socket_speaks_ipv4(socket6)) break;
      }
      g_assert(socket4 == NULL);
      socket4 = g_socket_new(G_SOCKET_FAMILY_IPV4,G_SOCKET_TYPE_STREAM,G_SOCKET_PROTOCOL_DEFAULT, socket6 ? NULL : error);
      if (socket4 == NULL) break;
      inet_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
      address = g_inet_socket_address_new(inet_address, candidate_port);
      g_object_unref(inet_address);
      result = g_socket_bind(socket4, address, TRUE,(candidate_port && attempts) ? NULL : error);
      g_object_unref(address);
      if (candidate_port) {
          g_assert(socket6 != NULL);
          if (result) break;
          else {
              g_object_unref(socket4);
              socket4 = NULL;
              sockets_to_close = g_slist_prepend(sockets_to_close, socket6);
              candidate_port = 0;
              socket6 = NULL;
          }
      } else {
          g_assert(socket6 == NULL);
          if (!result || !(address = g_socket_get_local_address(socket4, error))) {
              g_object_unref(socket4);
              socket4 = NULL;
              break;
          }
          g_assert(G_IS_INET_SOCKET_ADDRESS(address));
          candidate_port = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(address));
          g_assert(candidate_port != 0);
          g_object_unref(address);
          break;
      }
  }
  g_assert((candidate_port != 0) == (socket4 || socket6));
  while(sockets_to_close) {
      g_object_unref(sockets_to_close->data);
      sockets_to_close = g_slist_delete_link(sockets_to_close, sockets_to_close);
  }
  if (socket6 != NULL) {
      g_socket_set_listen_backlog(socket6, listener->priv->listen_backlog);
      if (!g_socket_listen(socket6, error)) {
          g_object_unref(socket6);
          if (socket4) g_object_unref(socket4);
          return 0;
      }
      if (source_object) g_object_set_qdata_full(G_OBJECT(socket6), source_quark, g_object_ref(source_object), g_object_unref);
      g_ptr_array_add(listener->priv->sockets, socket6);
  }
  if (socket4 != NULL) {
      g_socket_set_listen_backlog(socket4, listener->priv->listen_backlog);
      if (!g_socket_listen(socket4, error)) {
          g_object_unref(socket4);
          if (socket6) g_object_unref(socket6);
          return 0;
      }
      if (source_object) g_object_set_qdata_full(G_OBJECT(socket4), source_quark, g_object_ref(source_object), g_object_unref);
      g_ptr_array_add(listener->priv->sockets, socket4);
  }
  if ((socket4 != NULL || socket6 != NULL) && G_SOCKET_LISTENER_GET_CLASS (listener)->changed) G_SOCKET_LISTENER_GET_CLASS(listener)->changed(listener);
  return candidate_port;
}