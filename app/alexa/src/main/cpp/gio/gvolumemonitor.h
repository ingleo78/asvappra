#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_VOLUME_MONITOR_H__
#define __G_VOLUME_MONITOR_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_VOLUME_MONITOR  (g_volume_monitor_get_type())
#define G_VOLUME_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_VOLUME_MONITOR, GVolumeMonitor))
#define G_VOLUME_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_VOLUME_MONITOR, GVolumeMonitorClass))
#define G_VOLUME_MONITOR_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_VOLUME_MONITOR, GVolumeMonitorClass))
#define G_IS_VOLUME_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_VOLUME_MONITOR))
#define G_IS_VOLUME_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_VOLUME_MONITOR))
#define G_VOLUME_MONITOR_EXTENSION_POINT_NAME  "gio-volume-monitor"
typedef struct _GVolumeMonitorClass GVolumeMonitorClass;
struct _GVolumeMonitor {
  GObject parent_instance;
  gpointer priv;
};
struct _GVolumeMonitorClass {
  GObjectClass parent_class;
  void (*volume_added)(GVolumeMonitor *volume_monitor, GVolume *volume);
  void (*volume_removed)(GVolumeMonitor *volume_monitor, GVolume *volume);
  void (*volume_changed)(GVolumeMonitor *volume_monitor, GVolume *volume);
  void (*mount_added)(GVolumeMonitor *volume_monitor, GMount *mount);
  void (*mount_removed)(GVolumeMonitor *volume_monitor, GMount *mount);
  void (*mount_pre_unmount)(GVolumeMonitor *volume_monitor, GMount *mount);
  void (*mount_changed)(GVolumeMonitor *volume_monitor, GMount *mount);
  void (*drive_connected)(GVolumeMonitor *volume_monitor, GDrive *drive);
  void (*drive_disconnected)(GVolumeMonitor *volume_monitor, GDrive *drive);
  void (*drive_changed)(GVolumeMonitor *volume_monitor, GDrive *drive);
  gboolean (*is_supported)(void);
  GList *(*get_connected_drives)(GVolumeMonitor *volume_monitor);
  GList *(*get_volumes)(GVolumeMonitor *volume_monitor);
  GList *(*get_mounts)(GVolumeMonitor *volume_monitor);
  GVolume *(*get_volume_for_uuid)(GVolumeMonitor *volume_monitor, const char *uuid);
  GMount *(*get_mount_for_uuid)(GVolumeMonitor *volume_monitor, const char *uuid);
  GVolume *(*adopt_orphan_mount)(GMount *mount, GVolumeMonitor *volume_monitor);
  void (*drive_eject_button)(GVolumeMonitor *volume_monitor, GDrive *drive);
  void (*drive_stop_button)(GVolumeMonitor *volume_monitor, GDrive *drive);
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
  void (*_g_reserved6)(void);
};
GType g_volume_monitor_get_type(void) G_GNUC_CONST;
GVolumeMonitor *g_volume_monitor_get(void);
GList *g_volume_monitor_get_connected_drives(GVolumeMonitor *volume_monitor);
GList *g_volume_monitor_get_volume(GVolumeMonitor *volume_monitor);
GList *g_volume_monitor_get_mounts(GVolumeMonitor *volume_monitor);
GVolume *g_volume_monitor_get_volume_for_uuid(GVolumeMonitor *volume_monitor, const char *uuid);
GMount *g_volume_monitor_get_mount_for_uuid(GVolumeMonitor *volume_monitor, const char *uuid);
#ifndef G_DISABLE_DEPRECATED
GVolume *g_volume_monitor_adopt_orphan_mount(GMount *mount);
#endif
G_END_DECLS

#endif