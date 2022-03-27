#include <stdio.h>
#include <stdlib.h>
#include "../config.h"
#include "../dbus-string.h"
#include "../dbus-sysdeps.h"
#include "../dbus-internals.h"
#include "../dbus-message-internal.h"
#include "../dbus-test-tap.h"
#include "test.h"
#include "selinux.h"

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
#error This file is only relevant for the embedded tests
#endif

#ifdef DBUS_UNIX
#include "../dbus-sysdeps-unix.h"
#endif

static const char *only;
static DBusInitialFDs *initial_fds = NULL;
static DBusString test_data_dir;
static void test_pre_hook(void) {
  if (_dbus_getenv("DBUS_TEST_SELINUX") && (!bus_selinux_pre_init() || !bus_selinux_full_init()))
      _dbus_test_fatal("Could not init selinux support");
  initial_fds = _dbus_check_fdleaks_enter();
}
static void test_post_hook(const char *name) {
  if (_dbus_getenv("DBUS_TEST_SELINUX")) bus_selinux_shutdown();
  _dbus_test_check_memleaks(name);
  _dbus_check_fdleaks_leave(initial_fds);
  initial_fds = NULL;
}
static void test_one(const char *name, dbus_bool_t (*func) (const DBusString *)) {
  if (only != NULL && strcmp(only, name) != 0) {
      _dbus_test_skip("%s - Only intending to run %s", name, only);
      return;
  }
  _dbus_test_diag("Running test: %s", name);
  test_pre_hook();
  if (func(&test_data_dir)) _dbus_test_ok("%s", name);
  else _dbus_test_not_ok("%s", name);
  test_post_hook(name);
}
int main(int argc, char **argv) {
  const char *dir;
  if (argc > 1 && strcmp(argv[1], "--tap") != 0) dir = argv[1];
  else dir = _dbus_getenv("DBUS_TEST_DATA");
  if (argc > 2) only = argv[2];
  else only = NULL;
  if (dir == NULL) _dbus_test_fatal("Must specify test data directory as argv[1] or in DBUS_TEST_DATA env variable");
  _dbus_string_init_const(&test_data_dir, dir);
#ifdef DBUS_UNIX
  _dbus_close_all();
#endif
  if (!_dbus_threads_init_debug()) _dbus_test_fatal("OOM initializing debug threads");
  test_one("expire-list", bus_expire_list_test);
  test_one("config-parser", bus_config_parser_test);
  test_one("signals", bus_signals_test);
  test_one("dispatch-sha1", bus_dispatch_sha1_test);
  test_one("dispatch", bus_dispatch_test);
  test_one("activation-service-reload", bus_activation_service_reload_test);
#ifdef HAVE_UNIX_FD_PASSING
  test_one("unix-fds-passing", bus_unix_fds_passing_test);
#else
  _dbus_test_skip("fd-passing not supported on this platform");
#endif
  return _dbus_test_done_testing();
}