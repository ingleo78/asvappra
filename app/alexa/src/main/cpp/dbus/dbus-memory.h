#if defined (DBUS_INSIDE_DBUS_H) && !defined (DBUS_COMPILATION)
#error "Only <dbus/dbus.h> can be included directly, this file may disappear or change contents."
#endif

#ifndef DBUS_MEMORY_H
#define DBUS_MEMORY_H

#include <stddef.h>
#include "dbus-macros.h"

DBUS_BEGIN_DECLS
typedef unsigned int size_t;
DBUS_EXPORT DBUS_MALLOC DBUS_ALLOC_SIZE(1) void* dbus_malloc(size_t bytes);
DBUS_EXPORT DBUS_MALLOC DBUS_ALLOC_SIZE(1) void* dbus_malloc0(size_t bytes);
DBUS_EXPORT DBUS_MALLOC DBUS_ALLOC_SIZE(2) void* dbus_realloc(void *memory, size_t bytes);
DBUS_EXPORT void  dbus_free(void *memory);
#define dbus_new(type, count)((type*)dbus_malloc(sizeof(type) * (count)))
#define dbus_new0(type, count)((type*)dbus_malloc0(sizeof(type) * (count)))
DBUS_EXPORT void dbus_free_string_array(char **str_array);
typedef void (*DBusFreeFunction)(void *memory);
DBUS_EXPORT void dbus_shutdown(void);
DBUS_END_DECLS

#endif