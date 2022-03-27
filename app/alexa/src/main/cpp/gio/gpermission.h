#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_PERMISSION_H__
#define __G_PERMISSION_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_PERMISSION  (g_permission_get_type())
#define G_PERMISSION(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_PERMISSION, GPermission))
#define G_PERMISSION_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_PERMISSION, GPermissionClass))
#define G_IS_PERMISSION(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_PERMISSION))
#define G_IS_PERMISSION_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_PERMISSION))
#define G_PERMISSION_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_PERMISSION, GPermissionClass))
typedef struct _GPermissionPrivate GPermissionPrivate;
typedef struct _GPermissionClass GPermissionClass;
struct _GPermission {
  GObject parent_instance;
  GPermissionPrivate *priv;
};
struct _GPermissionClass {
  GObjectClass parent_class;
  gboolean (*acquire)(GPermission *permission, GCancellable *cancellable, GError **error);
  void (*acquire_async)(GPermission *permission, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*acquire_finish)(GPermission *permission, GAsyncResult *result, GError **error);
  gboolean (*release)(GPermission *permission, GCancellable *cancellable, GError **error);
  void (*release_async)(GPermission *permission, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*release_finish)(GPermission *permission, GAsyncResult *result, GError **error);
  gpointer reserved[16];
};
GType g_permission_get_type(void);
gboolean g_permission_acquire(GPermission *permission, GCancellable *cancellable, GError **error);
void g_permission_acquire_async(GPermission *permission, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_permission_acquire_finish(GPermission *permission, GAsyncResult *result, GError **error);
gboolean g_permission_release(GPermission *permission, GCancellable *cancellable, GError **error);
void g_permission_release_async(GPermission *permission, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_permission_release_finish(GPermission *permission, GAsyncResult *result, GError **error);
gboolean g_permission_get_allowed(GPermission *permission);
gboolean g_permission_get_can_acquire(GPermission *permission);
gboolean g_permission_get_can_release(GPermission *permission);
void g_permission_impl_update(GPermission *permission, gboolean allowed, gboolean can_acquire, gboolean can_release);
G_END_DECLS

#endif