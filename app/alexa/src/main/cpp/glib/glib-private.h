#ifndef __GLIB_PRIVATE_H__
#define __GLIB_PRIVATE_H__

#include "../gio/gioenumtypes.h"
#include "gwakeup.h"
#include "gstdioprivate.h"

GMainContext *g_get_worker_context(void);
gboolean g_check_setuid(void);
GMainContext *g_main_context_new_with_next_id(guint next_id);
#ifdef G_OS_WIN32
gchar *_glib_get_dll_directory(void);
GLIB_AVAILABLE_IN_ALL gchar *_glib_get_locale_dir(void);
#endif
GDir *g_dir_open_with_errno(const gchar *path, guint flags);
GDir *g_dir_new_from_dirp(gpointer dirp);
#define GLIB_PRIVATE_CALL(symbol)  (glib__private__()->symbol)
typedef struct {
  GWakeup *(*g_wakeup_new)(void);
  void (*g_wakeup_free)(GWakeup *wakeup);
  void (*g_wakeup_get_pollfd)(GWakeup *wakeup, GPollFD *poll_fd);
  void (*g_wakeup_signal)(GWakeup *wakeup);
  void (*g_wakeup_acknowledge)(GWakeup *wakeup);
  GMainContext *(*g_get_worker_context)(void);
  gboolean (*g_check_setuid)(void);
  GMainContext *(*g_main_context_new_with_next_id)(guint next_id);
  GDir *(*g_dir_open_with_errno)(const gchar *path, guint flags);
  GDir *(*g_dir_new_from_dirp)(gpointer dirp);
  void (*glib_init)(void);
#ifdef G_OS_WIN32
  int (*g_win32_stat_utf8)(const gchar *filename, GWin32PrivateStat *buf);
  int (*g_win32_lstat_utf8)(const gchar *filename, GWin32PrivateStat *buf);
  int (*g_win32_readlink_utf8)(const gchar *filename, gchar *buf, gsize buf_size, gchar **alloc_buf, gboolean terminate);
  int (*g_win32_fstat)(int fd, GWin32PrivateStat *buf);
#endif
} GLibPrivateVTable;
GLIB_AVAILABLE_IN_ALL GLibPrivateVTable *glib__private__(void);
#ifdef G_OS_WIN32
#define GLIB_DEFAULT_LOCALE ".ACP"
#else
#define GLIB_DEFAULT_LOCALE ""
#endif
#endif