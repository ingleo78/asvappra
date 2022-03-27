#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/glib-object.h"
#include "../glib/glib.h"
#include "config.h"
#include "gbufferedinputstream.h"
#include "ginputstream.h"
#include "gcancellable.h"
#include "gasyncresult.h"
#include "gsimpleasyncresult.h"
#include "gioerror.h"

#define DEFAULT_BUFFER_SIZE 4096
struct _GBufferedInputStreamPrivate {
  guint8 *buffer;
  gsize len;
  gsize pos;
  gsize end;
  GAsyncReadyCallback outstanding_callback;
};
enum {
  PROP_0,
  PROP_BUFSIZE
};
static void g_buffered_input_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void g_buffered_input_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void g_buffered_input_stream_finalize(GObject *object);
static gssize g_buffered_input_stream_skip(GInputStream *stream, gsize count, GCancellable *cancellable, GError **error);
static void g_buffered_input_stream_skip_async(GInputStream *stream, gsize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                               gpointer user_data);
static gssize g_buffered_input_stream_skip_finish(GInputStream *stream, GAsyncResult *result, GError **error);
static gssize g_buffered_input_stream_read(GInputStream *stream, void *buffer, gsize count, GCancellable  *cancellable, GError **error);
static void   g_buffered_input_stream_read_async(GInputStream *stream, void *buffer, gsize count, int io_priority, GCancellable *cancellable,
                                                 GAsyncReadyCallback callback, gpointer user_data);
static gssize g_buffered_input_stream_read_finish(GInputStream *stream, GAsyncResult *result, GError **error);
static gssize g_buffered_input_stream_real_fill(GBufferedInputStream *stream, gssize count, GCancellable *cancellable, GError **error);
static void g_buffered_input_stream_real_fill_async(GBufferedInputStream *stream, gssize count, int io_priority, GCancellable *cancellable,
                                                    GAsyncReadyCallback callback, gpointer user_data);
static gssize g_buffered_input_stream_real_fill_finish(GBufferedInputStream *stream, GAsyncResult *result, GError **error);
static void compact_buffer(GBufferedInputStream *stream);
G_DEFINE_TYPE(GBufferedInputStream, g_buffered_input_stream, G_TYPE_FILTER_INPUT_STREAM);
static void g_buffered_input_stream_class_init(GBufferedInputStreamClass *klass) {
  GObjectClass *object_class;
  GInputStreamClass *istream_class;
  GBufferedInputStreamClass *bstream_class;
  g_type_class_add_private(klass, sizeof(GBufferedInputStreamPrivate));
  object_class = G_OBJECT_CLASS(klass);
  object_class->get_property = g_buffered_input_stream_get_property;
  object_class->set_property = g_buffered_input_stream_set_property;
  object_class->finalize = g_buffered_input_stream_finalize;
  istream_class = G_INPUT_STREAM_CLASS(klass);
  istream_class->skip = g_buffered_input_stream_skip;
  istream_class->skip_async  = g_buffered_input_stream_skip_async;
  istream_class->skip_finish = g_buffered_input_stream_skip_finish;
  istream_class->read_fn = g_buffered_input_stream_read;
  istream_class->read_async  = g_buffered_input_stream_read_async;
  istream_class->read_finish = g_buffered_input_stream_read_finish;
  bstream_class = G_BUFFERED_INPUT_STREAM_CLASS(klass);
  bstream_class->fill = g_buffered_input_stream_real_fill;
  bstream_class->fill_async = g_buffered_input_stream_real_fill_async;
  bstream_class->fill_finish = g_buffered_input_stream_real_fill_finish;
  g_object_class_install_property(object_class,PROP_BUFSIZE,g_param_spec_uint("buffer-size", P_("Buffer Size"),
                                  P_("The size of the backend buffer"),1,G_MAXUINT, DEFAULT_BUFFER_SIZE,G_PARAM_READWRITE |
                                  G_PARAM_CONSTRUCT | G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB));
}
gsize g_buffered_input_stream_get_buffer_size(GBufferedInputStream  *stream) {
  g_return_val_if_fail(G_IS_BUFFERED_INPUT_STREAM(stream), 0);
  return stream->priv->len;
}
void g_buffered_input_stream_set_buffer_size(GBufferedInputStream *stream, gsize size) {
  GBufferedInputStreamPrivate *priv;
  gsize in_buffer;
  guint8 *buffer;
  g_return_if_fail(G_IS_BUFFERED_INPUT_STREAM(stream));
  priv = stream->priv;
  if (priv->len == size) return;
  if (priv->buffer) {
      in_buffer = priv->end - priv->pos;
      size = MAX(size, in_buffer);
      buffer = g_malloc(size);
      memcpy(buffer, priv->buffer + priv->pos, in_buffer);
      priv->len = size;
      priv->pos = 0;
      priv->end = in_buffer;
      g_free(priv->buffer);
      priv->buffer = buffer;
  } else {
      priv->len = size;
      priv->pos = 0;
      priv->end = 0;
      priv->buffer = g_malloc(size);
  }
  g_object_notify(G_OBJECT(stream), "buffer-size");
}
static void g_buffered_input_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GBufferedInputStream *bstream;
  bstream = G_BUFFERED_INPUT_STREAM(object);
  switch(prop_id) {
      case PROP_BUFSIZE: g_buffered_input_stream_set_buffer_size(bstream, g_value_get_uint(value)); break;
      default:G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_buffered_input_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GBufferedInputStreamPrivate *priv;
  GBufferedInputStream *bstream;
  bstream = G_BUFFERED_INPUT_STREAM(object);
  priv = bstream->priv;
  switch(prop_id) {
      case PROP_BUFSIZE: g_value_set_uint(value, priv->len); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}

static void g_buffered_input_stream_finalize(GObject *object) {
  GBufferedInputStreamPrivate *priv;
  GBufferedInputStream *stream;
  stream = G_BUFFERED_INPUT_STREAM(object);
  priv = stream->priv;
  g_free(priv->buffer);
  G_OBJECT_CLASS(g_buffered_input_stream_parent_class)->finalize (object);
}
static void g_buffered_input_stream_init(GBufferedInputStream *stream) {
  stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(stream, G_TYPE_BUFFERED_INPUT_STREAM, GBufferedInputStreamPrivate);
}
GInputStream *g_buffered_input_stream_new(GInputStream *base_stream) {
  GInputStream *stream;
  g_return_val_if_fail(G_IS_INPUT_STREAM (base_stream), NULL);
  stream = g_object_new(G_TYPE_BUFFERED_INPUT_STREAM, "base-stream", base_stream, NULL);
  return stream;
}
GInputStream *g_buffered_input_stream_new_sized(GInputStream *base_stream, gsize size) {
  GInputStream *stream;
  g_return_val_if_fail(G_IS_INPUT_STREAM (base_stream), NULL);
  stream = g_object_new(G_TYPE_BUFFERED_INPUT_STREAM, "base-stream", base_stream, "buffer-size", (guint)size, NULL);
  return stream;
}
gssize g_buffered_input_stream_fill(GBufferedInputStream *stream, gssize count, GCancellable *cancellable, GError **error) {
  GBufferedInputStreamClass *class;
  GInputStream *input_stream;
  gssize res;
  g_return_val_if_fail(G_IS_BUFFERED_INPUT_STREAM(stream), -1);
  input_stream = G_INPUT_STREAM(stream);
  if (count < -1) {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, _("Too large count value passed to %s"), G_STRFUNC);
      return -1;
  }
  if (!g_input_stream_set_pending(input_stream, error)) return -1;
  if (cancellable) g_cancellable_push_current(cancellable);
  class = G_BUFFERED_INPUT_STREAM_GET_CLASS(stream);
  res = class->fill(stream, count, cancellable, error);
  if (cancellable) g_cancellable_pop_current(cancellable);
  g_input_stream_clear_pending(input_stream);
  return res;
}
static void async_fill_callback_wrapper(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GBufferedInputStream *stream = G_BUFFERED_INPUT_STREAM(source_object);
  g_input_stream_clear_pending (G_INPUT_STREAM(stream));
  (*stream->priv->outstanding_callback)(source_object, res, user_data);
  g_object_unref(stream);
}
void g_buffered_input_stream_fill_async(GBufferedInputStream *stream, gssize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                        gpointer user_data) {
  GBufferedInputStreamClass *class;
  GSimpleAsyncResult *simple;
  GError *error = NULL;
  g_return_if_fail(G_IS_BUFFERED_INPUT_STREAM(stream));
  if (count == 0) {
      simple = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_buffered_input_stream_fill_async);
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  if (count < -1) {
      g_simple_async_report_error_in_idle(G_OBJECT(stream), callback, user_data, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
                                          "Too large count value passed to %s", G_STRFUNC);
      return;
  }
  if (!g_input_stream_set_pending(G_INPUT_STREAM(stream), &error)) {
      g_simple_async_report_take_gerror_in_idle(G_OBJECT(stream), callback, user_data, error);
      return;
  }
  class = G_BUFFERED_INPUT_STREAM_GET_CLASS(stream);
  stream->priv->outstanding_callback = callback;
  g_object_ref(stream);
  class->fill_async(stream, count, io_priority, cancellable, async_fill_callback_wrapper, user_data);
}
gssize g_buffered_input_stream_fill_finish(GBufferedInputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  GBufferedInputStreamClass *class;
  g_return_val_if_fail(G_IS_BUFFERED_INPUT_STREAM(stream), -1);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), -1);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return -1;
      if (g_simple_async_result_get_source_tag(simple) == g_buffered_input_stream_fill_async)
        return 0;
  }
  class = G_BUFFERED_INPUT_STREAM_GET_CLASS(stream);
  return class->fill_finish(stream, result, error);
}
gsize g_buffered_input_stream_get_available(GBufferedInputStream *stream) {
  g_return_val_if_fail(G_IS_BUFFERED_INPUT_STREAM(stream), -1);
  return stream->priv->end - stream->priv->pos;
}
gsize g_buffered_input_stream_peek(GBufferedInputStream *stream, void *buffer, gsize offset, gsize count) {
  gsize available;
  gsize end;
  g_return_val_if_fail(G_IS_BUFFERED_INPUT_STREAM (stream), -1);
  g_return_val_if_fail(buffer != NULL, -1);
  available = g_buffered_input_stream_get_available(stream);
  if (offset > available) return 0;
  end = MIN(offset + count, available);
  count = end - offset;
  memcpy(buffer, stream->priv->buffer + stream->priv->pos + offset, count);
  return count;
}
const void* g_buffered_input_stream_peek_buffer(GBufferedInputStream *stream, gsize *count) {
  GBufferedInputStreamPrivate *priv;
  g_return_val_if_fail(G_IS_BUFFERED_INPUT_STREAM(stream), NULL);
  priv = stream->priv;
  if (count) *count = priv->end - priv->pos;
  return priv->buffer + priv->pos;
}
static void compact_buffer(GBufferedInputStream *stream) {
  GBufferedInputStreamPrivate *priv;
  gsize current_size;
  priv = stream->priv;
  current_size = priv->end - priv->pos;
  g_memmove(priv->buffer, priv->buffer + priv->pos, current_size);
  priv->pos = 0;
  priv->end = current_size;
}
static gssize
g_buffered_input_stream_real_fill(GBufferedInputStream *stream, gssize count, GCancellable *cancellable, GError **error) {
  GBufferedInputStreamPrivate *priv;
  GInputStream *base_stream;
  gssize nread;
  gsize in_buffer;
  priv = stream->priv;
  if (count == -1) count = priv->len;
  in_buffer = priv->end - priv->pos;
  count = MIN(count, priv->len - in_buffer);
  if (priv->len - priv->end < count) compact_buffer(stream);
  base_stream = G_FILTER_INPUT_STREAM(stream)->base_stream;
  nread = g_input_stream_read(base_stream, priv->buffer + priv->end, count, cancellable, error);
  if (nread > 0) priv->end += nread;
  return nread;
}
static gssize g_buffered_input_stream_skip(GInputStream *stream, gsize count, GCancellable *cancellable, GError **error) {
  GBufferedInputStream *bstream;
  GBufferedInputStreamPrivate *priv;
  GBufferedInputStreamClass *class;
  GInputStream *base_stream;
  gsize available, bytes_skipped;
  gssize nread;
  bstream = G_BUFFERED_INPUT_STREAM(stream);
  priv = bstream->priv;
  available = priv->end - priv->pos;
  if (count <= available) {
      priv->pos += count;
      return count;
  }
  priv->pos = 0;
  priv->end = 0;
  bytes_skipped = available;
  count -= available;
  if (bytes_skipped > 0) error = NULL;
  if (count > priv->len) {
      base_stream = G_FILTER_INPUT_STREAM(stream)->base_stream;
      nread = g_input_stream_skip(base_stream, count, cancellable, error);
      if (nread < 0 && bytes_skipped == 0) return -1;
      if (nread > 0) bytes_skipped += nread;
      return bytes_skipped;
  }
  class = G_BUFFERED_INPUT_STREAM_GET_CLASS(stream);
  nread = class->fill(bstream, priv->len, cancellable, error);
  if (nread < 0) {
      if (bytes_skipped == 0) return -1;
      else return bytes_skipped;
  }
  available = priv->end - priv->pos;
  count = MIN(count, available);
  bytes_skipped += count;
  priv->pos += count;
  return bytes_skipped;
}
static gssize g_buffered_input_stream_read(GInputStream *stream, void *buffer, gsize count, GCancellable *cancellable, GError **error) {
  GBufferedInputStream *bstream;
  GBufferedInputStreamPrivate *priv;
  GBufferedInputStreamClass *class;
  GInputStream *base_stream;
  gsize available, bytes_read;
  gssize nread;
  bstream = G_BUFFERED_INPUT_STREAM(stream);
  priv = bstream->priv;
  available = priv->end - priv->pos;
  if (count <= available) {
      memcpy(buffer, priv->buffer + priv->pos, count);
      priv->pos += count;
      return count;
  }
  memcpy(buffer, priv->buffer + priv->pos, available);
  priv->pos = 0;
  priv->end = 0;
  bytes_read = available;
  count -= available;
  if (bytes_read > 0) error = NULL;
  if (count > priv->len) {
      base_stream = G_FILTER_INPUT_STREAM(stream)->base_stream;
      nread = g_input_stream_read(base_stream, (char*)buffer + bytes_read, count, cancellable, error);
      if (nread < 0 && bytes_read == 0) return -1;
      if (nread > 0) bytes_read += nread;
      return bytes_read;
  }
  class = G_BUFFERED_INPUT_STREAM_GET_CLASS(stream);
  nread = class->fill(bstream, priv->len, cancellable, error);
  if (nread < 0) {
      if (bytes_read == 0) return -1;
      else return bytes_read;
  }
  available = priv->end - priv->pos;
  count = MIN(count, available);
  memcpy((char*)buffer + bytes_read, (char*)priv->buffer + priv->pos, count);
  bytes_read += count;
  priv->pos += count;
  return bytes_read;
}
int g_buffered_input_stream_read_byte(GBufferedInputStream *stream, GCancellable *cancellable, GError **error) {
  GBufferedInputStreamPrivate *priv;
  GBufferedInputStreamClass *class;
  GInputStream *input_stream;
  gsize available;
  gssize nread;
  g_return_val_if_fail (G_IS_BUFFERED_INPUT_STREAM (stream), -1);
  priv = stream->priv;
  input_stream = G_INPUT_STREAM (stream);
  if (g_input_stream_is_closed (input_stream)) {
      g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_CLOSED, _("Stream is already closed"));
      return -1;
  }
  if (!g_input_stream_set_pending (input_stream, error)) return -1;
  available = priv->end - priv->pos;
  if (available != 0) {
      g_input_stream_clear_pending (input_stream);
      return priv->buffer[priv->pos++];
  }
  if (cancellable) g_cancellable_push_current (cancellable);
  priv->pos = 0;
  priv->end = 0;
  class = G_BUFFERED_INPUT_STREAM_GET_CLASS (stream);
  nread = class->fill(stream, priv->len, cancellable, error);
  if (cancellable) g_cancellable_pop_current (cancellable);
  g_input_stream_clear_pending (input_stream);
  if (nread <= 0) return -1;
  return priv->buffer[priv->pos++];
}
static void fill_async_callback(GObject *source_object, GAsyncResult *result, gpointer user_data) {
  GError *error;
  gssize res;
  GSimpleAsyncResult *simple;
  simple = user_data;
  error = NULL;
  res = g_input_stream_read_finish(G_INPUT_STREAM(source_object), result, &error);
  g_simple_async_result_set_op_res_gssize(simple, res);
  if (res == -1) g_simple_async_result_take_error (simple, error);
  else {
      GBufferedInputStreamPrivate *priv;
      GObject *object;
      object = g_async_result_get_source_object(G_ASYNC_RESULT(simple));
      priv = G_BUFFERED_INPUT_STREAM(object)->priv;
      g_assert_cmpint(priv->end + res, <=, priv->len);
      priv->end += res;
      g_object_unref(object);
  }
  g_simple_async_result_complete(simple);
  g_object_unref(simple);
}
static void g_buffered_input_stream_real_fill_async(GBufferedInputStream *stream, gssize count, int io_priority, GCancellable *cancellable,
                                                    GAsyncReadyCallback callback, gpointer user_data) {
  GBufferedInputStreamPrivate *priv;
  GInputStream *base_stream;
  GSimpleAsyncResult *simple;
  gsize in_buffer;
  priv = stream->priv;
  if (count == -1) count = priv->len;
  in_buffer = priv->end - priv->pos;
  count = MIN(count, priv->len - in_buffer);
  if (priv->len - priv->end < count) compact_buffer(stream);
  simple = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_buffered_input_stream_real_fill_async);
  base_stream = G_FILTER_INPUT_STREAM(stream)->base_stream;
  g_input_stream_read_async(base_stream, priv->buffer + priv->end, count, io_priority, cancellable, fill_async_callback, simple);
}
static gssize g_buffered_input_stream_real_fill_finish(GBufferedInputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  gssize nread;
  simple = G_SIMPLE_ASYNC_RESULT(result);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_buffered_input_stream_real_fill_async);
  nread = g_simple_async_result_get_op_res_gssize(simple);
  return nread;
}
typedef struct {
  gssize bytes_read;
  gssize count;
  void *buffer;
} ReadAsyncData;
static void free_read_async_data(gpointer _data) {
  ReadAsyncData *data = _data;
  g_slice_free(ReadAsyncData, data);
}
static void large_read_callback(GObject *source_object, GAsyncResult *result, gpointer user_data) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (user_data);
  ReadAsyncData *data;
  GError *error;
  gssize nread;
  data = g_simple_async_result_get_op_res_gpointer(simple);
  error = NULL;
  nread = g_input_stream_read_finish(G_INPUT_STREAM(source_object), result, &error);
  if (nread < 0 && data->bytes_read == 0) g_simple_async_result_take_error(simple, error);
  else if (error) g_error_free(error);
  if (nread > 0) data->bytes_read += nread;
  g_simple_async_result_complete(simple);
  g_object_unref(simple);
}
static void read_fill_buffer_callback(GObject *source_object, GAsyncResult *result, gpointer user_data) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(user_data);
  GBufferedInputStream *bstream;
  GBufferedInputStreamPrivate *priv;
  ReadAsyncData *data;
  GError *error;
  gssize nread;
  gsize available;
  bstream = G_BUFFERED_INPUT_STREAM(source_object);
  priv = bstream->priv;
  data = g_simple_async_result_get_op_res_gpointer(simple);
  error = NULL;
  nread = g_buffered_input_stream_fill_finish(bstream, result, &error);
  if (nread < 0 && data->bytes_read == 0) g_simple_async_result_take_error(simple, error);
  else if (error) g_error_free(error);
  if (nread > 0) {
      available = priv->end - priv->pos;
      data->count = MIN(data->count, available);
      memcpy((char*)data->buffer + data->bytes_read,(char*)priv->buffer + priv->pos, data->count);
      data->bytes_read += data->count;
      priv->pos += data->count;
  }
  g_simple_async_result_complete(simple);
  g_object_unref(simple);
}
static void g_buffered_input_stream_read_async(GInputStream *stream, void *buffer, gsize count, int io_priority, GCancellable *cancellable,
                                               GAsyncReadyCallback callback, gpointer user_data) {
  GBufferedInputStream *bstream;
  GBufferedInputStreamPrivate *priv;
  GBufferedInputStreamClass *class;
  GInputStream *base_stream;
  gsize available;
  GSimpleAsyncResult *simple;
  ReadAsyncData *data;
  bstream = G_BUFFERED_INPUT_STREAM(stream);
  priv = bstream->priv;
  data = g_slice_new(ReadAsyncData);
  data->buffer = buffer;
  data->bytes_read = 0;
  simple = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_buffered_input_stream_read_async);
  g_simple_async_result_set_op_res_gpointer(simple, data, free_read_async_data);
  available = priv->end - priv->pos;
  if (count <= available) {
      memcpy(buffer, priv->buffer + priv->pos, count);
      priv->pos += count;
      data->bytes_read = count;
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  memcpy(buffer, priv->buffer + priv->pos, available);
  priv->pos = 0;
  priv->end = 0;
  count -= available;
  data->bytes_read = available;
  data->count = count;
  if (count > priv->len) {
      base_stream = G_FILTER_INPUT_STREAM(stream)->base_stream;
      g_input_stream_read_async(base_stream, (char*)buffer + data->bytes_read, count, io_priority, cancellable, large_read_callback, simple);
  } else {
      class = G_BUFFERED_INPUT_STREAM_GET_CLASS(stream);
      class->fill_async(bstream, priv->len, io_priority, cancellable, read_fill_buffer_callback, simple);
  }
}
static gssize g_buffered_input_stream_read_finish(GInputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  ReadAsyncData *data;
  simple = G_SIMPLE_ASYNC_RESULT(result);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_buffered_input_stream_read_async);
  data = g_simple_async_result_get_op_res_gpointer (simple);
  return data->bytes_read;
}
typedef struct {
  gssize bytes_skipped;
  gssize count;
} SkipAsyncData;
static void free_skip_async_data(gpointer _data) {
  SkipAsyncData *data = _data;
  g_slice_free(SkipAsyncData, data);
}
static void large_skip_callback(GObject *source_object, GAsyncResult *result, gpointer user_data) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(user_data);
  SkipAsyncData *data;
  GError *error;
  gssize nread;
  data = g_simple_async_result_get_op_res_gpointer(simple);
  error = NULL;
  nread = g_input_stream_skip_finish(G_INPUT_STREAM(source_object),result, &error);
  if (nread < 0 && data->bytes_skipped == 0) g_simple_async_result_take_error(simple, error);
  else if (error) g_error_free(error);
  if (nread > 0) data->bytes_skipped += nread;
  g_simple_async_result_complete(simple);
  g_object_unref(simple);
}
static void skip_fill_buffer_callback(GObject *source_object, GAsyncResult *result, gpointer user_data) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(user_data);
  GBufferedInputStream *bstream;
  GBufferedInputStreamPrivate *priv;
  SkipAsyncData *data;
  GError *error;
  gssize nread;
  gsize available;
  bstream = G_BUFFERED_INPUT_STREAM(source_object);
  priv = bstream->priv;
  data = g_simple_async_result_get_op_res_gpointer(simple);
  error = NULL;
  nread = g_buffered_input_stream_fill_finish(bstream, result, &error);
  if (nread < 0 && data->bytes_skipped == 0) g_simple_async_result_take_error(simple, error);
  else if (error) g_error_free(error);
  if (nread > 0) {
      available = priv->end - priv->pos;
      data->count = MIN(data->count, available);
      data->bytes_skipped += data->count;
      priv->pos += data->count;
  }
  g_simple_async_result_complete(simple);
  g_object_unref(simple);
}
static void g_buffered_input_stream_skip_async(GInputStream *stream, gsize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                               gpointer user_data) {
  GBufferedInputStream *bstream;
  GBufferedInputStreamPrivate *priv;
  GBufferedInputStreamClass *class;
  GInputStream *base_stream;
  gsize available;
  GSimpleAsyncResult *simple;
  SkipAsyncData *data;
  bstream = G_BUFFERED_INPUT_STREAM(stream);
  priv = bstream->priv;
  data = g_slice_new(SkipAsyncData);
  data->bytes_skipped = 0;
  simple = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_buffered_input_stream_skip_async);
  g_simple_async_result_set_op_res_gpointer(simple, data, free_skip_async_data);
  available = priv->end - priv->pos;
  if (count <= available) {
      priv->pos += count;
      data->bytes_skipped = count;
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  priv->pos = 0;
  priv->end = 0;
  count -= available;
  data->bytes_skipped = available;
  data->count = count;
  if (count > priv->len) {
      base_stream = G_FILTER_INPUT_STREAM(stream)->base_stream;
      g_input_stream_skip_async(base_stream, count, io_priority, cancellable, large_skip_callback, simple);
  } else {
      class = G_BUFFERED_INPUT_STREAM_GET_CLASS(stream);
      class->fill_async(bstream, priv->len, io_priority, cancellable, skip_fill_buffer_callback, simple);
  }
}
static gssize g_buffered_input_stream_skip_finish(GInputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  SkipAsyncData *data;
  simple = G_SIMPLE_ASYNC_RESULT(result);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_buffered_input_stream_skip_async);
  data = g_simple_async_result_get_op_res_gpointer(simple);
  return data->bytes_skipped;
}