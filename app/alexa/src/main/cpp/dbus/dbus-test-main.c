#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "config.h"
#include "dbus-types.h"
#include "dbus-test.h"
#include "dbus-test-tap.h"
#include "dbus-sysdeps-unix.h"

int main(int argc, char **argv) {
  const char *test_data_dir;
  const char *specific_test;
#ifdef DBUS_UNIX
  _dbus_close_all();
#endif
#if HAVE_SETLOCALE
  setlocale(LC_ALL, "");
#endif
  if (argc > 1 && strcmp(argv[1], "--tap") != 0) test_data_dir = argv[1];
  else test_data_dir = NULL;
  if (argc > 2) specific_test = argv[2];
  else specific_test = NULL;
  _dbus_run_tests(test_data_dir, specific_test);
  return _dbus_test_done_testing();
}