#ifndef DBUS_DATASLOT_H
#define DBUS_DATASLOT_H

#include "dbus-internals.h"

DBUS_BEGIN_DECLS
typedef struct DBusDataSlotAllocator DBusDataSlotAllocator;
typedef struct DBusDataSlotList DBusDataSlotList;
typedef struct DBusDataSlot DBusDataSlot;
struct DBusDataSlot {
  void *data;
  DBusFreeFunction free_data_func;
};
typedef struct DBusAllocatedSlot DBusAllocatedSlot;
struct DBusAllocatedSlot {
  dbus_int32_t slot_id;
  int refcount;
};
struct DBusDataSlotAllocator {
  DBusAllocatedSlot *allocated_slots;
  int  n_allocated_slots;
  int  n_used_slots;
  DBusGlobalLock lock;
};
#define _DBUS_DATA_SLOT_ALLOCATOR_INIT(x) { NULL, 0, 0, x }
struct DBusDataSlotList {
  DBusDataSlot *slots;
  int n_slots;
};
dbus_bool_t _dbus_data_slot_allocator_init(DBusDataSlotAllocator *allocator, DBusGlobalLock lock);
dbus_bool_t _dbus_data_slot_allocator_alloc(DBusDataSlotAllocator *allocator, int *slot_id_p);
void _dbus_data_slot_allocator_free(DBusDataSlotAllocator *allocator, int *slot_id_p);
void _dbus_data_slot_list_init(DBusDataSlotList *list);
dbus_bool_t _dbus_data_slot_list_set(DBusDataSlotAllocator *allocator, DBusDataSlotList *list, int slot, void *data, DBusFreeFunction free_data_func,
                                     DBusFreeFunction *old_free_func, void **old_data);
void* _dbus_data_slot_list_get(DBusDataSlotAllocator *allocator, DBusDataSlotList *list, int slot);
void _dbus_data_slot_list_clear(DBusDataSlotList *list);
void _dbus_data_slot_list_free(DBusDataSlotList *list);
DBUS_END_DECLS

#endif