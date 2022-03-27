#ifndef DBUS_TRANSPORT_PROTECTED_H
#define DBUS_TRANSPORT_PROTECTED_H

#include "dbus-internals.h"
#include "dbus-errors.h"
#include "dbus-transport.h"
#include "dbus-message-internal.h"
#include "dbus-auth.h"
#include "dbus-resources.h"

DBUS_BEGIN_DECLS
typedef struct DBusTransportVTable DBusTransportVTable;
struct DBusTransportVTable {
  void (*finalize)(DBusTransport *transport);
  dbus_bool_t (*handle_watch)(DBusTransport *transport, DBusWatch *watch, unsigned int flags);
  void (*disconnect)(DBusTransport *transport);
  dbus_bool_t (*connection_set)(DBusTransport *transport);
  void (*do_iteration)(DBusTransport *transport, unsigned int flags, int timeout_milliseconds);
  void (*live_messages_changed)(DBusTransport *transport);
  dbus_bool_t (*get_socket_fd)(DBusTransport *transport, DBusSocket *fd_p);
};
struct DBusTransport {
  int refcount;
  const DBusTransportVTable *vtable;
  DBusConnection *connection;
  DBusMessageLoader *loader;
  DBusAuth *auth;
  DBusCredentials *credentials;
  long max_live_messages_size;
  long max_live_messages_unix_fds;
  DBusCounter *live_messages;
  char *address;
  char *expected_guid;
  DBusAllowUnixUserFunction unix_user_function;
  void *unix_user_data;
  DBusFreeFunction free_unix_user_data;
  DBusAllowWindowsUserFunction windows_user_function;
  void *windows_user_data;
  DBusFreeFunction free_windows_user_data;
  unsigned int disconnected : 1;
  unsigned int authenticated : 1;
  unsigned int send_credentials_pending : 1;
  unsigned int receive_credentials_pending : 1;
  unsigned int is_server : 1;
  unsigned int unused_bytes_recovered : 1;
  unsigned int allow_anonymous : 1;
};
dbus_bool_t _dbus_transport_init_base(DBusTransport *transport, const DBusTransportVTable *vtable, const DBusString *server_guid, const DBusString *address);
void _dbus_transport_finalize_base (DBusTransport *transport);
typedef enum {
  DBUS_TRANSPORT_OPEN_NOT_HANDLED,
  DBUS_TRANSPORT_OPEN_OK,
  DBUS_TRANSPORT_OPEN_BAD_ADDRESS,
  DBUS_TRANSPORT_OPEN_DID_NOT_CONNECT
} DBusTransportOpenResult;

DBusTransportOpenResult _dbus_transport_open_platform_specific(DBusAddressEntry *entry, DBusTransport **transport_p, DBusError *error);
#define DBUS_TRANSPORT_CAN_SEND_UNIX_FD(x)   _dbus_auth_get_unix_fd_negotiated((x)->auth)
DBUS_END_DECLS

#endif