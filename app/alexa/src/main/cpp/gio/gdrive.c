#include "../glib/glibintl.h"
#include "config.h"
#include "gdrive.h"
#include "gsimpleasyncresult.h"
#include "gasyncresult.h"
#include "gioerror.h"

typedef GDriveIface GDriveInterface;
G_DEFINE_INTERFACE(GDrive, g_drive, G_TYPE_OBJECT);
static void g_drive_default_init (GDriveInterface *iface) {
  g_signal_new(I_("changed"), G_TYPE_DRIVE, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GDriveIface, changed), NULL, NULL, g_cclosure_marshal_VOID__VOID,
		       G_TYPE_NONE, 0);
  g_signal_new(I_("disconnected"), G_TYPE_DRIVE, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GDriveIface, disconnected), NULL, NULL, g_cclosure_marshal_VOID__VOID,
		       G_TYPE_NONE, 0);
  g_signal_new(I_("eject-button"), G_TYPE_DRIVE, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GDriveIface, eject_button), NULL, NULL, g_cclosure_marshal_VOID__VOID,
		       G_TYPE_NONE, 0);
  g_signal_new(I_("stop-button"), G_TYPE_DRIVE, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GDriveIface, stop_button), NULL, NULL,g_cclosure_marshal_VOID__VOID,
		       G_TYPE_NONE, 0);
}
char *g_drive_get_name(GDrive *drive) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), NULL);
  iface = G_DRIVE_GET_IFACE(drive);
  return (*iface->get_name)(drive);
}
GIcon *g_drive_get_icon(GDrive *drive) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), NULL);
  iface = G_DRIVE_GET_IFACE(drive);
  return (*iface->get_icon)(drive);
}
gboolean g_drive_has_volumes(GDrive *drive) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), FALSE);
  iface = G_DRIVE_GET_IFACE(drive);
  return (*iface->has_volumes)(drive);
}
GList *g_drive_get_volumes(GDrive *drive) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), NULL);
  iface = G_DRIVE_GET_IFACE(drive);
  return (*iface->get_volumes)(drive);
}
gboolean g_drive_is_media_check_automatic(GDrive *drive) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), FALSE);
  iface = G_DRIVE_GET_IFACE(drive);
  return (*iface->is_media_check_automatic)(drive);
}
gboolean g_drive_is_media_removable(GDrive *drive) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), FALSE);
  iface = G_DRIVE_GET_IFACE(drive);
  return (*iface->is_media_removable)(drive);
}
gboolean g_drive_has_media(GDrive *drive) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), FALSE);
  iface = G_DRIVE_GET_IFACE(drive);
  return (*iface->has_media)(drive);
}
gboolean g_drive_can_eject(GDrive *drive) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), FALSE);
  iface = G_DRIVE_GET_IFACE(drive);
  if (iface->can_eject == NULL) return FALSE;
  return (*iface->can_eject)(drive);
}
gboolean g_drive_can_poll_for_media(GDrive *drive) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), FALSE);
  iface = G_DRIVE_GET_IFACE(drive);
  if (iface->poll_for_media == NULL) return FALSE;
  return (*iface->can_poll_for_media)(drive);
}
void g_drive_eject(GDrive *drive, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GDriveIface *iface;
  g_return_if_fail(G_IS_DRIVE(drive));
  iface = G_DRIVE_GET_IFACE(drive);
  if (iface->eject == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(drive), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"drive doesn't "
                                          "implement eject");
      return;
  }
  (*iface->eject)(drive, flags, cancellable, callback, user_data);
}
gboolean g_drive_eject_finish(GDrive *drive, GAsyncResult *result, GError **error) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_DRIVE_GET_IFACE(drive);
  return (*iface->eject_finish)(drive, result, error);
}
void g_drive_eject_with_operation(GDrive *drive, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable,
                                  GAsyncReadyCallback callback, gpointer user_data) {
  GDriveIface *iface;
  g_return_if_fail(G_IS_DRIVE(drive));
  iface = G_DRIVE_GET_IFACE(drive);
  if (iface->eject == NULL && iface->eject_with_operation == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(drive), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"drive doesn't "
                                           "implement eject or eject_with_operation");
      return;
  }
  if (iface->eject_with_operation != NULL) (*iface->eject_with_operation)(drive, flags, mount_operation, cancellable, callback, user_data);
  else (*iface->eject)(drive, flags, cancellable, callback, user_data);
}
gboolean g_drive_eject_with_operation_finish(GDrive *drive, GAsyncResult *result, GError **error) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE (drive), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error (simple, error)) return FALSE;
  }
  iface = G_DRIVE_GET_IFACE(drive);
  if (iface->eject_with_operation_finish != NULL) return (*iface->eject_with_operation_finish)(drive, result, error);
  else return (*iface->eject_finish)(drive, result, error);
}
void g_drive_poll_for_media(GDrive *drive, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GDriveIface *iface;
  g_return_if_fail(G_IS_DRIVE(drive));
  iface = G_DRIVE_GET_IFACE(drive);
  if (iface->poll_for_media == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(drive), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"drive doesn't "
                                          "implement polling for media");
      return;
  }
  (* iface->poll_for_media)(drive, cancellable, callback, user_data);
}
gboolean g_drive_poll_for_media_finish(GDrive *drive, GAsyncResult *result, GError **error) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_DRIVE_GET_IFACE(drive);
  return (*iface->poll_for_media_finish)(drive, result, error);
}
char *g_drive_get_identifier(GDrive *drive, const char *kind) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), NULL);
  g_return_val_if_fail(kind != NULL, NULL);
  iface = G_DRIVE_GET_IFACE(drive);
  if (iface->get_identifier == NULL) return NULL;
  return (*iface->get_identifier)(drive, kind);
}
char **g_drive_enumerate_identifiers(GDrive *drive) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), NULL);
  iface = G_DRIVE_GET_IFACE(drive);
  if (iface->enumerate_identifiers == NULL) return NULL;
  return (*iface->enumerate_identifiers)(drive);
}
GDriveStartStopType g_drive_get_start_stop_type(GDrive *drive) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), FALSE);
  iface = G_DRIVE_GET_IFACE(drive);
  if (iface->get_start_stop_type == NULL) return G_DRIVE_START_STOP_TYPE_UNKNOWN;
  return (*iface->get_start_stop_type)(drive);
}
gboolean g_drive_can_start(GDrive *drive) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), FALSE);
  iface = G_DRIVE_GET_IFACE(drive);
  if (iface->can_start == NULL) return FALSE;
  return (*iface->can_start)(drive);
}
gboolean g_drive_can_start_degraded(GDrive *drive) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), FALSE);
  iface = G_DRIVE_GET_IFACE(drive);
  if (iface->can_start_degraded == NULL) return FALSE;
  return (*iface->can_start_degraded)(drive);
}
void g_drive_start(GDrive *drive, GDriveStartFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                   gpointer user_data) {
  GDriveIface *iface;
  g_return_if_fail(G_IS_DRIVE(drive));
  iface = G_DRIVE_GET_IFACE (drive);
  if (iface->start == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(drive), callback, user_data, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED,"drive doesn't implement start");
      return;
  }
  (*iface->start)(drive, flags, mount_operation, cancellable, callback, user_data);
}
gboolean g_drive_start_finish(GDrive *drive, GAsyncResult *result, GError **error) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_DRIVE_GET_IFACE(drive);
  return (*iface->start_finish)(drive, result, error);
}
gboolean g_drive_can_stop(GDrive *drive) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), FALSE);
  iface = G_DRIVE_GET_IFACE(drive);
  if (iface->can_stop == NULL) return FALSE;
  return (*iface->can_stop)(drive);
}
void g_drive_stop(GDrive *drive, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                  gpointer user_data) {
  GDriveIface *iface;
  g_return_if_fail(G_IS_DRIVE(drive));
  iface = G_DRIVE_GET_IFACE(drive);
  if (iface->stop == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(drive), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"drive doesn't "
                                          "implement stop");
      return;
  }
  (*iface->stop)(drive, flags, mount_operation, cancellable, callback, user_data);
}
gbooleang_drive_stop_finish(GDrive *drive, GAsyncResult *result, GError **error) {
  GDriveIface *iface;
  g_return_val_if_fail(G_IS_DRIVE(drive), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_DRIVE_GET_IFACE(drive);
  return (*iface->stop_finish)(drive, result, error);
}