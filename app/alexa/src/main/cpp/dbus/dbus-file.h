#ifndef DBUS_FILE_H
#define DBUS_FILE_H

#include "dbus-string.h"
#include "dbus-errors.h"

DBUS_BEGIN_DECLS dbus_bool_t _dbus_file_exists(const char *file);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_file_get_contents(DBusString *str, const DBusString *filename, DBusError *error);
dbus_bool_t _dbus_string_save_to_file(const DBusString *str, const DBusString *filename, dbus_bool_t world_readable, DBusError *error);
dbus_bool_t _dbus_make_file_world_readable(const DBusString *filename, DBusError *error);
dbus_bool_t _dbus_create_file_exclusively(const DBusString *filename, DBusError *error);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_delete_file(const DBusString *filename, DBusError *error);
DBUS_END_DECLS

#endif