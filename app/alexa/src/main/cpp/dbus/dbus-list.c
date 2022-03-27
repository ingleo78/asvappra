#include "config.h"
#include "dbus-internals.h"
#include "dbus-list.h"
#include "dbus-mempool.h"
#include "dbus-threads-internal.h"
#include "dbus-test-tap.h"

static DBusMemPool *list_pool;
static DBusList* alloc_link(void *data) {
  DBusList *link;
  if (!_DBUS_LOCK(list)) return FALSE;
  if (list_pool == NULL) {
      list_pool = _dbus_mem_pool_new(sizeof(DBusList), TRUE);
      if (list_pool == NULL) {
          _DBUS_UNLOCK(list);
          return NULL;
      }
      link = _dbus_mem_pool_alloc(list_pool);
      if (link == NULL) {
          _dbus_mem_pool_free(list_pool);
          list_pool = NULL;
          _DBUS_UNLOCK(list);
          return NULL;
      }
  } else link = _dbus_mem_pool_alloc(list_pool);
  if (link) link->data = data;
  _DBUS_UNLOCK(list);
  return link;
}
static void free_link(DBusList *link) {
  if (!_DBUS_LOCK(list)) _dbus_assert_not_reached("we should have initialized global locks before we allocated a linked-list link");
  if (_dbus_mem_pool_dealloc(list_pool, link)) {
      _dbus_mem_pool_free(list_pool);
      list_pool = NULL;
  }
  _DBUS_UNLOCK(list);
}
static void link_before(DBusList **list, DBusList *before_this_link, DBusList *link) {
  if (*list == NULL) {
      link->prev = link;
      link->next = link;
      *list = link;
  } else {
      link->next = before_this_link;
      link->prev = before_this_link->prev;
      before_this_link->prev = link;
      link->prev->next = link;
      if (before_this_link == *list) *list = link;
  }
}
static void link_after(DBusList **list, DBusList *after_this_link, DBusList *link) {
  if (*list == NULL) {
      link->prev = link;
      link->next = link;
      *list = link;
  } else {
      link->prev = after_this_link;
      link->next = after_this_link->next;
      after_this_link->next = link;
      link->next->prev = link;
  }
}
#ifdef DBUS_ENABLE_STATS
void _dbus_list_get_stats(dbus_uint32_t *in_use_p, dbus_uint32_t *in_free_list_p, dbus_uint32_t *allocated_p) {
  if (!_DBUS_LOCK(list)) {
      *in_use_p = 0;
      *in_free_list_p = 0;
      *allocated_p = 0;
      return;
  }
  _dbus_mem_pool_get_stats(list_pool, in_use_p, in_free_list_p, allocated_p);
  _DBUS_UNLOCK(list);
}
#endif
DBusList* _dbus_list_alloc_link(void *data) {
  return alloc_link(data);
}
void _dbus_list_free_link(DBusList *link) {
  free_link(link);
}
dbus_bool_t _dbus_list_append(DBusList **list, void *data) {
  if (!_dbus_list_prepend(list, data)) return FALSE;
  *list = (*list)->next;
  return TRUE;
}
dbus_bool_t _dbus_list_prepend(DBusList **list, void *data) {
  DBusList *link;
  link = alloc_link(data);
  if (link == NULL) return FALSE;
  link_before(list, *list, link);
  return TRUE;
}
void _dbus_list_append_link(DBusList **list, DBusList *link) {
  _dbus_list_prepend_link(list, link);
  *list = (*list)->next;
}
void _dbus_list_prepend_link(DBusList **list, DBusList *link) {
  link_before(list, *list, link);
}
dbus_bool_t _dbus_list_insert_after(DBusList **list, DBusList *after_this_link, void *data) {
  DBusList *link;
  if (after_this_link == NULL) return _dbus_list_prepend(list, data);
  else {
      link = alloc_link(data);
      if (link == NULL) return FALSE;
      link_after(list, after_this_link, link);
  }
  return TRUE;
}
void _dbus_list_insert_before_link(DBusList **list, DBusList *before_this_link, DBusList *link) {
  if (before_this_link == NULL) _dbus_list_append_link(list, link);
  else link_before(list, before_this_link, link);
}
void _dbus_list_insert_after_link(DBusList **list, DBusList *after_this_link, DBusList *link) {
  if (after_this_link == NULL) _dbus_list_prepend_link(list, link);
  else link_after(list, after_this_link, link);
}
dbus_bool_t _dbus_list_remove(DBusList **list, void *data) {
  DBusList *link;
  link = *list;
  while(link != NULL) {
      if (link->data == data) {
          _dbus_list_remove_link(list, link);
          return TRUE;
      }
      link = _dbus_list_get_next_link(list, link);
  }
  return FALSE;
}
dbus_bool_t _dbus_list_remove_last(DBusList **list, void *data) {
  DBusList *link;
  link = _dbus_list_find_last(list, data);
  if (link) {
      _dbus_list_remove_link(list, link);
      return TRUE;
  } else return FALSE;
}
DBusList* _dbus_list_find_last(DBusList **list, void *data) {
  DBusList *link;
  link = _dbus_list_get_last_link(list);
  while(link != NULL) {
      if (link->data == data) return link;
      link = _dbus_list_get_prev_link(list, link);
  }
  return NULL;
}
void _dbus_list_unlink(DBusList **list, DBusList *link) {
  if (link->next == link) *list = NULL;
  else {
      link->prev->next = link->next;
      link->next->prev = link->prev;
      if (*list == link) *list = link->next;
  }
  link->next = NULL;
  link->prev = NULL;
}
void _dbus_list_remove_link(DBusList **list, DBusList *link) {
  _dbus_list_unlink(list, link);
  free_link(link);
}
void _dbus_list_clear(DBusList **list) {
  DBusList *link;
  link = *list;
  while(link != NULL) {
      DBusList *next = _dbus_list_get_next_link(list, link);
      free_link (link);
      link = next;
  }
  *list = NULL;
}
DBusList* _dbus_list_get_first_link(DBusList **list) {
  return *list;
}
DBusList* _dbus_list_get_last_link(DBusList **list) {
  if (*list == NULL) return NULL;
  else return (*list)->prev;
}
void* _dbus_list_get_last(DBusList **list) {
  if (*list == NULL) return NULL;
  else return (*list)->prev->data;
}
void* _dbus_list_get_first(DBusList **list) {
  if (*list == NULL) return NULL;
  else return (*list)->data;
}
DBusList* _dbus_list_pop_first_link(DBusList **list) {
  DBusList *link;
  link = _dbus_list_get_first_link(list);
  if (link == NULL) return NULL;
  _dbus_list_unlink(list, link);
  return link;
}
void* _dbus_list_pop_first(DBusList **list) {
  DBusList *link;
  void *data;
  link = _dbus_list_get_first_link(list);
  if (link == NULL) return NULL;
  data = link->data;
  _dbus_list_remove_link(list, link);
  return data;
}
void* _dbus_list_pop_last(DBusList **list) {
  DBusList *link;
  void *data;
  link = _dbus_list_get_last_link(list);
  if (link == NULL) return NULL;
  data = link->data;
  _dbus_list_remove_link(list, link);
  return data;
}
dbus_bool_t _dbus_list_copy(DBusList **list, DBusList **dest) {
  DBusList *link;
  _dbus_assert(list != dest);
  *dest = NULL;
  link = *list;
  while(link != NULL) {
      if (!_dbus_list_append(dest, link->data)) {
          _dbus_list_clear(dest);
          return FALSE;
      }
      link = _dbus_list_get_next_link(list, link);
  }
  return TRUE;
}
int _dbus_list_get_length(DBusList **list) {
  DBusList *link;
  int length;
  length = 0;
  link = *list;
  while(link != NULL) {
      ++length;
      link = _dbus_list_get_next_link(list, link);
  }
  return length;
}
void _dbus_list_foreach(DBusList **list, DBusForeachFunction function, void *data) {
  DBusList *link;
  link = *list;
  while(link != NULL) {
      DBusList *next = _dbus_list_get_next_link(list, link);
      (*function)(link->data, data);
      link = next;
    }
}
dbus_bool_t _dbus_list_length_is_one(DBusList **list) {
  return (*list != NULL && (*list)->next == *list);
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include <stdio.h>
#include "dbus-test.h"

static void verify_list(DBusList **list) {
  DBusList *link;
  int length;
  link = *list;
  if (link == NULL) return;
  if (link->next == link) {
      _dbus_assert(link->prev == link);
      _dbus_assert(*list == link);
      return;
  }
  length = 0;
  do {
      length += 1;
      _dbus_assert(link->prev->next == link);
      _dbus_assert(link->next->prev == link);
      link = link->next;
  } while(link != *list);
  _dbus_assert(length == _dbus_list_get_length(list));
  if (length == 1) { _dbus_assert(_dbus_list_length_is_one(list)); }
  else { _dbus_assert(!_dbus_list_length_is_one(list)); }
}
static dbus_bool_t is_ascending_sequence(DBusList **list) {
  DBusList *link;
  int prev;
  prev = _DBUS_INT_MIN;
  link = _dbus_list_get_first_link(list);
  while(link != NULL) {
      int v = _DBUS_POINTER_TO_INT(link->data);
      if (v <= prev) return FALSE;
      prev = v;
      link = _dbus_list_get_next_link(list, link);
  }
  return TRUE;
}
static dbus_bool_t is_descending_sequence(DBusList **list) {
  DBusList *link;
  int prev;
  prev = _DBUS_INT_MAX;
  link = _dbus_list_get_first_link(list);
  while(link != NULL) {
      int v = _DBUS_POINTER_TO_INT(link->data);
      if (v >= prev) return FALSE;
      prev = v;
      link = _dbus_list_get_next_link(list, link);
  }
  return TRUE;
}
static dbus_bool_t all_even_values(DBusList **list) {
  DBusList *link;
  link = _dbus_list_get_first_link(list);
  while(link != NULL) {
      int v = _DBUS_POINTER_TO_INT(link->data);
      if ((v % 2) != 0) return FALSE;
      link = _dbus_list_get_next_link(list, link);
  }
  return TRUE;
}
static dbus_bool_t all_odd_values(DBusList **list) {
  DBusList *link;
  link = _dbus_list_get_first_link(list);
  while(link != NULL) {
      int v = _DBUS_POINTER_TO_INT(link->data);
      if ((v % 2) == 0) return FALSE;
      link = _dbus_list_get_next_link(list, link);
  }
  return TRUE;
}
static dbus_bool_t lists_equal(DBusList **list1, DBusList **list2) {
  DBusList *link1;
  DBusList *link2;
  link1 = _dbus_list_get_first_link(list1);
  link2 = _dbus_list_get_first_link(list2);
  while(link1 && link2) {
      if (link1->data != link2->data) return FALSE;
      link1 = _dbus_list_get_next_link(list1, link1);
      link2 = _dbus_list_get_next_link(list2, link2);
  }
  if (link1 || link2) return FALSE;
  return TRUE;
}
dbus_bool_t _dbus_list_test(void) {
  DBusList *list1;
  DBusList *list2;
  DBusList *link1;
  DBusList *link2;
  DBusList *copy1;
  DBusList *copy2;
  int i;
  list1 = NULL;
  list2 = NULL;
  i = 0;
  while(i < 10) {
      if (!_dbus_list_append(&list1, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for append");
      if (!_dbus_list_prepend(&list2, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("count not allocate for prepend");
      ++i;
      verify_list(&list1);
      verify_list(&list2);
      _dbus_assert(_dbus_list_get_length(&list1) == i);
      _dbus_assert(_dbus_list_get_length(&list2) == i);
  }
  _dbus_assert(is_ascending_sequence(&list1));
  _dbus_assert(is_descending_sequence(&list2));
  _dbus_list_clear(&list1);
  _dbus_list_clear(&list2);
  verify_list(&list1);
  verify_list(&list2);
  i = 0;
  while(i < 10) {
      if (!_dbus_list_append(&list1, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for append");
      if (!_dbus_list_prepend(&list2, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for prepend");
      ++i;
  }
  --i;
  while(i >= 0) {
      void *got_data1;
      void *got_data2;
      void *data1;
      void *data2;
      got_data1 = _dbus_list_get_last(&list1);
      got_data2 = _dbus_list_get_first(&list2);
      data1 = _dbus_list_pop_last(&list1);
      data2 = _dbus_list_pop_first(&list2);
      _dbus_assert(got_data1 == data1);
      _dbus_assert(got_data2 == data2);
      _dbus_assert(_DBUS_POINTER_TO_INT(data1) == i);
      _dbus_assert(_DBUS_POINTER_TO_INT(data2) == i);
      verify_list(&list1);
      verify_list(&list2);
      _dbus_assert(is_ascending_sequence(&list1));
      _dbus_assert(is_descending_sequence(&list2));
      --i;
  }
  _dbus_assert(list1 == NULL);
  _dbus_assert(list2 == NULL);
  i = 0;
  while(i < 10) {
      if (!_dbus_list_append(&list1, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for append");
      if (!_dbus_list_prepend(&list2, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for prepend");
      ++i;
  }
  --i;
  while(i >= 0) {
      DBusList *got_link1;
      DBusList *got_link2;
      void *data1_indirect;
      void *data1;
      void *data2;
      got_link1 = _dbus_list_get_last_link(&list1);
      got_link2 = _dbus_list_get_first_link(&list2);
      link2 = _dbus_list_pop_first_link(&list2);
      _dbus_assert(got_link2 == link2);
      data1_indirect = got_link1->data;
      data1 = _dbus_list_pop_last(&list1);
      _dbus_assert(data1 == data1_indirect);
      data2 = link2->data;
      _dbus_list_free_link(link2);
      _dbus_assert(_DBUS_POINTER_TO_INT(data1) == i);
      _dbus_assert(_DBUS_POINTER_TO_INT(data2) == i);
      verify_list(&list1);
      verify_list(&list2);
      _dbus_assert(is_ascending_sequence(&list1));
      _dbus_assert(is_descending_sequence(&list2));
      --i;
  }
  _dbus_assert(list1 == NULL);
  _dbus_assert(list2 == NULL);
  i = 0;
  while(i < 10) {
      if (!_dbus_list_append(&list1, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for append");
      if (!_dbus_list_prepend(&list2, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for prepend");
      ++i;
      verify_list(&list1);
      verify_list(&list2);
      _dbus_assert(_dbus_list_get_length(&list1) == i);
      _dbus_assert(_dbus_list_get_length(&list2) == i);
  }
  _dbus_assert(is_ascending_sequence(&list1));
  _dbus_assert(is_descending_sequence(&list2));
  --i;
  link2 = _dbus_list_get_first_link(&list2);
  while(link2 != NULL) {
      verify_list(&link2);
      _dbus_assert(_DBUS_POINTER_TO_INT(link2->data) == i);
      link2 = _dbus_list_get_next_link(&list2, link2);
      --i;
  }
  i = 0;
  link1 = _dbus_list_get_first_link(&list1);
  while(link1 != NULL) {
      verify_list(&link1);
      _dbus_assert(_DBUS_POINTER_TO_INT(link1->data) == i);
      link1 = _dbus_list_get_next_link(&list1, link1);
      ++i;
  }
  --i;
  link1 = _dbus_list_get_last_link(&list1);
  while(link1 != NULL) {
      verify_list(&link1);
      _dbus_assert(_DBUS_POINTER_TO_INT(link1->data) == i);
      link1 = _dbus_list_get_prev_link(&list1, link1);
      --i;
  }
  _dbus_list_clear(&list1);
  _dbus_list_clear(&list2);
  i = 0;
  while(i < 10) {
      if (!_dbus_list_append(&list1, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for append");
      if (!_dbus_list_prepend(&list2, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for prepend");
      ++i;
  }
  --i;
  while(i >= 0) {
      if ((i % 2) == 0) {
          if (!_dbus_list_remove(&list1, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("element should have been in list");
          if (!_dbus_list_remove(&list2, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("element should have been in list");
          verify_list(&list1);
          verify_list(&list2);
      }
      --i;
  }
  _dbus_assert(all_odd_values(&list1));
  _dbus_assert(all_odd_values(&list2));
  _dbus_list_clear(&list1);
  _dbus_list_clear(&list2);
  i = 0;
  while(i < 10) {
      if (!_dbus_list_append(&list1, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for append");
      if (!_dbus_list_prepend(&list2, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for prepend");
      ++i;
  }
  --i;
  while(i >= 0) {
      if ((i % 2) != 0) {
          if (!_dbus_list_remove(&list1, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("element should have been in list");
          if (!_dbus_list_remove(&list2, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("element should have been in list");
          verify_list (&list1);
          verify_list (&list2);
      }
      --i;
  }
  _dbus_assert(all_even_values(&list1));
  _dbus_assert(all_even_values(&list2));
  while(list1 != NULL) {
      _dbus_list_remove_link(&list1, list1);
      verify_list(&list1);
  }
  while(list2 != NULL) {
      _dbus_list_remove_link(&list2, list2);
      verify_list(&list2);
  }
  i = 0;
  while(i < 10) {
      if (!_dbus_list_append(&list1, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for append");
      if (!_dbus_list_prepend(&list2, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for prepend");
      ++i;
  }
  --i;
  link2 = _dbus_list_get_first_link(&list2);
  while(link2 != NULL) {
      DBusList *next = _dbus_list_get_next_link(&list2, link2);
      _dbus_assert(_DBUS_POINTER_TO_INT(link2->data) == i);
      if ((i % 2) == 0) _dbus_list_remove_link(&list2, link2);
      verify_list(&list2);
      link2 = next;
      --i;
  }
  _dbus_assert(all_odd_values(&list2));
  _dbus_list_clear(&list2);
  i = 0;
  link1 = _dbus_list_get_first_link(&list1);
  while(link1 != NULL) {
      DBusList *next = _dbus_list_get_next_link(&list1, link1);
      _dbus_assert(_DBUS_POINTER_TO_INT(link1->data) == i);
      if ((i % 2) != 0) _dbus_list_remove_link(&list1, link1);
      verify_list(&list1);
      link1 = next;
      ++i;
  }
  _dbus_assert(all_even_values(&list1));
  _dbus_list_clear(&list1);
  i = 0;
  while(i < 10) {
      if (!_dbus_list_append(&list1, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for append");
      if (!_dbus_list_prepend(&list2, _DBUS_INT_TO_POINTER(i))) _dbus_test_fatal("could not allocate for prepend");
      ++i;
  }
  copy1 = _DBUS_INT_TO_POINTER(0x342234);
  copy2 = _DBUS_INT_TO_POINTER(23);
  _dbus_list_copy(&list1, &copy1);
  verify_list(&list1);
  verify_list(&copy1);
  _dbus_assert(lists_equal(&list1, &copy1));
  _dbus_list_copy(&list2, &copy2);
  verify_list(&list2);
  verify_list(&copy2);
  _dbus_assert(lists_equal(&list2, &copy2));
  _dbus_list_clear(&list1);
  _dbus_list_clear(&list2);
  _dbus_list_clear(&copy1);
  _dbus_list_clear(&copy2);
  copy1 = _DBUS_INT_TO_POINTER(0x342234);
  copy2 = _DBUS_INT_TO_POINTER(23);
  _dbus_list_copy(&list1, &copy1);
  verify_list(&list1);
  verify_list(&copy1);
  _dbus_assert(lists_equal(&list1, &copy1));
  _dbus_list_copy(&list2, &copy2);
  verify_list(&list2);
  verify_list(&copy2);
  _dbus_assert(lists_equal(&list2, &copy2));
  _dbus_list_clear(&list1);
  _dbus_list_clear(&list2);
  _dbus_list_insert_after(&list1, NULL,_DBUS_INT_TO_POINTER(0));
  verify_list(&list1);
  _dbus_list_insert_after(&list1, list1, _DBUS_INT_TO_POINTER(1));
  verify_list(&list1);
  _dbus_assert(is_ascending_sequence(&list1));
  _dbus_list_insert_after(&list1, list1->next, _DBUS_INT_TO_POINTER(2));
  verify_list(&list1);
  _dbus_assert(is_ascending_sequence(&list1));
  _dbus_list_insert_after(&list1, NULL, _DBUS_INT_TO_POINTER(-1));
  verify_list(&list1);
  _dbus_assert(is_ascending_sequence(&list1));
  _dbus_list_clear(&list1);
  if (!_dbus_list_append(&list1, _DBUS_INT_TO_POINTER(2))) _dbus_test_fatal("could not allocate for append");
  if (!_dbus_list_append(&list1, _DBUS_INT_TO_POINTER(1))) _dbus_test_fatal("could not allocate for append");
  if (!_dbus_list_append(&list1, _DBUS_INT_TO_POINTER(3))) _dbus_test_fatal("could not allocate for append");
  _dbus_list_remove_last(&list1, _DBUS_INT_TO_POINTER(2));
  verify_list(&list1);
  _dbus_assert(is_ascending_sequence(&list1));
  _dbus_list_clear(&list1);
  return TRUE;
}
#endif