#ifndef __G_UNION_VOLUME_MONITOR_H__
#define __G_UNION_VOLUME_MONITOR_H__

#include "../gobject/gobject.h"
#include "gvolumemonitor.h"

G_BEGIN_DECLS
#define G_TYPE_UNION_VOLUME_MONITOR  (_g_union_volume_monitor_get_type())
#define G_UNION_VOLUME_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_UNION_VOLUME_MONITOR, GUnionVolumeMonitor))
#define G_UNION_VOLUME_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_UNION_VOLUME_MONITOR, GUnionVolumeMonitorClass))
#define G_IS_UNION_VOLUME_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_UNION_VOLUME_MONITOR))
#define G_IS_UNION_VOLUME_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_UNION_VOLUME_MONITOR))
typedef struct _GUnionVolumeMonitor GUnionVolumeMonitor;
typedef struct _GUnionVolumeMonitorClass GUnionVolumeMonitorClass;
struct _GUnionVolumeMonitorClass {
  GVolumeMonitorClass parent_class;
};
GType _g_union_volume_monitor_get_type(void) G_GNUC_CONST;
G_END_DECLS

#endif