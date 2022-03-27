#ifndef __G_UNIX_VOLUME_H__
#define __G_UNIX_VOLUME_H__

#include "giotypes.h"
#include "gunixvolumemonitor.h"
#include "gunixmounts.h"

G_BEGIN_DECLS
#define G_TYPE_UNIX_VOLUME  (_g_unix_volume_get_type())
#define G_UNIX_VOLUME(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_UNIX_VOLUME, GUnixVolume))
#define G_UNIX_VOLUME_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_UNIX_VOLUME, GUnixVolumeClass))
#define G_IS_UNIX_VOLUME(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_UNIX_VOLUME))
#define G_IS_UNIX_VOLUME_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_UNIX_VOLUME))
typedef struct _GUnixVolumeClass GUnixVolumeClass;
struct _GUnixVolumeClass {
  GObjectClass parent_class;
};
GType _g_unix_volume_get_type(void) G_GNUC_CONST;
GUnixVolume *_g_unix_volume_new(GVolumeMonitor *volume_monitor, GUnixMountPoint *mountpoint);
gboolean _g_unix_volume_has_mount_path(GUnixVolume *volume, const char *mount_path);
void _g_unix_volume_set_mount(GUnixVolume *volume, GUnixMount *mount);
void _g_unix_volume_unset_mount(GUnixVolume *volume, GUnixMount *mount);
void _g_unix_volume_disconnected(GUnixVolume *volume);
G_END_DECLS

#endif