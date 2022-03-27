#ifndef BUS_APPARMOR_H
#define BUS_APPARMOR_H

#include "../dbus.h"
#include "bus.h"

dbus_bool_t bus_apparmor_pre_init(void);
dbus_bool_t bus_apparmor_set_mode_from_config(const char *mode, DBusError *error);
dbus_bool_t bus_apparmor_full_init(DBusError *error);
void bus_apparmor_shutdown(void);
dbus_bool_t bus_apparmor_enabled(void);
void bus_apparmor_confinement_unref(BusAppArmorConfinement *confinement);
void bus_apparmor_confinement_ref(BusAppArmorConfinement *confinement);
BusAppArmorConfinement* bus_apparmor_init_connection_confinement(DBusConnection *connection, DBusError *error);
dbus_bool_t bus_apparmor_allows_acquire_service(DBusConnection *connection, const char *bustype, const char *service_name, DBusError *error);
dbus_bool_t bus_apparmor_allows_send(DBusConnection *sender, DBusConnection *proposed_recipient, dbus_bool_t requested_reply, const char *bustype, int msgtype,
                                     const char *path, const char *interface, const char *member, const char *error_name, const char *destination, const char *source,
                                     BusActivationEntry *activation_entry, DBusError *error);
dbus_bool_t bus_apparmor_allows_eavesdropping(DBusConnection *connection, const char *bustype, DBusError *error);

#endif