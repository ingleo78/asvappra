#ifndef DBUS_KEYRING_H
#define DBUS_KEYRING_H

#include "dbus-macros.h"
#include "dbus-errors.h"
#include "dbus-string.h"
#include "dbus-credentials.h"

DBUS_BEGIN_DECLS
typedef struct DBusKeyring DBusKeyring;
DBusKeyring* _dbus_keyring_new_for_credentials(DBusCredentials *credentials, const DBusString *context, DBusError *error);
DBusKeyring* _dbus_keyring_ref(DBusKeyring *keyring);
void _dbus_keyring_unref(DBusKeyring *keyring);
dbus_bool_t _dbus_keyring_validate_context(const DBusString *context);
int _dbus_keyring_get_best_key(DBusKeyring *keyring, DBusError *error);
dbus_bool_t _dbus_keyring_is_for_credentials(DBusKeyring *keyring, DBusCredentials *credentials);
dbus_bool_t _dbus_keyring_get_hex_key(DBusKeyring *keyring, int key_id, DBusString *hex_key);
DBUS_END_DECLS

#endif