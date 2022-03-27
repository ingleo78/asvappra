#if defined (DBUS_INSIDE_DBUS_H) && !defined (DBUS_COMPILATION)
#error "Only <dbus/dbus.h> can be included directly, this file may disappear or change contents."
#endif

#ifndef DBUS_PENDING_CALL_H
#define DBUS_PENDING_CALL_H

#include "dbus-macros.h"
#include "dbus-types.h"
#include "dbus-connection.h"

DBUS_BEGIN_DECLS
#define DBUS_TIMEOUT_INFINITE  ((int)0x7fffffff)
#define DBUS_TIMEOUT_USE_DEFAULT  (-1)
DBUS_EXPORT DBusPendingCall* dbus_pending_call_ref(DBusPendingCall *pending);
DBUS_EXPORT void dbus_pending_call_unref(DBusPendingCall *pending);
DBUS_EXPORT dbus_bool_t dbus_pending_call_set_notify(DBusPendingCall *pending, DBusPendingCallNotifyFunction function, void *user_data, DBusFreeFunction free_user_data);
DBUS_EXPORT void dbus_pending_call_cancel(DBusPendingCall *pending);
DBUS_EXPORT dbus_bool_t dbus_pending_call_get_completed(DBusPendingCall *pending);
DBUS_EXPORT DBusMessage* dbus_pending_call_steal_reply(DBusPendingCall *pending);
DBUS_EXPORT void dbus_pending_call_block(DBusPendingCall *pending);
DBUS_EXPORT dbus_bool_t dbus_pending_call_allocate_data_slot(dbus_int32_t *slot_p);
DBUS_EXPORT void dbus_pending_call_free_data_slot(dbus_int32_t *slot_p);
DBUS_EXPORT dbus_bool_t dbus_pending_call_set_data(DBusPendingCall *pending, dbus_int32_t slot, void *data, DBusFreeFunction free_data_func);
DBUS_EXPORT void* dbus_pending_call_get_data(DBusPendingCall *pending, dbus_int32_t slot);
static inline void dbus_clear_pending_call(DBusPendingCall **pointer_to_pending_call) {
  _dbus_clear_pointer_impl(DBusPendingCall, pointer_to_pending_call, dbus_pending_call_unref);
}
DBUS_END_DECLS

#endif