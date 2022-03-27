#ifndef DBUS_MARSHAL_BYTESWAP_H
#define DBUS_MARSHAL_BYTESWAP_H

#include "dbus-protocol.h"
#include "dbus-string.h"
#include "dbus-marshal-recursive.h"

DBUS_PRIVATE_EXPORT void _dbus_marshal_byteswap(const DBusString *signature, int signature_start, int old_byte_order, int new_byte_order, DBusString *value_str,
                                                int value_pos);

#endif