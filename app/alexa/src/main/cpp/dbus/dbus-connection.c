#include "config.h"
#include "dbus-shared.h"
#include "dbus-connection.h"
#include "dbus-list.h"
#include "dbus-timeout.h"
#include "dbus-transport.h"
#include "dbus-watch.h"
#include "dbus-connection-internal.h"
#include "dbus-pending-call-internal.h"
#include "dbus-list.h"
#include "dbus-hash.h"
#include "dbus-message-internal.h"
#include "dbus-message-private.h"
#include "dbus-threads.h"
#include "dbus-protocol.h"
#include "dbus-dataslot.h"
#include "dbus-string.h"
#include "dbus-signature.h"
#include "dbus-pending-call.h"
#include "dbus-object-tree.h"
#include "dbus-threads-internal.h"
#include "dbus-bus.h"
#include "dbus-marshal-basic.h"

#ifdef DBUS_DISABLE_CHECKS
#define TOOK_LOCK_CHECK(connection)
#define RELEASING_LOCK_CHECK(connection)
#define HAVE_LOCK_CHECK(connection)
#else
#define TOOK_LOCK_CHECK(connection) \
do {                \
    _dbus_assert(!(connection)->have_connection_lock); \
    (connection)->have_connection_lock = TRUE;          \
} while(0);
#define RELEASING_LOCK_CHECK(connection) \
do {            \
    _dbus_assert((connection)->have_connection_lock);   \
    (connection)->have_connection_lock = FALSE;          \
} while(0);
#define HAVE_LOCK_CHECK(connection)  _dbus_assert((connection)->have_connection_lock)
#endif
#define TRACE_LOCKS 1
#define CONNECTION_LOCK(connection) \
do {                                      \
    if (TRACE_LOCKS) { _dbus_verbose("LOCK\n"); }   \
    _dbus_rmutex_lock((connection)->mutex);                                    \
    TOOK_LOCK_CHECK(connection);                                               \
} while(0);
#define CONNECTION_UNLOCK(connection)  _dbus_connection_unlock(connection)
#define SLOTS_LOCK(connection) \
do {                     \
    _dbus_rmutex_lock((connection)->slot_mutex);       \
} while(0);
#define SLOTS_UNLOCK(connection) \
do {                   \
    _dbus_rmutex_unlock((connection)->slot_mutex);     \
} while(0);
#define DISPATCH_STATUS_NAME(s)   ((s) == DBUS_DISPATCH_COMPLETE ? "complete" : (s) == DBUS_DISPATCH_DATA_REMAINS ? "data remains" : \
                                   (s) == DBUS_DISPATCH_NEED_MEMORY ? "need memory" : "???")
static void _dbus_connection_trace_ref(DBusConnection *connection, int old_refcount, int new_refcount, const char *why) {
#ifdef DBUS_ENABLE_VERBOSE_MODE
  static int enabled = -1;
  _dbus_trace_ref("DBusConnection", connection, old_refcount, new_refcount, why, "DBUS_CONNECTION_TRACE", &enabled);
#endif
}
typedef struct DBusMessageFilter DBusMessageFilter;
struct DBusMessageFilter {
  DBusAtomic refcount;
  DBusHandleMessageFunction function;
  void *user_data;
  DBusFreeFunction free_user_data_function;
};
struct DBusPreallocatedSend {
  DBusConnection *connection;
  DBusList *queue_link;
  DBusList *counter_link;
};
#if HAVE_DECL_MSG_NOSIGNAL
static dbus_bool_t _dbus_modify_sigpipe = FALSE;
#else
static dbus_bool_t _dbus_modify_sigpipe = TRUE;
#endif
struct DBusConnection {
  DBusAtomic refcount;
  DBusRMutex *mutex;
  DBusCMutex *dispatch_mutex;
  DBusCondVar *dispatch_cond;
  DBusCMutex *io_path_mutex;
  DBusCondVar *io_path_cond;
  DBusList *outgoing_messages;
  DBusList *incoming_messages;
  DBusList *expired_messages;
  DBusMessage *message_borrowed;
  int n_outgoing;
  int n_incoming;
  DBusCounter *outgoing_counter;
  DBusTransport *transport;
  DBusWatchList *watches;
  DBusTimeoutList *timeouts;
  DBusList *filter_list;
  DBusRMutex *slot_mutex;
  DBusDataSlotList slot_list;
  DBusHashTable *pending_replies;
  dbus_uint32_t client_serial;
  DBusList *disconnect_message_link;
  DBusWakeupMainFunction wakeup_main_function;
  void *wakeup_main_data;
  DBusFreeFunction free_wakeup_main_data;
  DBusDispatchStatusFunction dispatch_status_function;
  void *dispatch_status_data;
  DBusFreeFunction free_dispatch_status_data;
  DBusDispatchStatus last_dispatch_status;
  DBusObjectTree *objects;
  char *server_guid;
  dbus_bool_t dispatch_acquired;
  dbus_bool_t io_path_acquired;
  unsigned int shareable : 1;
  unsigned int exit_on_disconnect : 1;
  unsigned int route_peer_messages : 1;
  unsigned int disconnected_message_arrived : 1;
  unsigned int disconnected_message_processed : 1;
#ifndef DBUS_DISABLE_CHECKS
  unsigned int have_connection_lock : 1;
#endif
#if defined(DBUS_ENABLE_CHECKS) || defined(DBUS_ENABLE_ASSERT)
  int generation;
#endif
};
static DBusDispatchStatus _dbus_connection_get_dispatch_status_unlocked(DBusConnection *connection);
static void _dbus_connection_update_dispatch_status_and_unlock(DBusConnection *connection, DBusDispatchStatus new_status);
static void _dbus_connection_last_unref(DBusConnection *connection);
static void _dbus_connection_acquire_dispatch(DBusConnection *connection);
static void _dbus_connection_release_dispatch(DBusConnection *connection);
static DBusDispatchStatus _dbus_connection_flush_unlocked(DBusConnection *connection);
static void _dbus_connection_close_possibly_shared_and_unlock(DBusConnection *connection);
static dbus_bool_t _dbus_connection_get_is_connected_unlocked(DBusConnection *connection);
static dbus_bool_t _dbus_connection_peek_for_reply_unlocked(DBusConnection *connection, dbus_uint32_t client_serial);
static DBusMessageFilter *_dbus_message_filter_ref(DBusMessageFilter *filter) {
#ifdef DBUS_DISABLE_ASSERT
  _dbus_atomic_inc(&filter->refcount);
#else
  dbus_int32_t old_value;
  old_value = _dbus_atomic_inc(&filter->refcount);
  _dbus_assert(old_value > 0);
#endif
  return filter;
}
static void _dbus_message_filter_unref(DBusMessageFilter *filter) {
  dbus_int32_t old_value;
  old_value = _dbus_atomic_dec(&filter->refcount);
  _dbus_assert(old_value > 0);
  if (old_value == 1) {
      if (filter->free_user_data_function) (*filter->free_user_data_function)(filter->user_data);
      dbus_free(filter);
  }
}
void _dbus_connection_lock(DBusConnection *connection) {
  CONNECTION_LOCK(connection);
}
void _dbus_connection_unlock(DBusConnection *connection) {
  DBusList *expired_messages;
  DBusList *iter;
  if (TRACE_LOCKS) { _dbus_verbose("UNLOCK\n"); }
  expired_messages = connection->expired_messages;
  connection->expired_messages = NULL;
  RELEASING_LOCK_CHECK(connection);
  _dbus_rmutex_unlock(connection->mutex);
  for (iter = _dbus_list_pop_first_link(&expired_messages); iter != NULL; iter = _dbus_list_pop_first_link(&expired_messages)) {
      DBusMessage *message = iter->data;
      dbus_message_unref(message);
      _dbus_list_free_link(iter);
  }
}
static void _dbus_connection_wakeup_mainloop(DBusConnection *connection) {
  if (connection->wakeup_main_function)(*connection->wakeup_main_function)(connection->wakeup_main_data);
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
void _dbus_connection_test_get_locks(DBusConnection *connection, DBusMutex **mutex_loc, DBusMutex **dispatch_mutex_loc, DBusMutex **io_path_mutex_loc,
                                     DBusCondVar **dispatch_cond_loc, DBusCondVar **io_path_cond_loc) {
  *mutex_loc = (DBusMutex*)connection->mutex;
  *dispatch_mutex_loc = (DBusMutex*)connection->dispatch_mutex;
  *io_path_mutex_loc = (DBusMutex*)connection->io_path_mutex;
  *dispatch_cond_loc = connection->dispatch_cond;
  *io_path_cond_loc = connection->io_path_cond;
}
#endif
void _dbus_connection_queue_received_message_link(DBusConnection  *connection, DBusList *link) {
  DBusPendingCall *pending;
  dbus_uint32_t reply_serial;
  DBusMessage *message;
  _dbus_assert(_dbus_transport_peek_is_authenticated(connection->transport));
  _dbus_list_append_link(&connection->incoming_messages, link);
  message = link->data;
  reply_serial = dbus_message_get_reply_serial(message);
  if (reply_serial != 0) {
      pending = _dbus_hash_table_lookup_int(connection->pending_replies, reply_serial);
      if (pending != NULL) {
          if (_dbus_pending_call_is_timeout_added_unlocked(pending))
              _dbus_connection_remove_timeout_unlocked(connection, _dbus_pending_call_get_timeout_unlocked(pending));
          _dbus_pending_call_set_timeout_added_unlocked(pending, FALSE);
	  }
  }
  connection->n_incoming += 1;
  _dbus_connection_wakeup_mainloop(connection);
  _dbus_verbose("Message %p (%s %s %s %s '%s' reply to %u) added to incoming queue %p, %d incoming\n", message,
                dbus_message_type_to_string(dbus_message_get_type(message)), dbus_message_get_path(message) ? dbus_message_get_path(message) : "no path",
                dbus_message_get_interface(message) ? dbus_message_get_interface(message) : "no interface", dbus_message_get_member(message) ?
                dbus_message_get_member(message) : "no member", dbus_message_get_signature(message), dbus_message_get_reply_serial(message), connection,
                connection->n_incoming);
  _dbus_message_trace_ref(message, -1, -1,"_dbus_conection_queue_received_message_link");
}
void _dbus_connection_queue_synthesized_message_link(DBusConnection *connection, DBusList *link) {
  HAVE_LOCK_CHECK (connection);
  _dbus_list_append_link(&connection->incoming_messages, link);
  connection->n_incoming += 1;
  _dbus_connection_wakeup_mainloop(connection);
  _dbus_message_trace_ref(link->data, -1, -1,"_dbus_connection_queue_synthesized_message_link");
  _dbus_verbose("Synthesized message %p added to incoming queue %p, %d incoming\n", link->data, connection, connection->n_incoming);
}
dbus_bool_t _dbus_connection_has_messages_to_send_unlocked(DBusConnection *connection) {
  HAVE_LOCK_CHECK(connection);
  return connection->outgoing_messages != NULL;
}
dbus_bool_t dbus_connection_has_messages_to_send(DBusConnection *connection) {
  dbus_bool_t v;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  CONNECTION_LOCK(connection);
  v = _dbus_connection_has_messages_to_send_unlocked(connection);
  CONNECTION_UNLOCK(connection);
  return v;
}
DBusMessage* _dbus_connection_get_message_to_send(DBusConnection *connection) {
  HAVE_LOCK_CHECK(connection);
  return _dbus_list_get_last(&connection->outgoing_messages);
}
void _dbus_connection_message_sent_unlocked(DBusConnection *connection, DBusMessage *message) {
  DBusList *link;
  HAVE_LOCK_CHECK(connection);
  link = _dbus_list_get_last_link(&connection->outgoing_messages);
  _dbus_assert(link != NULL);
  _dbus_assert(link->data == message);
  _dbus_list_unlink(&connection->outgoing_messages, link);
  _dbus_list_prepend_link(&connection->expired_messages, link);
  connection->n_outgoing -= 1;
  _dbus_verbose("Message %p (%s %s %s %s '%s') removed from outgoing queue %p, %d left to send\n", message,
                dbus_message_type_to_string(dbus_message_get_type(message)), dbus_message_get_path(message) ? dbus_message_get_path(message) :
                "no path", dbus_message_get_interface(message) ? dbus_message_get_interface(message) : "no interface", dbus_message_get_member(message) ?
                dbus_message_get_member(message) : "no member", dbus_message_get_signature(message), connection, connection->n_outgoing);
  _dbus_message_remove_counter(message, connection->outgoing_counter);
}
typedef dbus_bool_t (*DBusWatchAddFunction)(DBusWatchList *list, DBusWatch *watch);
typedef void (*DBusWatchRemoveFunction)(DBusWatchList *list, DBusWatch *watch);
typedef void (*DBusWatchToggleFunction)(DBusWatchList *list, DBusWatch *watch, dbus_bool_t enabled);
static dbus_bool_t protected_change_watch(DBusConnection *connection, DBusWatch *watch, DBusWatchAddFunction add_function, DBusWatchRemoveFunction remove_function,
                                          DBusWatchToggleFunction toggle_function, dbus_bool_t enabled) {
  dbus_bool_t retval;
  if (connection->watches) {
      if (add_function) retval = (*add_function)(connection->watches, watch);
      else if (remove_function) {
          retval = TRUE;
          (*remove_function)(connection->watches, watch);
      } else {
          retval = TRUE;
          (*toggle_function)(connection->watches, watch, enabled);
      }
      return retval;
  } else return FALSE;
}
dbus_bool_t _dbus_connection_add_watch_unlocked(DBusConnection *connection, DBusWatch *watch) {
  return protected_change_watch(connection, watch, _dbus_watch_list_add_watch,NULL, NULL, FALSE);
}
void _dbus_connection_remove_watch_unlocked(DBusConnection *connection, DBusWatch *watch) {
  protected_change_watch(connection, watch,NULL, _dbus_watch_list_remove_watch,NULL, FALSE);
}
void _dbus_connection_toggle_watch_unlocked(DBusConnection *connection, DBusWatch *watch, dbus_bool_t enabled) {
  _dbus_assert(watch != NULL);
  protected_change_watch(connection, watch,NULL, NULL, _dbus_watch_list_toggle_watch, enabled);
}
typedef dbus_bool_t (*DBusTimeoutAddFunction)(DBusTimeoutList *list, DBusTimeout *timeout);
typedef void (*DBusTimeoutRemoveFunction)(DBusTimeoutList *list, DBusTimeout *timeout);
typedef void (*DBusTimeoutToggleFunction)(DBusTimeoutList *list, DBusTimeout *timeout, dbus_bool_t enabled);
static dbus_bool_t protected_change_timeout(DBusConnection *connection, DBusTimeout *timeout, DBusTimeoutAddFunction add_function,
                                            DBusTimeoutRemoveFunction remove_function, DBusTimeoutToggleFunction toggle_function, dbus_bool_t enabled) {
  dbus_bool_t retval;
  HAVE_LOCK_CHECK(connection);
  if (connection->timeouts) {
      if (add_function) retval = (*add_function)(connection->timeouts, timeout);
      else if (remove_function) {
          retval = TRUE;
          (*remove_function)(connection->timeouts, timeout);
      } else {
          retval = TRUE;
          (*toggle_function)(connection->timeouts, timeout, enabled);
      }
      return retval;
  } else return FALSE;
}
dbus_bool_t _dbus_connection_add_timeout_unlocked(DBusConnection *connection, DBusTimeout *timeout) {
  return protected_change_timeout(connection, timeout,_dbus_timeout_list_add_timeout,NULL, NULL, FALSE);
}
void _dbus_connection_remove_timeout_unlocked(DBusConnection *connection, DBusTimeout *timeout) {
  protected_change_timeout(connection, timeout,NULL, _dbus_timeout_list_remove_timeout, NULL, FALSE);
}
void _dbus_connection_toggle_timeout_unlocked(DBusConnection *connection, DBusTimeout *timeout, dbus_bool_t enabled) {
  protected_change_timeout(connection, timeout,NULL,NULL, _dbus_timeout_list_toggle_timeout, enabled);
}
static dbus_bool_t _dbus_connection_attach_pending_call_unlocked(DBusConnection *connection, DBusPendingCall *pending) {
  dbus_uint32_t reply_serial;
  DBusTimeout *timeout;
  HAVE_LOCK_CHECK(connection);
  reply_serial = _dbus_pending_call_get_reply_serial_unlocked(pending);
  _dbus_assert(reply_serial != 0);
  timeout = _dbus_pending_call_get_timeout_unlocked(pending);
  if (timeout) {
      if (!_dbus_connection_add_timeout_unlocked(connection, timeout)) return FALSE;
      if (!_dbus_hash_table_insert_int(connection->pending_replies, reply_serial, pending)) {
          _dbus_connection_remove_timeout_unlocked(connection, timeout);
          _dbus_pending_call_set_timeout_added_unlocked(pending, FALSE);
          HAVE_LOCK_CHECK(connection);
          return FALSE;
      }
      _dbus_pending_call_set_timeout_added_unlocked(pending, TRUE);
  } else {
      if (!_dbus_hash_table_insert_int(connection->pending_replies, reply_serial, pending)) {
          HAVE_LOCK_CHECK(connection);
          return FALSE;
      }
  }
  _dbus_pending_call_ref_unlocked(pending);
  HAVE_LOCK_CHECK(connection);
  return TRUE;
}
static void free_pending_call_on_hash_removal(void *data) {
  DBusPendingCall *pending;
  DBusConnection  *connection;
  if (data == NULL) return;
  pending = data;
  connection = _dbus_pending_call_get_connection_unlocked(pending);
  HAVE_LOCK_CHECK(connection);
  if (_dbus_pending_call_is_timeout_added_unlocked(pending)) {
      _dbus_connection_remove_timeout_unlocked(connection, _dbus_pending_call_get_timeout_unlocked(pending));
      _dbus_pending_call_set_timeout_added_unlocked(pending, FALSE);
  }
  _dbus_connection_ref_unlocked(connection);
  _dbus_pending_call_unref_and_unlock(pending);
  CONNECTION_LOCK(connection);
  _dbus_connection_unref_unlocked(connection);
}
static void _dbus_connection_detach_pending_call_unlocked(DBusConnection *connection, DBusPendingCall *pending) {
  _dbus_hash_table_remove_int(connection->pending_replies, _dbus_pending_call_get_reply_serial_unlocked (pending));
}
static void _dbus_connection_detach_pending_call_and_unlock(DBusConnection  *connection, DBusPendingCall *pending) {
  _dbus_pending_call_ref_unlocked(pending);
  _dbus_hash_table_remove_int(connection->pending_replies, _dbus_pending_call_get_reply_serial_unlocked (pending));
  if (_dbus_pending_call_is_timeout_added_unlocked(pending))
      _dbus_connection_remove_timeout_unlocked(connection, _dbus_pending_call_get_timeout_unlocked(pending));
  _dbus_pending_call_set_timeout_added_unlocked(pending, FALSE);
  _dbus_pending_call_unref_and_unlock(pending);
}
void _dbus_connection_remove_pending_call(DBusConnection *connection, DBusPendingCall *pending) {
  CONNECTION_LOCK(connection);
  _dbus_connection_detach_pending_call_and_unlock(connection, pending);
}
static dbus_bool_t _dbus_connection_acquire_io_path(DBusConnection *connection, int timeout_milliseconds) {
  dbus_bool_t we_acquired;
  HAVE_LOCK_CHECK(connection);
  _dbus_connection_ref_unlocked(connection);
  CONNECTION_UNLOCK(connection);
  _dbus_verbose("locking io_path_mutex\n");
  _dbus_cmutex_lock(connection->io_path_mutex);
  _dbus_verbose("start connection->io_path_acquired = %d timeout = %d\n", connection->io_path_acquired, timeout_milliseconds);
  we_acquired = FALSE;
  if (connection->io_path_acquired) {
      if (timeout_milliseconds != -1) {
          _dbus_verbose("waiting %d for IO path to be acquirable\n", timeout_milliseconds);
          //if (!_dbus_condvar_wait_timeout(connection->io_path_cond, connection->io_path_mutex, timeout_milliseconds)) {}
      } else {
          while(connection->io_path_acquired) {
              _dbus_verbose("waiting for IO path to be acquirable\n");
              _dbus_condvar_wait(connection->io_path_cond, connection->io_path_mutex);
          }
      }
  }
  if (!connection->io_path_acquired) {
      we_acquired = TRUE;
      connection->io_path_acquired = TRUE;
  }
  _dbus_verbose("end connection->io_path_acquired = %d we_acquired = %d\n", connection->io_path_acquired, we_acquired);
  _dbus_verbose("unlocking io_path_mutex\n");
  _dbus_cmutex_unlock(connection->io_path_mutex);
  CONNECTION_LOCK(connection);
  HAVE_LOCK_CHECK(connection);
  _dbus_connection_unref_unlocked(connection);
  return we_acquired;
}
static void _dbus_connection_release_io_path(DBusConnection *connection) {
  HAVE_LOCK_CHECK(connection);
  _dbus_verbose("locking io_path_mutex\n");
  _dbus_cmutex_lock(connection->io_path_mutex);
  _dbus_assert(connection->io_path_acquired);
  _dbus_verbose("start connection->io_path_acquired = %d\n", connection->io_path_acquired);
  connection->io_path_acquired = FALSE;
  _dbus_condvar_wake_one(connection->io_path_cond);
  _dbus_verbose("unlocking io_path_mutex\n");
  _dbus_cmutex_unlock(connection->io_path_mutex);
}
void _dbus_connection_do_iteration_unlocked(DBusConnection *connection, DBusPendingCall *pending, unsigned int flags, int timeout_milliseconds) {
  _dbus_verbose("start\n");
  HAVE_LOCK_CHECK(connection);
  if (connection->n_outgoing == 0) flags &= ~DBUS_ITERATION_DO_WRITING;
  if (_dbus_connection_acquire_io_path(connection, (flags & DBUS_ITERATION_BLOCK) ? timeout_milliseconds : 0)) {
      HAVE_LOCK_CHECK (connection);
      if ((pending != NULL) && _dbus_pending_call_get_completed_unlocked(pending)) { _dbus_verbose("pending call completed while acquiring I/O path"); }
      else if ((pending != NULL) && _dbus_connection_peek_for_reply_unlocked(connection, _dbus_pending_call_get_reply_serial_unlocked(pending))) {
          _dbus_verbose("pending call completed while acquiring I/O path (reply found in queue)");
      } else _dbus_transport_do_iteration(connection->transport, flags, timeout_milliseconds);
      _dbus_connection_release_io_path(connection);
  }
  HAVE_LOCK_CHECK(connection);
  _dbus_verbose("end\n");
}
DBusConnection* _dbus_connection_new_for_transport(DBusTransport *transport) {
  DBusConnection *connection;
  DBusWatchList *watch_list;
  DBusTimeoutList *timeout_list;
  DBusHashTable *pending_replies;
  DBusList *disconnect_link;
  DBusMessage *disconnect_message;
  DBusCounter *outgoing_counter;
  DBusObjectTree *objects;
  watch_list = NULL;
  connection = NULL;
  pending_replies = NULL;
  timeout_list = NULL;
  disconnect_link = NULL;
  disconnect_message = NULL;
  outgoing_counter = NULL;
  objects = NULL;
  watch_list = _dbus_watch_list_new();
  if (watch_list == NULL) goto error;
  timeout_list = _dbus_timeout_list_new();
  if (timeout_list == NULL) goto error;
  pending_replies = _dbus_hash_table_new(DBUS_HASH_INT,NULL, (DBusFreeFunction)free_pending_call_on_hash_removal);
  if (pending_replies == NULL) goto error;
  connection = dbus_new0(DBusConnection, 1);
  if (connection == NULL) goto error;
  _dbus_rmutex_new_at_location(&connection->mutex);
  if (connection->mutex == NULL) goto error;
  _dbus_cmutex_new_at_location(&connection->io_path_mutex);
  if (connection->io_path_mutex == NULL) goto error;
  _dbus_cmutex_new_at_location(&connection->dispatch_mutex);
  if (connection->dispatch_mutex == NULL) goto error;
  _dbus_condvar_new_at_location(&connection->dispatch_cond);
  if (connection->dispatch_cond == NULL) goto error;
  _dbus_condvar_new_at_location(&connection->io_path_cond);
  if (connection->io_path_cond == NULL) goto error;
  _dbus_rmutex_new_at_location(&connection->slot_mutex);
  if (connection->slot_mutex == NULL) goto error;
  disconnect_message = dbus_message_new_signal(DBUS_PATH_LOCAL, DBUS_INTERFACE_LOCAL,"Disconnected");
  if (disconnect_message == NULL) goto error;
  disconnect_link = _dbus_list_alloc_link(disconnect_message);
  if (disconnect_link == NULL) goto error;
  outgoing_counter = _dbus_counter_new();
  if (outgoing_counter == NULL) goto error;
  objects = _dbus_object_tree_new(connection);
  if (objects == NULL) goto error;
  if (_dbus_modify_sigpipe) _dbus_disable_sigpipe();
  _dbus_atomic_inc(&connection->refcount);
  connection->transport = transport;
  connection->watches = watch_list;
  connection->timeouts = timeout_list;
  connection->pending_replies = pending_replies;
  connection->outgoing_counter = outgoing_counter;
  connection->filter_list = NULL;
  connection->last_dispatch_status = DBUS_DISPATCH_COMPLETE;
  connection->objects = objects;
  connection->exit_on_disconnect = FALSE;
  connection->shareable = FALSE;
  connection->route_peer_messages = FALSE;
  connection->disconnected_message_arrived = FALSE;
  connection->disconnected_message_processed = FALSE;
#if defined(DBUS_ENABLE_CHECKS) || defined(DBUS_ENABLE_ASSERT)
  connection->generation = _dbus_current_generation;
#endif
  _dbus_data_slot_list_init(&connection->slot_list);
  connection->client_serial = 1;
  connection->disconnect_message_link = disconnect_link;
  CONNECTION_LOCK(connection);
  if (!_dbus_transport_set_connection(transport, connection)) {
      CONNECTION_UNLOCK(connection);
      goto error;
  }
  _dbus_transport_ref(transport);
  CONNECTION_UNLOCK(connection);
  _dbus_connection_trace_ref(connection, 0, 1, "new_for_transport");
  return connection;
error:
  if (disconnect_message != NULL) dbus_message_unref(disconnect_message);
  if (disconnect_link != NULL) _dbus_list_free_link(disconnect_link);
  if (connection != NULL) {
      _dbus_condvar_free_at_location(&connection->io_path_cond);
      _dbus_condvar_free_at_location(&connection->dispatch_cond);
      _dbus_rmutex_free_at_location(&connection->mutex);
      _dbus_cmutex_free_at_location(&connection->io_path_mutex);
      _dbus_cmutex_free_at_location(&connection->dispatch_mutex);
      _dbus_rmutex_free_at_location(&connection->slot_mutex);
      dbus_free(connection);
  }
  if (pending_replies) _dbus_hash_table_unref(pending_replies);
  if (watch_list) _dbus_watch_list_free(watch_list);
  if (timeout_list) _dbus_timeout_list_free(timeout_list);
  if (outgoing_counter) _dbus_counter_unref(outgoing_counter);
  if (objects) _dbus_object_tree_unref(objects);
  return NULL;
}
DBusConnection *_dbus_connection_ref_unlocked(DBusConnection *connection) {
  dbus_int32_t old_refcount;
  _dbus_assert(connection != NULL);
  _dbus_assert(connection->generation == _dbus_current_generation);
  HAVE_LOCK_CHECK(connection);
  old_refcount = _dbus_atomic_inc(&connection->refcount);
  _dbus_connection_trace_ref(connection, old_refcount, old_refcount + 1,"ref_unlocked");
  return connection;
}
void _dbus_connection_unref_unlocked(DBusConnection *connection) {
  dbus_int32_t old_refcount;
  HAVE_LOCK_CHECK(connection);
  _dbus_assert(connection != NULL);
  old_refcount = _dbus_atomic_dec(&connection->refcount);
  _dbus_connection_trace_ref(connection, old_refcount, old_refcount - 1,"unref_unlocked");
  if (old_refcount == 1) _dbus_connection_last_unref(connection);
}
static dbus_uint32_t _dbus_connection_get_next_client_serial(DBusConnection *connection) {
  dbus_uint32_t serial;
  serial = connection->client_serial++;
  if (connection->client_serial == 0) connection->client_serial = 1;
  return serial;
}
dbus_bool_t _dbus_connection_handle_watch(DBusWatch *watch, unsigned int condition, void *data) {
  DBusConnection *connection;
  dbus_bool_t retval;
  DBusDispatchStatus status;
  connection = data;
  _dbus_verbose("start\n");
  CONNECTION_LOCK(connection);
  if (!_dbus_connection_acquire_io_path(connection, 1)) {
      CONNECTION_UNLOCK(connection);
      return TRUE;
  }
  HAVE_LOCK_CHECK(connection);
  retval = _dbus_transport_handle_watch(connection->transport, watch, condition);
  _dbus_connection_release_io_path(connection);
  HAVE_LOCK_CHECK(connection);
  _dbus_verbose("middle\n");
  status = _dbus_connection_get_dispatch_status_unlocked(connection);
  _dbus_connection_update_dispatch_status_and_unlock(connection, status);
  _dbus_verbose("end\n");
  return retval;
}
static DBusHashTable *shared_connections = NULL;
static DBusList *shared_connections_no_guid = NULL;
static void close_connection_on_shutdown(DBusConnection *connection) {
  DBusMessage *message;
  dbus_connection_ref(connection);
  _dbus_connection_close_possibly_shared(connection);
  while((message = dbus_connection_pop_message(connection))) dbus_message_unref(message);
  dbus_connection_unref(connection);
}
static void shared_connections_shutdown(void *data) {
  int n_entries;
  if (!_DBUS_LOCK(shared_connections)) return;
  while((n_entries = _dbus_hash_table_get_n_entries(shared_connections)) > 0) {
      DBusConnection *connection;
      DBusHashIter iter;
      _dbus_hash_iter_init(shared_connections, &iter);
      _dbus_hash_iter_next(&iter);
      connection = _dbus_hash_iter_get_value(&iter);
      _DBUS_UNLOCK(shared_connections);
      close_connection_on_shutdown(connection);
      if (!_DBUS_LOCK(shared_connections)) _dbus_assert_not_reached("global locks were already initialized");
      _dbus_assert(_dbus_hash_table_get_n_entries(shared_connections) < n_entries);
  }
  _dbus_assert(_dbus_hash_table_get_n_entries(shared_connections) == 0);
  _dbus_hash_table_unref(shared_connections);
  shared_connections = NULL;
  if (shared_connections_no_guid != NULL) {
      DBusConnection *connection;
      connection = _dbus_list_pop_first(&shared_connections_no_guid);
      while(connection != NULL) {
          _DBUS_UNLOCK(shared_connections);
          close_connection_on_shutdown(connection);
          if (!_DBUS_LOCK(shared_connections)) _dbus_assert_not_reached("global locks were already initialized");
          connection = _dbus_list_pop_first(&shared_connections_no_guid);
      }
  }
  shared_connections_no_guid = NULL;
  _DBUS_UNLOCK(shared_connections);
}
static dbus_bool_t connection_lookup_shared(DBusAddressEntry *entry, DBusConnection **result) {
  _dbus_verbose("checking for existing connection\n");
  *result = NULL;
  if (!_DBUS_LOCK(shared_connections)) return FALSE;
  if (shared_connections == NULL) {
      _dbus_verbose("creating shared_connections hash table\n");
      shared_connections = _dbus_hash_table_new(DBUS_HASH_STRING, dbus_free,NULL);
      if (shared_connections == NULL) {
          _DBUS_UNLOCK(shared_connections);
          return FALSE;
      }
      if (!_dbus_register_shutdown_func(shared_connections_shutdown, NULL)) {
          _dbus_hash_table_unref(shared_connections);
          shared_connections = NULL;
          _DBUS_UNLOCK(shared_connections);
          return FALSE;
      }
      _dbus_verbose("  successfully created shared_connections\n");
      _DBUS_UNLOCK(shared_connections);
      return TRUE;
  } else {
      const char *guid;
      guid = dbus_address_entry_get_value(entry, "guid");
      if (guid != NULL) {
          DBusConnection *connection;
          connection = _dbus_hash_table_lookup_string(shared_connections, guid);
          if (connection) {
              CONNECTION_LOCK(connection);
              if (_dbus_connection_get_is_connected_unlocked(connection)) {
                  _dbus_connection_ref_unlocked(connection);
                  *result = connection;
                  _dbus_verbose("looked up existing connection to server guid %s\n", guid);
              } else { _dbus_verbose("looked up existing connection to server guid %s but it was disconnected so ignoring it\n", guid); }
              CONNECTION_UNLOCK(connection);
          }
      }
      _DBUS_UNLOCK(shared_connections);
      return TRUE;
  }
}
static dbus_bool_t connection_record_shared_unlocked(DBusConnection *connection, const char *guid) {
  char *guid_key;
  char *guid_in_connection;
  HAVE_LOCK_CHECK(connection);
  _dbus_assert(connection->server_guid == NULL);
  _dbus_assert(connection->shareable);
  _dbus_connection_ref_unlocked(connection);
  if (guid == NULL) {
      if (!_DBUS_LOCK(shared_connections)) return FALSE;
      if (!_dbus_list_prepend(&shared_connections_no_guid, connection)) {
          _DBUS_UNLOCK(shared_connections);
          return FALSE;
      }
      _DBUS_UNLOCK(shared_connections);
      return TRUE;
  }
  guid_key = _dbus_strdup(guid);
  if (guid_key == NULL) return FALSE;
  guid_in_connection = _dbus_strdup(guid);
  if (guid_in_connection == NULL) {
      dbus_free(guid_key);
      return FALSE;
  }
  if (!_DBUS_LOCK(shared_connections)) {
      dbus_free(guid_in_connection);
      dbus_free(guid_key);
      return FALSE;
  }
  _dbus_assert(shared_connections != NULL);
  if (!_dbus_hash_table_insert_string(shared_connections, guid_key, connection)) {
      dbus_free(guid_key);
      dbus_free(guid_in_connection);
      _DBUS_UNLOCK(shared_connections);
      return FALSE;
  }
  connection->server_guid = guid_in_connection;
  _dbus_verbose("stored connection to %s to be shared\n", connection->server_guid);
  _DBUS_UNLOCK(shared_connections);
  _dbus_assert(connection->server_guid != NULL);
  return TRUE;
}
static void connection_forget_shared_unlocked(DBusConnection *connection) {
  HAVE_LOCK_CHECK (connection);
  if (!connection->shareable) return;
  if (!_DBUS_LOCK (shared_connections)) return;
  if (connection->server_guid != NULL) {
      _dbus_verbose("dropping connection to %s out of the shared table\n", connection->server_guid);
      if (!_dbus_hash_table_remove_string(shared_connections, connection->server_guid)) _dbus_assert_not_reached("connection was not in the shared table");
      dbus_free(connection->server_guid);
      connection->server_guid = NULL;
  } else _dbus_list_remove(&shared_connections_no_guid, connection);
  _DBUS_UNLOCK(shared_connections);
  _dbus_connection_unref_unlocked(connection);
}
static DBusConnection* connection_try_from_address_entry(DBusAddressEntry *entry, DBusError *error) {
  DBusTransport *transport;
  DBusConnection *connection;
  transport = _dbus_transport_open(entry, error);
  if (transport == NULL) {
      _DBUS_ASSERT_ERROR_IS_SET(error);
      return NULL;
  }
  connection = _dbus_connection_new_for_transport(transport);
  _dbus_transport_unref(transport);
  if (connection == NULL) {
      _DBUS_SET_OOM(error);
      return NULL;
  }
#ifndef DBUS_DISABLE_CHECKS
  _dbus_assert(!connection->have_connection_lock);
#endif
  return connection;
}
static DBusConnection* _dbus_connection_open_internal(const char *address, dbus_bool_t shared, DBusError *error) {
  DBusConnection *connection;
  DBusAddressEntry **entries;
  DBusError tmp_error = DBUS_ERROR_INIT;
  DBusError first_error = DBUS_ERROR_INIT;
  int len, i;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  _dbus_verbose("opening %s connection to: %s\n", shared ? "shared" : "private", address);
  if (!dbus_parse_address(address, &entries, &len, error)) return NULL;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  connection = NULL;
  for (i = 0; i < len; i++) {
      if (shared) {
          if (!connection_lookup_shared(entries[i], &connection)) _DBUS_SET_OOM(&tmp_error);
      }
      if (connection == NULL) {
          connection = connection_try_from_address_entry(entries[i], &tmp_error);
          if (connection != NULL && shared) {
              const char *guid;
              connection->shareable = TRUE;
              guid = dbus_address_entry_get_value(entries[i], "guid");
              CONNECTION_LOCK(connection);
              if (!connection_record_shared_unlocked(connection, guid)) {
                  _DBUS_SET_OOM(&tmp_error);
                  _dbus_connection_close_possibly_shared_and_unlock(connection);
                  dbus_connection_unref(connection);
                  connection = NULL;
              } else CONNECTION_UNLOCK(connection);
          }
      }
      if (connection) break;
      _DBUS_ASSERT_ERROR_IS_SET(&tmp_error);
      if (i == 0) dbus_move_error(&tmp_error, &first_error);
      else dbus_error_free(&tmp_error);
  }
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  _DBUS_ASSERT_ERROR_IS_CLEAR(&tmp_error);
  if (connection == NULL) {
      _DBUS_ASSERT_ERROR_IS_SET(&first_error);
      dbus_move_error(&first_error, error);
  } else dbus_error_free(&first_error);
  dbus_address_entries_free(entries);
  return connection;
}
void _dbus_connection_close_possibly_shared(DBusConnection *connection) {
  _dbus_assert(connection != NULL);
  _dbus_assert(connection->generation == _dbus_current_generation);
  CONNECTION_LOCK(connection);
  _dbus_connection_close_possibly_shared_and_unlock(connection);
}
static DBusPreallocatedSend* _dbus_connection_preallocate_send_unlocked(DBusConnection *connection) {
  DBusPreallocatedSend *preallocated;
  HAVE_LOCK_CHECK(connection);
  _dbus_assert(connection != NULL);
  preallocated = dbus_new(DBusPreallocatedSend, 1);
  if (preallocated == NULL) return NULL;
  preallocated->queue_link = _dbus_list_alloc_link(NULL);
  if (preallocated->queue_link == NULL) goto failed_0;
  preallocated->counter_link = _dbus_list_alloc_link(connection->outgoing_counter);
  if (preallocated->counter_link == NULL) goto failed_1;
  _dbus_counter_ref(preallocated->counter_link->data);
  preallocated->connection = connection;
  return preallocated;
failed_1:
  _dbus_list_free_link(preallocated->queue_link);
failed_0:
  dbus_free(preallocated);
  return NULL;
}
static void _dbus_connection_send_preallocated_unlocked_no_update(DBusConnection *connection, DBusPreallocatedSend *preallocated, DBusMessage *message,
                                                                  dbus_uint32_t *client_serial) {
  dbus_uint32_t serial;
  preallocated->queue_link->data = message;
  _dbus_list_prepend_link(&connection->outgoing_messages, preallocated->queue_link);
  _dbus_message_add_counter_link(message, preallocated->counter_link);
  dbus_free(preallocated);
  preallocated = NULL;
  dbus_message_ref(message);
  connection->n_outgoing += 1;
  _dbus_verbose("Message %p (%s %s %s %s '%s') for %s added to outgoing queue %p, %d pending to send\n", message,
                dbus_message_type_to_string(dbus_message_get_type(message)), dbus_message_get_path(message) ? dbus_message_get_path(message) : "no path",
                dbus_message_get_interface(message) ? dbus_message_get_interface(message) : "no interface", dbus_message_get_member(message) ?
                dbus_message_get_member(message) : "no member", dbus_message_get_signature(message), dbus_message_get_destination(message) ?
                dbus_message_get_destination(message) : "null", connection, connection->n_outgoing);
  if (dbus_message_get_serial (message) == 0) {
      serial = _dbus_connection_get_next_client_serial(connection);
      dbus_message_set_serial(message, serial);
      if (client_serial) *client_serial = serial;
  } else {
      if (client_serial) *client_serial = dbus_message_get_serial(message);
  }
  _dbus_verbose("Message %p serial is %u\n", message, dbus_message_get_serial(message));
  dbus_message_lock(message);
  _dbus_connection_do_iteration_unlocked(connection,NULL,DBUS_ITERATION_DO_WRITING,-1);
  if (connection->n_outgoing > 0) _dbus_connection_wakeup_mainloop(connection);
}
static void _dbus_connection_send_preallocated_and_unlock(DBusConnection *connection, DBusPreallocatedSend *preallocated, DBusMessage *message,
					                                      dbus_uint32_t *client_serial) {
  DBusDispatchStatus status;
  HAVE_LOCK_CHECK(connection);
  _dbus_connection_send_preallocated_unlocked_no_update(connection, preallocated, message, client_serial);
  _dbus_verbose("middle\n");
  status = _dbus_connection_get_dispatch_status_unlocked(connection);
  _dbus_connection_update_dispatch_status_and_unlock(connection, status);
}
dbus_bool_t _dbus_connection_send_and_unlock(DBusConnection *connection, DBusMessage *message, dbus_uint32_t *client_serial) {
  DBusPreallocatedSend *preallocated;
  _dbus_assert(connection != NULL);
  _dbus_assert(message != NULL);
  preallocated = _dbus_connection_preallocate_send_unlocked(connection);
  if (preallocated == NULL) {
      CONNECTION_UNLOCK(connection);
      return FALSE;
  }
  _dbus_connection_send_preallocated_and_unlock(connection, preallocated, message, client_serial);
  return TRUE;
}
void _dbus_connection_close_if_only_one_ref(DBusConnection *connection) {
  dbus_int32_t refcount;
  CONNECTION_LOCK(connection);
  refcount = _dbus_atomic_get(&connection->refcount);
  _dbus_assert(refcount >= 1);
  if (refcount == 1) _dbus_connection_close_possibly_shared_and_unlock(connection);
  else CONNECTION_UNLOCK(connection);
}
static void _dbus_memory_pause_based_on_timeout(int timeout_milliseconds) {
  if (timeout_milliseconds == -1) _dbus_sleep_milliseconds(1000);
  else if (timeout_milliseconds < 100);
  else if (timeout_milliseconds <= 1000) _dbus_sleep_milliseconds(timeout_milliseconds / 3);
  else _dbus_sleep_milliseconds(1000);
}
static DBusMessage *generate_local_error_message(dbus_uint32_t serial, const char *error_name, const char *error_msg) {
  DBusMessage *message;
  message = dbus_message_new(DBUS_MESSAGE_TYPE_ERROR);
  if (!message) goto out;
  if (!dbus_message_set_error_name(message, error_name)) {
      dbus_message_unref(message);
      message = NULL;
      goto out;
  }
  dbus_message_set_no_reply(message, TRUE);
  if (!dbus_message_set_reply_serial(message, serial)) {
      dbus_message_unref(message);
      message = NULL;
      goto out;
  }
  if (error_msg != NULL) {
      DBusMessageIter iter;
      dbus_message_iter_init_append(message, &iter);
      if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &error_msg)) {
          dbus_message_unref(message);
          message = NULL;
	      goto out;
      }
  }
out:
  return message;
}
static dbus_bool_t _dbus_connection_peek_for_reply_unlocked(DBusConnection *connection, dbus_uint32_t client_serial) {
  DBusList *link;
  HAVE_LOCK_CHECK(connection);
  link = _dbus_list_get_first_link(&connection->incoming_messages);
  while(link != NULL) {
      DBusMessage *reply = link->data;
      if (dbus_message_get_reply_serial(reply) == client_serial) {
          _dbus_verbose("%s reply to %d found in queue\n", _DBUS_FUNCTION_NAME, client_serial);
          return TRUE;
      }
      link = _dbus_list_get_next_link(&connection->incoming_messages, link);
  }
  return FALSE;
}
static DBusMessage* check_for_reply_unlocked(DBusConnection *connection, dbus_uint32_t client_serial) {
  DBusList *link;
  HAVE_LOCK_CHECK(connection);
  link = _dbus_list_get_first_link(&connection->incoming_messages);
  while(link != NULL) {
      DBusMessage *reply = link->data;
      if (dbus_message_get_reply_serial(reply) == client_serial) {
          _dbus_list_remove_link(&connection->incoming_messages, link);
          connection->n_incoming  -= 1;
          return reply;
	  }
      link = _dbus_list_get_next_link(&connection->incoming_messages, link);
  }
  return NULL;
}
static void connection_timeout_and_complete_all_pending_calls_unlocked(DBusConnection *connection) {
  while(_dbus_hash_table_get_n_entries(connection->pending_replies) > 0) {
      DBusPendingCall *pending;
      DBusHashIter iter;
      _dbus_hash_iter_init(connection->pending_replies, &iter);
      _dbus_hash_iter_next(&iter);
      pending = _dbus_hash_iter_get_value(&iter);
      _dbus_pending_call_ref_unlocked(pending);
      _dbus_pending_call_queue_timeout_error_unlocked(pending, connection);
      if (_dbus_pending_call_is_timeout_added_unlocked(pending))
          _dbus_connection_remove_timeout_unlocked(connection, _dbus_pending_call_get_timeout_unlocked(pending));
      _dbus_pending_call_set_timeout_added_unlocked(pending, FALSE);
      _dbus_hash_iter_remove_entry(&iter);
      _dbus_pending_call_unref_and_unlock(pending);
      CONNECTION_LOCK(connection);
  }
  HAVE_LOCK_CHECK(connection);
}
static void complete_pending_call_and_unlock(DBusConnection *connection, DBusPendingCall *pending, DBusMessage *message) {
  _dbus_pending_call_set_reply_unlocked(pending, message);
  _dbus_pending_call_ref_unlocked(pending);
  _dbus_connection_detach_pending_call_and_unlock(connection, pending);
  _dbus_pending_call_complete(pending);
  dbus_pending_call_unref(pending);
}
static dbus_bool_t check_for_reply_and_update_dispatch_unlocked(DBusConnection  *connection, DBusPendingCall *pending) {
  DBusMessage *reply;
  DBusDispatchStatus status;
  reply = check_for_reply_unlocked(connection, _dbus_pending_call_get_reply_serial_unlocked (pending));
  if (reply != NULL) {
      _dbus_verbose("checked for reply\n");
      _dbus_verbose("dbus_connection_send_with_reply_and_block(): got reply\n");
      complete_pending_call_and_unlock(connection, pending, reply);
      dbus_message_unref(reply);
      CONNECTION_LOCK(connection);
      status = _dbus_connection_get_dispatch_status_unlocked(connection);
      _dbus_connection_update_dispatch_status_and_unlock(connection, status);
      dbus_pending_call_unref(pending);
      return TRUE;
  }
  return FALSE;
}
void _dbus_connection_block_pending_call(DBusPendingCall *pending) {
  long start_tv_sec, start_tv_usec;
  long tv_sec, tv_usec;
  DBusDispatchStatus status;
  DBusConnection *connection;
  dbus_uint32_t client_serial;
  DBusTimeout *timeout;
  int timeout_milliseconds, elapsed_milliseconds;
  _dbus_assert(pending != NULL);
  if (dbus_pending_call_get_completed(pending)) return;
  dbus_pending_call_ref(pending);
  connection = _dbus_pending_call_get_connection_and_lock(pending);
  _dbus_connection_flush_unlocked(connection);
  client_serial = _dbus_pending_call_get_reply_serial_unlocked(pending);
  timeout = _dbus_pending_call_get_timeout_unlocked(pending);
  _dbus_get_monotonic_time(&start_tv_sec, &start_tv_usec);
  if (timeout) {
      timeout_milliseconds = dbus_timeout_get_interval(timeout);
      _dbus_verbose("dbus_connection_send_with_reply_and_block(): will block %d milliseconds for reply serial %u from %ld sec %ld usec\n", timeout_milliseconds,
                    client_serial, start_tv_sec, start_tv_usec);
  } else {
      timeout_milliseconds = -1;
      _dbus_verbose("dbus_connection_send_with_reply_and_block(): will block for reply serial %u\n", client_serial);
  }
  if (check_for_reply_and_update_dispatch_unlocked(connection, pending)) return;
  _dbus_connection_do_iteration_unlocked(connection, pending,DBUS_ITERATION_DO_READING | DBUS_ITERATION_BLOCK, timeout_milliseconds);
recheck_status:
  _dbus_verbose("top of recheck\n");
  HAVE_LOCK_CHECK(connection);
  status = _dbus_connection_get_dispatch_status_unlocked(connection);
  if (_dbus_pending_call_get_completed_unlocked(pending)) {
      _dbus_verbose("Pending call completed by dispatch\n");
      _dbus_connection_update_dispatch_status_and_unlock(connection, status);
      dbus_pending_call_unref(pending);
      return;
  }
  if (status == DBUS_DISPATCH_DATA_REMAINS) {
      if (check_for_reply_and_update_dispatch_unlocked(connection, pending)) return;
  }
  _dbus_get_monotonic_time(&tv_sec, &tv_usec);
  elapsed_milliseconds = (tv_sec - start_tv_sec) * 1000 + (tv_usec - start_tv_usec) / 1000;
  if (!_dbus_connection_get_is_connected_unlocked(connection)) {
      DBusMessage *error_msg;
      error_msg = generate_local_error_message(client_serial, DBUS_ERROR_DISCONNECTED,"Connection was disconnected before a reply was received");
      complete_pending_call_and_unlock(connection, pending, error_msg);
      if (error_msg != NULL) dbus_message_unref(error_msg);
      dbus_pending_call_unref(pending);
      return;
  } else if (connection->disconnect_message_link == NULL) _dbus_verbose("dbus_connection_send_with_reply_and_block(): disconnected\n");
  else if (timeout == NULL) {
       if (status == DBUS_DISPATCH_NEED_MEMORY) {
          _dbus_verbose("dbus_connection_send_with_reply_and_block() waiting for more memory\n");
          _dbus_memory_pause_based_on_timeout(timeout_milliseconds - elapsed_milliseconds);
       } else {
          _dbus_connection_do_iteration_unlocked(connection, pending,DBUS_ITERATION_DO_READING | DBUS_ITERATION_BLOCK,
                                                 timeout_milliseconds - elapsed_milliseconds);
       }
       goto recheck_status;
  } else if (tv_sec < start_tv_sec) _dbus_verbose("dbus_connection_send_with_reply_and_block(): clock set backward\n");
  else if (elapsed_milliseconds < timeout_milliseconds) {
      _dbus_verbose("dbus_connection_send_with_reply_and_block(): %d milliseconds remain\n", timeout_milliseconds - elapsed_milliseconds);
      if (status == DBUS_DISPATCH_NEED_MEMORY) {
          _dbus_verbose("dbus_connection_send_with_reply_and_block() waiting for more memory\n");
          _dbus_memory_pause_based_on_timeout(timeout_milliseconds - elapsed_milliseconds);
      } else {
          _dbus_connection_do_iteration_unlocked(connection,NULL,DBUS_ITERATION_DO_READING | DBUS_ITERATION_BLOCK,
                                                 timeout_milliseconds - elapsed_milliseconds);
      }
      goto recheck_status;
  }
  _dbus_verbose("dbus_connection_send_with_reply_and_block(): Waited %d milliseconds and got no reply\n", elapsed_milliseconds);
  _dbus_assert(!_dbus_pending_call_get_completed_unlocked(pending));
  complete_pending_call_and_unlock(connection, pending, NULL);
  CONNECTION_LOCK(connection);
  status = _dbus_connection_get_dispatch_status_unlocked(connection);
  _dbus_connection_update_dispatch_status_and_unlock(connection, status);
  dbus_pending_call_unref(pending);
}
int _dbus_connection_get_pending_fds_count(DBusConnection *connection) {
  return _dbus_transport_get_pending_fds_count(connection->transport);
}
void _dbus_connection_set_pending_fds_function(DBusConnection *connection, DBusPendingFdsChangeFunction callback, void *data) {
  _dbus_transport_set_pending_fds_function(connection->transport, callback, data);
}
DBusConnection* dbus_connection_open(const char *address, DBusError *error) {
  DBusConnection *connection;
  _dbus_return_val_if_fail(address != NULL, NULL);
  _dbus_return_val_if_error_is_set(error, NULL);
  connection = _dbus_connection_open_internal(address, TRUE, error);
  return connection;
}
DBusConnection* dbus_connection_open_private(const char *address, DBusError *error) {
  DBusConnection *connection;
  _dbus_return_val_if_fail(address != NULL, NULL);
  _dbus_return_val_if_error_is_set(error, NULL);
  connection = _dbus_connection_open_internal(address,FALSE, error);
  return connection;
}
DBusConnection *dbus_connection_ref(DBusConnection *connection) {
  dbus_int32_t old_refcount;
  _dbus_return_val_if_fail(connection != NULL, NULL);
  _dbus_return_val_if_fail(connection->generation == _dbus_current_generation, NULL);
  old_refcount = _dbus_atomic_inc(&connection->refcount);
  _dbus_connection_trace_ref(connection, old_refcount, old_refcount + 1,"ref");
  return connection;
}
static void free_outgoing_message(void *element, void *data) {
  DBusMessage *message = element;
  DBusConnection *connection = data;
  _dbus_message_remove_counter(message, connection->outgoing_counter);
  dbus_message_unref(message);
}
static void _dbus_connection_last_unref(DBusConnection *connection) {
  DBusList *link;
  _dbus_verbose("Finalizing connection %p\n", connection);
  _dbus_assert(_dbus_atomic_get(&connection->refcount) == 0);
  _dbus_assert(!_dbus_transport_get_is_connected(connection->transport));
  _dbus_assert(connection->server_guid == NULL);
  _dbus_object_tree_free_all_unlocked(connection->objects);
  dbus_connection_set_dispatch_status_function(connection, NULL, NULL, NULL);
  dbus_connection_set_wakeup_main_function(connection, NULL, NULL, NULL);
  dbus_connection_set_unix_user_function(connection, NULL, NULL, NULL);
  dbus_connection_set_windows_user_function(connection, NULL, NULL, NULL);
  _dbus_watch_list_free(connection->watches);
  connection->watches = NULL;
  _dbus_timeout_list_free(connection->timeouts);
  connection->timeouts = NULL;
  _dbus_data_slot_list_free(&connection->slot_list);
  link = _dbus_list_get_first_link(&connection->filter_list);
  while(link != NULL) {
      DBusMessageFilter *filter = link->data;
      DBusList *next = _dbus_list_get_next_link(&connection->filter_list, link);
      filter->function = NULL;
      _dbus_message_filter_unref(filter);
      link->data = NULL;
      link = next;
  }
  _dbus_list_clear(&connection->filter_list);
  _dbus_object_tree_unref(connection->objects);
  _dbus_hash_table_unref(connection->pending_replies);
  connection->pending_replies = NULL;
  _dbus_list_foreach(&connection->outgoing_messages, free_outgoing_message, connection);
  _dbus_list_clear(&connection->outgoing_messages);
  _dbus_list_foreach(&connection->incoming_messages, (DBusForeachFunction)dbus_message_unref,NULL);
  _dbus_list_clear(&connection->incoming_messages);
  _dbus_counter_unref(connection->outgoing_counter);
  _dbus_transport_unref(connection->transport);
  if (connection->disconnect_message_link) {
      DBusMessage *message = connection->disconnect_message_link->data;
      dbus_message_unref(message);
      _dbus_list_free_link(connection->disconnect_message_link);
  }
  _dbus_condvar_free_at_location(&connection->dispatch_cond);
  _dbus_condvar_free_at_location(&connection->io_path_cond);
  _dbus_cmutex_free_at_location(&connection->io_path_mutex);
  _dbus_cmutex_free_at_location(&connection->dispatch_mutex);
  _dbus_rmutex_free_at_location(&connection->slot_mutex);
  _dbus_rmutex_free_at_location(&connection->mutex);
  dbus_free(connection);
}
void dbus_connection_unref(DBusConnection *connection) {
  dbus_int32_t old_refcount;
  _dbus_return_if_fail(connection != NULL);
  _dbus_return_if_fail(connection->generation == _dbus_current_generation);
  old_refcount = _dbus_atomic_dec(&connection->refcount);
  _dbus_connection_trace_ref(connection, old_refcount, old_refcount - 1,"unref");
  if (old_refcount == 1) {
  #ifndef DBUS_DISABLE_CHECKS
      if (_dbus_transport_get_is_connected(connection->transport)) {
          _dbus_warn_check_failed("The last reference on a connection was dropped without closing the connection. This is a bug in an application. See "
                                  "dbus_connection_unref() documentation for details.\n%s", connection->shareable ? "Most likely, the application called "
                                  "unref() too many times and removed a reference belonging to libdbus, since this is a shared connection." :
                                  "Most likely, the application was supposed to call dbus_connection_close(), since this is a private connection.");
          return;
      }
  #endif
      _dbus_connection_last_unref(connection);
   }
}
static void _dbus_connection_close_possibly_shared_and_unlock(DBusConnection *connection) {
  DBusDispatchStatus status;
  HAVE_LOCK_CHECK(connection);
  _dbus_verbose("Disconnecting %p\n", connection);
  _dbus_connection_ref_unlocked(connection);
  _dbus_transport_disconnect(connection->transport);
  status = _dbus_connection_get_dispatch_status_unlocked(connection);
  _dbus_connection_update_dispatch_status_and_unlock(connection, status);
  dbus_connection_unref(connection);
}
void dbus_connection_close(DBusConnection *connection) {
  _dbus_return_if_fail(connection != NULL);
  _dbus_return_if_fail(connection->generation == _dbus_current_generation);
  CONNECTION_LOCK(connection);
#ifndef DBUS_DISABLE_CHECKS
  if (connection->shareable) {
      CONNECTION_UNLOCK(connection);
      _dbus_warn_check_failed("Applications must not close shared connections - see dbus_connection_close() docs. This is a bug in the application.");
      return;
  }
#endif
  _dbus_connection_close_possibly_shared_and_unlock(connection);
}
static dbus_bool_t _dbus_connection_get_is_connected_unlocked(DBusConnection *connection) {
  HAVE_LOCK_CHECK(connection);
  return _dbus_transport_get_is_connected(connection->transport);
}
dbus_bool_t dbus_connection_get_is_connected(DBusConnection *connection) {
  dbus_bool_t res;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  CONNECTION_LOCK(connection);
  res = _dbus_connection_get_is_connected_unlocked(connection);
  CONNECTION_UNLOCK(connection);
  return res;
}
dbus_bool_t dbus_connection_get_is_authenticated(DBusConnection *connection) {
  dbus_bool_t res;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  CONNECTION_LOCK(connection);
  res = _dbus_transport_try_to_authenticate(connection->transport);
  CONNECTION_UNLOCK(connection);
  return res;
}
dbus_bool_t dbus_connection_get_is_anonymous(DBusConnection *connection) {
  dbus_bool_t res;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  CONNECTION_LOCK(connection);
  res = _dbus_transport_get_is_anonymous(connection->transport);
  CONNECTION_UNLOCK(connection);
  return res;
}
char* dbus_connection_get_server_id(DBusConnection *connection) {
  char *id;
  _dbus_return_val_if_fail(connection != NULL, NULL);
  CONNECTION_LOCK(connection);
  id = _dbus_strdup(_dbus_transport_get_server_id (connection->transport));
  CONNECTION_UNLOCK(connection);
  return id;
}
dbus_bool_t dbus_connection_can_send_type(DBusConnection *connection, int type) {
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  if (!dbus_type_is_valid(type)) return FALSE;
  if (type != DBUS_TYPE_UNIX_FD) return TRUE;
#ifdef HAVE_UNIX_FD_PASSING
  {
      dbus_bool_t b;
      CONNECTION_LOCK(connection);
      b = _dbus_transport_can_pass_unix_fd(connection->transport);
      CONNECTION_UNLOCK(connection);
      return b;
  }
#endif
  return FALSE;
}
void dbus_connection_set_exit_on_disconnect(DBusConnection *connection, dbus_bool_t exit_on_disconnect) {
  _dbus_return_if_fail(connection != NULL);
  CONNECTION_LOCK(connection);
  connection->exit_on_disconnect = exit_on_disconnect != FALSE;
  CONNECTION_UNLOCK(connection);
}
DBusPreallocatedSend* dbus_connection_preallocate_send(DBusConnection *connection) {
  DBusPreallocatedSend *preallocated;
  _dbus_return_val_if_fail(connection != NULL, NULL);
  CONNECTION_LOCK(connection);
  preallocated = _dbus_connection_preallocate_send_unlocked(connection);
  CONNECTION_UNLOCK(connection);
  return preallocated;
}
void dbus_connection_free_preallocated_send(DBusConnection *connection, DBusPreallocatedSend *preallocated) {
  _dbus_return_if_fail(connection != NULL);
  _dbus_return_if_fail(preallocated != NULL);
  _dbus_return_if_fail(connection == preallocated->connection);
  _dbus_list_free_link(preallocated->queue_link);
  _dbus_counter_unref(preallocated->counter_link->data);
  _dbus_list_free_link(preallocated->counter_link);
  dbus_free(preallocated);
}
void dbus_connection_send_preallocated(DBusConnection *connection, DBusPreallocatedSend *preallocated, DBusMessage *message, dbus_uint32_t *client_serial) {
  _dbus_return_if_fail(connection != NULL);
  _dbus_return_if_fail(preallocated != NULL);
  _dbus_return_if_fail(message != NULL);
  _dbus_return_if_fail(preallocated->connection == connection);
  _dbus_return_if_fail(dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_METHOD_CALL || dbus_message_get_member(message) != NULL);
  _dbus_return_if_fail(dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_SIGNAL || (dbus_message_get_interface(message) != NULL &&
                       dbus_message_get_member(message) != NULL));
  CONNECTION_LOCK(connection);
#ifdef HAVE_UNIX_FD_PASSING
  if (!_dbus_transport_can_pass_unix_fd(connection->transport) && message->n_unix_fds > 0) {
      CONNECTION_UNLOCK(connection);
      return;
  }
#endif
  _dbus_connection_send_preallocated_and_unlock(connection, preallocated, message, client_serial);
}
static dbus_bool_t _dbus_connection_send_unlocked_no_update(DBusConnection *connection, DBusMessage *message, dbus_uint32_t *client_serial) {
  DBusPreallocatedSend *preallocated;
  _dbus_assert(connection != NULL);
  _dbus_assert(message != NULL);
  preallocated = _dbus_connection_preallocate_send_unlocked(connection);
  if (preallocated == NULL) return FALSE;
  _dbus_connection_send_preallocated_unlocked_no_update(connection, preallocated, message, client_serial);
  return TRUE;
}
dbus_bool_t dbus_connection_send(DBusConnection *connection, DBusMessage *message, dbus_uint32_t *serial) {
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(message != NULL, FALSE);
  CONNECTION_LOCK(connection);
#ifdef HAVE_UNIX_FD_PASSING
  if (!_dbus_transport_can_pass_unix_fd(connection->transport) && message->n_unix_fds > 0) {
      CONNECTION_UNLOCK(connection);
      return FALSE;
  }
#endif
  return _dbus_connection_send_and_unlock(connection, message, serial);
}
static dbus_bool_t reply_handler_timeout(void *data) {
  DBusConnection *connection;
  DBusDispatchStatus status;
  DBusPendingCall *pending = data;
  connection = _dbus_pending_call_get_connection_and_lock(pending);
  _dbus_connection_ref_unlocked(connection);
  _dbus_pending_call_queue_timeout_error_unlocked(pending, connection);
  _dbus_connection_remove_timeout_unlocked(connection, _dbus_pending_call_get_timeout_unlocked(pending));
  _dbus_pending_call_set_timeout_added_unlocked(pending, FALSE);
  _dbus_verbose("middle\n");
  status = _dbus_connection_get_dispatch_status_unlocked(connection);
  _dbus_connection_update_dispatch_status_and_unlock(connection, status);
  dbus_connection_unref(connection);
  return TRUE;
}
dbus_bool_t dbus_connection_send_with_reply(DBusConnection *connection, DBusMessage *message, DBusPendingCall **pending_return, int timeout_milliseconds) {
  DBusPendingCall *pending;
  dbus_int32_t serial = -1;
  DBusDispatchStatus status;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(message != NULL, FALSE);
  _dbus_return_val_if_fail(timeout_milliseconds >= 0 || timeout_milliseconds == -1, FALSE);
  if (pending_return) *pending_return = NULL;
  CONNECTION_LOCK (connection);
#ifdef HAVE_UNIX_FD_PASSING
  if (!_dbus_transport_can_pass_unix_fd(connection->transport) && message->n_unix_fds > 0) {
      CONNECTION_UNLOCK(connection);
      return TRUE;
  }
#endif
  if (!_dbus_connection_get_is_connected_unlocked(connection)) {
      CONNECTION_UNLOCK(connection);
      return TRUE;
  }
  pending = _dbus_pending_call_new_unlocked(connection, timeout_milliseconds, reply_handler_timeout);
  if (pending == NULL) {
      CONNECTION_UNLOCK(connection);
      return FALSE;
  }
  serial = dbus_message_get_serial(message);
  if (serial == 0) {
      serial = _dbus_connection_get_next_client_serial(connection);
      dbus_message_set_serial(message, serial);
  }
  if (!_dbus_pending_call_set_timeout_error_unlocked(pending, message, serial)) goto error;
  if (!_dbus_connection_attach_pending_call_unlocked(connection, pending)) goto error;
  if (!_dbus_connection_send_unlocked_no_update(connection, message, NULL)) {
      _dbus_connection_detach_pending_call_and_unlock(connection, pending);
      goto error_unlocked;
  }
  if (pending_return) *pending_return = pending;
  else _dbus_connection_detach_pending_call_unlocked(connection, pending);
  status = _dbus_connection_get_dispatch_status_unlocked(connection);
  _dbus_connection_update_dispatch_status_and_unlock(connection, status);
  if (pending_return == NULL) dbus_pending_call_unref(pending);
  return TRUE;
error:
  CONNECTION_UNLOCK(connection);
error_unlocked:
  dbus_pending_call_unref(pending);
  return FALSE;
}
DBusMessage* dbus_connection_send_with_reply_and_block(DBusConnection *connection, DBusMessage *message, int timeout_milliseconds, DBusError *error) {
  DBusMessage *reply;
  DBusPendingCall *pending;
  _dbus_return_val_if_fail(connection != NULL, NULL);
  _dbus_return_val_if_fail(message != NULL, NULL);
  _dbus_return_val_if_fail(timeout_milliseconds >= 0 || timeout_milliseconds == -1, NULL);
  _dbus_return_val_if_error_is_set(error, NULL);
#ifdef HAVE_UNIX_FD_PASSING
  CONNECTION_LOCK(connection);
  if (!_dbus_transport_can_pass_unix_fd(connection->transport) && message->n_unix_fds > 0) {
      CONNECTION_UNLOCK(connection);
      dbus_set_error(error, DBUS_ERROR_FAILED, "Cannot send file descriptors on this connection.");
      return NULL;
  }
  CONNECTION_UNLOCK(connection);
#endif
  if (!dbus_connection_send_with_reply(connection, message, &pending, timeout_milliseconds)) {
      _DBUS_SET_OOM(error);
      return NULL;
  }
  if (pending == NULL) {
      dbus_set_error(error, DBUS_ERROR_DISCONNECTED, "Connection is closed");
      return NULL;
  }
  dbus_pending_call_block(pending);
  reply = dbus_pending_call_steal_reply(pending);
  dbus_pending_call_unref(pending);
  _dbus_assert(reply != NULL);
  if (dbus_set_error_from_message(error, reply)) {
      dbus_message_unref(reply);
      return NULL;
  } else return reply;
}
static DBusDispatchStatus _dbus_connection_flush_unlocked(DBusConnection *connection) {
  DBusDispatchStatus status;
  HAVE_LOCK_CHECK(connection);
  while(connection->n_outgoing > 0 && _dbus_connection_get_is_connected_unlocked(connection)) {
      _dbus_verbose("doing iteration in\n");
      HAVE_LOCK_CHECK(connection);
      _dbus_connection_do_iteration_unlocked(connection,NULL,DBUS_ITERATION_DO_READING | DBUS_ITERATION_DO_WRITING | DBUS_ITERATION_BLOCK,
                                             -1);
  }
  HAVE_LOCK_CHECK(connection);
  _dbus_verbose("middle\n");
  status = _dbus_connection_get_dispatch_status_unlocked(connection);
  HAVE_LOCK_CHECK(connection);
  return status;
}
void dbus_connection_flush(DBusConnection *connection) {
  DBusDispatchStatus status;
  _dbus_return_if_fail(connection != NULL);
  CONNECTION_LOCK(connection);
  status = _dbus_connection_flush_unlocked(connection);
  HAVE_LOCK_CHECK(connection);
  _dbus_connection_update_dispatch_status_and_unlock(connection, status);
  _dbus_verbose("end\n");
}
static dbus_bool_t _dbus_connection_read_write_dispatch(DBusConnection *connection, int timeout_milliseconds, dbus_bool_t dispatch) {
  DBusDispatchStatus dstatus;
  dbus_bool_t progress_possible;
  dbus_connection_ref(connection);
  dstatus = dbus_connection_get_dispatch_status(connection);
  if (dispatch && dstatus == DBUS_DISPATCH_DATA_REMAINS) {
      _dbus_verbose("doing dispatch\n");
      dbus_connection_dispatch(connection);
      CONNECTION_LOCK(connection);
  } else if (dstatus == DBUS_DISPATCH_NEED_MEMORY) {
      _dbus_verbose("pausing for memory\n");
      _dbus_memory_pause_based_on_timeout(timeout_milliseconds);
      CONNECTION_LOCK(connection);
  } else {
      CONNECTION_LOCK(connection);
      if (_dbus_connection_get_is_connected_unlocked(connection)) {
          _dbus_verbose("doing iteration\n");
          _dbus_connection_do_iteration_unlocked(connection,NULL,DBUS_ITERATION_DO_READING | DBUS_ITERATION_DO_WRITING | DBUS_ITERATION_BLOCK,
                                                 timeout_milliseconds);
      }
  }
  HAVE_LOCK_CHECK(connection);
  if (dispatch) progress_possible = connection->n_incoming != 0 || connection->disconnect_message_link != NULL;
  else progress_possible = _dbus_connection_get_is_connected_unlocked(connection);
  CONNECTION_UNLOCK(connection);
  dbus_connection_unref(connection);
  return progress_possible;
}
dbus_bool_t dbus_connection_read_write_dispatch(DBusConnection *connection, int timeout_milliseconds) {
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(timeout_milliseconds >= 0 || timeout_milliseconds == -1, FALSE);
   return _dbus_connection_read_write_dispatch(connection, timeout_milliseconds, TRUE);
}
dbus_bool_t dbus_connection_read_write(DBusConnection *connection, int timeout_milliseconds) {
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(timeout_milliseconds >= 0 || timeout_milliseconds == -1, FALSE);
   return _dbus_connection_read_write_dispatch(connection, timeout_milliseconds, FALSE);
}
static void check_disconnected_message_arrived_unlocked(DBusConnection *connection, DBusMessage *head_of_queue) {
  HAVE_LOCK_CHECK(connection);
  if (connection->disconnect_message_link == NULL && dbus_message_is_signal(head_of_queue, DBUS_INTERFACE_LOCAL,"Disconnected")) {
      connection->disconnected_message_arrived = TRUE;
  }
}
DBusMessage* dbus_connection_borrow_message(DBusConnection *connection) {
  DBusDispatchStatus status;
  DBusMessage *message;
  _dbus_return_val_if_fail(connection != NULL, NULL);
  _dbus_verbose("start\n");
  status = dbus_connection_get_dispatch_status(connection);
  if (status != DBUS_DISPATCH_DATA_REMAINS) return NULL;
  CONNECTION_LOCK(connection);
  _dbus_connection_acquire_dispatch(connection);
  _dbus_assert(connection->message_borrowed == NULL);
  connection->message_borrowed = _dbus_list_get_first(&connection->incoming_messages);
  message = connection->message_borrowed;
  check_disconnected_message_arrived_unlocked(connection, message);
  if (message == NULL) _dbus_connection_release_dispatch(connection);
  CONNECTION_UNLOCK(connection);
  _dbus_message_trace_ref(message, -1, -1, "dbus_connection_borrow_message");
  return message;
}
void dbus_connection_return_message(DBusConnection *connection, DBusMessage *message) {
  DBusDispatchStatus status;
  _dbus_return_if_fail(connection != NULL);
  _dbus_return_if_fail(message != NULL);
  _dbus_return_if_fail(message == connection->message_borrowed);
  _dbus_return_if_fail(connection->dispatch_acquired);
  CONNECTION_LOCK(connection);
  _dbus_assert(message == connection->message_borrowed);
  connection->message_borrowed = NULL;
  _dbus_connection_release_dispatch(connection);
  status = _dbus_connection_get_dispatch_status_unlocked(connection);
  _dbus_connection_update_dispatch_status_and_unlock(connection, status);
  _dbus_message_trace_ref(message, -1, -1, "dbus_connection_return_message");
}
void dbus_connection_steal_borrowed_message(DBusConnection *connection, DBusMessage *message) {
  DBusMessage *pop_message;
  DBusDispatchStatus status;
  _dbus_return_if_fail(connection != NULL);
  _dbus_return_if_fail(message != NULL);
  _dbus_return_if_fail(message == connection->message_borrowed);
  _dbus_return_if_fail(connection->dispatch_acquired);
  CONNECTION_LOCK(connection);
  _dbus_assert(message == connection->message_borrowed);
  pop_message = _dbus_list_pop_first(&connection->incoming_messages);
  _dbus_assert(message == pop_message);
  (void)pop_message;
  connection->n_incoming -= 1;
  _dbus_verbose("Incoming message %p stolen from queue, %d incoming\n", message, connection->n_incoming);
  connection->message_borrowed = NULL;
  _dbus_connection_release_dispatch(connection);
  status = _dbus_connection_get_dispatch_status_unlocked(connection);
  _dbus_connection_update_dispatch_status_and_unlock(connection, status);
  _dbus_message_trace_ref(message, -1, -1,"dbus_connection_steal_borrowed_message");
}
static DBusList* _dbus_connection_pop_message_link_unlocked(DBusConnection *connection) {
  HAVE_LOCK_CHECK(connection);
  _dbus_assert(connection->message_borrowed == NULL);
  if (connection->n_incoming > 0) {
      DBusList *link;
      link = _dbus_list_pop_first_link(&connection->incoming_messages);
      connection->n_incoming -= 1;
      _dbus_verbose("Message %p (%s %s %s %s sig:'%s' serial:%u) removed from incoming queue %p, %d incoming\n", link->data,
                    dbus_message_type_to_string(dbus_message_get_type(link->data)), dbus_message_get_path(link->data) ? dbus_message_get_path(link->data) :
                    "no path", dbus_message_get_interface(link->data) ? dbus_message_get_interface(link->data) : "no interface",
                    dbus_message_get_member(link->data) ? dbus_message_get_member(link->data) : "no member", dbus_message_get_signature(link->data),
                    dbus_message_get_serial(link->data), connection, connection->n_incoming);
      _dbus_message_trace_ref(link->data, -1, -1,"_dbus_connection_pop_message_link_unlocked");
      check_disconnected_message_arrived_unlocked(connection, link->data);
      return link;
  } else return NULL;
}
static DBusMessage* _dbus_connection_pop_message_unlocked(DBusConnection *connection) {
  DBusList *link;
  HAVE_LOCK_CHECK(connection);
  link = _dbus_connection_pop_message_link_unlocked(connection);
  if (link != NULL) {
      DBusMessage *message;
      message = link->data;
      _dbus_list_free_link(link);
      return message;
  } else return NULL;
}
static void _dbus_connection_putback_message_link_unlocked(DBusConnection *connection, DBusList *message_link) {
  HAVE_LOCK_CHECK(connection);
  _dbus_assert(message_link != NULL);
  _dbus_assert(connection->message_borrowed == NULL);
  _dbus_assert(connection->dispatch_acquired);
  _dbus_list_prepend_link(&connection->incoming_messages, message_link);
  connection->n_incoming += 1;
  _dbus_verbose("Message %p (%s %s %s '%s') put back into queue %p, %d incoming\n", message_link->data,
                dbus_message_type_to_string(dbus_message_get_type(message_link->data)), dbus_message_get_interface(message_link->data) ?
                dbus_message_get_interface(message_link->data) : "no interface", dbus_message_get_member(message_link->data) ?
                dbus_message_get_member(message_link->data) : "no member", dbus_message_get_signature(message_link->data), connection, connection->n_incoming);
  _dbus_message_trace_ref(message_link->data, -1, -1,"_dbus_connection_putback_message_link_unlocked");
}
DBusMessage* dbus_connection_pop_message(DBusConnection *connection) {
  DBusMessage *message;
  DBusDispatchStatus status;
  _dbus_verbose("start\n");
  status = dbus_connection_get_dispatch_status(connection);
  if (status != DBUS_DISPATCH_DATA_REMAINS) return NULL;
  CONNECTION_LOCK(connection);
  _dbus_connection_acquire_dispatch(connection);
  HAVE_LOCK_CHECK(connection);
  message = _dbus_connection_pop_message_unlocked(connection);
  _dbus_verbose("Returning popped message %p\n", message);
  _dbus_connection_release_dispatch(connection);
  status = _dbus_connection_get_dispatch_status_unlocked (connection);
  _dbus_connection_update_dispatch_status_and_unlock (connection, status);
  return message;
}
static void _dbus_connection_acquire_dispatch(DBusConnection *connection) {
  HAVE_LOCK_CHECK(connection);
  _dbus_connection_ref_unlocked(connection);
  CONNECTION_UNLOCK(connection);
  _dbus_verbose ("locking dispatch_mutex\n");
  _dbus_cmutex_lock (connection->dispatch_mutex);
  while (connection->dispatch_acquired) {
      _dbus_verbose ("waiting for dispatch to be acquirable\n");
      _dbus_condvar_wait (connection->dispatch_cond, connection->dispatch_mutex);
  }
  _dbus_assert (!connection->dispatch_acquired);
  connection->dispatch_acquired = TRUE;
  _dbus_verbose ("unlocking dispatch_mutex\n");
  _dbus_cmutex_unlock (connection->dispatch_mutex);
  CONNECTION_LOCK (connection);
  _dbus_connection_unref_unlocked (connection);
}
static void _dbus_connection_release_dispatch(DBusConnection *connection) {
  HAVE_LOCK_CHECK (connection);
  _dbus_verbose ("locking dispatch_mutex\n");
  _dbus_cmutex_lock (connection->dispatch_mutex);
  _dbus_assert (connection->dispatch_acquired);
  connection->dispatch_acquired = FALSE;
  _dbus_condvar_wake_one(connection->dispatch_cond);
  _dbus_verbose("unlocking dispatch_mutex\n");
  _dbus_cmutex_unlock(connection->dispatch_mutex);
}
static void _dbus_connection_failed_pop(DBusConnection *connection, DBusList *message_link) {
  _dbus_list_prepend_link(&connection->incoming_messages, message_link);
  connection->n_incoming += 1;
}
static void notify_disconnected_unlocked(DBusConnection *connection) {
  HAVE_LOCK_CHECK(connection);
  _dbus_bus_notify_shared_connection_disconnected_unlocked(connection);
  if (connection->n_outgoing > 0) {
      DBusList *link;
      _dbus_verbose("Dropping %d outgoing messages since we're disconnected\n", connection->n_outgoing);
      while((link = _dbus_list_get_last_link(&connection->outgoing_messages))) _dbus_connection_message_sent_unlocked(connection, link->data);
  }
}
static DBusDispatchStatus notify_disconnected_and_dispatch_complete_unlocked(DBusConnection *connection) {
  HAVE_LOCK_CHECK(connection);
  if (connection->disconnect_message_link != NULL) {
      _dbus_verbose("Sending disconnect message\n");
      connection_timeout_and_complete_all_pending_calls_unlocked(connection);
      _dbus_connection_queue_synthesized_message_link(connection, connection->disconnect_message_link);
      connection->disconnect_message_link = NULL;
      return DBUS_DISPATCH_DATA_REMAINS;
  }
  return DBUS_DISPATCH_COMPLETE;
}
static DBusDispatchStatus _dbus_connection_get_dispatch_status_unlocked (DBusConnection *connection) {
  HAVE_LOCK_CHECK(connection);
  if (connection->n_incoming > 0) return DBUS_DISPATCH_DATA_REMAINS;
  else if (!_dbus_transport_queue_messages(connection->transport)) return DBUS_DISPATCH_NEED_MEMORY;
  else {
      DBusDispatchStatus status;
      dbus_bool_t is_connected;
      status = _dbus_transport_get_dispatch_status(connection->transport);
      is_connected = _dbus_transport_get_is_connected(connection->transport);
      _dbus_verbose("dispatch status = %s is_connected = %d\n", DISPATCH_STATUS_NAME(status), is_connected);
      if (!is_connected) {
          notify_disconnected_unlocked(connection);
          if (status == DBUS_DISPATCH_COMPLETE) status = notify_disconnected_and_dispatch_complete_unlocked(connection);
      }
      if (status != DBUS_DISPATCH_COMPLETE) return status;
      else if (connection->n_incoming > 0) return DBUS_DISPATCH_DATA_REMAINS;
      else return DBUS_DISPATCH_COMPLETE;
  }
}
static void _dbus_connection_update_dispatch_status_and_unlock(DBusConnection *connection, DBusDispatchStatus new_status) {
  dbus_bool_t changed;
  DBusDispatchStatusFunction function;
  void *data;
  HAVE_LOCK_CHECK(connection);
  _dbus_connection_ref_unlocked(connection);
  changed = new_status != connection->last_dispatch_status;
  connection->last_dispatch_status = new_status;
  function = connection->dispatch_status_function;
  data = connection->dispatch_status_data;
  if (connection->disconnected_message_arrived && !connection->disconnected_message_processed) {
      connection->disconnected_message_processed = TRUE;
      connection_forget_shared_unlocked(connection);
      if (connection->exit_on_disconnect) {
          CONNECTION_UNLOCK(connection);
          _dbus_verbose("Exiting on Disconnected signal\n");
          _dbus_exit(1);
          _dbus_assert_not_reached("Call to exit() returned");
      }
  }
  CONNECTION_UNLOCK (connection);
  if (changed && function) {
      _dbus_verbose("Notifying of change to dispatch status of %p now %d (%s)\n", connection, new_status, DISPATCH_STATUS_NAME(new_status));
      (*function)(connection, new_status, data);
  }
  dbus_connection_unref (connection);
}
DBusDispatchStatus dbus_connection_get_dispatch_status(DBusConnection *connection) {
  DBusDispatchStatus status;
  _dbus_return_val_if_fail(connection != NULL, DBUS_DISPATCH_COMPLETE);
  _dbus_verbose("start\n");
  CONNECTION_LOCK(connection);
  status = _dbus_connection_get_dispatch_status_unlocked(connection);
  CONNECTION_UNLOCK(connection);
  return status;
}
static DBusHandlerResult _dbus_connection_peer_filter_unlocked_no_update(DBusConnection *connection, DBusMessage *message) {
  dbus_bool_t sent = FALSE;
  DBusMessage *ret = NULL;
  DBusList *expire_link;
  if (connection->route_peer_messages && dbus_message_get_destination(message) != NULL) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  if (!dbus_message_has_interface(message, DBUS_INTERFACE_PEER)) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  expire_link = _dbus_list_alloc_link(NULL);
  if (!expire_link) return DBUS_HANDLER_RESULT_NEED_MEMORY;
  if (dbus_message_is_method_call(message, DBUS_INTERFACE_PEER, "Ping")) {
      ret = dbus_message_new_method_return(message);
      if (ret == NULL) goto out;
      sent = _dbus_connection_send_unlocked_no_update(connection, ret, NULL);
  } else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PEER, "GetMachineId")) {
      DBusString uuid;
      DBusError error = DBUS_ERROR_INIT;
      if (!_dbus_string_init(&uuid)) goto out;
      if (_dbus_get_local_machine_uuid_encoded(&uuid, &error)) {
          const char *v_STRING;
          ret = dbus_message_new_method_return(message);
          if (ret == NULL) {
              _dbus_string_free(&uuid);
              goto out;
          }
          v_STRING = _dbus_string_get_const_data(&uuid);
          if (dbus_message_append_args(ret, DBUS_TYPE_STRING, &v_STRING, DBUS_TYPE_INVALID))
              sent = _dbus_connection_send_unlocked_no_update(connection, ret, NULL);
      } else if (dbus_error_has_name(&error, DBUS_ERROR_NO_MEMORY)) {
          dbus_error_free(&error);
          goto out;
      } else {
          ret = dbus_message_new_error(message, error.name, error.message);
          dbus_error_free(&error);
          if (ret == NULL) goto out;
          sent = _dbus_connection_send_unlocked_no_update(connection, ret, NULL);
      }
      _dbus_string_free(&uuid);
  } else {
      ret = dbus_message_new_error(message, DBUS_ERROR_UNKNOWN_METHOD,"Unknown method invoked on org.freedesktop.DBus.Peer interface");
      if (ret == NULL) goto out;
      sent = _dbus_connection_send_unlocked_no_update(connection, ret, NULL);
  }
out:
  if (ret == NULL) _dbus_list_free_link(expire_link);
  else {
      expire_link->data = ret;
      _dbus_list_prepend_link(&connection->expired_messages, expire_link);
  }
  if (!sent) return DBUS_HANDLER_RESULT_NEED_MEMORY;
  return DBUS_HANDLER_RESULT_HANDLED;
}
static DBusHandlerResult _dbus_connection_run_builtin_filters_unlocked_no_update(DBusConnection *connection, DBusMessage *message) {
  return _dbus_connection_peer_filter_unlocked_no_update(connection, message);
}
DBusDispatchStatus dbus_connection_dispatch(DBusConnection *connection) {
  DBusMessage *message;
  DBusList *link, *filter_list_copy, *message_link;
  DBusHandlerResult result;
  DBusPendingCall *pending;
  dbus_int32_t reply_serial;
  DBusDispatchStatus status;
  dbus_bool_t found_object;
  _dbus_return_val_if_fail(connection != NULL, DBUS_DISPATCH_COMPLETE);
  _dbus_verbose("\n");
  CONNECTION_LOCK(connection);
  status = _dbus_connection_get_dispatch_status_unlocked(connection);
  if (status != DBUS_DISPATCH_DATA_REMAINS) {
      _dbus_connection_update_dispatch_status_and_unlock(connection, status);
      return status;
  }
  _dbus_connection_ref_unlocked(connection);
  _dbus_connection_acquire_dispatch(connection);
  HAVE_LOCK_CHECK(connection);
  message_link = _dbus_connection_pop_message_link_unlocked(connection);
  if (message_link == NULL) {
      _dbus_verbose("another thread dispatched message (during acquire_dispatch above)\n");
      _dbus_connection_release_dispatch(connection);
      status = _dbus_connection_get_dispatch_status_unlocked(connection);
      _dbus_connection_update_dispatch_status_and_unlock(connection, status);
      dbus_connection_unref(connection);
      return status;
  }
  message = message_link->data;
  _dbus_verbose(" dispatching message %p (%s %s %s '%s')\n", message, dbus_message_type_to_string(dbus_message_get_type(message)),
                dbus_message_get_interface(message) ? dbus_message_get_interface(message) : "no interface", dbus_message_get_member(message) ?
                dbus_message_get_member(message) : "no member", dbus_message_get_signature(message));
  result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  reply_serial = dbus_message_get_reply_serial(message);
  pending = _dbus_hash_table_lookup_int(connection->pending_replies, reply_serial);
  if (pending) {
      _dbus_verbose("Dispatching a pending reply\n");
      complete_pending_call_and_unlock(connection, pending, message);
      pending = NULL;
      CONNECTION_LOCK(connection);
      _dbus_verbose("pending call completed in dispatch\n");
      result = DBUS_HANDLER_RESULT_HANDLED;
      goto out;
  }
  result = _dbus_connection_run_builtin_filters_unlocked_no_update(connection, message);
  if (result != DBUS_HANDLER_RESULT_NOT_YET_HANDLED) goto out;
  if (!_dbus_list_copy(&connection->filter_list, &filter_list_copy)) {
      _dbus_connection_release_dispatch(connection);
      HAVE_LOCK_CHECK(connection);
      _dbus_connection_failed_pop(connection, message_link);
      _dbus_connection_update_dispatch_status_and_unlock(connection,DBUS_DISPATCH_NEED_MEMORY);
      dbus_connection_unref(connection);
      return DBUS_DISPATCH_NEED_MEMORY;
  }
  _dbus_list_foreach(&filter_list_copy, (DBusForeachFunction)_dbus_message_filter_ref,NULL);
  CONNECTION_UNLOCK(connection);
  link = _dbus_list_get_first_link(&filter_list_copy);
  while(link != NULL) {
      DBusMessageFilter *filter = link->data;
      DBusList *next = _dbus_list_get_next_link(&filter_list_copy, link);
      if (filter->function == NULL) {
          _dbus_verbose("  filter was removed in a callback function\n");
          link = next;
          continue;
      }
      _dbus_verbose("  running filter on message %p\n", message);
      result = (*filter->function)(connection, message, filter->user_data);
      if (result != DBUS_HANDLER_RESULT_NOT_YET_HANDLED) break;
      link = next;
  }
  _dbus_list_foreach(&filter_list_copy, (DBusForeachFunction)_dbus_message_filter_unref,NULL);
  _dbus_list_clear(&filter_list_copy);
  CONNECTION_LOCK(connection);
  if (result == DBUS_HANDLER_RESULT_NEED_MEMORY) {
      _dbus_verbose("No memory\n");
      goto out;
  } else if (result == DBUS_HANDLER_RESULT_HANDLED) {
      _dbus_verbose("filter handled message in dispatch\n");
      goto out;
  }
  _dbus_verbose("  running object path dispatch on message %p (%s %s %s '%s')\n", message, dbus_message_type_to_string(dbus_message_get_type (message)),
                dbus_message_get_interface(message) ? dbus_message_get_interface(message) : "no interface", dbus_message_get_member(message) ?
                dbus_message_get_member(message) : "no member", dbus_message_get_signature(message));
  HAVE_LOCK_CHECK(connection);
  result = _dbus_object_tree_dispatch_and_unlock(connection->objects, message, &found_object);
  CONNECTION_LOCK(connection);
  if (result != DBUS_HANDLER_RESULT_NOT_YET_HANDLED) {
      _dbus_verbose("object tree handled message in dispatch\n");
      goto out;
  }
  if (dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_METHOD_CALL) {
      DBusMessage *reply;
      DBusString str;
      DBusPreallocatedSend *preallocated;
      DBusList *expire_link;
      _dbus_verbose("  sending error %s\n", DBUS_ERROR_UNKNOWN_METHOD);
      if (!_dbus_string_init(&str)) {
          result = DBUS_HANDLER_RESULT_NEED_MEMORY;
          _dbus_verbose("no memory for error string in dispatch\n");
          goto out;
      }
      if (!_dbus_string_append_printf(&str,"Method \"%s\" with signature \"%s\" on interface \"%s\" doesn't exist\n", dbus_message_get_member(message),
          dbus_message_get_signature(message), dbus_message_get_interface(message))) {
          _dbus_string_free(&str);
          result = DBUS_HANDLER_RESULT_NEED_MEMORY;
          _dbus_verbose("no memory for error string in dispatch\n");
          goto out;
      }
      reply = dbus_message_new_error(message,found_object ? DBUS_ERROR_UNKNOWN_METHOD : DBUS_ERROR_UNKNOWN_OBJECT,
                                     _dbus_string_get_const_data(&str));
      _dbus_string_free(&str);
      if (reply == NULL) {
          result = DBUS_HANDLER_RESULT_NEED_MEMORY;
          _dbus_verbose("no memory for error reply in dispatch\n");
          goto out;
      }
      expire_link = _dbus_list_alloc_link(reply);
      if (expire_link == NULL) {
          dbus_message_unref(reply);
          result = DBUS_HANDLER_RESULT_NEED_MEMORY;
          _dbus_verbose("no memory for error send in dispatch\n");
          goto out;
      }
      preallocated = _dbus_connection_preallocate_send_unlocked(connection);
      if (preallocated == NULL) {
          _dbus_list_free_link(expire_link);
          dbus_message_unref(reply);
          result = DBUS_HANDLER_RESULT_NEED_MEMORY;
          _dbus_verbose("no memory for error send in dispatch\n");
          goto out;
      }
      _dbus_connection_send_preallocated_unlocked_no_update(connection, preallocated, reply, NULL);
      _dbus_list_prepend_link(&connection->expired_messages, expire_link);
      result = DBUS_HANDLER_RESULT_HANDLED;
  }
  _dbus_verbose("  done dispatching %p (%s %s %s '%s') on connection %p\n", message, dbus_message_type_to_string(dbus_message_get_type(message)),
                dbus_message_get_interface(message) ? dbus_message_get_interface(message) : "no interface", dbus_message_get_member(message) ?
                dbus_message_get_member(message) : "no member", dbus_message_get_signature(message), connection);
out:
  if (result == DBUS_HANDLER_RESULT_NEED_MEMORY)    {
      _dbus_verbose("out of memory\n");
      _dbus_connection_putback_message_link_unlocked(connection, message_link);
      message_link = NULL;
      message = NULL;
  } else { _dbus_verbose(" ... done dispatching\n"); }
  _dbus_connection_release_dispatch(connection);
  HAVE_LOCK_CHECK(connection);
  if (message != NULL) {
      CONNECTION_UNLOCK(connection);
      dbus_message_unref(message);
      CONNECTION_LOCK(connection);
  }
  if (message_link != NULL) _dbus_list_free_link(message_link);
  _dbus_verbose("before final status update\n");
  status = _dbus_connection_get_dispatch_status_unlocked(connection);
  _dbus_connection_update_dispatch_status_and_unlock(connection, status);
  dbus_connection_unref(connection);
  return status;
}
dbus_bool_t dbus_connection_set_watch_functions(DBusConnection *connection, DBusAddWatchFunction add_function, DBusRemoveWatchFunction remove_function,
                                                DBusWatchToggledFunction toggled_function, void *data, DBusFreeFunction free_data_function) {
  dbus_bool_t retval;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  CONNECTION_LOCK(connection);
  retval = _dbus_watch_list_set_functions(connection->watches, add_function, remove_function, toggled_function, data, free_data_function);
  CONNECTION_UNLOCK(connection);
  return retval;
}
dbus_bool_t dbus_connection_set_timeout_functions(DBusConnection *connection, DBusAddTimeoutFunction add_function, DBusRemoveTimeoutFunction remove_function,
                                                  DBusTimeoutToggledFunction toggled_function, void *data, DBusFreeFunction free_data_function) {
  dbus_bool_t retval;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  CONNECTION_LOCK(connection);
  retval = _dbus_timeout_list_set_functions(connection->timeouts, add_function, remove_function, toggled_function, data, free_data_function);
  CONNECTION_UNLOCK(connection);
  return retval;
}
void dbus_connection_set_wakeup_main_function(DBusConnection *connection, DBusWakeupMainFunction wakeup_main_function, void *data,
					                          DBusFreeFunction free_data_function) {
  void *old_data;
  DBusFreeFunction old_free_data;
  _dbus_return_if_fail(connection != NULL);
  CONNECTION_LOCK(connection);
  old_data = connection->wakeup_main_data;
  old_free_data = connection->free_wakeup_main_data;
  connection->wakeup_main_function = wakeup_main_function;
  connection->wakeup_main_data = data;
  connection->free_wakeup_main_data = free_data_function;
  CONNECTION_UNLOCK(connection);
  if (old_free_data) (*old_free_data)(old_data);
}
void dbus_connection_set_dispatch_status_function(DBusConnection *connection, DBusDispatchStatusFunction function, void *data,
                                                  DBusFreeFunction free_data_function) {
  void *old_data;
  DBusFreeFunction old_free_data;
  _dbus_return_if_fail(connection != NULL);
  CONNECTION_LOCK(connection);
  old_data = connection->dispatch_status_data;
  old_free_data = connection->free_dispatch_status_data;
  connection->dispatch_status_function = function;
  connection->dispatch_status_data = data;
  connection->free_dispatch_status_data = free_data_function;
  CONNECTION_UNLOCK(connection);
  if (old_free_data) (*old_free_data)(old_data);
}
dbus_bool_t dbus_connection_get_unix_fd(DBusConnection *connection, int *fd) {
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(connection->transport != NULL, FALSE);
#ifdef DBUS_WIN
  return FALSE;
#endif
  return dbus_connection_get_socket(connection, fd);
}
dbus_bool_t dbus_connection_get_socket(DBusConnection *connection, int *fd) {
  dbus_bool_t retval;
  DBusSocket s = DBUS_SOCKET_INIT;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(connection->transport != NULL, FALSE);
  CONNECTION_LOCK(connection);
  retval = _dbus_transport_get_socket_fd(connection->transport, &s);
  if (retval) *fd = _dbus_socket_get_int(s);
  CONNECTION_UNLOCK(connection);
  return retval;
}
dbus_bool_t dbus_connection_get_unix_user(DBusConnection *connection, unsigned long *uid) {
  dbus_bool_t result;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(uid != NULL, FALSE);
  CONNECTION_LOCK(connection);
  if (!_dbus_transport_try_to_authenticate(connection->transport)) result = FALSE;
  else result = _dbus_transport_get_unix_user(connection->transport, uid);
#ifdef DBUS_WIN
  _dbus_assert(!result);
#endif
  CONNECTION_UNLOCK(connection);
  return result;
}
dbus_bool_t dbus_connection_get_unix_process_id(DBusConnection *connection, unsigned long *pid) {
  dbus_bool_t result;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(pid != NULL, FALSE);
  CONNECTION_LOCK(connection);
  if (!_dbus_transport_try_to_authenticate(connection->transport)) result = FALSE;
  else result = _dbus_transport_get_unix_process_id(connection->transport, pid);
  CONNECTION_UNLOCK(connection);
  return result;
}
dbus_bool_t dbus_connection_get_adt_audit_session_data(DBusConnection *connection, void **data, dbus_int32_t *data_size) {
  dbus_bool_t result;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(data != NULL, FALSE);
  _dbus_return_val_if_fail(data_size != NULL, FALSE);
  CONNECTION_LOCK(connection);
  if (!_dbus_transport_try_to_authenticate(connection->transport)) result = FALSE;
  else result = _dbus_transport_get_adt_audit_session_data(connection->transport, data, data_size);
  CONNECTION_UNLOCK(connection);
  return result;
}
void dbus_connection_set_unix_user_function(DBusConnection *connection, DBusAllowUnixUserFunction function, void *data, DBusFreeFunction free_data_function) {
  void *old_data = NULL;
  DBusFreeFunction old_free_function = NULL;
  _dbus_return_if_fail(connection != NULL);
  CONNECTION_LOCK(connection);
  _dbus_transport_set_unix_user_function(connection->transport, function, data, free_data_function, &old_data, &old_free_function);
  CONNECTION_UNLOCK(connection);
  if (old_free_function != NULL) (*old_free_function)(old_data);
}
dbus_bool_t _dbus_connection_get_linux_security_label(DBusConnection *connection, char **label_p) {
  dbus_bool_t result;
  _dbus_assert(connection != NULL);
  _dbus_assert(label_p != NULL);
  CONNECTION_LOCK(connection);
  if (!_dbus_transport_try_to_authenticate(connection->transport)) result = FALSE;
  else result = _dbus_transport_get_linux_security_label(connection->transport, label_p);
#ifndef __linux__
  _dbus_assert(!result);
#endif
  CONNECTION_UNLOCK(connection);
  return result;
}
dbus_bool_t dbus_connection_get_windows_user(DBusConnection *connection, char **windows_sid_p) {
  dbus_bool_t result;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(windows_sid_p != NULL, FALSE);
  CONNECTION_LOCK(connection);
  if (!_dbus_transport_try_to_authenticate(connection->transport)) result = FALSE;
  else result = _dbus_transport_get_windows_user(connection->transport, windows_sid_p);
#ifdef DBUS_UNIX
  _dbus_assert(!result);
#endif
  CONNECTION_UNLOCK(connection);
  return result;
}
void dbus_connection_set_windows_user_function(DBusConnection *connection, DBusAllowWindowsUserFunction function, void *data, DBusFreeFunction free_data_function) {
  void *old_data = NULL;
  DBusFreeFunction old_free_function = NULL;
  _dbus_return_if_fail(connection != NULL);
  CONNECTION_LOCK(connection);
  _dbus_transport_set_windows_user_function(connection->transport, function, data, free_data_function, &old_data, &old_free_function);
  CONNECTION_UNLOCK(connection);
  if (old_free_function != NULL) (*old_free_function)(old_data);
}
void dbus_connection_set_allow_anonymous(DBusConnection *connection, dbus_bool_t value) {
  _dbus_return_if_fail(connection != NULL);
  CONNECTION_LOCK(connection);
  _dbus_transport_set_allow_anonymous(connection->transport, value);
  CONNECTION_UNLOCK(connection);
}
void dbus_connection_set_route_peer_messages(DBusConnection *connection, dbus_bool_t value) {
  _dbus_return_if_fail(connection != NULL);
  CONNECTION_LOCK(connection);
  connection->route_peer_messages = value;
  CONNECTION_UNLOCK(connection);
}
dbus_bool_t dbus_connection_add_filter(DBusConnection *connection, DBusHandleMessageFunction function, void *user_data, DBusFreeFunction free_data_function) {
  DBusMessageFilter *filter;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(function != NULL, FALSE);
  filter = dbus_new0(DBusMessageFilter, 1);
  if (filter == NULL) return FALSE;
  _dbus_atomic_inc(&filter->refcount);
  CONNECTION_LOCK(connection);
  if (!_dbus_list_append(&connection->filter_list, filter)) {
      _dbus_message_filter_unref(filter);
      CONNECTION_UNLOCK(connection);
      return FALSE;
  }
  filter->function = function;
  filter->user_data = user_data;
  filter->free_user_data_function = free_data_function;
  CONNECTION_UNLOCK(connection);
  return TRUE;
}
void dbus_connection_remove_filter(DBusConnection *connection, DBusHandleMessageFunction function, void *user_data) {
  DBusList *link;
  DBusMessageFilter *filter;
  _dbus_return_if_fail(connection != NULL);
  _dbus_return_if_fail(function != NULL);
  CONNECTION_LOCK(connection);
  filter = NULL;
  link = _dbus_list_get_last_link(&connection->filter_list);
  while(link != NULL) {
      filter = link->data;
      if (filter->function == function && filter->user_data == user_data) {
          _dbus_list_remove_link(&connection->filter_list, link);
          filter->function = NULL;
          break;
      }
      link = _dbus_list_get_prev_link(&connection->filter_list, link);
      filter = NULL;
  }
  CONNECTION_UNLOCK(connection);
#ifndef DBUS_DISABLE_CHECKS
  if (filter == NULL) {
      _dbus_warn_check_failed("Attempt to remove filter function %p user data %p, but no such filter has been added", function, user_data);
      return;
  }
#endif
  if (filter->free_user_data_function) (*filter->free_user_data_function)(filter->user_data);
  filter->free_user_data_function = NULL;
  filter->user_data = NULL;
  _dbus_message_filter_unref(filter);
}
static dbus_bool_t _dbus_connection_register_object_path(DBusConnection *connection, dbus_bool_t fallback, const char *path, const DBusObjectPathVTable *vtable,
                                                         void *user_data, DBusError *error) {
  char **decomposed_path;
  dbus_bool_t retval;
  if (!_dbus_decompose_path(path, strlen(path), &decomposed_path, NULL)) return FALSE;
  CONNECTION_LOCK(connection);
  retval = _dbus_object_tree_register(connection->objects, fallback, (const char**)decomposed_path, vtable, user_data, error);
  CONNECTION_UNLOCK(connection);
  dbus_free_string_array(decomposed_path);
  return retval;
}
dbus_bool_t dbus_connection_try_register_object_path(DBusConnection *connection, const char *path, const DBusObjectPathVTable *vtable, void *user_data,
                                                     DBusError *error) {
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(path != NULL, FALSE);
  _dbus_return_val_if_fail(path[0] == '/', FALSE);
  _dbus_return_val_if_fail(vtable != NULL, FALSE);
  return _dbus_connection_register_object_path(connection, FALSE, path, vtable, user_data, error);
}
dbus_bool_t dbus_connection_register_object_path(DBusConnection *connection, const char *path, const DBusObjectPathVTable *vtable, void *user_data) {
  dbus_bool_t retval;
  DBusError error = DBUS_ERROR_INIT;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(path != NULL, FALSE);
  _dbus_return_val_if_fail(path[0] == '/', FALSE);
  _dbus_return_val_if_fail(vtable != NULL, FALSE);
  retval = _dbus_connection_register_object_path (connection, FALSE, path, vtable, user_data, &error);
  if (dbus_error_has_name(&error, DBUS_ERROR_OBJECT_PATH_IN_USE)) {
      _dbus_warn("%s", error.message);
      dbus_error_free(&error);
      return FALSE;
  }
  return retval;
}
dbus_bool_t dbus_connection_try_register_fallback(DBusConnection *connection, const char *path, const DBusObjectPathVTable *vtable, void *user_data,
                                                  DBusError *error) {
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(path != NULL, FALSE);
  _dbus_return_val_if_fail(path[0] == '/', FALSE);
  _dbus_return_val_if_fail(vtable != NULL, FALSE);
  return _dbus_connection_register_object_path(connection, TRUE, path, vtable, user_data, error);
}
dbus_bool_t dbus_connection_register_fallback(DBusConnection *connection, const char *path, const DBusObjectPathVTable *vtable, void *user_data) {
  dbus_bool_t retval;
  DBusError error = DBUS_ERROR_INIT;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(path != NULL, FALSE);
  _dbus_return_val_if_fail(path[0] == '/', FALSE);
  _dbus_return_val_if_fail(vtable != NULL, FALSE);
  retval = _dbus_connection_register_object_path(connection, TRUE, path, vtable, user_data, &error);
  if (dbus_error_has_name(&error, DBUS_ERROR_OBJECT_PATH_IN_USE)) {
      _dbus_warn("%s", error.message);
      dbus_error_free(&error);
      return FALSE;
  }
  return retval;
}
dbus_bool_t dbus_connection_unregister_object_path(DBusConnection *connection, const char *path) {
  char **decomposed_path;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(path != NULL, FALSE);
  _dbus_return_val_if_fail(path[0] == '/', FALSE);
  if (!_dbus_decompose_path(path, strlen (path), &decomposed_path, NULL)) return FALSE;
  CONNECTION_LOCK(connection);
  _dbus_object_tree_unregister_and_unlock(connection->objects, (const char**)decomposed_path);
  dbus_free_string_array(decomposed_path);
  return TRUE;
}
dbus_bool_t dbus_connection_get_object_path_data(DBusConnection *connection, const char *path, void **data_p) {
  char **decomposed_path;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(path != NULL, FALSE);
  _dbus_return_val_if_fail(data_p != NULL, FALSE);
  *data_p = NULL;
  if (!_dbus_decompose_path(path, strlen(path), &decomposed_path, NULL)) return FALSE;
  CONNECTION_LOCK(connection);
  *data_p = _dbus_object_tree_get_user_data_unlocked(connection->objects, (const char**)decomposed_path);
  CONNECTION_UNLOCK(connection);
  dbus_free_string_array(decomposed_path);
  return TRUE;
}
dbus_bool_t dbus_connection_list_registered(DBusConnection *connection, const char *parent_path, char ***child_entries) {
  char **decomposed_path;
  dbus_bool_t retval;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(parent_path != NULL, FALSE);
  _dbus_return_val_if_fail(parent_path[0] == '/', FALSE);
  _dbus_return_val_if_fail(child_entries != NULL, FALSE);
  if (!_dbus_decompose_path(parent_path, strlen(parent_path), &decomposed_path, NULL)) return FALSE;
  CONNECTION_LOCK(connection);
  retval = _dbus_object_tree_list_registered_and_unlock(connection->objects, (const char**)decomposed_path, child_entries);
  dbus_free_string_array(decomposed_path);
  return retval;
}
static DBusDataSlotAllocator slot_allocator = _DBUS_DATA_SLOT_ALLOCATOR_INIT(_DBUS_LOCK_NAME(connection_slots));
dbus_bool_t dbus_connection_allocate_data_slot(dbus_int32_t *slot_p) {
  return _dbus_data_slot_allocator_alloc(&slot_allocator, slot_p);
}
void dbus_connection_free_data_slot(dbus_int32_t *slot_p) {
  _dbus_return_if_fail(*slot_p >= 0);
  _dbus_data_slot_allocator_free(&slot_allocator, slot_p);
}
dbus_bool_t dbus_connection_set_data(DBusConnection *connection, dbus_int32_t slot, void *data, DBusFreeFunction free_data_func) {
  DBusFreeFunction old_free_func;
  void *old_data;
  dbus_bool_t retval;
  _dbus_return_val_if_fail(connection != NULL, FALSE);
  _dbus_return_val_if_fail(slot >= 0, FALSE);
  SLOTS_LOCK(connection);
  retval = _dbus_data_slot_list_set(&slot_allocator, &connection->slot_list, slot, data, free_data_func, &old_free_func, &old_data);
  SLOTS_UNLOCK(connection);
  if (retval) {
      if (old_free_func) (*old_free_func)(old_data);
  }
  return retval;
}
void* dbus_connection_get_data(DBusConnection *connection, dbus_int32_t slot) {
  void *res;
  _dbus_return_val_if_fail(connection != NULL, NULL);
  _dbus_return_val_if_fail(slot >= 0, NULL);
  SLOTS_LOCK(connection);
  res = _dbus_data_slot_list_get(&slot_allocator, &connection->slot_list, slot);
  SLOTS_UNLOCK(connection);
  return res;
}
void dbus_connection_set_change_sigpipe(dbus_bool_t will_modify_sigpipe) {
  _dbus_modify_sigpipe = will_modify_sigpipe != FALSE;
}
void dbus_connection_set_max_message_size(DBusConnection *connection, long size) {
  _dbus_return_if_fail(connection != NULL);
  CONNECTION_LOCK(connection);
  _dbus_transport_set_max_message_size(connection->transport, size);
  CONNECTION_UNLOCK(connection);
}
long dbus_connection_get_max_message_size(DBusConnection *connection) {
  long res;
  _dbus_return_val_if_fail(connection != NULL, 0);
  CONNECTION_LOCK(connection);
  res = _dbus_transport_get_max_message_size(connection->transport);
  CONNECTION_UNLOCK(connection);
  return res;
}
void dbus_connection_set_max_message_unix_fds(DBusConnection *connection, long n) {
  _dbus_return_if_fail(connection != NULL);
  CONNECTION_LOCK(connection);
  _dbus_transport_set_max_message_unix_fds(connection->transport, n);
  CONNECTION_UNLOCK(connection);
}
long dbus_connection_get_max_message_unix_fds(DBusConnection *connection) {
  long res;
  _dbus_return_val_if_fail(connection != NULL, 0);
  CONNECTION_LOCK(connection);
  res = _dbus_transport_get_max_message_unix_fds(connection->transport);
  CONNECTION_UNLOCK(connection);
  return res;
}
void dbus_connection_set_max_received_size(DBusConnection *connection, long size) {
  _dbus_return_if_fail(connection != NULL);
  CONNECTION_LOCK(connection);
  _dbus_transport_set_max_received_size(connection->transport, size);
  CONNECTION_UNLOCK(connection);
}
long dbus_connection_get_max_received_size(DBusConnection *connection) {
  long res;
  _dbus_return_val_if_fail(connection != NULL, 0);
  CONNECTION_LOCK(connection);
  res = _dbus_transport_get_max_received_size(connection->transport);
  CONNECTION_UNLOCK(connection);
  return res;
}
void dbus_connection_set_max_received_unix_fds(DBusConnection *connection, long n) {
  _dbus_return_if_fail(connection != NULL);
  CONNECTION_LOCK(connection);
  _dbus_transport_set_max_received_unix_fds(connection->transport, n);
  CONNECTION_UNLOCK(connection);
}
long dbus_connection_get_max_received_unix_fds(DBusConnection *connection) {
  long res;
  _dbus_return_val_if_fail(connection != NULL, 0);
  CONNECTION_LOCK(connection);
  res = _dbus_transport_get_max_received_unix_fds(connection->transport);
  CONNECTION_UNLOCK(connection);
  return res;
}
long dbus_connection_get_outgoing_size(DBusConnection *connection) {
  long res;
  _dbus_return_val_if_fail(connection != NULL, 0);
  CONNECTION_LOCK(connection);
  res = _dbus_counter_get_size_value(connection->outgoing_counter);
  CONNECTION_UNLOCK(connection);
  return res;
}
#ifdef DBUS_ENABLE_STATS
void _dbus_connection_get_stats(DBusConnection *connection, dbus_uint32_t *in_messages, dbus_uint32_t *in_bytes, dbus_uint32_t *in_fds,
                                dbus_uint32_t *in_peak_bytes, dbus_uint32_t *in_peak_fds, dbus_uint32_t *out_messages, dbus_uint32_t *out_bytes,
                                dbus_uint32_t *out_fds, dbus_uint32_t *out_peak_bytes, dbus_uint32_t *out_peak_fds) {
  CONNECTION_LOCK(connection);
  if (in_messages != NULL) *in_messages = connection->n_incoming;
  _dbus_transport_get_stats(connection->transport, in_bytes, in_fds, in_peak_bytes, in_peak_fds);
  if (out_messages != NULL) *out_messages = connection->n_outgoing;
  if (out_bytes != NULL) *out_bytes = _dbus_counter_get_size_value(connection->outgoing_counter);
  if (out_fds != NULL) *out_fds = _dbus_counter_get_unix_fd_value(connection->outgoing_counter);
  if (out_peak_bytes != NULL) *out_peak_bytes = _dbus_counter_get_peak_size_value(connection->outgoing_counter);
  if (out_peak_fds != NULL) *out_peak_fds = _dbus_counter_get_peak_unix_fd_value(connection->outgoing_counter);
  CONNECTION_UNLOCK(connection);
}
#endif
long dbus_connection_get_outgoing_unix_fds(DBusConnection *connection) {
  long res;
  _dbus_return_val_if_fail(connection != NULL, 0);
  CONNECTION_LOCK(connection);
  res = _dbus_counter_get_unix_fd_value(connection->outgoing_counter);
  CONNECTION_UNLOCK(connection);
  return res;
}
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
const char* _dbus_connection_get_address(DBusConnection *connection) {
  return _dbus_transport_get_address(connection->transport);
}
#endif