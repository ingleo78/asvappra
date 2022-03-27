#ifndef __G_POLL_FILE_MONITOR_H__
#define __G_POLL_FILE_MONITOR_H__

#include "gfilemonitor.h"

G_BEGIN_DECLS
#define G_TYPE_POLL_FILE_MONITOR  (_g_poll_file_monitor_get_type())
#define G_POLL_FILE_MONITOR(o)	(G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_POLL_FILE_MONITOR, GPollFileMonitor))
#define G_POLL_FILE_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_POLL_FILE_MONITOR, GPollFileMonitorClass))
#define G_IS_POLL_FILE_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_POLL_FILE_MONITOR))
#define G_IS_POLL_FILE_MONITOR_CLASS(k)	 (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_POLL_FILE_MONITOR))
typedef struct _GPollFileMonitor GPollFileMonitor;
typedef struct _GPollFileMonitorClass GPollFileMonitorClass;
struct _GPollFileMonitorClass {
  GFileMonitorClass parent_class;
};
GType _g_poll_file_monitor_get_type(void) G_GNUC_CONST;
GFileMonitor *_g_poll_file_monitor_new(GFile *file);
G_END_DECLS

#endif