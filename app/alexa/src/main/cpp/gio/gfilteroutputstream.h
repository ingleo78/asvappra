#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_FILTER_OUTPUT_STREAM_H__
#define __G_FILTER_OUTPUT_STREAM_H__

#include "goutputstream.h"

G_BEGIN_DECLS
#define G_TYPE_FILTER_OUTPUT_STREAM  (g_filter_output_stream_get_type())
#define G_FILTER_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_FILTER_OUTPUT_STREAM, GFilterOutputStream))
#define G_FILTER_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_FILTER_OUTPUT_STREAM, GFilterOutputStreamClass))
#define G_IS_FILTER_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_FILTER_OUTPUT_STREAM))
#define G_IS_FILTER_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_FILTER_OUTPUT_STREAM))
#define G_FILTER_OUTPUT_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_FILTER_OUTPUT_STREAM, GFilterOutputStreamClass))
typedef struct _GFilterOutputStreamClass GFilterOutputStreamClass;
struct _GFilterOutputStream {
  GOutputStream parent_instance;
  GOutputStream *base_stream;
};
struct _GFilterOutputStreamClass {
  GOutputStreamClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
};
GType g_filter_output_stream_get_type(void) G_GNUC_CONST;
GOutputStream *g_filter_output_stream_get_base_stream(GFilterOutputStream *stream);
gboolean g_filter_output_stream_get_close_base_stream(GFilterOutputStream *stream);
void g_filter_output_stream_set_close_base_stream(GFilterOutputStream *stream, gboolean close_base);
G_END_DECLS

#endif