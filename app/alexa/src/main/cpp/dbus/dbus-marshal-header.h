#ifndef DBUS_MARSHAL_HEADER_H
#define DBUS_MARSHAL_HEADER_H

#include "dbus-marshal-basic.h"
#include "dbus-marshal-validate.h"

typedef struct DBusHeader      DBusHeader;
typedef struct DBusHeaderField DBusHeaderField;
#define _DBUS_HEADER_FIELD_VALUE_UNKNOWN  -1
#define _DBUS_HEADER_FIELD_VALUE_NONEXISTENT  -2
struct DBusHeaderField {
  int value_pos;
};
struct DBusHeader {
  DBusString data;
  DBusHeaderField fields[DBUS_HEADER_FIELD_LAST + 1];
  dbus_uint32_t padding : 3;
  dbus_uint32_t byte_order : 8;
};
dbus_bool_t _dbus_header_init(DBusHeader *header);
void _dbus_header_free(DBusHeader *header);
void _dbus_header_reinit(DBusHeader *header);
dbus_bool_t _dbus_header_create(DBusHeader *header, int byte_order, int type, const char *destination, const char *path, const char *interface, const char *member,
                                const char *error_name);
dbus_bool_t _dbus_header_copy(const DBusHeader *header, DBusHeader *dest);
int _dbus_header_get_message_type(DBusHeader *header);
void _dbus_header_set_serial(DBusHeader *header, dbus_uint32_t serial);
dbus_uint32_t _dbus_header_get_serial(DBusHeader *header);
void _dbus_header_update_lengths(DBusHeader *header, int body_len);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_header_set_field_basic(DBusHeader *header, int field, int type, const void *value);
dbus_bool_t _dbus_header_get_field_basic(DBusHeader *header, int field, int type, void *value);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_header_get_field_raw(DBusHeader *header, int field, const DBusString **str, int *pos);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_header_delete_field(DBusHeader *header, int field);
void _dbus_header_toggle_flag(DBusHeader *header, dbus_uint32_t flag, dbus_bool_t value);
dbus_bool_t _dbus_header_get_flag(DBusHeader *header, dbus_uint32_t flag);
dbus_bool_t _dbus_header_ensure_signature(DBusHeader *header, DBusString **type_str, int *type_pos);
dbus_bool_t _dbus_header_have_message_untrusted(int max_message_length, DBusValidity *validity, int *byte_order, int *fields_array_len, int *header_len,
                                                int *body_len, const DBusString *str, int start, int len);
dbus_bool_t _dbus_header_load(DBusHeader *header, DBusValidationMode mode, DBusValidity *validity, int byte_order, int fields_array_len, int header_len,
                              int body_len, const DBusString *str);
void _dbus_header_byteswap(DBusHeader *header, int new_order);
DBUS_PRIVATE_EXPORT char _dbus_header_get_byte_order(const DBusHeader *header);
dbus_bool_t _dbus_header_remove_unknown_fields(DBusHeader *header);

#endif
