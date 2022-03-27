#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "../glib/glib.h"
#include "config.h"
#include "gunixfdmessage.h"
#include "gunixfdlist.h"
#include "gioerror.h"

G_DEFINE_TYPE(GUnixFDMessage, g_unix_fd_message, G_TYPE_SOCKET_CONTROL_MESSAGE);
struct _GUnixFDMessagePrivate {
  GUnixFDList *list;
};
static gsize g_unix_fd_message_get_size(GSocketControlMessage *message) {
  GUnixFDMessage *fd_message = G_UNIX_FD_MESSAGE(message);
  return g_unix_fd_list_get_length(fd_message->priv->list) * sizeof(gint);
}
static int g_unix_fd_message_get_level(GSocketControlMessage *message) {
  return SOL_SOCKET;
}
static int g_unix_fd_message_get_msg_type(GSocketControlMessage *message) {
  return SCM_RIGHTS;
}
static GSocketControlMessage *g_unix_fd_message_deserialize(int level, int type, gsize size, gpointer data) {
  GSocketControlMessage *message;
  GUnixFDList *list;
  gint n, s, i;
  gint *fds;
  if (level != SOL_SOCKET || type != SCM_RIGHTS) return NULL;
  if (size % 4 > 0) {
      g_warning("Kernel returned non-integral number of fds");
      return NULL;
  }
  fds = data;
  n = size / sizeof(gint);
  for (i = 0; i < n; i++) {
      do {
          s = fcntl(fds[i], F_SETFD, FD_CLOEXEC);
      } while(s < 0 && errno == EINTR);
      if (s < 0) {
          g_warning("Error setting close-on-exec flag on incoming fd: %s", g_strerror(errno));
          return NULL;
      }
  }
  list = g_unix_fd_list_new_from_array(fds, n);
  message = g_unix_fd_message_new_with_fd_list(list);
  g_object_unref(list);
  return message;
}
static void g_unix_fd_message_serialize(GSocketControlMessage *message, gpointer data) {
  GUnixFDMessage *fd_message = G_UNIX_FD_MESSAGE(message);
  const gint *fds;
  gint n_fds;
  fds = g_unix_fd_list_peek_fds(fd_message->priv->list, &n_fds);
  memcpy(data, fds, sizeof(gint) * n_fds);
}
static void g_unix_fd_message_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GUnixFDMessage *message = G_UNIX_FD_MESSAGE(object);
  g_assert(message->priv->list == NULL);
  g_assert_cmpint(prop_id, ==, 1);
  message->priv->list = g_value_dup_object(value);
  if (message->priv->list == NULL) message->priv->list = g_unix_fd_list_new();
}
GUnixFDList *g_unix_fd_message_get_fd_list(GUnixFDMessage *message) {
  return message->priv->list;
}
static void g_unix_fd_message_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GUnixFDMessage *message = G_UNIX_FD_MESSAGE(object);
  g_assert_cmpint(prop_id, ==, 1);
  g_value_set_object(value, g_unix_fd_message_get_fd_list(message));
}
static void g_unix_fd_message_init(GUnixFDMessage *message) {
  message->priv = G_TYPE_INSTANCE_GET_PRIVATE(message, G_TYPE_UNIX_FD_MESSAGE, GUnixFDMessagePrivate);
}
static void g_unix_fd_message_finalize(GObject *object) {
  GUnixFDMessage *message = G_UNIX_FD_MESSAGE(object);
  g_object_unref(message->priv->list);
  G_OBJECT_CLASS(g_unix_fd_message_parent_class)->finalize(object);
}
static void g_unix_fd_message_class_init(GUnixFDMessageClass *class) {
  GSocketControlMessageClass *scm_class = G_SOCKET_CONTROL_MESSAGE_CLASS(class);
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  g_type_class_add_private(class, sizeof(GUnixFDMessagePrivate));
  scm_class->get_size = g_unix_fd_message_get_size;
  scm_class->get_level = g_unix_fd_message_get_level;
  scm_class->get_type = g_unix_fd_message_get_msg_type;
  scm_class->serialize = g_unix_fd_message_serialize;
  scm_class->deserialize = g_unix_fd_message_deserialize;
  object_class->finalize = g_unix_fd_message_finalize;
  object_class->set_property = g_unix_fd_message_set_property;
  object_class->get_property = g_unix_fd_message_get_property;
  g_object_class_install_property(object_class, 1, g_param_spec_object("fd-list", "file descriptor list", "The GUnixFDList object to send with the message",
                                  G_TYPE_UNIX_FD_LIST, G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}
GSocketControlMessage *g_unix_fd_message_new(void) {
  return g_object_new(G_TYPE_UNIX_FD_MESSAGE, NULL);
}
GSocketControlMessage *g_unix_fd_message_new_with_fd_list(GUnixFDList *fd_list) {
  return g_object_new(G_TYPE_UNIX_FD_MESSAGE,"fd-list", fd_list, NULL);
}
gint *g_unix_fd_message_steal_fds(GUnixFDMessage *message, gint *length) {
  g_return_val_if_fail(G_UNIX_FD_MESSAGE(message), NULL);
  return g_unix_fd_list_steal_fds(message->priv->list, length);
}
gboolean g_unix_fd_message_append_fd(GUnixFDMessage *message, gint fd, GError **error) {
  g_return_val_if_fail(G_UNIX_FD_MESSAGE(message), FALSE);
  return g_unix_fd_list_append(message->priv->list, fd, error) >= 0;
}