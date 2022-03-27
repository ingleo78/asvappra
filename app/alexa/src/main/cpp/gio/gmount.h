#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_MOUNT_H__
#define __G_MOUNT_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_MOUNT  (g_mount_get_type())
#define G_MOUNT(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_MOUNT, GMount))
#define G_IS_MOUNT(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_MOUNT))
#define G_MOUNT_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_MOUNT, GMountIface))
typedef struct _GMountIface GMountIface;
struct _GMountIface {
  GTypeInterface g_iface;
  void (*changed)(GMount *mount);
  void (*unmounted)(GMount *mount);
  GFile *(*get_root)(GMount *mount);
  char *(*get_name)(GMount *mount);
  GIcon *(*get_icon)(GMount *mount);
  char *(*get_uuid)(GMount *mount);
  GVolume *(*get_volume)(GMount *mount);
  GDrive *(*get_drive)(GMount *mount);
  gboolean (*can_unmount)(GMount *mount);
  gboolean (*can_eject)(GMount *mount);
  void (*unmount)(GMount *mount, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*unmount_finish)(GMount *mount, GAsyncResult *result, GError **error);
  void (*eject)(GMount *mount, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*eject_finish)(GMount *mount, GAsyncResult *result, GError **error);
  void (*remount)(GMount *mount, GMountMountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*remount_finish)(GMount *mount, GAsyncResult *result, GError **error);
  void (*guess_content_type)(GMount *mount, gboolean force_rescan, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gchar **(*guess_content_type_finish)(GMount *mount, GAsyncResult *result, GError **error);
  gchar **(*guess_content_type_sync)(GMount *mount, gboolean force_rescan, GCancellable *cancellable, GError **error);
  void (*pre_unmount)(GMount *mount);
  void (*unmount_with_operation)(GMount *mount, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*unmount_with_operation_finish)(GMount *mount, GAsyncResult *result, GError **error);
  void (*eject_with_operation)(GMount *mount, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                               gpointer user_data);
  gboolean (*eject_with_operation_finish)(GMount *mount, GAsyncResult *result, GError **error);
  GFile *(*get_default_location)(GMount *mount);
};
GType g_mount_get_type(void) G_GNUC_CONST;
GFile *g_mount_get_root(GMount *mount);
GFile *g_mount_get_default_location(GMount *mount);
char *g_mount_get_name(GMount *mount);
GIcon *g_mount_get_icon(GMount *mount);
char *g_mount_get_uuid(GMount *mount);
GVolume *g_mount_get_volume(GMount *mount);
GDrive *g_mount_get_drive(GMount *mount);
gboolean g_mount_can_unmount(GMount *mount);
gboolean g_mount_can_eject(GMount *mount);
#ifndef G_DISABLE_DEPRECATED
void g_mount_unmount(GMount *mount, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_mount_unmount_finish(GMount *mount, GAsyncResult *result, GError **error);
void g_mount_eject(GMount *mount, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_mount_eject_finish(GMount *mount, GAsyncResult *result, GError **error);
#endif
void g_mount_remount(GMount *mount, GMountMountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_mount_remount_finish(GMount *mount, GAsyncResult *result, GError **error);
void g_mount_guess_content_type(GMount *mount, gboolean force_rescan, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gchar **g_mount_guess_content_type_finish(GMount *mount, GAsyncResult *result, GError **error);
gchar **g_mount_guess_content_type_sync(GMount *mount, gboolean force_rescan, GCancellable *cancellable, GError **error);
gboolean g_mount_is_shadowed(GMount *mount);
void g_mount_shadow(GMount *mount);
void g_mount_unshadow(GMount *mount);
void g_mount_unmount_with_operation(GMount *mount, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                                    gpointer user_data);
gboolean g_mount_unmount_with_operation_finish(GMount *mount, GAsyncResult *result, GError **error);
void g_mount_eject_with_operation(GMount *mount, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                                  gpointer user_data);
gboolean g_mount_eject_with_operation_finish(GMount *mount, GAsyncResult *result, GError **error);
G_END_DECLS

#endif