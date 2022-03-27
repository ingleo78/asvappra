#ifndef BUS_STATS_H
#define BUS_STATS_H

#include "bus.h"

#define BUS_INTERFACE_STATS "org.freedesktop.DBus.Debug.Stats"
dbus_bool_t bus_stats_handle_get_stats(DBusConnection *connection, BusTransaction *transaction, DBusMessage *message, DBusError *error);
dbus_bool_t bus_stats_handle_get_connection_stats(DBusConnection *connection, BusTransaction *transaction, DBusMessage *message, DBusError *error);
dbus_bool_t bus_stats_handle_get_all_match_rules(DBusConnection *caller_connection, BusTransaction *transaction, DBusMessage *message, DBusError *error);

#endif