#include "config.h"
#include "dbus-internals.h"
#include "dbus-timeout.h"
#include "dbus-list.h"

struct DBusTimeout {
  int refcount;
  int interval;
  DBusTimeoutHandler handler;
  void *handler_data;
  DBusFreeFunction free_handler_data_function;
  void *data;
  DBusFreeFunction free_data_function;
  unsigned int enabled : 1;
  unsigned int needs_restart : 1;
};
DBusTimeout* _dbus_timeout_new(int interval, DBusTimeoutHandler handler, void *data, DBusFreeFunction free_data_function) {
  DBusTimeout *timeout;
  timeout = dbus_new0(DBusTimeout, 1);
  if (timeout == NULL) return NULL;
  timeout->refcount = 1;
  timeout->interval = interval;
  timeout->handler = handler;
  timeout->handler_data = data;
  timeout->free_handler_data_function = free_data_function;
  timeout->enabled = TRUE;
  timeout->needs_restart = FALSE;
  return timeout;
}
DBusTimeout *_dbus_timeout_ref(DBusTimeout *timeout) {
  timeout->refcount += 1;
  return timeout;
}
void _dbus_timeout_unref(DBusTimeout *timeout) {
  _dbus_assert(timeout != NULL);
  _dbus_assert(timeout->refcount > 0);
  timeout->refcount -= 1;
  if (timeout->refcount == 0) {
      dbus_timeout_set_data(timeout, NULL, NULL);
      if (timeout->free_handler_data_function) (*timeout->free_handler_data_function)(timeout->handler_data);
      dbus_free(timeout);
  }
}
void _dbus_timeout_restart(DBusTimeout *timeout, int interval) {
  _dbus_assert(interval >= 0);
  timeout->interval = interval;
  timeout->enabled = TRUE;
  timeout->needs_restart = TRUE;
}
void _dbus_timeout_disable(DBusTimeout  *timeout) {
  timeout->enabled = FALSE;
}
struct DBusTimeoutList {
  DBusList *timeouts;
  DBusAddTimeoutFunction add_timeout_function;
  DBusRemoveTimeoutFunction remove_timeout_function;
  DBusTimeoutToggledFunction timeout_toggled_function;
  void *timeout_data;
  DBusFreeFunction timeout_free_data_function;
};
DBusTimeoutList* _dbus_timeout_list_new(void) {
  DBusTimeoutList *timeout_list;
  timeout_list = dbus_new0(DBusTimeoutList, 1);
  if (timeout_list == NULL) return NULL;
  return timeout_list;
}
void _dbus_timeout_list_free(DBusTimeoutList *timeout_list) {
  _dbus_timeout_list_set_functions(timeout_list,NULL, NULL, NULL, NULL, NULL);
  _dbus_list_foreach(&timeout_list->timeouts, (DBusForeachFunction)_dbus_timeout_unref,NULL);
  _dbus_list_clear(&timeout_list->timeouts);
  dbus_free(timeout_list);
}
dbus_bool_t _dbus_timeout_list_set_functions(DBusTimeoutList *timeout_list, DBusAddTimeoutFunction add_function, DBusRemoveTimeoutFunction remove_function,
                                             DBusTimeoutToggledFunction toggled_function, void *data, DBusFreeFunction free_data_function) {
  if (add_function != NULL) {
      DBusList *link;
      link = _dbus_list_get_first_link(&timeout_list->timeouts);
      while(link != NULL) {
          DBusList *next = _dbus_list_get_next_link(&timeout_list->timeouts, link);
          if (!(*add_function)(link->data, data)) {
              DBusList *link2;
              link2 = _dbus_list_get_first_link(&timeout_list->timeouts);
              while(link2 != link) {
                  DBusList *next2 = _dbus_list_get_next_link(&timeout_list->timeouts, link2);
                  (*remove_function)(link2->data, data);
                  link2 = next2;
              }
              return FALSE;
          }
          link = next;
      }
  }
  if (timeout_list->remove_timeout_function != NULL) {
      _dbus_list_foreach(&timeout_list->timeouts, (DBusForeachFunction)timeout_list->remove_timeout_function, timeout_list->timeout_data);
  }
  if (timeout_list->timeout_free_data_function != NULL) (*timeout_list->timeout_free_data_function)(timeout_list->timeout_data);
  timeout_list->add_timeout_function = add_function;
  timeout_list->remove_timeout_function = remove_function;
  timeout_list->timeout_toggled_function = toggled_function;
  timeout_list->timeout_data = data;
  timeout_list->timeout_free_data_function = free_data_function;
  return TRUE;
}
dbus_bool_t _dbus_timeout_list_add_timeout(DBusTimeoutList *timeout_list, DBusTimeout *timeout) {
  if (!_dbus_list_append(&timeout_list->timeouts, timeout)) return FALSE;
  _dbus_timeout_ref(timeout);
  if (timeout_list->add_timeout_function != NULL) {
      if (!(*timeout_list->add_timeout_function)(timeout, timeout_list->timeout_data)) {
          _dbus_list_remove_last(&timeout_list->timeouts, timeout);
          _dbus_timeout_unref(timeout);
          return FALSE;
      }
  }
  return TRUE;
}
void _dbus_timeout_list_remove_timeout(DBusTimeoutList *timeout_list, DBusTimeout *timeout) {
  if (!_dbus_list_remove(&timeout_list->timeouts, timeout)) _dbus_assert_not_reached("Nonexistent timeout was removed");
  if (timeout_list->remove_timeout_function != NULL) (*timeout_list->remove_timeout_function)(timeout, timeout_list->timeout_data);
  _dbus_timeout_unref(timeout);
}
void _dbus_timeout_list_toggle_timeout(DBusTimeoutList *timeout_list, DBusTimeout *timeout, dbus_bool_t enabled) {
  enabled = !!enabled;
  if (enabled == timeout->enabled) return;
  timeout->enabled = enabled;
  if (timeout_list->timeout_toggled_function != NULL) (*timeout_list->timeout_toggled_function)(timeout, timeout_list->timeout_data);
}
dbus_bool_t _dbus_timeout_needs_restart(DBusTimeout *timeout) {
  return timeout->needs_restart;
}
void _dbus_timeout_restarted(DBusTimeout *timeout) {
  timeout->needs_restart = FALSE;
}
int dbus_timeout_get_interval(DBusTimeout *timeout) {
  return timeout->interval;
}
void* dbus_timeout_get_data(DBusTimeout *timeout) {
  return timeout->data;
}
void dbus_timeout_set_data(DBusTimeout *timeout, void *data, DBusFreeFunction free_data_function) {
  if (timeout->free_data_function != NULL) (*timeout->free_data_function)(timeout->data);
  timeout->data = data;
  timeout->free_data_function = free_data_function;
}
dbus_bool_t dbus_timeout_handle(DBusTimeout *timeout) {
  return (*timeout->handler)(timeout->handler_data);
}
dbus_bool_t dbus_timeout_get_enabled(DBusTimeout *timeout) {
  return timeout->enabled;
}