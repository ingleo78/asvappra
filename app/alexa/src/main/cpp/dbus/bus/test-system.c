#include <stdio.h>
#include <stdlib.h>
#include "../config.h"
#include "../dbus-string.h"
#include "../dbus-sysdeps.h"
#include "../dbus-internals.h"
#include "../dbus-test-tap.h"
#include "test.h"

#if defined(DBUS_ENABLE_EMBEDDED_TESTS) || !defined(DBUS_UNIX)
#error This file is only relevant for the embedded tests on Unix
#endif

static void test_pre_hook(void) {}
static const char *progname = "";
static void test_post_hook(void) {
  _dbus_test_check_memleaks(progname);
}
int main(int argc, char **argv) {
  const char *dir;
  DBusString test_data_dir;
  progname = argv[0];
  if (argc > 1 && strcmp(argv[1], "--tap") != 0) dir = argv[1];
  else dir = _dbus_getenv("DBUS_TEST_DATA");
  if (dir == NULL) _dbus_test_fatal("Must specify test data directory as argv[1] or in DBUS_TEST_DATA env variable");
  _dbus_string_init_const(&test_data_dir, dir);
  if (!_dbus_threads_init_debug()) _dbus_test_fatal("OOM initializing debug threads");
  test_pre_hook();
  _dbus_test_diag("%s: Running config file parser (trivial) test", argv[0]);
  if (!bus_config_parser_trivial_test(&test_data_dir)) _dbus_test_fatal("OOM creating parser");
  _dbus_test_ok("%s", argv[0]);
  test_post_hook();
  return _dbus_test_done_testing();
}