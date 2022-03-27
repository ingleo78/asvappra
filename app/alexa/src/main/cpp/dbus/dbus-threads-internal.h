#ifndef DBUS_THREADS_INTERNAL_H
#define DBUS_THREADS_INTERNAL_H

#include "dbus-macros.h"
#include "dbus-types.h"
#include "dbus-threads.h"

typedef struct DBusRMutex DBusRMutex;
typedef struct DBusCMutex DBusCMutex;
DBUS_BEGIN_DECLS
DBUS_PRIVATE_EXPORT void _dbus_rmutex_lock(DBusRMutex *mutex);
DBUS_PRIVATE_EXPORT void _dbus_rmutex_unlock(DBusRMutex *mutex);
void _dbus_rmutex_new_at_location(DBusRMutex **location_p);
void _dbus_rmutex_free_at_location(DBusRMutex **location_p);
void _dbus_cmutex_lock(DBusCMutex *mutex);
void _dbus_cmutex_unlock(DBusCMutex *mutex);
void _dbus_cmutex_new_at_location(DBusCMutex **location_p);
void _dbus_cmutex_free_at_location(DBusCMutex **location_p);
DBusCondVar* _dbus_condvar_new(void);
void _dbus_condvar_free(DBusCondVar *cond);
void _dbus_condvar_wait(DBusCondVar *cond, DBusCMutex *mutex);
dbus_bool_t _dbus_condvar_wait_timeout(DBusCondVar *cond, DBusCMutex *mutex, int timeout_milliseconds);
void _dbus_condvar_wake_one(DBusCondVar *cond);
void _dbus_condvar_new_at_location(DBusCondVar **location_p);
void _dbus_condvar_free_at_location(DBusCondVar **location_p);
DBusRMutex *_dbus_platform_rmutex_new(void);
void _dbus_platform_rmutex_free(DBusRMutex *mutex);
void _dbus_platform_rmutex_lock(DBusRMutex *mutex);
void _dbus_platform_rmutex_unlock(DBusRMutex *mutex);
DBusCMutex *_dbus_platform_cmutex_new(void);
void _dbus_platform_cmutex_free(DBusCMutex *mutex);
void _dbus_platform_cmutex_lock(DBusCMutex *mutex);
void _dbus_platform_cmutex_unlock(DBusCMutex *mutex);
DBusCondVar* _dbus_platform_condvar_new(void);
void _dbus_platform_condvar_free(DBusCondVar *cond);
void _dbus_platform_condvar_wait(DBusCondVar *cond, DBusCMutex *mutex);
dbus_bool_t _dbus_platform_condvar_wait_timeout(DBusCondVar *cond, DBusCMutex *mutex, int timeout_milliseconds);
void _dbus_platform_condvar_wake_one(DBusCondVar *cond);
DBUS_END_DECLS

#endif
