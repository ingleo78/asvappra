#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "../glib/glibintl.h"
#include "config.h"
#include "gdbusauth.h"
#include "gdbusutils.h"
#include "gdbusaddress.h"
#include "gdbusmessage.h"
#include "gdbusconnection.h"
#include "gdbuserror.h"
#include "gioenumtypes.h"
#include "gdbusintrospection.h"
#include "gdbusmethodinvocation.h"
#include "gdbusprivate.h"
#include "gdbusauthobserver.h"
#include "ginitable.h"
#include "gasyncinitable.h"
#include "giostream.h"
#include "gasyncresult.h"
#include "gsimpleasyncresult.h"
#include "gunixconnection.h"
#include "gunixfdmessage.h"

static guint signals[2];
typedef struct _GDBusConnectionClass GDBusConnectionClass;
struct _GDBusConnectionClass {
  GObjectClass parent_class;
  void (*closed)(GDBusConnection *connection, gboolean remote_peer_vanished, GError *error);
};
G_LOCK_DEFINE_STATIC(message_bus_lock);
static GDBusConnection *the_session_bus = NULL;
static GDBusConnection *the_system_bus = NULL;
typedef struct {
  GDestroyNotify callback;
  gpointer user_data;
  GMainContext *context;
} CallDestroyNotifyData;
static gboolean call_destroy_notify_data_in_idle(gpointer user_data) {
  CallDestroyNotifyData *data = user_data;
  data->callback(data->user_data);
  return FALSE;
}
static void call_destroy_notify_data_free(CallDestroyNotifyData *data) {
  if (data->context != NULL) g_main_context_unref(data->context);
  g_free(data);
}
static void call_destroy_notify(GMainContext *context, GDestroyNotify callback, gpointer user_data) {
  if (callback == NULL) return;
  if (context == g_main_context_get_thread_default()) callback(user_data);
  else {
      GSource *idle_source;
      CallDestroyNotifyData *data;
      data = g_new0(CallDestroyNotifyData, 1);
      data->callback = callback;
      data->user_data = user_data;
      data->context = context;
      if (data->context != NULL) g_main_context_ref(data->context);
      idle_source = g_idle_source_new();
      g_source_set_priority(idle_source, G_PRIORITY_DEFAULT);
      g_source_set_callback(idle_source, call_destroy_notify_data_in_idle, data, (GDestroyNotify)call_destroy_notify_data_free);
      g_source_attach(idle_source, data->context);
      g_source_unref(idle_source);
  }
}
static gboolean _g_strv_has_string(const gchar* const *haystack, const gchar *needle) {
  guint n;
  for (n = 0; haystack != NULL && haystack[n] != NULL; n++) {
      if (g_strcmp0(haystack[n], needle) == 0) return TRUE;
  }
  return FALSE;
}
#ifndef G_OS_WIN32
#define CONNECTION_ENSURE_LOCK(obj) do { ; } while (FALSE)
#else
#define CONNECTION_ENSURE_LOCK(obj) \
  do {  \
      if (G_UNLIKELY (g_mutex_trylock((obj)->lock))) { \
          g_assertion_message(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, "CONNECTION_ENSURE_LOCK: GDBusConnection object lock is not locked"); \
      }  \
  } while(FALSE)
#endif
#define CONNECTION_LOCK(obj) \
  do { \
      g_mutex_lock((obj)->lock); \
  } while(FALSE)
#define CONNECTION_UNLOCK(obj) \
  do { \
      g_mutex_unlock((obj)->lock); \
  } while(FALSE)
struct _GDBusConnection {
  GObject parent_instance;
  GMutex *lock;
  GMutex *init_lock;
  gchar *machine_id;
  GIOStream *stream;
  GDBusAuth *auth;
  gboolean closed;
  guint32 last_serial;
  GDBusWorker *worker;
  gchar *bus_unique_name;
  gchar *guid;
  gboolean is_initialized;
  GError *initialization_error;
  GMainContext *main_context_at_construction;
  gchar *address;
  GDBusConnectionFlags flags;
  GHashTable *map_method_serial_to_send_message_data;
  GHashTable *map_rule_to_signal_data;
  GHashTable *map_id_to_signal_data;
  GHashTable *map_sender_unique_name_to_signal_data_array;
  GHashTable *map_object_path_to_eo;
  GHashTable *map_id_to_ei;
  GHashTable *map_object_path_to_es;
  GHashTable *map_id_to_es;
  GPtrArray *filters;
  gboolean exit_on_close;
  GDBusCapabilityFlags capabilities;
  GDBusAuthObserver *authentication_observer;
  GCredentials *credentials;
  gboolean finalizing;
};
typedef struct ExportedObject ExportedObject;
static void exported_object_free(ExportedObject *eo);
typedef struct ExportedSubtree ExportedSubtree;
static void exported_subtree_free (ExportedSubtree *es);
enum {
  CLOSED_SIGNAL,
  LAST_SIGNAL,
};
enum {
  PROP_0,
  PROP_STREAM,
  PROP_ADDRESS,
  PROP_FLAGS,
  PROP_GUID,
  PROP_UNIQUE_NAME,
  PROP_CLOSED,
  PROP_EXIT_ON_CLOSE,
  PROP_CAPABILITY_FLAGS,
  PROP_AUTHENTICATION_OBSERVER,
};
static void distribute_signals(GDBusConnection *connection, GDBusMessage *message);
static void distribute_method_call(GDBusConnection *connection, GDBusMessage *message);
static gboolean handle_generic_unlocked(GDBusConnection *connection, GDBusMessage *message);
static void purge_all_signal_subscriptions(GDBusConnection *connection);
static void purge_all_filters(GDBusConnection *connection);
#define _G_ENSURE_LOCK(name) \
  do { \
      if (G_UNLIKELY (G_TRYLOCK(name))) { \
        g_assertion_message(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, "_G_ENSURE_LOCK: Lock `" #name "' is not locked"); \
      } \
  } while(FALSE) \
static guint signals[LAST_SIGNAL] = { 0 };
static void initable_iface_init(GInitableIface *initable_iface);
static void async_initable_iface_init(GAsyncInitableIface *async_initable_iface);
G_DEFINE_TYPE_WITH_CODE(GDBusConnection, g_dbus_connection, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE, initable_iface_init)
                          G_IMPLEMENT_INTERFACE(G_TYPE_ASYNC_INITABLE, async_initable_iface_init));
static GHashTable *alive_connections = NULL;
static void g_dbus_connection_dispose(GObject *object) {
  GDBusConnection *connection = G_DBUS_CONNECTION(object);
  G_LOCK(message_bus_lock);
  if (connection == the_session_bus) the_session_bus = NULL;
  else if (connection == the_system_bus) the_system_bus = NULL;
  CONNECTION_LOCK(connection);
  if (connection->worker != NULL) {
      _g_dbus_worker_stop(connection->worker);
      connection->worker = NULL;
      if (alive_connections != NULL) g_warn_if_fail(g_hash_table_remove(alive_connections, connection));
  } else {
      if (alive_connections != NULL) g_warn_if_fail(g_hash_table_lookup(alive_connections, connection) == NULL);
  }
  CONNECTION_UNLOCK(connection);
  G_UNLOCK(message_bus_lock);
  if (G_OBJECT_CLASS(g_dbus_connection_parent_class)->dispose != NULL) G_OBJECT_CLASS(g_dbus_connection_parent_class)->dispose(object);
}
static void g_dbus_connection_finalize(GObject *object) {
  GDBusConnection *connection = G_DBUS_CONNECTION(object);
  connection->finalizing = TRUE;
  purge_all_signal_subscriptions(connection);
  purge_all_filters(connection);
  g_ptr_array_unref(connection->filters);
  if (connection->authentication_observer != NULL) g_object_unref(connection->authentication_observer);
  if (connection->auth != NULL) g_object_unref(connection->auth);
  if (connection->credentials) g_object_unref(connection->credentials);
  if (connection->stream != NULL) {
      g_io_stream_close_async(connection->stream, G_PRIORITY_DEFAULT,NULL,NULL,NULL);
      g_object_unref(connection->stream);
      connection->stream = NULL;
  }
  g_free(connection->address);
  g_free(connection->guid);
  g_free(connection->bus_unique_name);
  if (connection->initialization_error != NULL) g_error_free(connection->initialization_error);
  g_hash_table_unref(connection->map_method_serial_to_send_message_data);
  g_hash_table_unref(connection->map_rule_to_signal_data);
  g_hash_table_unref(connection->map_id_to_signal_data);
  g_hash_table_unref(connection->map_sender_unique_name_to_signal_data_array);
  g_hash_table_unref(connection->map_id_to_ei);
  g_hash_table_unref(connection->map_object_path_to_eo);
  g_hash_table_unref(connection->map_id_to_es);
  g_hash_table_unref(connection->map_object_path_to_es);
  if (connection->main_context_at_construction != NULL) g_main_context_unref(connection->main_context_at_construction);
  g_free(connection->machine_id);
  g_mutex_free(connection->init_lock);
  g_mutex_free(connection->lock);
  G_OBJECT_CLASS(g_dbus_connection_parent_class)->finalize(object);
}
static void g_dbus_connection_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GDBusConnection *connection = G_DBUS_CONNECTION(object);
  switch(prop_id) {
      case PROP_STREAM: g_value_set_object(value, g_dbus_connection_get_stream(connection)); break;
      case PROP_GUID: g_value_set_string(value, g_dbus_connection_get_guid(connection)); break;
      case PROP_UNIQUE_NAME: g_value_set_string(value, g_dbus_connection_get_unique_name(connection)); break;
      case PROP_CLOSED: g_value_set_boolean (value, g_dbus_connection_is_closed(connection)); break;
      case PROP_EXIT_ON_CLOSE: g_value_set_boolean(value, g_dbus_connection_get_exit_on_close(connection)); break;
      case PROP_CAPABILITY_FLAGS: g_value_set_flags(value, g_dbus_connection_get_capabilities(connection)); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_dbus_connection_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GDBusConnection *connection = G_DBUS_CONNECTION(object);
  switch(prop_id) {
      case PROP_STREAM: connection->stream = g_value_dup_object(value); break;
      case PROP_GUID: connection->guid = g_value_dup_string(value); break;
      case PROP_ADDRESS: connection->address = g_value_dup_string(value); break;
      case PROP_FLAGS: connection->flags = g_value_get_flags(value); break;
      case PROP_EXIT_ON_CLOSE: g_dbus_connection_set_exit_on_close(connection, g_value_get_boolean(value)); break;
      case PROP_AUTHENTICATION_OBSERVER: connection->authentication_observer = g_value_dup_object(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_dbus_connection_real_closed(GDBusConnection *connection, gboolean remote_peer_vanished, GError *error) {
  if (remote_peer_vanished && connection->exit_on_close) {
      if (error != NULL) {
          g_print("%s: Remote peer vanished with error: %s (%s, %d). Exiting.\n", G_STRFUNC, error->message, g_quark_to_string(error->domain), error->code);
      } else g_print ("%s: Remote peer vanished. Exiting.\n", G_STRFUNC);
      raise(SIGTERM);
  }
}
static void g_dbus_connection_class_init(GDBusConnectionClass *klass) {
  GObjectClass *gobject_class;
  gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_dbus_connection_finalize;
  gobject_class->dispose = g_dbus_connection_dispose;
  gobject_class->set_property = g_dbus_connection_set_property;
  gobject_class->get_property = g_dbus_connection_get_property;
  klass->closed = g_dbus_connection_real_closed;
  g_object_class_install_property(gobject_class, PROP_STREAM, g_param_spec_object("stream", "IO Stream","The underlying streams used for I/O",
                                  G_TYPE_IO_STREAM, G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB |
                                  G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class,PROP_ADDRESS,g_param_spec_string("address","Address",
                                  "D-Bus address specifying potential socket endpoints",NULL,G_PARAM_WRITABLE |
                                  G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class, PROP_FLAGS, g_param_spec_flags("flags", "Flags", "Flags", G_TYPE_DBUS_CONNECTION_FLAGS,
                                  G_DBUS_CONNECTION_FLAGS_NONE, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB |
                                  G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class,PROP_GUID,g_param_spec_string("guid","GUID",
                                  "GUID of the server peer",NULL,G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY |
                                  G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class,PROP_UNIQUE_NAME,g_param_spec_string("unique-name","unique-name",
                                  "Unique name of bus connection",NULL,G_PARAM_READABLE | G_PARAM_STATIC_NAME |
                                  G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class,PROP_CLOSED,g_param_spec_boolean("closed","Closed",
                                  "Whether the connection is closed",FALSE,G_PARAM_READABLE | G_PARAM_STATIC_NAME |
                                  G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class,PROP_EXIT_ON_CLOSE,g_param_spec_boolean("exit-on-close","Exit on close",
                                  "Whether the process is terminated when the connection is closed",FALSE,G_PARAM_READABLE |
                                  G_PARAM_WRITABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class, PROP_CAPABILITY_FLAGS, g_param_spec_flags("capabilities", "Capabilities", "Capabilities",
                                  G_TYPE_DBUS_CAPABILITY_FLAGS, G_DBUS_CAPABILITY_FLAGS_NONE, G_PARAM_READABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB |
                                  G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class, PROP_AUTHENTICATION_OBSERVER, g_param_spec_object("authentication-observer","Authentication Observer",
                                  "Object used to assist in the authentication process", G_TYPE_DBUS_AUTH_OBSERVER, G_PARAM_WRITABLE |
                                  G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));
  signals[CLOSED_SIGNAL] = g_signal_new("closed", G_TYPE_DBUS_CONNECTION, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GDBusConnectionClass, closed), NULL, NULL,
                                        NULL, G_TYPE_NONE, 2, G_TYPE_BOOLEAN, G_TYPE_ERROR);
}
static void g_dbus_connection_init(GDBusConnection *connection) {
  connection->lock = g_mutex_new();
  connection->init_lock = g_mutex_new();
  connection->map_method_serial_to_send_message_data = g_hash_table_new(g_direct_hash, g_direct_equal);
  connection->map_rule_to_signal_data = g_hash_table_new(g_str_hash, g_str_equal);
  connection->map_id_to_signal_data = g_hash_table_new(g_direct_hash, g_direct_equal);
  connection->map_sender_unique_name_to_signal_data_array = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_ptr_array_unref);
  connection->map_object_path_to_eo = g_hash_table_new_full(g_str_hash, g_str_equal,NULL, (GDestroyNotify)exported_object_free);
  connection->map_id_to_ei = g_hash_table_new(g_direct_hash, g_direct_equal);
  connection->map_object_path_to_es = g_hash_table_new_full(g_str_hash, g_str_equal,NULL, (GDestroyNotify)exported_subtree_free);
  connection->map_id_to_es = g_hash_table_new(g_direct_hash, g_direct_equal);
  connection->main_context_at_construction = g_main_context_get_thread_default();
  if (connection->main_context_at_construction != NULL) g_main_context_ref(connection->main_context_at_construction);
  connection->filters = g_ptr_array_new();
}
GIOStream *g_dbus_connection_get_stream(GDBusConnection *connection) {
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), NULL);
  return connection->stream;
}
void g_dbus_connection_start_message_processing(GDBusConnection *connection) {
  g_return_if_fail(G_IS_DBUS_CONNECTION(connection));
  _g_dbus_worker_unfreeze(connection->worker);
}
gboolean g_dbus_connection_is_closed(GDBusConnection *connection) {
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), FALSE);
  return connection->closed;
}
GDBusCapabilityFlags g_dbus_connection_get_capabilities(GDBusConnection *connection) {
  g_return_val_if_fail (G_IS_DBUS_CONNECTION(connection), G_DBUS_CAPABILITY_FLAGS_NONE);
  return connection->capabilities;
}
static void flush_in_thread_func(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GError *error;
  error = NULL;
  if (!g_dbus_connection_flush_sync(G_DBUS_CONNECTION(object), cancellable, &error)) g_simple_async_result_take_error(res, error);
}
void g_dbus_connection_flush(GDBusConnection *connection, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *simple;
  g_return_if_fail(G_IS_DBUS_CONNECTION(connection));
  simple = g_simple_async_result_new(G_OBJECT(connection), callback, user_data, g_dbus_connection_flush);
  g_simple_async_result_run_in_thread(simple, flush_in_thread_func, G_PRIORITY_DEFAULT, cancellable);
  g_object_unref(simple);
}
gboolean g_dbus_connection_flush_finish(GDBusConnection *connection, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  gboolean ret;
  ret = FALSE;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_dbus_connection_flush);
  if (g_simple_async_result_propagate_error(simple, error)) goto out;
  ret = TRUE;
out:
  return ret;
}
gboolean g_dbus_connection_flush_sync(GDBusConnection *connection, GCancellable *cancellable, GError **error) {
  gboolean ret;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  ret = FALSE;
  if (connection->closed) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_CLOSED, "The connection is closed");
      goto out;
  }
  ret = _g_dbus_worker_flush_sync(connection->worker, cancellable, error);
out:
  return ret;
}
typedef struct {
  GDBusConnection *connection;
  GError *error;
  gboolean remote_peer_vanished;
} EmitClosedData;
static void emit_closed_data_free(EmitClosedData *data) {
  g_object_unref(data->connection);
  if (data->error != NULL) g_error_free(data->error);
  g_free(data);
}
static gboolean emit_closed_in_idle(gpointer user_data) {
  EmitClosedData *data = user_data;
  gboolean result;
  g_object_notify(G_OBJECT(data->connection), "closed");
  g_signal_emit(data->connection, signals[CLOSED_SIGNAL], 0, data->remote_peer_vanished, data->error, &result);
  return FALSE;
}
static void set_closed_unlocked(GDBusConnection *connection, gboolean remote_peer_vanished, GError *error) {
  GSource *idle_source;
  EmitClosedData *data;
  CONNECTION_ENSURE_LOCK(connection);
  g_assert(!connection->closed);
  connection->closed = TRUE;
  data = g_new0(EmitClosedData, 1);
  data->connection = g_object_ref(connection);
  data->remote_peer_vanished = remote_peer_vanished;
  data->error = error != NULL ? g_error_copy(error) : NULL;
  idle_source = g_idle_source_new();
  g_source_set_priority(idle_source, G_PRIORITY_DEFAULT);
  g_source_set_callback(idle_source, emit_closed_in_idle, data, (GDestroyNotify)emit_closed_data_free);
  g_source_attach(idle_source, connection->main_context_at_construction);
  g_source_unref(idle_source);
}
static void close_in_thread_func(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GError *error;
  error = NULL;
  if (!g_dbus_connection_close_sync(G_DBUS_CONNECTION(object), cancellable, &error))
    g_simple_async_result_take_error(res, error);
}
void g_dbus_connection_close(GDBusConnection *connection, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *simple;
  g_return_if_fail(G_IS_DBUS_CONNECTION(connection));
  simple = g_simple_async_result_new(G_OBJECT(connection), callback, user_data, g_dbus_connection_close);
  g_simple_async_result_run_in_thread(simple, close_in_thread_func, G_PRIORITY_DEFAULT, cancellable);
  g_object_unref(simple);
}
gboolean g_dbus_connection_close_finish(GDBusConnection *connection, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (res);
  gboolean ret;
  ret = FALSE;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION (connection), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT (res), FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  g_warn_if_fail (g_simple_async_result_get_source_tag(simple) == g_dbus_connection_close);
  if (g_simple_async_result_propagate_error(simple, error)) goto out;
  ret = TRUE;
out:
  return ret;
}
gboolean g_dbus_connection_close_sync(GDBusConnection *connection, GCancellable *cancellable, GError **error) {
  gboolean ret;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  ret = FALSE;
  CONNECTION_LOCK(connection);
  if (!connection->closed) {
      ret = g_io_stream_close(connection->stream, cancellable, error);
      if (ret) set_closed_unlocked(connection, FALSE, NULL);
  } else g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_CLOSED,"The connection is closed");
  CONNECTION_UNLOCK(connection);
  return ret;
}
static gboolean g_dbus_connection_send_message_unlocked(GDBusConnection *connection, GDBusMessage *message, GDBusSendMessageFlags flags,
                                                        volatile guint32 *out_serial, GError **error) {
  guchar *blob;
  gsize blob_size;
  guint32 serial_to_use;
  gboolean ret;
  CONNECTION_ENSURE_LOCK(connection);
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), FALSE);
  g_return_val_if_fail(G_IS_DBUS_MESSAGE(message), FALSE);
  ret = FALSE;
  blob = NULL;
  if (out_serial != NULL) *out_serial = 0;
  if (connection->closed) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_CLOSED,"The connection is closed");
      goto out;
  }
  blob = g_dbus_message_to_blob(message, &blob_size, connection->capabilities, error);
  if (blob == NULL) goto out;
  if (flags & G_DBUS_SEND_MESSAGE_FLAGS_PRESERVE_SERIAL) serial_to_use = g_dbus_message_get_serial(message);
  else serial_to_use = ++connection->last_serial;
  switch(blob[0]) {
      case 'l': ((guint32*)blob)[2] = GUINT32_TO_LE(serial_to_use); break;
      case 'B': ((guint32*)blob)[2] = GUINT32_TO_BE(serial_to_use); break;
      default: g_assert_not_reached(); break;
  }
#if 0
  g_printerr("Writing message of %" G_GSIZE_FORMAT " bytes (serial %d) on %p:\n", blob_size, serial_to_use, connection);
  g_printerr("----\n");
  hexdump(blob, blob_size);
  g_printerr("----\n");
#endif
  if (out_serial != NULL) *out_serial = serial_to_use;
  if (!(flags & G_DBUS_SEND_MESSAGE_FLAGS_PRESERVE_SERIAL)) g_dbus_message_set_serial(message, serial_to_use);
  g_dbus_message_lock(message);
  _g_dbus_worker_send_message(connection->worker, message, (gchar*)blob, blob_size);
  blob = NULL;
  ret = TRUE;
out:
  g_free(blob);
  return ret;
}
gboolean g_dbus_connection_send_message(GDBusConnection *connection, GDBusMessage *message, GDBusSendMessageFlags flags, volatile guint32 *out_serial,
                                        GError **error) {
  gboolean ret;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), FALSE);
  g_return_val_if_fail(G_IS_DBUS_MESSAGE(message), FALSE);
  g_return_val_if_fail((flags & G_DBUS_SEND_MESSAGE_FLAGS_PRESERVE_SERIAL) || !g_dbus_message_get_locked(message), FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  CONNECTION_LOCK(connection);
  ret = g_dbus_connection_send_message_unlocked(connection, message, flags, out_serial, error);
  CONNECTION_UNLOCK(connection);
  return ret;
}
typedef struct {
  volatile gint ref_count;
  GDBusConnection *connection;
  guint32 serial;
  GSimpleAsyncResult *simple;
  GMainContext *main_context;
  GCancellable *cancellable;
  gulong cancellable_handler_id;
  GSource *timeout_source;
  gboolean delivered;
} SendMessageData;
static SendMessageData *send_message_data_ref(SendMessageData *data) {
  g_atomic_int_inc(&data->ref_count);
  return data;
}
static void send_message_data_unref(SendMessageData *data) {
  if (g_atomic_int_dec_and_test(&data->ref_count)) {
      g_assert(data->timeout_source == NULL);
      g_assert(data->simple == NULL);
      g_assert(data->cancellable_handler_id == 0);
      g_object_unref(data->connection);
      if (data->cancellable != NULL) g_object_unref(data->cancellable);
      if (data->main_context != NULL) g_main_context_unref(data->main_context);
      g_free(data);
  }
}
static void send_message_with_reply_deliver(SendMessageData *data) {
  CONNECTION_ENSURE_LOCK(data->connection);
  g_assert(!data->delivered);
  data->delivered = TRUE;
  g_simple_async_result_complete_in_idle(data->simple);
  g_object_unref(data->simple);
  data->simple = NULL;
  if (data->timeout_source != NULL) {
      g_source_destroy(data->timeout_source);
      data->timeout_source = NULL;
  }
  if (data->cancellable_handler_id > 0) {
      g_cancellable_disconnect(data->cancellable, data->cancellable_handler_id);
      data->cancellable_handler_id = 0;
  }
  g_warn_if_fail(g_hash_table_remove(data->connection->map_method_serial_to_send_message_data, GUINT_TO_POINTER(data->serial)));
  send_message_data_unref(data);
}
static void send_message_data_deliver_reply_unlocked(SendMessageData *data, GDBusMessage *reply) {
  if (data->delivered) return;
  g_simple_async_result_set_op_res_gpointer(data->simple, g_object_ref(reply), g_object_unref);
  send_message_with_reply_deliver(data);
}
static gboolean send_message_with_reply_cancelled_idle_cb(gpointer user_data) {
  SendMessageData *data = user_data;
  CONNECTION_LOCK(data->connection);
  if (data->delivered) goto out;
  g_simple_async_result_set_error(data->simple, G_IO_ERROR, G_IO_ERROR_CANCELLED,"Operation was cancelled");
  send_message_with_reply_deliver(data);
out:
  CONNECTION_UNLOCK(data->connection);
  return FALSE;
}
static void send_message_with_reply_cancelled_cb(GCancellable *cancellable, gpointer user_data) {
  SendMessageData *data = user_data;
  GSource *idle_source;
  idle_source = g_idle_source_new();
  g_source_set_priority(idle_source, G_PRIORITY_DEFAULT);
  g_source_set_callback(idle_source, send_message_with_reply_cancelled_idle_cb, send_message_data_ref(data), (GDestroyNotify)send_message_data_unref);
  g_source_attach(idle_source, data->main_context);
  g_source_unref(idle_source);
}
static gboolean send_message_with_reply_timeout_cb(gpointer user_data) {
  SendMessageData *data = user_data;
  CONNECTION_LOCK(data->connection);
  if (data->delivered) goto out;
  g_simple_async_result_set_error(data->simple, G_IO_ERROR,G_IO_ERROR_TIMED_OUT,"Timeout was reached");
  send_message_with_reply_deliver(data);
out:
  CONNECTION_UNLOCK(data->connection);
  return FALSE;
}
static void g_dbus_connection_send_message_with_reply_unlocked(GDBusConnection *connection, GDBusMessage *message, GDBusSendMessageFlags flags,
                                                               gint timeout_msec, volatile guint32 *out_serial, GCancellable *cancellable,
                                                               GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *simple;
  SendMessageData *data;
  GError *error;
  volatile guint32 serial;
  data = NULL;
  if (out_serial == NULL) out_serial = &serial;
  if (timeout_msec == -1) timeout_msec = 25 * 1000;
  simple = g_simple_async_result_new(G_OBJECT(connection), callback, user_data, g_dbus_connection_send_message_with_reply);
  if (g_cancellable_is_cancelled(cancellable)) {
      g_simple_async_result_set_error(simple, G_IO_ERROR,G_IO_ERROR_CANCELLED,"Operation was cancelled");
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  if (connection->closed) {
      g_simple_async_result_set_error(simple, G_IO_ERROR, G_IO_ERROR_CLOSED,"The connection is closed");
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  error = NULL;
  if (!g_dbus_connection_send_message_unlocked(connection, message, flags, out_serial, &error)) {
      g_simple_async_result_take_error(simple, error);
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  data = g_new0(SendMessageData, 1);
  data->ref_count = 1;
  data->connection = g_object_ref(connection);
  data->simple = simple;
  data->serial = *out_serial;
  data->main_context = g_main_context_get_thread_default();
  if (data->main_context != NULL) g_main_context_ref(data->main_context);
  if (cancellable != NULL) {
      data->cancellable = g_object_ref(cancellable);
      data->cancellable_handler_id = g_cancellable_connect(cancellable, G_CALLBACK(send_message_with_reply_cancelled_cb), send_message_data_ref(data),
                                                           (GDestroyNotify)send_message_data_unref);
      g_object_set_data_full(G_OBJECT(simple),"cancellable", g_object_ref(cancellable), (GDestroyNotify)g_object_unref);
  }
  if (timeout_msec != G_MAXINT) {
      data->timeout_source = g_timeout_source_new(timeout_msec);
      g_source_set_priority(data->timeout_source, G_PRIORITY_DEFAULT);
      g_source_set_callback(data->timeout_source, send_message_with_reply_timeout_cb, send_message_data_ref (data), (GDestroyNotify)send_message_data_unref);
      g_source_attach(data->timeout_source, data->main_context);
      g_source_unref(data->timeout_source);
  }
  g_hash_table_insert(connection->map_method_serial_to_send_message_data, GUINT_TO_POINTER(*out_serial), data);
}
void g_dbus_connection_send_message_with_reply(GDBusConnection *connection, GDBusMessage *message, GDBusSendMessageFlags flags, gint timeout_msec,
                                               volatile guint32 *out_serial, GCancellable *cancellable, GAsyncReadyCallback callback,
                                               gpointer user_data) {
  g_return_if_fail(G_IS_DBUS_CONNECTION (connection));
  g_return_if_fail(G_IS_DBUS_MESSAGE (message));
  g_return_if_fail((flags & G_DBUS_SEND_MESSAGE_FLAGS_PRESERVE_SERIAL) || !g_dbus_message_get_locked (message));
  g_return_if_fail(timeout_msec >= 0 || timeout_msec == -1);
  CONNECTION_LOCK(connection);
  g_dbus_connection_send_message_with_reply_unlocked(connection, message, flags, timeout_msec, out_serial, cancellable, callback, user_data);
  CONNECTION_UNLOCK(connection);
}
GDBusMessage *g_dbus_connection_send_message_with_reply_finish(GDBusConnection *connection, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  GDBusMessage *reply;
  GCancellable *cancellable;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  reply = NULL;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_dbus_connection_send_message_with_reply);
  if (g_simple_async_result_propagate_error(simple, error)) goto out;
  reply = g_object_ref(g_simple_async_result_get_op_res_gpointer(simple));
  cancellable = g_object_get_data(G_OBJECT(simple), "cancellable");
  if (cancellable != NULL && g_cancellable_is_cancelled(cancellable)) {
      g_object_unref(reply);
      reply = NULL;
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_CANCELLED,"Operation was cancelled");
  }
out:
  return reply;
}
typedef struct {
  GAsyncResult *res;
  GMainContext *context;
  GMainLoop *loop;
} SendMessageSyncData;
static void send_message_with_reply_sync_cb(GDBusConnection *connection, GAsyncResult *res, gpointer user_data) {
  SendMessageSyncData *data = user_data;
  data->res = g_object_ref(res);
  g_main_loop_quit (data->loop);
}
GDBusMessage *g_dbus_connection_send_message_with_reply_sync(GDBusConnection *connection, GDBusMessage *message, GDBusSendMessageFlags flags, gint timeout_msec,
                                                             volatile guint32 *out_serial, GCancellable *cancellable, GError **error) {
  SendMessageSyncData *data;
  GDBusMessage *reply;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), NULL);
  g_return_val_if_fail(G_IS_DBUS_MESSAGE(message), NULL);
  g_return_val_if_fail((flags & G_DBUS_SEND_MESSAGE_FLAGS_PRESERVE_SERIAL) || !g_dbus_message_get_locked(message), FALSE);
  g_return_val_if_fail(timeout_msec >= 0 || timeout_msec == -1, NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  data = g_new0(SendMessageSyncData, 1);
  data->context = g_main_context_new();
  data->loop = g_main_loop_new(data->context, FALSE);
  g_main_context_push_thread_default(data->context);
  g_dbus_connection_send_message_with_reply(connection, message, flags, timeout_msec, out_serial, cancellable, (GAsyncReadyCallback)send_message_with_reply_sync_cb,
                                            data);
  g_main_loop_run(data->loop);
  reply = g_dbus_connection_send_message_with_reply_finish(connection, data->res, error);
  g_main_context_pop_thread_default(data->context);
  g_main_context_unref(data->context);
  g_main_loop_unref(data->loop);
  g_object_unref(data->res);
  g_free(data);
  return reply;
}
typedef struct {
  GDBusMessageFilterFunction func;
  gpointer user_data;
} FilterCallback;
typedef struct {
  guint id;
  GDBusMessageFilterFunction filter_function;
  gpointer user_data;
  GDestroyNotify user_data_free_func;
} FilterData;
static void
on_worker_message_received(GDBusWorker *worker, GDBusMessage *message, gpointer user_data) {
  GDBusConnection *connection;
  FilterCallback *filters;
  gboolean consumed_by_filter;
  gboolean altered_by_filter;
  guint num_filters;
  guint n;
  gboolean alive;
  G_LOCK(message_bus_lock);
  alive = (g_hash_table_lookup(alive_connections, user_data) != NULL);
  if (!alive) {
      G_UNLOCK(message_bus_lock);
      return;
  }
  connection = G_DBUS_CONNECTION(user_data);
  g_object_ref(connection);
  G_UNLOCK(message_bus_lock);
  g_object_ref(message);
  g_dbus_message_lock(message);
  CONNECTION_LOCK(connection);
  num_filters = connection->filters->len;
  filters = g_new0(FilterCallback, num_filters);
  for (n = 0; n < num_filters; n++) {
      FilterData *data = connection->filters->pdata[n];
      filters[n].func = data->filter_function;
      filters[n].user_data = data->user_data;
    }
  CONNECTION_UNLOCK(connection);
  consumed_by_filter = FALSE;
  altered_by_filter = FALSE;
  for (n = 0; n < num_filters; n++) {
      message = filters[n].func(connection, message, TRUE, filters[n].user_data);
      if (message == NULL) break;
      g_dbus_message_lock(message);
  }
  if (message != NULL) {
      GDBusMessageType message_type;
      message_type = g_dbus_message_get_message_type(message);
      if (message_type == G_DBUS_MESSAGE_TYPE_METHOD_RETURN || message_type == G_DBUS_MESSAGE_TYPE_ERROR) {
          guint32 reply_serial;
          SendMessageData *send_message_data;
          reply_serial = g_dbus_message_get_reply_serial(message);
          CONNECTION_LOCK(connection);
          send_message_data = g_hash_table_lookup(connection->map_method_serial_to_send_message_data, GUINT_TO_POINTER(reply_serial));
          if (send_message_data != NULL) send_message_data_deliver_reply_unlocked(send_message_data, message);
          CONNECTION_UNLOCK(connection);
      } else if (message_type == G_DBUS_MESSAGE_TYPE_SIGNAL) {
          CONNECTION_LOCK(connection);
          distribute_signals(connection, message);
          CONNECTION_UNLOCK(connection);
      } else if (message_type == G_DBUS_MESSAGE_TYPE_METHOD_CALL) {
          CONNECTION_LOCK(connection);
          distribute_method_call(connection, message);
          CONNECTION_UNLOCK(connection);
      }
  }
  if (message != NULL) g_object_unref(message);
  g_object_unref(connection);
  g_free(filters);
}
static GDBusMessage *on_worker_message_about_to_be_sent(GDBusWorker *worker, GDBusMessage *message, gpointer user_data) {
  GDBusConnection *connection;
  FilterCallback *filters;
  guint num_filters;
  guint n;
  gboolean alive;
  G_LOCK(message_bus_lock);
  alive = (g_hash_table_lookup(alive_connections, user_data) != NULL);
  if (!alive) {
      G_UNLOCK(message_bus_lock);
      return message;
  }
  connection = G_DBUS_CONNECTION(user_data);
  g_object_ref(connection);
  G_UNLOCK(message_bus_lock);
  CONNECTION_LOCK(connection);
  num_filters = connection->filters->len;
  filters = g_new0(FilterCallback, num_filters);
  for (n = 0; n < num_filters; n++) {
      FilterData *data = connection->filters->pdata[n];
      filters[n].func = data->filter_function;
      filters[n].user_data = data->user_data;
  }
  CONNECTION_UNLOCK(connection);
  for (n = 0; n < num_filters; n++) {
      g_dbus_message_lock(message);
      message = filters[n].func(connection, message, FALSE, filters[n].user_data);
      if (message == NULL) break;
  }
  g_object_unref(connection);
  g_free(filters);
  return message;
}
static void on_worker_closed(GDBusWorker *worker, gboolean remote_peer_vanished, GError *error, gpointer user_data) {
  GDBusConnection *connection;
  gboolean alive;
  G_LOCK(message_bus_lock);
  alive = (g_hash_table_lookup (alive_connections, user_data) != NULL);
  if (!alive) {
      G_UNLOCK(message_bus_lock);
      return;
  }
  connection = G_DBUS_CONNECTION(user_data);
  g_object_ref(connection);
  G_UNLOCK(message_bus_lock);
  CONNECTION_LOCK(connection);
  if (!connection->closed) set_closed_unlocked(connection, remote_peer_vanished, error);
  CONNECTION_UNLOCK(connection);
  g_object_unref(connection);
}
static GDBusCapabilityFlags get_offered_capabilities_max(GDBusConnection *connection) {
  GDBusCapabilityFlags ret;
  ret = G_DBUS_CAPABILITY_FLAGS_NONE;
#ifndef G_OS_UNIX
  if (G_IS_UNIX_CONNECTION(connection->stream)) ret |= G_DBUS_CAPABILITY_FLAGS_UNIX_FD_PASSING;
#endif
  return ret;
}
static gboolean initable_init(GInitable *initable, GCancellable *cancellable, GError **error) {
  GDBusConnection *connection = G_DBUS_CONNECTION(initable);
  gboolean ret;
  g_mutex_lock(connection->init_lock);
  ret = FALSE;
  if (connection->is_initialized) {
      if (connection->stream != NULL) ret = TRUE;
      else g_assert(connection->initialization_error != NULL);
      goto out;
  }
  g_assert(connection->initialization_error == NULL);
  if (connection->address != NULL) {
      g_assert(connection->stream == NULL);
      if ((connection->flags & G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_SERVER) || (connection->flags & G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_ALLOW_ANONYMOUS)) {
          g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,"Unsupported flags encountered when constructing a client-side connection");
          goto out;
      }
      connection->stream = g_dbus_address_get_stream_sync(connection->address, NULL, cancellable, &connection->initialization_error);
      if (connection->stream == NULL) goto out;
  } else if (connection->stream != NULL);
  else { g_assert_not_reached(); }
  if (connection->flags & G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_SERVER) {
      g_assert(!(connection->flags & G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT));
      g_assert(connection->guid != NULL);
      connection->auth = _g_dbus_auth_new(connection->stream);
      if (!_g_dbus_auth_run_server(connection->auth, connection->authentication_observer, connection->guid, (connection->flags &
          G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_ALLOW_ANONYMOUS), get_offered_capabilities_max(connection), &connection->capabilities, &connection->credentials,
          cancellable, &connection->initialization_error)) {
          goto out;
      }
  } else if (connection->flags & G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT) {
      g_assert(!(connection->flags & G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_SERVER));
      g_assert(connection->guid == NULL);
      connection->auth = _g_dbus_auth_new(connection->stream);
      connection->guid = _g_dbus_auth_run_client(connection->auth, get_offered_capabilities_max(connection), &connection->capabilities, cancellable,
                                                 &connection->initialization_error);
      if (connection->guid == NULL) goto out;
  }
  if (connection->authentication_observer != NULL) {
      g_object_unref(connection->authentication_observer);
      connection->authentication_observer = NULL;
  }
#ifndef G_OS_UNIX
  if (G_IS_SOCKET_CONNECTION(connection->stream))g_socket_set_blocking(g_socket_connection_get_socket(G_SOCKET_CONNECTION(connection->stream)), FALSE);
#endif
  G_LOCK(message_bus_lock);
  if (alive_connections == NULL) alive_connections = g_hash_table_new(g_direct_hash, g_direct_equal);
  g_hash_table_insert(alive_connections, connection, connection);
  G_UNLOCK(message_bus_lock);
  connection->worker = _g_dbus_worker_new(connection->stream, connection->capabilities, (connection->flags & G_DBUS_CONNECTION_FLAGS_DELAY_MESSAGE_PROCESSING),
                                          on_worker_message_received, on_worker_message_about_to_be_sent, on_worker_closed, connection);
  if (connection->flags & G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION) {
      GVariant *hello_result;
      if (connection->flags & G_DBUS_CONNECTION_FLAGS_DELAY_MESSAGE_PROCESSING) {
          g_set_error_literal(&connection->initialization_error, G_IO_ERROR, G_IO_ERROR_FAILED, "Cannot use DELAY_MESSAGE_PROCESSING with MESSAGE_BUS_CONNECTION");
          goto out;
      }
      hello_result = g_dbus_connection_call_sync(connection, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "Hello", NULL,
                                                  G_VARIANT_TYPE ("(s)"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &connection->initialization_error);
      if (hello_result == NULL) goto out;
      g_variant_get(hello_result, "(s)", &connection->bus_unique_name);
      g_variant_unref(hello_result);
  }
  connection->is_initialized = TRUE;
  ret = TRUE;
 out:
  if (!ret) {
      g_assert(connection->initialization_error != NULL);
      g_propagate_error(error, g_error_copy(connection->initialization_error));
  }
  g_mutex_unlock(connection->init_lock);
  return ret;
}
static void initable_iface_init(GInitableIface *initable_iface) {
  initable_iface->init = initable_init;
}
static void async_initable_iface_init (GAsyncInitableIface *async_initable_iface) {}
void g_dbus_connection_new(GIOStream *stream, const gchar *guid, GDBusConnectionFlags flags, GDBusAuthObserver *observer, GCancellable *cancellable,
                           GAsyncReadyCallback callback, gpointer user_data) {
  g_return_if_fail(G_IS_IO_STREAM(stream));
  g_async_initable_new_async(G_TYPE_DBUS_CONNECTION, G_PRIORITY_DEFAULT, cancellable, callback, user_data, "stream", stream, "guid", guid,
                             "flags", flags, "authentication-observer", observer, NULL);
}
GDBusConnection *g_dbus_connection_new_finish(GAsyncResult *res, GError **error) {
  GObject *object;
  GObject *source_object;
  g_return_val_if_fail (G_IS_ASYNC_RESULT (res), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  source_object = g_async_result_get_source_object (res);
  g_assert (source_object != NULL);
  object = g_async_initable_new_finish (G_ASYNC_INITABLE (source_object), res, error);
  g_object_unref (source_object);
  if (object != NULL) return G_DBUS_CONNECTION (object);
  else return NULL;
}
GDBusConnection *g_dbus_connection_new_sync(GIOStream *stream, const gchar *guid, GDBusConnectionFlags flags, GDBusAuthObserver *observer,
                                            GCancellable *cancellable, GError **error) {
  g_return_val_if_fail(G_IS_IO_STREAM(stream), NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  return g_initable_new(G_TYPE_DBUS_CONNECTION, cancellable, error,"stream", stream, "guid", guid, "flags", flags, "authentication-observer",
                        observer, NULL);
}
void g_dbus_connection_new_for_address(const gchar *address, GDBusConnectionFlags flags, GDBusAuthObserver *observer, GCancellable *cancellable,
                                       GAsyncReadyCallback callback, gpointer user_data) {
  g_return_if_fail(address != NULL);
  g_async_initable_new_async(G_TYPE_DBUS_CONNECTION, G_PRIORITY_DEFAULT, cancellable, callback, user_data,"address", address, "flags", flags,
                             "authentication-observer", observer, NULL);
}
GDBusConnection *g_dbus_connection_new_for_address_finish(GAsyncResult *res, GError **error) {
  GObject *object;
  GObject *source_object;
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  source_object = g_async_result_get_source_object(res);
  g_assert(source_object != NULL);
  object = g_async_initable_new_finish(G_ASYNC_INITABLE(source_object), res, error);
  g_object_unref(source_object);
  if (object != NULL) return G_DBUS_CONNECTION(object);
  else return NULL;
}
GDBusConnection *g_dbus_connection_new_for_address_sync(const gchar *address, GDBusConnectionFlags flags, GDBusAuthObserver *observer, GCancellable *cancellable,
                                                        GError **error) {
  g_return_val_if_fail(address != NULL, NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  return g_initable_new(G_TYPE_DBUS_CONNECTION, cancellable, error,"address", address, "flags", flags, "authentication-observer", observer,
                        NULL);
}
void g_dbus_connection_set_exit_on_close(GDBusConnection *connection, gboolean exit_on_close) {
  g_return_if_fail(G_IS_DBUS_CONNECTION(connection));
  connection->exit_on_close = exit_on_close;
}
gboolean g_dbus_connection_get_exit_on_close(GDBusConnection *connection) {
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), FALSE);
  return connection->exit_on_close;
}
const gchar *g_dbus_connection_get_guid(GDBusConnection *connection) {
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), NULL);
  return connection->guid;
}
const gchar *g_dbus_connection_get_unique_name(GDBusConnection *connection) {
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), NULL);
  return connection->bus_unique_name;
}
GCredentials * g_dbus_connection_get_peer_credentials(GDBusConnection *connection) {
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), NULL);
  return connection->credentials;
}
static guint _global_filter_id = 1;
guint g_dbus_connection_add_filter(GDBusConnection *connection, GDBusMessageFilterFunction filter_function, gpointer user_data,
                                   GDestroyNotify user_data_free_func) {
  FilterData *data;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), 0);
  g_return_val_if_fail(filter_function != NULL, 0);
  CONNECTION_LOCK(connection);
  data = g_new0(FilterData, 1);
  data->id = _global_filter_id++;
  data->filter_function = filter_function;
  data->user_data = user_data;
  data->user_data_free_func = user_data_free_func;
  g_ptr_array_add(connection->filters, data);
  CONNECTION_UNLOCK(connection);
  return data->id;
}
static void purge_all_filters(GDBusConnection *connection) {
  guint n;
  for (n = 0; n < connection->filters->len; n++) {
      FilterData *data = connection->filters->pdata[n];
      if (data->user_data_free_func != NULL) data->user_data_free_func(data->user_data);
      g_free(data);
  }
}
void g_dbus_connection_remove_filter(GDBusConnection *connection, guint filter_id) {
  guint n;
  FilterData *to_destroy;
  g_return_if_fail(G_IS_DBUS_CONNECTION(connection));
  CONNECTION_LOCK(connection);
  to_destroy = NULL;
  for (n = 0; n < connection->filters->len; n++) {
      FilterData *data = connection->filters->pdata[n];
      if (data->id == filter_id) {
          g_ptr_array_remove_index(connection->filters, n);
          to_destroy = data;
          break;
      }
  }
  CONNECTION_UNLOCK(connection);
  if (to_destroy != NULL) {
      if (to_destroy->user_data_free_func != NULL) to_destroy->user_data_free_func(to_destroy->user_data);
      g_free(to_destroy);
  } else g_warning("g_dbus_connection_remove_filter: No filter found for filter_id %d", filter_id);
}
typedef struct {
  gchar *rule;
  gchar *sender;
  gchar *sender_unique_name;
  gchar *interface_name;
  gchar *member;
  gchar *object_path;
  gchar *arg0;
  GArray *subscribers;
} SignalData;
typedef struct {
  GDBusSignalCallback callback;
  gpointer user_data;
  GDestroyNotify user_data_free_func;
  guint id;
  GMainContext *context;
} SignalSubscriber;
static void signal_data_free(SignalData *signal_data) {
  g_free(signal_data->rule);
  g_free(signal_data->sender);
  g_free(signal_data->sender_unique_name);
  g_free(signal_data->interface_name);
  g_free(signal_data->member);
  g_free(signal_data->object_path);
  g_free(signal_data->arg0);
  g_array_free(signal_data->subscribers, TRUE);
  g_free(signal_data);
}
static gchar *args_to_rule(const gchar *sender, const gchar *interface_name, const gchar *member, const gchar *object_path, const gchar *arg0, gboolean negate) {
  GString *rule;
  rule = g_string_new("type='signal'");
  if (negate) g_string_prepend_c(rule, '-');
  if (sender != NULL) g_string_append_printf(rule, ",sender='%s'", sender);
  if (interface_name != NULL) g_string_append_printf(rule, ",interface='%s'", interface_name);
  if (member != NULL) g_string_append_printf(rule, ",member='%s'", member);
  if (object_path != NULL) g_string_append_printf(rule, ",path='%s'", object_path);
  if (arg0 != NULL) g_string_append_printf(rule, ",arg0='%s'", arg0);
  return g_string_free(rule, FALSE);
}
static guint _global_subscriber_id = 1;
static guint _global_registration_id = 1;
static guint _global_subtree_registration_id = 1;
static void add_match_rule(GDBusConnection *connection, const gchar *match_rule) {
  GError *error;
  GDBusMessage *message;
  if (match_rule[0] == '-') return;
  message = g_dbus_message_new_method_call("org.freedesktop.DBus","/org/freedesktop/DBus","org.freedesktop.DBus","AddMatch");
  g_dbus_message_set_body(message, g_variant_new("(s)", match_rule));
  error = NULL;
  if (!g_dbus_connection_send_message_unlocked(connection, message,G_DBUS_SEND_MESSAGE_FLAGS_NONE,NULL, &error)) {
      g_critical("Error while sending AddMatch() message: %s", error->message);
      g_error_free(error);
  }
  g_object_unref(message);
}
static void remove_match_rule(GDBusConnection *connection, const gchar *match_rule) {
  GError *error;
  GDBusMessage *message;
  if (match_rule[0] == '-') return;
  message = g_dbus_message_new_method_call("org.freedesktop.DBus","/org/freedesktop/DBus","org.freedesktop.DBus","RemoveMatch");
  g_dbus_message_set_body(message, g_variant_new("(s)", match_rule));
  error = NULL;
  if (!g_dbus_connection_send_message_unlocked(connection, message,G_DBUS_SEND_MESSAGE_FLAGS_NONE,NULL, &error)) {
      g_critical("Error while sending RemoveMatch() message: %s", error->message);
      g_error_free(error);
  }
  g_object_unref(message);
}
static gboolean is_signal_data_for_name_lost_or_acquired(SignalData *signal_data) {
  return g_strcmp0(signal_data->sender_unique_name, "org.freedesktop.DBus") == 0 && g_strcmp0(signal_data->interface_name, "org.freedesktop.DBus") == 0 &&
         g_strcmp0(signal_data->object_path, "/org/freedesktop/DBus") == 0 && (g_strcmp0(signal_data->member, "NameLost") == 0 ||
         g_strcmp0(signal_data->member, "NameAcquired") == 0);
}
guint g_dbus_connection_signal_subscribe(GDBusConnection *connection, const gchar *sender, const gchar *interface_name, const gchar *member,
                                         const gchar *object_path, const gchar *arg0, GDBusSignalFlags flags, GDBusSignalCallback callback,
                                         gpointer user_data, GDestroyNotify user_data_free_func) {
  gchar *rule;
  SignalData *signal_data;
  SignalSubscriber subscriber;
  GPtrArray *signal_data_array;
  const gchar *sender_unique_name;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), 0);
  g_return_val_if_fail(sender == NULL || (g_dbus_is_name(sender) && (connection->flags & G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION)), 0);
  g_return_val_if_fail(interface_name == NULL || g_dbus_is_interface_name(interface_name), 0);
  g_return_val_if_fail(member == NULL || g_dbus_is_member_name(member), 0);
  g_return_val_if_fail(object_path == NULL || g_variant_is_object_path(object_path), 0);
  g_return_val_if_fail(callback != NULL, 0);
  CONNECTION_LOCK(connection);
  rule = args_to_rule(sender, interface_name, member, object_path, arg0,(flags & G_DBUS_SIGNAL_FLAGS_NO_MATCH_RULE) != 0);
  if (sender != NULL && (g_dbus_is_unique_name(sender) || g_strcmp0(sender, "org.freedesktop.DBus") == 0)) sender_unique_name = sender;
  else sender_unique_name = "";
  subscriber.callback = callback;
  subscriber.user_data = user_data;
  subscriber.user_data_free_func = user_data_free_func;
  subscriber.id = _global_subscriber_id++;
  subscriber.context = g_main_context_get_thread_default();
  if (subscriber.context != NULL) g_main_context_ref(subscriber.context);
  signal_data = g_hash_table_lookup(connection->map_rule_to_signal_data, rule);
  if (signal_data != NULL) {
      g_array_append_val(signal_data->subscribers, subscriber);
      g_free(rule);
      goto out;
  }
  signal_data = g_new0(SignalData,1);
  signal_data->rule = rule;
  signal_data->sender = g_strdup(sender);
  signal_data->sender_unique_name = g_strdup(sender_unique_name);
  signal_data->interface_name = g_strdup(interface_name);
  signal_data->member = g_strdup(member);
  signal_data->object_path = g_strdup(object_path);
  signal_data->arg0 = g_strdup(arg0);
  signal_data->subscribers = g_array_new(FALSE,FALSE, sizeof(SignalSubscriber));
  g_array_append_val(signal_data->subscribers, subscriber);
  g_hash_table_insert(connection->map_rule_to_signal_data, signal_data->rule, signal_data);
  if (connection->flags & G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION) {
      if (!is_signal_data_for_name_lost_or_acquired(signal_data)) add_match_rule(connection, signal_data->rule);
  }
  signal_data_array = g_hash_table_lookup(connection->map_sender_unique_name_to_signal_data_array, signal_data->sender_unique_name);
  if (signal_data_array == NULL) {
      signal_data_array = g_ptr_array_new();
      g_hash_table_insert(connection->map_sender_unique_name_to_signal_data_array, g_strdup(signal_data->sender_unique_name), signal_data_array);
  }
  g_ptr_array_add(signal_data_array, signal_data);
out:
  g_hash_table_insert(connection->map_id_to_signal_data, GUINT_TO_POINTER(subscriber.id), signal_data);
  CONNECTION_UNLOCK(connection);
  return subscriber.id;
}
static void unsubscribe_id_internal(GDBusConnection *connection, guint subscription_id, GArray *out_removed_subscribers) {
  SignalData *signal_data;
  GPtrArray *signal_data_array;
  guint n;
  signal_data = g_hash_table_lookup(connection->map_id_to_signal_data, GUINT_TO_POINTER(subscription_id));
  if (signal_data == NULL) return;
  for (n = 0; n < signal_data->subscribers->len; n++) {
      SignalSubscriber *subscriber;
      subscriber = &(g_array_index(signal_data->subscribers, SignalSubscriber, n));
      if (subscriber->id != subscription_id) continue;
      g_warn_if_fail(g_hash_table_remove(connection->map_id_to_signal_data, GUINT_TO_POINTER(subscription_id)));
      g_array_append_val(out_removed_subscribers, *subscriber);
      g_array_remove_index(signal_data->subscribers, n);
      if (signal_data->subscribers->len == 0) {
          g_warn_if_fail(g_hash_table_remove(connection->map_rule_to_signal_data, signal_data->rule));
          signal_data_array = g_hash_table_lookup(connection->map_sender_unique_name_to_signal_data_array, signal_data->sender_unique_name);
          g_warn_if_fail(signal_data_array != NULL);
          g_warn_if_fail(g_ptr_array_remove(signal_data_array, signal_data));
          if (signal_data_array->len == 0) {
              g_warn_if_fail(g_hash_table_remove(connection->map_sender_unique_name_to_signal_data_array, signal_data->sender_unique_name));
          }
          if (connection->flags & G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION) {
              if (!is_signal_data_for_name_lost_or_acquired(signal_data))
                if (!connection->closed && !connection->finalizing) remove_match_rule(connection, signal_data->rule);
          }
          signal_data_free(signal_data);
      }
      return;
  }
  g_assert_not_reached();
}
void g_dbus_connection_signal_unsubscribe(GDBusConnection *connection, guint subscription_id) {
  GArray *subscribers;
  guint n;
  g_return_if_fail(G_IS_DBUS_CONNECTION(connection));
  subscribers = g_array_new(FALSE, FALSE, sizeof(SignalSubscriber));
  CONNECTION_LOCK(connection);
  unsubscribe_id_internal(connection, subscription_id, subscribers);
  CONNECTION_UNLOCK(connection);
  g_assert(subscribers->len == 0 || subscribers->len == 1);
  for (n = 0; n < subscribers->len; n++) {
      SignalSubscriber *subscriber;
      subscriber = &(g_array_index(subscribers, SignalSubscriber, n));
      call_destroy_notify(subscriber->context, subscriber->user_data_free_func, subscriber->user_data);
      if (subscriber->context != NULL) g_main_context_unref(subscriber->context);
  }
  g_array_free(subscribers, TRUE);
}
typedef struct {
  guint subscription_id;
  GDBusSignalCallback callback;
  gpointer user_data;
  GDBusMessage *message;
  GDBusConnection *connection;
  const gchar *sender;
  const gchar *path;
  const gchar *interface;
  const gchar *member;
} SignalInstance;
static gboolean emit_signal_instance_in_idle_cb(gpointer data) {
  SignalInstance *signal_instance = data;
  GVariant *parameters;
  gboolean has_subscription;
  parameters = g_dbus_message_get_body(signal_instance->message);
  if (parameters == NULL) {
      parameters = g_variant_new("()");
      g_variant_ref_sink(parameters);
  } else g_variant_ref_sink(parameters);
#if 0
  g_print("in emit_signal_instance_in_idle_cb(id=%d sender=%s path=%s interface=%s member=%s params=%s)\n", signal_instance->subscription_id,
          signal_instance->sender, signal_instance->path, signal_instance->interface, signal_instance->member, g_variant_print(parameters, TRUE));
#endif
  CONNECTION_LOCK(signal_instance->connection);
  has_subscription = FALSE;
  if (g_hash_table_lookup(signal_instance->connection->map_id_to_signal_data, GUINT_TO_POINTER(signal_instance->subscription_id)) != NULL) has_subscription = TRUE;
  CONNECTION_UNLOCK(signal_instance->connection);
  if (has_subscription) {
      signal_instance->callback(signal_instance->connection, signal_instance->sender, signal_instance->path, signal_instance->interface, signal_instance->member,
                                parameters, signal_instance->user_data);
  }
  if (parameters != NULL) g_variant_unref(parameters);
  return FALSE;
}
static void signal_instance_free(SignalInstance *signal_instance) {
  g_object_unref(signal_instance->message);
  g_object_unref(signal_instance->connection);
  g_free(signal_instance);
}
static void schedule_callbacks(GDBusConnection *connection, GPtrArray *signal_data_array, GDBusMessage *message, const gchar *sender) {
  guint n, m;
  const gchar *interface;
  const gchar *member;
  const gchar *path;
  const gchar *arg0;
  interface = NULL;
  member = NULL;
  path = NULL;
  arg0 = NULL;
  interface = g_dbus_message_get_interface (message);
  member = g_dbus_message_get_member (message);
  path = g_dbus_message_get_path (message);
  arg0 = g_dbus_message_get_arg0 (message);
#if 0
  g_print("In schedule_callbacks:\n  sender    = `%s'\n  interface = `%s'\n  member    = `%s'\n  path      = `%s'\n  arg0      = `%s'\n", sender, interface, member,
          path, arg0);
#endif
  for (n = 0; n < signal_data_array->len; n++) {
      SignalData *signal_data = signal_data_array->pdata[n];
      if (signal_data->interface_name != NULL && g_strcmp0(signal_data->interface_name, interface) != 0) continue;
      if (signal_data->member != NULL && g_strcmp0(signal_data->member, member) != 0) continue;
      if (signal_data->object_path != NULL && g_strcmp0(signal_data->object_path, path) != 0) continue;
      if (signal_data->arg0 != NULL && g_strcmp0 (signal_data->arg0, arg0) != 0) continue;
      for (m = 0; m < signal_data->subscribers->len; m++) {
          SignalSubscriber *subscriber;
          GSource *idle_source;
          SignalInstance *signal_instance;
          subscriber = &(g_array_index(signal_data->subscribers, SignalSubscriber, m));
          signal_instance = g_new0(SignalInstance, 1);
          signal_instance->subscription_id = subscriber->id;
          signal_instance->callback = subscriber->callback;
          signal_instance->user_data = subscriber->user_data;
          signal_instance->message = g_object_ref (message);
          signal_instance->connection = g_object_ref(connection);
          signal_instance->sender = sender;
          signal_instance->path = path;
          signal_instance->interface = interface;
          signal_instance->member = member;
          idle_source = g_idle_source_new();
          g_source_set_priority(idle_source, G_PRIORITY_DEFAULT);
          g_source_set_callback(idle_source, emit_signal_instance_in_idle_cb, signal_instance, (GDestroyNotify)signal_instance_free);
          g_source_attach (idle_source, subscriber->context);
          g_source_unref (idle_source);
      }
  }
}
static void distribute_signals(GDBusConnection *connection, GDBusMessage *message) {
  GPtrArray *signal_data_array;
  const gchar *sender;
  sender = g_dbus_message_get_sender(message);
  if (G_UNLIKELY(_g_dbus_debug_signal())) {
      _g_dbus_debug_print_lock();
      g_print("========================================================================\nGDBus-debug:Signal:\n <<<< RECEIVED SIGNAL %s.%s\n"
              "      on object %s\n      sent by name %s\n", g_dbus_message_get_interface(message), g_dbus_message_get_member(message),
              g_dbus_message_get_path(message), sender != NULL ? sender : "(none)");
      _g_dbus_debug_print_unlock();
  }
  if (sender != NULL) {
      signal_data_array = g_hash_table_lookup(connection->map_sender_unique_name_to_signal_data_array, sender);
      if (signal_data_array != NULL) schedule_callbacks(connection, signal_data_array, message, sender);
  }
  signal_data_array = g_hash_table_lookup(connection->map_sender_unique_name_to_signal_data_array, "");
  if (signal_data_array != NULL) schedule_callbacks(connection, signal_data_array, message, sender);
}
static void purge_all_signal_subscriptions(GDBusConnection *connection) {
  GHashTableIter iter;
  gpointer key;
  GArray *ids;
  GArray *subscribers;
  guint n;
  ids = g_array_new(FALSE, FALSE, sizeof(guint));
  g_hash_table_iter_init(&iter, connection->map_id_to_signal_data);
  while(g_hash_table_iter_next(&iter, &key, NULL)) {
      guint subscription_id = GPOINTER_TO_UINT(key);
      g_array_append_val(ids, subscription_id);
  }
  subscribers = g_array_new(FALSE, FALSE, sizeof(SignalSubscriber));
  for (n = 0; n < ids->len; n++) {
      guint subscription_id = g_array_index(ids, guint, n);
      unsubscribe_id_internal(connection, subscription_id, subscribers);
  }
  g_array_free(ids, TRUE);
  for (n = 0; n < subscribers->len; n++) {
      SignalSubscriber *subscriber;
      subscriber = &(g_array_index(subscribers, SignalSubscriber, n));
      call_destroy_notify(subscriber->context, subscriber->user_data_free_func, subscriber->user_data);
      if (subscriber->context != NULL) g_main_context_unref(subscriber->context);
  }
  g_array_free(subscribers, TRUE);
}
static GDBusInterfaceVTable *_g_dbus_interface_vtable_copy(const GDBusInterfaceVTable *vtable) {
  return g_memdup((gconstpointer)vtable, 3 * sizeof(gpointer));
}
static void _g_dbus_interface_vtable_free(GDBusInterfaceVTable *vtable) {
  g_free(vtable);
}
static GDBusSubtreeVTable *_g_dbus_subtree_vtable_copy(const GDBusSubtreeVTable *vtable) {
  return g_memdup((gconstpointer)vtable, 3 * sizeof(gpointer));
}
static void _g_dbus_subtree_vtable_free(GDBusSubtreeVTable *vtable) {
  g_free (vtable);
}
struct ExportedObject {
  gchar *object_path;
  GDBusConnection *connection;
  GHashTable *map_if_name_to_ei;
};
static void exported_object_free(ExportedObject *eo) {
  g_free(eo->object_path);
  g_hash_table_unref(eo->map_if_name_to_ei);
  g_free(eo);
}
typedef struct {
  ExportedObject *eo;
  guint id;
  gchar *interface_name;
  GDBusInterfaceVTable *vtable;
  GDBusInterfaceInfo *interface_info;
  GMainContext *context;
  gpointer user_data;
  GDestroyNotify user_data_free_func;
} ExportedInterface;
static void exported_interface_free(ExportedInterface *ei) {
  g_dbus_interface_info_unref((GDBusInterfaceInfo*)ei->interface_info);
  call_destroy_notify(ei->context, ei->user_data_free_func, ei->user_data);
  if (ei->context != NULL) g_main_context_unref(ei->context);
  g_free(ei->interface_name);
  _g_dbus_interface_vtable_free(ei->vtable);
  g_free(ei);
}
static gboolean has_object_been_unregistered(GDBusConnection *connection, guint registration_id, guint subtree_registration_id) {
  gboolean ret;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), FALSE);
  ret = FALSE;
  CONNECTION_LOCK(connection);
  if (registration_id != 0 && g_hash_table_lookup(connection->map_id_to_ei, GUINT_TO_POINTER(registration_id)) == NULL) ret = TRUE;
  else if (subtree_registration_id != 0 && g_hash_table_lookup(connection->map_id_to_es, GUINT_TO_POINTER(subtree_registration_id)) == NULL) ret = TRUE;
  CONNECTION_UNLOCK(connection);
  return ret;
}
typedef struct {
  GDBusConnection *connection;
  GDBusMessage *message;
  gpointer user_data;
  const gchar *property_name;
  const GDBusInterfaceVTable *vtable;
  GDBusInterfaceInfo *interface_info;
  const GDBusPropertyInfo *property_info;
  guint registration_id;
  guint subtree_registration_id;
} PropertyData;
static void property_data_free(PropertyData *data) {
  g_object_unref(data->connection);
  g_object_unref(data->message);
  g_free(data);
}
static gboolean invoke_get_property_in_idle_cb(gpointer _data) {
  PropertyData *data = _data;
  GVariant *value;
  GError *error;
  GDBusMessage *reply;
  if (has_object_been_unregistered(data->connection, data->registration_id, data->subtree_registration_id)) {
      reply = g_dbus_message_new_method_error(data->message,"org.freedesktop.DBus.Error.UnknownMethod","No such interface `org.freedesktop."
                                              "DBus.Properties' on object at path %s", g_dbus_message_get_path(data->message));
      g_dbus_connection_send_message(data->connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
      g_object_unref(reply);
      goto out;
  }
  error = NULL;
  value = data->vtable->get_property(data->connection, g_dbus_message_get_sender(data->message), g_dbus_message_get_path(data->message), data->interface_info->name,
                                     data->property_name, &error, data->user_data);
  if (value != NULL) {
      g_assert_no_error(error);
      if (g_variant_is_floating(value)) g_variant_ref_sink(value);
      reply = g_dbus_message_new_method_reply(data->message);
      g_dbus_message_set_body(reply, g_variant_new("(v)", value));
      g_dbus_connection_send_message(data->connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
      g_variant_unref(value);
      g_object_unref(reply);
  } else {
      gchar *dbus_error_name;
      g_assert(error != NULL);
      dbus_error_name = g_dbus_error_encode_gerror(error);
      reply = g_dbus_message_new_method_error_literal(data->message, dbus_error_name, error->message);
      g_dbus_connection_send_message(data->connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
      g_free(dbus_error_name);
      g_error_free(error);
      g_object_unref(reply);
  }
out:
  return FALSE;
}
static gboolean invoke_set_property_in_idle_cb(gpointer _data) {
  PropertyData *data = _data;
  GError *error;
  GDBusMessage *reply;
  GVariant *value;
  error = NULL;
  value = NULL;
  g_variant_get(g_dbus_message_get_body(data->message),"(ssv)", NULL, NULL, &value);
  if (g_strcmp0(g_variant_get_type_string(value), data->property_info->signature) != 0) {
      reply = g_dbus_message_new_method_error(data->message,"org.freedesktop.DBus.Error.InvalidArgs","Error setting property `%s': Expected"
                                              " type `%s' but got `%s'", data->property_info->name, data->property_info->signature,
                                              g_variant_get_type_string(value));
      goto out;
  }
  if (!data->vtable->set_property(data->connection, g_dbus_message_get_sender(data->message), g_dbus_message_get_path(data->message), data->interface_info->name,
                                  data->property_name, value, &error, data->user_data)) {
      gchar *dbus_error_name;
      g_assert(error != NULL);
      dbus_error_name = g_dbus_error_encode_gerror(error);
      reply = g_dbus_message_new_method_error_literal(data->message, dbus_error_name, error->message);
      g_free(dbus_error_name);
      g_error_free(error);
  } else reply = g_dbus_message_new_method_reply(data->message);
out:
  g_assert(reply != NULL);
  g_dbus_connection_send_message(data->connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
  g_object_unref(reply);
  g_variant_unref(value);
  return FALSE;
}
static gboolean validate_and_maybe_schedule_property_getset(GDBusConnection *connection, GDBusMessage *message, guint registration_id, guint  subtree_registration_id,
                                                            gboolean is_get, GDBusInterfaceInfo *interface_info, const GDBusInterfaceVTable *vtable,
                                                            GMainContext *main_context, gpointer user_data) {
  gboolean handled;
  const char *interface_name;
  const char *property_name;
  const GDBusPropertyInfo *property_info;
  GSource *idle_source;
  PropertyData *property_data;
  GDBusMessage *reply;
  handled = FALSE;
  if (is_get) g_variant_get(g_dbus_message_get_body(message),"(&s&s)", &interface_name, &property_name);
  else g_variant_get(g_dbus_message_get_body(message),"(&s&sv)", &interface_name, &property_name, NULL);
  if (is_get) {
      if (vtable == NULL || vtable->get_property == NULL) goto out;
  } else {
      if (vtable == NULL || vtable->set_property == NULL) goto out;
  }
  property_info = NULL;
  property_info = g_dbus_interface_info_lookup_property(interface_info, property_name);
  if (property_info == NULL) {
      reply = g_dbus_message_new_method_error(message,"org.freedesktop.DBus.Error.InvalidArgs","No such property `%s'", property_name);
      g_dbus_connection_send_message_unlocked(connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
      g_object_unref(reply);
      handled = TRUE;
      goto out;
  }
  if (is_get && !(property_info->flags & G_DBUS_PROPERTY_INFO_FLAGS_READABLE)) {
      reply = g_dbus_message_new_method_error(message,"org.freedesktop.DBus.Error.InvalidArgs","Property `%s' is not readable", property_name);
      g_dbus_connection_send_message_unlocked(connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
      g_object_unref(reply);
      handled = TRUE;
      goto out;
  } else if (!is_get && !(property_info->flags & G_DBUS_PROPERTY_INFO_FLAGS_WRITABLE)) {
      reply = g_dbus_message_new_method_error(message, "org.freedesktop.DBus.Error.InvalidArgs","Property `%s' is not writable", property_name);
      g_dbus_connection_send_message_unlocked(connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
      g_object_unref(reply);
      handled = TRUE;
      goto out;
  }
  property_data = g_new0(PropertyData, 1);
  property_data->connection = g_object_ref (connection);
  property_data->message = g_object_ref (message);
  property_data->user_data = user_data;
  property_data->property_name = property_name;
  property_data->vtable = vtable;
  property_data->interface_info = interface_info;
  property_data->property_info = property_info;
  property_data->registration_id = registration_id;
  property_data->subtree_registration_id = subtree_registration_id;
  idle_source = g_idle_source_new();
  g_source_set_priority(idle_source, G_PRIORITY_DEFAULT);
  g_source_set_callback(idle_source, is_get ? invoke_get_property_in_idle_cb : invoke_set_property_in_idle_cb, property_data, (GDestroyNotify)property_data_free);
  g_source_attach(idle_source, main_context);
  g_source_unref(idle_source);
  handled = TRUE;
out:
  return handled;
}
static gboolean handle_getset_property(GDBusConnection *connection, ExportedObject *eo, GDBusMessage *message, gboolean is_get) {
  ExportedInterface *ei;
  gboolean handled;
  const char *interface_name;
  const char *property_name;
  handled = FALSE;
  if (is_get) g_variant_get(g_dbus_message_get_body(message), "(&s&s)", &interface_name, &property_name);
  else g_variant_get(g_dbus_message_get_body(message), "(&s&sv)", &interface_name, &property_name, NULL);
  ei = g_hash_table_lookup(eo->map_if_name_to_ei, interface_name);
  if (ei == NULL) {
      GDBusMessage *reply;
      reply = g_dbus_message_new_method_error(message, "org.freedesktop.DBus.Error.InvalidArgs","No such interface `%s'", interface_name);
      g_dbus_connection_send_message_unlocked(eo->connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
      g_object_unref(reply);
      handled = TRUE;
      goto out;
  }
  handled = validate_and_maybe_schedule_property_getset(eo->connection, message, ei->id, 0, is_get, ei->interface_info, ei->vtable, ei->context, ei->user_data);
out:
  return handled;
}
typedef struct {
  GDBusConnection *connection;
  GDBusMessage *message;
  gpointer user_data;
  const GDBusInterfaceVTable *vtable;
  GDBusInterfaceInfo *interface_info;
  guint registration_id;
  guint subtree_registration_id;
} PropertyGetAllData;
static void property_get_all_data_free(PropertyData *data) {
  g_object_unref(data->connection);
  g_object_unref(data->message);
  g_free(data);
}
static gboolean invoke_get_all_properties_in_idle_cb(gpointer _data) {
  PropertyGetAllData *data = _data;
  GVariantBuilder builder;
  GError *error;
  GDBusMessage *reply;
  guint n;
  if (has_object_been_unregistered(data->connection, data->registration_id, data->subtree_registration_id)) {
      reply = g_dbus_message_new_method_error(data->message, "org.freedesktop.DBus.Error.UnknownMethod","No such interface `org.freedesktop."
                                              "DBus.Properties' on object at path %s", g_dbus_message_get_path(data->message));
      g_dbus_connection_send_message(data->connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
      g_object_unref(reply);
      goto out;
  }
  error = NULL;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("(a{sv})"));
  g_variant_builder_open(&builder, G_VARIANT_TYPE("a{sv}"));
  for (n = 0; data->interface_info->properties != NULL && data->interface_info->properties[n] != NULL; n++) {
      const GDBusPropertyInfo *property_info = data->interface_info->properties[n];
      GVariant *value;
      if (!(property_info->flags & G_DBUS_PROPERTY_INFO_FLAGS_READABLE)) continue;
      value = data->vtable->get_property(data->connection, g_dbus_message_get_sender(data->message), g_dbus_message_get_path(data->message),
                                         data->interface_info->name, property_info->name, NULL, data->user_data);
      if (value == NULL) continue;
      if (g_variant_is_floating(value)) g_variant_ref_sink(value);
      g_variant_builder_add(&builder,"{sv}", property_info->name, value);
      g_variant_unref(value);
  }
  g_variant_builder_close(&builder);
  reply = g_dbus_message_new_method_reply(data->message);
  g_dbus_message_set_body(reply, g_variant_builder_end(&builder));
  g_dbus_connection_send_message(data->connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
  g_object_unref(reply);
out:
  return FALSE;
}
static gboolean validate_and_maybe_schedule_property_get_all(GDBusConnection *connection, GDBusMessage *message, guint registration_id, guint subtree_registration_id,
                                                             GDBusInterfaceInfo *interface_info, const GDBusInterfaceVTable *vtable, GMainContext *main_context,
                                                             gpointer user_data) {
  gboolean handled;
  const char *interface_name;
  GSource *idle_source;
  PropertyGetAllData *property_get_all_data;
  handled = FALSE;
  g_variant_get(g_dbus_message_get_body(message),"(&s)", &interface_name);
  if (vtable == NULL || vtable->get_property == NULL) goto out;
  property_get_all_data = g_new0(PropertyGetAllData, 1);
  property_get_all_data->connection = g_object_ref(connection);
  property_get_all_data->message = g_object_ref(message);
  property_get_all_data->user_data = user_data;
  property_get_all_data->vtable = vtable;
  property_get_all_data->interface_info = interface_info;
  property_get_all_data->registration_id = registration_id;
  property_get_all_data->subtree_registration_id = subtree_registration_id;
  idle_source = g_idle_source_new();
  g_source_set_priority(idle_source, G_PRIORITY_DEFAULT);
  g_source_set_callback(idle_source, invoke_get_all_properties_in_idle_cb, property_get_all_data, (GDestroyNotify)property_get_all_data_free);
  g_source_attach(idle_source, main_context);
  g_source_unref(idle_source);
  handled = TRUE;
out:
  return handled;
}
static gboolean handle_get_all_properties(GDBusConnection *connection, ExportedObject *eo, GDBusMessage *message) {
  ExportedInterface *ei;
  gboolean handled;
  const char *interface_name;
  handled = FALSE;
  g_variant_get(g_dbus_message_get_body(message),"(&s)", &interface_name);
  ei = g_hash_table_lookup(eo->map_if_name_to_ei, interface_name);
  if (ei == NULL) {
      GDBusMessage *reply;
      reply = g_dbus_message_new_method_error(message,"org.freedesktop.DBus.Error.InvalidArgs","No such interface", interface_name);
      g_dbus_connection_send_message_unlocked(eo->connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
      g_object_unref(reply);
      handled = TRUE;
      goto out;
  }
  handled = validate_and_maybe_schedule_property_get_all(eo->connection, message, ei->id,0, ei->interface_info, ei->vtable, ei->context,
                                                         ei->user_data);
out:
  return handled;
}
static const gchar introspect_header[] = "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n                      \""
                                         "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n<!-- GDBus " PACKAGE_VERSION " -->\n<node>\n";
static const gchar introspect_tail[] = "</node>\n";
static const gchar introspect_properties_interface[] = "  <interface name=\"org.freedesktop.DBus.Properties\">\n    <method name=\"Get\">\n      <arg type=\"s\""
                                                       " name=\"interface_name\" direction=\"in\"/>\n      <arg type=\"s\" name=\"property_name\" direction=\"in"
                                                       "\"/>\n      <arg type=\"v\" name=\"value\" direction=\"out\"/>\n    </method>\n    <method name=\"GetAll"
                                                       "\">\n      <arg type=\"s\" name=\"interface_name\" direction=\"in\"/>\n      <arg type=\"a{sv}\" "
                                                       "name=\"properties\" direction=\"out\"/>\n    </method>\n    <method name=\"Set\">\n      <arg type=\"s\""
                                                       " name=\"interface_name\" direction=\"in\"/>\n      <arg type=\"s\" name=\"property_name\" direction=\"in"
                                                       "\"/>\n      <arg type=\"v\" name=\"value\" direction=\"in\"/>\n    </method>\n    <signal "
                                                       "name=\"PropertiesChanged\">\n      <arg type=\"s\" name=\"interface_name\"/>\n      <arg type=\"a{sv}\""
                                                       " name=\"changed_properties\"/>\n      <arg type=\"as\" name=\"invalidated_properties\"/>\n    </signal>\n"
                                                       "  </interface>\n";
static const gchar introspect_introspectable_interface[] = "  <interface name=\"org.freedesktop.DBus.Introspectable\">\n    <method name=\"Introspect\">\n"
                                                           "      <arg type=\"s\" name=\"xml_data\" direction=\"out\"/>\n    </method>\n  </interface>\n"
                                                           "  <interface name=\"org.freedesktop.DBus.Peer\">\n    <method name=\"Ping\"/>\n""    <method "
                                                           "name=\"GetMachineId\">\n      <arg type=\"s\" name=\"machine_uuid\" direction=\"out\"/>\n    "
                                                           "</method>\n  </interface>\n";
static void introspect_append_header(GString *s) {
  g_string_append(s, introspect_header);
}
static void maybe_add_path(const gchar *path, gsize path_len, const gchar *object_path, GHashTable *set) {
  if (g_str_has_prefix(object_path, path) && strlen(object_path) > path_len && object_path[path_len-1] == '/') {
      const gchar *begin;
      const gchar *end;
      gchar *s;
      begin = object_path + path_len;
      end = strchr(begin, '/');
      if (end != NULL) s = g_strndup(begin, end - begin);
      else s = g_strdup(begin);
      if (g_hash_table_lookup(set, s) == NULL) g_hash_table_insert(set, s, GUINT_TO_POINTER(1));
      else g_free(s);
  }
}
static gchar **g_dbus_connection_list_registered_unlocked(GDBusConnection *connection, const gchar *path) {
  GPtrArray *p;
  gchar **ret;
  GHashTableIter hash_iter;
  const gchar *object_path;
  gsize path_len;
  GHashTable *set;
  GList *keys;
  GList *l;
  CONNECTION_ENSURE_LOCK(connection);
  path_len = strlen(path);
  if (path_len > 1) path_len++;
  set = g_hash_table_new(g_str_hash, g_str_equal);
  g_hash_table_iter_init(&hash_iter, connection->map_object_path_to_eo);
  while(g_hash_table_iter_next(&hash_iter, (gpointer)&object_path, NULL)) maybe_add_path(path, path_len, object_path, set);
  g_hash_table_iter_init(&hash_iter, connection->map_object_path_to_es);
  while (g_hash_table_iter_next(&hash_iter, (gpointer)&object_path, NULL)) maybe_add_path(path, path_len, object_path, set);
  p = g_ptr_array_new();
  keys = g_hash_table_get_keys(set);
  for (l = keys; l != NULL; l = l->next) g_ptr_array_add(p, l->data);
  g_hash_table_unref(set);
  g_list_free(keys);
  g_ptr_array_add(p, NULL);
  ret = (gchar**)g_ptr_array_free(p, FALSE);
  return ret;
}
static gchar **g_dbus_connection_list_registered(GDBusConnection *connection, const gchar *path) {
  gchar **ret;
  CONNECTION_LOCK(connection);
  ret = g_dbus_connection_list_registered_unlocked(connection, path);
  CONNECTION_UNLOCK(connection);
  return ret;
}
static gboolean handle_introspect(GDBusConnection *connection, ExportedObject *eo, GDBusMessage *message) {
  guint n;
  GString *s;
  GDBusMessage *reply;
  GHashTableIter hash_iter;
  ExportedInterface *ei;
  gchar **registered;
  s = g_string_sized_new(sizeof(introspect_header) + sizeof(introspect_properties_interface) + sizeof(introspect_introspectable_interface) +
                         sizeof(introspect_tail));
  introspect_append_header(s);
  if (!g_hash_table_lookup(eo->map_if_name_to_ei,"org.freedesktop.DBus.Properties")) g_string_append(s, introspect_properties_interface);
  if (!g_hash_table_lookup(eo->map_if_name_to_ei, "org.freedesktop.DBus.Introspectable")) g_string_append(s, introspect_introspectable_interface);
  g_hash_table_iter_init(&hash_iter, eo->map_if_name_to_ei);
  while(g_hash_table_iter_next(&hash_iter, NULL, (gpointer)&ei)) g_dbus_interface_info_generate_xml(ei->interface_info, 2, s);
  registered = g_dbus_connection_list_registered_unlocked(connection, eo->object_path);
  for (n = 0; registered != NULL && registered[n] != NULL; n++) g_string_append_printf(s, "  <node name=\"%s\"/>\n", registered[n]);
  g_strfreev(registered);
  g_string_append(s, introspect_tail);
  reply = g_dbus_message_new_method_reply(message);
  g_dbus_message_set_body(reply, g_variant_new("(s)", s->str));
  g_dbus_connection_send_message_unlocked(connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
  g_object_unref(reply);
  g_string_free(s, TRUE);
  return TRUE;
}
static gboolean call_in_idle_cb(gpointer user_data) {
  GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(user_data);
  GDBusInterfaceVTable *vtable;
  guint registration_id;
  guint subtree_registration_id;
  vtable = g_object_get_data(G_OBJECT(invocation), "g-dbus-interface-vtable");
  g_assert(vtable != NULL && vtable->method_call != NULL);
  registration_id = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(invocation), "g-dbus-registration-id"));
  subtree_registration_id = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(invocation), "g-dbus-subtree-registration-id"));
  if (has_object_been_unregistered(g_dbus_method_invocation_get_connection(invocation), registration_id, subtree_registration_id)) {
      GDBusMessage *reply;
      reply = g_dbus_message_new_method_error(g_dbus_method_invocation_get_message(invocation),"org.freedesktop.DBus.Error.UnknownMethod",
                                              "No such interface `%s' on object at path %s", g_dbus_method_invocation_get_interface_name(invocation),
                                              g_dbus_method_invocation_get_object_path(invocation));
      g_dbus_connection_send_message(g_dbus_method_invocation_get_connection(invocation), reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
      g_object_unref(reply);
      goto out;
  }
  vtable->method_call(g_dbus_method_invocation_get_connection(invocation), g_dbus_method_invocation_get_sender(invocation),
                      g_dbus_method_invocation_get_object_path(invocation), g_dbus_method_invocation_get_interface_name(invocation),
                      g_dbus_method_invocation_get_method_name(invocation), g_dbus_method_invocation_get_parameters(invocation),
                      g_object_ref(invocation), g_dbus_method_invocation_get_user_data(invocation));
out:
  return FALSE;
}
static gboolean validate_and_maybe_schedule_method_call(GDBusConnection *connection, GDBusMessage *message, guint registration_id, guint subtree_registration_id,
                                                        GDBusInterfaceInfo *interface_info, const GDBusInterfaceVTable *vtable, GMainContext *main_context,
                                                        gpointer user_data) {
  GDBusMethodInvocation *invocation;
  const GDBusMethodInfo *method_info;
  GDBusMessage *reply;
  GVariant *parameters;
  GSource *idle_source;
  gboolean handled;
  GVariantType *in_type;
  handled = FALSE;
  method_info = g_dbus_interface_info_lookup_method(interface_info, g_dbus_message_get_member(message));
  if (method_info == NULL) {
      reply = g_dbus_message_new_method_error(message,"org.freedesktop.DBus.Error.UnknownMethod","No such method `%s'",
                                              g_dbus_message_get_member(message));
      g_dbus_connection_send_message_unlocked(connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
      g_object_unref(reply);
      handled = TRUE;
      goto out;
  }
  parameters = g_dbus_message_get_body(message);
  if (parameters == NULL) {
      parameters = g_variant_new("()");
      g_variant_ref_sink(parameters);
  } else g_variant_ref(parameters);
  in_type = _g_dbus_compute_complete_signature(method_info->in_args);
  if (!g_variant_is_of_type(parameters, in_type)) {
      gchar *type_string;
      type_string = g_variant_type_dup_string(in_type);
      reply = g_dbus_message_new_method_error(message,"org.freedesktop.DBus.Error.InvalidArgs","Type of message, `%s', does not match "
                                              "expected type `%s'", g_variant_get_type_string(parameters), type_string);
      g_dbus_connection_send_message_unlocked(connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
      g_variant_type_free(in_type);
      g_variant_unref(parameters);
      g_object_unref(reply);
      g_free(type_string);
      handled = TRUE;
      goto out;
  }
  g_variant_type_free(in_type);
  invocation = _g_dbus_method_invocation_new(g_dbus_message_get_sender(message), g_dbus_message_get_path(message), g_dbus_message_get_interface(message),
                                             g_dbus_message_get_member(message), method_info, connection, message, parameters, user_data);
  g_variant_unref(parameters);
  g_object_set_data(G_OBJECT(invocation), "g-dbus-interface-vtable", (gpointer)vtable);
  g_object_set_data(G_OBJECT(invocation), "g-dbus-registration-id", GUINT_TO_POINTER(registration_id));
  g_object_set_data(G_OBJECT(invocation), "g-dbus-subtree-registration-id", GUINT_TO_POINTER(subtree_registration_id));
  idle_source = g_idle_source_new();
  g_source_set_priority(idle_source, G_PRIORITY_DEFAULT);
  g_source_set_callback(idle_source, call_in_idle_cb, invocation, g_object_unref);
  g_source_attach(idle_source, main_context);
  g_source_unref(idle_source);
  handled = TRUE;
out:
  return handled;
}
static gboolean obj_message_func(GDBusConnection *connection, ExportedObject *eo, GDBusMessage *message) {
  const gchar *interface_name;
  const gchar *member;
  const gchar *signature;
  gboolean handled;
  handled = FALSE;
  interface_name = g_dbus_message_get_interface(message);
  member = g_dbus_message_get_member(message);
  signature = g_dbus_message_get_signature(message);
  if (interface_name != NULL) {
      ExportedInterface *ei;
      ei = g_hash_table_lookup(eo->map_if_name_to_ei, interface_name);
      if (ei != NULL) {
          if (ei->vtable == NULL || ei->vtable->method_call == NULL) goto out;
          handled = validate_and_maybe_schedule_method_call(connection, message, ei->id, 0, ei->interface_info, ei->vtable, ei->context,
                                                            ei->user_data);
          goto out;
      }
  }
  if (g_strcmp0(interface_name,"org.freedesktop.DBus.Introspectable") == 0 && g_strcmp0(member,"Introspect") == 0 && g_strcmp0(signature,"") == 0) {
      handled = handle_introspect(connection, eo, message);
      goto out;
  } else if (g_strcmp0(interface_name,"org.freedesktop.DBus.Properties") == 0 && g_strcmp0(member,"Get") == 0 && g_strcmp0(signature,"ss") == 0) {
      handled = handle_getset_property(connection, eo, message, TRUE);
      goto out;
  } else if (g_strcmp0(interface_name,"org.freedesktop.DBus.Properties") == 0 && g_strcmp0(member, "Set") == 0 && g_strcmp0(signature,"ssv") == 0) {
      handled = handle_getset_property(connection, eo, message, FALSE);
      goto out;
  } else if (g_strcmp0(interface_name,"org.freedesktop.DBus.Properties") == 0 && g_strcmp0(member,"GetAll") == 0 && g_strcmp0(signature,"s") == 0) {
      handled = handle_get_all_properties(connection, eo, message);
      goto out;
  }
out:
  return handled;
}
guint g_dbus_connection_register_object(GDBusConnection *connection, const gchar *object_path, GDBusInterfaceInfo *interface_info, const GDBusInterfaceVTable *vtable,
                                        gpointer user_data, GDestroyNotify user_data_free_func, GError **error) {
  ExportedObject *eo;
  ExportedInterface *ei;
  guint ret;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), 0);
  g_return_val_if_fail(object_path != NULL && g_variant_is_object_path(object_path), 0);
  g_return_val_if_fail(interface_info != NULL, 0);
  g_return_val_if_fail(g_dbus_is_interface_name(interface_info->name), 0);
  g_return_val_if_fail(error == NULL || *error == NULL, 0);
  ret = 0;
  CONNECTION_LOCK(connection);
  eo = g_hash_table_lookup(connection->map_object_path_to_eo, object_path);
  if (eo == NULL) {
      eo = g_new0(ExportedObject, 1);
      eo->object_path = g_strdup(object_path);
      eo->connection = connection;
      eo->map_if_name_to_ei = g_hash_table_new_full(g_str_hash, g_str_equal,NULL, (GDestroyNotify)exported_interface_free);
      g_hash_table_insert(connection->map_object_path_to_eo, eo->object_path, eo);
  }
  ei = g_hash_table_lookup(eo->map_if_name_to_ei, interface_info->name);
  if (ei != NULL) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_EXISTS,"An object is already exported for the interface %s at %s", interface_info->name,
                  object_path);
      goto out;
  }
  ei = g_new0(ExportedInterface, 1);
  ei->id = _global_registration_id++;
  ei->eo = eo;
  ei->user_data = user_data;
  ei->user_data_free_func = user_data_free_func;
  ei->vtable = _g_dbus_interface_vtable_copy(vtable);
  ei->interface_info = g_dbus_interface_info_ref(interface_info);
  ei->interface_name = g_strdup(interface_info->name);
  ei->context = g_main_context_get_thread_default();
  if (ei->context != NULL) g_main_context_ref(ei->context);
  g_hash_table_insert(eo->map_if_name_to_ei, (gpointer)ei->interface_name, ei);
  g_hash_table_insert(connection->map_id_to_ei, GUINT_TO_POINTER(ei->id), ei);
  ret = ei->id;
out:
  CONNECTION_UNLOCK(connection);
  return ret;
}
gboolean g_dbus_connection_unregister_object(GDBusConnection *connection, guint registration_id) {
  ExportedInterface *ei;
  ExportedObject *eo;
  gboolean ret;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), FALSE);
  ret = FALSE;
  CONNECTION_LOCK(connection);
  ei = g_hash_table_lookup(connection->map_id_to_ei, GUINT_TO_POINTER(registration_id));
  if (ei == NULL) goto out;
  eo = ei->eo;
  g_warn_if_fail(g_hash_table_remove(connection->map_id_to_ei, GUINT_TO_POINTER(ei->id)));
  g_warn_if_fail(g_hash_table_remove(eo->map_if_name_to_ei, ei->interface_name));
  if (g_hash_table_size(eo->map_if_name_to_ei) == 0) g_warn_if_fail(g_hash_table_remove(connection->map_object_path_to_eo, eo->object_path));
  ret = TRUE;
out:
  CONNECTION_UNLOCK(connection);
  return ret;
}
gboolean g_dbus_connection_emit_signal(GDBusConnection *connection, const gchar *destination_bus_name, const gchar *object_path, const gchar *interface_name,
                                       const gchar *signal_name, GVariant *parameters, GError **error) {
  GDBusMessage *message;
  gboolean ret;
  message = NULL;
  ret = FALSE;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), FALSE);
  g_return_val_if_fail(destination_bus_name == NULL || g_dbus_is_name(destination_bus_name), FALSE);
  g_return_val_if_fail(object_path != NULL && g_variant_is_object_path(object_path), FALSE);
  g_return_val_if_fail(interface_name != NULL && g_dbus_is_interface_name(interface_name), FALSE);
  g_return_val_if_fail(signal_name != NULL && g_dbus_is_member_name(signal_name), FALSE);
  g_return_val_if_fail(parameters == NULL || g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE), FALSE);
  if (G_UNLIKELY(_g_dbus_debug_emission())) {
      _g_dbus_debug_print_lock();
      g_print("========================================================================\nGDBus-debug:Emission:\n >>>> SIGNAL EMISSION %s.%s()\n      "
              "on object %s\n      destination %s\n", interface_name, signal_name, object_path, destination_bus_name != NULL ? destination_bus_name : "(none)");
      _g_dbus_debug_print_unlock();
  }
  message = g_dbus_message_new_signal(object_path, interface_name, signal_name);
  if (destination_bus_name != NULL) g_dbus_message_set_header(message,G_DBUS_MESSAGE_HEADER_FIELD_DESTINATION, g_variant_new_string(destination_bus_name));
  if (parameters != NULL) g_dbus_message_set_body(message, parameters);
  ret = g_dbus_connection_send_message(connection, message, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, error);
  g_object_unref(message);
  return ret;
}
static void add_call_flags(GDBusMessage *message, GDBusCallFlags flags) {
  if (flags & G_DBUS_CALL_FLAGS_NO_AUTO_START) g_dbus_message_set_flags(message, G_DBUS_MESSAGE_FLAGS_NO_AUTO_START);
}
static GVariant *decode_method_reply(GDBusMessage *reply, const gchar *method_name, const GVariantType *reply_type, GError **error) {
  GVariant *result;
  result = NULL;
  switch(g_dbus_message_get_message_type(reply)) {
      case G_DBUS_MESSAGE_TYPE_METHOD_RETURN:
          result = g_dbus_message_get_body(reply);
          if (result == NULL) {
              result = g_variant_new("()");
              g_variant_ref_sink(result);
          } else g_variant_ref(result);
          if (!g_variant_is_of_type(result, reply_type)) {
              gchar *type_string = g_variant_type_dup_string(reply_type);
              g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Method `%s' returned type `%s', but expected `%s'", method_name,
                          g_variant_get_type_string(result), type_string);
              g_variant_unref(result);
              g_free(type_string);
              result = NULL;
          }
          break;
      case G_DBUS_MESSAGE_TYPE_ERROR: g_dbus_message_to_gerror(reply, error); break;
      default: g_assert_not_reached(); break;
  }
  return result;
}
typedef struct {
  GSimpleAsyncResult *simple;
  GVariantType *reply_type;
  gchar *method_name;
  guint32 serial;
} CallState;
static void g_dbus_connection_call_done(GObject *source, GAsyncResult *result, gpointer user_data) {
  GDBusConnection *connection = G_DBUS_CONNECTION (source);
  CallState *state = user_data;
  GError *error;
  GDBusMessage *reply;
  GVariant *value;
  error = NULL;
  reply = g_dbus_connection_send_message_with_reply_finish(connection, result, &error);
  if (G_UNLIKELY(_g_dbus_debug_call())) {
      _g_dbus_debug_print_lock();
      g_print("========================================================================\nGDBus-debug:Call:\n <<<< ASYNC COMPLETE %s() (serial %d)\n      ",
              state->method_name, state->serial);
      if (reply != NULL) g_print("SUCCESS\n");
      else g_print("FAILED: %s\n", error->message);
      _g_dbus_debug_print_unlock();
  }
  if (reply != NULL) {
      value = decode_method_reply(reply, state->method_name, state->reply_type, &error);
      g_object_unref(reply);
  } else value = NULL;
  if (value == NULL) g_simple_async_result_take_error(state->simple, error);
  else g_simple_async_result_set_op_res_gpointer(state->simple, value, (GDestroyNotify)g_variant_unref);
  g_simple_async_result_complete(state->simple);
  g_variant_type_free(state->reply_type);
  g_object_unref(state->simple);
  g_free(state->method_name);
  g_slice_free(CallState, state);
}
void g_dbus_connection_call(GDBusConnection *connection, const gchar *bus_name, const gchar *object_path, const gchar *interface_name, const gchar *method_name,
                            GVariant *parameters, const GVariantType *reply_type, GDBusCallFlags flags, gint timeout_msec, GCancellable *cancellable,
                            GAsyncReadyCallback callback, gpointer user_data) {
  GDBusMessage *message;
  CallState *state;
  g_return_if_fail(G_IS_DBUS_CONNECTION(connection));
  g_return_if_fail(bus_name == NULL || g_dbus_is_name(bus_name));
  g_return_if_fail(object_path != NULL && g_variant_is_object_path(object_path));
  g_return_if_fail(interface_name != NULL && g_dbus_is_interface_name(interface_name));
  g_return_if_fail(method_name != NULL && g_dbus_is_member_name(method_name));
  g_return_if_fail(timeout_msec >= 0 || timeout_msec == -1);
  g_return_if_fail((parameters == NULL) || g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE));
  state = g_slice_new(CallState);
  state->simple = g_simple_async_result_new(G_OBJECT(connection), callback, user_data, g_dbus_connection_call);
  state->method_name = g_strjoin(".", interface_name, method_name, NULL);
  if (reply_type == NULL) reply_type = G_VARIANT_TYPE_ANY;
  state->reply_type = g_variant_type_copy(reply_type);
  message = g_dbus_message_new_method_call(bus_name, object_path, interface_name, method_name);
  add_call_flags(message, flags);
  if (parameters != NULL) g_dbus_message_set_body(message, parameters);
  g_dbus_connection_send_message_with_reply(connection, message,G_DBUS_SEND_MESSAGE_FLAGS_NONE, timeout_msec, &state->serial, cancellable,
                                            g_dbus_connection_call_done, state);
  if (G_UNLIKELY(_g_dbus_debug_call())) {
      _g_dbus_debug_print_lock();
      g_print("========================================================================\nGDBus-debug:Call:\n >>>> ASYNC %s.%s()\n      on object %s\n"
              "      owned by name %s (serial %d)\n", interface_name, method_name, object_path, bus_name != NULL ? bus_name : "(none)", state->serial);
      _g_dbus_debug_print_unlock();
  }
  if (message != NULL) g_object_unref(message);
}
GVariant *g_dbus_connection_call_finish(GDBusConnection *connection, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), NULL);
  g_return_val_if_fail(g_simple_async_result_is_valid(res, G_OBJECT(connection), g_dbus_connection_call), NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  simple = G_SIMPLE_ASYNC_RESULT(res);
  if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  return g_variant_ref(g_simple_async_result_get_op_res_gpointer(simple));
}
GVariant *g_dbus_connection_call_sync(GDBusConnection *connection, const gchar *bus_name, const gchar *object_path, const gchar *interface_name,
                                      const gchar *method_name, GVariant *parameters, const GVariantType *reply_type, GDBusCallFlags flags, gint timeout_msec,
                                      GCancellable *cancellable, GError **error) {
  GDBusMessage *message;
  GDBusMessage *reply;
  GVariant *result;
  GError *local_error;
  message = NULL;
  reply = NULL;
  result = NULL;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), NULL);
  g_return_val_if_fail(bus_name == NULL || g_dbus_is_name(bus_name), NULL);
  g_return_val_if_fail(object_path != NULL && g_variant_is_object_path(object_path), NULL);
  g_return_val_if_fail(interface_name != NULL && g_dbus_is_interface_name(interface_name), NULL);
  g_return_val_if_fail(method_name != NULL && g_dbus_is_member_name(method_name), NULL);
  g_return_val_if_fail(timeout_msec >= 0 || timeout_msec == -1, NULL);
  g_return_val_if_fail((parameters == NULL) || g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE), NULL);
  if (reply_type == NULL) reply_type = G_VARIANT_TYPE_ANY;
  message = g_dbus_message_new_method_call(bus_name, object_path, interface_name, method_name);
  add_call_flags(message, flags);
  if (parameters != NULL) g_dbus_message_set_body(message, parameters);
  if (G_UNLIKELY(_g_dbus_debug_call())) {
      _g_dbus_debug_print_lock();
      g_print("========================================================================\nGDBus-debug:Call:\n >>>> SYNC %s.%s()\n      on object %s\n"
              "      owned by name %s\n", interface_name, method_name, object_path, bus_name != NULL ? bus_name : "(none)");
      _g_dbus_debug_print_unlock();
  }
  local_error = NULL;
  reply = g_dbus_connection_send_message_with_reply_sync(connection, message,G_DBUS_SEND_MESSAGE_FLAGS_NONE, timeout_msec,NULL, cancellable,
                                                         &local_error);
  if (G_UNLIKELY(_g_dbus_debug_call())) {
      _g_dbus_debug_print_lock();
      g_print("========================================================================\nGDBus-debug:Call:\n <<<< SYNC COMPLETE %s.%s()\n      ",
              interface_name, method_name);
      if (reply != NULL) g_print("SUCCESS\n");
      else g_print("FAILED: %s\n", local_error->message);
      _g_dbus_debug_print_unlock();
  }
  if (reply == NULL) {
      if (error != NULL) *error = local_error;
      else g_error_free(local_error);
      goto out;
  }
  result = decode_method_reply(reply, method_name, reply_type, error);
out:
  if (message != NULL) g_object_unref(message);
  if (reply != NULL) g_object_unref(reply);
  return result;
}
struct ExportedSubtree {
  guint id;
  gchar *object_path;
  GDBusConnection *connection;
  GDBusSubtreeVTable *vtable;
  GDBusSubtreeFlags flags;
  GMainContext *context;
  gpointer user_data;
  GDestroyNotify user_data_free_func;
};
static void exported_subtree_free(ExportedSubtree *es) {
  call_destroy_notify(es->context, es->user_data_free_func, es->user_data);
  if (es->context != NULL) g_main_context_unref(es->context);
  _g_dbus_subtree_vtable_free(es->vtable);
  g_free(es->object_path);
  g_free(es);
}
static gboolean handle_subtree_introspect(GDBusConnection *connection, ExportedSubtree *es, GDBusMessage *message) {
  GString *s;
  gboolean handled;
  GDBusMessage *reply;
  gchar **children;
  gboolean is_root;
  const gchar *sender;
  const gchar *requested_object_path;
  const gchar *requested_node;
  GDBusInterfaceInfo **interfaces;
  guint n;
  gchar **subnode_paths;
  gboolean has_properties_interface;
  gboolean has_introspectable_interface;
  handled = FALSE;
  requested_object_path = g_dbus_message_get_path(message);
  sender = g_dbus_message_get_sender(message);
  is_root = (g_strcmp0(requested_object_path, es->object_path) == 0);
  s = g_string_new(NULL);
  introspect_append_header(s);
  children = es->vtable->enumerate(es->connection, sender, es->object_path, es->user_data);
  if (!is_root) {
      requested_node = strrchr(requested_object_path, '/') + 1;
      if (!(es->flags & G_DBUS_SUBTREE_FLAGS_DISPATCH_TO_UNENUMERATED_NODES) && !_g_strv_has_string((const gchar * const *)children, requested_node)) goto out;
  } else requested_node = NULL;
  interfaces = es->vtable->introspect(es->connection, sender, es->object_path, requested_node, es->user_data);
  if (interfaces != NULL) {
      has_properties_interface = FALSE;
      has_introspectable_interface = FALSE;
      for (n = 0; interfaces[n] != NULL; n++) {
          if (strcmp(interfaces[n]->name, "org.freedesktop.DBus.Properties") == 0) has_properties_interface = TRUE;
          else if (strcmp(interfaces[n]->name, "org.freedesktop.DBus.Introspectable") == 0) has_introspectable_interface = TRUE;
      }
      if (!has_properties_interface) g_string_append(s, introspect_properties_interface);
      if (!has_introspectable_interface) g_string_append(s, introspect_introspectable_interface);
      for (n = 0; interfaces[n] != NULL; n++) {
          g_dbus_interface_info_generate_xml(interfaces[n], 2, s);
          g_dbus_interface_info_unref(interfaces[n]);
      }
      g_free(interfaces);
  }
  if (is_root) {
      for (n = 0; children != NULL && children[n] != NULL; n++) g_string_append_printf(s, "  <node name=\"%s\"/>\n", children[n]);
  }
  subnode_paths = g_dbus_connection_list_registered(es->connection, requested_object_path);
  for (n = 0; subnode_paths != NULL && subnode_paths[n] != NULL; n++) g_string_append_printf(s, "  <node name=\"%s\"/>\n", subnode_paths[n]);
  g_strfreev(subnode_paths);
  g_string_append(s, "</node>\n");
  reply = g_dbus_message_new_method_reply(message);
  g_dbus_message_set_body(reply, g_variant_new("(s)", s->str));
  g_dbus_connection_send_message(connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
  g_object_unref(reply);
  handled = TRUE;
out:
  g_string_free(s, TRUE);
  g_strfreev(children);
  return handled;
}
static gboolean handle_subtree_method_invocation(GDBusConnection *connection, ExportedSubtree *es, GDBusMessage *message) {
  gboolean handled;
  const gchar *sender;
  const gchar *interface_name;
  const gchar *member;
  const gchar *signature;
  const gchar *requested_object_path;
  const gchar *requested_node;
  gboolean is_root;
  GDBusInterfaceInfo *interface_info;
  const GDBusInterfaceVTable *interface_vtable;
  gpointer interface_user_data;
  guint n;
  GDBusInterfaceInfo **interfaces;
  gboolean is_property_get;
  gboolean is_property_set;
  gboolean is_property_get_all;
  handled = FALSE;
  interfaces = NULL;
  requested_object_path = g_dbus_message_get_path(message);
  sender = g_dbus_message_get_sender(message);
  interface_name = g_dbus_message_get_interface(message);
  member = g_dbus_message_get_member(message);
  signature = g_dbus_message_get_signature(message);
  is_root = (g_strcmp0(requested_object_path, es->object_path) == 0);
  is_property_get = FALSE;
  is_property_set = FALSE;
  is_property_get_all = FALSE;
  if (g_strcmp0(interface_name, "org.freedesktop.DBus.Properties") == 0) {
      if (g_strcmp0(member, "Get") == 0 && g_strcmp0(signature, "ss") == 0) is_property_get = TRUE;
      else if (g_strcmp0(member, "Set") == 0 && g_strcmp0(signature, "ssv") == 0) is_property_set = TRUE;
      else if (g_strcmp0(member, "GetAll") == 0 && g_strcmp0(signature, "s") == 0) is_property_get_all = TRUE;
  }
  if (!is_root) {
      requested_node = strrchr(requested_object_path, '/') + 1;
      if (~es->flags & G_DBUS_SUBTREE_FLAGS_DISPATCH_TO_UNENUMERATED_NODES) {
          gchar **children;
          gboolean exists;
          children = es->vtable->enumerate(es->connection, sender, es->object_path, es->user_data);
          exists = _g_strv_has_string((const gchar * const *)children, requested_node);
          g_strfreev(children);
          if (!exists) goto out;
      }
  } else requested_node = NULL;
  interfaces = es->vtable->introspect(es->connection, sender, requested_object_path, requested_node, es->user_data);
  if (interfaces == NULL) goto out;
  interface_info = NULL;
  for (n = 0; interfaces[n] != NULL; n++) {
      if (g_strcmp0(interfaces[n]->name, interface_name) == 0) interface_info = interfaces[n];
  }
  if (interface_info != NULL) {
      interface_user_data = NULL;
      interface_vtable = es->vtable->dispatch(es->connection, sender, es->object_path, interface_name, requested_node, &interface_user_data, es->user_data);
      if (interface_vtable == NULL) goto out;
      CONNECTION_LOCK(connection);
      handled = validate_and_maybe_schedule_method_call(es->connection, message, 0, es->id, interface_info, interface_vtable, es->context,
                                                        interface_user_data);
      CONNECTION_UNLOCK(connection);
  } else if (is_property_get || is_property_set || is_property_get_all) {
      if (is_property_get) g_variant_get (g_dbus_message_get_body(message), "(&s&s)", &interface_name, NULL);
      else if (is_property_set) g_variant_get(g_dbus_message_get_body(message), "(&s&sv)", &interface_name, NULL, NULL);
      else if (is_property_get_all) g_variant_get(g_dbus_message_get_body(message), "(&s)", &interface_name, NULL, NULL);
      else g_assert_not_reached();
      for (n = 0; interfaces[n] != NULL; n++) {
          if (g_strcmp0(interfaces[n]->name, interface_name) == 0) interface_info = interfaces[n];
      }
      if (interface_info == NULL) {
          GDBusMessage *reply;
          reply = g_dbus_message_new_method_error(message,"org.freedesktop.DBus.Error.InvalidArgs","No such interface `%s'", interface_name);
          g_dbus_connection_send_message(es->connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
          g_object_unref(reply);
          handled = TRUE;
          goto out;
      }
      interface_user_data = NULL;
      interface_vtable = es->vtable->dispatch(es->connection, sender, es->object_path, interface_name, requested_node, &interface_user_data, es->user_data);
      if (interface_vtable == NULL) {
          g_warning("The subtree introspection function indicates that '%s' is a valid interface name, but calling the dispatch function on that "
                    "interface gave us NULL", interface_name);
          goto out;
      }
      if (is_property_get || is_property_set) {
          CONNECTION_LOCK(connection);
          handled = validate_and_maybe_schedule_property_getset(es->connection, message,0, es->id, is_property_get, interface_info, interface_vtable,
                                                                es->context, interface_user_data);
          CONNECTION_UNLOCK(connection);
      } else if (is_property_get_all) {
          CONNECTION_LOCK(connection);
          handled = validate_and_maybe_schedule_property_get_all(es->connection, message,0, es->id, interface_info, interface_vtable, es->context,
                                                                 interface_user_data);
          CONNECTION_UNLOCK(connection);
      }
  }
out:
  if (interfaces != NULL) {
      for (n = 0; interfaces[n] != NULL; n++) g_dbus_interface_info_unref(interfaces[n]);
      g_free(interfaces);
  }
  return handled;
}
typedef struct {
  GDBusMessage *message;
  ExportedSubtree *es;
} SubtreeDeferredData;
static void subtree_deferred_data_free(SubtreeDeferredData *data) {
  g_object_unref(data->message);
  g_free(data);
}
static gboolean process_subtree_vtable_message_in_idle_cb(gpointer _data) {
  SubtreeDeferredData *data = _data;
  gboolean handled;
  handled = FALSE;
  if (g_strcmp0(g_dbus_message_get_interface(data->message),"org.freedesktop.DBus.Introspectable") == 0 &&
      g_strcmp0(g_dbus_message_get_member(data->message),"Introspect") == 0 && g_strcmp0(g_dbus_message_get_signature (data->message), "") == 0) {
      handled = handle_subtree_introspect(data->es->connection, data->es, data->message);
  } else handled = handle_subtree_method_invocation(data->es->connection, data->es, data->message);
  if (!handled) {
      CONNECTION_LOCK(data->es->connection);
      handled = handle_generic_unlocked(data->es->connection, data->message);
      CONNECTION_UNLOCK(data->es->connection);
  }
  if (!handled) {
      GDBusMessage *reply;
      reply = g_dbus_message_new_method_error(data->message,"org.freedesktop.DBus.Error.UnknownMethod","Method `%s' on interface `%s' with "
                                              "signature `%s' does not exist", g_dbus_message_get_member(data->message), g_dbus_message_get_interface(data->message),
                                               g_dbus_message_get_signature(data->message));
      g_dbus_connection_send_message(data->es->connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
      g_object_unref(reply);
  }
  return FALSE;
}
static gboolean subtree_message_func(GDBusConnection *connection, ExportedSubtree *es, GDBusMessage *message) {
  GSource *idle_source;
  SubtreeDeferredData *data;
  data = g_new0(SubtreeDeferredData, 1);
  data->message = g_object_ref(message);
  data->es = es;
  idle_source = g_idle_source_new();
  g_source_set_priority(idle_source, G_PRIORITY_HIGH);
  g_source_set_callback(idle_source, process_subtree_vtable_message_in_idle_cb, data, (GDestroyNotify)subtree_deferred_data_free);
  g_source_attach(idle_source, es->context);
  g_source_unref(idle_source);
  return TRUE;
}
guint g_dbus_connection_register_subtree(GDBusConnection *connection, const gchar *object_path, const GDBusSubtreeVTable *vtable, GDBusSubtreeFlags flags,
                                         gpointer user_data, GDestroyNotify user_data_free_func, GError **error) {
  guint ret;
  ExportedSubtree *es;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), 0);
  g_return_val_if_fail(object_path != NULL && g_variant_is_object_path(object_path), 0);
  g_return_val_if_fail(vtable != NULL, 0);
  g_return_val_if_fail(error == NULL || *error == NULL, 0);
  ret = 0;
  CONNECTION_LOCK(connection);
  es = g_hash_table_lookup(connection->map_object_path_to_es, object_path);
  if (es != NULL) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_EXISTS,"A subtree is already exported for %s", object_path);
      goto out;
  }
  es = g_new0(ExportedSubtree, 1);
  es->object_path = g_strdup(object_path);
  es->connection = connection;
  es->vtable = _g_dbus_subtree_vtable_copy(vtable);
  es->flags = flags;
  es->id = _global_subtree_registration_id++;
  es->user_data = user_data;
  es->user_data_free_func = user_data_free_func;
  es->context = g_main_context_get_thread_default();
  if (es->context != NULL) g_main_context_ref(es->context);
  g_hash_table_insert(connection->map_object_path_to_es, es->object_path, es);
  g_hash_table_insert(connection->map_id_to_es, GUINT_TO_POINTER(es->id), es);
  ret = es->id;
out:
  CONNECTION_UNLOCK(connection);
  return ret;
}
gboolean g_dbus_connection_unregister_subtree(GDBusConnection *connection, guint registration_id) {
  ExportedSubtree *es;
  gboolean ret;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), FALSE);
  ret = FALSE;
  CONNECTION_LOCK(connection);
  es = g_hash_table_lookup(connection->map_id_to_es, GUINT_TO_POINTER(registration_id));
  if (es == NULL) goto out;
  g_warn_if_fail(g_hash_table_remove(connection->map_id_to_es, GUINT_TO_POINTER(es->id)));
  g_warn_if_fail(g_hash_table_remove(connection->map_object_path_to_es, es->object_path));
  ret = TRUE;
out:
  CONNECTION_UNLOCK(connection);
  return ret;
}
static void handle_generic_ping_unlocked(GDBusConnection *connection, const gchar *object_path, GDBusMessage *message) {
  GDBusMessage *reply;
  reply = g_dbus_message_new_method_reply(message);
  g_dbus_connection_send_message_unlocked(connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
  g_object_unref(reply);
}
static void handle_generic_get_machine_id_unlocked(GDBusConnection *connection, const gchar *object_path, GDBusMessage *message) {
  GDBusMessage *reply;
  reply = NULL;
  if (connection->machine_id == NULL) {
      GError *error;
      error = NULL;
      connection->machine_id = _g_dbus_get_machine_id(&error);
      if (connection->machine_id == NULL) {
          reply = g_dbus_message_new_method_error_literal(message, "org.freedesktop.DBus.Error.Failed", error->message);
          g_error_free(error);
      }
  }
  if (reply == NULL) {
      reply = g_dbus_message_new_method_reply(message);
      g_dbus_message_set_body(reply, g_variant_new("(s)", connection->machine_id));
  }
  g_dbus_connection_send_message_unlocked(connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
  g_object_unref(reply);
}
static void handle_generic_introspect_unlocked(GDBusConnection *connection, const gchar *object_path, GDBusMessage *message) {
  guint n;
  GString *s;
  gchar **registered;
  GDBusMessage *reply;
  s = g_string_new(NULL);
  introspect_append_header(s);
  registered = g_dbus_connection_list_registered_unlocked(connection, object_path);
  for (n = 0; registered != NULL && registered[n] != NULL; n++) g_string_append_printf(s, "  <node name=\"%s\"/>\n", registered[n]);
  g_strfreev(registered);
  g_string_append(s, "</node>\n");
  reply = g_dbus_message_new_method_reply(message);
  g_dbus_message_set_body(reply, g_variant_new("(s)", s->str));
  g_dbus_connection_send_message_unlocked(connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
  g_object_unref(reply);
  g_string_free(s, TRUE);
}
static gboolean handle_generic_unlocked(GDBusConnection *connection, GDBusMessage *message) {
  gboolean handled;
  const gchar *interface_name;
  const gchar *member;
  const gchar *signature;
  const gchar *path;
  CONNECTION_ENSURE_LOCK(connection);
  handled = FALSE;
  interface_name = g_dbus_message_get_interface(message);
  member = g_dbus_message_get_member(message);
  signature = g_dbus_message_get_signature(message);
  path = g_dbus_message_get_path(message);
  if (g_strcmp0(interface_name, "org.freedesktop.DBus.Introspectable") == 0 && g_strcmp0(member, "Introspect") == 0 &&
      g_strcmp0(signature, "") == 0) {
      handle_generic_introspect_unlocked(connection, path, message);
      handled = TRUE;
  } else if (g_strcmp0(interface_name, "org.freedesktop.DBus.Peer") == 0 && g_strcmp0(member, "Ping") == 0 && g_strcmp0(signature, "") == 0) {
      handle_generic_ping_unlocked(connection, path, message);
      handled = TRUE;
  } else if (g_strcmp0(interface_name, "org.freedesktop.DBus.Peer") == 0 && g_strcmp0(member, "GetMachineId") == 0 &&
             g_strcmp0(signature, "") == 0) {
      handle_generic_get_machine_id_unlocked(connection, path, message);
      handled = TRUE;
  }
  return handled;
}
static void distribute_method_call(GDBusConnection *connection, GDBusMessage *message) {
  GDBusMessage *reply;
  ExportedObject *eo;
  ExportedSubtree *es;
  const gchar *object_path;
  const gchar *interface_name;
  const gchar *member;
  const gchar *signature;
  const gchar *path;
  gchar *subtree_path;
  gchar *needle;
  g_assert (g_dbus_message_get_message_type(message) == G_DBUS_MESSAGE_TYPE_METHOD_CALL);
  interface_name = g_dbus_message_get_interface(message);
  member = g_dbus_message_get_member(message);
  signature = g_dbus_message_get_signature(message);
  path = g_dbus_message_get_path(message);
  subtree_path = g_strdup(path);
  needle = strrchr(subtree_path, '/');
  if (needle != NULL && needle != subtree_path) *needle = '\0';
  else {
      g_free(subtree_path);
      subtree_path = NULL;
  }
  if (G_UNLIKELY(_g_dbus_debug_incoming())) {
      _g_dbus_debug_print_lock();
      g_print("========================================================================\nGDBus-debug:Incoming:\n <<<< METHOD INVOCATION %s.%s()\n"
              "      on object %s\n      invoked by name %s\n      serial %d\n", interface_name, member, path,
              g_dbus_message_get_sender(message) != NULL ? g_dbus_message_get_sender(message) : "(none)", g_dbus_message_get_serial(message));
      _g_dbus_debug_print_unlock();
  }
#if 0
  g_debug("interface    = `%s'", interface_name);
  g_debug("member       = `%s'", member);
  g_debug("signature    = `%s'", signature);
  g_debug("path         = `%s'", path);
  g_debug("subtree_path = `%s'", subtree_path != NULL ? subtree_path : "N/A");
#endif
  object_path = g_dbus_message_get_path(message);
  g_assert(object_path != NULL);
  eo = g_hash_table_lookup(connection->map_object_path_to_eo, object_path);
  if (eo != NULL) {
      if (obj_message_func(connection, eo, message)) goto out;
  }
  es = g_hash_table_lookup(connection->map_object_path_to_es, object_path);
  if (es != NULL) {
      if (subtree_message_func(connection, es, message)) goto out;
  }
  if (subtree_path != NULL) {
      es = g_hash_table_lookup(connection->map_object_path_to_es, subtree_path);
      if (es != NULL) {
          if (subtree_message_func(connection, es, message)) goto out;
      }
  }
  if (handle_generic_unlocked(connection, message)) goto out;
  reply = g_dbus_message_new_method_error(message,"org.freedesktop.DBus.Error.UnknownMethod","No such interface `%s' on object at path %s",
                                          interface_name, object_path);
  g_dbus_connection_send_message_unlocked(connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
  g_object_unref(reply);
 out:
  g_free(subtree_path);
}
static GDBusConnection **message_bus_get_singleton(GBusType bus_type, GError **error) {
  GDBusConnection **ret;
  const gchar *starter_bus;
  ret = NULL;
  switch(bus_type) {
      case G_BUS_TYPE_SESSION: ret = &the_session_bus; break;
      case G_BUS_TYPE_SYSTEM: ret = &the_system_bus; break;
      case G_BUS_TYPE_STARTER:
          starter_bus = g_getenv("DBUS_STARTER_BUS_TYPE");
          if (g_strcmp0(starter_bus, "session") == 0) {
              ret = message_bus_get_singleton(G_BUS_TYPE_SESSION, error);
              goto out;
          } else if (g_strcmp0(starter_bus, "system") == 0) {
              ret = message_bus_get_singleton(G_BUS_TYPE_SYSTEM, error);
              goto out;
          } else {
              if (starter_bus != NULL){
                  g_set_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,"Cannot determine bus address from DBUS_STARTER_BUS_TYPE environment variable"
                              " - unknown value `%s'", starter_bus);
              } else {
                  g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,"Cannot determine bus address because the DBUS_STARTER_BUS_TYPE environment "
                                      "variable is not set");
              }
          }
          break;
      default: g_assert_not_reached(); break;
  }
out:
  return ret;
}
static GDBusConnection *get_uninitialized_connection(GBusType bus_type, GCancellable *cancellable, GError **error) {
  GDBusConnection **singleton;
  GDBusConnection *ret;
  ret = NULL;
  G_LOCK(message_bus_lock);
  singleton = message_bus_get_singleton(bus_type, error);
  if (singleton == NULL) goto out;
  if (*singleton == NULL) {
      gchar *address;
      address = g_dbus_address_get_for_bus_sync(bus_type, cancellable, error);
      if (address == NULL) goto out;
      ret = *singleton = g_object_new(G_TYPE_DBUS_CONNECTION,"address", address, "flags", G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
                                      G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION, "exit-on-close", TRUE, NULL);
      g_free(address);
  } else ret = g_object_ref(*singleton);
  g_assert(ret != NULL);
out:
  G_UNLOCK(message_bus_lock);
  return ret;
}
GDBusConnection *g_bus_get_sync(GBusType bus_type, GCancellable *cancellable, GError **error) {
  GDBusConnection *connection;
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  connection = get_uninitialized_connection(bus_type, cancellable, error);
  if (connection == NULL) goto out;
  if (!g_initable_init(G_INITABLE(connection), cancellable, error)) {
      g_object_unref(connection);
      connection = NULL;
  }
 out:
  return connection;
}
static void bus_get_async_initable_cb(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(user_data);
  GError *error;
  error = NULL;
  if (!g_async_initable_init_finish(G_ASYNC_INITABLE(source_object), res, &error)) {
      g_assert(error != NULL);
      g_simple_async_result_take_error(simple, error);
      g_object_unref(source_object);
  } else  g_simple_async_result_set_op_res_gpointer(simple, source_object, g_object_unref);
  g_simple_async_result_complete_in_idle(simple);
  g_object_unref(simple);
}
void g_bus_get(GBusType bus_type, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GDBusConnection *connection;
  GSimpleAsyncResult *simple;
  GError *error;
  simple = g_simple_async_result_new(NULL, callback, user_data, g_bus_get);
  error = NULL;
  connection = get_uninitialized_connection(bus_type, cancellable, &error);
  if (connection == NULL) {
      g_assert(error != NULL);
      g_simple_async_result_take_error(simple, error);
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
  } else g_async_initable_init_async(G_ASYNC_INITABLE(connection), G_PRIORITY_DEFAULT, cancellable, bus_get_async_initable_cb, simple);
}
GDBusConnection *g_bus_get_finish(GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  GObject *object;
  GDBusConnection *ret;
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_bus_get);
  ret = NULL;
  if (g_simple_async_result_propagate_error(simple, error)) goto out;
  object = g_simple_async_result_get_op_res_gpointer(simple);
  g_assert(object != NULL);
  ret = g_object_ref(G_DBUS_CONNECTION(object));
out:
  return ret;
}