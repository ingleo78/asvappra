#include <stdio.h>
#include <stdlib.h>
#include "../../config.h"
#include "../../dbus.h"
#include "../../dbus-internals.h"
#include "../../dbus-string.h"
#include "../test-utils.h"

static void do_test(int minimum, const char *format, ...) _DBUS_GNUC_PRINTF(2, 3);
static void do_test(int minimum, const char *format, ...) {
  va_list ap;
  int result;
  va_start(ap, format);
  result = _dbus_printf_string_upper_bound(format, ap);
  va_end(ap);
  if (result < minimum) {
      fprintf(stderr, "expected at least %d, got %d\n", minimum, result);
      abort();
  }
}
#define X_TIMES_8  "XXXXXXXX"
#define X_TIMES_16  X_TIMES_8   X_TIMES_8
#define X_TIMES_32  X_TIMES_16  X_TIMES_16
#define X_TIMES_64  X_TIMES_32  X_TIMES_32
#define X_TIMES_128  X_TIMES_64  X_TIMES_64
#define X_TIMES_256  X_TIMES_128 X_TIMES_128
#define X_TIMES_512  X_TIMES_256 X_TIMES_256
#define X_TIMES_1024  X_TIMES_512 X_TIMES_512
int main(int argc, char **argv) {
  char buf[] = X_TIMES_1024 X_TIMES_1024 X_TIMES_1024 X_TIMES_1024;
  int i;
  int test_num = 0;
  do_test(1, "%d", 0);
  printf("ok %d\n", ++test_num);
  do_test(7, "%d", 1234567);
  printf("ok %d\n", ++test_num);
  do_test(3, "%f", 3.5);
  printf("ok %d\n", ++test_num);
  do_test(0, "%s", "");
  printf("ok %d\n", ++test_num);
  do_test(1024, "%s", X_TIMES_1024);
  printf("ok %d\n", ++test_num);
  do_test(1025, "%s", X_TIMES_1024 "Y");
  printf("ok %d\n", ++test_num);
  for (i = 4096; i > 0; i--) {
      buf[i] = '\0';
      do_test(i, "%s", buf);
      do_test(i + 3, "%s:%d", buf, 42);
  }
  printf("ok %d\n", ++test_num);
  printf("1..%d\n", test_num);
  return 0;
}