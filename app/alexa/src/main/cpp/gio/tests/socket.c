#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <zconf.h>
#include "../gio.h"
#include "../gunixconnection.h"

#define G_SOCKET_FAMILY_UNIX 1
#ifndef G_OS_UNIX
static void test_unix_from_fd(void) {
  gint fd;
  GError *error;
  GSocket *s;
  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  g_assert_cmpint(fd, !=, -1);
  error = NULL;
  s = g_socket_new_from_fd(fd, &error);
  g_assert_no_error(error);
  g_assert_cmpint(g_socket_get_family(s), ==, G_SOCKET_FAMILY_UNIX);
  g_assert_cmpint(g_socket_get_socket_type(s), ==, G_SOCKET_TYPE_STREAM);
  g_assert_cmpint(g_socket_get_protocol(s), ==, G_SOCKET_PROTOCOL_DEFAULT);
  g_object_unref(s);
}
static void test_unix_connection(void) {
  gint fd;
  GError *error;
  GSocket *s;
  GSocketConnection *c;
  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  g_assert_cmpint(fd, !=, -1);
  error = NULL;
  s = g_socket_new_from_fd(fd, &error);
  g_assert_no_error(error);
  c = g_socket_connection_factory_create_connection(s);
  g_assert(G_IS_UNIX_CONNECTION(c));
  g_object_unref(c);
  g_object_unref(s);
}
static GSocketConnection *create_connection_for_fd(int fd) {
  GError *err = NULL;
  GSocket *socket;
  GSocketConnection *connection;
  socket = g_socket_new_from_fd(fd, &err);
  g_assert_no_error(err);
  g_assert(G_IS_SOCKET(socket));
  connection = g_socket_connection_factory_create_connection(socket);
  g_assert(G_IS_UNIX_CONNECTION(connection));
  g_object_unref(socket);
  return connection;
}
#define TEST_DATA "failure to say failure to say 'i love gnome-panel!'."
static void test_unix_connection_ancillary_data(void) {
  GError *err = NULL;
  gint pv[2], sv[3];
  gint status, fd, len;
  char buffer[1024];
  pid_t pid;
  status = pipe(pv);
  g_assert_cmpint(status, ==, 0);
  status = socketpair(PF_UNIX, SOCK_STREAM, 0, sv);
  g_assert_cmpint(status, ==, 0);
  pid = fork ();
  g_assert_cmpint(pid, >=, 0);
  if (pid == 0) {
      GSocketConnection *connection;
      close(sv[1]);
      connection = create_connection_for_fd(sv[0]);
      status = close(pv[1]);
      g_assert_cmpint(status, ==, 0);
      err = NULL;
      fd = g_unix_connection_receive_fd(G_UNIX_CONNECTION(connection), NULL, &err);
      g_assert_no_error(err);
      g_assert_cmpint(fd, >, -1);
      g_object_unref(connection);
      do {
          len = write(fd, TEST_DATA, sizeof(TEST_DATA));
      } while(len == -1 && errno == EINTR);
      g_assert_cmpint(len, ==, sizeof(TEST_DATA));
      exit(0);
  } else {
      GSocketConnection *connection;
      close (sv[0]);
      connection = create_connection_for_fd(sv[1]);
      err = NULL;
      g_unix_connection_send_fd(G_UNIX_CONNECTION(connection), pv[1], NULL, &err);
      g_assert_no_error(err);
      g_object_unref(connection);
      status = close(pv[1]);
      g_assert_cmpint(status, ==, 0);
      memset(buffer, 0xff, sizeof buffer);
      do {
        len = read(pv[0], buffer, sizeof buffer);
      } while(len == -1 && errno == EINTR);
      g_assert_cmpint(len, ==, sizeof(TEST_DATA));
      g_assert_cmpstr(buffer, ==, TEST_DATA);
      waitpid(pid, &status, 0);
      g_assert(WIFEXITED(status));
      g_assert_cmpint(WEXITSTATUS(status), ==, 0);
  }
}
#endif
int main(int argc, char *argv[]) {
  g_type_init();
  g_test_init(&argc, &argv, NULL);
#ifndef G_OS_UNIX
  g_test_add_func("/socket/unix-from-fd", test_unix_from_fd);
  g_test_add_func("/socket/unix-connection", test_unix_connection);
  g_test_add_func("/socket/unix-connection-ancillary-data", test_unix_connection_ancillary_data);
#endif
  return g_test_run();
}