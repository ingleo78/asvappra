#ifdef DBUS_INSIDE_DBUS_H
#error "You can't include dbus-uuidgen.h in the public header dbus.h"
#endif

#ifndef DBUS_UUIDGEN_H
#define DBUS_UUIDGEN_H

#include "dbus-types.h"
#include "dbus-errors.h"

DBUS_BEGIN_DECLS
DBUS_PRIVATE_EXPORT dbus_bool_t dbus_internal_do_not_use_get_uuid(const char *filename, char **uuid_p, dbus_bool_t create_if_not_found, DBusError *error);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_create_uuid(char **uuid_p, DBusError *error);
DBUS_END_DECLS

#endif