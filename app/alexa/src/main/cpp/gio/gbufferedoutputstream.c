#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "../gobject/gobject.h"
#include "../glib/glib-object.h"
#include "config.h"
#include "gbufferedoutputstream.h"
#include "goutputstream.h"
#include "gsimpleasyncresult.h"
#include "string.h"

#define DEFAULT_BUFFER_SIZE 4096
struct _GBufferedOutputStreamPrivate {
  guint8 *buffer; 
  gsize len;
  goffset pos;
  gboolean auto_grow;
};
enum {
  PROP_0,
  PROP_BUFSIZE,
  PROP_AUTO_GROW
};
static void g_buffered_output_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void g_buffered_output_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void g_buffered_output_stream_finalize(GObject *object);
static gssize g_buffered_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error);
static gboolean g_buffered_output_stream_flush(GOutputStream *stream, GCancellable *cancellable, GError **error);
static gboolean g_buffered_output_stream_close(GOutputStream *stream, GCancellable *cancellable, GError **error);
static void g_buffered_output_stream_write_async(GOutputStream *stream, const void *buffer, gsize count, int io_priority, GCancellable *cancellable,
                                                 GAsyncReadyCallback callback, gpointer data);
static gssize g_buffered_output_stream_write_finish(GOutputStream *stream, GAsyncResult *result, GError **error);
static void g_buffered_output_stream_flush_async(GOutputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
static gboolean g_buffered_output_stream_flush_finish(GOutputStream *stream, GAsyncResult *result, GError **error);
static void     g_buffered_output_stream_close_async(GOutputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                                     gpointer data);
static gboolean g_buffered_output_stream_close_finish(GOutputStream *stream, GAsyncResult *result, GError **error);
G_DEFINE_TYPE(GBufferedOutputStream, g_buffered_output_stream, G_TYPE_FILTER_OUTPUT_STREAM)
static void g_buffered_output_stream_class_init(GBufferedOutputStreamClass *klass) {
  GObjectClass *object_class;
  GOutputStreamClass *ostream_class;
  g_type_class_add_private (klass, sizeof(GBufferedOutputStreamPrivate));
  object_class = G_OBJECT_CLASS(klass);
  object_class->get_property = g_buffered_output_stream_get_property;
  object_class->set_property = g_buffered_output_stream_set_property;
  object_class->finalize     = g_buffered_output_stream_finalize;
  ostream_class = G_OUTPUT_STREAM_CLASS(klass);
  ostream_class->write_fn = g_buffered_output_stream_write;
  ostream_class->flush = g_buffered_output_stream_flush;
  ostream_class->close_fn = g_buffered_output_stream_close;
  ostream_class->write_async  = g_buffered_output_stream_write_async;
  ostream_class->write_finish = g_buffered_output_stream_write_finish;
  ostream_class->flush_async  = g_buffered_output_stream_flush_async;
  ostream_class->flush_finish = g_buffered_output_stream_flush_finish;
  ostream_class->close_async  = g_buffered_output_stream_close_async;
  ostream_class->close_finish = g_buffered_output_stream_close_finish;
  g_object_class_install_property(object_class,PROP_BUFSIZE,g_param_spec_uint("buffer-size","Buffer Size",
                                  "The size of the backend buffer", 1, G_MAXUINT, DEFAULT_BUFFER_SIZE,G_PARAM_READWRITE |
                                  G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB));
  g_object_class_install_property(object_class,PROP_AUTO_GROW,g_param_spec_boolean("auto-grow","Auto-grow",
                                  "Whether the buffer should automatically grow", FALSE, G_PARAM_READWRITE |
                                  G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB));

}
gsize g_buffered_output_stream_get_buffer_size(GBufferedOutputStream *stream) {
  g_return_val_if_fail(G_IS_BUFFERED_OUTPUT_STREAM(stream), -1);
  return stream->priv->len;
}
void g_buffered_output_stream_set_buffer_size(GBufferedOutputStream *stream, gsize size) {
  GBufferedOutputStreamPrivate *priv;
  guint8 *buffer;
  g_return_if_fail(G_IS_BUFFERED_OUTPUT_STREAM(stream));
  priv = stream->priv;
  if (size == priv->len) return;
  if (priv->buffer) {
      size = MAX(size, priv->pos);
      buffer = g_malloc(size);
      memcpy(buffer, priv->buffer, priv->pos);
      g_free(priv->buffer);
      priv->buffer = buffer;
      priv->len = size;
  } else {
      priv->buffer = g_malloc(size);
      priv->len = size;
      priv->pos = 0;
  }
  g_object_notify(G_OBJECT(stream), "buffer-size");
}
gboolean g_buffered_output_stream_get_auto_grow(GBufferedOutputStream *stream) {
  g_return_val_if_fail (G_IS_BUFFERED_OUTPUT_STREAM (stream), FALSE);
  return stream->priv->auto_grow;
}
void
g_buffered_output_stream_set_auto_grow(GBufferedOutputStream *stream, gboolean auto_grow) {
  GBufferedOutputStreamPrivate *priv;
  g_return_if_fail(G_IS_BUFFERED_OUTPUT_STREAM (stream));
  priv = stream->priv;
  auto_grow = auto_grow != FALSE;
  if (priv->auto_grow != auto_grow) {
      priv->auto_grow = auto_grow;
      g_object_notify(G_OBJECT(stream), "auto-grow");
  }
}
static void g_buffered_output_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GBufferedOutputStream *stream;
  stream = G_BUFFERED_OUTPUT_STREAM(object);
  switch(prop_id) {
      case PROP_BUFSIZE: g_buffered_output_stream_set_buffer_size(stream, g_value_get_uint(value)); break;
      case PROP_AUTO_GROW: g_buffered_output_stream_set_auto_grow(stream, g_value_get_boolean(value)); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_buffered_output_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GBufferedOutputStream *buffered_stream;
  GBufferedOutputStreamPrivate *priv;
  buffered_stream = G_BUFFERED_OUTPUT_STREAM (object);
  priv = buffered_stream->priv;
  switch(prop_id) {
      case PROP_BUFSIZE: g_value_set_uint(value, priv->len); break;
      case PROP_AUTO_GROW: g_value_set_boolean(value, priv->auto_grow); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_buffered_output_stream_finalize(GObject *object) {
  GBufferedOutputStream *stream;
  GBufferedOutputStreamPrivate *priv;
  stream = G_BUFFERED_OUTPUT_STREAM(object);
  priv = stream->priv;
  g_free (priv->buffer);
  G_OBJECT_CLASS(g_buffered_output_stream_parent_class)->finalize(object);
}
static void g_buffered_output_stream_init(GBufferedOutputStream *stream) {
  stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(stream, G_TYPE_BUFFERED_OUTPUT_STREAM, GBufferedOutputStreamPrivate);
}
GOutputStream *g_buffered_output_stream_new(GOutputStream *base_stream) {
  GOutputStream *stream;
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(base_stream), NULL);
  stream = g_object_new(G_TYPE_BUFFERED_OUTPUT_STREAM,"base-stream", base_stream, NULL);
  return stream;
}
GOutputStream *g_buffered_output_stream_new_sized(GOutputStream *base_stream, gsize size) {
  GOutputStream *stream;
  g_return_val_if_fail (G_IS_OUTPUT_STREAM (base_stream), NULL);
  stream = g_object_new (G_TYPE_BUFFERED_OUTPUT_STREAM,"base-stream", base_stream, "buffer-size", size, NULL);
  return stream;
}
static gboolean flush_buffer(GBufferedOutputStream *stream, GCancellable *cancellable, GError **error) {
  GBufferedOutputStreamPrivate *priv;
  GOutputStream *base_stream;
  gboolean res;
  gsize bytes_written;
  gsize count;
  priv = stream->priv;
  bytes_written = 0;
  base_stream = G_FILTER_OUTPUT_STREAM (stream)->base_stream;
  g_return_val_if_fail (G_IS_OUTPUT_STREAM (base_stream), FALSE);
  res = g_output_stream_write_all (base_stream, priv->buffer, priv->pos, &bytes_written, cancellable, error);
  count = priv->pos - bytes_written;
  if (count > 0) g_memmove (priv->buffer, priv->buffer + bytes_written, count);
  priv->pos -= bytes_written;
  return res;
}
static gssize g_buffered_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error) {
  GBufferedOutputStream *bstream;
  GBufferedOutputStreamPrivate *priv;
  gboolean res;
  gsize n;
  gsize new_size;
  bstream = G_BUFFERED_OUTPUT_STREAM(stream);
  priv = bstream->priv;
  n = priv->len - priv->pos;
  if (priv->auto_grow && n < count) {
      new_size = MAX(priv->len * 2, priv->len + count);
      g_buffered_output_stream_set_buffer_size(bstream, new_size);
  } else if (n == 0) {
      res = flush_buffer(bstream, cancellable, error);
      if (res == FALSE) return -1;
  }
  n = priv->len - priv->pos;
  count = MIN (count, n);
  memcpy (priv->buffer + priv->pos, buffer, count);
  priv->pos += count;
  return count;
}
static gboolean g_buffered_output_stream_flush(GOutputStream *stream, GCancellable *cancellable, GError **error) {
  GBufferedOutputStream *bstream;
  GOutputStream *base_stream;
  gboolean res;
  bstream = G_BUFFERED_OUTPUT_STREAM(stream);
  base_stream = G_FILTER_OUTPUT_STREAM(stream)->base_stream;
  res = flush_buffer(bstream, cancellable, error);
  if (res == FALSE) return FALSE;
  res = g_output_stream_flush(base_stream, cancellable, error);
  return res;
}
static gboolean g_buffered_output_stream_close(GOutputStream *stream, GCancellable *cancellable, GError **error) {
  GBufferedOutputStream *bstream;
  GOutputStream *base_stream;
  gboolean res;
  bstream = G_BUFFERED_OUTPUT_STREAM(stream);
  base_stream = G_FILTER_OUTPUT_STREAM(bstream)->base_stream;
  res = flush_buffer(bstream, cancellable, error);
  if (g_filter_output_stream_get_close_base_stream(G_FILTER_OUTPUT_STREAM(stream))) {
      if (res) res = g_output_stream_close(base_stream, cancellable, error);
      else g_output_stream_close(base_stream, cancellable, NULL);
  }
  return res;
}
typedef struct {
  guint flush_stream : 1;
  guint close_stream : 1;
} FlushData;
static void free_flush_data(gpointer data) {
  g_slice_free(FlushData, data);
}
static void flush_buffer_thread(GSimpleAsyncResult *result, GObject *object, GCancellable *cancellable) {
  GBufferedOutputStream *stream;
  GOutputStream *base_stream;
  FlushData *fdata;
  gboolean res;
  GError *error = NULL;
  stream = G_BUFFERED_OUTPUT_STREAM(object);
  fdata = g_simple_async_result_get_op_res_gpointer(result);
  base_stream = G_FILTER_OUTPUT_STREAM(stream)->base_stream;
  res = flush_buffer(stream, cancellable, &error);
  if (res && fdata->flush_stream) res = g_output_stream_flush(base_stream, cancellable, &error);
  if (fdata->close_stream) {
      if (g_filter_output_stream_get_close_base_stream(G_FILTER_OUTPUT_STREAM (stream))) {
          if (res == FALSE) g_output_stream_close(base_stream, cancellable, NULL);
          else res = g_output_stream_close(base_stream, cancellable, &error);
      }
  }
  if (res == FALSE) g_simple_async_result_take_error(result, error);
}
typedef struct {
  FlushData fdata;
  gsize  count;
  const void  *buffer;
} WriteData;
static void free_write_data(gpointer data) {
  g_slice_free(WriteData, data);
}
static void g_buffered_output_stream_write_async(GOutputStream *stream, const void *buffer, gsize count, int io_priority, GCancellable *cancellable,
                                                 GAsyncReadyCallback callback, gpointer data) {
  GBufferedOutputStream *buffered_stream;
  GBufferedOutputStreamPrivate *priv;
  GSimpleAsyncResult *res;
  WriteData *wdata;
  buffered_stream = G_BUFFERED_OUTPUT_STREAM(stream);
  priv = buffered_stream->priv;
  wdata = g_slice_new(WriteData);
  wdata->count  = count;
  wdata->buffer = buffer;
  res = g_simple_async_result_new(G_OBJECT(stream), callback, data, g_buffered_output_stream_write_async);
  g_simple_async_result_set_op_res_gpointer(res, wdata, free_write_data);
  if (priv->len - priv->pos > 0) g_simple_async_result_complete_in_idle (res);
  else {
      wdata->fdata.flush_stream = FALSE;
      wdata->fdata.close_stream = FALSE;
      g_simple_async_result_run_in_thread(res, flush_buffer_thread, io_priority, cancellable);
      g_object_unref(res);
  }
}
static gssize g_buffered_output_stream_write_finish(GOutputStream *stream, GAsyncResult *result, GError **error) {
  GBufferedOutputStreamPrivate *priv;
  GBufferedOutputStream *buffered_stream;
  GSimpleAsyncResult *simple;
  WriteData *wdata;
  gssize count;
  simple = G_SIMPLE_ASYNC_RESULT(result);
  buffered_stream = G_BUFFERED_OUTPUT_STREAM(stream);
  priv = buffered_stream->priv;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_buffered_output_stream_write_async);
  wdata = g_simple_async_result_get_op_res_gpointer(simple);
  count = priv->len - priv->pos; 
  count = MIN(wdata->count, count);
  memcpy(priv->buffer + priv->pos, wdata->buffer, count);
  priv->pos += count;
  return count;
}
static void g_buffered_output_stream_flush_async(GOutputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data) {
  GSimpleAsyncResult *res;
  FlushData *fdata;
  fdata = g_slice_new(FlushData);
  fdata->flush_stream = TRUE;
  fdata->close_stream = FALSE;
  res = g_simple_async_result_new(G_OBJECT(stream), callback, data, g_buffered_output_stream_flush_async);
  g_simple_async_result_set_op_res_gpointer(res, fdata, free_flush_data);
  g_simple_async_result_run_in_thread(res, flush_buffer_thread, io_priority, cancellable);
  g_object_unref(res);
}
static gboolean g_buffered_output_stream_flush_finish(GOutputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  simple = G_SIMPLE_ASYNC_RESULT(result);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_buffered_output_stream_flush_async);
  return TRUE;
}
static void g_buffered_output_stream_close_async(GOutputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data) {
  GSimpleAsyncResult *res;
  FlushData *fdata;
  fdata = g_slice_new(FlushData);
  fdata->close_stream = TRUE;
  res = g_simple_async_result_new(G_OBJECT(stream), callback, data, g_buffered_output_stream_close_async);
  g_simple_async_result_set_op_res_gpointer(res, fdata, free_flush_data);
  g_simple_async_result_run_in_thread(res, flush_buffer_thread, io_priority, cancellable);
  g_object_unref(res);
}
static gboolean g_buffered_output_stream_close_finish(GOutputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  simple = G_SIMPLE_ASYNC_RESULT(result);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_buffered_output_stream_close_async);
  return TRUE;
}