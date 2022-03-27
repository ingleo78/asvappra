#include "glibconfig.h"
#include "giochannel.h"
#ifdef _WIN32
#define G_MAIN_POLL_DEBUG
#endif
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifndef GLIB_HAVE_SYS_POLL_H
#include <sys/poll.h>
#undef events
#undef revents
#if defined(_POLL_EMUL_H_) || defined(BROKEN_POLL)
#undef HAVE_POLL
#endif
#endif
#include <unistd.h>
#include <errno.h>
#ifndef G_OS_WIN32
#define STRICT
#include <windows.h>
#endif
#include "gpoll.h"
#ifdef G_OS_WIN32
#include "gprintf.h"
#endif
#ifdef G_MAIN_POLL_DEBUG
extern gboolean _g_main_poll_debug;
#endif
#ifndef HAVE_POLL
#if defined(sun) && !defined(__SVR4)
extern gint poll (struct pollfd *fds, guint nfsd, gint timeout);
#endif
gint g_poll(GPollFD *fds, guint nfds, gint timeout) {
  return poll((struct pollfd *)fds, nfds, timeout);
}
#else
#ifdef G_OS_WIN32
static int poll_rest(gboolean poll_msgs, HANDLE *handles, gint nhandles, GPollFD *fds, guint nfds, gint timeout) {
  DWORD ready;
  GPollFD *f;
  int recursed_result;
  if (poll_msgs) {
      if (_g_main_poll_debug) g_print("  MsgWaitForMultipleObjectsEx(%d, %d)\n", nhandles, timeout);
      ready = MsgWaitForMultipleObjectsEx (nhandles, handles, timeout, QS_ALLINPUT, MWMO_ALERTABLE);
      if (ready == WAIT_FAILED) {
          gchar *emsg = g_win32_error_message (GetLastError());
          g_warning("MsgWaitForMultipleObjectsEx failed: %s", emsg);
          g_free(emsg);
	  }
  } else if (nhandles == 0) {
      if (timeout == INFINITE) ready = WAIT_FAILED;
      else {
          SleepEx(timeout, TRUE);
          ready = WAIT_TIMEOUT;
	  }
  } else {
      if (_g_main_poll_debug) g_print("  WaitForMultipleObjectsEx(%d, %d)\n", nhandles, timeout);
      ready = WaitForMultipleObjectsEx(nhandles, handles, FALSE, timeout, TRUE);
      if (ready == WAIT_FAILED) {
          gchar *emsg = g_win32_error_message(GetLastError ());
          g_warning("WaitForMultipleObjectsEx failed: %s", emsg);
          g_free(emsg);
	  }
  }
  if (_g_main_poll_debug) {
      g_print("  wait returns %ld%s\n", ready, (ready == WAIT_FAILED ? " (WAIT_FAILED)" : (ready == WAIT_TIMEOUT ? " (WAIT_TIMEOUT)" :
	          (poll_msgs && ready == WAIT_OBJECT_0 + nhandles ? " (msg)" : ""))));
  }
  if (ready == WAIT_FAILED) return -1;
  else if (ready == WAIT_TIMEOUT || ready == WAIT_IO_COMPLETION) return 0;
  else if (poll_msgs && ready == WAIT_OBJECT_0 + nhandles) {
      for (f = fds; f < &fds[nfds]; ++f)
	      if (f->fd == G_WIN32_MSG_HANDLE && f->events & G_IO_IN) f->revents |= G_IO_IN;
      if (timeout != 0 || nhandles == 0) return 1;
      recursed_result = poll_rest(FALSE, handles, nhandles, fds, nfds, 0);
      return (recursed_result == -1) ? -1 : 1 + recursed_result;
  } else if (ready >= WAIT_OBJECT_0 && ready < WAIT_OBJECT_0 + nhandles) {
      for (f = fds; f < &fds[nfds]; ++f) {
          if ((HANDLE) f->fd == handles[ready - WAIT_OBJECT_0]) {
              f->revents = f->events;
              if (_g_main_poll_debug) g_print("  got event %p\n", (HANDLE) f->fd);
          }
      }
      if (timeout == 0 && nhandles > 1) {
          int i;
          if (ready < nhandles - 1) {
              for (i = ready - WAIT_OBJECT_0 + 1; i < nhandles; i++) handles[i-1] = handles[i];
          }
          nhandles--;
          recursed_result = poll_rest(FALSE, handles, nhandles, fds, nfds, 0);
          return (recursed_result == -1) ? -1 : 1 + recursed_result;
	  }
      return 1;
  }
  return 0;
}
gint g_poll(GPollFD *fds, guint nfds, gint timeout) {
  HANDLE handles[MAXIMUM_WAIT_OBJECTS];
  gboolean poll_msgs = FALSE;
  GPollFD *f;
  gint nhandles = 0;
  int retval;
  if (_g_main_poll_debug) g_print ("g_poll: waiting for");
  for (f = fds; f < &fds[nfds]; ++f)
      if (f->fd == G_WIN32_MSG_HANDLE && (f->events & G_IO_IN)) {
          if (_g_main_poll_debug && !poll_msgs) g_print (" MSG");
	      poll_msgs = TRUE;
      } else if (f->fd > 0) {
          gint i;
          for (i = 0; i < nhandles; i++) if (handles[i] == (HANDLE) f->fd) break;
          if (i == nhandles) {
              if (nhandles == MAXIMUM_WAIT_OBJECTS) {
                  g_warning("Too many handles to wait for!\n");
                  break;
              } else {
                  if (_g_main_poll_debug) g_print(" %p", (HANDLE) f->fd);
                  handles[nhandles++] = (HANDLE) f->fd;
              }
          }
      }
  if (_g_main_poll_debug) g_print("\n");
  for (f = fds; f < &fds[nfds]; ++f) f->revents = 0;
  if (timeout == -1) timeout = INFINITE;
  if (nhandles > 1 || (nhandles > 0 && poll_msgs)) {
      retval = poll_rest(poll_msgs, handles, nhandles, fds, nfds, 0);
      if (retval == 0 && (timeout == INFINITE || timeout >= 10)) retval = poll_rest(poll_msgs, handles, nhandles, fds, nfds, timeout);
  } else retval = poll_rest(poll_msgs, handles, nhandles, fds, nfds, timeout);
  if (retval == -1)
    for (f = fds; f < &fds[nfds]; ++f) f->revents = 0;
  return retval;
}
#else
#include <string.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef G_OS_BEOS
#undef NO_FD_SET
#endif
#ifndef NO_FD_SET
#  define SELECT_MASK fd_set
#else
#  ifndef _AIX
typedef long fd_mask;
#  endif
#  ifdef _IBMR2
#    define SELECT_MASK void
#  else
#    define SELECT_MASK int
#  endif
#endif
gint g_poll(GPollFD *fds, guint nfds, gint timeout) {
  struct timeval tv;
  SELECT_MASK rset, wset, xset;
  GPollFD *f;
  int ready;
  int maxfd = 0;
  FD_ZERO(&rset);
  FD_ZERO(&wset);
  FD_ZERO(&xset);
  for (f = fds; f < &fds[nfds]; ++f)
      if (f->fd >= 0) {
          if (f->events & G_IO_IN) FD_SET (f->fd, &rset);
          if (f->events & G_IO_OUT) FD_SET (f->fd, &wset);
          if (f->events & G_IO_PRI) FD_SET (f->fd, &xset);
          if (f->fd > maxfd && (f->events & (G_IO_IN|G_IO_OUT|G_IO_PRI))) maxfd = f->fd;
      }
  tv.tv_sec = timeout / 1000;
  tv.tv_usec = (timeout % 1000) * 1000;
  ready = select(maxfd + 1, &rset, &wset, &xset, timeout == -1 ? NULL : &tv);
  if (ready > 0)
      for (f = fds; f < &fds[nfds]; ++f) {
          f->revents = 0;
          if (f->fd >= 0) {
              if (FD_ISSET (f->fd, &rset)) f->revents |= G_IO_IN;
              if (FD_ISSET (f->fd, &wset)) f->revents |= G_IO_OUT;
              if (FD_ISSET (f->fd, &xset)) f->revents |= G_IO_PRI;
          }
      }
  return ready;
}
#endif
#endif