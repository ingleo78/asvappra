#include "config.h"
#include "dbus-threads.h"
#include "dbus-internals.h"
#include "dbus-threads-internal.h"
#include "dbus-list.h"

static int thread_init_generation = 0;
void _dbus_rmutex_new_at_location(DBusRMutex **location_p) {
  _dbus_assert(location_p != NULL);
  if (!dbus_threads_init_default()) {
      *location_p = NULL;
      return;
  }
  *location_p = _dbus_platform_rmutex_new();
}
void _dbus_cmutex_new_at_location(DBusCMutex **location_p) {
  _dbus_assert(location_p != NULL);
  if (!dbus_threads_init_default()) {
      *location_p = NULL;
      return;
  }
  *location_p = _dbus_platform_cmutex_new();
}
void _dbus_rmutex_free_at_location(DBusRMutex **location_p) {
  if (location_p == NULL) return;
  if (*location_p != NULL) _dbus_platform_rmutex_free(*location_p);
}
void _dbus_cmutex_free_at_location(DBusCMutex **location_p) {
  if (location_p == NULL) return;
  if (*location_p != NULL) _dbus_platform_cmutex_free(*location_p);
}
void _dbus_rmutex_lock(DBusRMutex *mutex) {
  if (mutex == NULL) return;
  _dbus_platform_rmutex_lock(mutex);
}
void _dbus_cmutex_lock(DBusCMutex *mutex) {
  if (mutex == NULL) return;
  _dbus_platform_cmutex_lock(mutex);
}
void _dbus_rmutex_unlock(DBusRMutex *mutex) {
  if (mutex == NULL) return;
  _dbus_platform_rmutex_unlock(mutex);
}
void _dbus_cmutex_unlock(DBusCMutex *mutex) {
  if (mutex == NULL) return;
  _dbus_platform_cmutex_unlock(mutex);
}
DBusCondVar *_dbus_condvar_new(void) {
  if (!dbus_threads_init_default()) return NULL;
  return _dbus_platform_condvar_new();
}
void _dbus_condvar_new_at_location(DBusCondVar **location_p) {
  _dbus_assert(location_p != NULL);
  *location_p = _dbus_condvar_new();
}
void _dbus_condvar_free(DBusCondVar *cond) {
  if (cond == NULL) return;
  _dbus_platform_condvar_free(cond);
}
void _dbus_condvar_free_at_location(DBusCondVar **location_p) {
  if (location_p == NULL) return;
  if (*location_p != NULL) _dbus_platform_condvar_free(*location_p);
}
void _dbus_condvar_wait(DBusCondVar *cond, DBusCMutex *mutex) {
  if (cond == NULL || mutex == NULL) return;
  _dbus_platform_condvar_wait(cond, mutex);
}
dbus_bool_t _dbus_condvar_wait_timeout(DBusCondVar *cond, DBusCMutex *mutex, int timeout_milliseconds) {
  if (cond == NULL || mutex == NULL) return TRUE;
  return _dbus_platform_condvar_wait_timeout(cond, mutex, timeout_milliseconds);
}
void _dbus_condvar_wake_one(DBusCondVar *cond) {
  if (cond == NULL) return;
  _dbus_platform_condvar_wake_one(cond);
}
static DBusRMutex *global_locks[_DBUS_N_GLOBAL_LOCKS] = { NULL };
static void shutdown_global_locks(void *nil) {
  int i;
  for (i = 0; i < _DBUS_N_GLOBAL_LOCKS; i++) {
      _dbus_assert(global_locks[i] != NULL);
      _dbus_platform_rmutex_free(global_locks[i]);
      global_locks[i] = NULL;
  }
}
static dbus_bool_t init_global_locks(void) {
  int i;
  dbus_bool_t ok;
  for (i = 0; i < _DBUS_N_GLOBAL_LOCKS; i++) {
      _dbus_assert(global_locks[i] == NULL);
      global_locks[i] = _dbus_platform_rmutex_new();
      if (global_locks[i] == NULL) goto failed;
  }
  _dbus_platform_rmutex_lock(global_locks[_DBUS_LOCK_shutdown_funcs]);
  ok = _dbus_register_shutdown_func_unlocked(shutdown_global_locks, NULL);
  _dbus_platform_rmutex_unlock(global_locks[_DBUS_LOCK_shutdown_funcs]);
  if (!ok) goto failed;
  return TRUE;
failed:
  for (i = i - 1; i >= 0; i--) {
      _dbus_platform_rmutex_free(global_locks[i]);
      global_locks[i] = NULL;
  }
  return FALSE;
}
dbus_bool_t _dbus_lock(DBusGlobalLock lock) {
  _dbus_assert(lock >= 0);
  _dbus_assert(lock < _DBUS_N_GLOBAL_LOCKS);
  if (thread_init_generation != _dbus_current_generation && !dbus_threads_init_default()) return FALSE;
  _dbus_platform_rmutex_lock(global_locks[lock]);
  return TRUE;
}
void _dbus_unlock(DBusGlobalLock lock) {
  _dbus_assert(lock >= 0);
  _dbus_assert(lock < _DBUS_N_GLOBAL_LOCKS);
  _dbus_platform_rmutex_unlock(global_locks[lock]);
}
dbus_bool_t dbus_threads_init(const DBusThreadFunctions *functions) {
  _dbus_threads_lock_platform_specific();
  if (thread_init_generation == _dbus_current_generation) {
      _dbus_threads_unlock_platform_specific();
      return TRUE;
  }
  if (!_dbus_threads_init_platform_specific() || !init_global_locks()) {
      _dbus_threads_unlock_platform_specific();
      return FALSE;
  }
  thread_init_generation = _dbus_current_generation;
  _dbus_threads_unlock_platform_specific();
  return TRUE;
}
dbus_bool_t dbus_threads_init_default(void) {
  return dbus_threads_init(NULL);
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
dbus_bool_t _dbus_threads_init_debug(void) {
  return dbus_threads_init(NULL);
}
#endif