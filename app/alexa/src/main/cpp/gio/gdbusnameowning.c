#include <stdlib.h>
#include "../glib/glibintl.h"
#include "../glib/glib-object.h"
#include "config.h"
#include "gdbusutils.h"
#include "gdbusnameowning.h"
#include "gdbuserror.h"
#include "gdbusprivate.h"
#include "gdbusconnection.h"

G_LOCK_DEFINE_STATIC (lock);
typedef enum {
  PREVIOUS_CALL_NONE = 0,
  PREVIOUS_CALL_ACQUIRED,
  PREVIOUS_CALL_LOST,
} PreviousCall;
typedef struct {
  volatile gint ref_count;
  guint id;
  GBusNameOwnerFlags flags;
  gchar *name;
  GBusAcquiredCallback bus_acquired_handler;
  GBusNameAcquiredCallback name_acquired_handler;
  GBusNameLostCallback name_lost_handler;
  gpointer user_data;
  GDestroyNotify user_data_free_func;
  GMainContext *main_context;
  PreviousCall previous_call;
  GDBusConnection *connection;
  gulong disconnected_signal_handler_id;
  guint name_acquired_subscription_id;
  guint name_lost_subscription_id;
  gboolean cancelled;
  gboolean needs_release;
} Client;
static guint next_global_id = 1;
static GHashTable *map_id_to_client = NULL;
static Client *
client_ref (Client *client) {
  g_atomic_int_inc(&client->ref_count);
  return client;
}
static void client_unref(Client *client) {
  if (g_atomic_int_dec_and_test(&client->ref_count)) {
      if (client->connection != NULL) {
          if (client->disconnected_signal_handler_id > 0) g_signal_handler_disconnect(client->connection, client->disconnected_signal_handler_id);
          if (client->name_acquired_subscription_id > 0) g_dbus_connection_signal_unsubscribe(client->connection, client->name_acquired_subscription_id);
          if (client->name_lost_subscription_id > 0) g_dbus_connection_signal_unsubscribe(client->connection, client->name_lost_subscription_id);
          g_object_unref(client->connection);
      }
      if (client->main_context != NULL) g_main_context_unref(client->main_context);
      g_free(client->name);
      if (client->user_data_free_func != NULL) client->user_data_free_func(client->user_data);
      g_free(client);
  }
}
typedef enum {
  CALL_TYPE_NAME_ACQUIRED,
  CALL_TYPE_NAME_LOST
} CallType;
typedef struct {
  Client *client;
  GDBusConnection *connection;
  CallType call_type;
} CallHandlerData;
static void call_handler_data_free(CallHandlerData *data) {
  if (data->connection != NULL) g_object_unref(data->connection);
  client_unref(data->client);
  g_free(data);
}
static void actually_do_call(Client *client, GDBusConnection *connection, CallType call_type) {
  switch(call_type) {
      case CALL_TYPE_NAME_ACQUIRED:
          if (client->name_acquired_handler != NULL) client->name_acquired_handler(connection, client->name, client->user_data);
          break;
      case CALL_TYPE_NAME_LOST:
          if (client->name_lost_handler != NULL) client->name_lost_handler(connection, client->name, client->user_data);
          break;
      default: g_assert_not_reached(); break;
  }
}
static gboolean call_in_idle_cb(gpointer _data) {
  CallHandlerData *data = _data;
  actually_do_call(data->client, data->connection, data->call_type);
  return FALSE;
}
static void schedule_call_in_idle(Client *client, CallType  call_type) {
  CallHandlerData *data;
  GSource *idle_source;
  data = g_new0(CallHandlerData, 1);
  data->client = client_ref(client);
  data->connection = client->connection != NULL ? g_object_ref(client->connection) : NULL;
  data->call_type = call_type;
  idle_source = g_idle_source_new();
  g_source_set_priority(idle_source, G_PRIORITY_HIGH);
  g_source_set_callback(idle_source, call_in_idle_cb, data, (GDestroyNotify)call_handler_data_free);
  g_source_attach(idle_source, client->main_context);
  g_source_unref(idle_source);
}
static void do_call(Client *client, CallType call_type) {
  if (g_main_context_get_thread_default() != client->main_context) schedule_call_in_idle(client, call_type);
  else actually_do_call(client, client->connection, call_type);
}
static void call_acquired_handler(Client *client) {
  if (client->previous_call != PREVIOUS_CALL_ACQUIRED) {
      client->previous_call = PREVIOUS_CALL_ACQUIRED;
      if (!client->cancelled) do_call(client, CALL_TYPE_NAME_ACQUIRED);
  }
}
static void call_lost_handler(Client *client) {
  if (client->previous_call != PREVIOUS_CALL_LOST) {
      client->previous_call = PREVIOUS_CALL_LOST;
      if (!client->cancelled) do_call(client, CALL_TYPE_NAME_LOST);
  }
}
static void on_name_lost_or_acquired(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path, const gchar *interface_name,
                                     const gchar *signal_name, GVariant *parameters, gpointer user_data) {
  Client *client = user_data;
  const gchar *name;
  if (g_strcmp0(object_path, "/org/freedesktop/DBus") != 0 || g_strcmp0(interface_name, "org.freedesktop.DBus") != 0 ||
      g_strcmp0(sender_name, "org.freedesktop.DBus") != 0) {
    return;
  }
  if (g_strcmp0(signal_name, "NameLost") == 0) {
      g_variant_get(parameters, "(&s)", &name);
      if (g_strcmp0(name, client->name) == 0) call_lost_handler(client);
  } else if (g_strcmp0(signal_name, "NameAcquired") == 0) {
      g_variant_get(parameters, "(&s)", &name);
      if (g_strcmp0(name, client->name) == 0) call_acquired_handler(client);
  }
}
static void request_name_cb(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  Client *client = user_data;
  GVariant *result;
  guint32 request_name_reply;
  gboolean subscribe;
  request_name_reply = 0;
  result = NULL;
  result = g_dbus_connection_call_finish(client->connection, res,NULL);
  if (result != NULL) {
      g_variant_get(result, "(u)", &request_name_reply);
      g_variant_unref(result);
  }
  subscribe = FALSE;
  switch(request_name_reply) {
      case 1:
          call_acquired_handler(client);
          subscribe = TRUE;
          client->needs_release = TRUE;
          break;
      case 2:
          call_lost_handler(client);
          subscribe = TRUE;
          client->needs_release = TRUE;
          break;
      default: call_lost_handler(client); break;
  }
  if (subscribe) {
      client->name_lost_subscription_id = g_dbus_connection_signal_subscribe(client->connection,"org.freedesktop.DBus","org.freedesktop.DBus",
                                                                     "NameLost","/org/freedesktop/DBus", client->name,
                                                                        G_DBUS_SIGNAL_FLAGS_NONE, on_name_lost_or_acquired, client,NULL);
      client->name_acquired_subscription_id = g_dbus_connection_signal_subscribe(client->connection,"org.freedesktop.DBus","org.freedesktop.DBus",
                                                                                 "NameAcquired","/org/freedesktop/DBus", client->name,
                                                                            G_DBUS_SIGNAL_FLAGS_NONE, on_name_lost_or_acquired, client,NULL);
  }
  client_unref(client);
}
static void on_connection_disconnected(GDBusConnection *connection, gboolean remote_peer_vanished, GError *error, gpointer user_data) {
  Client *client = user_data;
  if (client->disconnected_signal_handler_id > 0) g_signal_handler_disconnect(client->connection, client->disconnected_signal_handler_id);
  if (client->name_acquired_subscription_id > 0) g_dbus_connection_signal_unsubscribe(client->connection, client->name_acquired_subscription_id);
  if (client->name_lost_subscription_id > 0) g_dbus_connection_signal_unsubscribe(client->connection, client->name_lost_subscription_id);
  g_object_unref(client->connection);
  client->disconnected_signal_handler_id = 0;
  client->name_acquired_subscription_id = 0;
  client->name_lost_subscription_id = 0;
  client->connection = NULL;
  call_lost_handler(client);
}
static void has_connection(Client *client) {
  client->disconnected_signal_handler_id = g_signal_connect(client->connection,"closed",G_CALLBACK(on_connection_disconnected),
                                                            client);
  g_dbus_connection_call (client->connection,"org.freedesktop.DBus","/org/freedesktop/DBus","org.freedesktop.DBus",
                          "RequestName",g_variant_new("(su)", client->name, client->flags), G_VARIANT_TYPE("(u)"),
                          G_DBUS_CALL_FLAGS_NONE,-1,NULL, (GAsyncReadyCallback)request_name_cb, client_ref(client));
}
static void connection_get_cb(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  Client *client = user_data;
  client->connection = g_bus_get_finish(res, NULL);
  if (client->connection == NULL) {
      call_lost_handler(client);
      goto out;
  }
  if (client->bus_acquired_handler != NULL) client->bus_acquired_handler(client->connection, client->name, client->user_data);
  has_connection(client);
out:
  client_unref(client);
}
guint g_bus_own_name_on_connection(GDBusConnection *connection, const gchar *name, GBusNameOwnerFlags flags, GBusNameAcquiredCallback name_acquired_handler,
                                   GBusNameLostCallback name_lost_handler, gpointer user_data, GDestroyNotify user_data_free_func) {
  Client *client;
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection),0);
  g_return_val_if_fail(g_dbus_is_name(name) && !g_dbus_is_unique_name(name),0);
  G_LOCK(lock);
  client = g_new0(Client,1);
  client->ref_count = 1;
  client->id = next_global_id++;
  client->name = g_strdup(name);
  client->flags = flags;
  client->name_acquired_handler = name_acquired_handler;
  client->name_lost_handler = name_lost_handler;
  client->user_data = user_data;
  client->user_data_free_func = user_data_free_func;
  client->main_context = g_main_context_get_thread_default();
  if (client->main_context != NULL) g_main_context_ref(client->main_context);
  client->connection = g_object_ref(connection);
  if (map_id_to_client == NULL) map_id_to_client = g_hash_table_new(g_direct_hash, g_direct_equal);
  g_hash_table_insert(map_id_to_client, GUINT_TO_POINTER(client->id), client);
  G_UNLOCK(lock);
  has_connection(client);
  return client->id;
}
guint g_bus_own_name(GBusType bus_type, const gchar *name, GBusNameOwnerFlags flags, GBusAcquiredCallback bus_acquired_handler,
                     GBusNameAcquiredCallback name_acquired_handler, GBusNameLostCallback name_lost_handler, gpointer user_data,
                     GDestroyNotify user_data_free_func) {
  Client *client;
  g_return_val_if_fail(g_dbus_is_name(name) && !g_dbus_is_unique_name(name), 0);
  G_LOCK (lock);
  client = g_new0(Client, 1);
  client->ref_count = 1;
  client->id = next_global_id++;
  client->name = g_strdup(name);
  client->flags = flags;
  client->bus_acquired_handler = bus_acquired_handler;
  client->name_acquired_handler = name_acquired_handler;
  client->name_lost_handler = name_lost_handler;
  client->user_data = user_data;
  client->user_data_free_func = user_data_free_func;
  client->main_context = g_main_context_get_thread_default();
  if (client->main_context != NULL) g_main_context_ref(client->main_context);
  if (map_id_to_client == NULL) map_id_to_client = g_hash_table_new(g_direct_hash, g_direct_equal);
  g_hash_table_insert(map_id_to_client, GUINT_TO_POINTER(client->id), client);
  g_bus_get(bus_type,NULL, connection_get_cb, client_ref(client));
  G_UNLOCK(lock);
  return client->id;
}
typedef struct {
  GClosure *bus_acquired_closure;
  GClosure *name_acquired_closure;
  GClosure *name_lost_closure;
} OwnNameData;
static OwnNameData *own_name_data_new(GClosure *bus_acquired_closure, GClosure *name_acquired_closure, GClosure *name_lost_closure) {
  OwnNameData *data;
  data = g_new0(OwnNameData, 1);
  if (bus_acquired_closure != NULL) {
      data->bus_acquired_closure = g_closure_ref(bus_acquired_closure);
      g_closure_sink(bus_acquired_closure);
      if (G_CLOSURE_NEEDS_MARSHAL(bus_acquired_closure)) g_closure_set_marshal(bus_acquired_closure, NULL);
  }
  if (name_acquired_closure != NULL) {
      data->name_acquired_closure = g_closure_ref(name_acquired_closure);
      g_closure_sink(name_acquired_closure);
      if (G_CLOSURE_NEEDS_MARSHAL(name_acquired_closure)) g_closure_set_marshal(name_acquired_closure, NULL);
  }
  if (name_lost_closure != NULL) {
      data->name_lost_closure = g_closure_ref(name_lost_closure);
      g_closure_sink(name_lost_closure);
      if (G_CLOSURE_NEEDS_MARSHAL(name_lost_closure)) g_closure_set_marshal(name_lost_closure, NULL);
  }
  return data;
}
static void own_with_closures_on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data) {
  OwnNameData *data = user_data;
  GValue params[2] = { { 0, }, { 0, } };
  g_value_init(&params[0], G_TYPE_DBUS_CONNECTION);
  g_value_set_object(&params[0], connection);
  g_value_init(&params[1], G_TYPE_STRING);
  g_value_set_string(&params[1], name);
  g_closure_invoke(data->bus_acquired_closure, NULL, 2, params, NULL);
  g_value_unset(params + 0);
  g_value_unset(params + 1);
}
static void own_with_closures_on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data) {
  OwnNameData *data = user_data;
  GValue params[2] = { { 0, }, { 0, } };
  g_value_init(&params[0], G_TYPE_DBUS_CONNECTION);
  g_value_set_object(&params[0], connection);
  g_value_init(&params[1], G_TYPE_STRING);
  g_value_set_string(&params[1], name);
  g_closure_invoke(data->name_acquired_closure, NULL, 2, params, NULL);
  g_value_unset(params + 0);
  g_value_unset(params + 1);
}
static void own_with_closures_on_name_lost(GDBusConnection *connection, const gchar *name, gpointer user_data) {
  OwnNameData *data = user_data;
  GValue params[2] = { { 0, }, { 0, } };
  g_value_init(&params[0], G_TYPE_DBUS_CONNECTION);
  g_value_set_object(&params[0], connection);
  g_value_init(&params[1], G_TYPE_STRING);
  g_value_set_string(&params[1], name);
  g_closure_invoke(data->name_lost_closure, NULL, 2, params, NULL);
  g_value_unset(params + 0);
  g_value_unset(params + 1);
}
static void bus_own_name_free_func(gpointer user_data) {
  OwnNameData *data = user_data;
  if (data->bus_acquired_closure != NULL) g_closure_unref(data->bus_acquired_closure);
  if (data->name_acquired_closure != NULL) g_closure_unref(data->name_acquired_closure);
  if (data->name_lost_closure != NULL) g_closure_unref(data->name_lost_closure);
  g_free(data);
}
guint g_bus_own_name_with_closures(GBusType bus_type, const gchar *name, GBusNameOwnerFlags flags, GClosure *bus_acquired_closure, GClosure *name_acquired_closure,
                                   GClosure *name_lost_closure) {
  return g_bus_own_name(bus_type, name, flags, bus_acquired_closure != NULL ? own_with_closures_on_bus_acquired : NULL,
                        name_acquired_closure != NULL ? own_with_closures_on_name_acquired : NULL,
                        name_lost_closure != NULL ? own_with_closures_on_name_lost : NULL,
                        own_name_data_new(bus_acquired_closure, name_acquired_closure, name_lost_closure),
                        bus_own_name_free_func);
}
guint g_bus_own_name_on_connection_with_closures(GDBusConnection *connection, const gchar *name, GBusNameOwnerFlags flags, GClosure *name_acquired_closure,
                                                 GClosure *name_lost_closure) {
  return g_bus_own_name_on_connection(connection, name, flags, name_acquired_closure != NULL ? own_with_closures_on_name_acquired : NULL,
                                      name_lost_closure != NULL ? own_with_closures_on_name_lost : NULL, own_name_data_new(NULL, name_acquired_closure,
                                      name_lost_closure), bus_own_name_free_func);
}
void g_bus_unown_name(guint owner_id) {
  Client *client;
  g_return_if_fail(owner_id > 0);
  client = NULL;
  G_LOCK(lock);
  if (owner_id == 0 || map_id_to_client == NULL || (client = g_hash_table_lookup (map_id_to_client, GUINT_TO_POINTER(owner_id))) == NULL) {
      g_warning("Invalid id %d passed to g_bus_unown_name()", owner_id);
      goto out;
  }
  client->cancelled = TRUE;
  g_warn_if_fail(g_hash_table_remove (map_id_to_client, GUINT_TO_POINTER(owner_id)));
out:
  G_UNLOCK(lock);
  if (client != NULL) {
      if (client->needs_release && client->connection != NULL) {
          GVariant *result;
          GError *error;
          guint32 release_name_reply;
          error = NULL;
          result = g_dbus_connection_call_sync(client->connection,"org.freedesktop.DBus","/org/freedesktop/DBus","org.freedesktop.DBus",
                                               "ReleaseName",g_variant_new("(s)", client->name), G_VARIANT_TYPE("(u)"),
                                               G_DBUS_CALL_FLAGS_NONE,-1,NULL, &error);
          if (result == NULL) {
              g_warning ("Error releasing name %s: %s", client->name, error->message);
              g_error_free (error);
          } else {
              g_variant_get (result, "(u)", &release_name_reply);
              if (release_name_reply != 1) g_warning ("Unexpected reply %d when releasing name %s", release_name_reply, client->name);
              g_variant_unref (result);
          }
      }
      if (client->disconnected_signal_handler_id > 0) g_signal_handler_disconnect (client->connection, client->disconnected_signal_handler_id);
      if (client->name_acquired_subscription_id > 0) g_dbus_connection_signal_unsubscribe (client->connection, client->name_acquired_subscription_id);
      if (client->name_lost_subscription_id > 0) g_dbus_connection_signal_unsubscribe (client->connection, client->name_lost_subscription_id);
      client->disconnected_signal_handler_id = 0;
      client->name_acquired_subscription_id = 0;
      client->name_lost_subscription_id = 0;
      if (client->connection != NULL) {
          g_object_unref (client->connection);
          client->connection = NULL;
      }
      client_unref (client);
  }
}