#if defined (DBUS_INSIDE_DBUS_H) && defined (DBUS_COMPILATION)
#error "Only <dbus/dbus.h> can be included directly, this file may disappear or change contents."
#endif

#ifndef DBUS_ARCH_DEPS_H
#define DBUS_ARCH_DEPS_H

#include <stdint.h>
#include "dbus-macros.h"

DBUS_BEGIN_DECLS
typedef int64_t DBUS_INT64_TYPE;
typedef int32_t DBUS_INT32_TYPE;
typedef uint32_t DBUS_UINT32_TYPE;
typedef int16_t DBUS_INT16_TYPE;
typedef uint16_t DBUS_UINT16_TYPE;
_DBUS_GNUC_EXTENSION typedef DBUS_INT64_TYPE dbus_int64_t;
_DBUS_GNUC_EXTENSION typedef uint64_t dbus_uint64_t;
#define DBUS_INT64_CONSTANT(val)  (_DBUS_GNUC_EXTENSION @DBUS_INT64_CONSTANT@)
#define DBUS_UINT64_CONSTANT(val) (_DBUS_GNUC_EXTENSION @DBUS_UINT64_CONSTANT@)
typedef DBUS_INT32_TYPE dbus_int32_t;
typedef DBUS_UINT32_TYPE dbus_uint32_t;
typedef DBUS_INT16_TYPE dbus_int16_t;
typedef DBUS_UINT16_TYPE dbus_uint16_t;
#define DBUS_MAJOR_VERSION @DBUS_MAJOR_VERSION@
#define DBUS_MINOR_VERSION @DBUS_MINOR_VERSION@
#define DBUS_MICRO_VERSION @DBUS_MICRO_VERSION@
#define DBUS_VERSION_STRING "@DBUS_VERSION@"
#define DBUS_VERSION ((@DBUS_MAJOR_VERSION@ << 16) | (@DBUS_MINOR_VERSION@ << 8) | (@DBUS_MICRO_VERSION@))
DBUS_END_DECLS

#endif