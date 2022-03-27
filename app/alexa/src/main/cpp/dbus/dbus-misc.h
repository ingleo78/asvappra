#if defined (DBUS_INSIDE_DBUS_H) && !defined (DBUS_COMPILATION)
#error "Only <dbus/dbus.h> can be included directly, this file may disappear or change contents."
#endif

#ifndef DBUS_MISC_H
#define DBUS_MISC_H

#include "dbus-types.h"
#include "dbus-errors.h"

DBUS_BEGIN_DECLS
DBUS_EXPORT char* dbus_get_local_machine_id(void);
DBUS_EXPORT void dbus_get_version(int *major_version_p, int *minor_version_p, int *micro_version_p);
DBUS_EXPORT dbus_bool_t dbus_setenv(const char *variable, const char *value);
DBUS_EXPORT char *dbus_try_get_local_machine_id(DBusError *error);
DBUS_END_DECLS

#endif