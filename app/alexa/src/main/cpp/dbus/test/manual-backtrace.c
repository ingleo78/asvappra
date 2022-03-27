#include <stdio.h>
#include "../config.h"
#include "../dbus.h"
#include "../dbus-sysdeps.h"

static void test2(void) {
  _dbus_print_backtrace();
}
static void test1(void) {
  test2();
}
static void test(void) {
  test1();
}
int main(int argc, char **argv) {
  if (argc == 2) {
      fprintf(stderr, "dbus_abort test\n");
      _dbus_abort();
  } else test();
  return 0;
}