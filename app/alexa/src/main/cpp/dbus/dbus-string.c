#include <string.h>
#include <stdio.h>
#include "config.h"
#include "dbus-internals.h"
#include "dbus-string.h"
#define DBUS_CAN_USE_DBUS_STRING_PRIVATE 1
#include "dbus-string-private.h"
#include "dbus-marshal-basic.h"
#include "dbus-sysdeps.h"

static void fixup_alignment(DBusRealString *real) {
  unsigned char *aligned;
  unsigned char *real_block;
  unsigned int old_align_offset;
  _dbus_assert(real->len <= real->allocated - _DBUS_STRING_ALLOCATION_PADDING);
  old_align_offset = real->align_offset;
  real_block = real->str - old_align_offset;
  aligned = _DBUS_ALIGN_ADDRESS(real_block, 8);
  real->align_offset = aligned - real_block;
  real->str = aligned;
  if (old_align_offset != real->align_offset) memmove(real_block + real->align_offset, real_block + old_align_offset, real->len + 1);
  _dbus_assert(real->align_offset < 8);
  _dbus_assert(_DBUS_ALIGN_ADDRESS(real->str, 8) == real->str);
}
static void undo_alignment(DBusRealString *real) {
  if (real->align_offset != 0) {
      memmove(real->str - real->align_offset, real->str, real->len + 1);
      real->str = real->str - real->align_offset;
      real->align_offset = 0;
  }
}
dbus_bool_t _dbus_string_init_preallocated(DBusString *str, int allocate_size) {
  DBusRealString *real;
  _DBUS_STATIC_ASSERT(sizeof(DBusString) == sizeof(DBusRealString));
  _dbus_assert(str != NULL);
  real = (DBusRealString*)str;
  real->str = dbus_malloc(_DBUS_STRING_ALLOCATION_PADDING + allocate_size);
  if (real->str == NULL) return FALSE;
  real->allocated = _DBUS_STRING_ALLOCATION_PADDING + allocate_size;
  real->len = 0;
  real->str[real->len] = '\0';
  real->constant = FALSE;
  real->locked = FALSE;
  real->valid = TRUE;
  real->align_offset = 0;
  fixup_alignment(real);
  return TRUE;
}
dbus_bool_t _dbus_string_init(DBusString *str) {
  return _dbus_string_init_preallocated(str, 0);
}
void _dbus_string_init_const(DBusString *str, const char *value) {
  _dbus_assert(value != NULL);
  _dbus_string_init_const_len(str, value, strlen(value));
}
void _dbus_string_init_const_len(DBusString *str, const char *value, int len) {
  DBusRealString *real;
  _dbus_assert(str != NULL);
  _dbus_assert(len == 0 || value != NULL);
  _dbus_assert(len <= _DBUS_STRING_MAX_LENGTH);
  _dbus_assert(len >= 0);
  real = (DBusRealString*)str;
  real->str = (unsigned char*)value;
  real->len = len;
  real->allocated = real->len + _DBUS_STRING_ALLOCATION_PADDING;
  real->constant = TRUE;
  real->locked = TRUE;
  real->valid = TRUE;
  real->align_offset = 0;
}
dbus_bool_t _dbus_string_init_from_string(DBusString *str, const DBusString *from) {
 if (!_dbus_string_init(str)) return FALSE;
 return _dbus_string_append(str, _dbus_string_get_const_data(from));
}
void _dbus_string_free(DBusString *str) {
  DBusRealString *real = (DBusRealString*)str;
  DBusRealString invalid = _DBUS_STRING_INIT_INVALID;
  if (real->str == NULL && real->len == 0 && real->allocated == 0 && !real->constant && !real->locked && !real->valid && real->align_offset == 0) return;
  DBUS_GENERIC_STRING_PREAMBLE(real);
  if (real->constant) goto wipe;
  if (real->str == NULL) goto wipe;
  dbus_free(real->str - real->align_offset);
wipe:
  *real = invalid;
  real->valid = FALSE;
}
static dbus_bool_t compact(DBusRealString *real, int max_waste) {
  unsigned char *new_str;
  int new_allocated;
  int waste;
  waste = real->allocated - (real->len + _DBUS_STRING_ALLOCATION_PADDING);
  if (waste <= max_waste) return TRUE;
  new_allocated = real->len + _DBUS_STRING_ALLOCATION_PADDING;
  new_str = dbus_realloc(real->str - real->align_offset, new_allocated);
  if (_DBUS_UNLIKELY(new_str == NULL)) return FALSE;
  real->str = new_str + real->align_offset;
  real->allocated = new_allocated;
  fixup_alignment(real);
  return TRUE;
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
void _dbus_string_lock(DBusString *str) {
  DBUS_LOCKED_STRING_PREAMBLE(str);
  real->locked = TRUE;
#define MAX_WASTE 48
  compact(real, MAX_WASTE);
}
#endif
static dbus_bool_t reallocate_for_length(DBusRealString *real, int new_length) {
  int new_allocated;
  unsigned char *new_str;
  if (real->allocated > (_DBUS_STRING_MAX_LENGTH + _DBUS_STRING_ALLOCATION_PADDING) / 2) new_allocated = _DBUS_STRING_MAX_LENGTH + _DBUS_STRING_ALLOCATION_PADDING;
  else new_allocated = real->allocated * 2;
#if defined(DBUS_ENABLE_EMBEDDED_TESTS) && !defined(DBUS_DISABLE_ASSERT)
  new_allocated = 0;
#endif
  new_allocated = MAX(new_allocated,new_length + _DBUS_STRING_ALLOCATION_PADDING);
  _dbus_assert(new_allocated >= real->allocated);
  new_str = dbus_realloc(real->str - real->align_offset, new_allocated);
  if (_DBUS_UNLIKELY(new_str == NULL)) return FALSE;
  real->str = new_str + real->align_offset;
  real->allocated = new_allocated;
  fixup_alignment(real);
  return TRUE;
}
dbus_bool_t _dbus_string_compact(DBusString *str, int max_waste) {
  DBUS_STRING_PREAMBLE(str);
  return compact(real, max_waste);
}
static dbus_bool_t set_length(DBusRealString *real, int new_length) {
  if (_DBUS_UNLIKELY(new_length > _DBUS_STRING_MAX_LENGTH)) return FALSE;
  else if (new_length > (real->allocated - _DBUS_STRING_ALLOCATION_PADDING) && _DBUS_UNLIKELY(!reallocate_for_length(real, new_length))) return FALSE;
  else {
      real->len = new_length;
      real->str[new_length] = '\0';
      return TRUE;
  }
}
static dbus_bool_t open_gap(int len, DBusRealString *dest, int insert_at) {
  if (len == 0) return TRUE;
  if (len > _DBUS_STRING_MAX_LENGTH - dest->len) return FALSE;
  if (!set_length(dest, dest->len + len)) return FALSE;
  memmove(dest->str + insert_at + len,dest->str + insert_at,dest->len - len - insert_at);
  return TRUE;
}
#ifndef _dbus_string_get_data
char* _dbus_string_get_data(DBusString *str) {
  DBUS_STRING_PREAMBLE(str);
  return (char*)real->str;
}
#endif
#ifndef _dbus_string_get_const_data
const char* _dbus_string_get_const_data(const DBusString  *str) {
  DBUS_CONST_STRING_PREAMBLE(str);
  return (const char*)real->str;
}
#endif
char* _dbus_string_get_data_len(DBusString *str, int start, int len) {
  DBUS_STRING_PREAMBLE(str);
  _dbus_assert(start >= 0);
  _dbus_assert(len >= 0);
  _dbus_assert(start <= real->len);
  _dbus_assert(len <= real->len - start);
  return (char*)real->str + start;
}
#ifndef _dbus_string_get_const_data_len
const char* _dbus_string_get_const_data_len(const DBusString *str, int start, int len) {
  DBUS_CONST_STRING_PREAMBLE(str);
  _dbus_assert(start >= 0);
  _dbus_assert(len >= 0);
  _dbus_assert(start <= real->len);
  _dbus_assert(len <= real->len - start);
  return (const char*)real->str + start;
}
#endif
#ifndef _dbus_string_set_byte
void _dbus_string_set_byte(DBusString *str, int i, unsigned char byte) {
  DBUS_STRING_PREAMBLE(str);
  _dbus_assert(i < real->len);
  _dbus_assert(i >= 0);
  real->str[i] = byte;
}
#endif
#ifndef _dbus_string_get_byte
unsigned char _dbus_string_get_byte(const DBusString *str, int start) {
  DBUS_CONST_STRING_PREAMBLE(str);
  _dbus_assert(start <= real->len);
  _dbus_assert(start >= 0);
  return real->str[start];
}
#endif
dbus_bool_t _dbus_string_insert_bytes(DBusString *str, int i, int n_bytes, unsigned char byte) {
  DBUS_STRING_PREAMBLE(str);
  _dbus_assert(i <= real->len);
  _dbus_assert(i >= 0);
  _dbus_assert(n_bytes >= 0);
  if (n_bytes == 0) return TRUE;
  if (!open_gap(n_bytes, real, i)) return FALSE;
  memset(real->str + i, byte, n_bytes);
  return TRUE;
}
dbus_bool_t _dbus_string_insert_byte(DBusString *str, int i, unsigned char byte) {
  DBUS_STRING_PREAMBLE(str);
  _dbus_assert(i <= real->len);
  _dbus_assert(i >= 0);
  if (!open_gap(1, real, i)) return FALSE;
  real->str[i] = byte;
  return TRUE;
}
dbus_bool_t _dbus_string_steal_data(DBusString *str, char **data_return) {
  DBUS_STRING_PREAMBLE(str);
  _dbus_assert(data_return != NULL);
  undo_alignment(real);
  *data_return = (char*)real->str;
  if (!_dbus_string_init(str)) {
      real->str = (unsigned char*)*data_return;
      *data_return = NULL;
      fixup_alignment(real);
      return FALSE;
  }
  return TRUE;
}
dbus_bool_t _dbus_string_copy_data(const DBusString *str, char **data_return) {
  DBUS_CONST_STRING_PREAMBLE(str);
  _dbus_assert(data_return != NULL);
  *data_return = dbus_malloc(real->len + 1);
  if (*data_return == NULL) return FALSE;
  memcpy (*data_return, real->str, real->len + 1);
  return TRUE;
}
void _dbus_string_copy_to_buffer(const DBusString *str, char *buffer, int avail_len) {
  DBUS_CONST_STRING_PREAMBLE(str);
  _dbus_assert(avail_len >= 0);
  _dbus_assert(avail_len >= real->len);
  memcpy(buffer, real->str, real->len);
}
void _dbus_string_copy_to_buffer_with_nul(const DBusString *str, char *buffer, int avail_len) {
  DBUS_CONST_STRING_PREAMBLE(str);
  _dbus_assert(avail_len >= 0);
  _dbus_assert(avail_len > real->len);
  memcpy(buffer, real->str, real->len+1);
}
#ifndef _dbus_string_get_length
int _dbus_string_get_length(const DBusString *str) {
  DBUS_CONST_STRING_PREAMBLE(str);
  return real->len;
}
#endif
dbus_bool_t _dbus_string_lengthen(DBusString *str, int additional_length) {
  DBUS_STRING_PREAMBLE(str);
  _dbus_assert(additional_length >= 0);
  if (_DBUS_UNLIKELY(additional_length > _DBUS_STRING_MAX_LENGTH - real->len)) return FALSE;
  return set_length(real,real->len + additional_length);
}
void _dbus_string_shorten(DBusString *str, int length_to_remove) {
  DBUS_STRING_PREAMBLE(str);
  _dbus_assert(length_to_remove >= 0);
  _dbus_assert(length_to_remove <= real->len);
  set_length(real,real->len - length_to_remove);
}
dbus_bool_t _dbus_string_set_length(DBusString *str, int length) {
  DBUS_STRING_PREAMBLE(str);
  _dbus_assert(length >= 0);
  return set_length(real, length);
}
static dbus_bool_t align_insert_point_then_open_gap(DBusString *str, int *insert_at_p, int alignment, int gap_size) {
  unsigned long new_len;
  unsigned long gap_pos;
  int insert_at;
  int delta;
  DBUS_STRING_PREAMBLE(str);
  _dbus_assert(alignment >= 1);
  _dbus_assert(alignment <= 8);
  insert_at = *insert_at_p;
  _dbus_assert(insert_at <= real->len);
  gap_pos = _DBUS_ALIGN_VALUE(insert_at, alignment);
  new_len = real->len + (gap_pos - insert_at) + gap_size;
  if (_DBUS_UNLIKELY(new_len > (unsigned long)_DBUS_STRING_MAX_LENGTH)) return FALSE;
  delta = new_len - real->len;
  _dbus_assert(delta >= 0);
  if (delta == 0) {
      _dbus_assert(((unsigned long)*insert_at_p) == gap_pos);
      return TRUE;
  }
  if (_DBUS_UNLIKELY(!open_gap(new_len - real->len, real, insert_at))) return FALSE;
  if (gap_size < delta) memset(&real->str[insert_at], '\0',gap_pos - insert_at);
  *insert_at_p = gap_pos;
  return TRUE;
}
static dbus_bool_t align_length_then_lengthen(DBusString *str, int alignment, int then_lengthen_by) {
  int insert_at;
  insert_at = _dbus_string_get_length(str);
  return align_insert_point_then_open_gap(str, &insert_at, alignment, then_lengthen_by);
}
dbus_bool_t _dbus_string_align_length(DBusString *str, int alignment) {
  return align_length_then_lengthen(str, alignment, 0);
}
dbus_bool_t _dbus_string_alloc_space(DBusString *str, int extra_bytes) {
  if (!_dbus_string_lengthen(str, extra_bytes)) return FALSE;
  _dbus_string_shorten(str, extra_bytes);
  return TRUE;
}
static dbus_bool_t append(DBusRealString *real, const char *buffer, int buffer_len) {
  if (buffer_len == 0) return TRUE;
  if (!_dbus_string_lengthen((DBusString*)real, buffer_len)) return FALSE;
  memcpy(real->str + (real->len - buffer_len), buffer, buffer_len);
  return TRUE;
}
dbus_bool_t _dbus_string_append(DBusString *str, const char *buffer) {
  unsigned long buffer_len;
  DBUS_STRING_PREAMBLE(str);
  _dbus_assert(buffer != NULL);
  buffer_len = strlen(buffer);
  if (buffer_len > (unsigned long)_DBUS_STRING_MAX_LENGTH) return FALSE;
  return append(real, buffer, buffer_len);
}
#define ASSIGN_2_OCTETS(p, octets)  *((dbus_uint16_t*)(p)) = *((dbus_uint16_t*)(octets));
#define ASSIGN_4_OCTETS(p, octets)  *((dbus_uint32_t*)(p)) = *((dbus_uint32_t*)(octets));
#define ASSIGN_8_OCTETS(p, octets)  *((dbus_uint64_t*)(p)) = *((dbus_uint64_t*)(octets));
dbus_bool_t _dbus_string_insert_2_aligned(DBusString *str, int insert_at, const unsigned char octets[2]) {
  DBUS_STRING_PREAMBLE(str);
  if (!align_insert_point_then_open_gap(str, &insert_at, 2, 2)) return FALSE;
  ASSIGN_2_OCTETS(real->str + insert_at, octets);
  return TRUE;
}
dbus_bool_t _dbus_string_insert_4_aligned(DBusString *str, int insert_at, const unsigned char octets[4]) {
  DBUS_STRING_PREAMBLE(str);
  if (!align_insert_point_then_open_gap(str, &insert_at, 4, 4)) return FALSE;
  ASSIGN_4_OCTETS(real->str + insert_at, octets);
  return TRUE;
}
dbus_bool_t _dbus_string_insert_8_aligned(DBusString *str, int insert_at, const unsigned char octets[8]) {
  DBUS_STRING_PREAMBLE(str);
  if (!align_insert_point_then_open_gap(str, &insert_at, 8, 8)) return FALSE;
  _dbus_assert(_DBUS_ALIGN_VALUE(insert_at, 8) == (unsigned)insert_at);
  ASSIGN_8_OCTETS(real->str + insert_at, octets);
  return TRUE;
}
dbus_bool_t _dbus_string_insert_alignment(DBusString *str, int *insert_at, int alignment) {
  DBUS_STRING_PREAMBLE(str);
  if (!align_insert_point_then_open_gap(str, insert_at, alignment, 0)) return FALSE;
  _dbus_assert(_DBUS_ALIGN_VALUE(*insert_at, alignment) == (unsigned)*insert_at);
  return TRUE;
}
dbus_bool_t _dbus_string_append_printf_valist(DBusString *str, const char *format, va_list args) {
  dbus_bool_t ret = FALSE;
  /*int len;
  va_list args_copy;
  DBUS_STRING_PREAMBLE(str);
  DBUS_VA_COPY(args_copy, args);
  len = _dbus_printf_string_upper_bound(format, args);
  if (len < 0)goto out;
  if (!_dbus_string_lengthen(str, len)) goto out;
  vsprintf((char*)(real->str + (real->len - len)), format, args_copy);
  ret = TRUE;
out:
  va_end(args_copy);*/
  return ret;
}
dbus_bool_t _dbus_string_append_printf(DBusString *str, const char *format, ...) {
  va_list args;
  dbus_bool_t retval;
  va_start(args, format);
  retval = _dbus_string_append_printf_valist(str, format, args);
  va_end(args);
  return retval;
}
dbus_bool_t _dbus_string_append_len(DBusString *str, const char *buffer, int len) {
  DBUS_STRING_PREAMBLE(str);
  _dbus_assert(buffer != NULL);
  _dbus_assert(len >= 0);
  return append(real, buffer, len);
}
dbus_bool_t _dbus_string_append_byte(DBusString *str, unsigned char byte) {
  DBUS_STRING_PREAMBLE(str);
  if (!set_length(real, real->len + 1)) return FALSE;
  real->str[real->len-1] = byte;
  return TRUE;
}
static void delete(DBusRealString *real, int start, int len) {
  if (len == 0) return;
  memmove(real->str + start, real->str + start + len, real->len - (start + len));
  real->len -= len;
  real->str[real->len] = '\0';
}
void _dbus_string_delete(DBusString *str, int start, int len) {
  DBUS_STRING_PREAMBLE(str);
  _dbus_assert(start >= 0);
  _dbus_assert(len >= 0);
  _dbus_assert(start <= real->len);
  _dbus_assert(len <= real->len - start);
  delete(real, start, len);
}
static dbus_bool_t copy(DBusRealString *source, int start, int len, DBusRealString *dest, int insert_at) {
  if (len == 0) return TRUE;
  if (!open_gap(len, dest, insert_at)) return FALSE;
  memmove(dest->str + insert_at,source->str + start, len);
  return TRUE;
}
#define DBUS_STRING_COPY_PREAMBLE(source, start, dest, insert_at)       \
  DBusRealString *real_source = (DBusRealString*)source;               \
  DBusRealString *real_dest = (DBusRealString*)dest;                   \
  _dbus_assert((source) != (dest));                                    \
  DBUS_GENERIC_STRING_PREAMBLE(real_source);                           \
  DBUS_GENERIC_STRING_PREAMBLE(real_dest);                             \
  _dbus_assert(!real_dest->constant);                                  \
  _dbus_assert(!real_dest->locked);                                    \
  _dbus_assert((start) >= 0);                                          \
  _dbus_assert((start) <= real_source->len);                           \
  _dbus_assert((insert_at) >= 0);                                      \
  _dbus_assert((insert_at) <= real_dest->len)
dbus_bool_t _dbus_string_move(DBusString *source, int start, DBusString *dest, int insert_at) {
  DBusRealString *real_source = (DBusRealString*) source;
  _dbus_assert(start <= real_source->len);
  return _dbus_string_move_len(source, start,real_source->len - start, dest, insert_at);
}
dbus_bool_t _dbus_string_copy(const DBusString *source, int start, DBusString *dest, int insert_at) {
  DBUS_STRING_COPY_PREAMBLE(source, start, dest, insert_at);
  return copy(real_source, start,real_source->len - start, real_dest, insert_at);
}
dbus_bool_t _dbus_string_move_len(DBusString *source, int start, int len, DBusString *dest, int insert_at) {
  DBUS_STRING_COPY_PREAMBLE(source, start, dest, insert_at);
  _dbus_assert(len >= 0);
  _dbus_assert((start + len) <= real_source->len);
  if (len == 0) return TRUE;
  else if (start == 0 && len == real_source->len && real_dest->len == 0) {
      #define ASSIGN_DATA(a, b) \
          do { \
              (a)->str = (b)->str; \
              (a)->len = (b)->len; \
              (a)->allocated = (b)->allocated; \
              (a)->align_offset = (b)->align_offset; \
          } while(0);
      DBusRealString tmp;
      ASSIGN_DATA(&tmp, real_source);
      ASSIGN_DATA(real_source, real_dest);
      ASSIGN_DATA(real_dest, &tmp);
      return TRUE;
  } else {
      if (!copy(real_source, start, len, real_dest, insert_at)) return FALSE;
      delete(real_source, start, len);
      return TRUE;
  }
}
dbus_bool_t _dbus_string_copy_len(const DBusString *source, int start, int len, DBusString *dest, int insert_at) {
  DBUS_STRING_COPY_PREAMBLE(source, start, dest, insert_at);
  _dbus_assert(len >= 0);
  _dbus_assert(start <= real_source->len);
  _dbus_assert(len <= real_source->len - start);
  return copy(real_source, start, len, real_dest, insert_at);
}
dbus_bool_t _dbus_string_replace_len(const DBusString *source, int start, int len, DBusString *dest, int replace_at, int replace_len) {
  DBUS_STRING_COPY_PREAMBLE(source, start, dest, replace_at);
  _dbus_assert(len >= 0);
  _dbus_assert(start <= real_source->len);
  _dbus_assert(len <= real_source->len - start);
  _dbus_assert(replace_at >= 0);
  _dbus_assert(replace_at <= real_dest->len);
  _dbus_assert(replace_len <= real_dest->len - replace_at);
  if (len == replace_len) memmove(real_dest->str + replace_at,real_source->str + start, len);
  else if (len < replace_len) {
      memmove(real_dest->str + replace_at,real_source->str + start, len);
      delete(real_dest, replace_at + len,replace_len - len);
  } else {
      int diff;
      _dbus_assert(len > replace_len);
      diff = len - replace_len;
      if (!copy(real_source, start + replace_len, diff, real_dest, replace_at + replace_len)) return FALSE;
      memmove(real_dest->str + replace_at,real_source->str + start, replace_len);
  }
  return TRUE;
}
dbus_bool_t _dbus_string_split_on_byte(DBusString *source, unsigned char byte, DBusString *tail) {
  int byte_position;
  char byte_string[2] = "";
  int head_length;
  int tail_length;
  byte_string[0] = (char)byte;
  if (!_dbus_string_find (source, 0, byte_string, &byte_position)) return FALSE;
  head_length = byte_position;
  tail_length = _dbus_string_get_length (source) - head_length - 1;
  if (!_dbus_string_move_len (source, byte_position + 1, tail_length, tail, 0)) return FALSE;
  if (!_dbus_string_set_length (source, head_length)) return FALSE;
  return TRUE;
}
#define UTF8_COMPUTE(Char, Mask, Len) \
  if (Char < 128) { \
      Len = 1; \
      Mask = 0x7f;\
  } else if ((Char & 0xe0) == 0xc0) { \
      Len = 2; \
      Mask = 0x1f; \
  }	else if ((Char & 0xf0) == 0xe0) { \
      Len = 3; \
      Mask = 0x0f; \
  } else if ((Char & 0xf8) == 0xf0) { \
      Len = 4; \
      Mask = 0x07; \
  } else if ((Char & 0xfc) == 0xf8) { \
      Len = 5; \
      Mask = 0x03; \
  }	else if ((Char & 0xfe) == 0xfc) { \
      Len = 6; \
      Mask = 0x01; \
  } else { \
      Len = 0; \
      Mask = 0; \
  }
#define UTF8_LENGTH(Char)  ((Char) < 0x80 ? 1 : ((Char) < 0x800 ? 2 : ((Char) < 0x10000 ? 3 : ((Char) < 0x200000 ? 4 : ((Char) < 0x4000000 ? 5 : 6)))))
#define UTF8_GET(Result, Chars, Count, Mask, Len) \
  (Result) = (Chars)[0] & (Mask); \
  for ((Count) = 1; (Count) < (Len); ++(Count))	{ \
      if (((Chars)[(Count)] & 0xc0) != 0x80) { \
          (Result) = -1; \
          break; \
	  }	\
      (Result) <<= 6; \
      (Result) |= ((Chars)[(Count)] & 0x3f); \
  }
#define UNICODE_VALID(Char)  ((Char) < 0x110000 && (((Char) & 0xFFFFF800) != 0xD800))
dbus_bool_t _dbus_string_find(const DBusString *str, int start, const char *substr, int *found) {
  return _dbus_string_find_to(str, start, ((const DBusRealString*)str)->len, substr, found);
}
dbus_bool_t _dbus_string_find_eol(const DBusString *str, int start, int *found, int *found_len) {
  int i;
  DBUS_CONST_STRING_PREAMBLE(str);
  _dbus_assert(start <= real->len);
  _dbus_assert(start >= 0);
  i = start;
  while(i < real->len) {
      if (real->str[i] == '\r') {
          if ((i+1) < real->len && real->str[i+1] == '\n') {
              if (found) *found = i;
              if (found_len) *found_len = 2;
              return TRUE;
          } else {
              if (found) *found = i;
              if (found_len) *found_len = 1;
              return TRUE;
          }
      } else if (real->str[i] == '\n') {
          if (found) *found = i;
          if (found_len) *found_len = 1;
          return TRUE;
      }
      ++i;
  }
  if (found) *found = real->len;
  if (found_len) *found_len = 0;
  return FALSE;
}
dbus_bool_t _dbus_string_find_to(const DBusString *str, int start, int end, const char *substr, int *found) {
  int i;
  DBUS_CONST_STRING_PREAMBLE(str);
  _dbus_assert(substr != NULL);
  _dbus_assert(start <= real->len);
  _dbus_assert(start >= 0);
  _dbus_assert(substr != NULL);
  _dbus_assert(end <= real->len);
  _dbus_assert(start <= end);
  if (*substr == '\0') {
      if (found) *found = start;
      return TRUE;
  }
  i = start;
  while(i < end) {
      if (real->str[i] == substr[0]) {
          int j = i + 1;
          while(j < end) {
              if (substr[j - i] == '\0') break;
              else if (real->str[j] != substr[j - i]) break;
              ++j;
          }
          if (substr[j - i] == '\0') {
              if (found) *found = i;
              return TRUE;
          }
      }
      ++i;
  }
  if (found) *found = end;
  return FALSE;  
}
dbus_bool_t _dbus_string_find_blank(const DBusString *str, int start, int *found) {
  int i;
  DBUS_CONST_STRING_PREAMBLE(str);
  _dbus_assert(start <= real->len);
  _dbus_assert(start >= 0);
  i = start;
  while(i < real->len) {
      if (real->str[i] == ' ' || real->str[i] == '\t') {
          if (found) *found = i;
          return TRUE;
      }
      ++i;
  }
  if (found) *found = real->len;
  return FALSE;
}
void _dbus_string_skip_blank(const DBusString *str, int start, int *end) {
  int i;
  DBUS_CONST_STRING_PREAMBLE(str);
  _dbus_assert(start <= real->len);
  _dbus_assert(start >= 0);
  i = start;
  while(i < real->len) {
      if (!DBUS_IS_ASCII_BLANK(real->str[i])) break;
      ++i;
  }
  _dbus_assert(i == real->len || !DBUS_IS_ASCII_WHITE(real->str[i]));
  if (end) *end = i;
}
void _dbus_string_skip_white(const DBusString *str, int start, int *end) {
  int i;
  DBUS_CONST_STRING_PREAMBLE(str);
  _dbus_assert(start <= real->len);
  _dbus_assert(start >= 0);
  i = start;
  while(i < real->len) {
      if (!DBUS_IS_ASCII_WHITE(real->str[i])) break;
      ++i;
  }
  _dbus_assert(i == real->len || !(DBUS_IS_ASCII_WHITE(real->str[i])));
  if (end) *end = i;
}
void _dbus_string_skip_white_reverse(const DBusString *str, int  end, int *start) {
  int i;
  DBUS_CONST_STRING_PREAMBLE(str);
  _dbus_assert(end <= real->len);
  _dbus_assert(end >= 0);
  i = end;
  while(i > 0) {
      if (!DBUS_IS_ASCII_WHITE(real->str[i-1])) break;
      --i;
  }
  _dbus_assert(i >= 0 && (i == 0 || !(DBUS_IS_ASCII_WHITE(real->str[i-1]))));
  if (start) *start = i;
}
dbus_bool_t _dbus_string_pop_line(DBusString *source, DBusString *dest) {
  int eol, eol_len;
  _dbus_string_set_length(dest, 0);
  eol = 0;
  eol_len = 0;
  if (!_dbus_string_find_eol(source, 0, &eol, &eol_len)) {
      _dbus_assert(eol == _dbus_string_get_length(source));
      if (eol == 0) return FALSE;
  }
  if (!_dbus_string_move_len(source, 0, eol + eol_len, dest, 0)) return FALSE;
  if (!_dbus_string_set_length(dest, eol)) {
      _dbus_assert_not_reached("out of memory when shortening a string");
      return FALSE;
  }
  return TRUE;
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
void _dbus_string_delete_first_word(DBusString *str) {
  int i;
  if (_dbus_string_find_blank(str, 0, &i)) _dbus_string_skip_blank(str, i, &i);
  _dbus_string_delete(str, 0, i);
}
#endif
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
void _dbus_string_delete_leading_blanks(DBusString *str) {
  int i;
  _dbus_string_skip_blank(str, 0, &i);
  if (i > 0) _dbus_string_delete(str, 0, i);
}
#endif
void _dbus_string_chop_white(DBusString *str) {
  int i;
  _dbus_string_skip_white(str, 0, &i);
  if (i > 0) _dbus_string_delete(str, 0, i);
  _dbus_string_skip_white_reverse(str, _dbus_string_get_length(str), &i);
  _dbus_string_set_length(str, i);
}
dbus_bool_t _dbus_string_equal(const DBusString *a, const DBusString *b) {
  const unsigned char *ap;
  const unsigned char *bp;
  const unsigned char *a_end;
  const DBusRealString *real_a = (const DBusRealString*)a;
  const DBusRealString *real_b = (const DBusRealString*)b;
  DBUS_GENERIC_STRING_PREAMBLE(real_a);
  DBUS_GENERIC_STRING_PREAMBLE(real_b);
  if (real_a->len != real_b->len) return FALSE;
  ap = real_a->str;
  bp = real_b->str;
  a_end = real_a->str + real_a->len;
  while(ap != a_end) {
      if (*ap != *bp) return FALSE;
      ++ap;
      ++bp;
  }
  return TRUE;
}
dbus_bool_t _dbus_string_equal_len(const DBusString *a, const DBusString *b, int len) {
  const unsigned char *ap;
  const unsigned char *bp;
  const unsigned char *a_end;
  const DBusRealString *real_a = (const DBusRealString*)a;
  const DBusRealString *real_b = (const DBusRealString*)b;
  DBUS_GENERIC_STRING_PREAMBLE(real_a);
  DBUS_GENERIC_STRING_PREAMBLE(real_b);
  if (real_a->len != real_b->len && (real_a->len < len || real_b->len < len)) return FALSE;
  ap = real_a->str;
  bp = real_b->str;
  a_end = real_a->str + MIN(real_a->len, len);
  while(ap != a_end) {
      if (*ap != *bp) return FALSE;
      ++ap;
      ++bp;
  }
  return TRUE;
}
dbus_bool_t _dbus_string_equal_substring(const DBusString  *a, int a_start, int a_len, const DBusString *b, int b_start) {
  const unsigned char *ap;
  const unsigned char *bp;
  const unsigned char *a_end;
  const DBusRealString *real_a = (const DBusRealString*)a;
  const DBusRealString *real_b = (const DBusRealString*)b;
  DBUS_GENERIC_STRING_PREAMBLE(real_a);
  DBUS_GENERIC_STRING_PREAMBLE(real_b);
  _dbus_assert(a_start >= 0);
  _dbus_assert(a_len >= 0);
  _dbus_assert(a_start <= real_a->len);
  _dbus_assert(a_len <= real_a->len - a_start);
  _dbus_assert(b_start >= 0);
  _dbus_assert(b_start <= real_b->len);
  if (a_len > real_b->len - b_start) return FALSE;
  ap = real_a->str + a_start;
  bp = real_b->str + b_start;
  a_end = ap + a_len;
  while(ap != a_end) {
      if (*ap != *bp) return FALSE;
      ++ap;
      ++bp;
  }
  _dbus_assert(bp <= (real_b->str + real_b->len));
  return TRUE;
}
dbus_bool_t _dbus_string_equal_c_str(const DBusString *a, const char *c_str) {
  const unsigned char *ap;
  const unsigned char *bp;
  const unsigned char *a_end;
  const DBusRealString *real_a = (const DBusRealString*)a;
  DBUS_GENERIC_STRING_PREAMBLE(real_a);
  _dbus_assert(c_str != NULL);
  ap = real_a->str;
  bp = (const unsigned char*)c_str;
  a_end = real_a->str + real_a->len;
  while(ap != a_end && *bp) {
      if (*ap != *bp) return FALSE;
      ++ap;
      ++bp;
  }
  if (ap != a_end || *bp) return FALSE;
  return TRUE;
}
dbus_bool_t _dbus_string_starts_with_c_str(const DBusString *a, const char *c_str) {
  const unsigned char *ap;
  const unsigned char *bp;
  const unsigned char *a_end;
  const DBusRealString *real_a = (const DBusRealString*)a;
  DBUS_GENERIC_STRING_PREAMBLE(real_a);
  _dbus_assert(c_str != NULL);
  ap = real_a->str;
  bp = (const unsigned char*)c_str;
  a_end = real_a->str + real_a->len;
  while(ap != a_end && *bp) {
      if (*ap != *bp) return FALSE;
      ++ap;
      ++bp;
  }
  if (*bp == '\0') return TRUE;
  else return FALSE;
}
dbus_bool_t _dbus_string_append_byte_as_hex(DBusString *str, unsigned char byte) {
  const char hexdigits[16] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      'a', 'b', 'c', 'd', 'e', 'f'
  };
  if (!_dbus_string_append_byte(str,hexdigits[(byte >> 4)])) return FALSE;
  if (!_dbus_string_append_byte(str,hexdigits[(byte & 0x0f)])) {
      _dbus_string_set_length(str,_dbus_string_get_length(str) - 1);
      return FALSE;
  }
  return TRUE;
}
dbus_bool_t _dbus_string_hex_encode(const DBusString *source, int start, DBusString *dest, int insert_at) {
  DBusString result;
  const unsigned char *p;
  const unsigned char *end;
  dbus_bool_t retval;
  _dbus_assert(start <= _dbus_string_get_length(source));
  if (!_dbus_string_init(&result)) return FALSE;
  retval = FALSE;
  p = (const unsigned char*)_dbus_string_get_const_data(source);
  end = p + _dbus_string_get_length(source);
  p += start;
  while(p != end) {
      if (!_dbus_string_append_byte_as_hex(&result, *p)) goto out;
      ++p;
  }
  if (!_dbus_string_move(&result, 0, dest, insert_at)) goto out;
  retval = TRUE;
out:
  _dbus_string_free(&result);
  return retval;
}
dbus_bool_t _dbus_string_hex_decode(const DBusString *source, int start, int *end_return, DBusString *dest, int insert_at) {
  DBusString result;
  const unsigned char *p;
  const unsigned char *end;
  dbus_bool_t retval;
  dbus_bool_t high_bits;
  _dbus_assert(start <= _dbus_string_get_length(source));
  if (!_dbus_string_init(&result)) return FALSE;
  retval = FALSE;
  high_bits = TRUE;
  p = (const unsigned char*)_dbus_string_get_const_data(source);
  end = p + _dbus_string_get_length(source);
  p += start;
  while(p != end) {
      unsigned int val;
      switch(*p) {
          case '0': val = 0; break;
          case '1': val = 1; break;
          case '2': val = 2; break;
          case '3': val = 3; break;
          case '4': val = 4; break;
          case '5': val = 5; break;
          case '6': val = 6; break;
          case '7': val = 7; break;
          case '8': val = 8; break;
          case '9': val = 9; break;
          case 'a': case 'A': val = 10; break;
          case 'b': case 'B': val = 11; break;
          case 'c': case 'C': val = 12; break;
          case 'd': case 'D': val = 13; break;
          case 'e': case 'E': val = 14; break;
          case 'f': case 'F': val = 15; break;
          default: goto done;
      }
      if (high_bits) {
          if (!_dbus_string_append_byte(&result, val << 4)) goto out;
      } else {
          int len;
          unsigned char b;
          len = _dbus_string_get_length(&result);
          b = _dbus_string_get_byte(&result, len - 1);
          b |= val;
          _dbus_string_set_byte(&result, len - 1, b);
      }
      high_bits = !high_bits;
      ++p;
  }
done:
  if (!_dbus_string_move(&result, 0, dest, insert_at)) goto out;
  if (end_return) *end_return = p - (const unsigned char*)_dbus_string_get_const_data(source);
  retval = TRUE;
out:
  _dbus_string_free(&result);
  return retval;
}
dbus_bool_t _dbus_string_validate_ascii(const DBusString *str, int start, int len) {
  const unsigned char *s;
  const unsigned char *end;
  DBUS_CONST_STRING_PREAMBLE(str);
  _dbus_assert(start >= 0);
  _dbus_assert(start <= real->len);
  _dbus_assert(len >= 0);
  if (len > real->len - start) return FALSE;
  s = real->str + start;
  end = s + len;
  while(s != end) {
      if (_DBUS_UNLIKELY(!_DBUS_ISASCII(*s))) return FALSE;
      ++s;
  }
  return TRUE;
}
void _dbus_string_tolower_ascii(const DBusString *str, int start, int len) {
  unsigned char *s;
  unsigned char *end;
  DBUS_STRING_PREAMBLE(str);
  _dbus_assert(start >= 0);
  _dbus_assert(start <= real->len);
  _dbus_assert(len >= 0);
  _dbus_assert(len <= real->len - start);
  s = real->str + start;
  end = s + len;
  while(s != end) {
      if (*s >= 'A' && *s <= 'Z') *s += 'a' - 'A';
      ++s;
  }
}
void _dbus_string_toupper_ascii(const DBusString *str, int start, int len) {
  unsigned char *s;
  unsigned char *end;
  DBUS_STRING_PREAMBLE(str);
  _dbus_assert(start >= 0);
  _dbus_assert(start <= real->len);
  _dbus_assert(len >= 0);
  _dbus_assert(len <= real->len - start);
  s = real->str + start;
  end = s + len;
  while(s != end) {
      if (*s >= 'a' && *s <= 'z') *s += 'A' - 'a';
      ++s;
  }
}
dbus_bool_t _dbus_string_validate_utf8(const DBusString *str, int start, int len) {
  const unsigned char *p;
  const unsigned char *end;
  DBUS_CONST_STRING_PREAMBLE(str);
  _dbus_assert(start >= 0);
  _dbus_assert(start <= real->len);
  _dbus_assert(len >= 0);
  if (_DBUS_UNLIKELY(len > real->len - start)) return FALSE;
  p = real->str + start;
  end = p + len;
  while(p < end) {
      int i, mask, char_len;
      dbus_unichar_t result;
      if (*p == '\0') break;
      if (*p < 128) {
          ++p;
          continue;
      }
      UTF8_COMPUTE(*p, mask, char_len);
      if (_DBUS_UNLIKELY(char_len == 0)) break;
      if (_DBUS_UNLIKELY((end - p) < char_len)) break;
      UTF8_GET(result, p, i, mask, char_len);
      if (_DBUS_UNLIKELY(UTF8_LENGTH(result) != char_len)) break;
  #if 0
      if (_DBUS_UNLIKELY(result == (dbus_unichar_t)-1))  break;
  #endif
      if (_DBUS_UNLIKELY(!UNICODE_VALID(result))) break;
      _dbus_assert(result != (dbus_unichar_t)-1);
      p += char_len;
  }
  if (_DBUS_UNLIKELY(p != end)) return FALSE;
  else return TRUE;
}
dbus_bool_t _dbus_string_validate_nul(const DBusString *str, int start, int len) {
  const unsigned char *s;
  const unsigned char *end;
  DBUS_CONST_STRING_PREAMBLE (str);
  _dbus_assert(start >= 0);
  _dbus_assert(len >= 0);
  _dbus_assert(start <= real->len);
  if (len > real->len - start) return FALSE;
  s = real->str + start;
  end = s + len;
  while(s != end) {
      if (_DBUS_UNLIKELY(*s != '\0')) return FALSE;
      ++s;
  }
  return TRUE;
}
void _dbus_string_zero(DBusString *str) {
  DBUS_STRING_PREAMBLE(str);
  memset(real->str - real->align_offset, '\0', real->allocated);
}