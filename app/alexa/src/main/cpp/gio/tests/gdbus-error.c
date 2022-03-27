#include <unistd.h>
#include <string.h>
#include "../gio.h"

static void check_registered_error(const gchar *given_dbus_error_name, GQuark error_domain, gint error_code) {
  GError *error;
  gchar *dbus_error_name;
  error = g_dbus_error_new_for_dbus_error(given_dbus_error_name, "test message");
  g_assert_error(error, error_domain, error_code);
  g_assert(g_dbus_error_is_remote_error (error));
  g_assert(g_dbus_error_strip_remote_error (error));
  g_assert_cmpstr(error->message, ==, "test message");
  dbus_error_name = g_dbus_error_get_remote_error(error);
  g_assert_cmpstr(dbus_error_name, ==, given_dbus_error_name);
  g_free(dbus_error_name);
  g_error_free(error);
}
static void test_registered_errors(void) {
  check_registered_error("org.freedesktop.DBus.Error.Failed", G_DBUS_ERROR, G_DBUS_ERROR_FAILED);
  check_registered_error("org.freedesktop.DBus.Error.AddressInUse", G_DBUS_ERROR, G_DBUS_ERROR_ADDRESS_IN_USE);
  check_registered_error("org.freedesktop.DBus.Error.UnknownMethod", G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD);
}
static void check_unregistered_error(const gchar *given_dbus_error_name) {
  GError *error;
  gchar *dbus_error_name;
  error = g_dbus_error_new_for_dbus_error(given_dbus_error_name, "test message");
  g_assert_error(error, G_IO_ERROR, G_IO_ERROR_DBUS_ERROR);
  g_assert(g_dbus_error_is_remote_error(error));
  dbus_error_name = g_dbus_error_get_remote_error(error);
  g_assert_cmpstr(dbus_error_name, ==, given_dbus_error_name);
  g_free(dbus_error_name);
  g_assert(g_dbus_error_strip_remote_error(error));
  g_assert_cmpstr(error->message, ==, "test message");
  g_assert(g_dbus_error_get_remote_error (error) == NULL);
  g_error_free(error);
}
static void test_unregistered_errors(void) {
  check_unregistered_error("com.example.Error.Failed");
  check_unregistered_error("foobar.buh");
}
static void check_transparent_gerror(GQuark error_domain, gint error_code) {
  GError *error;
  gchar *given_dbus_error_name;
  gchar *dbus_error_name;
  error = g_error_new(error_domain, error_code,"test message");
  given_dbus_error_name = g_dbus_error_encode_gerror(error);
  g_assert(g_str_has_prefix(given_dbus_error_name, "org.gtk.GDBus.UnmappedGError.Quark"));
  g_error_free(error);
  error = g_dbus_error_new_for_dbus_error(given_dbus_error_name, "test message");
  g_assert_error(error, error_domain, error_code);
  g_assert(g_dbus_error_is_remote_error(error));
  dbus_error_name = g_dbus_error_get_remote_error(error);
  g_assert_cmpstr(dbus_error_name, ==, given_dbus_error_name);
  g_free(dbus_error_name);
  g_free(given_dbus_error_name);
  g_assert(g_dbus_error_strip_remote_error(error));
  g_assert_cmpstr(error->message, ==, "test message");
  g_assert(g_dbus_error_get_remote_error(error) == NULL);
  g_error_free(error);
}
static void test_transparent_gerror(void) {
  check_transparent_gerror(G_IO_ERROR, G_IO_ERROR_FAILED);
  check_transparent_gerror(G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_PARSE);
}
typedef enum {
  TEST_ERROR_FAILED,
  TEST_ERROR_BLA
} TestError;
GDBusErrorEntry test_error_entries[] = {
  { TEST_ERROR_FAILED, "org.gtk.test.Error.Failed" },
  { TEST_ERROR_BLA,    "org.gtk.test.Error.Bla"    }
};
static void test_register_error(void) {
  gsize test_error_quark = 0;
  gboolean res;
  gchar *msg;
  GError *error;
  //g_dbus_error_register_error_domain("test-error-quark", &test_error_quark, test_error_entries, G_N_ELEMENTS (test_error_entries));
  g_assert_cmpint(test_error_quark, !=, 0);
  error = g_dbus_error_new_for_dbus_error("org.gtk.test.Error.Failed", "Failed");
  g_assert_error(error, test_error_quark, TEST_ERROR_FAILED);
  res = g_dbus_error_is_remote_error(error);
  msg = g_dbus_error_get_remote_error(error);
  g_assert(res);
  g_assert_cmpstr(msg, ==, "org.gtk.test.Error.Failed");
  res = g_dbus_error_strip_remote_error(error);
  g_assert(res);
  g_assert_cmpstr(error->message, ==, "Failed");
  g_clear_error(&error);
  g_free(msg);
  g_dbus_error_set_dbus_error(&error, "org.gtk.test.Error.Failed", "Failed again", "Prefix %d", 1);
  res = g_dbus_error_is_remote_error(error);
  msg = g_dbus_error_get_remote_error(error);
  g_assert(res);
  g_assert_cmpstr(msg, ==, "org.gtk.test.Error.Failed");
  res = g_dbus_error_strip_remote_error(error);
  g_assert(res);
  g_assert_cmpstr(error->message, ==, "Prefix 1: Failed again");
  g_clear_error(&error);
  g_free(msg);
  error = g_error_new_literal(G_IO_ERROR, G_IO_ERROR_NOT_EMPTY, "Not Empty");
  res = g_dbus_error_is_remote_error(error);
  msg = g_dbus_error_get_remote_error(error);
  g_assert(!res);
  g_assert_cmpstr(msg, ==, NULL);
  res = g_dbus_error_strip_remote_error(error);
  g_assert(!res);
  g_assert_cmpstr(error->message, ==, "Not Empty");
  g_clear_error(&error);
  error = g_error_new_literal(test_error_quark, TEST_ERROR_BLA, "Bla");
  msg = g_dbus_error_encode_gerror(error);
  g_assert_cmpstr(msg, ==, "org.gtk.test.Error.Bla");
  g_free(msg);
  g_clear_error(&error);
  res = g_dbus_error_unregister_error(test_error_quark, TEST_ERROR_BLA, "org.gtk.test.Error.Bla");
  g_assert(res);
}
int main(int argc, char *argv[]) {
  g_type_init();
  g_test_init(&argc, &argv, NULL);
  g_test_add_func("/gdbus/registered-errors", test_registered_errors);
  g_test_add_func("/gdbus/unregistered-errors", test_unregistered_errors);
  g_test_add_func("/gdbus/transparent-gerror", test_transparent_gerror);
  g_test_add_func("/gdbus/register-error", test_register_error);
  return g_test_run();
}