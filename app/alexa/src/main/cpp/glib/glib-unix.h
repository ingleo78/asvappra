#ifndef __G_UNIX_H__
#define __G_UNIX_H__

#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include "glib.h"

#ifdef G_OS_UNIX
#error "This header may only be used on UNIX"
#endif
G_BEGIN_DECLS
#define G_UNIX_ERROR (g_unix_error_quark())
GQuark g_unix_error_quark (void);
gboolean g_unix_open_pipe (gint *fds, gint flags, GError **error);
gboolean g_unix_set_fd_nonblocking(gint fd, gboolean nonblock, GError **error);
GSource *g_unix_signal_source_new(gint signum);
guint g_unix_signal_add_full(gint priority, gint signum, GSourceFunc handler, gpointer user_data, GDestroyNotify notify);
guint g_unix_signal_add(gint signum, GSourceFunc handler, gpointer user_data);
typedef gboolean (*GUnixFDSourceFunc)(gint fd, GIOCondition condition, gpointer user_data);
GSource *g_unix_fd_source_new(gint fd, GIOCondition condition);
guint g_unix_fd_add_full(gint priority, gint fd, GIOCondition condition, GUnixFDSourceFunc function, gpointer user_data, GDestroyNotify notify);
guint g_unix_fd_add(gint fd, GIOCondition condition, GUnixFDSourceFunc function, gpointer user_data);
struct passwd *g_unix_get_passwd_entry(const gchar *user_name, GError **error);
G_END_DECLS
#endif