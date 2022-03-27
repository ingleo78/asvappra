#ifndef DBUS_LIST_H
#define DBUS_LIST_H

#include "dbus-internals.h"
#include "dbus-memory.h"
#include "dbus-types.h"
#include "dbus-sysdeps.h"

DBUS_BEGIN_DECLS
struct DBusList {
  DBusList *prev;
  DBusList *next;
  void *data;
};
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_list_append(DBusList **list, void *data);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_list_prepend(DBusList **list, void *data);
dbus_bool_t _dbus_list_insert_before(DBusList **list, DBusList *before_this_link, void *data);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_list_insert_after(DBusList **list, DBusList *after_this_link, void *data);
DBUS_PRIVATE_EXPORT void _dbus_list_insert_before_link(DBusList **list, DBusList *before_this_link, DBusList *link);
DBUS_PRIVATE_EXPORT void _dbus_list_insert_after_link(DBusList **list, DBusList *after_this_link, DBusList *link);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_list_remove(DBusList **list, void *data);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_list_remove_last(DBusList **list, void *data);
DBUS_PRIVATE_EXPORT void _dbus_list_remove_link(DBusList **list, DBusList *link);
DBUS_PRIVATE_EXPORT DBusList* _dbus_list_find_last(DBusList **list, void *data);
DBUS_PRIVATE_EXPORT void _dbus_list_clear(DBusList **list);
DBUS_PRIVATE_EXPORT DBusList* _dbus_list_get_first_link(DBusList **list);
DBUS_PRIVATE_EXPORT DBusList* _dbus_list_get_last_link(DBusList **list);
DBUS_PRIVATE_EXPORT void* _dbus_list_get_last(DBusList **list);
DBUS_PRIVATE_EXPORT void* _dbus_list_get_first(DBusList **list);
DBUS_PRIVATE_EXPORT void* _dbus_list_pop_first(DBusList **list);
DBUS_PRIVATE_EXPORT void* _dbus_list_pop_last(DBusList **list);
DBUS_PRIVATE_EXPORT DBusList* _dbus_list_pop_first_link(DBusList **list);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_list_copy(DBusList **list, DBusList **dest);
DBUS_PRIVATE_EXPORT int _dbus_list_get_length(DBusList **list);
DBUS_PRIVATE_EXPORT DBusList* _dbus_list_alloc_link(void *data);
DBUS_PRIVATE_EXPORT void _dbus_list_free_link(DBusList *link);
DBUS_PRIVATE_EXPORT void _dbus_list_unlink(DBusList **list, DBusList *link);
DBUS_PRIVATE_EXPORT void _dbus_list_append_link(DBusList **list, DBusList *link);
DBUS_PRIVATE_EXPORT void _dbus_list_prepend_link(DBusList **list, DBusList *link);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_list_length_is_one(DBusList **list);
DBUS_PRIVATE_EXPORT void _dbus_list_foreach(DBusList **list, DBusForeachFunction function, void *data);
#define _dbus_list_get_next_link(list, link)  ((link)->next == *(list) ? NULL : (link)->next)
#define _dbus_list_get_prev_link(list, link)  ((link) == *(list) ? NULL : (link)->prev)
DBUS_PRIVATE_EXPORT void _dbus_list_get_stats(dbus_uint32_t *in_use_p, dbus_uint32_t *in_free_list_p, dbus_uint32_t *allocated_p);
DBUS_END_DECLS

#endif