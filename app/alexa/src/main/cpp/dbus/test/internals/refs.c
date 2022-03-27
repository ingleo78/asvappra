#include "../../../glib/glib.h"
#include "../../../glib/glib-object.h"
#include "../../config.h"
#include "../../dbus.h"
#include "../../dbus-connection-internal.h"
#include "../../dbus-mainloop.h"
#include "../../dbus-message-internal.h"
#include "../../dbus-pending-call-internal.h"
#include "../../dbus-server-protected.h"
#include "../../test/test-utils-glib.h"

static void assert_no_error(const DBusError *e) {
  if (G_UNLIKELY(dbus_error_is_set(e))) g_error("expected success but got error: %s: %s", e->name, e->message);
}
#define N_THREADS  200
#define N_REFS  10000
G_STATIC_ASSERT(((unsigned)N_THREADS * (unsigned)N_REFS) < G_MAXINT32);
static dbus_int32_t connection_slot = -1;
static dbus_int32_t server_slot = -1;
static dbus_int32_t message_slot = -1;
static dbus_int32_t pending_call_slot = -1;
typedef struct {
  DBusError e;
  DBusLoop *loop;
  DBusServer *server;
  DBusConnection *connection;
  DBusConnection *server_connection;
  DBusMessage *message;
  GThread *threads[N_THREADS];
  gboolean last_unref;
  unsigned n_refs;
  unsigned n_threads;
} Fixture;
typedef void *(*RefFunc)(void*);
typedef void (*VoidFunc)(void*);
typedef struct {
  const Fixture *f;
  void *thing;
  RefFunc ref;
  VoidFunc ref_void;
  VoidFunc unref;
  void *mutex;
  VoidFunc lock;
  VoidFunc unlock;
} Thread;
static gpointer ref_thread(gpointer data) {
  Thread *thread = data;
  const Fixture *f = thread->f;
  unsigned i;
  for (i = 0; i < f->n_refs; i++) {
      if (thread->lock != NULL) (thread->lock)(thread->mutex);
      if (thread->ref != NULL) {
          gpointer ret = (thread->ref)(thread->thing);
          g_assert(ret == thread->thing);
      } else (thread->ref_void)(thread->thing);
      if (thread->unlock != NULL) (thread->unlock)(thread->mutex);
  }
  return NULL;
}
static gpointer cycle_thread(gpointer data) {
  Thread *thread = data;
  const Fixture *f = thread->f;
  unsigned i;
  for (i = 0; i < f->n_refs; i++) {
      if (thread->lock != NULL) (thread->lock)(thread->mutex);
      if (thread->ref != NULL) {
          gpointer ret = (thread->ref)(thread->thing);
          g_assert(ret == thread->thing);
      } else (thread->ref_void)(thread->thing);
      (thread->unref)(thread->thing);
      if (thread->unlock != NULL) (thread->unlock)(thread->mutex);
  }
  return NULL;
}
static gpointer unref_thread(gpointer data) {
  Thread *thread = data;
  const Fixture *f = thread->f;
  unsigned i;
  for (i = 0; i < f->n_refs; i++) {
      if (thread->lock != NULL) (thread->lock)(thread->mutex);
      (thread->unref)(thread->thing);
      if (thread->unlock != NULL) (thread->unlock)(thread->mutex);
  }
  return NULL;
}
static void last_unref(void *data) {
  Fixture *f = data;
  g_assert(!f->last_unref);
  f->last_unref = TRUE;
}
static void wait_for_all_threads(Fixture *f) {
  unsigned i;
  for (i = 0; i < f->n_threads; i++) g_thread_join(f->threads[i]);
}
static void new_conn_cb(DBusServer *server, DBusConnection *server_connection, void *data) {
  Fixture *f = data;
  g_assert(f->server_connection == NULL);
  f->server_connection = dbus_connection_ref(server_connection);
  test_connection_setup(f->loop, f->server_connection);
}
static void setup(Fixture *f, gconstpointer data) {
  if (!dbus_threads_init_default()) g_error("OOM");
  test_timeout_reset(1);
  f->n_threads = N_THREADS;
  f->n_refs = N_REFS;
  if (g_getenv("WINESERVERSOCKET") != NULL) {
      f->n_threads = 10;
      f->n_refs = 10;
  }
  f->loop = _dbus_loop_new();
  g_assert(f->loop != NULL);
  dbus_error_init(&f->e);
  f->server = dbus_server_listen("tcp:host=127.0.0.1", &f->e);
  assert_no_error(&f->e);
  g_assert(f->server != NULL);
  if (!dbus_connection_allocate_data_slot(&connection_slot)) g_error("OOM");
  if (!dbus_server_allocate_data_slot(&server_slot)) g_error("OOM");
  if (!dbus_message_allocate_data_slot(&message_slot)) g_error("OOM");
  if (!dbus_pending_call_allocate_data_slot(&pending_call_slot)) g_error("OOM");
}
static void setup_connection(Fixture *f, gconstpointer data) {
  char *address;
  setup(f, data);
  dbus_server_set_new_connection_function(f->server, new_conn_cb, f, NULL);
  test_server_setup(f->loop, f->server);
  address = dbus_server_get_address(f->server);
  g_assert(address != NULL);
  f->connection = dbus_connection_open_private(address, &f->e);
  assert_no_error(&f->e);
  g_assert(f->connection != NULL);
  dbus_free(address);
  test_connection_setup(f->loop, f->connection);
  while(f->server_connection == NULL) _dbus_loop_iterate(f->loop, TRUE);
  test_connection_shutdown(f->loop, f->connection);
  test_server_shutdown(f->loop, f->server);
}
static void test_connection(Fixture *f, gconstpointer data) {
  Thread public_api = {
      f,
      f->connection,
      (RefFunc)dbus_connection_ref,
      NULL,
      (VoidFunc)dbus_connection_unref,
      NULL,
      NULL,
      NULL
  };
  Thread internal_api = {
      f,
      f->connection,
      (RefFunc)_dbus_connection_ref_unlocked,
      NULL,
      (VoidFunc)_dbus_connection_unref_unlocked,
      f->connection,
      (VoidFunc)_dbus_connection_lock,
      (VoidFunc)_dbus_connection_unlock
  };
  unsigned i;
  if (!dbus_connection_set_data(f->connection, connection_slot, f, last_unref)) g_error("OOM");
  for (i = 0; i < f->n_threads; i++) {
      if ((i % 2) == 0) f->threads[i] = NULL;//g_thread_new(NULL, ref_thread, &public_api);
      else f->threads[i] = NULL;//g_thread_new(NULL, ref_thread, &internal_api);
      g_assert(f->threads[i] != NULL);
  }
  wait_for_all_threads(f);
  for (i = 0; i < f->n_threads; i++) {
      if ((i % 2) == 0) f->threads[i] = NULL;//g_thread_new(NULL, cycle_thread, &public_api);
      else f->threads[i] = NULL;//g_thread_new(NULL, cycle_thread, &internal_api);
      g_assert(f->threads[i] != NULL);
  }
  wait_for_all_threads (f);
  for (i = 0; i < f->n_threads; i++) {
      if ((i % 2) == 0) f->threads[i] = NULL;//g_thread_new(NULL, unref_thread, &public_api);
      else f->threads[i] = NULL;//g_thread_new(NULL, unref_thread, &internal_api);
      g_assert(f->threads[i] != NULL);
  }
  wait_for_all_threads(f);
  g_assert(!f->last_unref);
  dbus_connection_close(f->connection);
  dbus_connection_unref(f->connection);
  f->connection = NULL;
  g_assert(f->last_unref);
}
static void server_lock(void *server) {
  SERVER_LOCK(((DBusServer*)server));
}
static void server_unlock(void *server) {
  SERVER_UNLOCK(((DBusServer*)server));
}
static void test_server(Fixture *f, gconstpointer data) {
  Thread public_api = {
      f,
      f->server,
      (RefFunc)dbus_server_ref,
      NULL,
      (VoidFunc)dbus_server_unref,
      NULL,
      NULL,
      NULL
  };
  Thread internal_api = {
      f,
      f->server,
      NULL,
      (VoidFunc)_dbus_server_ref_unlocked,
      (VoidFunc)_dbus_server_unref_unlocked,
      f->server,
      server_lock,
      server_unlock
  };
  unsigned i;
  if (!dbus_server_set_data(f->server, server_slot, f, last_unref)) g_error("OOM");
  for (i = 0; i < f->n_threads; i++) {
      if ((i % 2) == 0) f->threads[i] = NULL;//g_thread_new(NULL, ref_thread, &public_api);
      else f->threads[i] = NULL;//g_thread_new(NULL, ref_thread, &internal_api);
      g_assert(f->threads[i] != NULL);
  }
  wait_for_all_threads(f);
  for (i = 0; i < f->n_threads; i++) {
      if ((i % 2) == 0) f->threads[i] = NULL;//g_thread_new(NULL, cycle_thread, &public_api);
      else f->threads[i] = NULL;//g_thread_new(NULL, cycle_thread, &internal_api);
      g_assert(f->threads[i] != NULL);
  }
  wait_for_all_threads(f);
  for (i = 0; i < f->n_threads; i++) {
      if ((i % 2) == 0) f->threads[i] = NULL;//g_thread_new(NULL, unref_thread, &public_api);
      else f->threads[i] = NULL;//g_thread_new(NULL, unref_thread, &internal_api);
      g_assert(f->threads[i] != NULL);
  }
  wait_for_all_threads(f);
  g_assert(!f->last_unref);
  dbus_server_disconnect(f->server);
  dbus_server_unref(f->server);
  f->server = NULL;
  g_assert(f->last_unref);
}
static void test_message(Fixture *f, gconstpointer data) {
  DBusMessage *message = dbus_message_new_signal("/foo", "foo.bar.baz","Foo");
  Thread public_api = {
      f,
      message,
      (RefFunc)dbus_message_ref,
      NULL,
      (VoidFunc)dbus_message_unref,
      NULL,
      NULL,
      NULL
  };
  unsigned i;
  if (!dbus_message_set_data(message, message_slot, f, last_unref)) g_error("OOM");
  for (i = 0; i < f->n_threads; i++) {
      f->threads[i] = NULL;//g_thread_new(NULL, ref_thread, &public_api);
      g_assert(f->threads[i] != NULL);
  }
  wait_for_all_threads(f);
  for (i = 0; i < f->n_threads; i++) {
      f->threads[i] = NULL;//g_thread_new(NULL, cycle_thread, &public_api);
      g_assert(f->threads[i] != NULL);
  }
  wait_for_all_threads(f);
  for (i = 0; i < f->n_threads; i++) {
      f->threads[i] = NULL;//g_thread_new(NULL, unref_thread, &public_api);
      g_assert(f->threads[i] != NULL);
  }
  wait_for_all_threads(f);
  g_assert(!f->last_unref);
  dbus_message_unref(message);
  g_assert(f->last_unref);
}
static void test_pending_call(Fixture *f, gconstpointer data) {
  Thread public_api = {
      f,
      NULL,
      (RefFunc)dbus_pending_call_ref,
      NULL,
      (VoidFunc)dbus_pending_call_unref,
      NULL,
      NULL,
      NULL
  };
  Thread internal_api = {
      f,
      NULL,
      (RefFunc)_dbus_pending_call_ref_unlocked,
      NULL,
      (VoidFunc)dbus_pending_call_unref,
      f->connection,
      (VoidFunc)_dbus_connection_lock,
      (VoidFunc)_dbus_connection_unlock
  };
  Thread unref_and_unlock_api = {
      f,
      NULL,
      (RefFunc)_dbus_pending_call_ref_unlocked,
      NULL,
      (VoidFunc)_dbus_pending_call_unref_and_unlock,
      f->connection,
      (VoidFunc)_dbus_connection_lock,
      NULL
  };
  unsigned i;
  DBusPendingCall *pending_call;
  _dbus_connection_lock(f->connection);
  pending_call = _dbus_pending_call_new_unlocked(f->connection, DBUS_TIMEOUT_INFINITE, NULL);
  g_assert(pending_call != NULL);
  _dbus_connection_unlock(f->connection);
  public_api.thing = pending_call;
  internal_api.thing = pending_call;
  unref_and_unlock_api.thing = pending_call;
  if (!dbus_pending_call_set_data(pending_call, pending_call_slot, f, last_unref)) g_error("OOM");
  for (i = 0; i < f->n_threads; i++) {
      if ((i % 2) == 0) f->threads[i] = NULL;//g_thread_new(NULL, ref_thread, &public_api);
      else f->threads[i] = NULL;//g_thread_new(NULL, ref_thread, &internal_api);
      g_assert(f->threads[i] != NULL);
  }
  wait_for_all_threads(f);
  for (i = 0; i < f->n_threads; i++) {
      switch(i % 3) {
          case 0: f->threads[i] = NULL;//g_thread_new(NULL, cycle_thread, &public_api); break;
          case 1: f->threads[i] = NULL;//g_thread_new(NULL, cycle_thread, &internal_api); break;
          default: f->threads[i] = NULL;//g_thread_new(NULL, cycle_thread, &unref_and_unlock_api);
      }
      g_assert(f->threads[i] != NULL);
  }
  wait_for_all_threads(f);
  for (i = 0; i < f->n_threads; i++) {
      switch(i % 3) {
          case 0: f->threads[i] = NULL;//g_thread_new(NULL, unref_thread, &public_api); break;
          case 1: f->threads[i] = NULL;//g_thread_new(NULL, unref_thread, &internal_api); break;
          default: f->threads[i] = NULL;//g_thread_new(NULL, unref_thread, &unref_and_unlock_api);
      }
      g_assert(f->threads[i] != NULL);
  }
  wait_for_all_threads(f);
  g_assert(!f->last_unref);
  dbus_pending_call_unref(pending_call);
  g_assert(f->last_unref);
}
static void teardown(Fixture *f, gconstpointer data) {
  if (f->server_connection != NULL) {
      dbus_connection_close(f->server_connection);
      dbus_connection_unref(f->server_connection);
  }
  if (f->connection != NULL) {
      dbus_connection_close(f->connection);
      dbus_connection_unref(f->connection);
  }
  if (f->server != NULL) {
      dbus_server_disconnect(f->server);
      dbus_server_unref(f->server);
  }
  dbus_connection_free_data_slot(&connection_slot);
  dbus_server_free_data_slot(&server_slot);
  dbus_message_free_data_slot(&message_slot);
  dbus_pending_call_free_data_slot(&pending_call_slot);
  _dbus_loop_unref(f->loop);
  dbus_error_free(&f->e);
}
int main(int argc, char **argv){
  test_init (&argc, &argv);
  g_test_add("/refs/connection", Fixture, NULL, setup_connection, test_connection, teardown);
  g_test_add("/refs/message", Fixture, NULL, setup, test_message, teardown);
  g_test_add("/refs/pending-call", Fixture, NULL, setup_connection, test_pending_call, teardown);
  g_test_add("/refs/server", Fixture, NULL, setup, test_server, teardown);
  return g_test_run();
}