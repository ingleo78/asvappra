#ifndef __G_FAM_DIRECTORY_MONITOR_H__
#define __G_FAM_DIRECTORY_MONITOR_H__

#include <string.h>
#include "../../glib/glib-object.h"
#include "../../gobject/gtype.h"
#include "../../gio/glocaldirectorymonitor.h"
#include "../../gio/giomodule.h"
#include "../../gio/giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_FAM_DIRECTORY_MONITOR  (g_fam_directory_monitor_get_type())
#define G_FAM_DIRECTORY_MONITOR(o)	(G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_FAM_DIRECTORY_MONITOR, GFamDirectoryMonitor))
#define G_FAM_DIRECTORY_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_FAM_DIRECTORY_MONITOR, GFamDirectoryMonitorClass))
#define G_IS_FAM_DIRECTORY_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_FAM_DIRECTORY_MONITOR))
#define G_IS_FAM_DIRECTORY_MONITOR_CLASS(k)	 (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_FAM_DIRECTORY_MONITOR))
typedef struct _GFamDirectoryMonitor GFamDirectoryMonitor;
typedef struct _GFamDirectoryMonitorClass GFamDirectoryMonitorClass;
struct _GFamDirectoryMonitorClass {
    GLocalDirectoryMonitorClass parent_class;
};
GType g_fam_directory_monitor_get_type(void);
void g_fam_directory_monitor_register(GIOModule *module);
G_END_DECLS

#endif