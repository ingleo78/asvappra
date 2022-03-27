#include "config.h"
#include "dbus-internals.h"
#include "dbus-server-win.h"
#include "dbus-server-socket.h"

DBusServerListenResult _dbus_server_listen_platform_specific(DBusAddressEntry *entry, DBusServer **server_p, DBusError *error) {
  const char *method;
  *server_p  = NULL;
  method = dbus_address_entry_get_method(entry);
  if (strcmp(method, "autolaunch") == 0) {
      const char *host = "localhost";
      const char *bind = "localhost";
      const char *port = "0";
      const char *family = "ipv4";
      const char *scope = dbus_address_entry_get_value(entry, "scope");
      if (_dbus_daemon_is_session_bus_address_published(scope)) return DBUS_SERVER_LISTEN_ADDRESS_ALREADY_USED;
      *server_p = _dbus_server_new_for_tcp_socket(host, bind, port, family, error, FALSE);
      if (*server_p) {
          _DBUS_ASSERT_ERROR_IS_CLEAR(error);
          (*server_p)->published_address = _dbus_daemon_publish_session_bus_address((*server_p)->address, scope);
          return DBUS_SERVER_LISTEN_OK;
      } else {
          _dbus_daemon_unpublish_session_bus_address();
          _DBUS_ASSERT_ERROR_IS_SET(error);
          return DBUS_SERVER_LISTEN_DID_NOT_CONNECT;
      }
  } else {
      _DBUS_ASSERT_ERROR_IS_CLEAR(error);
      return DBUS_SERVER_LISTEN_NOT_HANDLED;
  }
}