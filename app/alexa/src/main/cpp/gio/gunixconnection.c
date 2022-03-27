#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include "../glib/glibintl.h"
#include "../gobject/gobject.h"
#include "../pulse_audio/pulsecore/i18n.h"
#include "config.h"
#include "gunixconnection.h"
#include "gunixcredentialsmessage.h"
#include "gsocketcontrolmessage.h"
#include "gunixfdmessage.h"
#include "gsocket.h"
#include "gio.h"
#include "gtask.h"

G_DEFINE_TYPE_WITH_CODE(GUnixConnection, g_unix_connection, G_TYPE_SOCKET_CONNECTION,g_socket_connection_factory_register_type(g_define_type_id,
					    G_SOCKET_FAMILY_UNIX, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT););
gboolean g_unix_connection_send_fd(GUnixConnection *connection, gint fd, GCancellable *cancellable, GError **error) {
  GSocketControlMessage *scm;
  GSocket *socket;
  g_return_val_if_fail(G_IS_UNIX_CONNECTION(connection), FALSE);
  g_return_val_if_fail(fd >= 0, FALSE);
  scm = g_unix_fd_message_new();
  if (!g_unix_fd_message_append_fd(G_UNIX_FD_MESSAGE(scm), fd, error)) {
      g_object_unref(scm);
      return FALSE;
  }
  g_object_get(connection, "socket", &socket, NULL);
  if (g_socket_send_message(socket, NULL, NULL, 0, &scm, 1, 0, cancellable, error) != 1) {
      g_object_unref(socket);
      g_object_unref(scm);
      return FALSE;
  }
  g_object_unref(socket);
  g_object_unref(scm);
  return TRUE;
}
gint g_unix_connection_receive_fd(GUnixConnection *connection, GCancellable *cancellable, GError **error) {
  GSocketControlMessage **scms;
  gint *fds, nfd, fd, nscm;
  GUnixFDMessage *fdmsg;
  GSocket *socket;
  g_return_val_if_fail(G_IS_UNIX_CONNECTION(connection), -1);
  g_object_get(connection, "socket", &socket, NULL);
  if (g_socket_receive_message(socket, NULL, NULL, 0, &scms, &nscm, NULL, cancellable, error) != 1) {
      g_object_unref(socket);
      return -1;
  }
  g_object_unref(socket);
  if (nscm != 1) {
      gint i;
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Expecting 1 control message, got %d"), nscm);
      for (i = 0; i < nscm; i++) g_object_unref(scms[i]);
      g_free(scms);
      return -1;
  }
  if (!G_IS_UNIX_FD_MESSAGE(scms[0])) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Unexpected type of ancillary data"));
      g_object_unref(scms[0]);
      g_free(scms);
      return -1;
  }
  fdmsg = G_UNIX_FD_MESSAGE(scms[0]);
  g_free(scms);
  fds = g_unix_fd_message_steal_fds(fdmsg, &nfd);
  g_object_unref(fdmsg);
  if (nfd != 1) {
      gint i;
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Expecting one fd, but got %d\n"), nfd);
      for (i = 0; i < nfd; i++) close(fds[i]);
      g_free(fds);
      return -1;
  }
  fd = *fds;
  g_free(fds);
  if (fd < 0) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Received invalid fd"));
      fd = -1;
  }
  return fd;
}
static void g_unix_connection_init(GUnixConnection *connection) {}
static void g_unix_connection_class_init(GUnixConnectionClass *class) {}
void g_unix_connection_send_fd_async(GUnixConnection *connection, gint fd, gboolean close, gint io_priority, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_unix_connection_send_fd_finish(GUnixConnection *connection, GError **error);
gboolean g_unix_connection_send_fds(GUnixConnection *connection, gint *fds, gint nfds, GError **error);
void g_unix_connection_send_fds_async(GUnixConnection *connection, gint *fds, gint nfds, gint io_priority, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_unix_connection_send_fds_finish(GUnixConnection *connection, GError **error);
void g_unix_connection_receive_fd_async(GUnixConnection *connection, gint io_priority, GAsyncReadyCallback callback, gpointer user_data);
gint g_unix_connection_receive_fd_finish(GUnixConnection *connection, GError **error);
void g_unix_connection_send_credentials_async(GUnixConnection *connection, gint io_priority, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_unix_connection_send_credentials_finish(GUnixConnection *connection, GError **error);
gboolean g_unix_connection_send_fake_credentials(GUnixConnection *connection, guint64 pid, guint64 uid, guint64 gid, GError **error);
void g_unix_connection_send_fake_credentials_async(GUnixConnection *connection, guint64 pid, guint64 uid, guint64 gid, gint io_priority, GAsyncReadyCallback callback,
                                                   gpointer user_data);
gboolean g_unix_connection_send_fake_credentials_finish(GUnixConnection *connection, GError **error);
GCredentials *g_unix_connection_receive_credentials(GUnixConnection *connection, GCancellable *cancellable, GError **error);
void g_unix_connection_receive_credentials_async(GUnixConnection *connection, gint io_priority, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_unix_connection_receive_credentials_finish(GUnixConnection *connection, GAsyncResult result, GError **error) {
    g_return_val_if_fail(g_task_is_valid(&result, connection), FALSE);
    return g_task_propagate_boolean(G_TASK(&result), error);
}
gboolean g_unix_connection_create_pair(GUnixConnection **one, GUnixConnection **two, GError **error);
gboolean g_unix_connection_send_credentials(GUnixConnection *connection, GCancellable *cancellable, GError **error) {
  GCredentials *credentials;
  GSocketControlMessage *scm;
  GSocket *socket;
  gboolean ret;
  GOutputVector vector;
  guchar nul_byte[1] = {'\0'};
  g_return_val_if_fail(G_IS_UNIX_CONNECTION(connection), FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  ret = FALSE;
  credentials = g_credentials_new();
  vector.buffer = &nul_byte;
  vector.size = 1;
  scm = g_unix_credentials_message_new_with_credentials(credentials);
  g_object_get(connection, "socket", &socket, NULL);
  if (g_socket_send_message(socket,NULL, &vector,1, &scm,1,G_SOCKET_MSG_NONE, cancellable, error) != 1) {
      g_prefix_error(error, _("Error sending credentials: "));
      goto out;
  }
  ret = TRUE;
out:
  g_object_unref(socket);
  g_object_unref(scm);
  g_object_unref(credentials);
  return ret;
}
GCredentials *g_unix_connection_receive_credentials(GUnixConnection *connection, GCancellable *cancellable, GError **error) {
  GCredentials *ret;
  GSocketControlMessage **scms;
  gint nscm;
  GSocket *socket;
  gint n;
  volatile GType credentials_message_gtype;
  gssize num_bytes_read;
#ifdef __linux__
  gboolean turn_off_so_passcreds;
#endif
  g_return_val_if_fail(G_IS_UNIX_CONNECTION(connection), NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  ret = NULL;
  scms = NULL;
  g_object_get(connection, "socket", &socket, NULL);
#ifdef __linux__
  {
      gint opt_val;
      socklen_t opt_len;
      turn_off_so_passcreds = FALSE;
      opt_val = 0;
      opt_len = sizeof(gint);
      if (getsockopt(g_socket_get_fd(socket), SOL_SOCKET, SO_PASSCRED, &opt_val, &opt_len) != 0) {
          g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errno), _("Error checking if SO_PASSCRED is enabled for socket: %s"), strerror (errno));
          goto out;
      }
      if (opt_len != sizeof(gint)) {
          g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED, _("Unexpected option length while checking if SO_PASSCRED is enabled for socket. "
                      "Expected %d bytes, got %d"), (gint) sizeof(gint), (gint) opt_len);
          goto out;
      }
      if (opt_val == 0) {
          opt_val = 1;
          if (setsockopt(g_socket_get_fd(socket), SOL_SOCKET, SO_PASSCRED, &opt_val, sizeof opt_val) != 0) {
              g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errno), _("Error enabling SO_PASSCRED: %s"), strerror (errno));
              goto out;
          }
          turn_off_so_passcreds = TRUE;
      }
  }
#endif
  credentials_message_gtype = G_TYPE_UNIX_CREDENTIALS_MESSAGE;
  num_bytes_read = g_socket_receive_message(socket,NULL,NULL,0, &scms, &nscm,NULL, cancellable, error);
  if (num_bytes_read != 1) {
      if (num_bytes_read == 0 && error != NULL && *error == NULL) {
          g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_FAILED, _("Expecting to read a single byte for receiving credentials but read zero bytes"));
      }
      goto out;
  }
  if (nscm != 1) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED, _("Expecting 1 control message, got %d"), nscm);
      goto out;
  }
  if (!G_IS_UNIX_CREDENTIALS_MESSAGE(scms[0])) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_FAILED, _("Unexpected type of ancillary data"));
      goto out;
  }
  ret = g_unix_credentials_message_get_credentials(G_UNIX_CREDENTIALS_MESSAGE(scms[0]));
  g_object_ref(ret);
out:
#ifdef __linux__
  if (turn_off_so_passcreds) {
      gint opt_val;
      opt_val = 0;
      if (setsockopt(g_socket_get_fd(socket), SOL_SOCKET, SO_PASSCRED, &opt_val, sizeof opt_val) != 0) {
          g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errno), _("Error while disabling SO_PASSCRED: %s"), strerror (errno));
          goto out;
      }
  }
#endif
  if (scms != NULL) {
      for (n = 0; n < nscm; n++) g_object_unref(scms[n]);
      g_free(scms);
  }
  g_object_unref(socket);
  return ret;
}
static void receive_credentials_async_thread(GTask *task, gpointer source_object, gpointer task_data, GCancellable  *cancellable) {
    GCredentials *creds;
    GError *error = NULL;
    creds = g_unix_connection_receive_credentials(G_UNIX_CONNECTION(source_object), cancellable, &error);
    if (creds) g_task_return_pointer(task, creds, g_object_unref);
    else g_task_return_error(task, error);
    g_object_unref(task);
}