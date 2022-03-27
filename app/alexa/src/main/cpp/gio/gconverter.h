#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_CONVERTER_H__
#define __G_CONVERTER_H__

#include "../gobject/gtype.h"
#include "giotypes.h"
#include "gioenums.h"

G_BEGIN_DECLS
#define G_TYPE_CONVERTER  (g_converter_get_type())
#define G_CONVERTER(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_CONVERTER, GConverter))
#define G_IS_CONVERTER(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_CONVERTER))
#define G_CONVERTER_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_CONVERTER, GConverterIface))
typedef struct _GConverterIface GConverterIface;
struct _GConverterIface {
  GTypeInterface g_iface;
  GConverterResult (*convert)(GConverter *converter, const void *inbuf, gsize inbuf_size, void *outbuf, gsize outbuf_size, GConverterFlags flags, gsize *bytes_read,
							  gsize *bytes_written, GError **error);
  void (*reset)(GConverter *converter);
};
GType g_converter_get_type(void) G_GNUC_CONST;
GConverterResult g_converter_convert(GConverter *converter, const void *inbuf, gsize inbuf_size, void *outbuf, gsize outbuf_size, GConverterFlags flags,
				      				 gsize *bytes_read, gsize *bytes_written, GError **error);
void g_converter_reset(GConverter *converter);
G_END_DECLS

#endif