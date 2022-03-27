#include <unistd.h>
#include <string.h>
#include "../gio.h"
#include "gdbus-tests.h"

static GMainLoop *loop = NULL;
static void test_introspection (GDBusProxy *proxy) {
  GError *error;
  const gchar *xml_data;
  GDBusNodeInfo *node_info;
  GDBusInterfaceInfo *interface_info;
  GDBusMethodInfo *method_info;
  GDBusSignalInfo *signal_info;
  GVariant *result;
  error = NULL;
  result = g_dbus_proxy_call_sync(proxy, "org.freedesktop.DBus.Introspectable.Introspect", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_assert_no_error(error);
  g_assert(result != NULL);
  g_variant_get(result, "(&s)", &xml_data);
  node_info = g_dbus_node_info_new_for_xml(xml_data, &error);
  g_assert_no_error(error);
  g_assert(node_info != NULL);
  interface_info = g_dbus_node_info_lookup_interface(node_info, "com.example.NonExistantInterface");
  g_assert(interface_info == NULL);
  interface_info = g_dbus_node_info_lookup_interface(node_info, "org.freedesktop.DBus.Introspectable");
  g_assert(interface_info != NULL);
  method_info = g_dbus_interface_info_lookup_method(interface_info, "NonExistantMethod");
  g_assert(method_info == NULL);
  method_info = g_dbus_interface_info_lookup_method(interface_info, "Introspect");
  g_assert(method_info != NULL);
  g_assert(method_info->in_args != NULL);
  g_assert(method_info->in_args[0] == NULL);
  g_assert(method_info->out_args != NULL);
  g_assert(method_info->out_args[0] != NULL);
  g_assert(method_info->out_args[1] == NULL);
  g_assert_cmpstr(method_info->out_args[0]->signature, ==, "s");
  interface_info = g_dbus_node_info_lookup_interface(node_info, "com.example.Frob");
  g_assert(interface_info != NULL);
  signal_info = g_dbus_interface_info_lookup_signal(interface_info, "TestSignal");
  g_assert(signal_info != NULL);
  g_assert(signal_info->args != NULL);
  g_assert(signal_info->args[0] != NULL);
  g_assert_cmpstr(signal_info->args[0]->signature, ==, "s");
  g_assert(signal_info->args[1] != NULL);
  g_assert_cmpstr(signal_info->args[1]->signature, ==, "o");
  g_assert(signal_info->args[2] != NULL);
  g_assert_cmpstr(signal_info->args[2]->signature, ==, "v");
  g_assert(signal_info->args[3] == NULL);
  g_dbus_node_info_unref(node_info);
  g_variant_unref(result);
  g_main_loop_quit(loop);
}
static void test_introspection_parser(void) {
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
  g_assert_no_error(error);
  //g_assert(g_spawn_command_line_async(SRCDIR "/gdbus-testserver.py", NULL));
  _g_assert_property_notify(proxy, "g-name-owner");
  test_introspection(proxy);
  g_object_unref(proxy);
  g_object_unref(connection);
}
static void test_generate(void) {
  GDBusNodeInfo *info;
  GDBusNodeInfo *info2;
  GDBusInterfaceInfo *iinfo;
  GDBusMethodInfo *minfo;
  GDBusSignalInfo *sinfo;
  GDBusArgInfo *arginfo;
  GDBusPropertyInfo *pinfo;
  GDBusAnnotationInfo *aninfo;
  const gchar *data =
  "  <node>"
  "    <interface name='com.example.Frob'>"
  "      <annotation name='foo' value='bar'/>"
  "      <method name='PairReturn'>"
  "        <annotation name='org.freedesktop.DBus.GLib.Async' value=''/>"
  "        <arg type='u' name='somenumber' direction='in'/>"
  "        <arg type='s' name='somestring' direction='out'/>"
  "      </method>"
  "      <signal name='HelloWorld'>"
  "        <arg type='s' name='greeting' direction='out'/>"
  "      </signal>"
  "      <method name='Sleep'>"
  "        <arg type='i' name='timeout' direction='in'/>"
  "      </method>"
  "      <property name='y' type='y' access='readwrite'/>"
  "    </interface>"
  "  </node>";
  GString *string;
  GString *string2;
  GError *error;
  error = NULL;
  info = g_dbus_node_info_new_for_xml(data, &error);
  g_assert_no_error(error);
  iinfo = g_dbus_node_info_lookup_interface(info, "com.example.Frob");
  aninfo = iinfo->annotations[0];
  g_assert_cmpstr(aninfo->key, ==, "foo");
  g_assert_cmpstr(aninfo->value, ==, "bar");
  g_assert(iinfo->annotations[1] == NULL);
  minfo = g_dbus_interface_info_lookup_method(iinfo, "PairReturn");
  g_assert_cmpstr(g_dbus_annotation_info_lookup(minfo->annotations, "org.freedesktop.DBus.GLib.Async"), ==, "");
  arginfo = minfo->in_args[0];
  g_assert_cmpstr(arginfo->name, ==, "somenumber");
  g_assert_cmpstr(arginfo->signature, ==, "u");
  g_assert(minfo->in_args[1] == NULL);
  arginfo = minfo->out_args[0];
  g_assert_cmpstr(arginfo->name, ==, "somestring");
  g_assert_cmpstr(arginfo->signature, ==, "s");
  g_assert(minfo->out_args[1] == NULL);
  sinfo = g_dbus_interface_info_lookup_signal(iinfo, "HelloWorld");
  arginfo = sinfo->args[0];
  g_assert_cmpstr(arginfo->name, ==, "greeting");
  g_assert_cmpstr(arginfo->signature, ==, "s");
  g_assert(sinfo->args[1] == NULL);
  pinfo = g_dbus_interface_info_lookup_property(iinfo, "y");
  g_assert_cmpstr(pinfo->signature, ==, "y");
  g_assert_cmpint(pinfo->flags, ==, G_DBUS_PROPERTY_INFO_FLAGS_READABLE | G_DBUS_PROPERTY_INFO_FLAGS_WRITABLE);
  string = g_string_new("");
  g_dbus_node_info_generate_xml(info, 2, string);
  info2 = g_dbus_node_info_new_for_xml(string->str, &error);
  string2 = g_string_new("");
  g_dbus_node_info_generate_xml(info2, 2, string2);
  g_assert_cmpstr(string->str, ==, string2->str);
  g_string_free(string, TRUE);
  g_string_free(string2, TRUE);
  g_dbus_node_info_unref(info);
  g_dbus_node_info_unref(info2);
}
static void test_default_direction(void) {
  GDBusNodeInfo *info;
  GDBusInterfaceInfo *iinfo;
  GDBusMethodInfo *minfo;
  GDBusSignalInfo *sinfo;
  GDBusArgInfo *arginfo;
  const gchar *data =
  "  <node>"
  "    <interface name='com.example.Frob'>"
  "      <signal name='HelloWorld'>"
  "        <arg type='s' name='greeting'/>"
  "      </signal>"
  "      <method name='Sleep'>"
  "        <arg type='i' name='timeout'/>"
  "      </method>"
  "    </interface>"
  "  </node>";
  GError *error;
  error = NULL;
  info = g_dbus_node_info_new_for_xml(data, &error);
  g_assert_no_error(error);
  iinfo = g_dbus_node_info_lookup_interface(info, "com.example.Frob");
  sinfo = g_dbus_interface_info_lookup_signal(iinfo, "HelloWorld");
  g_assert(sinfo->args != NULL);
  arginfo = sinfo->args[0];
  g_assert_cmpstr(arginfo->name, ==, "greeting");
  g_assert(sinfo->args[1] == NULL);
  minfo = g_dbus_interface_info_lookup_method(iinfo, "Sleep");
  g_assert(minfo->in_args != NULL);
  arginfo = minfo->in_args[0];
  g_assert_cmpstr(arginfo->name, ==, "timeout");
  g_assert(minfo->in_args[1] == NULL);
  g_dbus_node_info_unref(info);
}
#if 0
static void test_extra_data(void) {
  GDBusNodeInfo *info;
  const gchar *data =
  "  <node>"
  "    <interface name='com.example.Frob' version='1.0'>"
  "      <annotation name='foo' value='bar' extra='bla'/>"
  "      <method name='PairReturn' anotherattribute='bla'>"
  "        <annotation name='org.freedesktop.DBus.GLib.Async' value=''/>"
  "        <arg type='u' name='somenumber' direction='in' spin='left'/>"
  "        <arg type='s' name='somestring' direction='out'/>"
  "      </method>"
  "      <signal name='HelloWorld'>"
  "        <arg type='s' name='somestring'/>"
  "      </signal>"
  "      <method name='Sleep'>"
  "        <arg type='i' name='timeout' direction='in'/>"
  "      </method>"
  "      <property name='y' type='y' access='readwrite'/>"
  "    </interface>"
  "  </node>";
  GError *error;
  error = NULL;
  info = g_dbus_node_info_new_for_xml(data, &error);
  g_assert_no_error(error);
  g_dbus_node_info_unref(info);
}
#endif
int main(int argc, char *argv[]) {
  g_type_init();
  g_test_init(&argc, &argv, NULL);
  loop = g_main_loop_new(NULL, FALSE);
  g_unsetenv("DISPLAY");
  g_setenv("DBUS_SESSION_BUS_ADDRESS", session_bus_get_temporary_address(), TRUE);
  g_test_add_func("/gdbus/introspection-parser", test_introspection_parser);
  g_test_add_func("/gdbus/introspection-generate", test_generate);
  g_test_add_func("/gdbus/introspection-default-direction", test_default_direction);
#if 0
  g_test_add_func("/gdbus/introspection-extra-data", test_extra_data);
#endif
  return g_test_run();
}