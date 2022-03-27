#ifndef DBUS_STRING_H
#define DBUS_STRING_H

#include <stdarg.h>
#include "dbus-macros.h"
#include "dbus-types.h"
#include "dbus-memory.h"

DBUS_BEGIN_DECLS
typedef struct DBusString DBusString;
struct DBusString {
#if defined(DBUS_WIN) && defined(_DEBUG)
  const char *dummy1;
#else
  const void *dummy1;
#endif
  int dummy2;
  int dummy3;
  unsigned int dummy_bit1 : 1;
  unsigned int dummy_bit2 : 1;
  unsigned int dummy_bit3 : 1;
  unsigned int dummy_bits : 3;
};
#define _DBUS_STRING_INIT_INVALID \
{ \
    NULL, /* dummy1 */ \
    0, /* dummy2 */ \
    0, /* dummy3 */ \
    0, /* dummy_bit1 */ \
    0, /* dummy_bit2 */ \
    0, /* dummy_bit3 */ \
    0 /* dummy_bits */ \
}
#ifdef DBUS_DISABLE_ASSERT
#define _dbus_string_get_data(s) ((char*)(((DBusString*)(s))->dummy1))
#define _dbus_string_get_length(s) (((DBusString*)(s))->dummy2)
#define _dbus_string_set_byte(s, i, b) ((((unsigned char*)(((DBusString*)(s))->dummy1))[(i)]) = (unsigned char) (b))
#define _dbus_string_get_byte(s, i) (((const unsigned char*)(((DBusString*)(s))->dummy1))[(i)])
#define _dbus_string_get_const_data(s) ((const char*)(((DBusString*)(s))->dummy1))
#define _dbus_string_get_const_data_len(s,start,len) (((const char*)(((DBusString*)(s))->dummy1)) + (start))
#endif
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_init(DBusString *str);
DBUS_PRIVATE_EXPORT void _dbus_string_init_const(DBusString *str, const char *value);
DBUS_PRIVATE_EXPORT void _dbus_string_init_const_len(DBusString *str, const char *value, int len);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_init_preallocated(DBusString *str, int allocate_size);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_init_from_string(DBusString *str, const DBusString *from);
DBUS_PRIVATE_EXPORT void _dbus_string_free(DBusString *str);
void _dbus_string_lock(DBusString *str);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_compact(DBusString *str, int max_waste);
#ifndef _dbus_string_get_data
DBUS_PRIVATE_EXPORT char* _dbus_string_get_data(DBusString *str);
#endif
#ifndef _dbus_string_get_const_data
DBUS_PRIVATE_EXPORT const char* _dbus_string_get_const_data(const DBusString *str);
#endif
DBUS_PRIVATE_EXPORT
char* _dbus_string_get_data_len(DBusString *str, int start, int len);
#ifndef _dbus_string_get_const_data_len
DBUS_PRIVATE_EXPORT const char* _dbus_string_get_const_data_len(const DBusString *str, int start, int len);
#endif
#ifndef _dbus_string_set_byte
DBUS_PRIVATE_EXPORT
void _dbus_string_set_byte(DBusString *str, int i, unsigned char byte);
#endif
#ifndef _dbus_string_get_byte
DBUS_PRIVATE_EXPORT
unsigned char _dbus_string_get_byte(const DBusString *str, int start);
#endif
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_insert_bytes(DBusString *str, int i, int n_bytes, unsigned char byte);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_insert_byte(DBusString *str, int i, unsigned char byte);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_steal_data(DBusString *str, char **data_return);
dbus_bool_t _dbus_string_steal_data_len(DBusString *str, char **data_return, int start, int len);
DBUS_PRIVATE_EXPORT
dbus_bool_t _dbus_string_copy_data(const DBusString *str, char **data_return);
dbus_bool_t _dbus_string_copy_data_len(const DBusString *str, char **data_return, int start, int len);
void _dbus_string_copy_to_buffer(const DBusString *str, char *buffer, int len);
DBUS_PRIVATE_EXPORT void _dbus_string_copy_to_buffer_with_nul(const DBusString *str, char *buffer, int avail_len);
#ifndef _dbus_string_get_length
DBUS_PRIVATE_EXPORT int _dbus_string_get_length(const DBusString *str);
#endif
static inline unsigned int _dbus_string_get_length_uint(const DBusString *str) {
  return (unsigned int)_dbus_string_get_length(str);
}
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_lengthen(DBusString *str, int additional_length);
DBUS_PRIVATE_EXPORT void _dbus_string_shorten(DBusString *str, int length_to_remove);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_set_length(DBusString *str, int length);
dbus_bool_t _dbus_string_align_length(DBusString *str, int alignment);
dbus_bool_t _dbus_string_alloc_space(DBusString *str, int extra_bytes);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_append(DBusString *str, const char *buffer);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_append_len(DBusString *str, const char *buffer, int len);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_append_int(DBusString *str, long value);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_append_uint(DBusString *str, unsigned long value);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_append_byte(DBusString *str, unsigned char byte);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_append_printf(DBusString *str, const char *format, ...) _DBUS_GNUC_PRINTF(2, 3);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_append_printf_valist(DBusString *str, const char *format, va_list args) _DBUS_GNUC_PRINTF(2, 0);
dbus_bool_t _dbus_string_insert_2_aligned(DBusString *str, int insert_at, const unsigned char octets[2]);
dbus_bool_t _dbus_string_insert_4_aligned(DBusString *str, int insert_at, const unsigned char octets[4]);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_insert_8_aligned(DBusString *str, int insert_at, const unsigned char octets[8]);
dbus_bool_t _dbus_string_insert_alignment(DBusString *str, int *insert_at, int alignment);
DBUS_PRIVATE_EXPORT void _dbus_string_delete(DBusString *str, int start, int len);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_move(DBusString *source, int start, DBusString *dest, int insert_at);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_copy(const DBusString *source, int start, DBusString *dest, int insert_at);
dbus_bool_t _dbus_string_move_len(DBusString *source, int start, int len, DBusString *dest, int insert_at);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_copy_len(const DBusString *source, int start, int len, DBusString *dest, int insert_at);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_replace_len(const DBusString *source, int start, int len, DBusString *dest, int replace_at, int replace_len);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_split_on_byte(DBusString *source, unsigned char byte, DBusString *tail);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_parse_int(const DBusString *str, int start, long *value_return, int *end_return);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_parse_uint(const DBusString *str, int start, unsigned long *value_return, int *end_return);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_find(const DBusString *str, int start, const char *substr, int *found);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_find_eol(const DBusString *str, int start, int *found, int *found_len);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_find_to(const DBusString *str, int start, int end, const char *substr, int *found);
dbus_bool_t _dbus_string_find_byte_backward(const DBusString *str, int start, unsigned char byte, int *found);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_find_blank(const DBusString *str, int start, int *found);
DBUS_PRIVATE_EXPORT void _dbus_string_skip_blank(const DBusString *str, int start, int *end);
DBUS_PRIVATE_EXPORT void _dbus_string_skip_white(const DBusString *str, int start, int *end);
void _dbus_string_skip_white_reverse(const DBusString *str, int end, int *start);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_equal(const DBusString *a, const DBusString *b);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_equal_c_str(const DBusString *a, const char *c_str);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_equal_len(const DBusString *a, const DBusString *b, int len);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_equal_substring(const DBusString *a, int a_start, int a_len, const DBusString *b, int b_start);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_starts_with_c_str(const DBusString *a, const char *c_str);
dbus_bool_t _dbus_string_ends_with_c_str(const DBusString *a, const char *c_str);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_pop_line(DBusString *source, DBusString *dest);
DBUS_PRIVATE_EXPORT void _dbus_string_delete_first_word(DBusString *str);
DBUS_PRIVATE_EXPORT void _dbus_string_delete_leading_blanks(DBusString *str);
DBUS_PRIVATE_EXPORT void _dbus_string_chop_white(DBusString *str);
dbus_bool_t _dbus_string_append_byte_as_hex(DBusString *str, unsigned char byte);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_hex_encode(const DBusString *source, int start, DBusString *dest, int insert_at);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_hex_decode(const DBusString *source, int start, int *end_return, DBusString *dest, int insert_at);
DBUS_PRIVATE_EXPORT void _dbus_string_tolower_ascii(const DBusString *str, int start, int len);
DBUS_PRIVATE_EXPORT void _dbus_string_toupper_ascii(const DBusString *str, int start, int len);
dbus_bool_t _dbus_string_validate_ascii(const DBusString *str, int start, int len);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_validate_utf8(const DBusString *str, int start, int len);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_string_validate_nul(const DBusString *str, int start, int len);
void _dbus_string_zero(DBusString *str);
static inline unsigned char *_dbus_string_get_udata(DBusString *str) {
  return (unsigned char*)_dbus_string_get_data(str);
}
static inline unsigned char *_dbus_string_get_udata_len(DBusString *str, int start, int len) {
  return (unsigned char*)_dbus_string_get_data_len(str, start, len);
}
static inline const unsigned char *_dbus_string_get_const_udata(const DBusString *str) {
  return (const unsigned char*)_dbus_string_get_const_data(str);
}
static inline const unsigned char *_dbus_string_get_const_udata_len(const DBusString *str, int start, int len) {
  return (const unsigned char*)_dbus_string_get_const_data_len(str, start, len);
}
#define _DBUS_STRING_ALLOCATION_PADDING  8
#define _DBUS_STRING_DEFINE_STATIC(name, str) \
  static const char _dbus_static_string_##name[] = str; \
  static const DBusString name = { \
      _dbus_static_string_##name, \
      sizeof(_dbus_static_string_##name) - 1, \
      sizeof(_dbus_static_string_##name) + _DBUS_STRING_ALLOCATION_PADDING, \
      TRUE, TRUE, TRUE, 0 \
  }
DBUS_END_DECLS

#endif