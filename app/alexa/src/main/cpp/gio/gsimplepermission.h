#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_SIMPLE_PERMISSION_H__
#define __G_SIMPLE_PERMISSION_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_SIMPLE_PERMISSION  (g_simple_permission_get_type())
#define G_SIMPLE_PERMISSION(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_SIMPLE_PERMISSION, GSimplePermission))
#define G_IS_SIMPLE_PERMISSION(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_SIMPLE_PERMISSION))
GType g_simple_permission_get_type(void);
GPermission *g_simple_permission_new(gboolean allowed);
G_END_DECLS

#endif