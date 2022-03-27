#include <fcntl.h>
#ifdef GLIB_COMPILATION
#include "gtypes.h"
#include "gpoll.h"
#else
#include "../gio/config.h"
#include "glib.h"
#endif
#include "gwakeup.h"
#ifdef _WIN32
#include <windows.h>

#ifdef GLIB_COMPILATION
#include "gmessages.h"
#include "giochannel.h"
#include "gwin32.h"
#endif
GWakeup *g_wakeup_new(void) {
  HANDLE wakeup;
  wakeup = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (wakeup == NULL) g_error("Cannot create event for GWakeup: %s", g_win32_error_message(GetLastError()));
  return (GWakeup*)wakeup;
}
void g_wakeup_get_pollfd(GWakeup *wakeup, GPollFD *poll_fd) {
  poll_fd->fd = (gintptr)wakeup;
  poll_fd->events = G_IO_IN;
}
void g_wakeup_acknowledge(GWakeup *wakeup) {
  ResetEvent((HANDLE)wakeup);
}
void g_wakeup_signal(GWakeup *wakeup) {
  SetEvent((HANDLE)wakeup);
}
void g_wakeup_free(GWakeup *wakeup) {
  CloseHandle((HANDLE)wakeup);
}
#else
#include "glib-unix.h"

#if defined (HAVE_EVENTFD)
#include <sys/eventfd.h>
#endif

struct _GWakeup {
  gint fds[2];
};
GWakeup *g_wakeup_new(void) {
  GError *error = NULL;
  GWakeup *wakeup;
  wakeup = g_slice_new(GWakeup);
#if defined (HAVE_EVENTFD)
#ifndef TEST_EVENTFD_FALLBACK
  wakeup->fds[0] = eventfd (0, EFD_CLOEXEC | EFD_NONBLOCK);
#else
  wakeup->fds[0] = -1;
#endif
  if (wakeup->fds[0] != -1) {
      wakeup->fds[1] = -1;
      return wakeup;
  }
#endif
  if (!g_unix_open_pipe(wakeup->fds, FD_CLOEXEC, &error)) g_error("Creating pipes for GWakeup: %s", error->message);
  if (!g_unix_set_fd_nonblocking(wakeup->fds[0], TRUE, &error) || !g_unix_set_fd_nonblocking(wakeup->fds[1], TRUE, &error))
      g_error("Set pipes non-blocking for GWakeup: %s", error->message);
  return wakeup;
}
void g_wakeup_get_pollfd(GWakeup *wakeup, GPollFD *poll_fd) {
  poll_fd->fd = wakeup->fds[0];
  poll_fd->events = G_IO_IN;
}
void g_wakeup_acknowledge(GWakeup *wakeup) {
  char buffer[16];
  while (read (wakeup->fds[0], buffer, sizeof buffer) == sizeof buffer);
}
void g_wakeup_signal(GWakeup *wakeup) {
  int res;
  if (wakeup->fds[1] == -1) {
      guint64 one = 1;
      do {
          res = write(wakeup->fds[0], &one, sizeof one);
      } while(G_UNLIKELY(res == -1 && errno == EINTR));
  } else {
      guint8 one = 1;
      do {
          res = write(wakeup->fds[1], &one, sizeof one);
      } while(G_UNLIKELY(res == -1 && errno == EINTR));
  }
}
void g_wakeup_free(GWakeup *wakeup) {
  close (wakeup->fds[0]);
  if (wakeup->fds[1] != -1) close (wakeup->fds[1]);
  g_slice_free (GWakeup, wakeup);
}
#endif