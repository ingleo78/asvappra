#ifndef BUS_CONTAINERS_H
#define BUS_CONTAINERS_H

#include "../dbus-macros.h"
#include "bus.h"

BusContainers*bus_containers_new(void);
BusContainers *bus_containers_ref(BusContainers *self);
void bus_containers_unref(BusContainers *self);
void bus_containers_stop_listening(BusContainers *self);
dbus_bool_t bus_containers_handle_add_server(DBusConnection *connection, BusTransaction *transaction, DBusMessage *message, DBusError *error);
dbus_bool_t bus_containers_handle_stop_instance(DBusConnection *connection, BusTransaction *transaction, DBusMessage *message, DBusError *error);
dbus_bool_t bus_containers_handle_stop_listening(DBusConnection *connection, BusTransaction *transaction, DBusMessage *message, DBusError *error);
dbus_bool_t bus_containers_handle_get_instance_info(DBusConnection *connection, BusTransaction *transaction, DBusMessage *message, DBusError *error);
dbus_bool_t bus_containers_handle_get_connection_instance(DBusConnection *connection, BusTransaction *transaction, DBusMessage *message, DBusError *error);
dbus_bool_t bus_containers_supported_arguments_getter(BusContext *context, DBusMessageIter *var_iter);
void bus_containers_remove_connection(BusContainers *self, DBusConnection *connection);
dbus_bool_t bus_containers_connection_is_contained(DBusConnection *connection, const char **path, const char **type, const char **name);
static inline void bus_clear_containers(BusContainers **containers_p) {
  _dbus_clear_pointer_impl(BusContainers, containers_p, bus_containers_unref);
}

#endif