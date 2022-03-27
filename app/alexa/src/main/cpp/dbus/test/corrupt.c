#include "../../glib/glib.h"
#include "../../gio/gio.h"
#include "../config.h"
#include "../dbus.h"
#include "test-utils-glib.h"

typedef struct {
    DBusError e;
    TestMainContext *ctx;
    DBusServer *server;
    DBusConnection *server_conn;
    GQueue client_messages;
    DBusConnection *client_conn;
} Fixture;
static void assert_no_error(const DBusError *e) {
  if (G_UNLIKELY(dbus_error_is_set(e))) g_error("expected success but got error: %s: %s", e->name, e->message);
}
static DBusHandlerResult client_message_cb(DBusConnection *client_conn, DBusMessage *message, void *data) {
  Fixture *f = data;
  g_assert(client_conn == f->client_conn);
  g_queue_push_tail(&f->client_messages, dbus_message_ref(message));
  return DBUS_HANDLER_RESULT_HANDLED;
}
static void new_conn_cb(DBusServer *server, DBusConnection *server_conn, void *data) {
  Fixture *f = data;
  g_assert(f->server_conn == NULL);
  f->server_conn = dbus_connection_ref(server_conn);
  test_connection_setup(f->ctx, server_conn);
}
static void setup(Fixture *f, gconstpointer addr) {
  f->ctx = test_main_context_get();
  dbus_error_init(&f->e);
  g_queue_init(&f->client_messages);
  f->server = dbus_server_listen(addr, &f->e);
  assert_no_error(&f->e);
  g_assert(f->server != NULL);
  dbus_server_set_new_connection_function(f->server, new_conn_cb, f, NULL);
  test_server_setup(f->ctx, f->server);
}
static void test_connect(Fixture *f, gconstpointer addr G_GNUC_UNUSED) {
  dbus_bool_t have_mem;
  char *address = NULL;
  g_assert(f->server_conn == NULL);
  address = dbus_server_get_address(f->server);
  f->client_conn = dbus_connection_open_private(address, &f->e);
  assert_no_error(&f->e);
  g_assert(f->client_conn != NULL);
  test_connection_setup(f->ctx, f->client_conn);
  dbus_free(address);
  while (f->server_conn == NULL) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  have_mem = dbus_connection_add_filter(f->client_conn, client_message_cb, f, NULL);
  g_assert(have_mem);
}
static void test_message(Fixture *f, gconstpointer addr) {
  dbus_bool_t have_mem;
  dbus_uint32_t serial;
  DBusMessage *outgoing, *incoming;
  test_connect(f, addr);
  outgoing = dbus_message_new_signal("/com/example/Hello","com.example.Hello", "Greeting");
  g_assert(outgoing != NULL);
  have_mem = dbus_connection_send(f->server_conn, outgoing, &serial);
  g_assert(have_mem);
  g_assert(serial != 0);
  while(g_queue_is_empty(&f->client_messages)) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  g_assert_cmpuint(g_queue_get_length(&f->client_messages), ==, 1);
  incoming = g_queue_pop_head(&f->client_messages);
  g_assert(!dbus_message_contains_unix_fds(incoming));
  g_assert_cmpstr(dbus_message_get_destination(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_error_name(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_interface(incoming), ==,"com.example.Hello");
  g_assert_cmpstr(dbus_message_get_member(incoming), ==, "Greeting");
  g_assert_cmpstr(dbus_message_get_sender(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_signature(incoming), ==, "");
  g_assert_cmpstr(dbus_message_get_path(incoming), ==, "/com/example/Hello");
  g_assert_cmpuint(dbus_message_get_serial(incoming), ==, serial);
  dbus_message_unref(incoming);
  dbus_message_unref(outgoing);
}
static void send_n_bytes(GSocket *socket, const gchar *blob, gssize blob_len) {
  gssize len, total_sent;
  GError *gerror = NULL;
  total_sent = 0;
  while(total_sent < blob_len) {
      len = g_socket_send(socket,blob + total_sent,blob_len - total_sent,NULL, &gerror);
      if (g_error_matches(gerror, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK)) {
          g_clear_error(&gerror);
          g_usleep(G_USEC_PER_SEC / 10);
          continue;
      }
      g_assert_no_error (gerror);
      g_assert (len >= 0);
      total_sent += len;
  }
}
#define CORRUPT_LEN 1024
static const gchar not_a_dbus_message[CORRUPT_LEN] = { 0 };
static void test_corrupt(Fixture *f, gconstpointer addr) {
  GSocket *socket;
  GError *gerror = NULL;
  int fd;
  DBusMessage *incoming;
  test_message(f, addr);
  dbus_connection_flush(f->server_conn);
  if (!dbus_connection_get_socket(f->server_conn, &fd)) g_error("failed to steal fd from server connection");
  socket = g_socket_new_from_fd(fd, &gerror);
  g_assert_no_error(gerror);
  g_assert(socket != NULL);
  send_n_bytes(socket, not_a_dbus_message, CORRUPT_LEN);
  while(g_queue_is_empty(&f->client_messages)) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  incoming = g_queue_pop_head(&f->client_messages);
  g_assert(!dbus_message_contains_unix_fds(incoming));
  g_assert_cmpstr(dbus_message_get_destination(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_error_name(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_interface(incoming), ==,"org.freedesktop.DBus.Local");
  g_assert_cmpstr(dbus_message_get_member(incoming), ==, "Disconnected");
  g_assert_cmpstr(dbus_message_get_sender(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_signature(incoming), ==, "");
  g_assert_cmpstr(dbus_message_get_path(incoming), ==,"/org/freedesktop/DBus/Local");
  dbus_message_unref(incoming);
  dbus_connection_close(f->server_conn);
  dbus_connection_unref(f->server_conn);
  f->server_conn = NULL;
  g_object_unref(socket);
}
static void test_byte_order(Fixture *f, gconstpointer addr) {
  GSocket *socket;
  GError *gerror = NULL;
  int fd;
  char *blob;
  const gchar *arg = not_a_dbus_message;
  int blob_len;
  DBusMessage *message;
  dbus_bool_t mem;
  test_message(f, addr);
  message = dbus_message_new_signal("/", "a.b", "c");
  g_assert(message != NULL);
  mem = dbus_message_append_args(message, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &arg, 0xFF, DBUS_TYPE_INVALID);
  g_assert(mem);
  mem = dbus_message_marshal(message, &blob, &blob_len);
  g_assert(mem);
  g_assert_cmpuint(blob_len, >, 0xFF);
  g_assert(blob != NULL);
  dbus_message_unref(message);
  if (blob[0] == 'B') blob[0] = 'l';
  else blob[0] = 'B';
  dbus_connection_flush(f->server_conn);
  if (!dbus_connection_get_socket(f->server_conn, &fd)) g_error("failed to steal fd from server connection");
  socket = g_socket_new_from_fd(fd, &gerror);
  g_assert_no_error(gerror);
  g_assert(socket != NULL);
  send_n_bytes(socket, blob, blob_len);
  dbus_free(blob);
  while(g_queue_is_empty(&f->client_messages)) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  message = g_queue_pop_head(&f->client_messages);
  g_assert(!dbus_message_contains_unix_fds(message));
  g_assert_cmpstr(dbus_message_get_destination(message), ==, NULL);
  g_assert_cmpstr(dbus_message_get_error_name(message), ==, NULL);
  g_assert_cmpstr(dbus_message_get_interface(message), ==,"org.freedesktop.DBus.Local");
  g_assert_cmpstr(dbus_message_get_member(message), ==, "Disconnected");
  g_assert_cmpstr(dbus_message_get_sender(message), ==, NULL);
  g_assert_cmpstr(dbus_message_get_signature(message), ==, "");
  g_assert_cmpstr(dbus_message_get_path(message), ==,"/org/freedesktop/DBus/Local");
  dbus_message_unref(message);
  dbus_connection_close(f->server_conn);
  dbus_connection_unref(f->server_conn);
  f->server_conn = NULL;
  g_object_unref(socket);
}
static void teardown(Fixture *f, gconstpointer addr G_GNUC_UNUSED) {
  if (f->client_conn != NULL) {
      test_connection_shutdown(f->ctx, f->client_conn);
      dbus_connection_close(f->client_conn);
      dbus_connection_unref(f->client_conn);
      f->client_conn = NULL;
  }
  if (f->server_conn != NULL) {
      test_connection_shutdown(f->ctx, f->server_conn);
      dbus_connection_close(f->server_conn);
      dbus_connection_unref(f->server_conn);
      f->server_conn = NULL;
  }
  if (f->server != NULL) {
      dbus_server_disconnect(f->server);
      dbus_server_unref(f->server);
      f->server = NULL;
  }
  test_main_context_unref(f->ctx);
}
int main(int argc, char **argv) {
  test_init(&argc, &argv);
  g_test_add("/corrupt/tcp", Fixture, "tcp:host=127.0.0.1", setup, test_corrupt, teardown);
#ifdef DBUS_UNIX
  g_test_add("/corrupt/unix", Fixture, "unix:tmpdir=/tmp", setup, test_corrupt, teardown);
#endif
  g_test_add("/corrupt/byte-order/tcp", Fixture, "tcp:host=127.0.0.1", setup, test_byte_order, teardown);
  return g_test_run();
}