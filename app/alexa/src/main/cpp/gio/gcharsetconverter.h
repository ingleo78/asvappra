#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_CHARSET_CONVERTER_H__
#define __G_CHARSET_CONVERTER_H__

#include "../gobject/gobject.h"
#include "../gobject/gtype.h"
#include "gconverter.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_CHARSET_CONVERTER  (g_charset_converter_get_type())
#define G_CHARSET_CONVERTER(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_CHARSET_CONVERTER, GCharsetConverter))
#define G_CHARSET_CONVERTER_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_CHARSET_CONVERTER, GCharsetConverterClass))
#define G_IS_CHARSET_CONVERTER(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_CHARSET_CONVERTER))
#define G_IS_CHARSET_CONVERTER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_CHARSET_CONVERTER))
#define G_CHARSET_CONVERTER_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_CHARSET_CONVERTER, GCharsetConverterClass))
typedef struct _GCharsetConverterClass GCharsetConverterClass;
struct _GCharsetConverterClass {
  GObjectClass parent_class;
};
GType g_charset_converter_get_type(void) G_GNUC_CONST;
GCharsetConverter *g_charset_converter_new(const gchar *to_charset, const gchar *from_charset, GError **error);
void g_charset_converter_set_use_fallback(GCharsetConverter *converter, gboolean use_fallback);
gboolean g_charset_converter_get_use_fallback(GCharsetConverter *converter);
guint g_charset_converter_get_num_fallbacks(GCharsetConverter *converter);
G_END_DECLS

#endif