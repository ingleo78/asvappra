#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_CONVERTER_INPUT_STREAM_H__
#define __G_CONVERTER_INPUT_STREAM_H__

#include "gfilterinputstream.h"
#include "gconverter.h"

G_BEGIN_DECLS
#define G_TYPE_CONVERTER_INPUT_STREAM  (g_converter_input_stream_get_type ())
#define G_CONVERTER_INPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_CONVERTER_INPUT_STREAM, GConverterInputStream))
#define G_CONVERTER_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_CONVERTER_INPUT_STREAM, GConverterInputStreamClass))
#define G_IS_CONVERTER_INPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_CONVERTER_INPUT_STREAM))
#define G_IS_CONVERTER_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_CONVERTER_INPUT_STREAM))
#define G_CONVERTER_INPUT_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), G_TYPE_CONVERTER_INPUT_STREAM, GConverterInputStreamClass))
typedef struct _GConverterInputStreamClass GConverterInputStreamClass;
typedef struct _GConverterInputStreamPrivate GConverterInputStreamPrivate;
struct _GConverterInputStream {
  GFilterInputStream parent_instance;
  GConverterInputStreamPrivate *priv;
};
struct _GConverterInputStreamClass {
  GFilterInputStreamClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
GType g_converter_input_stream_get_type(void) G_GNUC_CONST;
GInputStream *g_converter_input_stream_new(GInputStream *base_stream, GConverter *converter);
GConverter *g_converter_input_stream_get_converter(GConverterInputStream *converter_stream);
G_END_DECLS

#endif