#include <unistd.h>
#include <string.h>
#include "../gio.h"
#include "gdbus-tests.h"

static GMainLoop *loop = NULL;
static void test_methods(GDBusProxy *proxy) {
  GVariant *result;
  GError *error;
  const gchar *str;
  gchar *dbus_error_name;
  error = NULL;
  result = g_dbus_proxy_call_sync(proxy, "HelloWorld", g_variant_new("(s)", "Hey"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error(error);
  g_assert(result != NULL);
  g_assert_cmpstr(g_variant_get_type_string(result), ==, "(s)");
  g_variant_get(result, "(&s)", &str);
  g_assert_cmpstr(str, ==, "You greeted me with 'Hey'. Thanks!");
  g_variant_unref(result);
  result = g_dbus_proxy_call_sync(proxy, "HelloWorld", g_variant_new("(s)", "Yo"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_error(error, G_IO_ERROR, G_IO_ERROR_DBUS_ERROR);
  g_assert(g_dbus_error_is_remote_error(error));
  g_assert(g_dbus_error_is_remote_error(error));
  g_assert(result == NULL);
  dbus_error_name = g_dbus_error_get_remote_error(error);
  g_assert_cmpstr(dbus_error_name, ==, "com.example.TestException");
  g_free(dbus_error_name);
  g_assert(g_dbus_error_strip_remote_error(error));
  g_assert_cmpstr(error->message, ==, "Yo is not a proper greeting");
  g_clear_error(&error);
  error = NULL;
  result = g_dbus_proxy_call_sync(proxy, "Sleep", g_variant_new("(i)", 500), G_DBUS_CALL_FLAGS_NONE, 100, NULL, &error);
  g_assert_error(error, G_IO_ERROR, G_IO_ERROR_TIMED_OUT);
  g_assert(!g_dbus_error_is_remote_error(error));
  g_assert(result == NULL);
  g_clear_error(&error);
  g_assert_cmpint(g_dbus_proxy_get_default_timeout(proxy), ==, -1);
  result = g_dbus_proxy_call_sync(proxy, "Sleep", g_variant_new("(i)", 500), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error(error);
  g_assert(result != NULL);
  g_assert_cmpstr(g_variant_get_type_string(result), ==, "()");
  g_variant_unref(result);
  g_dbus_proxy_set_default_timeout(proxy, 250);
  g_assert_cmpint(g_dbus_proxy_get_default_timeout(proxy), ==, 250);
  result = g_dbus_proxy_call_sync(proxy, "Sleep", g_variant_new("(i)", 500), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_error(error, G_IO_ERROR, G_IO_ERROR_TIMED_OUT);
  g_assert(!g_dbus_error_is_remote_error(error));
  g_assert(result == NULL);
  g_clear_error(&error);
  g_dbus_proxy_set_default_timeout(proxy, -1);
}
static gboolean strv_equal(gchar **strv, ...) {
  gint count;
  va_list list;
  const gchar *str;
  gboolean res;
  res = TRUE;
  count = 0;
  va_start(list, strv);
  while(1) {
      str = va_arg(list, const gchar*);
      if (str == NULL) break;
      if (g_strcmp0(str, strv[count]) != 0) {
          res = FALSE;
          break;
      }
      count++;
  }
  va_end(list);
  if (res) res = g_strv_length(strv) == count;
  return res;
}
static void test_properties(GDBusProxy *proxy) {
  GError *error;
  GVariant *variant;
  GVariant *variant2;
  GVariant *result;
  gchar **names;
  error = NULL;
  if (g_dbus_proxy_get_flags(proxy) & G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES) {
       g_assert(g_dbus_proxy_get_cached_property_names (proxy) == NULL);
       return;
  }
  names = g_dbus_proxy_get_cached_property_names(proxy);
  g_assert(strv_equal(names, "PropertyThatWillBeInvalidated", "ab", "ad", "ai", "an", "ao", "aq", "as", "at", "au", "ax", "ay", "b", "d", "foo", "i", "n",
           "o", "q", "s", "t", "u", "x", "y", NULL));
  g_strfreev(names);
  variant = g_dbus_proxy_get_cached_property(proxy, "y");
  g_assert(variant != NULL);
  g_assert_cmpint(g_variant_get_byte(variant), ==, 1);
  g_variant_unref(variant);
  variant = g_dbus_proxy_get_cached_property(proxy, "o");
  g_assert(variant != NULL);
  g_assert_cmpstr(g_variant_get_string(variant, NULL), ==, "/some/path");
  g_variant_unref(variant);
  variant2 = g_variant_new_byte(42);
  result = g_dbus_proxy_call_sync(proxy, "FrobSetProperty", g_variant_new("(sv)", "y", variant2), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error(error);
  g_assert(result != NULL);
  g_assert_cmpstr(g_variant_get_type_string(result), ==, "()");
  g_variant_unref(result);
  _g_assert_signal_received(proxy, "g-properties-changed");
  variant = g_dbus_proxy_get_cached_property(proxy, "y");
  g_assert(variant != NULL);
  g_assert_cmpint(g_variant_get_byte(variant), ==, 42);
  g_variant_unref(variant);
  g_dbus_proxy_set_cached_property(proxy, "y", g_variant_new_byte(142));
  variant = g_dbus_proxy_get_cached_property(proxy, "y");
  g_assert(variant != NULL);
  g_assert_cmpint(g_variant_get_byte(variant), ==, 142);
  g_variant_unref(variant);
  g_dbus_proxy_set_cached_property(proxy, "y", NULL);
  variant = g_dbus_proxy_get_cached_property(proxy, "y");
  g_assert(variant == NULL);
  variant = g_dbus_proxy_get_cached_property(proxy, "PropertyThatWillBeInvalidated");
  g_assert(variant != NULL);
  g_assert_cmpstr(g_variant_get_string(variant, NULL), ==, "InitialValue");
  g_variant_unref(variant);
  result = g_dbus_proxy_call_sync(proxy, "FrobInvalidateProperty", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error(error);
  g_assert(result != NULL);
  g_assert_cmpstr(g_variant_get_type_string(result), ==, "()");
  g_variant_unref(result);
  _g_assert_signal_received(proxy, "g-properties-changed");
  variant = g_dbus_proxy_get_cached_property(proxy, "PropertyThatWillBeInvalidated");
  g_assert(variant == NULL);
}
static void test_proxy_signals_on_signal(GDBusProxy *proxy, const gchar *sender_name, const gchar *signal_name, GVariant *parameters, gpointer user_data) {
  GString *s = user_data;
  g_assert_cmpstr(signal_name, ==, "TestSignal");
  g_assert_cmpstr(g_variant_get_type_string(parameters), ==, "(sov)");
  g_variant_print_string(parameters, s, TRUE);
}
typedef struct {
  GMainLoop *internal_loop;
  GString *s;
} TestSignalData;
static void test_proxy_signals_on_emit_signal_cb(GDBusProxy *proxy, GAsyncResult *res, gpointer user_data) {
  TestSignalData *data = user_data;
  GError *error;
  GVariant *result;
  error = NULL;
  result = g_dbus_proxy_call_finish(proxy, res, &error);
  g_assert_no_error (error);
  g_assert (result != NULL);
  g_assert_cmpstr (g_variant_get_type_string (result), ==, "()");
  g_variant_unref (result);
  g_assert (strlen (data->s->str) > 0);
  g_main_loop_quit (data->internal_loop);
}
static void test_signals(GDBusProxy *proxy) {
  GError *error;
  GString *s;
  gulong signal_handler_id;
  TestSignalData data;
  GVariant *result;
  error = NULL;
  s = g_string_new (NULL);
  signal_handler_id = g_signal_connect(proxy,"g-signal",G_CALLBACK(test_proxy_signals_on_signal), s);
  result = g_dbus_proxy_call_sync(proxy, "EmitSignal", g_variant_new("(so)", "Accept the next proposition you hear", "/some/path"), G_DBUS_CALL_FLAGS_NONE, -1,
                                  NULL, &error);
  g_assert_no_error(error);
  g_assert(result != NULL);
  g_assert_cmpstr(g_variant_get_type_string (result), ==, "()");
  g_variant_unref(result);
  g_assert(strlen(s->str) == 0);
  _g_assert_signal_received(proxy, "g-signal");
  g_assert_cmpstr(s->str, ==,"('Accept the next proposition you hear .. in bed!', objectpath '/some/path/in/bed', <'a variant'>)");
  g_signal_handler_disconnect(proxy, signal_handler_id);
  g_string_free(s, TRUE);
  s = g_string_new(NULL);
  data.internal_loop = g_main_loop_new(NULL, FALSE);
  data.s = s;
  signal_handler_id = g_signal_connect(proxy,"g-signal",G_CALLBACK(test_proxy_signals_on_signal), s);
  g_dbus_proxy_call(proxy, "EmitSignal", g_variant_new("(so)", "You will make a great programmer", "/some/other/path"), G_DBUS_CALL_FLAGS_NONE, -1, NULL,
                    (GAsyncReadyCallback)test_proxy_signals_on_emit_signal_cb, &data);
  g_main_loop_run (data.internal_loop);
  g_main_loop_unref(data.internal_loop);
  g_assert_cmpstr(s->str, ==,"('You will make a great programmer .. in bed!', objectpath '/some/other/path/in/bed', <'a variant'>)");
  g_signal_handler_disconnect (proxy, signal_handler_id);
  g_string_free (s, TRUE);
}
static void test_bogus_method_return(GDBusProxy *proxy) {
  GError *error = NULL;
  GVariant *result;
  result = g_dbus_proxy_call_sync(proxy, "PairReturn", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT);
  g_error_free(error);
  g_assert(result == NULL);
}
static const gchar *frob_dbus_interface_xml =
  "<node>"
  "  <interface name='com.example.Frob'>"
  "    <method name='PairReturn'>"
  "      <arg type='u' name='somenumber' direction='in'/>"
  "      <arg type='s' name='somestring' direction='out'/>"
  "    </method>"
  "    <method name='HelloWorld'>"
  "      <arg type='s' name='somestring' direction='in'/>"
  "      <arg type='s' name='somestring' direction='out'/>"
  "    </method>"
  "    <method name='Sleep'>"
  "      <arg type='i' name='timeout' direction='in'/>"
  "    </method>"
  "    <property name='y' type='y' access='readwrite'/>"
  "  </interface>"
  "</node>";
static GDBusInterfaceInfo *frob_dbus_interface_info;
static void test_expected_interface(GDBusProxy *proxy) {
  g_dbus_proxy_set_cached_property(proxy, "y", g_variant_new_string ("error_me_out!"));
  g_dbus_proxy_set_cached_property(proxy, "y", g_variant_new_byte(42));
  g_dbus_proxy_set_cached_property(proxy, "does-not-exist", g_variant_new_string("something"));
  g_dbus_proxy_set_cached_property(proxy, "does-not-exist", NULL);
  g_dbus_proxy_set_interface_info(proxy, frob_dbus_interface_info);
  test_methods(proxy);
  test_bogus_method_return(proxy);
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR)) {
      g_dbus_proxy_set_cached_property(proxy, "y", g_variant_new_string("error_me_out!"));
  }
  g_test_trap_assert_stderr("*Trying to set property y of type s but according to the expected interface the type is y*");
  g_test_trap_assert_failed();
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR)) {
      g_dbus_proxy_set_cached_property(proxy, "does-not-exist", g_variant_new_string("something"));
  }
  g_test_trap_assert_stderr("*Trying to lookup property does-not-exist which isn't in expected interface com.example.Frob*");
  g_test_trap_assert_failed();
  g_dbus_proxy_set_cached_property(proxy, "y", g_variant_new_byte(42));
}
static void test_basic(GDBusProxy *proxy) {
  GDBusConnection *connection;
  GDBusConnection *conn;
  GDBusProxyFlags flags;
  GDBusInterfaceInfo *info;
  gchar *name;
  gchar *path;
  gchar *interface;
  gint timeout;
  connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
  g_assert(g_dbus_proxy_get_connection(proxy) == connection);
  g_assert(g_dbus_proxy_get_flags(proxy) == G_DBUS_PROXY_FLAGS_NONE);
  g_assert(g_dbus_proxy_get_interface_info(proxy) == NULL);
  g_assert_cmpstr(g_dbus_proxy_get_name(proxy), ==, "com.example.TestService");
  g_assert_cmpstr(g_dbus_proxy_get_object_path(proxy), ==, "/com/example/TestObject");
  g_assert_cmpstr(g_dbus_proxy_get_interface_name(proxy), ==, "com.example.Frob");
  g_assert_cmpint(g_dbus_proxy_get_default_timeout(proxy), ==, -1);
  g_object_get(proxy,"g-connection", &conn, "g-interface-info", &info, "g-flags", &flags, "g-name", &name, "g-object-path", &path,
               "g-interface-name", &interface, "g-default-timeout", &timeout, NULL);
  g_assert(conn == connection);
  g_assert(info == NULL);
  g_assert_cmpint(flags, ==, G_DBUS_PROXY_FLAGS_NONE);
  g_assert_cmpstr(name, ==, "com.example.TestService");
  g_assert_cmpstr(path, ==, "/com/example/TestObject");
  g_assert_cmpstr(interface, ==, "com.example.Frob");
  g_assert_cmpint(timeout, ==, -1);
  g_object_unref(conn);
  g_free(name);
  g_free(path);
  g_free(interface);
  g_object_unref(connection);
}
static void test_proxy(void) {
  GDBusProxy *proxy;
  GDBusConnection *connection;
  GError *error;
  session_bus_up();
  usleep(500 * 1000);
  error = NULL;
  connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);
  error = NULL;
  proxy = g_dbus_proxy_new_sync(connection, G_DBUS_PROXY_FLAGS_NONE, NULL, "com.example.TestService", "/com/example/TestObject", "com.example.Frob", NULL, &error);
  g_assert_no_error (error);
  //g_assert (g_spawn_command_line_async (SRCDIR "/gdbus-testserver.py", NULL));
  _g_assert_property_notify (proxy, "g-name-owner");
  test_basic (proxy);
  test_methods (proxy);
  test_properties (proxy);
  test_signals (proxy);
  test_expected_interface (proxy);
  g_object_unref (proxy);
  g_object_unref (connection);
}
static void proxy_ready(GObject *source, GAsyncResult *result, gpointer user_data) {
  GDBusProxy *proxy;
  GError *error;
  error = NULL;
  proxy = g_dbus_proxy_new_for_bus_finish (result, &error);
  g_assert_no_error (error);
  test_basic (proxy);
  test_methods (proxy);
  test_properties (proxy);
  test_signals (proxy);
  test_expected_interface (proxy);
  g_object_unref (proxy);
}
static void test_async(void) {
  g_dbus_proxy_new_for_bus(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL, "com.example.TestService", "/com/example/TestObject", "com.example.Frob", NULL,
                           proxy_ready, NULL);
}
static void test_no_properties(void) {
  GDBusProxy *proxy;
  GError *error;
  error = NULL;
  proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES, NULL, "com.example.TestService", "/com/example/TestObject",
                                        "com.example.Frob", NULL, &error);
  g_assert_no_error(error);
  test_properties(proxy);
  g_object_unref(proxy);
}
int main(int argc, char *argv[]) {
  gint ret;
  GDBusNodeInfo *introspection_data = NULL;
  g_type_init();
  g_test_init(&argc, &argv, NULL);
  introspection_data = g_dbus_node_info_new_for_xml(frob_dbus_interface_xml, NULL);
  g_assert(introspection_data != NULL);
  frob_dbus_interface_info = introspection_data->interfaces[0];
  loop = g_main_loop_new(NULL, FALSE);
  g_unsetenv("DISPLAY");
  g_setenv("DBUS_SESSION_BUS_ADDRESS", session_bus_get_temporary_address(), TRUE);
  g_test_add_func("/gdbus/proxy", test_proxy);
  g_test_add_func("/gdbus/proxy/async", test_async);
  g_test_add_func("/gdbus/proxy/no-properties", test_no_properties);
  ret = g_test_run();
  g_dbus_node_info_unref(introspection_data);
  return ret;
}