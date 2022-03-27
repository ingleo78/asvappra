#ifndef DBUS_ASV_UTIL_H
#define DBUS_ASV_UTIL_H

#include "dbus-internals.h"

DBUS_BEGIN_DECLS
DBusMessage *_dbus_asv_new_method_return(DBusMessage *message, DBusMessageIter *iter, DBusMessageIter *arr_iter);
dbus_bool_t _dbus_asv_close(DBusMessageIter *iter, DBusMessageIter *arr_iter);
void _dbus_asv_abandon(DBusMessageIter *iter, DBusMessageIter *arr_iter);
dbus_bool_t _dbus_asv_add_uint32(DBusMessageIter *arr_iter, const char *key, dbus_uint32_t value);
dbus_bool_t _dbus_asv_add_string(DBusMessageIter *arr_iter, const char *key, const char *value);
dbus_bool_t _dbus_asv_add_object_path(DBusMessageIter *arr_iter, const char *key, const char *value);
dbus_bool_t _dbus_asv_add_byte_array(DBusMessageIter *arr_iter, const char *key, const void *value, int n_elements);
dbus_bool_t _dbus_asv_open_entry(DBusMessageIter *arr_iter, DBusMessageIter *entry_iter, const char *key, const char *type, DBusMessageIter *var_iter);
dbus_bool_t _dbus_asv_close_entry(DBusMessageIter *arr_iter, DBusMessageIter *entry_iter, DBusMessageIter *var_iter);
void _dbus_asv_abandon_entry(DBusMessageIter *arr_iter, DBusMessageIter *entry_iter, DBusMessageIter *var_iter);
DBUS_END_DECLS

#endif