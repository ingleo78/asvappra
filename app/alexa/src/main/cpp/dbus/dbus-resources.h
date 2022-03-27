#ifndef DBUS_RESOURCES_H
#define DBUS_RESOURCES_H

#include "dbus-macros.h"
#include "dbus-errors.h"
#include "dbus-connection.h"

DBUS_BEGIN_DECLS
typedef struct DBusCounter DBusCounter;
typedef void (*DBusCounterNotifyFunction)(DBusCounter *counter, void *user_data);
DBusCounter* _dbus_counter_new(void);
DBusCounter* _dbus_counter_ref(DBusCounter *counter);
void _dbus_counter_unref(DBusCounter *counter);
void _dbus_counter_adjust_size(DBusCounter *counter, long delta);
void _dbus_counter_adjust_unix_fd(DBusCounter *counter, long delta);
void _dbus_counter_notify(DBusCounter *counter);
long _dbus_counter_get_size_value(DBusCounter *counter);
long _dbus_counter_get_unix_fd_value(DBusCounter *counter);
void _dbus_counter_set_notify(DBusCounter *counter, long size_guard_value, long unix_fd_guard_value, DBusCounterNotifyFunction function, void *user_data);
long _dbus_counter_get_peak_size_value(DBusCounter *counter);
long _dbus_counter_get_peak_unix_fd_value(DBusCounter *counter);
DBUS_END_DECLS

#endif