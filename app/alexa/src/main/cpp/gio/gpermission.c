#include "../glib/glibintl.h"
#include "../gobject/gvaluetypes.h"
#include "../gobject/gparamspecs.h"
#include "config.h"
#include "gpermission.h"

G_DEFINE_ABSTRACT_TYPE(GPermission, g_permission, G_TYPE_OBJECT);
struct _GPermissionPrivate {
  gboolean allowed;
  gboolean can_acquire;
  gboolean can_release;
};
enum  {
  PROP_NONE,
  PROP_ALLOWED,
  PROP_CAN_ACQUIRE,
  PROP_CAN_RELEASE
};
gboolean g_permission_acquire(GPermission *permission, GCancellable *cancellable, GError **error) {
  return G_PERMISSION_GET_CLASS(permission)->acquire(permission, cancellable, error);
}
void g_permission_acquire_async(GPermission *permission, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  G_PERMISSION_GET_CLASS(permission)->acquire_async(permission, cancellable, callback, user_data);
}
gboolean g_permission_acquire_finish(GPermission *permission, GAsyncResult *result, GError **error) {
  return G_PERMISSION_GET_CLASS(permission)->acquire_finish(permission, result, error);
}
gboolean g_permission_release(GPermission *permission, GCancellable *cancellable, GError **error) {
  return G_PERMISSION_GET_CLASS(permission)->release(permission, cancellable, error);
}
void g_permission_release_async(GPermission *permission, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  G_PERMISSION_GET_CLASS(permission)->release_async(permission, cancellable, callback, user_data);
}
gboolean g_permission_release_finish(GPermission *permission, GAsyncResult *result, GError **error) {
  return G_PERMISSION_GET_CLASS(permission)->release_finish(permission, result, error);
}
gboolean g_permission_get_allowed(GPermission *permission) {
  return permission->priv->allowed;
}
gboolean g_permission_get_can_acquire(GPermission *permission) {
  return permission->priv->can_acquire;
}
gboolean g_permission_get_can_release(GPermission *permission) {
  return permission->priv->can_release;
}
void g_permission_impl_update(GPermission *permission, gboolean allowed, gboolean can_acquire, gboolean can_release) {
  GObject *object = G_OBJECT(permission);
  g_object_freeze_notify (object);
  if (allowed != permission->priv->allowed) {
      permission->priv->allowed = !!allowed;
      g_object_notify(object, "allowed");
  }
  if (can_acquire != permission->priv->can_acquire) {
      permission->priv->can_acquire = !!can_acquire;
      g_object_notify(object, "can-acquire");
  }
  if (can_release != permission->priv->can_release) {
      permission->priv->can_release = !!can_release;
      g_object_notify(object, "can-release");
  }
  g_object_thaw_notify(object);
}
static void g_permission_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GPermission *permission = G_PERMISSION(object);
  switch(prop_id) {
      case PROP_ALLOWED: g_value_set_boolean(value, permission->priv->allowed); break;
      case PROP_CAN_ACQUIRE: g_value_set_boolean(value, permission->priv->can_acquire); break;
      case PROP_CAN_RELEASE: g_value_set_boolean(value, permission->priv->can_release); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_permission_init(GPermission *permission) {
  permission->priv = G_TYPE_INSTANCE_GET_PRIVATE (permission, G_TYPE_PERMISSION, GPermissionPrivate);
}
static void g_permission_class_init(GPermissionClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->get_property = g_permission_get_property;
  g_object_class_install_property(object_class, PROP_ALLOWED,g_param_spec_boolean ("allowed", P_("Is allowed"), P_("If the "
                                  "caller is allowed to perform the action"),FALSE, G_PARAM_STATIC_STRINGS | G_PARAM_READABLE));
  g_object_class_install_property(object_class, PROP_CAN_ACQUIRE,g_param_spec_boolean ("can-acquire", P_("Can acquire"),
                                  P_("If calling g_permission_acquire() makes sense"),FALSE,G_PARAM_STATIC_STRINGS | G_PARAM_READABLE));
  g_object_class_install_property(object_class, PROP_CAN_RELEASE,g_param_spec_boolean ("can-release", P_("Can release"),
                                  P_("If calling g_permission_release() makes sense"),FALSE,G_PARAM_STATIC_STRINGS | G_PARAM_READABLE));
  g_type_class_add_private(class, sizeof(GPermissionPrivate));
}