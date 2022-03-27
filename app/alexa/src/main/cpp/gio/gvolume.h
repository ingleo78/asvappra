#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_VOLUME_H__
#define __G_VOLUME_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_VOLUME_IDENTIFIER_KIND_HAL_UDI "hal-udi"
#define G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE  "unix-device"
#define G_VOLUME_IDENTIFIER_KIND_LABEL  "label"
#define G_VOLUME_IDENTIFIER_KIND_UUID  "uuid"
#define G_VOLUME_IDENTIFIER_KIND_NFS_MOUNT  "nfs-mount"
#define G_TYPE_VOLUME  (g_volume_get_type())
#define G_VOLUME(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_VOLUME, GVolume))
#define G_IS_VOLUME(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_VOLUME))
#define G_VOLUME_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_VOLUME, GVolumeIface))
typedef struct _GVolumeIface GVolumeIface;
struct _GVolumeIface {
  GTypeInterface g_iface;
  void (*changed)(GVolume *volume);
  void (*removed)(GVolume *volume);
  char *(*get_name)(GVolume *volume);
  GIcon *(*get_icon)(GVolume *volume);
  char *(*get_uuid)(GVolume *volume);
  GDrive *(*get_drive)(GVolume *volume);
  GMount *(*get_mount)(GVolume *volume);
  gboolean (*can_mount)(GVolume *volume);
  gboolean (*can_eject)(GVolume *volume);
  void (*mount_fn)(GVolume *volume, GMountMountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                   gpointer user_data);
  gboolean (*mount_finish)(GVolume *volume, GAsyncResult *result, GError **error);
  void (*eject)(GVolume *volume, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*eject_finish)(GVolume *volume, GAsyncResult *result, GError **error);
  char *(*get_identifier)(GVolume *volume, const char *kind);
  char **(*enumerate_identifiers)(GVolume *volume);
  gboolean (*should_automount)(GVolume *volume);
  GFile *(*get_activation_root)(GVolume *volume);
  void (*eject_with_operation)(GVolume *volume, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback  callback,
                               gpointer user_data);
  gboolean (*eject_with_operation_finish)(GVolume *volume, GAsyncResult *result, GError **error);
};
GType g_volume_get_type(void) G_GNUC_CONST;
char *g_volume_get_name(GVolume *volume);
GIcon *g_volume_get_icon(GVolume *volume);
char *g_volume_get_uuid(GVolume *volume);
GDrive *g_volume_get_drive(GVolume *volume);
GMount *g_volume_get_mount(GVolume *volume);
gboolean g_volume_can_mount(GVolume *volume);
gboolean g_volume_can_eject(GVolume *volume);
gboolean g_volume_should_automount(GVolume *volume);
void g_volume_mount(GVolume *volume, GMountMountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
					gpointer user_data);
gboolean g_volume_mount_finish(GVolume *volume, GAsyncResult *result, GError **error);
#ifndef G_DISABLE_DEPRECATED
void g_volume_eject(GVolume *volume, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_volume_eject_finish(GVolume *volume, GAsyncResult *result, GError **error);
#endif
char *g_volume_get_identifier(GVolume *volume, const char *kind);
char **g_volume_enumerate_identifiers(GVolume *volume);
GFile *g_volume_get_activation_root(GVolume *volume);
void g_volume_eject_with_operation(GVolume *volume, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                                   gpointer user_data);
gboolean g_volume_eject_with_operation_finish(GVolume *volume, GAsyncResult *result, GError **error);
G_END_DECLS

#endif