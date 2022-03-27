#include <glib/glibintl.h>
#include "config.h"
#include "gvolumemonitor.h"
#include "gvolume.h"
#include "gmount.h"
#include "gdrive.h"

G_DEFINE_TYPE(GVolumeMonitor, g_volume_monitor, G_TYPE_OBJECT);
enum {
  VOLUME_ADDED,
  VOLUME_REMOVED,
  VOLUME_CHANGED,
  MOUNT_ADDED,
  MOUNT_REMOVED,
  MOUNT_PRE_UNMOUNT,
  MOUNT_CHANGED,
  DRIVE_CONNECTED,
  DRIVE_DISCONNECTED,
  DRIVE_CHANGED,
  DRIVE_EJECT_BUTTON,
  DRIVE_STOP_BUTTON,
  LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };
static void g_volume_monitor_finalize(GObject *object) {
  G_OBJECT_CLASS(g_volume_monitor_parent_class)->finalize (object);
}
static void g_volume_monitor_class_init(GVolumeMonitorClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_volume_monitor_finalize;
  signals[VOLUME_ADDED] = g_signal_new(I_("volume-added"), G_TYPE_VOLUME_MONITOR, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GVolumeMonitorClass, volume_added),
                                       NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_VOLUME);
  signals[VOLUME_REMOVED] = g_signal_new(I_("volume-removed"), G_TYPE_VOLUME_MONITOR, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GVolumeMonitorClass, volume_removed),
                                         NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_VOLUME);
  signals[VOLUME_CHANGED] = g_signal_new(I_("volume-changed"), G_TYPE_VOLUME_MONITOR, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GVolumeMonitorClass, volume_changed),
                                         NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_VOLUME);
  signals[MOUNT_ADDED] = g_signal_new(I_("mount-added"), G_TYPE_VOLUME_MONITOR, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GVolumeMonitorClass, mount_added),
                                      NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_MOUNT);
  signals[MOUNT_REMOVED] = g_signal_new(I_("mount-removed"), G_TYPE_VOLUME_MONITOR, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GVolumeMonitorClass, mount_removed),
                                        NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_MOUNT);
  signals[MOUNT_PRE_UNMOUNT] = g_signal_new(I_("mount-pre-unmount"), G_TYPE_VOLUME_MONITOR, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GVolumeMonitorClass,
                                            mount_pre_unmount), NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_MOUNT);
  signals[MOUNT_CHANGED] = g_signal_new(I_("mount-changed"), G_TYPE_VOLUME_MONITOR, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GVolumeMonitorClass, mount_changed),
                                        NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_MOUNT);
  signals[DRIVE_CONNECTED] = g_signal_new(I_("drive-connected"), G_TYPE_VOLUME_MONITOR, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GVolumeMonitorClass, drive_connected),
					                      NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_DRIVE);
  signals[DRIVE_DISCONNECTED] = g_signal_new(I_("drive-disconnected"), G_TYPE_VOLUME_MONITOR, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GVolumeMonitorClass,
					                         drive_disconnected), NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_DRIVE);
  signals[DRIVE_CHANGED] = g_signal_new(I_("drive-changed"), G_TYPE_VOLUME_MONITOR, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GVolumeMonitorClass, drive_changed),
                                        NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_DRIVE);
  signals[DRIVE_EJECT_BUTTON] = g_signal_new(I_("drive-eject-button"), G_TYPE_VOLUME_MONITOR, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GVolumeMonitorClass,
                                             drive_eject_button), NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_DRIVE);
  signals[DRIVE_STOP_BUTTON] = g_signal_new(I_("drive-stop-button"), G_TYPE_VOLUME_MONITOR, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GVolumeMonitorClass,
                                            drive_stop_button), NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_DRIVE);
}
static void g_volume_monitor_init(GVolumeMonitor *monitor) {}
GList *g_volume_monitor_get_connected_drives(GVolumeMonitor *volume_monitor) {
  GVolumeMonitorClass *class;
  g_return_val_if_fail(G_IS_VOLUME_MONITOR(volume_monitor), NULL);
  class = G_VOLUME_MONITOR_GET_CLASS(volume_monitor);
  return class->get_connected_drives(volume_monitor);
}
GList *g_volume_monitor_get_volumes(GVolumeMonitor *volume_monitor) {
  GVolumeMonitorClass *class;
  g_return_val_if_fail(G_IS_VOLUME_MONITOR(volume_monitor), NULL);
  class = G_VOLUME_MONITOR_GET_CLASS(volume_monitor);
  return class->get_volumes(volume_monitor);
}
GList *g_volume_monitor_get_mounts(GVolumeMonitor *volume_monitor) {
  GVolumeMonitorClass *class;
  g_return_val_if_fail(G_IS_VOLUME_MONITOR(volume_monitor), NULL);
  class = G_VOLUME_MONITOR_GET_CLASS(volume_monitor);
  return class->get_mounts(volume_monitor);
}
GVolume *g_volume_monitor_get_volume_for_uuid(GVolumeMonitor *volume_monitor, const char *uuid) {
  GVolumeMonitorClass *class;
  g_return_val_if_fail(G_IS_VOLUME_MONITOR(volume_monitor), NULL);
  g_return_val_if_fail(uuid != NULL, NULL);
  class = G_VOLUME_MONITOR_GET_CLASS(volume_monitor);
  return class->get_volume_for_uuid(volume_monitor, uuid);
}
GMount *g_volume_monitor_get_mount_for_uuid(GVolumeMonitor *volume_monitor, const char *uuid) {
  GVolumeMonitorClass *class;
  g_return_val_if_fail(G_IS_VOLUME_MONITOR(volume_monitor), NULL);
  g_return_val_if_fail(uuid != NULL, NULL);
  class = G_VOLUME_MONITOR_GET_CLASS(volume_monitor);
  return class->get_mount_for_uuid(volume_monitor, uuid);
}