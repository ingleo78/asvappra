#ifndef __G_UNIX_VOLUME_MONITOR_H__
#define __G_UNIX_VOLUME_MONITOR_H__

#include "gnativevolumemonitor.h"

G_BEGIN_DECLS
#define G_TYPE_UNIX_VOLUME_MONITOR  (_g_unix_volume_monitor_get_type())
#define G_UNIX_VOLUME_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_UNIX_VOLUME_MONITOR, GUnixVolumeMonitor))
#define G_UNIX_VOLUME_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_UNIX_VOLUME_MONITOR, GUnixVolumeMonitorClass))
#define G_IS_UNIX_VOLUME_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_UNIX_VOLUME_MONITOR))
#define G_IS_UNIX_VOLUME_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_UNIX_VOLUME_MONITOR))
typedef struct _GUnixVolumeMonitor GUnixVolumeMonitor;
typedef struct _GUnixVolumeMonitorClass GUnixVolumeMonitorClass;
typedef struct _GUnixMount GUnixMount;
typedef struct _GUnixVolume GUnixVolume;
struct _GUnixVolumeMonitorClass {
  GNativeVolumeMonitorClass parent_class;
};
GType _g_unix_volume_monitor_get_type(void) G_GNUC_CONST;
GVolumeMonitor *_g_unix_volume_monitor_new(void);
GUnixVolume *_g_unix_volume_monitor_lookup_volume_for_mount_path(GUnixVolumeMonitor *monitor, const char *mount_path);
G_END_DECLS

#endif