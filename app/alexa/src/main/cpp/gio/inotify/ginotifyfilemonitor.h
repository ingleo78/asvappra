#ifndef __G_INOTIFY_FILE_MONITOR_H__
#define __G_INOTIFY_FILE_MONITOR_H__

#include <string.h>
#include "../../glib/glib-object.h"
#include "../gfilemonitor.h"
#include "../glocalfilemonitor.h"
#include "../giomodule.h"

G_BEGIN_DECLS
#define G_TYPE_INOTIFY_FILE_MONITOR	 (_g_inotify_file_monitor_get_type())
#define G_INOTIFY_FILE_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_INOTIFY_FILE_MONITOR, GInotifyFileMonitor))
#define G_INOTIFY_FILE_MONITOR_CLASS(k)	 (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_INOTIFY_FILE_MONITOR, GInotifyFileMonitorClass))
#define G_IS_INOTIFY_FILE_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_INOTIFY_FILE_MONITOR))
#define G_IS_INOTIFY_FILE_MONITOR_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_INOTIFY_FILE_MONITOR))
typedef struct _GInotifyFileMonitor GInotifyFileMonitor;
typedef struct _GInotifyFileMonitorClass GInotifyFileMonitorClass;
struct _GInotifyFileMonitorClass {
  GLocalFileMonitorClass parent_class;
};
GType _g_inotify_file_monitor_get_type(void);
G_END_DECLS

#endif