#ifndef __G_UNIX_MOUNT_H__
#define __G_UNIX_MOUNT_H__

#include "giotypes.h"
#include "gunixmounts.h"

G_BEGIN_DECLS
#define G_TYPE_UNIX_MOUNT  (_g_unix_mount_get_type())
#define G_UNIX_MOUNT(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_UNIX_MOUNT, GUnixMount))
#define G_UNIX_MOUNT_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_UNIX_MOUNT, GUnixMountClass))
#define G_IS_UNIX_MOUNT(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_UNIX_MOUNT))
#define G_IS_UNIX_MOUNT_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_UNIX_MOUNT))
typedef struct _GUnixMountClass GUnixMountClass;
struct _GUnixMountClass {
  GObjectClass parent_class;
};
GType _g_unix_mount_get_type(void) G_GNUC_CONST;
GUnixMount *_g_unix_mount_new(GVolumeMonitor *volume_monitor, GUnixMountEntry *mount_entry, GUnixVolume *volume);
gboolean _g_unix_mount_has_mount_path(GUnixMount *mount, const char *mount_path);
void _g_unix_mount_unset_volume(GUnixMount *mount, GUnixVolume *volume);
void _g_unix_mount_unmounted(GUnixMount *mount);
G_END_DECLS

#endif