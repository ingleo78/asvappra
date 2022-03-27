#include "../../gio/gio.h"
#include "../config.h"
#include "test-utils-glib.h"

typedef struct {
  gboolean skip;
  TestMainContext *ctx;
  DBusError e;
  GError *ge;
  GPid daemon_pid;
  DBusConnection *conn;
} Fixture;
typedef struct {
  const char *config_file;
  TestUser user;
  gboolean expect_success;
} Config;
static void setup(Fixture *f, gconstpointer context) {
  const Config *config = context;
  gchar *address;
  f->ctx = test_main_context_get();
  f->ge = NULL;
  dbus_error_init(&f->e);
  address = test_get_dbus_daemon(config ? config->config_file : NULL,TEST_USER_MESSAGEBUS, NULL, &f->daemon_pid);
  if (address == NULL) {
      f->skip = TRUE;
      return;
  }
  f->conn = test_try_connect_to_bus_as_user(f->ctx, address, config ? config->user : TEST_USER_ME, &f->ge);
  if (f->conn == NULL && g_error_matches(f->ge, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED)) {
      //g_test_skip(f->ge->message);
      g_clear_error(&f->ge);
      f->skip = TRUE;
  }
  g_assert_no_error(f->ge);
  g_free(address);
}
static void test_uae (Fixture *f, gconstpointer context) {
  const Config *config = context;
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusMessageIter args_iter;
  DBusMessageIter arr_iter;
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "UpdateActivationEnvironment");
  if (m == NULL) g_error("OOM");
  dbus_message_iter_init_append(m, &args_iter);
  if (!dbus_message_iter_open_container(&args_iter, DBUS_TYPE_ARRAY,"{ss}", &arr_iter) || !dbus_message_iter_close_container(&args_iter, &arr_iter))
      g_error("OOM");
  reply = test_main_context_call_and_wait(f->ctx, f->conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (config->expect_success) {
      g_assert_cmpint(dbus_message_get_type(reply), ==, DBUS_MESSAGE_TYPE_METHOD_RETURN);
  } else {
      g_assert_cmpint(dbus_message_get_type(reply), ==, DBUS_MESSAGE_TYPE_ERROR);
      g_assert_cmpstr(dbus_message_get_error_name(reply), ==, DBUS_ERROR_ACCESS_DENIED);
      g_assert_cmpstr(dbus_message_get_signature(reply), ==, "s");
  }
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void test_monitor(Fixture *f, gconstpointer context) {
  const Config *config = context;
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusMessageIter args_iter;
  DBusMessageIter arr_iter;
  dbus_uint32_t no_flags = 0;
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_MONITORING, "BecomeMonitor");
  if (m == NULL) g_error("OOM");
  dbus_message_iter_init_append(m, &args_iter);
  if (!dbus_message_iter_open_container(&args_iter, DBUS_TYPE_ARRAY,"s", &arr_iter) || !dbus_message_iter_close_container(&args_iter, &arr_iter) ||
      !dbus_message_iter_append_basic(&args_iter, DBUS_TYPE_UINT32, &no_flags))
      g_error("OOM");
  reply = test_main_context_call_and_wait(f->ctx, f->conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (config->expect_success) { g_assert_cmpint(dbus_message_get_type(reply), ==, DBUS_MESSAGE_TYPE_METHOD_RETURN); }
  else {
      g_assert_cmpint(dbus_message_get_type(reply), ==, DBUS_MESSAGE_TYPE_ERROR);
      g_assert_cmpstr(dbus_message_get_error_name(reply), ==, DBUS_ERROR_ACCESS_DENIED);
      g_assert_cmpstr(dbus_message_get_signature(reply), ==, "s");
  }
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void teardown(Fixture *f, gconstpointer context G_GNUC_UNUSED) {
  dbus_error_free(&f->e);
  g_clear_error(&f->ge);
  if (f->conn != NULL) {
      dbus_connection_close(f->conn);
      dbus_connection_unref(f->conn);
      f->conn = NULL;
  }
  if (f->daemon_pid != 0) {
      test_kill_pid(f->daemon_pid);
      g_spawn_close_pid(f->daemon_pid);
      f->daemon_pid = 0;
  }
  test_main_context_unref(f->ctx);
}
static Config root_ok_config = {
    "valid-config-files/multi-user.conf",
    TEST_USER_ROOT,
    TRUE
};
static Config messagebus_ok_config = {
    "valid-config-files/multi-user.conf",
    TEST_USER_MESSAGEBUS,
    TRUE
};
static Config other_fail_config = {
    "valid-config-files/multi-user.conf",
    TEST_USER_OTHER,
    FALSE
};
int main(int argc, char **argv) {
  test_init(&argc, &argv);
  g_test_add("/uid-permissions/uae/other", Fixture, &other_fail_config, setup, test_uae, teardown);
  g_test_add("/uid-permissions/monitor/root", Fixture, &root_ok_config, setup, test_monitor, teardown);
  g_test_add("/uid-permissions/monitor/messagebus", Fixture, &messagebus_ok_config, setup, test_monitor, teardown);
  g_test_add("/uid-permissions/monitor/other", Fixture, &other_fail_config, setup, test_monitor, teardown);
  return g_test_run();
}