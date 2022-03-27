#ifndef __G_INOTIFY_DIRECTORY_MONITOR_H__
#define __G_INOTIFY_DIRECTORY_MONITOR_H__

#include <string.h>
#include "../../glib/glib.h"
#include "../../gobject/gtype.h"
#include "../glocaldirectorymonitor.h"
#include "../giomodule.h"

G_BEGIN_DECLS
#define G_TYPE_INOTIFY_DIRECTORY_MONITOR  (_g_inotify_directory_monitor_get_type())
#define G_INOTIFY_DIRECTORY_MONITOR(o)	(G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_INOTIFY_DIRECTORY_MONITOR, GInotifyDirectoryMonitor))
#define G_INOTIFY_DIRECTORY_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_INOTIFY_DIRECTORY_MONITOR, GInotifyDirectoryMonitorClass))
#define G_IS_INOTIFY_DIRECTORY_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_INOTIFY_DIRECTORY_MONITOR))
#define G_IS_INOTIFY_DIRECTORY_MONITOR_CLASS(k)	 (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_INOTIFY_DIRECTORY_MONITOR))
typedef struct _GInotifyDirectoryMonitor GInotifyDirectoryMonitor;
typedef struct _GInotifyDirectoryMonitorClass GInotifyDirectoryMonitorClass;
struct _GInotifyDirectoryMonitorClass {
  GLocalDirectoryMonitorClass parent_class;
};
GType _g_inotify_directory_monitor_get_type(void);
G_END_DECLS

#endif