#ifndef DBUS_SERVER_PROTECTED_H
#define DBUS_SERVER_PROTECTED_H

#include "dbus-internals.h"
#include "dbus-threads-internal.h"
#include "dbus-server.h"
#include "dbus-address.h"
#include "dbus-timeout.h"
#include "dbus-watch.h"
#include "dbus-resources.h"
#include "dbus-dataslot.h"
#include "dbus-string.h"

DBUS_BEGIN_DECLS
typedef struct DBusServerVTable DBusServerVTable;
struct DBusServerVTable {
  void (*finalize)(DBusServer *server);
  void (*disconnect)(DBusServer *server);
};
struct DBusServer {
  DBusAtomic refcount;
  const DBusServerVTable *vtable;
  DBusRMutex *mutex;
  DBusGUID guid;
  DBusString guid_hex;
  DBusWatchList *watches;
  DBusTimeoutList *timeouts;
  char *address;
  dbus_uint32_t published_address;
  int max_connections;
  DBusDataSlotList slot_list;
  DBusNewConnectionFunction  new_connection_function;
  void *new_connection_data;
  DBusFreeFunction new_connection_free_data_function;
  char **auth_mechanisms;
  unsigned int disconnected : 1;
#ifndef DBUS_DISABLE_CHECKS
  unsigned int have_server_lock : 1;
#endif
};
dbus_uint32_t _dbus_server_init_base(DBusServer *server, const DBusServerVTable *vtable, const DBusString *address, DBusError *error);
void _dbus_server_finalize_base(DBusServer *server);
void _dbus_server_disconnect_unlocked(DBusServer *server);
dbus_uint32_t _dbus_server_add_watch(DBusServer *server, DBusWatch *watch);
void _dbus_server_remove_watch(DBusServer *server, DBusWatch *watch);
DBUS_PRIVATE_EXPORT
void _dbus_server_toggle_all_watches(DBusServer *server, dbus_uint32_t enabled);
dbus_uint32_t _dbus_server_add_timeout(DBusServer *server, DBusTimeout *timeout);
void _dbus_server_remove_timeout(DBusServer *server, DBusTimeout *timeout);
void _dbus_server_toggle_timeout(DBusServer *server, DBusTimeout *timeout, dbus_uint32_t enabled);
DBUS_PRIVATE_EXPORT void _dbus_server_ref_unlocked(DBusServer *server);
DBUS_PRIVATE_EXPORT void _dbus_server_unref_unlocked(DBusServer *server);
typedef enum {
  DBUS_SERVER_LISTEN_NOT_HANDLED,
  DBUS_SERVER_LISTEN_OK,
  DBUS_SERVER_LISTEN_BAD_ADDRESS,
  DBUS_SERVER_LISTEN_DID_NOT_CONNECT,
  DBUS_SERVER_LISTEN_ADDRESS_ALREADY_USED
} DBusServerListenResult;
DBusServerListenResult _dbus_server_listen_platform_specific(DBusAddressEntry *entry, DBusServer **server_p, DBusError *error);
#ifdef DBUS_ENABLE_VERBOSE_MODE
void _dbus_server_trace_ref(DBusServer *server, int old_refcount, int new_refcount, const char *why);
#else
#define _dbus_server_trace_ref(s,o,n,w) \
  do {\
      (void)(o); \
      (void)(n); \
  } while(0);
#endif
#ifdef DBUS_DISABLE_CHECKS
#define TOOK_LOCK_CHECK(server)
#define RELEASING_LOCK_CHECK(server)
#define HAVE_LOCK_CHECK(server)
#else
#define TOOK_LOCK_CHECK(server) \
  do {                \
      _dbus_assert(!(server)->have_server_lock); \
      (server)->have_server_lock = TRUE;          \
  } while(0);
#define RELEASING_LOCK_CHECK(server) \
  do {            \
      _dbus_assert((server)->have_server_lock);   \
      (server)->have_server_lock = FALSE;          \
  } while(0);
#define HAVE_LOCK_CHECK(server)  _dbus_assert((server)->have_server_lock)
#endif
#define TRACE_LOCKS 0
#define SERVER_LOCK(server) \
  do {                                              \
      if (TRACE_LOCKS) { _dbus_verbose("LOCK\n"); }   \
      _dbus_rmutex_lock((server)->mutex);                                        \
      TOOK_LOCK_CHECK(server);                                                   \
  } while(0);
#define SERVER_UNLOCK(server) \
  do {                                                      \
      if (TRACE_LOCKS) { _dbus_verbose("UNLOCK\n");  }        \
      RELEASING_LOCK_CHECK(server);                                                      \
      _dbus_rmutex_unlock((server)->mutex);                                              \
  } while(0);
DBUS_END_DECLS

#endif