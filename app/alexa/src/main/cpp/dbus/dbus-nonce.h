#ifndef DBUS_NONCE_H
#define DBUS_NONCE_H

#include "dbus-macros.h"
#include "dbus-types.h"
#include "dbus-errors.h"
#include "dbus-string.h"
#include "dbus-sysdeps.h"

DBUS_BEGIN_DECLS
typedef struct DBusNonceFile DBusNonceFile;
dbus_bool_t _dbus_noncefile_create(DBusNonceFile **noncefile_out, DBusError *error);
dbus_bool_t _dbus_noncefile_delete(DBusNonceFile **noncefile_location, DBusError *error);
dbus_bool_t _dbus_noncefile_check_nonce(DBusSocket fd, const DBusNonceFile *noncefile, DBusError *error);
const DBusString* _dbus_noncefile_get_path(const DBusNonceFile *noncefile);
DBusSocket _dbus_accept_with_noncefile(DBusSocket listen_fd, const DBusNonceFile *noncefile);
dbus_bool_t _dbus_read_nonce(const DBusString *fname, DBusString *nonce, DBusError *error);
dbus_bool_t _dbus_send_nonce(DBusSocket fd, const DBusString *noncefile, DBusError *error);
DBUS_END_DECLS

#endif