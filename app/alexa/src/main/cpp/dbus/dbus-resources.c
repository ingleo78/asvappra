#include "config.h"
#include "dbus-resources.h"
#include "dbus-internals.h"

struct DBusCounter {
  int refcount;
  long size_value;
  long unix_fd_value;
#ifdef DBUS_ENABLE_STATS
  long peak_size_value;
  long peak_unix_fd_value;
#endif
  long notify_size_guard_value;
  long notify_unix_fd_guard_value;
  DBusCounterNotifyFunction notify_function;
  void *notify_data;
  dbus_bool_t notify_pending : 1;
  DBusRMutex *mutex;
};
DBusCounter* _dbus_counter_new(void) {
  DBusCounter *counter;
  counter = dbus_new0(DBusCounter, 1);
  if (counter == NULL) return NULL;
  counter->refcount = 1;
  _dbus_rmutex_new_at_location(&counter->mutex);
  if (counter->mutex == NULL) {
      dbus_free(counter);
      counter = NULL;
  }
  return counter;
}
DBusCounter *_dbus_counter_ref(DBusCounter *counter) {
  _dbus_rmutex_lock(counter->mutex);
  _dbus_assert(counter->refcount > 0);
  counter->refcount += 1;
  _dbus_rmutex_unlock(counter->mutex);
  return counter;
}
void _dbus_counter_unref(DBusCounter *counter) {
  dbus_bool_t last_ref = FALSE;
  _dbus_rmutex_lock(counter->mutex);
  _dbus_assert(counter->refcount > 0);
  counter->refcount -= 1;
  last_ref = (counter->refcount == 0);
  _dbus_rmutex_unlock(counter->mutex);
  if (last_ref) {
      _dbus_rmutex_free_at_location(&counter->mutex);
      dbus_free(counter);
  }
}
void _dbus_counter_adjust_size(DBusCounter *counter, long delta) {
  long old = 0;
  _dbus_rmutex_lock(counter->mutex);
  old = counter->size_value;
  counter->size_value += delta;
#ifdef DBUS_ENABLE_STATS
  if (counter->peak_size_value < counter->size_value) counter->peak_size_value = counter->size_value;
#endif
#if 0
  _dbus_verbose("Adjusting counter %ld by %ld = %ld\n", old, delta, counter->size_value);
#endif
  if (counter->notify_function != NULL && ((old < counter->notify_size_guard_value && counter->size_value >= counter->notify_size_guard_value) ||
      (old >= counter->notify_size_guard_value && counter->size_value < counter->notify_size_guard_value)))
      counter->notify_pending = TRUE;
  _dbus_rmutex_unlock(counter->mutex);
}
void _dbus_counter_notify(DBusCounter *counter) {
  DBusCounterNotifyFunction notify_function = NULL;
  void *notify_data = NULL;
  _dbus_rmutex_lock(counter->mutex);
  if (counter->notify_pending) {
      counter->notify_pending = FALSE;
      notify_function = counter->notify_function;
      notify_data = counter->notify_data;
  }
  _dbus_rmutex_unlock(counter->mutex);
  if (notify_function != NULL) (*notify_function)(counter, notify_data);
}
void _dbus_counter_adjust_unix_fd(DBusCounter *counter, long delta) {
  long old = 0;
  _dbus_rmutex_lock(counter->mutex);
  old = counter->unix_fd_value;
  counter->unix_fd_value += delta;
#ifdef DBUS_ENABLE_STATS
  if (counter->peak_unix_fd_value < counter->unix_fd_value) counter->peak_unix_fd_value = counter->unix_fd_value;
#endif
#if 0
  _dbus_verbose("Adjusting counter %ld by %ld = %ld\n", old, delta, counter->unix_fd_value);
#endif
  if (counter->notify_function != NULL && ((old < counter->notify_unix_fd_guard_value && counter->unix_fd_value >= counter->notify_unix_fd_guard_value) ||
      (old >= counter->notify_unix_fd_guard_value && counter->unix_fd_value < counter->notify_unix_fd_guard_value)))
      counter->notify_pending = TRUE;
  _dbus_rmutex_unlock(counter->mutex);
}
long _dbus_counter_get_size_value(DBusCounter *counter) {
  return counter->size_value;
}
long _dbus_counter_get_unix_fd_value(DBusCounter *counter) {
  return counter->unix_fd_value;
}
void _dbus_counter_set_notify(DBusCounter *counter, long size_guard_value, long unix_fd_guard_value, DBusCounterNotifyFunction function, void *user_data) {
  _dbus_rmutex_lock(counter->mutex);
  counter->notify_size_guard_value = size_guard_value;
  counter->notify_unix_fd_guard_value = unix_fd_guard_value;
  counter->notify_function = function;
  counter->notify_data = user_data;
  counter->notify_pending = FALSE;
  _dbus_rmutex_unlock(counter->mutex);
}
#ifdef DBUS_ENABLE_STATS
long _dbus_counter_get_peak_size_value(DBusCounter *counter) {
  return counter->peak_size_value;
}
long _dbus_counter_get_peak_unix_fd_value(DBusCounter *counter) {
  return counter->peak_unix_fd_value;
}
#endif