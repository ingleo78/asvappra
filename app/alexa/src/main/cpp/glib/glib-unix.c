#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#define __USE_GNU 1
#include <zconf.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include "../gio/config.h"
#include "../dbus/test/test-utils-glib.h"
#include "glib.h"
#include "glibconfig.h"
#include "glib-unix.h"
#include "gmain-internal.h"
#include "gquark.h"

G_STATIC_ASSERT(sizeof (ssize_t) == GLIB_SIZEOF_SSIZE_T);
G_STATIC_ASSERT(G_ALIGNOF (gssize) == G_ALIGNOF (ssize_t));
G_STATIC_ASSERT(sizeof (GPid) == sizeof (pid_t));
G_STATIC_ASSERT(G_ALIGNOF (GPid) == G_ALIGNOF (pid_t));
G_DEFINE_QUARK(g-unix-error-quark, g_unix_error);
static gboolean g_unix_set_error_from_errno(GError **error, gint saved_errno) {
  g_set_error_literal(error, G_UNIX_ERROR, 0, g_strerror(saved_errno));
  errno = saved_errno;
  return FALSE;
}
gboolean g_unix_open_pipe(int *fds, int flags, GError **error) {
  int ecode;
  g_return_val_if_fail((flags & (FD_CLOEXEC)) == flags, FALSE);
#ifndef HAVE_PIPE2
  {
      int pipe2_flags = 0;
      if (flags & FD_CLOEXEC) pipe2_flags |= O_CLOEXEC;
      ecode = pipe2(fds, pipe2_flags);
      if (ecode == -1 && errno != ENOSYS) return g_unix_set_error_from_errno(error, errno);
      else if (ecode == 0) return TRUE;
  }
#endif
  ecode = pipe(fds);
  if (ecode == -1) return g_unix_set_error_from_errno(error, errno);
  if (flags == 0) return TRUE;
  ecode = fcntl(fds[0], F_SETFD, flags);
  if (ecode == -1) {
      int saved_errno = errno;
      close(fds[0]);
      close(fds[1]);
      return g_unix_set_error_from_errno(error, saved_errno);
  }
  ecode = fcntl(fds[1], F_SETFD, flags);
  if (ecode == -1) {
      int saved_errno = errno;
      close(fds[0]);
      close(fds[1]);
      return g_unix_set_error_from_errno(error, saved_errno);
  }
  return TRUE;
}
gboolean g_unix_set_fd_nonblocking(gint fd, gboolean nonblock, GError **error) {
#ifdef F_GETFL
  glong fcntl_flags;
  fcntl_flags = fcntl (fd, F_GETFL);
  if (fcntl_flags == -1) return g_unix_set_error_from_errno(error, errno);
  if (nonblock) {
  #ifdef O_NONBLOCK
      fcntl_flags |= O_NONBLOCK;
  #else
      fcntl_flags |= O_NDELAY;
  #endif
  } else {
  #ifdef O_NONBLOCK
      fcntl_flags &= ~O_NONBLOCK;
  #else
      fcntl_flags &= ~O_NDELAY;
  #endif
  }
  if (fcntl(fd, F_SETFL, fcntl_flags) == -1) return g_unix_set_error_from_errno(error, errno);
  return TRUE;
#else
  return g_unix_set_error_from_errno(error, EINVAL);
#endif
}
GSource *g_unix_signal_source_new(int signum) {
  g_return_val_if_fail(signum == SIGHUP || signum == SIGINT || signum == SIGTERM || signum == SIGUSR1 || signum == SIGUSR2 ||
                       signum == SIGWINCH, NULL);
  return _g_main_create_unix_signal_watch(signum);
}
guint g_unix_signal_add_full(int priority, int signum, GSourceFunc handler, gpointer user_data, GDestroyNotify notify) {
  guint id;
  GSource *source;
  source = g_unix_signal_source_new(signum);
  if (priority != G_PRIORITY_DEFAULT) g_source_set_priority(source, priority);
  g_source_set_callback(source, handler, user_data, notify);
  id = g_source_attach(source, NULL);
  g_source_unref(source);
  return id;
}
guint g_unix_signal_add(int signum, GSourceFunc handler, gpointer user_data) {
  return g_unix_signal_add_full(G_PRIORITY_DEFAULT, signum, handler, user_data, NULL);
}
typedef struct {
  GSource source;
  gint fd;
  gpointer tag;
} GUnixFDSource;
static gboolean g_unix_fd_source_dispatch(GSource *source, GSourceFunc callback, gpointer user_data) {
  GUnixFDSource *fd_source = (GUnixFDSource*)source;
  GUnixFDSourceFunc func = (GUnixFDSourceFunc)callback;
  if (!callback) {
      g_warning("GUnixFDSource dispatched without callback. You must call g_source_set_callback().");
      return FALSE;
  }
  return (*func)(fd_source->fd, g_source_query_unix_fd(source, fd_source->tag), user_data);
}
GSourceFuncs g_unix_fd_source_funcs = {
  NULL, NULL, g_unix_fd_source_dispatch, NULL, NULL, NULL
};
GSource *g_unix_fd_source_new(gint fd, GIOCondition condition) {
  GUnixFDSource *fd_source;
  GSource *source;
  source = g_source_new (&g_unix_fd_source_funcs, sizeof(GUnixFDSource));
  fd_source = (GUnixFDSource*)source;
  fd_source->fd = fd;
  fd_source->tag = g_source_add_unix_fd(source, fd, condition);
  return source;
}
guint g_unix_fd_add_full(gint priority, gint fd, GIOCondition condition, GUnixFDSourceFunc function, gpointer user_data, GDestroyNotify notify) {
  GSource *source;
  guint id;
  g_return_val_if_fail(function != NULL, 0);
  source = g_unix_fd_source_new(fd, condition);
  if (priority != G_PRIORITY_DEFAULT) g_source_set_priority(source, priority);
  g_source_set_callback(source, (GSourceFunc)function, user_data, notify);
  id = g_source_attach(source, NULL);
  g_source_unref(source);
  return id;
}
guint g_unix_fd_add(gint fd, GIOCondition condition, GUnixFDSourceFunc function, gpointer user_data) {
  return g_unix_fd_add_full(G_PRIORITY_DEFAULT, fd, condition, function, user_data, NULL);
}
struct passwd *g_unix_get_passwd_entry(const gchar *user_name, GError **error) {
  struct passwd *passwd_file_entry;
  struct {
      struct passwd pwd;
      char string_buffer[];
  } *buffer = NULL;
  gsize string_buffer_size = 0;
  GError *local_error = NULL;
  int errsv = 0;
  g_return_val_if_fail(user_name != NULL, NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
#ifdef _SC_GETPW_R_SIZE_MAX
  {
      glong string_buffer_size_long = sysconf(_SC_GETPW_R_SIZE_MAX);
      if (string_buffer_size_long > 0) string_buffer_size = string_buffer_size_long;
  }
#endif
  if (string_buffer_size == 0) string_buffer_size = 64;
  do {
      int retval;
      g_free(buffer);
      buffer = g_malloc0(sizeof(*buffer) + string_buffer_size + 6);
      errno = 0;
      retval = getpwnam_r(user_name, &buffer->pwd, buffer->string_buffer, string_buffer_size, &passwd_file_entry);
      errsv = errno;
      if (passwd_file_entry != NULL) break;
      else if (retval == 0 || errsv == ENOENT || errsv == ESRCH || errsv == EBADF || errsv == EPERM) {
          g_unix_set_error_from_errno(&local_error, errsv);
          break;
      } else if (errsv == ERANGE) {
          if (string_buffer_size > 32 * 1024) {
              g_unix_set_error_from_errno(&local_error, errsv);
              break;
          }
          string_buffer_size *= 2;
          continue;
      } else {
          g_unix_set_error_from_errno(&local_error, errsv);
          break;
      }
  } while (passwd_file_entry == NULL);
  g_assert (passwd_file_entry == NULL || (gpointer)passwd_file_entry == (gpointer)buffer);
  if (local_error != NULL) {
      g_clear_pointer(&buffer, g_free);
      g_propagate_error(error, g_steal_pointer(&local_error));
      errno = errsv;
  }
  return (struct passwd*)g_steal_pointer(&buffer);
}