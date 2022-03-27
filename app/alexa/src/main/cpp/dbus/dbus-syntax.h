#if defined (DBUS_INSIDE_DBUS_H) && !defined (DBUS_COMPILATION)
#error "Only <dbus/dbus.h> can be included directly, this file may disappear or change contents."
#endif

#ifndef DBUS_SYNTAX_H
#define DBUS_SYNTAX_H

#include "dbus-macros.h"
#include "dbus-types.h"
#include "dbus-errors.h"

DBUS_BEGIN_DECLS
DBUS_EXPORT dbus_bool_t dbus_validate_path(const char *path, DBusError *error);
DBUS_EXPORT dbus_bool_t dbus_validate_interface(const char *name, DBusError *error);
DBUS_EXPORT dbus_bool_t dbus_validate_member(const char *name, DBusError *error);
DBUS_EXPORT dbus_bool_t dbus_validate_error_name(const char *name, DBusError *error);
DBUS_EXPORT dbus_bool_t dbus_validate_bus_name(const char *name, DBusError *error);
DBUS_EXPORT dbus_bool_t dbus_validate_utf8(const char *alleged_utf8, DBusError *error);
DBUS_END_DECLS

#endif