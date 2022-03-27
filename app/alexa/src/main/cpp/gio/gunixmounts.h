#ifndef __G_UNIX_MOUNTS_H__
#define __G_UNIX_MOUNTS_H__

G_BEGIN_DECLS
typedef struct _GUnixMountEntry GUnixMountEntry;
typedef struct _GUnixMountPoint GUnixMountPoint;
typedef struct _GUnixMountMonitor GUnixMountMonitor;
typedef struct _GUnixMountMonitorClass GUnixMountMonitorClass;
#define G_TYPE_UNIX_MOUNT_MONITOR  (g_unix_mount_monitor_get_type())
#define G_UNIX_MOUNT_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_UNIX_MOUNT_MONITOR, GUnixMountMonitor))
#define G_UNIX_MOUNT_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_UNIX_MOUNT_MONITOR, GUnixMountMonitorClass))
#define G_IS_UNIX_MOUNT_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_UNIX_MOUNT_MONITOR))
#define G_IS_UNIX_MOUNT_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_UNIX_MOUNT_MONITOR))
void g_unix_mount_free(GUnixMountEntry *mount_entry);
void g_unix_mount_point_free(GUnixMountPoint *mount_point);
gint g_unix_mount_compare(GUnixMountEntry *mount1, GUnixMountEntry *mount2);
const char *g_unix_mount_get_mount_path(GUnixMountEntry *mount_entry);
const char *g_unix_mount_get_device_path(GUnixMountEntry *mount_entry);
const char *g_unix_mount_get_fs_type(GUnixMountEntry *mount_entry);
gboolean g_unix_mount_is_readonly(GUnixMountEntry *mount_entry);
gboolean g_unix_mount_is_system_internal(GUnixMountEntry *mount_entry);
gboolean g_unix_mount_guess_can_eject(GUnixMountEntry *mount_entry);
gboolean g_unix_mount_guess_should_display(GUnixMountEntry *mount_entry);
char *g_unix_mount_guess_name(GUnixMountEntry *mount_entry);
GIcon *g_unix_mount_guess_icon(GUnixMountEntry *mount_entry);
gint g_unix_mount_point_compare(GUnixMountPoint *mount1, GUnixMountPoint *mount2);
const char *g_unix_mount_point_get_mount_path(GUnixMountPoint *mount_point);
const char *g_unix_mount_point_get_device_path(GUnixMountPoint *mount_point);
const char *g_unix_mount_point_get_fs_type(GUnixMountPoint *mount_point);
gboolean g_unix_mount_point_is_readonly(GUnixMountPoint *mount_point);
gboolean g_unix_mount_point_is_user_mountable(GUnixMountPoint *mount_point);
gboolean g_unix_mount_point_is_loopback(GUnixMountPoint *mount_point);
gboolean g_unix_mount_point_guess_can_eject(GUnixMountPoint *mount_point);
char *g_unix_mount_point_guess_name(GUnixMountPoint *mount_point);
GIcon *g_unix_mount_point_guess_icon(GUnixMountPoint *mount_point);
GList *g_unix_mount_points_get(guint64 *time_read);
GList *g_unix_mounts_get(guint64 *time_read);
GUnixMountEntry *g_unix_mount_at(const char *mount_path, guint64 *time_read);
gboolean g_unix_mounts_changed_since(guint64 time);
gboolean g_unix_mount_points_changed_since(guint64 time);
GType g_unix_mount_monitor_get_type(void) G_GNUC_CONST;
GUnixMountMonitor *g_unix_mount_monitor_new(void);
void g_unix_mount_monitor_set_rate_limit (GUnixMountMonitor *mount_monitor, int limit_msec);
gboolean g_unix_is_mount_path_system_internal(const char *mount_path);
G_END_DECLS

#endif