#ifndef DBUS_SERVER_UNIX_H
#define DBUS_SERVER_UNIX_H

#include "dbus-internals.h"
#include "dbus-server-protected.h"

DBUS_BEGIN_DECLS
DBusServer* _dbus_server_new_for_domain_socket(const char *path, dbus_bool_t abstract, DBusError *error);
DBUS_END_DECLS

#endif