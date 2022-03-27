#ifndef DBUS_SERVER_SOCKET_H
#define DBUS_SERVER_SOCKET_H

#include "dbus-internals.h"
#include "dbus-server-protected.h"
#include "dbus-nonce.h"

DBUS_BEGIN_DECLS
DBusServer* _dbus_server_new_for_socket(DBusSocket *fds, int n_fds, const DBusString *address, DBusNonceFile *noncefile, DBusError *error);
DBusServer* _dbus_server_new_for_autolaunch(const DBusString *address, DBusError *error);
DBUS_PRIVATE_EXPORT DBusServer* _dbus_server_new_for_tcp_socket(const char *host, const char *bind, const char *port, const char *family, DBusError *error,
                                                                dbus_bool_t use_nonce);
DBusServerListenResult _dbus_server_listen_socket(DBusAddressEntry *entry, DBusServer **server_p, DBusError *error);
void _dbus_server_socket_own_filename(DBusServer *server, char *filename);
DBUS_END_DECLS

#endif