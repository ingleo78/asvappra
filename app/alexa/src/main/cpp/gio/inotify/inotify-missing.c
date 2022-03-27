#include "../../glib/glib.h"
#include "../config.h"
#include "inotify-missing.h"
#include "inotify-path.h"

#define SCAN_MISSING_TIME 4
static gboolean im_debug_enabled = FALSE;
#define IM_W if (im_debug_enabled) g_warning
static GList *missing_sub_list = NULL;
static gboolean im_scan_missing (gpointer user_data);
static gboolean scan_missing_running = FALSE;
static void (*missing_cb)(inotify_sub *sub) = NULL;
G_LOCK_EXTERN (inotify_lock);
void _im_startup(void (*callback)(inotify_sub *sub)) {
  static gboolean initialized = FALSE;
  if (!initialized) {
      missing_cb = callback;
      initialized = TRUE;
  }
}
void _im_add(inotify_sub *sub) {
  if (g_list_find(missing_sub_list, sub)) {
      IM_W("asked to add %s to missing list but it's already on the list!\n", sub->dirname);
      return;
  }
  IM_W("adding %s to missing list\n", sub->dirname);
  missing_sub_list = g_list_prepend(missing_sub_list, sub);
  if (!scan_missing_running) {
      scan_missing_running = TRUE;
      g_timeout_add_seconds(SCAN_MISSING_TIME, im_scan_missing, NULL);
  }
}
void _im_rm(inotify_sub *sub) {
  GList *link;
  link = g_list_find (missing_sub_list, sub);
  if (!link) {
      IM_W("asked to remove %s from missing list but it isn't on the list!\n", sub->dirname);
      return;
  }
  IM_W("removing %s from missing list\n", sub->dirname);
  missing_sub_list = g_list_remove_link(missing_sub_list, link);
  g_list_free_1(link);
}
static gboolean im_scan_missing(gpointer user_data) {
  GList *nolonger_missing = NULL;
  GList *l;
  G_LOCK(inotify_lock);
  IM_W("scanning missing list with %d items\n", g_list_length(missing_sub_list));
  for (l = missing_sub_list; l; l = l->next) {
      inotify_sub *sub = l->data;
      gboolean not_m = FALSE;
      IM_W("checking %p\n", sub);
      g_assert(sub);
      g_assert(sub->dirname);
      not_m = _ip_start_watching(sub);
      if (not_m) {
          missing_cb(sub);
          nolonger_missing = g_list_prepend(nolonger_missing, l);
	  }
  }
  for (l = nolonger_missing; l ; l = l->next) {
      GList *llink = l->data;
      missing_sub_list = g_list_remove_link(missing_sub_list, llink);
      g_list_free_1(llink);
  }
  g_list_free(nolonger_missing);
  if (missing_sub_list == NULL) {
      scan_missing_running = FALSE;
      G_UNLOCK(inotify_lock);
      return FALSE;
  } else {
      G_UNLOCK(inotify_lock);
      return TRUE;
  }
}
void _im_diag_dump(GIOChannel *ioc) {
  GList *l;
  g_io_channel_write_chars(ioc, "missing list:\n", -1, NULL, NULL);
  for (l = missing_sub_list; l; l = l->next) {
      inotify_sub *sub = l->data;
      g_io_channel_write_chars(ioc, sub->dirname, -1, NULL, NULL);
      g_io_channel_write_chars(ioc, "\n", -1, NULL, NULL);
  }
}