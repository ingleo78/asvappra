#if defined (DBUS_INSIDE_DBUS_H) && !defined (DBUS_COMPILATION)
#error "Only <dbus/dbus.h> can be included directly, this file may disappear or change contents."
#endif

#ifndef DBUS_ADDRESS_H
#define DBUS_ADDRESS_H

#include "dbus-types.h"
#include "dbus-errors.h"

DBUS_BEGIN_DECLS
typedef struct DBusAddressEntry DBusAddressEntry;
DBUS_EXPORT dbus_bool_t dbus_parse_address(const char *address, DBusAddressEntry ***entry_result, int *array_len, DBusError *error);
DBUS_EXPORT const char *dbus_address_entry_get_value(DBusAddressEntry *entry, const char *key);
DBUS_EXPORT const char *dbus_address_entry_get_method(DBusAddressEntry *entry);
DBUS_EXPORT void dbus_address_entries_free(DBusAddressEntry **entries);
DBUS_EXPORT char* dbus_address_escape_value(const char *value);
DBUS_EXPORT char* dbus_address_unescape_value(const char *value, DBusError *error);
static inline void dbus_clear_address_entries(DBusAddressEntry ***pointer_to_entries) {
  _dbus_clear_pointer_impl(DBusAddressEntry *, pointer_to_entries, dbus_address_entries_free);
}
DBUS_END_DECLS

#endif