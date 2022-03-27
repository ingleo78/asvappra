#include "config.h"
#include "dbus-internals.h"
#include "dbus-connection-internal.h"
#include "dbus-transport-socket.h"
#include "dbus-transport-protected.h"
#include "dbus-watch.h"
#include "dbus-sysdeps-win.h"

DBusTransportOpenResult _dbus_transport_open_platform_specific(DBusAddressEntry *entry, DBusTransport **transport_p, DBusError *error) {
  return DBUS_TRANSPORT_OPEN_NOT_HANDLED;
}