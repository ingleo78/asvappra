#include <stdlib.h>
#include "../../../glib/glib.h"
#include "../../config.h"
#include "../../dbus.h"
#include "../../dbus-sysdeps.h"
#include "../../test/test-utils-glib.h"

typedef struct {
    int dummy;
} Fixture;
static void setup (Fixture *f, gconstpointer data) {}
#define MESSAGE "regression test for _dbus_log(): "
static void test_syslog_normal(Fixture *f, gconstpointer data) {
  /*if (g_test_subprocess()) {
      _dbus_init_system_log("test-syslog",DBUS_LOG_FLAGS_SYSTEM_LOG | DBUS_LOG_FLAGS_STDERR);
      _dbus_log(DBUS_SYSTEM_LOG_INFO, MESSAGE "%d", 42);
      _dbus_log(DBUS_SYSTEM_LOG_WARNING, MESSAGE "%d", 45);
      _dbus_log(DBUS_SYSTEM_LOG_SECURITY, MESSAGE "%d", 666);
      _dbus_log(DBUS_SYSTEM_LOG_ERROR, MESSAGE "%d", 23);
      _dbus_init_system_log("test-syslog-stderr", DBUS_LOG_FLAGS_STDERR);
      _dbus_log(DBUS_SYSTEM_LOG_INFO, MESSAGE "this should not appear in the syslog");
      _dbus_init_system_log("test-syslog-both",DBUS_LOG_FLAGS_SYSTEM_LOG | DBUS_LOG_FLAGS_STDERR);
      _dbus_log(DBUS_SYSTEM_LOG_INFO, MESSAGE "this should appear in the syslog and on stderr");
      _dbus_init_system_log("test-syslog-only", DBUS_LOG_FLAGS_SYSTEM_LOG);
      _dbus_log(DBUS_SYSTEM_LOG_INFO, MESSAGE "this should appear in the syslog only");
      exit(0);
  }*/
  //g_test_trap_subprocess(NULL, 0, 0);
  g_test_trap_assert_passed();
  g_test_trap_assert_stderr("*" MESSAGE "42\n*" MESSAGE "45\n*" MESSAGE "666\n*" MESSAGE "23\n*test-syslog-stderr*" MESSAGE "this should not appear in "
                            "the syslog\n*test-syslog-both*" MESSAGE "this should appear in the syslog and on stderr\n");
  g_test_trap_assert_stderr_unmatched("*this should appear in the syslog only*");
  g_test_trap_assert_stderr_unmatched("*test-syslog-only*");
}
static void teardown(Fixture *f, gconstpointer data) {}
int main(int argc, char **argv) {
  test_init(&argc, &argv);
  g_test_add("/syslog/normal", Fixture, NULL, setup, test_syslog_normal, teardown);
  return g_test_run();
}