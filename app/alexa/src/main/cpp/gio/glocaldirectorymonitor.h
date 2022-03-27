#ifndef __G_LOCAL_DIRECTORY_MONITOR_H__
#define __G_LOCAL_DIRECTORY_MONITOR_H__

#include "gfilemonitor.h"
#include "gunixmounts.h"

G_BEGIN_DECLS
#define G_TYPE_LOCAL_DIRECTORY_MONITOR	(g_local_directory_monitor_get_type())
#define G_LOCAL_DIRECTORY_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_LOCAL_DIRECTORY_MONITOR, GLocalDirectoryMonitor))
#define G_LOCAL_DIRECTORY_MONITOR_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_LOCAL_DIRECTORY_MONITOR, GLocalDirectoryMonitorClass))
#define G_IS_LOCAL_DIRECTORY_MONITOR(o)	 (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_LOCAL_DIRECTORY_MONITOR))
#define G_IS_LOCAL_DIRECTORY_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_LOCAL_DIRECTORY_MONITOR))
#define G_LOCAL_DIRECTORY_MONITOR_EXTENSION_POINT_NAME "gio-local-directory-monitor"
typedef struct _GLocalDirectoryMonitor GLocalDirectoryMonitor;
typedef struct _GLocalDirectoryMonitorClass GLocalDirectoryMonitorClass;
struct _GLocalDirectoryMonitor {
  GFileMonitor parent_instance;
  gchar *dirname;
  GFileMonitorFlags flags;
  GUnixMountMonitor *mount_monitor;
  gboolean was_mounted;
};
struct _GLocalDirectoryMonitorClass {
  GFileMonitorClass parent_class;
  gboolean mount_notify;
  gboolean (*is_supported)(void);
};
GType g_local_directory_monitor_get_type(void) G_GNUC_CONST;
GFileMonitor *_g_local_directory_monitor_new(const char *dirname, GFileMonitorFlags flags, GError **error);
G_END_DECLS

#endif