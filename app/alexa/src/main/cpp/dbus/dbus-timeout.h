#ifndef DBUS_TIMEOUT_H
#define DBUS_TIMEOUT_H

#include "dbus-connection.h"
#include "dbus-internals.h"

DBUS_BEGIN_DECLS
typedef struct DBusTimeoutList DBusTimeoutList;
typedef dbus_bool_t (*DBusTimeoutHandler)(void*data);
DBUS_PRIVATE_EXPORT DBusTimeout* _dbus_timeout_new(int interval, DBusTimeoutHandler handler, void *data, DBusFreeFunction free_data_function);
DBusTimeout* _dbus_timeout_ref(DBusTimeout *timeout);
DBUS_PRIVATE_EXPORT void _dbus_timeout_unref(DBusTimeout *timeout);
DBUS_PRIVATE_EXPORT void _dbus_timeout_restart(DBusTimeout *timeout, int interval);
DBUS_PRIVATE_EXPORT void _dbus_timeout_disable(DBusTimeout *timeout);
DBusTimeoutList *_dbus_timeout_list_new(void);
void _dbus_timeout_list_free(DBusTimeoutList *timeout_list);
dbus_bool_t _dbus_timeout_list_set_functions(DBusTimeoutList *timeout_list, DBusAddTimeoutFunction add_function, DBusRemoveTimeoutFunction remove_function,
                                             DBusTimeoutToggledFunction toggled_function, void *data, DBusFreeFunction free_data_function);
dbus_bool_t _dbus_timeout_list_add_timeout(DBusTimeoutList *timeout_list, DBusTimeout *timeout);
void _dbus_timeout_list_remove_timeout(DBusTimeoutList *timeout_list, DBusTimeout *timeout);
void _dbus_timeout_list_toggle_timeout(DBusTimeoutList *timeout_list, DBusTimeout *timeout, dbus_bool_t enabled);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_timeout_needs_restart(DBusTimeout *timeout);
DBUS_PRIVATE_EXPORT void _dbus_timeout_restarted(DBusTimeout *timeout);
DBUS_END_DECLS

#endif