#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "../glib/glib.h"
#include "config.h"
#include "gunixfdlist.h"
#include "gioerror.h"

#define _GNU_SOURCE
G_DEFINE_TYPE(GUnixFDList, g_unix_fd_list, G_TYPE_OBJECT);
struct _GUnixFDListPrivate {
  gint *fds;
  gint nfd;
};
static void g_unix_fd_list_init(GUnixFDList *list) {
  list->priv = G_TYPE_INSTANCE_GET_PRIVATE(list, G_TYPE_UNIX_FD_LIST, GUnixFDListPrivate);
}
static void g_unix_fd_list_finalize(GObject *object) {
  GUnixFDList *list = G_UNIX_FD_LIST(object);
  gint i;
  for (i = 0; i < list->priv->nfd; i++) close(list->priv->fds[i]);
  g_free (list->priv->fds);
  G_OBJECT_CLASS(g_unix_fd_list_parent_class)->finalize(object);
}
static void g_unix_fd_list_class_init(GUnixFDListClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  g_type_class_add_private(class, sizeof(GUnixFDListPrivate));
  object_class->finalize = g_unix_fd_list_finalize;
}
static int dup_close_on_exec_fd(gint fd, GError **error) {
  gint new_fd;
  gint s;
#ifdef F_DUPFD_CLOEXEC
  do {
      new_fd = fcntl(fd, F_DUPFD_CLOEXEC, 0l);
  } while(new_fd < 0 && (errno == EINTR));
  if (new_fd >= 0) return new_fd;
#endif
  do {
      new_fd = dup(fd);
  } while(new_fd < 0 && (errno == EINTR));
  if (new_fd < 0) {
      int saved_errno = errno;
      g_set_error(error, G_IO_ERROR, g_io_error_from_errno(saved_errno),"dup: %s", g_strerror(saved_errno));
      close(new_fd);
      return -1;
  }
  do {
      s = fcntl(new_fd, F_GETFD);
      if (s >= 0) s = fcntl(new_fd, F_SETFD, (long)(s | FD_CLOEXEC));
  } while(s < 0 && (errno == EINTR));
  if (s < 0) {
      int saved_errno = errno;
      g_set_error(error, G_IO_ERROR, g_io_error_from_errno(saved_errno),"fcntl: %s", g_strerror(saved_errno));
      close(new_fd);
      return -1;
  }
  return new_fd;
}
GUnixFDList *g_unix_fd_list_new(void) {
  return g_object_new(G_TYPE_UNIX_FD_LIST, NULL);
}
GUnixFDList *g_unix_fd_list_new_from_array(const gint *fds, gint n_fds) {
  GUnixFDList *list;
  g_return_val_if_fail(fds != NULL || n_fds == 0, NULL);
  if (n_fds == -1) for (n_fds = 0; fds[n_fds] != -1; n_fds++);
  list = g_object_new(G_TYPE_UNIX_FD_LIST, NULL);
  list->priv->fds = g_new(gint, n_fds + 1);
  list->priv->nfd = n_fds;
  memcpy(list->priv->fds, fds, sizeof(gint) * n_fds);
  list->priv->fds[n_fds] = -1;
  return list;
}
gint *g_unix_fd_list_steal_fds(GUnixFDList *list, gint *length) {
  gint *result;
  g_return_val_if_fail(G_IS_UNIX_FD_LIST(list), NULL);
  if (list->priv->fds == NULL) {
      list->priv->fds = g_new(gint, 1);
      list->priv->fds[0] = -1;
      list->priv->nfd = 0;
  }
  if (length) *length = list->priv->nfd;
  result = list->priv->fds;
  list->priv->fds = NULL;
  list->priv->nfd = 0;
  return result;
}
const gint *g_unix_fd_list_peek_fds(GUnixFDList *list, gint *length) {
  g_return_val_if_fail(G_IS_UNIX_FD_LIST(list), NULL);
  if (list->priv->fds == NULL) {
      list->priv->fds = g_new(gint, 1);
      list->priv->fds[0] = -1;
      list->priv->nfd = 0;
  }
  if (length) *length = list->priv->nfd;
  return list->priv->fds;
}
gint g_unix_fd_list_append(GUnixFDList *list, gint fd, GError **error) {
  gint new_fd;
  g_return_val_if_fail(G_IS_UNIX_FD_LIST(list), -1);
  g_return_val_if_fail(fd >= 0, -1);
  g_return_val_if_fail(error == NULL || *error == NULL, -1);
  if ((new_fd = dup_close_on_exec_fd(fd, error)) < 0) return -1;
  list->priv->fds = g_realloc(list->priv->fds,sizeof(gint) * (list->priv->nfd + 2));
  list->priv->fds[list->priv->nfd++] = new_fd;
  list->priv->fds[list->priv->nfd] = -1;
  return list->priv->nfd - 1;
}
gint g_unix_fd_list_get(GUnixFDList *list, gint index_, GError **error) {
  g_return_val_if_fail(G_IS_UNIX_FD_LIST(list), -1);
  g_return_val_if_fail(index_ < list->priv->nfd, -1);
  g_return_val_if_fail(error == NULL || *error == NULL, -1);
  return dup_close_on_exec_fd(list->priv->fds[index_], error);
}
gint g_unix_fd_list_get_length(GUnixFDList *list) {
  g_return_val_if_fail(G_IS_UNIX_FD_LIST(list), 0);
  return list->priv->nfd;
}