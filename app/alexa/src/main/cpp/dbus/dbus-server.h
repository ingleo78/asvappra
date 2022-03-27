#if defined (DBUS_INSIDE_DBUS_H) && !defined (DBUS_COMPILATION)
#error "Only <dbus/dbus.h> can be included directly, this file may disappear or change contents."
#endif

#ifndef DBUS_SERVER_H
#define DBUS_SERVER_H

#include "dbus-errors.h"
#include "dbus-macros.h"
#include "dbus-message.h"
#include "dbus-connection.h"
#include "dbus-protocol.h"

DBUS_BEGIN_DECLS
typedef struct DBusServer DBusServer;
typedef void (*DBusNewConnectionFunction)(DBusServer *server, DBusConnection *new_connection, void *data);
DBUS_EXPORT DBusServer* dbus_server_listen(const char *address, DBusError *error);
DBUS_EXPORT DBusServer* dbus_server_ref(DBusServer *server);
DBUS_EXPORT void dbus_server_unref(DBusServer *server);
DBUS_EXPORT void dbus_server_disconnect(DBusServer *server);
DBUS_EXPORT dbus_bool_t dbus_server_get_is_connected(DBusServer *server);
DBUS_EXPORT char* dbus_server_get_address(DBusServer *server);
DBUS_EXPORT char* dbus_server_get_id(DBusServer *server);
DBUS_EXPORT void dbus_server_set_new_connection_function(DBusServer *server, DBusNewConnectionFunction function, void *data, DBusFreeFunction free_data_function);
DBUS_EXPORT dbus_bool_t dbus_server_set_watch_functions(DBusServer *server, DBusAddWatchFunction add_function, DBusRemoveWatchFunction remove_function,
                                                        DBusWatchToggledFunction toggled_function, void *data, DBusFreeFunction free_data_function);
DBUS_EXPORT dbus_bool_t dbus_server_set_timeout_functions(DBusServer *server, DBusAddTimeoutFunction add_function, DBusRemoveTimeoutFunction remove_function,
                                                          DBusTimeoutToggledFunction toggled_function, void *data, DBusFreeFunction free_data_function);
DBUS_EXPORT dbus_bool_t dbus_server_set_auth_mechanisms(DBusServer *server, const char **mechanisms);
DBUS_EXPORT dbus_bool_t dbus_server_allocate_data_slot(dbus_int32_t *slot_p);
DBUS_EXPORT void dbus_server_free_data_slot(dbus_int32_t *slot_p);
DBUS_EXPORT dbus_bool_t dbus_server_set_data(DBusServer *server, int slot, void *data, DBusFreeFunction free_data_func);
DBUS_EXPORT void* dbus_server_get_data(DBusServer *server, int  slot);
static inline void dbus_clear_server(DBusServer **pointer_to_server) {
  _dbus_clear_pointer_impl (DBusServer, pointer_to_server, dbus_server_unref);
}
DBUS_END_DECLS

#endif