#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_LOADABLE_ICON_H__
#define __G_LOADABLE_ICON_H__

#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_LOADABLE_ICON  (g_loadable_icon_get_type())
#define G_LOADABLE_ICON(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_LOADABLE_ICON, GLoadableIcon))
#define G_IS_LOADABLE_ICON(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_LOADABLE_ICON))
#define G_LOADABLE_ICON_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_LOADABLE_ICON, GLoadableIconIface))
typedef struct _GLoadableIconIface GLoadableIconIface;
struct _GLoadableIconIface {
  GTypeInterface g_iface;
  GInputStream *(*load)(GLoadableIcon *icon, int size, char **type, GCancellable *cancellable, GError **error);
  void (*load_async)(GLoadableIcon *icon, int size, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  GInputStream *(*load_finish)(GLoadableIcon *icon, GAsyncResult *res, char **type, GError **error);
};
GType g_loadable_icon_get_type(void) G_GNUC_CONST;
GInputStream *g_loadable_icon_load(GLoadableIcon *icon, int size, char **type, GCancellable *cancellable, GError **error);
void g_loadable_icon_load_async(GLoadableIcon *icon, int size, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GInputStream *g_loadable_icon_load_finish(GLoadableIcon *icon, GAsyncResult *res, char **type, GError **error);
G_END_DECLS

#endif