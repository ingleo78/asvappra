#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../gio.h"
#include "gdbus-tests.h"

static GMainLoop *loop = NULL;
static void
test_connection_flush_signal_handler(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path, const gchar *interface_name,
                                     const gchar *signal_name, GVariant *parameters, gpointer user_data) {
  g_main_loop_quit(loop);
}
static gboolean test_connection_flush_on_timeout(gpointer user_data) {
  guint iteration = GPOINTER_TO_UINT(user_data);
  g_printerr("Timeout waiting 1000 msec on iteration %d\n", iteration);
  g_assert_not_reached();
  return FALSE;
}
static void test_connection_flush(void) {
  GDBusConnection *connection;
  GError *error;
  guint n;
  guint signal_handler_id;
  session_bus_up();
  error = NULL;
  connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);
  g_assert(connection != NULL);
  signal_handler_id = g_dbus_connection_signal_subscribe(connection, NULL, "org.gtk.GDBus.FlushInterface", "SomeSignal", "/org/gtk/GDBus/FlushObject", NULL,
                                                         G_DBUS_SIGNAL_FLAGS_NONE, test_connection_flush_signal_handler, NULL, NULL);
  g_assert_cmpint(signal_handler_id, !=, 0);
  for (n = 0; n < 50; n++) {
      gboolean ret;
      gint exit_status;
      guint timeout_mainloop_id;
      error = NULL;
      ret = g_spawn_command_line_sync("./gdbus-connection-flush-helper", NULL, NULL, &exit_status, &error);
      g_assert_no_error(error);
      g_assert(WIFEXITED(exit_status));
      g_assert_cmpint(WEXITSTATUS(exit_status), ==, 0);
      g_assert(ret);
      timeout_mainloop_id = g_timeout_add(1000, test_connection_flush_on_timeout, GUINT_TO_POINTER(n));
      g_main_loop_run(loop);
      g_source_remove(timeout_mainloop_id);
  }
  g_dbus_connection_signal_unsubscribe(connection, signal_handler_id);
  _g_object_wait_for_single_ref(connection);
  g_object_unref(connection);
  session_bus_down();
}
#define LARGE_MESSAGE_STRING_LENGTH  (20*1024*1024)
static void large_message_on_name_appeared(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer user_data) {
  GError *error;
  gchar *request;
  const gchar *reply;
  GVariant *result;
  guint n;
  request = g_new (gchar, LARGE_MESSAGE_STRING_LENGTH + 1);
  for (n = 0; n < LARGE_MESSAGE_STRING_LENGTH; n++) request[n] = '0' + (n%10);
  request[n] = '\0';
  error = NULL;
  result = g_dbus_connection_call_sync(connection, "com.example.TestService", "/com/example/TestObject", "com.example.Frob", "HelloWorld",
                                       g_variant_new("(s)", request), G_VARIANT_TYPE ("(s)"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error(error);
  g_assert(result != NULL);
  g_variant_get(result, "(&s)", &reply);
  g_assert_cmpint(strlen(reply), >, LARGE_MESSAGE_STRING_LENGTH);
  g_assert(g_str_has_prefix(reply, "You greeted me with '01234567890123456789012"));
  g_assert(g_str_has_suffix(reply, "6789'. Thanks!"));
  g_variant_unref(result);
  g_free(request);
  g_main_loop_quit(loop);
}
static void large_message_on_name_vanished(GDBusConnection *connection, const gchar *name, gpointer user_data) {}
static void test_connection_large_message(void) {
  guint watcher_id;
  session_bus_up();
  //g_assert(g_spawn_command_line_async (SRCDIR "/gdbus-testserver.py", NULL));
  watcher_id = g_bus_watch_name(G_BUS_TYPE_SESSION, "com.example.TestService", G_BUS_NAME_WATCHER_FLAGS_NONE, large_message_on_name_appeared,
                                 large_message_on_name_vanished, NULL, NULL);
  g_main_loop_run(loop);
  g_bus_unwatch_name(watcher_id);
  session_bus_down();
}
int main(int argc, char *argv[]) {
  g_type_init();
  g_test_init(&argc, &argv, NULL);
  loop = g_main_loop_new(NULL, FALSE);
  g_unsetenv("DISPLAY");
  g_setenv("DBUS_SESSION_BUS_ADDRESS", session_bus_get_temporary_address(), TRUE);
  g_test_add_func("/gdbus/connection/flush", test_connection_flush);
  g_test_add_func("/gdbus/connection/large_message", test_connection_large_message);
  return g_test_run();
}