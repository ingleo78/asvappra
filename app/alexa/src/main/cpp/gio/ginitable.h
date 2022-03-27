#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_INITABLE_H__
#define __G_INITABLE_H__

#include "../gobject/gobject.h"
#include "../gobject/gtype.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_INITABLE  (g_initable_get_type())
#define G_INITABLE(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_INITABLE, GInitable))
#define G_IS_INITABLE(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_INITABLE))
#define G_INITABLE_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_INITABLE, GInitableIface))
#define G_TYPE_IS_INITABLE(type)  (g_type_is_a((type), G_TYPE_INITABLE))
typedef struct _GInitableIface GInitableIface;
struct _GInitableIface {
  GTypeInterface g_iface;
  gboolean (*init)(GInitable *initable, GCancellable *cancellable, GError **error);
};
GType g_initable_get_type(void) G_GNUC_CONST;
gboolean g_initable_init(GInitable *initable, GCancellable *cancellable, GError **error);
gpointer g_initable_new(GType object_type, GCancellable *cancellable, GError **error, const gchar *first_property_name, ...);
gpointer g_initable_newv(GType object_type, guint n_parameters, GParameter *parameters, GCancellable *cancellable, GError **error);
GObject* g_initable_new_valist(GType object_type, const gchar *first_property_name, va_list var_args, GCancellable *cancellable, GError **error);
G_END_DECLS

#endif