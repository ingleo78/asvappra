#if !defined (__GLIB_H_INSIDE__) && !defined (__G_MAIN_H__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_POLL_H__
#define __G_POLL_H__

#include "gmacros.h"
#include "gtypes.h"

G_BEGIN_DECLS
typedef struct _GPollFD GPollFD;
typedef unsigned short gushort;
typedef gint (*GPollFunc)(GPollFD *ufds, guint nfsd, gint timeout_);
struct _GPollFD {
#if defined (G_OS_WIN32) && GLIB_SIZEOF_VOID_P == 8
  gint64 fd;
#else
  gint	fd;
#endif
  gushort events;
  gushort revents;
};
#ifdef G_OS_WIN32
#if GLIB_SIZEOF_VOID_P == 8
#define G_POLLFD_FORMAT "%#I64x"
#else
#define G_POLLFD_FORMAT "%#x"
#endif
#else
#define G_POLLFD_FORMAT "%d"
#endif
gint g_poll(GPollFD *fds, guint nfds, gint timeout);
G_END_DECLS

#endif