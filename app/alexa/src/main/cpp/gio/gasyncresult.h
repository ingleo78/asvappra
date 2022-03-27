#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_ASYNC_RESULT_H__
#define __G_ASYNC_RESULT_H__

#include "../gobject/gtype.h"
#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_ASYNC_RESULT  (g_async_result_get_type())
#define G_ASYNC_RESULT(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_ASYNC_RESULT, GAsyncResult))
#define G_IS_ASYNC_RESULT(obj)	(G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_ASYNC_RESULT))
#define G_ASYNC_RESULT_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_ASYNC_RESULT, GAsyncResultIface))
typedef struct _GAsyncResultIface GAsyncResultIface;
struct _GAsyncResultIface {
  GTypeInterface g_iface;
  gpointer (*get_user_data)(GAsyncResult *res);
  GObject *(*get_source_object)(GAsyncResult *res);
  gboolean (*is_tagged)(GAsyncResult *res, gpointer source_tag);
};
GType g_async_result_get_type(void) G_GNUC_CONST;
gpointer g_async_result_get_user_data(GAsyncResult *res);
GObject *g_async_result_get_source_object(GAsyncResult *res);
G_END_DECLS

#endif