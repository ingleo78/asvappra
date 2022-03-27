#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "../glib/glibintl.h"
#include "config.h"
#include "giotypes.h"
#include "gioerror.h"
#include "gdbusaddress.h"
#include "gdbusutils.h"
#include "gdbusconnection.h"
#include "gdbusserver.h"
#include "gioenumtypes.h"
#include "gdbusprivate.h"
#include "gdbusauthobserver.h"
#include "ginitable.h"
#include "gsocketservice.h"
#include "gthreadedsocketservice.h"
#include "gresolver.h"
#include "ginetaddress.h"
#include "ginetsocketaddress.h"
#include "ginputstream.h"
#include "giostream.h"
#include "gunixsocketaddress.h"

struct _GDBusServer {
  GObject parent_instance;
  GDBusServerFlags flags;
  gchar *address;
  gchar *guid;
  guchar *nonce;
  gchar *nonce_file;
  gchar *client_address;
  GSocketListener *listener;
  gboolean is_using_listener;
  gulong run_signal_handler_id;
  GMainContext *main_context_at_construction;
  gboolean active;
  GDBusAuthObserver *authentication_observer;
};
typedef struct _GDBusServerClass GDBusServerClass;
struct _GDBusServerClass {
  GObjectClass parent_class;
  gboolean (*new_connection)(GDBusServer *server, GDBusConnection *connection);
};
enum {
  PROP_0,
  PROP_ADDRESS,
  PROP_CLIENT_ADDRESS,
  PROP_FLAGS,
  PROP_GUID,
  PROP_ACTIVE,
  PROP_AUTHENTICATION_OBSERVER
};
enum {
  NEW_CONNECTION_SIGNAL,
  LAST_SIGNAL,
};
guint _signals[LAST_SIGNAL] = {0};
static void initable_iface_init  (GInitableIface *initable_iface);
G_DEFINE_TYPE_WITH_CODE(GDBusServer, g_dbus_server, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE, initable_iface_init));
static void g_dbus_server_finalize(GObject *object) {
  GDBusServer *server = G_DBUS_SERVER(object);
  if (server->authentication_observer != NULL) g_object_unref(server->authentication_observer);
  if (server->run_signal_handler_id > 0) g_signal_handler_disconnect(server->listener, server->run_signal_handler_id);
  if (server->listener != NULL) g_object_unref(server->listener);
  g_free(server->address);
  g_free(server->guid);
  g_free(server->client_address);
  if (server->nonce != NULL) {
      memset(server->nonce, '\0', 16);
      g_free(server->nonce);
  }
  g_free(server->nonce_file);
  if (server->main_context_at_construction != NULL) g_main_context_unref(server->main_context_at_construction);
  G_OBJECT_CLASS(g_dbus_server_parent_class)->finalize(object);
}
static void g_dbus_server_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GDBusServer *server = G_DBUS_SERVER(object);
  switch(prop_id) {
      case PROP_FLAGS: g_value_set_flags(value, server->flags); break;
      case PROP_GUID: g_value_set_string(value, server->guid); break;
      case PROP_ADDRESS: g_value_set_string(value, server->address); break;
      case PROP_CLIENT_ADDRESS: g_value_set_string(value, server->client_address); break;
      case PROP_ACTIVE: g_value_set_boolean(value, server->active); break;
      case PROP_AUTHENTICATION_OBSERVER: g_value_set_object(value, server->authentication_observer); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_dbus_server_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GDBusServer *server = G_DBUS_SERVER(object);
  switch(prop_id) {
      case PROP_FLAGS: server->flags = g_value_get_flags(value); break;
      case PROP_GUID: server->guid = g_value_dup_string(value); break;
      case PROP_ADDRESS: server->address = g_value_dup_string(value); break;
      case PROP_AUTHENTICATION_OBSERVER: server->authentication_observer = g_value_dup_object(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_dbus_server_class_init(GDBusServerClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_dbus_server_finalize;
  gobject_class->set_property = g_dbus_server_set_property;
  gobject_class->get_property = g_dbus_server_get_property;
  g_object_class_install_property(gobject_class, PROP_FLAGS, g_param_spec_flags("flags", "Flags", "Flags for the server",
                                  G_TYPE_DBUS_SERVER_FLAGS, G_DBUS_SERVER_FLAGS_NONE, G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY |
                                  G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class,PROP_GUID,g_param_spec_string("guid","GUID","The guid of the server",
                                  NULL,G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB |
                                  G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class,PROP_ADDRESS,g_param_spec_string("address","Address","The address to listen on",
                                  NULL,G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB |
                                  G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class,PROP_CLIENT_ADDRESS,g_param_spec_string("client-address","Client Address",
                                  "The address clients can use",NULL,G_PARAM_READABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB |
                                  G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class,PROP_ACTIVE,g_param_spec_boolean("active","Active",
                                  "Whether the server is currently active",FALSE,G_PARAM_READABLE | G_PARAM_STATIC_NAME |
                                  G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class, PROP_AUTHENTICATION_OBSERVER, g_param_spec_object("authentication-observer","Authentication Observer",
                                  "Object used to assist in the authentication process", G_TYPE_DBUS_AUTH_OBSERVER, G_PARAM_READABLE | G_PARAM_WRITABLE |
                                  G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));
  _signals[NEW_CONNECTION_SIGNAL] = g_signal_new("new-connection", G_TYPE_DBUS_SERVER, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GDBusServerClass, new_connection),
                                                 g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 1, G_TYPE_DBUS_CONNECTION);
}
static void g_dbus_server_init(GDBusServer *server) {
  server->main_context_at_construction = g_main_context_get_thread_default();
  if (server->main_context_at_construction != NULL) g_main_context_ref(server->main_context_at_construction);
}
static gboolean on_run(GSocketService *service, GSocketConnection *socket_connection, GObject *source_object, gpointer user_data);
GDBusServer *g_dbus_server_new_sync(const gchar *address, GDBusServerFlags flags, const gchar *guid, GDBusAuthObserver *observer, GCancellable *cancellable,
                                    GError **error) {
  GDBusServer *server;
  g_return_val_if_fail(address != NULL, NULL);
  g_return_val_if_fail(g_dbus_is_guid (guid), NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  server = g_initable_new(G_TYPE_DBUS_SERVER, cancellable, error, "address", address, "flags", flags, "guid", guid, "authentication-observer", observer, NULL);
  if (server != NULL) {
      g_assert(server->is_using_listener);
      server->run_signal_handler_id = g_signal_connect(G_SOCKET_SERVICE(server->listener),"run", G_CALLBACK(on_run), server);
  }
  return server;
}
const gchar *g_dbus_server_get_client_address(GDBusServer *server) {
  g_return_val_if_fail(G_IS_DBUS_SERVER(server), NULL);
  return server->client_address;
}
const gchar *g_dbus_server_get_guid(GDBusServer *server) {
  g_return_val_if_fail(G_IS_DBUS_SERVER(server), NULL);
  return server->guid;
}
GDBusServerFlags g_dbus_server_get_flags(GDBusServer *server) {
  g_return_val_if_fail(G_IS_DBUS_SERVER(server), G_DBUS_SERVER_FLAGS_NONE);
  return server->flags;
}
gboolean g_dbus_server_is_active(GDBusServer *server) {
  g_return_val_if_fail(G_IS_DBUS_SERVER(server), G_DBUS_SERVER_FLAGS_NONE);
  return server->active;
}
void g_dbus_server_start(GDBusServer *server) {
  g_return_if_fail(G_IS_DBUS_SERVER(server));
  if (server->active) return;
  g_assert(server->is_using_listener);
  g_socket_service_start(G_SOCKET_SERVICE(server->listener));
  server->active = TRUE;
  g_object_notify(G_OBJECT(server), "active");
}
void g_dbus_server_stop(GDBusServer *server) {
  g_return_if_fail(G_IS_DBUS_SERVER(server));
  if (!server->active) return;
  g_assert(server->is_using_listener);
  g_assert(server->run_signal_handler_id > 0);
  g_signal_handler_disconnect(server->listener, server->run_signal_handler_id);
  server->run_signal_handler_id = 0;
  g_socket_service_stop(G_SOCKET_SERVICE(server->listener));
  server->active = FALSE;
  g_object_notify(G_OBJECT(server), "active");
}
#ifndef G_OS_UNIX
static gint random_ascii(void) {
  gint ret;
  ret = g_random_int_range(0, 60);
  if (ret < 25) ret += 'A';
  else if (ret < 50) ret += 'a' - 25;
  else ret += '0' - 50;
  return ret;
}
static gboolean try_unix(GDBusServer *server, const gchar *address_entry, GHashTable *key_value_pairs, GError **error) {
  gboolean ret;
  const gchar *path;
  const gchar *tmpdir;
  const gchar *abstract;
  GSocketAddress *address;
  ret = FALSE;
  address = NULL;
  path = g_hash_table_lookup(key_value_pairs, "path");
  tmpdir = g_hash_table_lookup(key_value_pairs, "tmpdir");
  abstract = g_hash_table_lookup(key_value_pairs, "abstract");
  if (path != NULL) address = g_unix_socket_address_new(path);
  else if (tmpdir != NULL) {
      gint n;
      GString *s;
      GError *local_error;
  retry:
      s = g_string_new(tmpdir);
      g_string_append(s, "/dbus-");
      for (n = 0; n < 8; n++) g_string_append_c(s, random_ascii());
      if (g_unix_socket_address_abstract_names_supported()) address = g_unix_socket_address_new_with_type (s->str,-1,G_UNIX_SOCKET_ADDRESS_ABSTRACT);
      else address = g_unix_socket_address_new(s->str);
      g_string_free(s, TRUE);
      local_error = NULL;
      if (!g_socket_listener_add_address(server->listener, address,G_SOCKET_TYPE_STREAM,G_SOCKET_PROTOCOL_DEFAULT,NULL,
          NULL, &local_error)) {
          if (local_error->domain == G_IO_ERROR && local_error->code == G_IO_ERROR_ADDRESS_IN_USE) {
              g_error_free(local_error);
              goto retry;
          }
          g_propagate_error(error, local_error);
          goto out;
      }
      ret = TRUE;
      goto out;
  } else if (abstract != NULL) {
      if (!g_unix_socket_address_abstract_names_supported()) {
          g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED,"Abstract name space not supported");
          goto out;
      }
      address = g_unix_socket_address_new_with_type(abstract,-1,G_UNIX_SOCKET_ADDRESS_ABSTRACT);
  } else { g_assert_not_reached(); }
  if (!g_socket_listener_add_address(server->listener, address,G_SOCKET_TYPE_STREAM,G_SOCKET_PROTOCOL_DEFAULT,NULL,NULL,
      error)) {
    goto out;
  }
  ret = TRUE;
out:
  if (address != NULL) {
      if (ret) {
          server->is_using_listener = TRUE;
          switch(g_unix_socket_address_get_address_type(G_UNIX_SOCKET_ADDRESS(address))) {
            case G_UNIX_SOCKET_ADDRESS_ABSTRACT:
              server->client_address = g_strdup_printf("unix:abstract=%s", g_unix_socket_address_get_path(G_UNIX_SOCKET_ADDRESS(address)));
              break;
            case G_UNIX_SOCKET_ADDRESS_PATH:
              server->client_address = g_strdup_printf("unix:path=%s", g_unix_socket_address_get_path(G_UNIX_SOCKET_ADDRESS(address)));
              break;
            default: g_assert_not_reached(); break;
          }
      }
      g_object_unref(address);
  }
  return ret;
}
#endif
static gboolean try_tcp(GDBusServer *server, const gchar *address_entry, GHashTable *key_value_pairs, gboolean do_nonce, GError **error) {
  gboolean ret;
  const gchar *host;
  const gchar *port;
  const gchar *family;
  gint port_num;
  GSocketAddress *address;
  GResolver *resolver;
  GList *resolved_addresses;
  GList *l;
  ret = FALSE;
  address = NULL;
  resolver = NULL;
  resolved_addresses = NULL;
  host = g_hash_table_lookup(key_value_pairs, "host");
  port = g_hash_table_lookup(key_value_pairs, "port");
  family = g_hash_table_lookup(key_value_pairs, "family");
  if (g_hash_table_lookup(key_value_pairs, "noncefile") != NULL) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Cannot specify nonce file when creating a server");
      goto out;
  }
  if (host == NULL) host = "localhost";
  if (port == NULL) port = "0";
  port_num = strtol(port, NULL, 10);
  resolver = g_resolver_get_default();
  resolved_addresses = g_resolver_lookup_by_name(resolver, host,NULL, error);
  if (resolved_addresses == NULL) goto out;
  for (l = resolved_addresses; l != NULL; l = l->next) {
      GInetAddress *address = G_INET_ADDRESS(l->data);
      GSocketAddress *socket_address;
      GSocketAddress *effective_address;
      socket_address = g_inet_socket_address_new(address, port_num);
      if (!g_socket_listener_add_address(server->listener, socket_address,G_SOCKET_TYPE_STREAM,G_SOCKET_PROTOCOL_TCP,NULL,
                                         &effective_address, error)) {
          g_object_unref(socket_address);
          goto out;
      }
      if (port_num == 0) port_num = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(effective_address));
      g_object_unref(effective_address);
      g_object_unref(socket_address);
  }
  if (do_nonce) {
      gint fd;
      guint n;
      gsize bytes_written;
      gsize bytes_remaining;
      server->nonce = g_new0(guchar,16);
      for (n = 0; n < 16; n++) server->nonce[n] = g_random_int_range(0,256);
      fd = g_file_open_tmp("gdbus-nonce-file-XXXXXX", &server->nonce_file, error);
      if (fd == -1) {
          g_socket_listener_close(server->listener);
          goto out;
      }
  again:
      bytes_written = 0;
      bytes_remaining = 16;
      while(bytes_remaining > 0) {
          gssize ret;
          ret = write(fd,server->nonce + bytes_written, bytes_remaining);
          if (ret == -1) {
              if (errno == EINTR) goto again;
              g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errno),"Error writing nonce file at `%s': %s", server->nonce_file, strerror(errno));
              goto out;
          }
          bytes_written += ret;
          bytes_remaining -= ret;
      }
      close(fd);
      server->client_address = g_strdup_printf("nonce-tcp:host=%s,port=%d,noncefile=%s", host, port_num, server->nonce_file);
  } else server->client_address = g_strdup_printf("tcp:host=%s,port=%d", host, port_num);
  server->is_using_listener = TRUE;
  ret = TRUE;
out:
  g_list_foreach(resolved_addresses, (GFunc)g_object_unref,NULL);
  g_list_free(resolved_addresses);
  g_object_unref(resolver);
  return ret;
}
typedef struct {
  GDBusServer *server;
  GDBusConnection *connection;
} EmitIdleData;
static void emit_idle_data_free(EmitIdleData *data) {
  g_object_unref(data->server);
  g_object_unref(data->connection);
  g_free(data);
}
static gboolean emit_new_connection_in_idle(gpointer user_data) {
  EmitIdleData *data = user_data;
  gboolean claimed;
  claimed = FALSE;
  g_signal_emit(data->server, _signals[NEW_CONNECTION_SIGNAL],0, data->connection, &claimed);
  if (claimed) g_dbus_connection_start_message_processing(data->connection);
  g_object_unref(data->connection);
  return FALSE;
}
static gboolean on_run(GSocketService *service, GSocketConnection *socket_connection, GObject *source_object, gpointer user_data) {
  GDBusServer *server = G_DBUS_SERVER(user_data);
  GDBusConnection *connection;
  GDBusConnectionFlags connection_flags;
  if (server->nonce != NULL) {
      gchar buf[16];
      gsize bytes_read;
      if (!g_input_stream_read_all(g_io_stream_get_input_stream(G_IO_STREAM(socket_connection)), buf, 16, &bytes_read, NULL, NULL)) goto out;
      if (bytes_read != 16) goto out;
      if (memcmp(buf, server->nonce, 16) != 0) goto out;
  }
  connection_flags = G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_SERVER | G_DBUS_CONNECTION_FLAGS_DELAY_MESSAGE_PROCESSING;
  if (server->flags & G_DBUS_SERVER_FLAGS_AUTHENTICATION_ALLOW_ANONYMOUS) connection_flags |= G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_ALLOW_ANONYMOUS;
  connection = g_dbus_connection_new_sync(G_IO_STREAM(socket_connection), server->guid, connection_flags, server->authentication_observer, NULL, NULL);
  if (connection == NULL) goto out;
  if (server->flags & G_DBUS_SERVER_FLAGS_RUN_IN_THREAD) {
      g_signal_emit(server, _signals[NEW_CONNECTION_SIGNAL],0, connection);
      g_dbus_connection_start_message_processing(connection);
      g_object_unref(connection);
  } else {
      GSource *idle_source;
      EmitIdleData *data;
      data = g_new0(EmitIdleData, 1);
      data->server = g_object_ref(server);
      data->connection = g_object_ref(connection);
      idle_source = g_idle_source_new();
      g_source_set_priority(idle_source, G_PRIORITY_DEFAULT);
      g_source_set_callback(idle_source, emit_new_connection_in_idle, data, (GDestroyNotify)emit_idle_data_free);
      g_source_attach(idle_source, server->main_context_at_construction);
      g_source_unref(idle_source);
  }
out:
  return TRUE;
}
static gboolean initable_init(GInitable *initable, GCancellable *cancellable, GError **error) {
  GDBusServer *server = G_DBUS_SERVER(initable);
  gboolean ret;
  guint n;
  gchar **addr_array;
  GError *last_error;
  ret = FALSE;
  addr_array = NULL;
  last_error = NULL;
  if (!g_dbus_is_guid(server->guid)) {
      g_set_error(&last_error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"The string `%s' is not a valid D-Bus GUID", server->guid);
      goto out;
  }
  server->listener = G_SOCKET_LISTENER(g_threaded_socket_service_new(-1));
  addr_array = g_strsplit(server->address, ";", 0);
  last_error = NULL;
  for (n = 0; addr_array != NULL && addr_array[n] != NULL; n++) {
      const gchar *address_entry = addr_array[n];
      GHashTable *key_value_pairs;
      gchar *transport_name;
      GError *this_error;
      this_error = NULL;
      if (g_dbus_is_supported_address(address_entry, &this_error) && _g_dbus_address_parse_entry(address_entry, &transport_name, &key_value_pairs, &this_error)) {
          if (g_strcmp0(transport_name, "unix") == 0) ret = try_unix(server, address_entry, key_value_pairs, &this_error);
          else if (g_strcmp0(transport_name, "tcp") == 0) ret = try_tcp(server, address_entry, key_value_pairs, FALSE, &this_error);
          else if (g_strcmp0(transport_name, "nonce-tcp") == 0) ret = try_tcp(server, address_entry, key_value_pairs, TRUE, &this_error);
          else g_set_error(&this_error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Cannot listen on unsupported transport `%s'", transport_name);
          g_free(transport_name);
          if (key_value_pairs != NULL) g_hash_table_unref(key_value_pairs);
          if (ret) {
              g_assert(this_error == NULL);
              goto out;
          }
      }
      if (this_error != NULL) {
          if (last_error != NULL) g_error_free(last_error);
          last_error = this_error;
      }
  }
  if (!ret) goto out;
out:
  g_strfreev(addr_array);
  if (ret) {
      if (last_error != NULL) g_error_free(last_error);
  } else {
      g_assert(last_error != NULL);
      g_propagate_error(error, last_error);
  }
  return ret;
}
static void initable_iface_init(GInitableIface *initable_iface) {
  initable_iface->init = initable_init;
}