#include "config.h"
#include "dbus-server.h"
#include "dbus-server-unix.h"
#include "dbus-server-socket.h"
#include "dbus-string.h"
#include "dbus-server-debug-pipe.h"
#include "dbus-address.h"
#include "dbus-protocol.h"

#ifndef _dbus_server_trace_ref
void _dbus_server_trace_ref(DBusServer *server, int old_refcount, int new_refcount, const char *why) {
  static int enabled = -1;
  _dbus_trace_ref("DBusServer", server, old_refcount, new_refcount, why,"DBUS_SERVER_TRACE", &enabled);
}
#endif
static char* copy_address_with_guid_appended(const DBusString *address, const DBusString *guid_hex) {
  DBusString with_guid;
  char *retval;
  if (!_dbus_string_init(&with_guid)) return NULL;
  if (!_dbus_string_copy(address, 0, &with_guid, _dbus_string_get_length(&with_guid)) || !_dbus_string_append(&with_guid, ",guid=") ||
      !_dbus_string_copy(guid_hex, 0, &with_guid, _dbus_string_get_length(&with_guid))) {
      _dbus_string_free(&with_guid);
      return NULL;
  }
  retval = NULL;
  _dbus_string_steal_data(&with_guid, &retval);
  _dbus_string_free(&with_guid);
  return retval;
}
dbus_bool_t _dbus_server_init_base(DBusServer *server, const DBusServerVTable *vtable, const DBusString *address, DBusError *error) {
  server->vtable = vtable;
#ifdef DBUS_DISABLE_ASSERT
  _dbus_atomic_inc(&server->refcount);
#else
  {
      dbus_int32_t old_refcount = _dbus_atomic_inc(&server->refcount);
      _dbus_assert (old_refcount == 0);
  }
#endif
  server->address = NULL;
  server->watches = NULL;
  server->timeouts = NULL;
  server->published_address = FALSE;
  if (!_dbus_string_init(&server->guid_hex)) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (!_dbus_generate_uuid(&server->guid, error)) goto failed;
  if (!_dbus_uuid_encode(&server->guid, &server->guid_hex)) goto oom;
  server->address = copy_address_with_guid_appended(address, &server->guid_hex);
  if (server->address == NULL) goto oom;
  _dbus_rmutex_new_at_location(&server->mutex);
  if (server->mutex == NULL) goto oom;
  server->watches = _dbus_watch_list_new();
  if (server->watches == NULL) goto oom;
  server->timeouts = _dbus_timeout_list_new();
  if (server->timeouts == NULL) goto oom;
  _dbus_data_slot_list_init(&server->slot_list);
  _dbus_verbose("Initialized server on address %s\n", server->address);
  return TRUE;
oom:
  _DBUS_SET_OOM(error);
failed:
  _dbus_rmutex_free_at_location(&server->mutex);
  server->mutex = NULL;
  if (server->watches) {
      _dbus_watch_list_free(server->watches);
      server->watches = NULL;
  }
  if (server->timeouts) {
      _dbus_timeout_list_free(server->timeouts);
      server->timeouts = NULL;
  }
  if (server->address) {
      dbus_free(server->address);
      server->address = NULL;
  }
  _dbus_string_free(&server->guid_hex);
  return FALSE;
}
void _dbus_server_finalize_base(DBusServer *server) {
#ifndef DBUS_DISABLE_CHECKS
  _dbus_assert(!server->have_server_lock);
#endif
  _dbus_assert(server->disconnected);
  _dbus_data_slot_list_free(&server->slot_list);
  dbus_server_set_new_connection_function(server, NULL, NULL, NULL);
  _dbus_watch_list_free(server->watches);
  _dbus_timeout_list_free(server->timeouts);
  _dbus_rmutex_free_at_location(&server->mutex);
  dbus_free(server->address);
  dbus_free_string_array(server->auth_mechanisms);
  _dbus_string_free(&server->guid_hex);
}
typedef dbus_bool_t (*DBusWatchAddFunction)(DBusWatchList *list, DBusWatch *watch);
typedef void (*DBusWatchRemoveFunction)(DBusWatchList *list, DBusWatch *watch);
typedef void (*DBusWatchToggleFunction)(DBusWatchList *list, DBusWatch *watch, dbus_bool_t enabled);
static dbus_bool_t protected_change_watch(DBusServer *server, DBusWatch *watch, DBusWatchAddFunction add_function, DBusWatchRemoveFunction remove_function,
                                          DBusWatchToggleFunction toggle_function, dbus_bool_t enabled) {
  DBusWatchList *watches;
  dbus_bool_t retval;
  HAVE_LOCK_CHECK(server);
  watches = server->watches;
  if (watches) {
      server->watches = NULL;
      _dbus_server_ref_unlocked(server);
      SERVER_UNLOCK(server);
      if (add_function) retval = (*add_function)(watches, watch);
      else if (remove_function) {
          retval = TRUE;
          (*remove_function)(watches, watch);
      } else {
          retval = TRUE;
          (*toggle_function)(watches, watch, enabled);
      }
      SERVER_LOCK(server);
      server->watches = watches;
      _dbus_server_unref_unlocked(server);
      return retval;
  } else return FALSE;
}
dbus_bool_t _dbus_server_add_watch(DBusServer *server, DBusWatch *watch) {
  HAVE_LOCK_CHECK(server);
  return protected_change_watch(server, watch, _dbus_watch_list_add_watch,NULL, NULL, FALSE);
}
void _dbus_server_remove_watch(DBusServer *server, DBusWatch *watch) {
  HAVE_LOCK_CHECK(server);
  protected_change_watch(server, watch,NULL, _dbus_watch_list_remove_watch,NULL, FALSE);
}
void _dbus_server_toggle_all_watches(DBusServer *server, dbus_bool_t enabled) {
  _dbus_watch_list_toggle_all_watches(server->watches, enabled);
}
typedef dbus_bool_t (*DBusTimeoutAddFunction)(DBusTimeoutList *list, DBusTimeout *timeout);
typedef void (*DBusTimeoutRemoveFunction)(DBusTimeoutList *list, DBusTimeout *timeout);
typedef void (*DBusTimeoutToggleFunction)(DBusTimeoutList *list, DBusTimeout *timeout, dbus_bool_t enabled);
static dbus_bool_t protected_change_timeout(DBusServer *server, DBusTimeout *timeout, DBusTimeoutAddFunction add_function, DBusTimeoutRemoveFunction remove_function,
                                            DBusTimeoutToggleFunction toggle_function, dbus_bool_t enabled) {
  DBusTimeoutList *timeouts;
  dbus_bool_t retval;
  HAVE_LOCK_CHECK(server);
  timeouts = server->timeouts;
  if (timeouts) {
      server->timeouts = NULL;
      _dbus_server_ref_unlocked(server);
      SERVER_UNLOCK(server);
      if (add_function) retval = (*add_function)(timeouts, timeout);
      else if (remove_function) {
          retval = TRUE;
          (*remove_function)(timeouts, timeout);
      } else {
          retval = TRUE;
          (*toggle_function)(timeouts, timeout, enabled);
      }
      SERVER_LOCK(server);
      server->timeouts = timeouts;
      _dbus_server_unref_unlocked(server);
      return retval;
  } else return FALSE;
}
dbus_bool_t _dbus_server_add_timeout(DBusServer *server, DBusTimeout *timeout) {
  return protected_change_timeout(server, timeout, _dbus_timeout_list_add_timeout,NULL, NULL, FALSE);
}
void _dbus_server_remove_timeout(DBusServer *server, DBusTimeout *timeout) {
  protected_change_timeout(server, timeout,NULL, _dbus_timeout_list_remove_timeout,NULL, FALSE);
}
void _dbus_server_toggle_timeout(DBusServer *server, DBusTimeout *timeout, dbus_bool_t enabled) {
  protected_change_timeout(server, timeout,NULL,NULL, _dbus_timeout_list_toggle_timeout, enabled);
}
void _dbus_server_ref_unlocked(DBusServer *server) {
  dbus_int32_t old_refcount;
  _dbus_assert(server != NULL);
  HAVE_LOCK_CHECK(server);
  old_refcount = _dbus_atomic_inc(&server->refcount);
  _dbus_assert(old_refcount > 0);
  _dbus_server_trace_ref(server, old_refcount, old_refcount + 1,"ref_unlocked");
}
void _dbus_server_unref_unlocked(DBusServer *server) {
  dbus_int32_t old_refcount;
  _dbus_assert(server != NULL);
  HAVE_LOCK_CHECK(server);
  old_refcount = _dbus_atomic_dec(&server->refcount);
  _dbus_assert(old_refcount > 0);
  _dbus_server_trace_ref(server, old_refcount, old_refcount - 1,"unref_unlocked");
  if (old_refcount == 1) {
      _dbus_assert(server->disconnected);
      SERVER_UNLOCK(server);
      _dbus_assert(server->vtable->finalize != NULL);
      (*server->vtable->finalize)(server);
  }
}
static const struct {
  DBusServerListenResult (*func)(DBusAddressEntry *entry, DBusServer **server_p, DBusError *error);
} listen_funcs[] = {
  { _dbus_server_listen_socket },
  { _dbus_server_listen_platform_specific }
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  , { _dbus_server_listen_debug_pipe }
#endif
};
DBusServer* dbus_server_listen(const char *address, DBusError *error) {
  DBusServer *server;
  DBusAddressEntry **entries;
  int len, i;
  DBusError first_connect_error = DBUS_ERROR_INIT;
  dbus_bool_t handled_once;
  _dbus_return_val_if_fail(address != NULL, NULL);
  _dbus_return_val_if_error_is_set(error, NULL);
  if (!dbus_parse_address(address, &entries, &len, error)) return NULL;
  server = NULL;
  handled_once = FALSE;
  for (i = 0; i < len; i++) {
      int j;
      for (j = 0; j < (int) _DBUS_N_ELEMENTS(listen_funcs); ++j) {
          DBusServerListenResult result;
          DBusError tmp_error = DBUS_ERROR_INIT;
          result = (*listen_funcs[j].func)(entries[i], &server, &tmp_error);
          if (result == DBUS_SERVER_LISTEN_OK) {
              _dbus_assert(server != NULL);
              _DBUS_ASSERT_ERROR_IS_CLEAR(&tmp_error);
              handled_once = TRUE;
              goto out;
          } else if (result == DBUS_SERVER_LISTEN_ADDRESS_ALREADY_USED) {
              _dbus_assert(server == NULL);
              dbus_set_error(error, DBUS_ERROR_ADDRESS_IN_USE,"Address '%s' already used", dbus_address_entry_get_method(entries[0]));
              handled_once = TRUE;
              goto out;
          } else if (result == DBUS_SERVER_LISTEN_BAD_ADDRESS) {
              _dbus_assert(server == NULL);
              _DBUS_ASSERT_ERROR_IS_SET(&tmp_error);
              dbus_move_error(&tmp_error, error);
              handled_once = TRUE;
              goto out;
          } else if (result == DBUS_SERVER_LISTEN_NOT_HANDLED) {
              _dbus_assert(server == NULL);
              _DBUS_ASSERT_ERROR_IS_CLEAR(&tmp_error);
          } else if (result == DBUS_SERVER_LISTEN_DID_NOT_CONNECT) {
              _dbus_assert(server == NULL);
              _DBUS_ASSERT_ERROR_IS_SET(&tmp_error);
              if (!dbus_error_is_set(&first_connect_error)) dbus_move_error(&tmp_error, &first_connect_error);
              else dbus_error_free(&tmp_error);
              handled_once = TRUE;
          }
      }
      _dbus_assert(server == NULL);
      _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  }
out:
  if (!handled_once) {
      _DBUS_ASSERT_ERROR_IS_CLEAR(error);
      if (len > 0) dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS,"Unknown address type '%s'", dbus_address_entry_get_method(entries[0]));
      else dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS,"Empty address '%s'", address);
  }
  dbus_address_entries_free(entries);
  if (server == NULL) {
      _dbus_assert(error == NULL || dbus_error_is_set (&first_connect_error) || dbus_error_is_set (error));
      if (error && dbus_error_is_set(error));
      else {
          _dbus_assert(error == NULL || dbus_error_is_set(&first_connect_error));
          dbus_move_error(&first_connect_error, error);
      }
      _DBUS_ASSERT_ERROR_IS_CLEAR(&first_connect_error);
      _DBUS_ASSERT_ERROR_IS_SET(error);
      return NULL;
  } else {
      _DBUS_ASSERT_ERROR_IS_CLEAR(error);
      return server;
  }
}
DBusServer *dbus_server_ref(DBusServer *server) {
  dbus_int32_t old_refcount;
  _dbus_return_val_if_fail(server != NULL, NULL);
  old_refcount = _dbus_atomic_inc(&server->refcount);
#ifdef DBUS_DISABLE_CHECKS
  if (_DBUS_UNLIKELY(old_refcount <= 0)) {
      _dbus_atomic_dec(&server->refcount);
      _dbus_warn_return_if_fail(_DBUS_FUNCTION_NAME, "old_refcount > 0", __FILE__, __LINE__);
      return NULL;
  }
#endif
  _dbus_server_trace_ref(server, old_refcount, old_refcount + 1, "ref");
  return server;
}
void dbus_server_unref(DBusServer *server) {
  dbus_int32_t old_refcount;
  _dbus_return_if_fail(server != NULL);
  old_refcount = _dbus_atomic_dec(&server->refcount);
#ifdef DBUS_DISABLE_CHECKS
  if (_DBUS_UNLIKELY(old_refcount <= 0)) {
      _dbus_atomic_inc(&server->refcount);
      _dbus_warn_return_if_fail(_DBUS_FUNCTION_NAME, "old_refcount > 0", __FILE__, __LINE__);
      return;
  }
#endif
  _dbus_server_trace_ref(server, old_refcount, old_refcount - 1, "unref");
  if (old_refcount == 1) {
      _dbus_assert(server->disconnected);
      _dbus_assert(server->vtable->finalize != NULL);
      (*server->vtable->finalize)(server);
  }
}
void _dbus_server_disconnect_unlocked(DBusServer *server) {
  _dbus_assert(server->vtable->disconnect != NULL);
  if (!server->disconnected) {
      server->disconnected = TRUE;
      (*server->vtable->disconnect)(server);
  }
}
void dbus_server_disconnect(DBusServer *server) {
  _dbus_return_if_fail(server != NULL);
  dbus_server_ref(server);
  SERVER_LOCK(server);
  _dbus_server_disconnect_unlocked(server);
  SERVER_UNLOCK(server);
  dbus_server_unref(server);
}
dbus_bool_t dbus_server_get_is_connected(DBusServer *server) {
  dbus_bool_t retval;
  _dbus_return_val_if_fail(server != NULL, FALSE);
  SERVER_LOCK(server);
  retval = !server->disconnected;
  SERVER_UNLOCK(server);
  return retval;
}
char* dbus_server_get_address(DBusServer *server) {
  char *retval;
  _dbus_return_val_if_fail(server != NULL, NULL);
  SERVER_LOCK(server);
  retval = _dbus_strdup(server->address);
  SERVER_UNLOCK(server);
  return retval;
}
char* dbus_server_get_id(DBusServer *server) {
  char *retval;
  _dbus_return_val_if_fail(server != NULL, NULL);
  SERVER_LOCK(server);
  retval = NULL;
  _dbus_string_copy_data(&server->guid_hex, &retval);
  SERVER_UNLOCK(server);
  return retval;
}
void dbus_server_set_new_connection_function(DBusServer *server, DBusNewConnectionFunction function, void *data, DBusFreeFunction free_data_function) {
  DBusFreeFunction old_free_function;
  void *old_data;
  _dbus_return_if_fail(server != NULL);
  SERVER_LOCK(server);
  old_free_function = server->new_connection_free_data_function;
  old_data = server->new_connection_data;
  server->new_connection_function = function;
  server->new_connection_data = data;
  server->new_connection_free_data_function = free_data_function;
  SERVER_UNLOCK(server);
  if (old_free_function != NULL) (*old_free_function)(old_data);
}
dbus_bool_t dbus_server_set_watch_functions(DBusServer *server, DBusAddWatchFunction add_function, DBusRemoveWatchFunction remove_function,
                                            DBusWatchToggledFunction toggled_function, void *data, DBusFreeFunction free_data_function) {
  dbus_bool_t result;
  DBusWatchList *watches;
  _dbus_return_val_if_fail(server != NULL, FALSE);
  SERVER_LOCK(server);
  watches = server->watches;
  server->watches = NULL;
  if (watches) {
      SERVER_UNLOCK(server);
      result = _dbus_watch_list_set_functions(watches, add_function, remove_function, toggled_function, data, free_data_function);
      SERVER_LOCK(server);
  } else {
      _dbus_warn_check_failed("Re-entrant call to %s", _DBUS_FUNCTION_NAME);
      result = FALSE;
  }
  server->watches = watches;
  SERVER_UNLOCK(server);
  return result;
}
dbus_bool_t dbus_server_set_timeout_functions(DBusServer *server, DBusAddTimeoutFunction add_function, DBusRemoveTimeoutFunction remove_function,
                                              DBusTimeoutToggledFunction toggled_function, void *data, DBusFreeFunction free_data_function) {
  dbus_bool_t result;
  DBusTimeoutList *timeouts;
  _dbus_return_val_if_fail(server != NULL, FALSE);
  SERVER_LOCK(server);
  timeouts = server->timeouts;
  server->timeouts = NULL;
  if (timeouts) {
      SERVER_UNLOCK(server);
      result = _dbus_timeout_list_set_functions(timeouts, add_function, remove_function, toggled_function, data, free_data_function);
      SERVER_LOCK(server);
  } else {
      _dbus_warn_check_failed("Re-entrant call to %s", _DBUS_FUNCTION_NAME);
      result = FALSE;
  }
  server->timeouts = timeouts;
  SERVER_UNLOCK(server);
  return result;
}
dbus_bool_t dbus_server_set_auth_mechanisms(DBusServer *server, const char **mechanisms) {
  char **copy;
  _dbus_return_val_if_fail(server != NULL, FALSE);
  SERVER_LOCK(server);
  if (mechanisms != NULL) {
      copy = _dbus_dup_string_array(mechanisms);
      if (copy == NULL) {
          SERVER_UNLOCK(server);
          return FALSE;
      }
  } else copy = NULL;
  dbus_free_string_array(server->auth_mechanisms);
  server->auth_mechanisms = copy;
  SERVER_UNLOCK(server);
  return TRUE;
}
static DBusDataSlotAllocator slot_allocator = _DBUS_DATA_SLOT_ALLOCATOR_INIT (_DBUS_LOCK_NAME (server_slots));
dbus_bool_t dbus_server_allocate_data_slot(dbus_int32_t *slot_p) {
  return _dbus_data_slot_allocator_alloc(&slot_allocator, slot_p);
}
void dbus_server_free_data_slot(dbus_int32_t *slot_p) {
  _dbus_return_if_fail(*slot_p >= 0);
  _dbus_data_slot_allocator_free(&slot_allocator, slot_p);
}
dbus_bool_t dbus_server_set_data(DBusServer *server, int slot, void *data, DBusFreeFunction free_data_func) {
  DBusFreeFunction old_free_func;
  void *old_data;
  dbus_bool_t retval;
  _dbus_return_val_if_fail(server != NULL, FALSE);
  SERVER_LOCK(server);
  retval = _dbus_data_slot_list_set(&slot_allocator, &server->slot_list, slot, data, free_data_func, &old_free_func, &old_data);
  SERVER_UNLOCK(server);
  if (retval) {
      if (old_free_func) (*old_free_func)(old_data);
  }
  return retval;
}
void* dbus_server_get_data(DBusServer *server, int slot) {
  void *res;
  _dbus_return_val_if_fail (server != NULL, NULL);
  SERVER_LOCK (server);
  res = _dbus_data_slot_list_get (&slot_allocator, &server->slot_list, slot);
  SERVER_UNLOCK (server);
  return res;
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include <string.h>
#include "dbus-test.h"

dbus_bool_t _dbus_server_test(void) {
  const char *valid_addresses[] = {
      "tcp:port=1234",
      "tcp:host=localhost,port=1234",
      "tcp:host=localhost,port=1234;tcp:port=5678",
  #ifdef DBUS_UNIX
      "unix:path=./boogie",
      "tcp:port=1234;unix:path=./boogie",
  #endif
  };
  DBusServer *server;
  int i;
  for (i = 0; i < _DBUS_N_ELEMENTS(valid_addresses); i++) {
      DBusError error = DBUS_ERROR_INIT;
      char *address;
      char *id;
      server = dbus_server_listen(valid_addresses[i], &error);
      if (server == NULL) {
          _dbus_warn("server listen error: %s: %s", error.name, error.message);
          dbus_error_free(&error);
          _dbus_assert_not_reached("Failed to listen for valid address.");
      }
      id = dbus_server_get_id(server);
      _dbus_assert(id != NULL);
      address = dbus_server_get_address(server);
      _dbus_assert(address != NULL);
      if (strstr(address, id) == NULL) {
          _dbus_warn("server id '%s' is not in the server address '%s'", id, address);
          _dbus_assert_not_reached("bad server id or address");
      }
      dbus_free(id);
      dbus_free(address);
      dbus_server_disconnect(server);
      dbus_server_unref(server);
  }
  return TRUE;
}
#endif