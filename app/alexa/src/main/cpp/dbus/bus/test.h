#ifndef BUS_TEST_H
#define BUS_TEST_H

#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include "../dbus.h"
#include "../dbus-string.h"
#include "connection.h"

dbus_bool_t bus_dispatch_test(const DBusString *test_data_dir);
dbus_bool_t bus_dispatch_sha1_test(const DBusString *test_data_dir);
dbus_bool_t bus_config_parser_test(const DBusString *test_data_dir);
dbus_bool_t bus_config_parser_trivial_test(const DBusString *test_data_dir);
dbus_bool_t bus_signals_test(const DBusString *test_data_dir);
dbus_bool_t bus_expire_list_test(const DBusString *test_data_dir);
dbus_bool_t bus_activation_service_reload_test (const DBusString *test_data_dir);
dbus_bool_t bus_setup_debug_client(DBusConnection *connection);
void bus_test_clients_foreach(BusConnectionForeachFunction function, void *data);
dbus_bool_t bus_test_client_listed(DBusConnection *connection);
void bus_test_run_bus_loop(BusContext *context, dbus_bool_t block);
void bus_test_run_clients_loop(dbus_bool_t block);
void bus_test_run_everything(BusContext *context);
BusContext* bus_context_new_test(const DBusString *test_data_dir, const char *filename);
#ifdef HAVE_UNIX_FD_PASSING
dbus_bool_t bus_unix_fds_passing_test(const DBusString *test_data_dir);
#endif
#endif

#endif