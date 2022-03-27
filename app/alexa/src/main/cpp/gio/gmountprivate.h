#ifndef __G_VOLUMEPRIV_H__
#define __G_VOLUMEPRIV_H__

#include "gvolume.h"

G_BEGIN_DECLS
GMount *_g_mount_get_for_mount_path(const char *mount_path, GCancellable *cancellable);
G_END_DECLS

#endif