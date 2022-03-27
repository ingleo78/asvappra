#include "../config.h"
#include "../giomodule.h"
#include "ginotifydirectorymonitor.h"
#define USE_INOTIFY 1
#include "inotify-helper.h"

struct _GInotifyDirectoryMonitor {
  GLocalDirectoryMonitor parent_instance;
  inotify_sub *sub;
};
static gboolean g_inotify_directory_monitor_cancel (GFileMonitor* monitor);
G_DEFINE_TYPE_WITH_CODE(GInotifyDirectoryMonitor, g_inotify_directory_monitor, G_TYPE_LOCAL_DIRECTORY_MONITOR,
			       g_io_extension_point_implement(G_LOCAL_DIRECTORY_MONITOR_EXTENSION_POINT_NAME, g_define_type_id, "inotify", 20))
static void g_inotify_directory_monitor_finalize(GObject *object) {
  GInotifyDirectoryMonitor *inotify_monitor = G_INOTIFY_DIRECTORY_MONITOR (object);
  inotify_sub *sub = inotify_monitor->sub;
  if (sub) {
      _ih_sub_cancel(sub);
      _ih_sub_free(sub);
      inotify_monitor->sub = NULL;
  }
  if (G_OBJECT_CLASS (g_inotify_directory_monitor_parent_class)->finalize) (*G_OBJECT_CLASS(g_inotify_directory_monitor_parent_class)->finalize)(object);
}
static GObject *g_inotify_directory_monitor_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_properties) {
  GObject *obj;
  GInotifyDirectoryMonitorClass *klass;
  GObjectClass *parent_class;
  GInotifyDirectoryMonitor *inotify_monitor;
  const gchar *dirname = NULL;
  inotify_sub *sub = NULL;
  gboolean ret_ih_startup;
  gboolean pair_moves;
  klass = G_INOTIFY_DIRECTORY_MONITOR_CLASS(g_type_class_peek(G_TYPE_INOTIFY_DIRECTORY_MONITOR));
  parent_class = G_OBJECT_CLASS(g_type_class_peek_parent(klass));
  obj = parent_class->constructor(type, n_construct_properties, construct_properties);
  inotify_monitor = G_INOTIFY_DIRECTORY_MONITOR(obj);
  dirname = G_LOCAL_DIRECTORY_MONITOR(obj)->dirname;
  g_assert(dirname != NULL);
  ret_ih_startup = _ih_startup();
  g_assert(ret_ih_startup);
  pair_moves = G_LOCAL_DIRECTORY_MONITOR(obj)->flags & G_FILE_MONITOR_SEND_MOVED;
  sub = _ih_sub_new(dirname, NULL, pair_moves, inotify_monitor);
  g_assert(sub != NULL);
  _ih_sub_add(sub);
  inotify_monitor->sub = sub;
  return obj;
}
static gboolean g_inotify_directory_monitor_is_supported(void) {
  return _ih_startup();
}
static void g_inotify_directory_monitor_class_init(GInotifyDirectoryMonitorClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
  GFileMonitorClass *directory_monitor_class = G_FILE_MONITOR_CLASS(klass);
  GLocalDirectoryMonitorClass *local_directory_monitor_class = G_LOCAL_DIRECTORY_MONITOR_CLASS(klass);
  gobject_class->finalize = g_inotify_directory_monitor_finalize;
  gobject_class->constructor = g_inotify_directory_monitor_constructor;
  directory_monitor_class->cancel = g_inotify_directory_monitor_cancel;
  local_directory_monitor_class->mount_notify = TRUE;
  local_directory_monitor_class->is_supported = g_inotify_directory_monitor_is_supported;
}
static void g_inotify_directory_monitor_init(GInotifyDirectoryMonitor* monitor) {}
static gboolean g_inotify_directory_monitor_cancel (GFileMonitor* monitor) {
  GInotifyDirectoryMonitor *inotify_monitor = G_INOTIFY_DIRECTORY_MONITOR (monitor);
  inotify_sub *sub = inotify_monitor->sub;
  if (sub) {
      _ih_sub_cancel(sub);
      _ih_sub_free(sub);
      inotify_monitor->sub = NULL;
  }
  if (G_FILE_MONITOR_CLASS(g_inotify_directory_monitor_parent_class)->cancel) (*G_FILE_MONITOR_CLASS(g_inotify_directory_monitor_parent_class)->cancel)(monitor);
  return TRUE;
}