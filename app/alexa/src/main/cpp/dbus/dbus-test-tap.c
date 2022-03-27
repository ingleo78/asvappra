#include "config.h"
#include "dbus-test-tap.h"
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include <stdio.h>
#include <stdlib.h>

static unsigned int failures = 0;
static unsigned int tap_test_counter = 0;
void _dbus_test_fatal (const char *format, ...) {
  va_list ap;
  printf("Bail out! ");
  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
  printf("\n");
  fflush(stdout);
  exit(1);
}
void _dbus_test_diag(const char *format, ...) {
  va_list ap;
  printf("# ");
  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
  printf("\n");
  fflush(stdout);
}
void _dbus_test_skip_all(const char *format, ...) {
  va_list ap;
  _dbus_assert(tap_test_counter == 0);
  printf("1..0 # SKIP - ");
  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
  printf("\n");
  fflush(stdout);
  exit(0);
}
void _dbus_test_ok(const char *format, ...) {
  va_list ap;
  printf("ok %u - ", ++tap_test_counter);
  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
  printf("\n");
  fflush(stdout);
}
void _dbus_test_not_ok(const char *format, ...) {
  va_list ap;
  printf("not ok %u - ", ++tap_test_counter);
  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
  printf("\n");
  failures++;
  fflush(stdout);
}
void _dbus_test_skip(const char *format, ...) {
  va_list ap;
  printf("ok %u # SKIP ", ++tap_test_counter);
  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
  printf("\n");
  fflush(stdout);
}
void _dbus_test_check_memleaks(const char *test_name) {
  dbus_shutdown();
  if (_dbus_get_malloc_blocks_outstanding() == 0) printf("ok %u - %s did not leak memory\n", ++tap_test_counter, test_name);
  else {
      printf("not ok %u - %s leaked %d blocks\n", ++tap_test_counter, test_name, _dbus_get_malloc_blocks_outstanding());
      failures++;
  }
}
int _dbus_test_done_testing(void) {
  if (failures == 0) _dbus_test_diag("%u tests passed", tap_test_counter);
  else _dbus_test_diag("%u/%u tests failed", failures, tap_test_counter);
  printf("1..%u\n", tap_test_counter);
  fflush(stdout);
  if (failures == 0) return 0;
  return 1;
}
#endif