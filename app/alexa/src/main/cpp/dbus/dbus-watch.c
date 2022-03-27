#include "config.h"
#include "dbus-internals.h"
#include "dbus-watch.h"
#include "dbus-list.h"

struct DBusWatch {
  int refcount;
  DBusPollable fd;
  unsigned int flags;
  DBusWatchHandler handler;
  void *handler_data;
  DBusFreeFunction free_handler_data_function;
  void *data;
  DBusFreeFunction free_data_function;
  unsigned int enabled : 1;
  unsigned int oom_last_time : 1;
};
dbus_bool_t _dbus_watch_get_enabled(DBusWatch *watch) {
  return watch->enabled;
}
dbus_bool_t _dbus_watch_get_oom_last_time(DBusWatch *watch) {
  return watch->oom_last_time;
}
void _dbus_watch_set_oom_last_time(DBusWatch *watch, dbus_bool_t oom) {
  watch->oom_last_time = oom;
}
DBusWatch* _dbus_watch_new(DBusPollable fd, unsigned int flags, dbus_bool_t enabled, DBusWatchHandler handler, void *data, DBusFreeFunction free_data_function) {
  DBusWatch *watch;
#define VALID_WATCH_FLAGS  (DBUS_WATCH_WRITABLE | DBUS_WATCH_READABLE)
  _dbus_assert((flags & VALID_WATCH_FLAGS) == flags);
  watch = dbus_new0(DBusWatch, 1);
  if (watch == NULL) return NULL;
  watch->refcount = 1;
  watch->fd = fd;
  watch->flags = flags;
  watch->enabled = enabled;
  watch->handler = handler;
  watch->handler_data = data;
  watch->free_handler_data_function = free_data_function;
  return watch;
}
DBusWatch *_dbus_watch_ref(DBusWatch *watch) {
  watch->refcount += 1;
  return watch;
}
void _dbus_watch_unref(DBusWatch *watch) {
  _dbus_assert(watch != NULL);
  _dbus_assert(watch->refcount > 0);
  watch->refcount -= 1;
  if (watch->refcount == 0) {
      if (_dbus_pollable_is_valid(watch->fd)) _dbus_warn("this watch should have been invalidated");
      dbus_watch_set_data(watch, NULL, NULL);
      if (watch->free_handler_data_function) (*watch->free_handler_data_function)(watch->handler_data);
      dbus_free(watch);
  }
}
void _dbus_watch_invalidate(DBusWatch *watch) {
  _dbus_pollable_invalidate (&watch->fd);
  watch->flags = 0;
}
void _dbus_watch_sanitize_condition(DBusWatch *watch, unsigned int *condition) {
  if (!(watch->flags & DBUS_WATCH_READABLE)) *condition &= ~DBUS_WATCH_READABLE;
  if (!(watch->flags & DBUS_WATCH_WRITABLE)) *condition &= ~DBUS_WATCH_WRITABLE;
}
struct DBusWatchList {
  DBusList *watches;
  DBusAddWatchFunction add_watch_function;
  DBusRemoveWatchFunction remove_watch_function;
  DBusWatchToggledFunction watch_toggled_function;
  void *watch_data;
  DBusFreeFunction watch_free_data_function;
};
DBusWatchList* _dbus_watch_list_new(void) {
  DBusWatchList *watch_list;
  watch_list = dbus_new0 (DBusWatchList, 1);
  if (watch_list == NULL) return NULL;
  return watch_list;
}
void _dbus_watch_list_free(DBusWatchList *watch_list) {
  _dbus_watch_list_set_functions (watch_list,NULL, NULL, NULL, NULL, NULL);
  _dbus_list_foreach(&watch_list->watches, (DBusForeachFunction)_dbus_watch_unref,NULL);
  _dbus_list_clear(&watch_list->watches);
  dbus_free(watch_list);
}
#ifdef DBUS_ENABLE_VERBOSE_MODE
static const char* watch_flags_to_string(int flags) {
  const char *watch_type;
  if ((flags & DBUS_WATCH_READABLE) && (flags & DBUS_WATCH_WRITABLE)) watch_type = "readwrite";
  else if (flags & DBUS_WATCH_READABLE) watch_type = "read";
  else if (flags & DBUS_WATCH_WRITABLE) watch_type = "write";
  else watch_type = "not read or write";
  return watch_type;
}
#endif
dbus_bool_t _dbus_watch_list_set_functions(DBusWatchList *watch_list, DBusAddWatchFunction add_function, DBusRemoveWatchFunction remove_function,
                                           DBusWatchToggledFunction toggled_function, void *data, DBusFreeFunction free_data_function) {
  if (add_function != NULL) {
      DBusList *link;
      link = _dbus_list_get_first_link(&watch_list->watches);
      while(link != NULL) {
          DBusList *next = _dbus_list_get_next_link(&watch_list->watches, link);
      #ifdef DBUS_ENABLE_VERBOSE_MODE
          DBusWatch *watch = link->data;
          _dbus_verbose("Adding a %s watch on fd %" DBUS_POLLABLE_FORMAT " using newly-set add watch function\n",
                        watch_flags_to_string(dbus_watch_get_flags (link->data)), _dbus_pollable_printable(watch->fd));
      #endif
          if (!(*add_function)(link->data, data)) {
              DBusList *link2;
              link2 = _dbus_list_get_first_link(&watch_list->watches);
              while(link2 != link) {
                  DBusList *next2 = _dbus_list_get_next_link(&watch_list->watches, link2);
              #ifdef DBUS_ENABLE_VERBOSE_MODE
                  DBusWatch *watch2 = link2->data;
                  _dbus_verbose("Removing watch on fd %" DBUS_POLLABLE_FORMAT " using newly-set remove function because initial add failed\n",
                                _dbus_pollable_printable(watch2->fd));
              #endif
                  (*remove_function)(link2->data, data);
                  link2 = next2;
              }
              return FALSE;
          }
          link = next;
      }
  }
  if (watch_list->remove_watch_function != NULL) {
      _dbus_verbose("Removing all pre-existing watches\n");
      _dbus_list_foreach(&watch_list->watches, (DBusForeachFunction)watch_list->remove_watch_function, watch_list->watch_data);
  }
  if (watch_list->watch_free_data_function != NULL) (*watch_list->watch_free_data_function)(watch_list->watch_data);
  watch_list->add_watch_function = add_function;
  watch_list->remove_watch_function = remove_function;
  watch_list->watch_toggled_function = toggled_function;
  watch_list->watch_data = data;
  watch_list->watch_free_data_function = free_data_function;
  return TRUE;
}
dbus_bool_t _dbus_watch_list_add_watch(DBusWatchList *watch_list, DBusWatch *watch) {
  if (!_dbus_list_append(&watch_list->watches, watch)) return FALSE;
  _dbus_watch_ref(watch);
  if (watch_list->add_watch_function != NULL) {
      _dbus_verbose("Adding watch on fd %" DBUS_POLLABLE_FORMAT "\n", _dbus_pollable_printable(watch->fd));
      if (!(*watch_list->add_watch_function)(watch, watch_list->watch_data)) {
          _dbus_list_remove_last(&watch_list->watches, watch);
          _dbus_watch_unref(watch);
          return FALSE;
      }
  }
  return TRUE;
}
void _dbus_watch_list_remove_watch(DBusWatchList *watch_list, DBusWatch *watch) {
  if (!_dbus_list_remove(&watch_list->watches, watch)) _dbus_assert_not_reached("Nonexistent watch was removed");
  if (watch_list->remove_watch_function != NULL) {
      _dbus_verbose("Removing watch on fd %" DBUS_POLLABLE_FORMAT "\n", _dbus_pollable_printable(watch->fd));
      (*watch_list->remove_watch_function)(watch, watch_list->watch_data);
  }
  _dbus_watch_unref(watch);
}
void _dbus_watch_list_toggle_watch(DBusWatchList *watch_list, DBusWatch *watch, dbus_bool_t enabled) {
  enabled = !!enabled;
  if (enabled == watch->enabled) return;
  watch->enabled = enabled;
  if (watch_list->watch_toggled_function != NULL) {
      _dbus_verbose("Toggling watch %p on fd %" DBUS_POLLABLE_FORMAT " to %d\n", watch, _dbus_pollable_printable(watch->fd), watch->enabled);
      (*watch_list->watch_toggled_function)(watch, watch_list->watch_data);
  }
}
void _dbus_watch_list_toggle_all_watches(DBusWatchList *watch_list, dbus_bool_t enabled) {
  DBusList *link;
  for (link = _dbus_list_get_first_link(&watch_list->watches); link != NULL; link = _dbus_list_get_next_link(&watch_list->watches, link)) {
      _dbus_watch_list_toggle_watch(watch_list, link->data, enabled);
  }
}
void _dbus_watch_set_handler(DBusWatch *watch, DBusWatchHandler handler, void *data, DBusFreeFunction free_data_function) {
  if (watch->free_handler_data_function) (*watch->free_handler_data_function)(watch->handler_data);
  watch->handler = handler;
  watch->handler_data = data;
  watch->free_handler_data_function = free_data_function;
}
int dbus_watch_get_fd(DBusWatch *watch) {
  _dbus_return_val_if_fail(watch != NULL, -1);
  return dbus_watch_get_unix_fd(watch);
}
int dbus_watch_get_unix_fd(DBusWatch *watch) {
  _dbus_return_val_if_fail(watch != NULL, -1);
#ifdef DBUS_UNIX
  return watch->fd;
#else
  return dbus_watch_get_socket(watch);
#endif
}
int dbus_watch_get_socket(DBusWatch *watch) {
  _dbus_return_val_if_fail(watch != NULL, -1);
#ifdef DBUS_UNIX
  return watch->fd;
#else
  return _dbus_socket_get_int(watch->fd);
#endif
}
DBusSocket _dbus_watch_get_socket(DBusWatch *watch) {
  DBusSocket s;
  _dbus_assert(watch != NULL);
#ifdef DBUS_UNIX
  s.fd = watch->fd;
#else
  s = watch->fd;
#endif
  return s;
}
DBusPollable _dbus_watch_get_pollable(DBusWatch *watch) {
  _dbus_assert(watch != NULL);
  return watch->fd;
}
unsigned int dbus_watch_get_flags(DBusWatch *watch) {
  _dbus_return_val_if_fail(watch != NULL, 0);
  _dbus_assert((watch->flags & VALID_WATCH_FLAGS) == watch->flags);
  return watch->flags;
}
void* dbus_watch_get_data(DBusWatch *watch) {
  _dbus_return_val_if_fail(watch != NULL, NULL);
  return watch->data;
}
void dbus_watch_set_data(DBusWatch *watch, void *data, DBusFreeFunction free_data_function) {
  _dbus_return_if_fail(watch != NULL);
  _dbus_verbose("Setting watch fd %" DBUS_POLLABLE_FORMAT " data to data = %p function = %p from data = %p function = %p\n", _dbus_pollable_printable(watch->fd),
                data, free_data_function, watch->data, watch->free_data_function);
  if (watch->free_data_function != NULL) (*watch->free_data_function)(watch->data);
  watch->data = data;
  watch->free_data_function = free_data_function;
}
dbus_bool_t dbus_watch_get_enabled(DBusWatch *watch) {
  _dbus_return_val_if_fail(watch != NULL, FALSE);
  return watch->enabled;
}
dbus_bool_t dbus_watch_handle(DBusWatch *watch, unsigned int flags) {
  _dbus_return_val_if_fail(watch != NULL, FALSE);
#ifndef DBUS_DISABLE_CHECKS
  if (!_dbus_pollable_is_valid(watch->fd) || watch->flags == 0) {
      _dbus_warn_check_failed("Watch is invalid, it should have been removed");
      return TRUE;
  }
#endif
  _dbus_return_val_if_fail(_dbus_pollable_is_valid(watch->fd), TRUE);
  _dbus_watch_sanitize_condition(watch, &flags);
  if (flags == 0) {
      _dbus_verbose("After sanitization, watch flags on fd %" DBUS_POLLABLE_FORMAT " were 0\n", _dbus_pollable_printable(watch->fd));
      return TRUE;
  } else return (*watch->handler)(watch, flags, watch->handler_data);
}