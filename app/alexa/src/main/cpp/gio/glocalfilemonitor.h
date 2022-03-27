#ifndef __G_LOCAL_FILE_MONITOR_H__
#define __G_LOCAL_FILE_MONITOR_H__

#include "gfilemonitor.h"

G_BEGIN_DECLS
#define G_TYPE_LOCAL_FILE_MONITOR	(g_local_file_monitor_get_type())
#define G_LOCAL_FILE_MONITOR(o)	 (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_LOCAL_FILE_MONITOR, GLocalFileMonitor))
#define G_LOCAL_FILE_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_LOCAL_FILE_MONITOR, GLocalFileMonitorClass))
#define G_IS_LOCAL_FILE_MONITOR(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_LOCAL_FILE_MONITOR))
#define G_IS_LOCAL_FILE_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_LOCAL_FILE_MONITOR))
#define G_LOCAL_FILE_MONITOR_EXTENSION_POINT_NAME "gio-local-file-monitor"
#define GType gsize
typedef struct _GLocalFileMonitor GLocalFileMonitor;
typedef struct _GLocalFileMonitorClass GLocalFileMonitorClass;
struct _GLocalFileMonitor {
  GFileMonitor parent_instance;
  gchar *filename;
  GFileMonitorFlags flags;
};
struct _GLocalFileMonitorClass {
  GFileMonitorClass parent_class;
  gboolean (*is_supported) (void);
};
GType g_local_file_monitor_get_type(void) G_GNUC_CONST;
GFileMonitor *_g_local_file_monitor_new(const char *pathname, GFileMonitorFlags flags, GError **error);
G_END_DECLS

#endif