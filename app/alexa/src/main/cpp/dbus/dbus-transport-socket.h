#ifndef DBUS_TRANSPORT_SOCKET_H
#define DBUS_TRANSPORT_SOCKET_H

#include "dbus-transport-protected.h"

DBUS_BEGIN_DECLS
DBusTransport* _dbus_transport_new_for_socket(DBusSocket fd, const DBusString *server_guid, const DBusString *address);
DBusTransport* _dbus_transport_new_for_tcp_socket(const char *host, const char *port, const char *family, const char *noncefile, DBusError *error);
DBusTransportOpenResult _dbus_transport_open_socket(DBusAddressEntry *entry, DBusTransport **transport_p, DBusError *error);
DBUS_END_DECLS

#endif