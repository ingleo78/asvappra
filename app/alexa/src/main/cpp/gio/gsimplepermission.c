#include "config.h"
#include "gsimplepermission.h"
#include "gpermission.h"

typedef GPermissionClass GSimplePermissionClass;
struct _GSimplePermission {
  GPermission parent_instance;
};
G_DEFINE_TYPE (GSimplePermission, g_simple_permission, G_TYPE_PERMISSION);
static void g_simple_permission_init(GSimplePermission *simple) {}
static void g_simple_permission_class_init(GSimplePermissionClass *class) {}
GPermission *g_simple_permission_new(gboolean allowed) {
  GPermission *permission = g_object_new(G_TYPE_SIMPLE_PERMISSION, NULL);
  g_permission_impl_update(permission, allowed, FALSE, FALSE);
  return permission;
}