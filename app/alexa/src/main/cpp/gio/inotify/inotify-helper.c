#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include "../config.h"
#include "../glocalfile.h"
#include "../gfilemonitor.h"
#include "../gfile.h"
#include "inotify-helper.h"
#include "inotify-missing.h"
#include "inotify-path.h"
#include "inotify-diag.h"

static gboolean ih_debug_enabled = FALSE;
#define IH_W if (ih_debug_enabled) g_warning
static void ih_event_callback (ik_event_t *event, inotify_sub *sub);
static void ih_not_missing_callback (inotify_sub *sub);
G_GNUC_INTERNAL G_LOCK_DEFINE (inotify_lock);
static GFileMonitorEvent ih_mask_to_EventFlags (guint32 mask);
gboolean _ih_startup(void) {
  static gboolean initialized = FALSE;
  static gboolean result = FALSE;
  G_LOCK(inotify_lock);
  if (initialized == TRUE) {
      G_UNLOCK(inotify_lock);
      return result;
  }
  result = _ip_startup(ih_event_callback);
  if (!result) {
      G_UNLOCK(inotify_lock);
      return FALSE;
  }
  _im_startup(ih_not_missing_callback);
  _id_startup();
  IH_W("started gvfs inotify backend\n");
  initialized = TRUE;
  G_UNLOCK(inotify_lock);
  return TRUE;
}
gboolean _ih_sub_add(inotify_sub *sub) {
  G_LOCK(inotify_lock);
  if (!_ip_start_watching(sub)) _im_add(sub);
  G_UNLOCK(inotify_lock);
  return TRUE;
}
gboolean _ih_sub_cancel(inotify_sub *sub) {
  G_LOCK(inotify_lock);
  if (!sub->cancelled) {
      IH_W("cancelling %s\n", sub->dirname);
      sub->cancelled = TRUE;
      _im_rm(sub);
      _ip_stop_watching(sub);
  }
  G_UNLOCK(inotify_lock);
  return TRUE;
}
static char *_ih_fullpath_from_event(ik_event_t *event, const char *dirname) {
  char *fullpath;
  if (event->name) fullpath = g_strdup_printf("%s/%s", dirname, event->name);
  else fullpath = g_strdup_printf("%s/", dirname);
  return fullpath;
}
static gboolean ih_event_is_paired_move(ik_event_t *event) {
  if (event->pair) {
      ik_event_t *paired = event->pair;
      return (event->mask | paired->mask) & IN_MOVE;
  }
  return FALSE;
}
static void ih_event_callback(ik_event_t *event, inotify_sub *sub) {
  gchar *fullpath;
  GFileMonitorEvent eflags;
  GFile* child;
  GFile* other;
  eflags = ih_mask_to_EventFlags(event->mask);
  fullpath = _ih_fullpath_from_event(event, sub->dirname);
  child = g_file_new_for_path(fullpath);
  g_free(fullpath);
  if (ih_event_is_paired_move(event) && sub->pair_moves) {
      const char *parent_dir = (char*)_ip_get_path_for_wd(event->pair->wd);
      fullpath = _ih_fullpath_from_event(event->pair, parent_dir);
      other = g_file_new_for_path(fullpath);
      g_free(fullpath);
      eflags = G_FILE_MONITOR_EVENT_MOVED;
      event->pair = NULL;
  } else other = NULL;
  g_file_monitor_emit_event(G_FILE_MONITOR(sub->user_data), child, other, eflags);
  g_object_unref(child);
  if (other) g_object_unref(other);
}
static void ih_not_missing_callback(inotify_sub *sub) {
  gchar *fullpath;
  GFileMonitorEvent eflags;
  guint32 mask;
  GFile* child;
  if (sub->filename) {
      fullpath = g_strdup_printf("%s/%s", sub->dirname, sub->filename);
      g_warning("Missing callback called fullpath = %s\n", fullpath);
      if (!g_file_test(fullpath, G_FILE_TEST_EXISTS)) {
          g_free(fullpath);
          return;
	  }
      mask = IN_CREATE;
  } else {
      fullpath = g_strdup_printf ("%s", sub->dirname);
      mask = IN_CREATE|IN_ISDIR;
  }
  eflags = ih_mask_to_EventFlags(mask);
  child = g_file_new_for_path(fullpath);
  g_free(fullpath);
  g_file_monitor_emit_event(G_FILE_MONITOR(sub->user_data), child, NULL, eflags);
  g_object_unref(child);
}
static GFileMonitorEvent ih_mask_to_EventFlags(guint32 mask) {
  mask &= ~IN_ISDIR;
  switch(mask) {
      case IN_MODIFY: return G_FILE_MONITOR_EVENT_CHANGED;
      case IN_CLOSE_WRITE: return G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT;
      case IN_ATTRIB: return G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED;
      case IN_MOVE_SELF: case IN_MOVED_FROM: case IN_DELETE: case IN_DELETE_SELF: return G_FILE_MONITOR_EVENT_DELETED;
      case IN_CREATE: case IN_MOVED_TO: return G_FILE_MONITOR_EVENT_CREATED;
      case IN_UNMOUNT: return G_FILE_MONITOR_EVENT_UNMOUNTED;
      default: return -1;
  }
}