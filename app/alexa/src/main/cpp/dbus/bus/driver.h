#ifndef BUS_DRIVER_H
#define BUS_DRIVER_H

#include "../dbus.h"
#include "connection.h"

typedef enum {
  BUS_DRIVER_FOUND_SELF,
  BUS_DRIVER_FOUND_PEER,
  BUS_DRIVER_FOUND_ERROR
} BusDriverFound;
void bus_driver_remove_connection(DBusConnection *connection);
dbus_bool_t bus_driver_handle_message(DBusConnection *connection, BusTransaction *transaction, DBusMessage *message, DBusError *error);
dbus_bool_t bus_driver_send_service_lost(DBusConnection *connection, const char *service_name, BusTransaction *transaction, DBusError *error);
dbus_bool_t bus_driver_send_service_acquired(DBusConnection *connection, const char *service_name, BusTransaction *transaction, DBusError *error);
dbus_bool_t bus_driver_send_service_owner_changed(const char *service_name, const char *old_owner, const char *new_owner, BusTransaction *transaction,
						                          DBusError *error);
dbus_bool_t bus_driver_generate_introspect_string(DBusString *xml, dbus_bool_t canonical_path, DBusMessage *message);
dbus_bool_t bus_driver_fill_connection_credentials(DBusConnection *conn, DBusMessageIter *asv_iter);
BusDriverFound bus_driver_get_conn_helper(DBusConnection *connection, DBusMessage *message, const char *what_we_want, const char **name_p, DBusConnection **peer_conn_p,
                                          DBusError *error);
dbus_bool_t bus_driver_send_ack_reply(DBusConnection *connection, BusTransaction *transaction, DBusMessage *message, DBusError *error);

#endif