#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_BUFFERED_INPUT_STREAM_H__
#define __G_BUFFERED_INPUT_STREAM_H__

#include "../gobject/gtype.h"
#include "../gobject/gobject.h"
#include "gfilterinputstream.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_BUFFERED_INPUT_STREAM   (g_buffered_input_stream_get_type())
#define G_BUFFERED_INPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_BUFFERED_INPUT_STREAM, GBufferedInputStream))
#define G_BUFFERED_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_BUFFERED_INPUT_STREAM, GBufferedInputStreamClass))
#define G_IS_BUFFERED_INPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_BUFFERED_INPUT_STREAM))
#define G_IS_BUFFERED_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_BUFFERED_INPUT_STREAM))
#define G_BUFFERED_INPUT_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_BUFFERED_INPUT_STREAM, GBufferedInputStreamClass))
typedef struct _GBufferedInputStreamClass GBufferedInputStreamClass;
typedef struct _GBufferedInputStreamPrivate GBufferedInputStreamPrivate;
struct _GBufferedInputStream {
  GFilterInputStream parent_instance;
  GBufferedInputStreamPrivate *priv;
};
struct _GBufferedInputStreamClass {
  GFilterInputStreamClass parent_class;
  gssize (*fill)(GBufferedInputStream *stream, gssize count, GCancellable *cancellable, GError **error);
  void (*fill_async)(GBufferedInputStream *stream, gssize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gssize (*fill_finish)(GBufferedInputStream *stream, GAsyncResult *result, GError **error);
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
GType g_buffered_input_stream_get_type(void) G_GNUC_CONST;
GInputStream* g_buffered_input_stream_new(GInputStream *base_stream);
GInputStream* g_buffered_input_stream_new_sized(GInputStream *base_stream, gsize size);
gsize g_buffered_input_stream_get_buffer_size(GBufferedInputStream *stream);
void g_buffered_input_stream_set_buffer_size(GBufferedInputStream *stream, gsize size);
gsize g_buffered_input_stream_get_available(GBufferedInputStream *stream);
gsize g_buffered_input_stream_peek(GBufferedInputStream *stream, void *buffer, gsize offset, gsize count);
const void* g_buffered_input_stream_peek_buffer(GBufferedInputStream *stream, gsize *count);
gssize g_buffered_input_stream_fill(GBufferedInputStream *stream, gssize count, GCancellable *cancellable, GError **error);
void g_buffered_input_stream_fill_async(GBufferedInputStream *stream, gssize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
						                gpointer user_data);
gssize g_buffered_input_stream_fill_finish(GBufferedInputStream  *stream, GAsyncResult *result, GError **error);
int g_buffered_input_stream_read_byte(GBufferedInputStream *stream, GCancellable *cancellable, GError **error);
G_END_DECLS

#endif