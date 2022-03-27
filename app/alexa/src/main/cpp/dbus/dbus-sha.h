#ifndef DBUS_SHA_H
#define DBUS_SHA_H

#include "dbus-macros.h"
#include "dbus-errors.h"
#include "dbus-string.h"

DBUS_BEGIN_DECLS
typedef struct DBusSHAContext DBusSHAContext;
struct DBusSHAContext {
  dbus_uint32_t digest[5];
  dbus_uint32_t count_lo;
  dbus_uint32_t count_hi;
  dbus_uint32_t data[16];
};
void _dbus_sha_init(DBusSHAContext *context);
void _dbus_sha_update(DBusSHAContext *context, const DBusString *data);
dbus_bool_t _dbus_sha_final(DBusSHAContext *context, DBusString *results);
dbus_bool_t _dbus_sha_compute(const DBusString *data, DBusString *ascii_output);
DBUS_END_DECLS

#endif