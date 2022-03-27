#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../gio.h"
#include "gdbus-tests.h"

static GMainLoop *loop = NULL;
G_GNUC_UNUSED static void _log(const gchar *format, ...) {
  GTimeVal now;
  time_t now_time;
  struct tm *now_tm;
  gchar time_buf[128];
  gchar *str;
  va_list var_args;
  va_start(var_args, format);
  str = g_strdup_vprintf(format, var_args);
  va_end(var_args);
  g_get_current_time(&now);
  now_time = (time_t)now.tv_sec;
  now_tm = localtime(&now_time);
  strftime(time_buf, sizeof time_buf, "%H:%M:%S", now_tm);
  g_print("%s.%06d: %s\n", time_buf, (gint)now.tv_usec / 1000, str);
  g_free(str);
}
static gboolean test_connection_quit_mainloop(gpointer user_data) {
  volatile gboolean *quit_mainloop_fired = user_data;
  *quit_mainloop_fired = TRUE;
  g_main_loop_quit(loop);
  return TRUE;
}
static const GDBusInterfaceInfo boo_interface_info = {
  -1,
  "org.example.Boo",
  (GDBusMethodInfo**)NULL,
  (GDBusSignalInfo**)NULL,
  (GDBusPropertyInfo**)NULL,
  NULL,
};
static const GDBusInterfaceVTable boo_vtable = {
  NULL, /* _method_call */
  NULL, /* _get_property */
  NULL  /* _set_property */
};
static GDBusMessage *some_filter_func(GDBusConnection *connection, GDBusMessage *message, gboolean incoming, gpointer user_data) {
  return message;
}
static void on_name_owner_changed(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path, const gchar *interface_name,
                                  const gchar *signal_name, GVariant *parameters, gpointer user_data) {}
static void a_gdestroynotify_that_sets_a_gboolean_to_true_and_quits_loop(gpointer user_data) {
  volatile gboolean *val = user_data;
  *val = TRUE;
  g_main_loop_quit(loop);
}
static void test_connection_life_cycle(void) {
  gboolean ret;
  GDBusConnection *c;
  GDBusConnection *c2;
  GError *error;
  volatile gboolean on_signal_registration_freed_called;
  volatile gboolean on_filter_freed_called;
  volatile gboolean on_register_object_freed_called;
  volatile gboolean quit_mainloop_fired;
  guint quit_mainloop_id;
  guint registration_id;
  error = NULL;
  c = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  _g_assert_error_domain(error, G_IO_ERROR);
  g_assert(!g_dbus_error_is_remote_error(error));
  g_assert(c == NULL);
  g_error_free(error);
  session_bus_up();
  error = NULL;
  c = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);
  g_assert(c != NULL);
  g_assert(!g_dbus_connection_is_closed(c));
  error = NULL;
  c2 = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);
  g_assert(c2 != NULL);
  g_assert(c == c2);
  g_object_unref(c2);
  c2 = _g_bus_get_priv(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);
  g_assert(c2 != NULL);
  g_assert(c != c2);
  g_object_unref(c2);
  c2 = _g_bus_get_priv(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);
  g_assert(c2 != NULL);
  g_assert(!g_dbus_connection_is_closed(c2));
  ret = g_dbus_connection_close_sync(c2, NULL, &error);
  g_assert_no_error(error);
  g_assert(ret);
  _g_assert_signal_received(c2, "closed");
  g_assert(g_dbus_connection_is_closed (c2));
  ret = g_dbus_connection_close_sync(c2, NULL, &error);
  g_assert_error(error, G_IO_ERROR, G_IO_ERROR_CLOSED);
  g_error_free(error);
  g_assert(!ret);
  g_object_unref(c2);
  error = NULL;
  c2 = _g_bus_get_priv(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);
  g_assert(c2 != NULL);
  on_signal_registration_freed_called = FALSE;
  g_dbus_connection_signal_subscribe(c2, "org.freedesktop.DBus", "org.freedesktop.DBus", "NameOwnerChanged", "/org/freesktop/DBus", NULL, G_DBUS_SIGNAL_FLAGS_NONE,
                                     on_name_owner_changed, (gpointer)&on_signal_registration_freed_called,
                                     a_gdestroynotify_that_sets_a_gboolean_to_true_and_quits_loop);
  on_filter_freed_called = FALSE;
  g_dbus_connection_add_filter(c2, some_filter_func, (gpointer) &on_filter_freed_called, a_gdestroynotify_that_sets_a_gboolean_to_true_and_quits_loop);
  on_register_object_freed_called = FALSE;
  error = NULL;
  registration_id = g_dbus_connection_register_object(c2, "/foo", (GDBusInterfaceInfo*)&boo_interface_info, &boo_vtable, (gpointer)&on_register_object_freed_called,
                                                      a_gdestroynotify_that_sets_a_gboolean_to_true_and_quits_loop, &error);
  g_assert_no_error(error);
  g_assert(registration_id > 0);
  g_object_unref(c2);
  quit_mainloop_fired = FALSE;
  quit_mainloop_id = g_timeout_add(30000, test_connection_quit_mainloop, (gpointer)&quit_mainloop_fired);
  while(TRUE) {
      if (on_signal_registration_freed_called && on_filter_freed_called && on_register_object_freed_called) break;
      if (quit_mainloop_fired) break;
      g_main_loop_run(loop);
  }
  g_source_remove(quit_mainloop_id);
  g_assert(on_signal_registration_freed_called);
  g_assert(on_filter_freed_called);
  g_assert(on_register_object_freed_called);
  g_assert(!quit_mainloop_fired);
  g_assert(!g_dbus_connection_is_closed(c));
  g_dbus_connection_set_exit_on_close(c, FALSE);
  session_bus_down();
  if (!g_dbus_connection_is_closed(c)) _g_assert_signal_received(c, "closed");
  g_assert(g_dbus_connection_is_closed(c));
  _g_object_wait_for_single_ref(c);
  g_object_unref(c);
}
static void msg_cb_expect_error_disconnected(GDBusConnection *connection, GAsyncResult *res, gpointer user_data) {
  GError *error;
  GVariant *result;
  error = NULL;
  result = g_dbus_connection_call_finish(connection, res, &error);
  g_assert_error(error, G_IO_ERROR, G_IO_ERROR_CLOSED);
  g_assert(!g_dbus_error_is_remote_error(error));
  g_error_free(error);
  g_assert(result == NULL);
  g_main_loop_quit(loop);
}
static void msg_cb_expect_error_unknown_method(GDBusConnection *connection, GAsyncResult *res, gpointer user_data) {
  GError *error;
  GVariant *result;
  error = NULL;
  result = g_dbus_connection_call_finish(connection, res, &error);
  g_assert_error(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD);
  g_assert(g_dbus_error_is_remote_error(error));
  g_error_free(error);
  g_assert(result == NULL);
  g_main_loop_quit(loop);
}
static void msg_cb_expect_success(GDBusConnection *connection, GAsyncResult *res, gpointer user_data) {
  GError *error;
  GVariant *result;
  error = NULL;
  result = g_dbus_connection_call_finish(connection, res, &error);
  g_assert_no_error(error);
  g_assert(result != NULL);
  g_variant_unref(result);
  g_main_loop_quit(loop);
}
static void msg_cb_expect_error_cancelled(GDBusConnection *connection, GAsyncResult *res, gpointer user_data) {
  GError *error;
  GVariant *result;
  error = NULL;
  result = g_dbus_connection_call_finish(connection, res, &error);
  g_assert_error(error, G_IO_ERROR, G_IO_ERROR_CANCELLED);
  g_assert(!g_dbus_error_is_remote_error(error));
  g_error_free(error);
  g_assert(result == NULL);
  g_main_loop_quit(loop);
}
static void msg_cb_expect_error_cancelled_2(GDBusConnection *connection, GAsyncResult *res, gpointer user_data) {
  GError *error;
  GVariant *result;
  error = NULL;
  result = g_dbus_connection_call_finish(connection, res, &error);
  g_assert_error(error, G_IO_ERROR, G_IO_ERROR_CANCELLED);
  g_assert(!g_dbus_error_is_remote_error(error));
  g_error_free(error);
  g_assert(result == NULL);
  g_main_loop_quit(loop);
}
static void test_connection_send(void) {
  GDBusConnection *c;
  GCancellable *ca;
  session_bus_up();
  c = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
  g_assert(c != NULL);
  g_assert(!g_dbus_connection_is_closed(c));
  ca = g_cancellable_new();
  g_cancellable_cancel(ca);
  g_dbus_connection_call(c, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, ca,
                         (GAsyncReadyCallback)msg_cb_expect_error_cancelled, NULL);
  g_main_loop_run(loop);
  g_object_unref(ca);
  g_dbus_connection_call(c, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                         (GAsyncReadyCallback)msg_cb_expect_success, NULL);
  g_main_loop_run(loop);
  g_dbus_connection_call(c, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "NonExistantMethod", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1,
                         NULL, (GAsyncReadyCallback)msg_cb_expect_error_unknown_method, NULL);
  g_main_loop_run(loop);
  ca = g_cancellable_new();
  g_dbus_connection_call(c, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, ca,
                         (GAsyncReadyCallback)msg_cb_expect_error_cancelled_2, NULL);
  g_cancellable_cancel(ca);
  g_main_loop_run(loop);
  g_object_unref(ca);
  g_dbus_connection_set_exit_on_close(c, FALSE);
  session_bus_down();
  _g_assert_signal_received(c, "closed");
  g_assert(g_dbus_connection_is_closed(c));
  g_dbus_connection_call(c, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId", NULL, NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                         (GAsyncReadyCallback) msg_cb_expect_error_disconnected, NULL);
  g_main_loop_run(loop);
  _g_object_wait_for_single_ref(c);
  g_object_unref(c);
}
static void test_connection_signal_handler(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path, const gchar *interface_name,
                                           const gchar *signal_name, GVariant *parameters, gpointer user_data) {
  gint *counter = user_data;
  *counter += 1;
  g_main_loop_quit(loop);
}
static void test_connection_signals(void) {
  GDBusConnection *c1;
  GDBusConnection *c2;
  GDBusConnection *c3;
  guint s1;
  guint s1b;
  guint s2;
  guint s3;
  gint count_s1;
  gint count_s1b;
  gint count_s2;
  gint count_name_owner_changed;
  GError *error;
  gboolean ret;
  GVariant *result;
  error = NULL;
  session_bus_up();
  if (g_getenv("G_DBUS_MONITOR") == NULL) {
      c1 = _g_bus_get_priv(G_BUS_TYPE_SESSION, NULL, NULL);
      g_assert(c1 != NULL);
      g_assert(!g_dbus_connection_is_closed(c1));
      g_object_unref(c1);
  }
  c1 = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
  g_assert(c1 != NULL);
  g_assert(!g_dbus_connection_is_closed(c1));
  g_assert_cmpstr(g_dbus_connection_get_unique_name(c1), ==, ":1.1");
  s1 = g_dbus_connection_signal_subscribe(c1, ":1.2", "org.gtk.GDBus.ExampleInterface", "Foo", "/org/gtk/GDBus/ExampleInterface", NULL, G_DBUS_SIGNAL_FLAGS_NONE,
                                          test_connection_signal_handler, &count_s1, NULL);
  s2 = g_dbus_connection_signal_subscribe(c1, NULL, "org.gtk.GDBus.ExampleInterface", "Foo", "/org/gtk/GDBus/ExampleInterface", NULL, G_DBUS_SIGNAL_FLAGS_NONE,
                                          test_connection_signal_handler, &count_s2, NULL);
  s3 = g_dbus_connection_signal_subscribe(c1, "org.freedesktop.DBus", "org.freedesktop.DBus", "NameOwnerChanged", "/org/freedesktop/DBus", NULL,
                                          G_DBUS_SIGNAL_FLAGS_NONE, test_connection_signal_handler, &count_name_owner_changed, NULL);
  s1b = g_dbus_connection_signal_subscribe(c1, ":1.2", "org.gtk.GDBus.ExampleInterface", "Foo", "/org/gtk/GDBus/ExampleInterface", NULL, G_DBUS_SIGNAL_FLAGS_NONE,
                                           test_connection_signal_handler, &count_s1b, NULL);
  g_assert(s1 != 0);
  g_assert(s1b != 0);
  g_assert(s2 != 0);
  g_assert(s3 != 0);
  count_s1 = 0;
  count_s1b = 0;
  count_s2 = 0;
  count_name_owner_changed = 0;
  c2 = _g_bus_get_priv(G_BUS_TYPE_SESSION, NULL, NULL);
  g_assert(c2 != NULL);
  g_assert(!g_dbus_connection_is_closed(c2));
  g_assert_cmpstr(g_dbus_connection_get_unique_name(c2), ==, ":1.2");
  c3 = _g_bus_get_priv(G_BUS_TYPE_SESSION, NULL, NULL);
  g_assert(c3 != NULL);
  g_assert(!g_dbus_connection_is_closed(c3));
  g_assert_cmpstr(g_dbus_connection_get_unique_name(c3), ==, ":1.3");
  result = g_dbus_connection_call_sync(c1, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetId", NULL, NULL, G_DBUS_CALL_FLAGS_NONE,
                                       -1, NULL, &error);
  g_assert_no_error(error);
  g_assert(result != NULL);
  g_variant_unref(result);
  ret = g_dbus_connection_emit_signal(c2, NULL, "/org/gtk/GDBus/ExampleInterface", "org.gtk.GDBus.ExampleInterface", "Foo", NULL, &error);
  g_assert_no_error(error);
  g_assert(ret);
  while(!(count_s1 >= 1 && count_s2 >= 1)) g_main_loop_run(loop);
  g_assert_cmpint(count_s1, ==, 1);
  g_assert_cmpint(count_s2, ==, 1);
  ret = g_dbus_connection_emit_signal(c3, NULL, "/org/gtk/GDBus/ExampleInterface", "org.gtk.GDBus.ExampleInterface", "Foo", NULL, &error);
  g_assert_no_error(error);
  g_assert(ret);
  while(!(count_s1 == 1 && count_s2 == 2)) g_main_loop_run(loop);
  g_assert_cmpint(count_s1, ==, 1);
  g_assert_cmpint(count_s2, ==, 2);
  gboolean quit_mainloop_fired;
  guint quit_mainloop_id;
  quit_mainloop_fired = FALSE;
  quit_mainloop_id = g_timeout_add(30000, test_connection_quit_mainloop, &quit_mainloop_fired);
  while(count_name_owner_changed < 2 && !quit_mainloop_fired) g_main_loop_run (loop);
  g_source_remove(quit_mainloop_id);
  g_assert_cmpint(count_s1, ==, 1);
  g_assert_cmpint(count_s2, ==, 2);
  g_assert_cmpint(count_name_owner_changed, ==, 2);
  g_dbus_connection_signal_unsubscribe(c1, s1);
  g_dbus_connection_signal_unsubscribe (c1, s2);
  g_dbus_connection_signal_unsubscribe (c1, s3);
  g_dbus_connection_signal_unsubscribe (c1, s1b);
  _g_object_wait_for_single_ref (c1);
  _g_object_wait_for_single_ref (c2);
  _g_object_wait_for_single_ref (c3);
  g_object_unref (c1);
  g_object_unref (c2);
  g_object_unref (c3);
  session_bus_down ();
}
typedef struct {
  guint num_handled;
  guint num_outgoing;
  guint32 serial;
} FilterData;
static GDBusMessage *filter_func(GDBusConnection *connection, GDBusMessage *message, gboolean incoming, gpointer user_data) {
  FilterData *data = user_data;
  guint32 reply_serial;
  if (incoming) {
      reply_serial = g_dbus_message_get_reply_serial (message);
      if (reply_serial == data->serial) data->num_handled += 1;
  } else data->num_outgoing += 1;
  return message;
}
typedef struct {
  gboolean alter_incoming;
  gboolean alter_outgoing;
} FilterEffects;
static GDBusMessage *other_filter_func(GDBusConnection *connection, GDBusMessage *message, gboolean incoming, gpointer user_data) {
  FilterEffects *effects = user_data;
  GDBusMessage *ret;
  gboolean alter;
  if (incoming) alter = effects->alter_incoming;
  else alter = effects->alter_outgoing;
  if (alter) {
      GDBusMessage *copy;
      GVariant *body;
      gchar *s;
      gchar *s2;
      copy = g_dbus_message_copy(message, NULL);
      g_object_unref(message);
      body = g_dbus_message_get_body(copy);
      g_variant_get(body, "(s)", &s);
      s2 = g_strdup_printf("MOD: %s", s);
      g_dbus_message_set_body(copy, g_variant_new("(s)", s2));
      g_free(s2);
      g_free(s);
      ret = copy;
  } else ret = message;
  return ret;
}
static void test_connection_filter_name_owner_changed_signal_handler(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path,
                                                                     const gchar *interface_name, const gchar *signal_name, GVariant *parameters,
                                                                     gpointer user_data) {
  const gchar *name;
  const gchar *old_owner;
  const gchar *new_owner;
  g_variant_get(parameters,"(&s&s&s)", &name, &old_owner, &new_owner);
  if (g_strcmp0(name, "com.example.TestService") == 0 && strlen(new_owner) > 0) g_main_loop_quit(loop);
}
static gboolean test_connection_filter_on_timeout(gpointer user_data) {
  g_printerr("Timeout waiting 30 sec on service\n");
  g_assert_not_reached ();
  return FALSE;
}
static void test_connection_filter (void) {
  GDBusConnection *c;
  FilterData data;
  GDBusMessage *m;
  GDBusMessage *m2;
  GDBusMessage *r;
  GError *error;
  guint filter_id;
  guint timeout_mainloop_id;
  guint signal_handler_id;
  FilterEffects effects;
  GVariant *result;
  const gchar *s;
  memset(&data, '\0', sizeof(FilterData));
  session_bus_up();
  error = NULL;
  c = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);
  g_assert(c != NULL);
  filter_id = g_dbus_connection_add_filter(c, filter_func, &data, NULL);
  m = g_dbus_message_new_method_call("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetNameOwner");
  g_dbus_message_set_body(m, g_variant_new("(s)", "org.freedesktop.DBus"));
  error = NULL;
  g_dbus_connection_send_message(c, m, G_DBUS_SEND_MESSAGE_FLAGS_NONE, &data.serial, &error);
  g_assert_no_error(error);
  while(data.num_handled == 0) g_thread_yield();
  m2 = g_dbus_message_copy(m, &error);
  g_assert_no_error(error);
  g_dbus_connection_send_message(c, m2, G_DBUS_SEND_MESSAGE_FLAGS_NONE, &data.serial, &error);
  g_object_unref(m2);
  g_assert_no_error(error);
  while(data.num_handled == 1) g_thread_yield();
  m2 = g_dbus_message_copy(m, &error);
  g_assert_no_error(error);
  g_dbus_message_set_serial(m2, data.serial);
  g_dbus_message_lock(m2);
  g_dbus_connection_send_message(c, m2, G_DBUS_SEND_MESSAGE_FLAGS_PRESERVE_SERIAL, &data.serial, &error);
  g_object_unref(m2);
  g_assert_no_error(error);
  while(data.num_handled == 2) g_thread_yield();
  m2 = g_dbus_message_copy(m, &error);
  g_assert_no_error(error);
  r = g_dbus_connection_send_message_with_reply_sync(c, m2, G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, &data.serial, NULL, &error);
  g_object_unref(m2);
  g_assert_no_error(error);
  g_assert(r != NULL);
  g_object_unref(r);
  g_assert_cmpint(data.num_handled, ==, 4);
  g_dbus_connection_remove_filter(c, filter_id);
  m2 = g_dbus_message_copy(m, &error);
  g_assert_no_error(error);
  r = g_dbus_connection_send_message_with_reply_sync(c, m2, G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, &data.serial, NULL, &error);
  g_object_unref(m2);
  g_assert_no_error(error);
  g_assert(r != NULL);
  g_object_unref(r);
  g_assert_cmpint(data.num_handled, ==, 4);
  g_assert_cmpint(data.num_outgoing, ==, 4);
  //g_assert(g_spawn_command_line_async(SRCDIR "/gdbus-testserver.py", NULL));
  signal_handler_id = g_dbus_connection_signal_subscribe(c, "org.freedesktop.DBus", "org.freedesktop.DBus", "NameOwnerChanged", "/org/freedesktop/DBus", NULL,
                                                         G_DBUS_SIGNAL_FLAGS_NONE, test_connection_filter_name_owner_changed_signal_handler, NULL, NULL);
  g_assert_cmpint(signal_handler_id, !=, 0);
  timeout_mainloop_id = g_timeout_add(30000, test_connection_filter_on_timeout, NULL);
  g_main_loop_run(loop);
  g_source_remove(timeout_mainloop_id);
  g_dbus_connection_signal_unsubscribe(c, signal_handler_id);
  filter_id = g_dbus_connection_add_filter(c, other_filter_func, &effects, NULL);
  effects.alter_incoming = FALSE;
  effects.alter_outgoing = FALSE;
  error = NULL;
  result = g_dbus_connection_call_sync(c, "com.example.TestService", "/com/example/TestObject", "com.example.Frob", "HelloWorld", g_variant_new("(s)", "Cat"),
                                       G_VARIANT_TYPE("(s)"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error(error);
  g_variant_get(result, "(&s)", &s);
  g_assert_cmpstr(s, ==, "You greeted me with 'Cat'. Thanks!");
  g_variant_unref(result);
  effects.alter_incoming = TRUE;
  effects.alter_outgoing = TRUE;
  error = NULL;
  result = g_dbus_connection_call_sync(c, "com.example.TestService", "/com/example/TestObject", "com.example.Frob", "HelloWorld", g_variant_new("(s)", "Cat"),
                                       G_VARIANT_TYPE("(s)"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error(error);
  g_variant_get(result, "(&s)", &s);
  g_assert_cmpstr(s, ==, "MOD: You greeted me with 'MOD: Cat'. Thanks!");
  g_variant_unref(result);
  g_dbus_connection_remove_filter(c, filter_id);
  _g_object_wait_for_single_ref(c);
  g_object_unref(c);
  g_object_unref(m);
  session_bus_down();
}
static void test_connection_basic(void) {
  GDBusConnection *connection;
  GError *error;
  GDBusCapabilityFlags flags;
  gchar *guid;
  gchar *name;
  gboolean closed;
  gboolean exit_on_close;
  GIOStream *stream;
  GCredentials *credentials;
  session_bus_up();
  error = NULL;
  connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);
  g_assert(connection != NULL);
  flags = g_dbus_connection_get_capabilities(connection);
  g_assert(flags == G_DBUS_CAPABILITY_FLAGS_NONE || flags == G_DBUS_CAPABILITY_FLAGS_UNIX_FD_PASSING);
  credentials = g_dbus_connection_get_peer_credentials(connection);
  g_assert(credentials == NULL);
  g_object_get(connection,"stream", &stream, "guid", &guid, "unique-name", &name, "closed", &closed, "exit-on-close", &exit_on_close,
               "capabilities", &flags, NULL);
  g_assert(G_IS_IO_STREAM(stream));
  g_assert(g_dbus_is_guid(guid));
  g_assert(g_dbus_is_unique_name(name));
  g_assert(!closed);
  g_assert(exit_on_close);
  g_assert(flags == G_DBUS_CAPABILITY_FLAGS_NONE || flags == G_DBUS_CAPABILITY_FLAGS_UNIX_FD_PASSING);
  g_object_unref(stream);
  g_free(name);
  g_free(guid);
  _g_object_wait_for_single_ref(connection);
  g_object_unref(connection);
  session_bus_down();
}
int main(int argc, char *argv[]) {
  g_type_init();
  g_test_init(&argc, &argv, NULL);
  loop = g_main_loop_new(NULL, FALSE);
  g_unsetenv("DISPLAY");
  g_setenv("DBUS_SESSION_BUS_ADDRESS", session_bus_get_temporary_address(), TRUE);
  g_test_add_func("/gdbus/connection/basic", test_connection_basic);
  g_test_add_func("/gdbus/connection/life-cycle", test_connection_life_cycle);
  g_test_add_func("/gdbus/connection/send", test_connection_send);
  g_test_add_func("/gdbus/connection/signals", test_connection_signals);
  g_test_add_func("/gdbus/connection/filter", test_connection_filter);
  return g_test_run();
}