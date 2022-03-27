#include <stdio.h>
#include <stdlib.h>
#include "../config.h"
#include "../dbus-internals.h"
#include "../dbus-misc.h"
#include "../dbus-test-tap.h"
#include "test.h"
#include "activation-helper.h"

#if defined(DBUS_ENABLE_EMBEDDED_TESTS) || !defined(DBUS_UNIX)
#error This file is only relevant for the embedded tests on Unix
#endif
#ifndef ACTIVATION_LAUNCHER_DO_OOM
static dbus_bool_t bus_activation_helper_oom_test(void *data, dbus_bool_t have_memory) {
  const char *service;
  DBusError error;
  dbus_bool_t retval;
  service = (const char*)data;
  retval = TRUE;
  dbus_error_init(&error);
  if (!run_launch_helper(service, &error)) {
      _DBUS_ASSERT_ERROR_IS_SET(&error);
      if (!dbus_error_has_name(&error, DBUS_ERROR_NO_MEMORY)) {
          _dbus_warn("FAILED SELF TEST: Error: %s", error.message);
          retval = FALSE;
      }
      dbus_error_free(&error);
  } else { _DBUS_ASSERT_ERROR_IS_CLEAR(&error); }
  return retval;
}
#endif
int main(int argc, char **argv) {
  const char *dir;
  DBusString config_file;
  if (argc > 1 && strcmp(argv[1], "--tap") != 0) dir = argv[1];
  else dir = _dbus_getenv("DBUS_TEST_DATA");
  if (dir == NULL) _dbus_test_fatal("Must specify test data directory as argv[1] or in DBUS_TEST_DATA env variable");
  _dbus_test_diag("%s: Running launch helper OOM checks", argv[0]);
  if (!_dbus_string_init(&config_file) || !_dbus_string_append(&config_file, dir) || !_dbus_string_append(&config_file, "/valid-config-files-system/debug-allow-all-pass.conf"))
      _dbus_test_fatal("OOM during initialization");
  dbus_setenv("TEST_LAUNCH_HELPER_CONFIG", _dbus_string_get_const_data(&config_file));
  _dbus_string_free(&config_file);
  if (!_dbus_test_oom_handling("dbus-daemon-launch-helper", bus_activation_helper_oom_test,"org.freedesktop.DBus.TestSuiteEchoService"))
      _dbus_test_fatal("OOM test failed");
  _dbus_test_ok("%s", argv[0]);
  _dbus_test_check_memleaks(argv[0]);
  return _dbus_test_done_testing();
}