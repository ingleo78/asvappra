#ifndef DBUS_MARSHAL_RECURSIVE_H
#define DBUS_MARSHAL_RECURSIVE_H

#include "dbus-protocol.h"
#include "dbus-list.h"

typedef struct DBusTypeReader DBusTypeReader;
typedef struct DBusTypeWriter DBusTypeWriter;
typedef struct DBusTypeReaderClass DBusTypeReaderClass;
typedef struct DBusArrayLenFixup DBusArrayLenFixup;
struct DBusTypeReader {
  dbus_uint32_t byte_order : 8;
  dbus_uint32_t finished : 1;
  dbus_uint32_t array_len_offset : 3;
  const DBusString *type_str;
  int type_pos;
  const DBusString *value_str;
  int value_pos;
  const DBusTypeReaderClass *klass;
  union {
      struct {
          int start_pos;
      } array;
  } u;
};
struct DBusTypeWriter {
  dbus_uint32_t byte_order : 8;
  dbus_uint32_t container_type : 8;
  dbus_uint32_t type_pos_is_expectation : 1;
  dbus_uint32_t enabled : 1;
  DBusString *type_str;
  int type_pos;
  DBusString *value_str;
  int value_pos;
  union {
      struct {
          int start_pos;
          int len_pos;
          int element_type_pos;
      } array;
  } u;
};
struct DBusArrayLenFixup {
  int len_pos_in_reader;
  int new_len;
};
DBUS_PRIVATE_EXPORT void _dbus_type_reader_init(DBusTypeReader *reader, int byte_order, const DBusString *type_str, int type_pos, const DBusString *value_str,
                                                int value_pos);
DBUS_PRIVATE_EXPORT void _dbus_type_reader_init_types_only(DBusTypeReader *reader, const DBusString *type_str, int type_pos);
DBUS_PRIVATE_EXPORT int _dbus_type_reader_get_current_type(const DBusTypeReader *reader);
DBUS_PRIVATE_EXPORT int _dbus_type_reader_get_element_type(const DBusTypeReader *reader);
DBUS_PRIVATE_EXPORT int _dbus_type_reader_get_value_pos(const DBusTypeReader *reader);
DBUS_PRIVATE_EXPORT void _dbus_type_reader_read_basic(const DBusTypeReader *reader, void *value);
int _dbus_type_reader_get_array_length(const DBusTypeReader *reader);
DBUS_PRIVATE_EXPORT void _dbus_type_reader_read_fixed_multi(const DBusTypeReader *reader, void *value, int *n_elements);
void _dbus_type_reader_read_raw(const DBusTypeReader *reader, const unsigned char **value_location);
DBUS_PRIVATE_EXPORT void _dbus_type_reader_recurse(DBusTypeReader *reader, DBusTypeReader *subreader);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_type_reader_next(DBusTypeReader *reader);
dbus_bool_t _dbus_type_reader_has_next(const DBusTypeReader *reader);
DBUS_PRIVATE_EXPORT void _dbus_type_reader_get_signature(const DBusTypeReader *reader, const DBusString **str_p, int *start_p, int *len_p);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_type_reader_set_basic(DBusTypeReader *reader, const void *value, const DBusTypeReader *realign_root);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_type_reader_delete(DBusTypeReader *reader, const DBusTypeReader *realign_root);
dbus_bool_t _dbus_type_reader_equal_values(const DBusTypeReader *lhs, const DBusTypeReader *rhs);
void _dbus_type_signature_next(const char *signature, int *type_pos);
DBUS_PRIVATE_EXPORT void _dbus_type_writer_init(DBusTypeWriter *writer, int byte_order, DBusString *type_str, int type_pos, DBusString *value_str, int value_pos);
void _dbus_type_writer_init_types_delayed(DBusTypeWriter *writer, int byte_order, DBusString *value_str, int value_pos);
void _dbus_type_writer_add_types(DBusTypeWriter *writer, DBusString *type_str, int type_pos);
void _dbus_type_writer_remove_types(DBusTypeWriter *writer);
DBUS_PRIVATE_EXPORT void _dbus_type_writer_init_values_only(DBusTypeWriter *writer, int byte_order, const DBusString *type_str, int type_pos, DBusString *value_str,
                                                            int value_pos);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_type_writer_write_basic(DBusTypeWriter *writer, int type, const void *value);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_type_writer_write_fixed_multi(DBusTypeWriter *writer, int element_type, const void *value, int n_elements);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_type_writer_recurse(DBusTypeWriter *writer, int container_type, const DBusString *contained_type, int contained_type_start,
                                                          DBusTypeWriter *sub);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_type_writer_unrecurse(DBusTypeWriter *writer, DBusTypeWriter *sub);
dbus_bool_t _dbus_type_writer_append_array(DBusTypeWriter *writer, const DBusString *contained_type, int contained_type_start, DBusTypeWriter *sub);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_type_writer_write_reader(DBusTypeWriter *writer, DBusTypeReader *reader);

#endif