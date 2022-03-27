#ifndef __G_WIN32_MOUNT_H__
#define __G_WIN32_MOUNT_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_WIN32_MOUNT  (_g_win32_mount_get_type())
#define G_WIN32_MOUNT(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_WIN32_MOUNT, GWin32Mount))
#define G_WIN32_MOUNT_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_WIN32_MOUNT, GWin32MountClass))
#define G_IS_WIN32_MOUNT(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_WIN32_MOUNT))
#define G_IS_WIN32_MOUNT_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_WIN32_MOUNT))
typedef struct _GWin32MountClass GWin32MountClass;
struct _GWin32MountClass {
  GObjectClass parent_class;
};
GType _g_win32_mount_get_type(void) G_GNUC_CONST;
GWin32Mount *_g_win32_mount_new(GVolumeMonitor *volume_monitor, const char *path, GWin32Volume *volume);
void _g_win32_mount_unset_volume(GWin32Mount *mount, GWin32Volume *volume);
void _g_win32_mount_unmounted(GWin32Mount *mount);
G_END_DECLS

#endif