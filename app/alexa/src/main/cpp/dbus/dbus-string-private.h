#ifndef DBUS_STRING_PRIVATE_H
#define DBUS_STRING_PRIVATE_H

#include "dbus-internals.h"
#include "dbus-memory.h"
#include "dbus-types.h"

#ifndef DBUS_CAN_USE_DBUS_STRING_PRIVATE
#error "Don't go including dbus-string-private.h for no good reason"
#endif

DBUS_BEGIN_DECLS
typedef struct {
  unsigned char *str;
  int len;
  int allocated;
  unsigned int constant : 1;
  unsigned int locked : 1;
  unsigned int valid : 1;
  unsigned int align_offset : 3;
} DBusRealString;
_DBUS_STATIC_ASSERT(sizeof(DBusRealString) == sizeof(DBusString));
#define _DBUS_STRING_MAX_LENGTH  (_DBUS_INT32_MAX - _DBUS_STRING_ALLOCATION_PADDING)
#define DBUS_GENERIC_STRING_PREAMBLE(real) \
  do { \
      (void)real; \
      _dbus_assert((real) != NULL); \
      _dbus_assert((real)->valid); \
      _dbus_assert((real)->len >= 0); \
      _dbus_assert((real)->allocated >= 0); \
      _dbus_assert((real)->len <= ((real)->allocated - _DBUS_STRING_ALLOCATION_PADDING)); \
      _dbus_assert((real)->len <= _DBUS_STRING_MAX_LENGTH); \
  } while(0);
#define DBUS_STRING_PREAMBLE(str) \
  DBusRealString *real = (DBusRealString*)str; \
  DBUS_GENERIC_STRING_PREAMBLE(real); \
  _dbus_assert(!(real)->constant); \
  _dbus_assert(!(real)->locked)
#define DBUS_LOCKED_STRING_PREAMBLE(str) \
  DBusRealString *real = (DBusRealString*)str; \
  DBUS_GENERIC_STRING_PREAMBLE(real); \
  _dbus_assert(!(real)->constant)
#define DBUS_CONST_STRING_PREAMBLE(str) \
  const DBusRealString *real = (DBusRealString*)str; \
  DBUS_GENERIC_STRING_PREAMBLE(real)
#define DBUS_IS_ASCII_BLANK(c)  ((c) == ' ' || (c) == '\t')
#define DBUS_IS_ASCII_WHITE(c)  ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r')
DBUS_END_DECLS

#endif