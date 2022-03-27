#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_ASYNC_INITABLE_H__
#define __G_ASYNC_INITABLE_H__

#include <stdarg.h>
#include "../gobject/gtype.h"
#include "../gobject/gobject.h"
#include "giotypes.h"
#include "ginitable.h"

G_BEGIN_DECLS
#define G_TYPE_ASYNC_INITABLE  (g_async_initable_get_type ())
#define G_ASYNC_INITABLE(obj)  (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_ASYNC_INITABLE, GAsyncInitable))
#define G_IS_ASYNC_INITABLE(obj)  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_ASYNC_INITABLE))
#define G_ASYNC_INITABLE_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), G_TYPE_ASYNC_INITABLE, GAsyncInitableIface))
#define G_TYPE_IS_ASYNC_INITABLE(type)  (g_type_is_a ((type), G_TYPE_ASYNC_INITABLE))
typedef struct _GAsyncInitableIface GAsyncInitableIface;
struct _GAsyncInitableIface {
  GTypeInterface g_iface;
  void (*init_async)(GAsyncInitable *initable, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*init_finish)(GAsyncInitable *initable, GAsyncResult *res, GError **error);
};
GType g_async_initable_get_type(void) G_GNUC_CONST;
void g_async_initable_init_async(GAsyncInitable *initable, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_async_initable_init_finish(GAsyncInitable *initable, GAsyncResult *res, GError **error);
void g_async_initable_new_async(GType object_type, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data,
					    		const gchar *first_property_name, ...);
void g_async_initable_newv_async(GType object_type, guint n_parameters, GParameter *parameters, int io_priority, GCancellable *cancellable,
					    		 GAsyncReadyCallback callback, gpointer user_data);
void g_async_initable_new_valist_async(GType object_type, const gchar *first_property_name, va_list var_args, int io_priority, GCancellable *cancellable,
					    			   GAsyncReadyCallback callback, gpointer user_data);
GObject *g_async_initable_new_finish(GAsyncInitable *initable, GAsyncResult *res, GError **error);
G_END_DECLS

#endif