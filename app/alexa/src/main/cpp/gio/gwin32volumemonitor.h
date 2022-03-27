#ifndef __G_WIN32_VOLUME_MONITOR_H__
#define __G_WIN32_VOLUME_MONITOR_H__

#include "gnativevolumemonitor.h"

G_BEGIN_DECLS
#define G_TYPE_WIN32_VOLUME_MONITOR  (_g_win32_volume_monitor_get_type())
#define G_WIN32_VOLUME_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_WIN32_VOLUME_MONITOR, GWin32VolumeMonitor))
#define G_WIN32_VOLUME_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_WIN32_VOLUME_MONITOR, GWin32VolumeMonitorClass))
#define G_IS_WIN32_VOLUME_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_WIN32_VOLUME_MONITOR))
#define G_IS_WIN32_VOLUME_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_WIN32_VOLUME_MONITOR))
typedef struct _GWin32VolumeMonitor GWin32VolumeMonitor;
typedef struct _GWin32VolumeMonitorClass GWin32VolumeMonitorClass;
typedef struct _GWin32Mount GWin32Mount;
typedef struct _GWin32Volume GWin32Volume;
struct _GWin32VolumeMonitorClass {
  GNativeVolumeMonitorClass parent_class;
};
GType _g_win32_volume_monitor_get_type(void) G_GNUC_CONST;
GVolumeMonitor *_g_win32_volume_monitor_new(void);
GWin32Volume *_g_win32_volume_monitor_lookup_volume_for_mount_path(GWin32VolumeMonitor *monitor, const char *mount_path);

G_END_DECLS

#endif