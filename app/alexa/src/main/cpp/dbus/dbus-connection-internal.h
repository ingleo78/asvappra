#ifndef DBUS_CONNECTION_INTERNAL_H
#define DBUS_CONNECTION_INTERNAL_H

#include "dbus-internals.h"
#include "dbus-connection.h"
#include "dbus-message.h"
#include "dbus-transport.h"
#include "dbus-resources.h"
#include "dbus-list.h"
#include "dbus-timeout.h"
#include "dbus-dataslot.h"

DBUS_BEGIN_DECLS
typedef enum {
  DBUS_ITERATION_DO_WRITING = 1 << 0,
  DBUS_ITERATION_DO_READING = 1 << 1,
  DBUS_ITERATION_BLOCK = 1 << 2
} DBusIterationFlags;
#define _DBUS_DEFAULT_TIMEOUT_VALUE  (25 * 1000)
typedef void (*DBusPendingFdsChangeFunction)(void *data);
DBUS_PRIVATE_EXPORT void _dbus_connection_lock(DBusConnection *connection);
DBUS_PRIVATE_EXPORT void _dbus_connection_unlock(DBusConnection *connection);
DBUS_PRIVATE_EXPORT DBusConnection *_dbus_connection_ref_unlocked(DBusConnection *connection);
DBUS_PRIVATE_EXPORT void _dbus_connection_unref_unlocked(DBusConnection *connection);
void _dbus_connection_queue_received_message_link (DBusConnection *connection, DBusList *link);
dbus_bool_t _dbus_connection_has_messages_to_send_unlocked (DBusConnection *connection);
DBusMessage* _dbus_connection_get_message_to_send(DBusConnection *connection);
void _dbus_connection_message_sent_unlocked(DBusConnection *connection, DBusMessage *message);
dbus_bool_t _dbus_connection_add_watch_unlocked(DBusConnection *connection, DBusWatch *watch);
void _dbus_connection_remove_watch_unlocked(DBusConnection *connection, DBusWatch *watch);
void _dbus_connection_toggle_watch_unlocked(DBusConnection *connection, DBusWatch *watch, dbus_bool_t enabled);
dbus_bool_t _dbus_connection_handle_watch(DBusWatch *watch, unsigned int condition, void *data);
dbus_bool_t _dbus_connection_add_timeout_unlocked(DBusConnection *connection, DBusTimeout *timeout);
void _dbus_connection_remove_timeout_unlocked(DBusConnection *connection, DBusTimeout *timeout);
void _dbus_connection_toggle_timeout_unlocked(DBusConnection *connection, DBusTimeout *timeout, dbus_bool_t enabled);
DBusConnection* _dbus_connection_new_for_transport(DBusTransport *transport);
void _dbus_connection_do_iteration_unlocked(DBusConnection *connection, DBusPendingCall *pending, unsigned int flags, int timeout_milliseconds);
void _dbus_connection_close_possibly_shared(DBusConnection *connection);
void _dbus_connection_close_if_only_one_ref(DBusConnection *connection);
DBusPendingCall* _dbus_pending_call_new(DBusConnection *connection, int timeout_milliseconds, DBusTimeoutHandler timeout_handler);
void _dbus_pending_call_notify(DBusPendingCall *pending);
void _dbus_connection_remove_pending_call(DBusConnection *connection, DBusPendingCall *pending);
void _dbus_connection_block_pending_call(DBusPendingCall *pending);
void _dbus_pending_call_complete_and_unlock(DBusPendingCall *pending, DBusMessage *message);
dbus_bool_t _dbus_connection_send_and_unlock(DBusConnection *connection, DBusMessage *message, dbus_uint32_t *client_serial);
void _dbus_connection_queue_synthesized_message_link(DBusConnection *connection, DBusList *link);
DBUS_PRIVATE_EXPORT void _dbus_connection_test_get_locks(DBusConnection *conn, DBusMutex **mutex_loc, DBusMutex **dispatch_mutex_loc, DBusMutex **io_path_mutex_loc,
                                                         DBusCondVar **dispatch_cond_loc, DBusCondVar **io_path_cond_loc);
DBUS_PRIVATE_EXPORT int _dbus_connection_get_pending_fds_count(DBusConnection *connection);
DBUS_PRIVATE_EXPORT void _dbus_connection_set_pending_fds_function(DBusConnection *connection, DBusPendingFdsChangeFunction callback, void *data);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_connection_get_linux_security_label(DBusConnection *connection, char **label_p);
DBUS_PRIVATE_EXPORT void _dbus_connection_get_stats(DBusConnection *connection, dbus_uint32_t *in_messages, dbus_uint32_t *in_bytes, dbus_uint32_t *in_fds,
													dbus_uint32_t *in_peak_bytes, dbus_uint32_t *in_peak_fds, dbus_uint32_t *out_messages, dbus_uint32_t *out_bytes,
													dbus_uint32_t *out_fds, dbus_uint32_t *out_peak_bytes, dbus_uint32_t *out_peak_fds);
const char* _dbus_connection_get_address(DBusConnection *connection);
void _dbus_bus_notify_shared_connection_disconnected_unlocked(DBusConnection *connection);
DBUS_END_DECLS

#endif