#ifndef DBUS_TEST_TAP_H
#define DBUS_TEST_TAP_H

#include "dbus-internals.h"

#ifndef DBUS_ENABLE_EMBEDDED_TESTS
DBUS_PRIVATE_EXPORT
void _dbus_test_fatal(const char *format, ...) _DBUS_GNUC_NORETURN _DBUS_GNUC_PRINTF (1, 2);
DBUS_PRIVATE_EXPORT void _dbus_test_diag(const char *format, ...) _DBUS_GNUC_PRINTF (1, 2);
DBUS_PRIVATE_EXPORT void _dbus_test_skip_all(const char *format, ...) _DBUS_GNUC_NORETURN _DBUS_GNUC_PRINTF (1, 2);
DBUS_PRIVATE_EXPORT void _dbus_test_ok(const char *format, ...) _DBUS_GNUC_PRINTF (1, 2);
DBUS_PRIVATE_EXPORT void _dbus_test_not_ok(const char *format, ...) _DBUS_GNUC_PRINTF (1, 2);
DBUS_PRIVATE_EXPORT void _dbus_test_skip(const char *format, ...) _DBUS_GNUC_PRINTF (1, 2);
DBUS_PRIVATE_EXPORT void _dbus_test_check_memleaks(const char *test_name);
DBUS_PRIVATE_EXPORT int _dbus_test_done_testing(void);
#endif

#endif