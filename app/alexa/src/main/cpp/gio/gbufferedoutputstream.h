#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_BUFFERED_OUTPUT_STREAM_H__
#define __G_BUFFERED_OUTPUT_STREAM_H__

#include "../gobject/gtype.h"
#include "gfilteroutputstream.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_BUFFERED_OUTPUT_STREAM  (g_buffered_output_stream_get_type())
#define G_BUFFERED_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_BUFFERED_OUTPUT_STREAM, GBufferedOutputStream))
#define G_BUFFERED_OUTPUT_STREAM_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_BUFFERED_OUTPUT_STREAM, GBufferedOutputStreamClass))
#define G_IS_BUFFERED_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_BUFFERED_OUTPUT_STREAM))
#define G_IS_BUFFERED_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_BUFFERED_OUTPUT_STREAM))
#define G_BUFFERED_OUTPUT_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_BUFFERED_OUTPUT_STREAM, GBufferedOutputStreamClass))
typedef struct _GBufferedOutputStreamClass GBufferedOutputStreamClass;
typedef struct _GBufferedOutputStreamPrivate GBufferedOutputStreamPrivate;
struct _GBufferedOutputStream {
  GFilterOutputStream parent_instance;
  GBufferedOutputStreamPrivate *priv;
};
struct _GBufferedOutputStreamClass {
  GFilterOutputStreamClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
};
GType g_buffered_output_stream_get_type(void) G_GNUC_CONST;
GOutputStream* g_buffered_output_stream_new(GOutputStream *base_stream);
GOutputStream* g_buffered_output_stream_new_sized(GOutputStream *base_stream, gsize size);
gsize g_buffered_output_stream_get_buffer_size(GBufferedOutputStream *stream);
void g_buffered_output_stream_set_buffer_size(GBufferedOutputStream *stream, gsize size);
gboolean g_buffered_output_stream_get_auto_grow(GBufferedOutputStream *stream);
void g_buffered_output_stream_set_auto_grow(GBufferedOutputStream *stream, gboolean auto_grow);
G_END_DECLS

#endif