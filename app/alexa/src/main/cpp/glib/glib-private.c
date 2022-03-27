#include "../gio/config.h"
#include "glib.h"
#include "glib-private.h"
#include "glib-init.h"

GLibPrivateVTable *glib__private__(void) {
  static GLibPrivateVTable table = {
      g_wakeup_new,
      g_wakeup_free,
      g_wakeup_get_pollfd,
      g_wakeup_signal,
      g_wakeup_acknowledge,
      g_get_worker_context,
      g_check_setuid,
      g_main_context_new_with_next_id,
      g_dir_open_with_errno,
      g_dir_new_from_dirp,
      glib_init,
  #ifdef G_OS_WIN32
      g_win32_stat_utf8,
      g_win32_lstat_utf8,
      g_win32_readlink_utf8,
      g_win32_fstat,
  #endif
  };
  return &table;
}