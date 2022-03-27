#include <string.h>
#include "../glib/glibintl.h"
#include "config.h"
#include "gmount.h"
#include "gmountprivate.h"
#include "gasyncresult.h"
#include "gsimpleasyncresult.h"
#include "gioerror.h"

typedef GMountIface GMountInterface;
G_DEFINE_INTERFACE(GMount, g_mount, G_TYPE_OBJECT);
static void g_mount_default_init(GMountInterface *iface) {
  g_signal_new(I_("changed"), G_TYPE_MOUNT, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GMountIface, changed), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
  g_signal_new(I_("unmounted"), G_TYPE_MOUNT, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GMountIface, unmounted), NULL, NULL, g_cclosure_marshal_VOID__VOID,
               G_TYPE_NONE, 0);
  g_signal_new(I_("pre-unmount"), G_TYPE_MOUNT, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GMountIface, pre_unmount), NULL, NULL, g_cclosure_marshal_VOID__VOID,
               G_TYPE_NONE, 0);
}
GFile *g_mount_get_root(GMount *mount) {
  GMountIface *iface;
  g_return_val_if_fail(G_IS_MOUNT(mount), NULL);
  iface = G_MOUNT_GET_IFACE(mount);
  return (*iface->get_root)(mount);
}
GFile *g_mount_get_default_location(GMount *mount) {
  GMountIface *iface;
  GFile *file;
  g_return_val_if_fail(G_IS_MOUNT(mount), NULL);
  iface = G_MOUNT_GET_IFACE(mount);
  if (iface->get_default_location) file = (*iface->get_default_location)(mount);
  else file = (*iface->get_root)(mount);
  return file;
}
char *g_mount_get_name(GMount *mount) {
  GMountIface *iface;
  g_return_val_if_fail(G_IS_MOUNT(mount), NULL);
  iface = G_MOUNT_GET_IFACE(mount);
  return (*iface->get_name)(mount);
}
GIcon *g_mount_get_icon(GMount *mount) {
  GMountIface *iface;
  g_return_val_if_fail(G_IS_MOUNT(mount), NULL);
  iface = G_MOUNT_GET_IFACE(mount);
  return (*iface->get_icon)(mount);
}
char *g_mount_get_uuid(GMount *mount) {
  GMountIface *iface;
  g_return_val_if_fail(G_IS_MOUNT(mount), NULL);
  iface = G_MOUNT_GET_IFACE(mount);
  return (*iface->get_uuid)(mount);
}
GVolume *g_mount_get_volume(GMount *mount) {
  GMountIface *iface;
  g_return_val_if_fail(G_IS_MOUNT(mount), NULL);
  iface = G_MOUNT_GET_IFACE(mount);
  return (*iface->get_volume)(mount);
}
GDrive *g_mount_get_drive(GMount *mount) {
  GMountIface *iface;
  g_return_val_if_fail(G_IS_MOUNT(mount), NULL);
  iface = G_MOUNT_GET_IFACE(mount);
  return (*iface->get_drive)(mount);
}
gboolean g_mount_can_unmount(GMount *mount) {
  GMountIface *iface;
  g_return_val_if_fail(G_IS_MOUNT(mount), FALSE);
  iface = G_MOUNT_GET_IFACE(mount);
  return (*iface->can_unmount)(mount);
}
gboolean g_mount_can_eject(GMount *mount) {
  GMountIface *iface;
  g_return_val_if_fail(G_IS_MOUNT(mount), FALSE);
  iface = G_MOUNT_GET_IFACE(mount);
  return (*iface->can_eject)(mount);
}
void g_mount_unmount(GMount *mount, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GMountIface *iface;
  g_return_if_fail(G_IS_MOUNT(mount));
  iface = G_MOUNT_GET_IFACE(mount);
  if (iface->unmount == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(mount), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("mount doesn't implement "
                                          "\"unmount\""));
      return;
  }
  (*iface->unmount)(mount, flags, cancellable, callback, user_data);
}
gboolean g_mount_unmount_finish(GMount *mount, GAsyncResult *result, GError **error) {
  GMountIface *iface;
  g_return_val_if_fail(G_IS_MOUNT(mount), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_MOUNT_GET_IFACE(mount);
  return (*iface->unmount_finish)(mount, result, error);
}
void g_mount_eject(GMount *mount, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GMountIface *iface;
  g_return_if_fail(G_IS_MOUNT(mount));
  iface = G_MOUNT_GET_IFACE(mount);
  if (iface->eject == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(mount), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("mount doesn't implement"
                                          " \"eject\""));
      return;
  }
  (* iface->eject)(mount, flags, cancellable, callback, user_data);
}
gboolean g_mount_eject_finish(GMount *mount, GAsyncResult *result, GError **error) {
  GMountIface *iface;
  g_return_val_if_fail(G_IS_MOUNT(mount), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_MOUNT_GET_IFACE(mount);
  return (*iface->eject_finish)(mount, result, error);
}
void g_mount_unmount_with_operation(GMount *mount, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                                    gpointer user_data) {
  GMountIface *iface;
  g_return_if_fail(G_IS_MOUNT(mount));
  iface = G_MOUNT_GET_IFACE(mount);
  if (iface->unmount == NULL && iface->unmount_with_operation == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(mount), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("mount doesn't "
                                          "implement \"unmount\" or \"unmount_with_operation\""));
      return;
  }
  if (iface->unmount_with_operation != NULL) (*iface->unmount_with_operation)(mount, flags, mount_operation, cancellable, callback, user_data);
  else (*iface->unmount)(mount, flags, cancellable, callback, user_data);
}
gboolean g_mount_unmount_with_operation_finish(GMount *mount, GAsyncResult *result, GError **error) {
  GMountIface *iface;
  g_return_val_if_fail(G_IS_MOUNT(mount), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_MOUNT_GET_IFACE(mount);
  if (iface->unmount_with_operation_finish != NULL) return (*iface->unmount_with_operation_finish)(mount, result, error);
  else return (*iface->unmount_finish)(mount, result, error);
}
void g_mount_eject_with_operation(GMount *mount, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                                  gpointer user_data) {
  GMountIface *iface;
  g_return_if_fail(G_IS_MOUNT(mount));
  iface = G_MOUNT_GET_IFACE(mount);
  if (iface->eject == NULL && iface->eject_with_operation == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(mount), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("mount doesn't implement"
                                          " \"eject\" or \"eject_with_operation\""));
      return;
  }
  if (iface->eject_with_operation != NULL) (*iface->eject_with_operation)(mount, flags, mount_operation, cancellable, callback, user_data);
  else (*iface->eject)(mount, flags, cancellable, callback, user_data);
}
gboolean g_mount_eject_with_operation_finish(GMount *mount, GAsyncResult *result, GError **error) {
  GMountIface *iface;
  g_return_val_if_fail(G_IS_MOUNT(mount), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_MOUNT_GET_IFACE(mount);
  if (iface->eject_with_operation_finish != NULL) return (*iface->eject_with_operation_finish)(mount, result, error);
  else return (*iface->eject_finish)(mount, result, error);
}
void g_mount_remount(GMount *mount, GMountMountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GMountIface *iface;
  g_return_if_fail(G_IS_MOUNT(mount));
  iface = G_MOUNT_GET_IFACE(mount);
  if (iface->remount == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(mount), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("mount doesn't implement"
                                          " \"remount\""));
      return;
  }
  (*iface->remount)(mount, flags, mount_operation, cancellable, callback, user_data);
}
gboolean g_mount_remount_finish(GMount *mount, GAsyncResult *result, GError **error) {
  GMountIface *iface;
  g_return_val_if_fail(G_IS_MOUNT(mount), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_MOUNT_GET_IFACE(mount);
  return (*iface->remount_finish)(mount, result, error);
}
void g_mount_guess_content_type(GMount *mount, gboolean force_rescan, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GMountIface *iface;
  g_return_if_fail(G_IS_MOUNT(mount));
  iface = G_MOUNT_GET_IFACE(mount);
  if (iface->guess_content_type == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(mount), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("mount doesn't implement"
                                          " content type guessing"));
      return;
  }
  (*iface->guess_content_type)(mount, force_rescan, cancellable, callback, user_data);
}
gchar **g_mount_guess_content_type_finish(GMount *mount, GAsyncResult *result, GError **error) {
  GMountIface *iface;
  g_return_val_if_fail(G_IS_MOUNT(mount), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_MOUNT_GET_IFACE (mount);
  return (*iface->guess_content_type_finish)(mount, result, error);
}
char **g_mount_guess_content_type_sync(GMount *mount, gboolean force_rescan, GCancellable *cancellable, GError **error) {
  GMountIface *iface;
  g_return_val_if_fail (G_IS_MOUNT (mount), NULL);
  iface = G_MOUNT_GET_IFACE (mount);
  if (iface->guess_content_type_sync == NULL) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("mount doesn't implement synchronous content type guessing"));
      return NULL;
  }
  return (*iface->guess_content_type_sync)(mount, force_rescan, cancellable, error);
}
G_LOCK_DEFINE_STATIC(priv_lock);
typedef struct {
  gint shadow_ref_count;
} GMountPrivate;
static void free_private(GMountPrivate *private) {
  G_LOCK(priv_lock);
  g_free(private);
  G_UNLOCK(priv_lock);
}
static GMountPrivate *get_private(GMount *mount) {
  GMountPrivate *private;
  private = g_object_get_data(G_OBJECT(mount), "g-mount-private");
  if (G_LIKELY(private != NULL)) goto out;
  private = g_new0(GMountPrivate, 1);
  g_object_set_data_full(G_OBJECT(mount),"g-mount-private", private, (GDestroyNotify)free_private);
out:
  return private;
}
gboolean g_mount_is_shadowed(GMount *mount) {
  GMountPrivate *priv;
  gboolean ret;
  g_return_val_if_fail(G_IS_MOUNT(mount), FALSE);
  G_LOCK(priv_lock);
  priv = get_private(mount);
  ret = (priv->shadow_ref_count > 0);
  G_UNLOCK(priv_lock);
  return ret;
}
void g_mount_shadow(GMount *mount) {
  GMountPrivate *priv;
  g_return_if_fail(G_IS_MOUNT(mount));
  G_LOCK(priv_lock);
  priv = get_private(mount);
  priv->shadow_ref_count += 1;
  G_UNLOCK(priv_lock);
}
void g_mount_unshadow(GMount *mount) {
  GMountPrivate *priv;
  g_return_if_fail(G_IS_MOUNT(mount));
  G_LOCK(priv_lock);
  priv = get_private(mount);
  priv->shadow_ref_count -= 1;
  if (priv->shadow_ref_count < 0) g_warning("Shadow ref count on GMount is negative");
  G_UNLOCK(priv_lock);
}