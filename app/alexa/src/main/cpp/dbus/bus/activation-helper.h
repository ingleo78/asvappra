#ifndef BUS_ACTIVATION_HELPER_H
#define BUS_ACTIVATION_HELPER_H

#include "../dbus.h"

dbus_bool_t run_launch_helper(const char *bus_name, DBusError *error);

#endif