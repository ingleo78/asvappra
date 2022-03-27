#ifndef DBUS_SERVER_LAUNCHD_H
#define DBUS_SERVER_LAUNCHD_H

#include "dbus-internals.h"
#include "dbus-server-protected.h"

DBUS_BEGIN_DECLS
DBusServer *_dbus_server_new_for_launchd(const char *launchd_env_var, DBusError * error);
DBUS_END_DECLS
#endif