#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_CONVERTER_OUTPUT_STREAM_H__
#define __G_CONVERTER_OUTPUT_STREAM_H__

#include "gfilteroutputstream.h"
#include "gconverter.h"

G_BEGIN_DECLS
#define G_TYPE_CONVERTER_OUTPUT_STREAM  (g_converter_output_stream_get_type ())
#define G_CONVERTER_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_CONVERTER_OUTPUT_STREAM, GConverterOutputStream))
#define G_CONVERTER_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_CONVERTER_OUTPUT_STREAM, GConverterOutputStreamClass))
#define G_IS_CONVERTER_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_CONVERTER_OUTPUT_STREAM))
#define G_IS_CONVERTER_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_CONVERTER_OUTPUT_STREAM))
#define G_CONVERTER_OUTPUT_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), G_TYPE_CONVERTER_OUTPUT_STREAM, GConverterOutputStreamClass))
typedef struct _GConverterOutputStreamClass GConverterOutputStreamClass;
typedef struct _GConverterOutputStreamPrivate GConverterOutputStreamPrivate;
struct _GConverterOutputStream {
  GFilterOutputStream parent_instance;
  GConverterOutputStreamPrivate *priv;
};
struct _GConverterOutputStreamClass {
  GFilterOutputStreamClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
GType g_converter_output_stream_get_type(void) G_GNUC_CONST;
GOutputStream *g_converter_output_stream_new(GOutputStream *base_stream, GConverter *converter);
GConverter *g_converter_output_stream_get_converter(GConverterOutputStream *converter_stream);
G_END_DECLS

#endif