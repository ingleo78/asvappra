#include "config.h"
#include "dbus-misc.h"
#include "dbus-internals.h"
#include "dbus-string.h"
#include "dbus-test-tap.h"

char *dbus_try_get_local_machine_id(DBusError *error) {
  DBusString uuid;
  char *s;
  s = NULL;
  if (!_dbus_string_init(&uuid)) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      return NULL;
  }
  if (!_dbus_get_local_machine_uuid_encoded(&uuid, error)) {
      _dbus_string_free(&uuid);
      return NULL;
  }
  if (!_dbus_string_steal_data(&uuid, &s)) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      _dbus_string_free(&uuid);
      return NULL;
  } else {
      _dbus_string_free(&uuid);
      return s;
  }
}
char *dbus_get_local_machine_id(void) {
  DBusError error = DBUS_ERROR_INIT;
  char *s;
  s = dbus_try_get_local_machine_id(&error);
  if (s == NULL) {
      if (!dbus_error_has_name(&error, DBUS_ERROR_NO_MEMORY)) _dbus_warn_check_failed("%s", error.message);
      dbus_error_free(&error);
      return NULL;
  }
  return s;
}
void dbus_get_version(int *major_version_p, int *minor_version_p, int *micro_version_p) {
  if (major_version_p) *major_version_p = 1;
  if (minor_version_p) *minor_version_p = 10;
  if (micro_version_p) *micro_version_p = 10;
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "dbus-test.h"
#include <stdlib.h>

dbus_bool_t _dbus_misc_test(void) {
  int major, minor, micro;
  DBusString str;
  dbus_get_version(NULL, NULL, NULL);
  dbus_get_version(&major, &minor, &micro);
  _dbus_assert(major == DBUS_MAJOR_VERSION);
  _dbus_assert(minor == DBUS_MINOR_VERSION);
  _dbus_assert(micro == DBUS_MICRO_VERSION);
#define MAKE_VERSION(x, y, z) (((x) << 16) | ((y) << 8) | (z))
  _dbus_assert(MAKE_VERSION(1, 0, 0) > MAKE_VERSION(0, 0, 0));
  _dbus_assert(MAKE_VERSION(1, 1, 0) > MAKE_VERSION(1, 0, 0));
  _dbus_assert(MAKE_VERSION(1, 1, 1) > MAKE_VERSION(1, 1, 0));
  _dbus_assert(MAKE_VERSION(2, 0, 0) > MAKE_VERSION(1, 1, 1));
  _dbus_assert(MAKE_VERSION(2, 1, 0) > MAKE_VERSION(1, 1, 1));
  _dbus_assert(MAKE_VERSION(2, 1, 1) > MAKE_VERSION(1, 1, 1));
  _dbus_assert(MAKE_VERSION(major, minor, micro) == DBUS_VERSION);
  _dbus_assert(MAKE_VERSION(major - 1, minor, micro) < DBUS_VERSION);
  _dbus_assert(MAKE_VERSION(major, minor - 1, micro) < DBUS_VERSION);
  _dbus_assert(MAKE_VERSION(major, minor, micro - 1) < DBUS_VERSION);
  _dbus_assert(MAKE_VERSION(major + 1, minor, micro) > DBUS_VERSION);
  _dbus_assert(MAKE_VERSION(major, minor + 1, micro) > DBUS_VERSION);
  _dbus_assert(MAKE_VERSION(major, minor, micro + 1) > DBUS_VERSION);
  if (!_dbus_string_init(&str)) _dbus_test_fatal("no memory");
  if (!(_dbus_string_append_int(&str, major) && _dbus_string_append_byte(&str, '.') && _dbus_string_append_int(&str, minor) &&
      _dbus_string_append_byte(&str, '.') && _dbus_string_append_int(&str, micro)))
      _dbus_test_fatal("no memory");
  _dbus_assert(_dbus_string_equal_c_str(&str, DBUS_VERSION_STRING));
  _dbus_string_free(&str);
  return TRUE;
}

#endif
#endif