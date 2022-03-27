#ifndef __G_FAM_FILE_MONITOR_H__
#define __G_FAM_FILE_MONITOR_H__

#include <string.h>
#include "../../glib/glib-object.h"
#include "../gfilemonitor.h"
#include "../glocalfilemonitor.h"
#include "../giomodule.h"

G_BEGIN_DECLS
#define G_TYPE_FAM_FILE_MONITOR	 (g_fam_file_monitor_get_type())
#define G_FAM_FILE_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_FAM_FILE_MONITOR, GFamFileMonitor))
#define G_FAM_FILE_MONITOR_CLASS(k)	 (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_FAM_FILE_MONITOR, GFamFileMonitorClass))
#define G_IS_FAM_FILE_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_FAM_FILE_MONITOR))
#define G_IS_FAM_FILE_MONITOR_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_FAM_FILE_MONITOR))
typedef struct _GFamFileMonitor GFamFileMonitor;
typedef struct _GFamFileMonitorClass GFamFileMonitorClass;
struct _GFamFileMonitorClass {
    GLocalFileMonitorClass parent_class;
};
GType g_fam_file_monitor_get_type(void);
void g_fam_file_monitor_register(GIOModule *module);
G_END_DECLS

#endif