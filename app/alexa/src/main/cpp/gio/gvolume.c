#include "../glib/glibintl.h"
#include "config.h"
#include "gmount.h"
#include "gvolume.h"
#include "gasyncresult.h"
#include "gsimpleasyncresult.h"
#include "gioerror.h"

typedef GVolumeIface GVolumeInterface;
G_DEFINE_INTERFACE(GVolume, g_volume, G_TYPE_OBJECT);
static void g_volume_default_init(GVolumeInterface *iface) {
  g_signal_new(I_("changed"), G_TYPE_VOLUME, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GVolumeIface, changed), NULL, NULL, g_cclosure_marshal_VOID__VOID,
		       G_TYPE_NONE, 0);
  g_signal_new(I_("removed"), G_TYPE_VOLUME, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GVolumeIface, removed), NULL, NULL, g_cclosure_marshal_VOID__VOID,
		       G_TYPE_NONE, 0);
}
char *g_volume_get_name(GVolume *volume) {
  GVolumeIface *iface;
  g_return_val_if_fail(G_IS_VOLUME(volume), NULL);
  iface = G_VOLUME_GET_IFACE(volume);
  return (*iface->get_name)(volume);
}
GIcon *g_volume_get_icon(GVolume *volume) {
  GVolumeIface *iface;
  g_return_val_if_fail(G_IS_VOLUME(volume), NULL);
  iface = G_VOLUME_GET_IFACE(volume);
  return (*iface->get_icon)(volume);
}
char *g_volume_get_uuid(GVolume *volume) {
  GVolumeIface *iface;
  g_return_val_if_fail(G_IS_VOLUME(volume), NULL);
  iface = G_VOLUME_GET_IFACE(volume);
  return (*iface->get_uuid)(volume);
}
GDrive *g_volume_get_drive(GVolume *volume) {
  GVolumeIface *iface;
  g_return_val_if_fail(G_IS_VOLUME(volume), NULL);
  iface = G_VOLUME_GET_IFACE(volume);
  return (*iface->get_drive)(volume);
}
GMount *g_volume_get_mount(GVolume *volume) {
  GVolumeIface *iface;
  g_return_val_if_fail(G_IS_VOLUME(volume), NULL);
  iface = G_VOLUME_GET_IFACE(volume);
  return (*iface->get_mount)(volume);
}
gboolean g_volume_can_mount(GVolume *volume) {
  GVolumeIface *iface;
  g_return_val_if_fail(G_IS_VOLUME(volume), FALSE);
  iface = G_VOLUME_GET_IFACE(volume);
  if (iface->can_mount == NULL) return FALSE;
  return (*iface->can_mount)(volume);
}
gboolean g_volume_can_eject(GVolume *volume) {
  GVolumeIface *iface;
  g_return_val_if_fail(G_IS_VOLUME(volume), FALSE);
  iface = G_VOLUME_GET_IFACE(volume);
  if (iface->can_eject == NULL) return FALSE;
  return (*iface->can_eject)(volume);
}
gboolean g_volume_should_automount(GVolume *volume) {
  GVolumeIface *iface;
  g_return_val_if_fail(G_IS_VOLUME(volume), FALSE);
  iface = G_VOLUME_GET_IFACE(volume);
  if (iface->should_automount == NULL) return FALSE;
  return (*iface->should_automount)(volume);
}
void g_volume_mount(GVolume *volume, GMountMountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                    gpointer user_data) {
  GVolumeIface *iface;
  g_return_if_fail(G_IS_VOLUME(volume));
  iface = G_VOLUME_GET_IFACE(volume);
  if (iface->mount_fn == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(volume), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("volume doesn't implement mount"));
      return;
  }
  (*iface->mount_fn)(volume, flags, mount_operation, cancellable, callback, user_data);
}
gboolean g_volume_mount_finish(GVolume *volume, GAsyncResult *result, GError **error) {
  GVolumeIface *iface;
  g_return_val_if_fail(G_IS_VOLUME(volume), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_VOLUME_GET_IFACE(volume);
  return (*iface->mount_finish)(volume, result, error);
}
void g_volume_eject(GVolume *volume, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback,gpointer user_data){
  GVolumeIface *iface;
  g_return_if_fail(G_IS_VOLUME(volume));
  iface = G_VOLUME_GET_IFACE(volume);
  if (iface->eject == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(volume), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("volume doesn't "
                                          "implement eject"));
      return;
  }
  (*iface->eject)(volume, flags, cancellable, callback, user_data);
}
gboolean g_volume_eject_finish(GVolume *volume, GAsyncResult *result, GError **error) {
  GVolumeIface *iface;
  g_return_val_if_fail(G_IS_VOLUME(volume), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_VOLUME_GET_IFACE(volume);
  return (*iface->eject_finish)(volume, result, error);
}
void g_volume_eject_with_operation(GVolume *volume, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable,
                                   GAsyncReadyCallback callback, gpointer user_data) {
  GVolumeIface *iface;
  g_return_if_fail (G_IS_VOLUME (volume));
  iface = G_VOLUME_GET_IFACE (volume);
  if (iface->eject == NULL && iface->eject_with_operation == NULL) {
      g_simple_async_report_error_in_idle (G_OBJECT (volume), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("volume doesn't "
                                          "implement eject or eject_with_operation"));
      return;
  }
  if (iface->eject_with_operation != NULL) (*iface->eject_with_operation)(volume, flags, mount_operation, cancellable, callback, user_data);
  else (*iface->eject)(volume, flags, cancellable, callback, user_data);
}
gboolean g_volume_eject_with_operation_finish(GVolume *volume, GAsyncResult *result, GError **error) {
  GVolumeIface *iface;
  g_return_val_if_fail(G_IS_VOLUME(volume), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error))
        return FALSE;
  }
  iface = G_VOLUME_GET_IFACE(volume);
  if (iface->eject_with_operation_finish != NULL) return (*iface->eject_with_operation_finish)(volume, result, error);
  else return (*iface->eject_finish)(volume, result, error);
}
char *g_volume_get_identifier(GVolume *volume, const char *kind) {
  GVolumeIface *iface;
  g_return_val_if_fail(G_IS_VOLUME(volume), NULL);
  g_return_val_if_fail(kind != NULL, NULL);
  iface = G_VOLUME_GET_IFACE(volume);
  if (iface->get_identifier == NULL) return NULL;
  return (*iface->get_identifier)(volume, kind);
}
char **g_volume_enumerate_identifiers(GVolume *volume) {
  GVolumeIface *iface;
  g_return_val_if_fail(G_IS_VOLUME(volume), NULL);
  iface = G_VOLUME_GET_IFACE(volume);
  if (iface->enumerate_identifiers == NULL) return NULL;
  return (*iface->enumerate_identifiers)(volume);
}
GFile *g_volume_get_activation_root(GVolume *volume) {
  GVolumeIface *iface;
  g_return_val_if_fail(G_IS_VOLUME(volume), NULL);
  iface = G_VOLUME_GET_IFACE(volume);
  if (iface->get_activation_root == NULL) return NULL;
  return (*iface->get_activation_root)(volume);
}