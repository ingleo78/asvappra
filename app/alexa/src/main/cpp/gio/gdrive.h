#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_DRIVE_H__
#define __G_DRIVE_H__

#include "../gobject/gtype.h"
#include "giotypes.h"
#include "gioenums.h"

G_BEGIN_DECLS
#define G_TYPE_DRIVE  (g_drive_get_type())
#define G_DRIVE(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_DRIVE, GDrive))
#define G_IS_DRIVE(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_DRIVE))
#define G_DRIVE_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_DRIVE, GDriveIface))
typedef struct _GDriveIface GDriveIface;
struct _GDriveIface {
  GTypeInterface g_iface;
  void (*changed)(GDrive *drive);
  void (*disconnected)(GDrive *drive);
  void (*eject_button)(GDrive *drive);
  char *(*get_name)(GDrive *drive);
  GIcon *(*get_icon)(GDrive *drive);
  gboolean (*has_volumes)(GDrive *drive);
  GList *(*get_volumes)(GDrive *drive);
  gboolean (*is_media_removable)(GDrive *drive);
  gboolean (*has_media)(GDrive *drive);
  gboolean (*is_media_check_automatic)(GDrive *drive);
  gboolean (*can_eject)(GDrive *drive);
  gboolean (*can_poll_for_media)(GDrive *drive);
  void (*eject)(GDrive *drive, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*eject_finish)(GDrive *drive, GAsyncResult *result, GError **error);
  void (*poll_for_media)(GDrive *drive, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*poll_for_media_finish)(GDrive *drive, GAsyncResult *result, GError **error);
  char *(*get_identifier)(GDrive *drive, const char *kind);
  char **(*enumerate_identifiers)(GDrive *drive);
  GDriveStartStopType (*get_start_stop_type)(GDrive *drive);
  gboolean (*can_start)(GDrive *drive);
  gboolean (*can_start_degraded)(GDrive *drive);
  void (*start)(GDrive *drive, GDriveStartFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*start_finish)(GDrive *drive, GAsyncResult *result, GError **error);
  gboolean (*can_stop)(GDrive *drive);
  void (*stop)(GDrive *drive, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*stop_finish)(GDrive *drive, GAsyncResult *result, GError **error);
  void (*stop_button)(GDrive *drive);
  void (*eject_with_operation)(GDrive *drive, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                               gpointer user_data);
  gboolean (*eject_with_operation_finish)(GDrive *drive, GAsyncResult *result, GError **error);
};
GType g_drive_get_type(void) G_GNUC_CONST;
char *g_drive_get_name(GDrive *drive);
GIcon *g_drive_get_icon(GDrive *drive);
gboolean g_drive_has_volumes(GDrive *drive);
GList *g_drive_get_volumes(GDrive *drive);
gboolean g_drive_is_media_removable(GDrive *drive);
gboolean g_drive_has_media(GDrive *drive);
gboolean g_drive_is_media_check_automatic(GDrive *drive);
gboolean g_drive_can_poll_for_media(GDrive *drive);
gboolean g_drive_can_eject(GDrive *drive);
#ifndef G_DISABLE_DEPRECATED
void g_drive_eject(GDrive *drive, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_drive_eject_finish(GDrive *drive, GAsyncResult *result, GError **error);
#endif
void g_drive_poll_for_media(GDrive *drive, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_drive_poll_for_media_finish(GDrive *drive, GAsyncResult *result, GError **error);
char *g_drive_get_identifier(GDrive *drive, const char *kind);
char **g_drive_enumerate_identifiers(GDrive *drive);
GDriveStartStopType g_drive_get_start_stop_type(GDrive *drive);
gboolean g_drive_can_start(GDrive *drive);
gboolean g_drive_can_start_degraded(GDrive *drive);
void g_drive_start(GDrive *drive, GDriveStartFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                   gpointer user_data);
gboolean g_drive_start_finish(GDrive *drive, GAsyncResult *result, GError **error);
gboolean g_drive_can_stop(GDrive *drive);
void g_drive_stop(GDrive *drive, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                  gpointer user_data);
gboolean g_drive_stop_finish(GDrive *drive, GAsyncResult *result, GError **error);
void g_drive_eject_with_operation(GDrive *drive, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable,
                                  GAsyncReadyCallback callback, gpointer user_data);
gboolean g_drive_eject_with_operation_finish(GDrive *drive, GAsyncResult *result, GError **error);
G_END_DECLS

#endif