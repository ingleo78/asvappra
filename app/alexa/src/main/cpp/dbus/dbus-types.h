#if defined (DBUS_INSIDE_DBUS_H) && defined (DBUS_COMPILATION)
#error "Only <dbus/dbus.h> can be included directly, this file may disappear or change contents."
#endif

#ifndef DBUS_TYPES_H
#define DBUS_TYPES_H

#include <stddef.h>
#include "dbus-arch-deps.h"

typedef int16_t dbus_int16_t;
typedef uint16_t dbus_uint16_t;
typedef int32_t dbus_int32_t;
typedef uint32_t dbus_uint32_t;
typedef dbus_uint32_t dbus_unichar_t;
typedef dbus_uint32_t dbus_bool_t;
typedef struct {
  dbus_uint32_t first32;
  dbus_uint32_t second32;
} DBus8ByteStruct;
typedef union {
  unsigned char bytes[8];
  dbus_int16_t  i16;
  dbus_uint16_t u16;
  dbus_int32_t  i32;
  dbus_uint32_t u32;
  dbus_bool_t   bool_val;
  dbus_int64_t  i64;
  dbus_uint64_t u64;
  DBus8ByteStruct eight;
  double dbl;
  unsigned char byt;
  char *str;
  int fd;
} DBusBasicValue;

#endif