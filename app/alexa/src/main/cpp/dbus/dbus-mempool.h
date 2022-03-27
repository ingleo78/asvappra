#ifndef DBUS_MEMPOOL_H
#define DBUS_MEMPOOL_H

#include "dbus-internals.h"
#include "dbus-memory.h"
#include "dbus-types.h"

DBUS_BEGIN_DECLS
typedef struct DBusMemPool DBusMemPool;
DBUS_PRIVATE_EXPORT DBusMemPool* _dbus_mem_pool_new(int element_size, dbus_bool_t zero_elements);
DBUS_PRIVATE_EXPORT void _dbus_mem_pool_free(DBusMemPool *pool);
DBUS_PRIVATE_EXPORT void* _dbus_mem_pool_alloc(DBusMemPool *pool);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_mem_pool_dealloc(DBusMemPool *pool, void *element);
void _dbus_mem_pool_get_stats(DBusMemPool *pool, dbus_uint32_t *in_use_p, dbus_uint32_t *in_free_list_p, dbus_uint32_t *allocated_p);
DBUS_END_DECLS

#endif