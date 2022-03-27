#if defined (DBUS_INSIDE_DBUS_H) && !defined (DBUS_COMPILATION)
#error "Only <dbus/dbus.h> can be included directly, this file may disappear or change contents."
#endif

#ifndef DBUS_ERROR_H
#define DBUS_ERROR_H

#include "dbus-macros.h"
#include "dbus-types.h"
#include "dbus-protocol.h"

DBUS_BEGIN_DECLS
typedef struct DBusError DBusError;
struct DBusError {
  const char *name;
  const char *message;
  unsigned int dummy1 : 1;
  unsigned int dummy2 : 1;
  unsigned int dummy3 : 1;
  unsigned int dummy4 : 1;
  unsigned int dummy5 : 1;
  void *padding1;
};
#define DBUS_ERROR_INIT { NULL, NULL, TRUE, 0, 0, 0, 0, NULL }
DBUS_EXPORT void dbus_error_init(DBusError *error);
DBUS_EXPORT void dbus_error_free(DBusError *error);
DBUS_EXPORT void dbus_set_error(DBusError *error, const char *name, const char *message, ...) _DBUS_GNUC_PRINTF(3, 4);
DBUS_EXPORT void dbus_set_error_const(DBusError *error, const char *name, const char *message);
DBUS_EXPORT void dbus_move_error(DBusError *src, DBusError *dest);
DBUS_EXPORT dbus_bool_t dbus_error_has_name(const DBusError *error, const char *name);
DBUS_EXPORT dbus_bool_t dbus_error_is_set(const DBusError *error);
DBUS_END_DECLS

#endif