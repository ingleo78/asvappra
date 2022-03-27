#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "ginputstream.h"
#include "gseekable.h"
#include "gcancellable.h"
#include "gasyncresult.h"
#include "gsimpleasyncresult.h"
#include "gioerror.h"

G_DEFINE_ABSTRACT_TYPE(GInputStream, g_input_stream, G_TYPE_OBJECT);
struct _GInputStreamPrivate {
  guint closed : 1;
  guint pending : 1;
  GAsyncReadyCallback outstanding_callback;
};
static gssize g_input_stream_real_skip(GInputStream *stream, gsize count, GCancellable *cancellable, GError **error);
static void g_input_stream_real_read_async(GInputStream  *stream, void *buffer, gsize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
						                   gpointer user_data);
static gssize g_input_stream_real_read_finish(GInputStream *stream, GAsyncResult *result, GError **error);
static void g_input_stream_real_skip_async(GInputStream *stream, gsize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
static gssize g_input_stream_real_skip_finish(GInputStream *stream, GAsyncResult *result, GError **error);
static void g_input_stream_real_close_async(GInputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
static gboolean g_input_stream_real_close_finish(GInputStream *stream, GAsyncResult *result, GError **error);
static void g_input_stream_finalize(GObject *object) {
  G_OBJECT_CLASS(g_input_stream_parent_class)->finalize(object);
}
static void g_input_stream_dispose(GObject *object) {
  GInputStream *stream;
  stream = G_INPUT_STREAM(object);
  if (!stream->priv->closed) g_input_stream_close(stream, NULL, NULL);
  G_OBJECT_CLASS(g_input_stream_parent_class)->dispose(object);
}
static void g_input_stream_class_init(GInputStreamClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GInputStreamPrivate));
  gobject_class->finalize = g_input_stream_finalize;
  gobject_class->dispose = g_input_stream_dispose;
  klass->skip = g_input_stream_real_skip;
  klass->read_async = g_input_stream_real_read_async;
  klass->read_finish = g_input_stream_real_read_finish;
  klass->skip_async = g_input_stream_real_skip_async;
  klass->skip_finish = g_input_stream_real_skip_finish;
  klass->close_async = g_input_stream_real_close_async;
  klass->close_finish = g_input_stream_real_close_finish;
}
static void g_input_stream_init(GInputStream *stream) {
  stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(stream, G_TYPE_INPUT_STREAM, GInputStreamPrivate);
}
gssize g_input_stream_read(GInputStream *stream, void *buffer, gsize count, GCancellable *cancellable, GError **error) {
  GInputStreamClass *class;
  gssize res;
  g_return_val_if_fail(G_IS_INPUT_STREAM(stream), -1);
  g_return_val_if_fail(buffer != NULL, 0);
  if (count == 0) return 0;
  if (((gssize)count) < 0) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, _("Too large count value passed to %s"), G_STRFUNC);
      return -1;
  }
  class = G_INPUT_STREAM_GET_CLASS(stream);
  if (class->read_fn == NULL) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Input stream doesn't implement read"));
      return -1;
  }
  if (!g_input_stream_set_pending(stream, error)) return -1;
  if (cancellable) g_cancellable_push_current(cancellable);
  res = class->read_fn(stream, buffer, count, cancellable, error);
  if (cancellable) g_cancellable_pop_current(cancellable);
  g_input_stream_clear_pending(stream);
  return res;
}
gboolean g_input_stream_read_all(GInputStream *stream, void *buffer, gsize  count, gsize *bytes_read, GCancellable *cancellable, GError **error) {
  gsize _bytes_read;
  gssize res;
  g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);
  g_return_val_if_fail(buffer != NULL, FALSE);
  _bytes_read = 0;
  while (_bytes_read < count) {
      res = g_input_stream_read(stream,(char*)buffer + _bytes_read,count - _bytes_read, cancellable, error);
      if (res == -1) {
          if (bytes_read) *bytes_read = _bytes_read;
          return FALSE;
	  }
      if (res == 0) break;
      _bytes_read += res;
  }
  if (bytes_read) *bytes_read = _bytes_read;
  return TRUE;
}
gssize g_input_stream_skip(GInputStream *stream, gsize count, GCancellable *cancellable, GError **error) {
  GInputStreamClass *class;
  gssize res;
  g_return_val_if_fail(G_IS_INPUT_STREAM(stream), -1);
  if (count == 0) return 0;
  if (((gssize)count) < 0) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, _("Too large count value passed to %s"), G_STRFUNC);
      return -1;
  }
  class = G_INPUT_STREAM_GET_CLASS(stream);
  if (!g_input_stream_set_pending(stream, error)) return -1;
  if (cancellable) g_cancellable_push_current(cancellable);
  res = class->skip(stream, count, cancellable, error);
  if (cancellable) g_cancellable_pop_current(cancellable);
  g_input_stream_clear_pending(stream);
  return res;
}
static gssize g_input_stream_real_skip(GInputStream *stream, gsize count, GCancellable *cancellable, GError **error) {
  GInputStreamClass *class;
  gssize ret, read_bytes;
  char buffer[8192];
  GError *my_error;
  if (G_IS_SEEKABLE(stream) && g_seekable_can_seek(G_SEEKABLE(stream))) {
      if (g_seekable_seek(G_SEEKABLE(stream), count,G_SEEK_CUR, cancellable,NULL)) return count;
  }
  class = G_INPUT_STREAM_GET_CLASS(stream);
  read_bytes = 0;
  while(1) {
      my_error = NULL;
      ret = class->read_fn(stream, buffer, MIN(sizeof(buffer), count), cancellable, &my_error);
      if (ret == -1) {
          if (read_bytes > 0 && my_error->domain == G_IO_ERROR && my_error->code == G_IO_ERROR_CANCELLED) {
              g_error_free(my_error);
              return read_bytes;
          }
          g_propagate_error(error, my_error);
          return -1;
	  }
      count -= ret;
      read_bytes += ret;
      if (ret == 0 || count == 0) return read_bytes;
  }
}
gboolean g_input_stream_close(GInputStream *stream, GCancellable *cancellable, GError **error) {
  GInputStreamClass *class;
  gboolean res;
  g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);
  class = G_INPUT_STREAM_GET_CLASS(stream);
  if (stream->priv->closed) return TRUE;
  res = TRUE;
  if (!g_input_stream_set_pending(stream, error)) return FALSE;
  if (cancellable) g_cancellable_push_current(cancellable);
  if (class->close_fn) res = class->close_fn(stream, cancellable, error);
  if (cancellable) g_cancellable_pop_current(cancellable);
  g_input_stream_clear_pending(stream);
  stream->priv->closed = TRUE;
  return res;
}
static void async_ready_callback_wrapper(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GInputStream *stream = G_INPUT_STREAM(source_object);
  g_input_stream_clear_pending(stream);
  if (stream->priv->outstanding_callback)(*stream->priv->outstanding_callback)(source_object, res, user_data);
  g_object_unref(stream);
}
static void async_ready_close_callback_wrapper(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GInputStream *stream = G_INPUT_STREAM(source_object);
  g_input_stream_clear_pending(stream);
  stream->priv->closed = TRUE;
  if (stream->priv->outstanding_callback) (*stream->priv->outstanding_callback)(source_object, res, user_data);
  g_object_unref(stream);
}
void g_input_stream_read_async(GInputStream *stream, void *buffer, gsize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                               gpointer user_data) {
  GInputStreamClass *class;
  GSimpleAsyncResult *simple;
  GError *error = NULL;
  g_return_if_fail(G_IS_INPUT_STREAM(stream));
  g_return_if_fail(buffer != NULL);
  if (count == 0) {
      simple = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_input_stream_read_async);
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  if (((gssize) count) < 0) {
      g_simple_async_report_error_in_idle(G_OBJECT(stream), callback, user_data, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, _("Too large count "
                                          "value passed to %s"), G_STRFUNC);
      return;
  }
  if (!g_input_stream_set_pending(stream, &error)) {
      g_simple_async_report_take_gerror_in_idle(G_OBJECT(stream), callback, user_data, error);
      return;
  }
  class = G_INPUT_STREAM_GET_CLASS(stream);
  stream->priv->outstanding_callback = callback;
  g_object_ref(stream);
  class->read_async(stream, buffer, count, io_priority, cancellable, async_ready_callback_wrapper, user_data);
}
gssize g_input_stream_read_finish(GInputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  GInputStreamClass *class;
  g_return_val_if_fail(G_IS_INPUT_STREAM(stream), -1);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), -1);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return -1;
      if (g_simple_async_result_get_source_tag(simple) == g_input_stream_read_async) return 0;
  }
  class = G_INPUT_STREAM_GET_CLASS(stream);
  return class->read_finish(stream, result, error);
}
void g_input_stream_skip_async(GInputStream *stream, gsize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GInputStreamClass *class;
  GSimpleAsyncResult *simple;
  GError *error = NULL;
  g_return_if_fail(G_IS_INPUT_STREAM(stream));
  if (count == 0) {
      simple = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_input_stream_skip_async);
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  if (((gssize)count) < 0) {
      g_simple_async_report_error_in_idle(G_OBJECT(stream), callback, user_data, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, _("Too large count "
                                          "value passed to %s"), G_STRFUNC);
      return;
  }
  if (!g_input_stream_set_pending(stream, &error)) {
      g_simple_async_report_take_gerror_in_idle(G_OBJECT(stream), callback, user_data, error);
      return;
  }
  class = G_INPUT_STREAM_GET_CLASS(stream);
  stream->priv->outstanding_callback = callback;
  g_object_ref(stream);
  class->skip_async(stream, count, io_priority, cancellable, async_ready_callback_wrapper, user_data);
}
gssize g_input_stream_skip_finish(GInputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  GInputStreamClass *class;
  g_return_val_if_fail(G_IS_INPUT_STREAM(stream), -1);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), -1);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return -1;
      if (g_simple_async_result_get_source_tag(simple) == g_input_stream_skip_async) return 0;
  }
  class = G_INPUT_STREAM_GET_CLASS(stream);
  return class->skip_finish(stream, result, error);
}
void g_input_stream_close_async(GInputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GInputStreamClass *class;
  GSimpleAsyncResult *simple;
  GError *error = NULL;
  g_return_if_fail(G_IS_INPUT_STREAM(stream));
  if (stream->priv->closed) {
      simple = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_input_stream_close_async);
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  if (!g_input_stream_set_pending(stream, &error)){
      g_simple_async_report_take_gerror_in_idle(G_OBJECT(stream), callback, user_data, error);
      return;
  }
  class = G_INPUT_STREAM_GET_CLASS(stream);
  stream->priv->outstanding_callback = callback;
  g_object_ref(stream);
  class->close_async(stream, io_priority, cancellable, async_ready_close_callback_wrapper, user_data);
}
gboolean g_input_stream_close_finish(GInputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  GInputStreamClass *class;
  g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT (result)) {
      simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
      if (g_simple_async_result_get_source_tag(simple) == g_input_stream_close_async) return TRUE;
  }
  class = G_INPUT_STREAM_GET_CLASS(stream);
  return class->close_finish(stream, result, error);
}
gboolean g_input_stream_is_closed(GInputStream *stream) {
  g_return_val_if_fail(G_IS_INPUT_STREAM(stream), TRUE);
  return stream->priv->closed;
}
gboolean g_input_stream_has_pending(GInputStream *stream) {
  g_return_val_if_fail(G_IS_INPUT_STREAM(stream), TRUE);
  return stream->priv->pending;
}
gboolean g_input_stream_set_pending(GInputStream *stream, GError **error) {
  g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);
  if (stream->priv->closed) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_CLOSED, _("Stream is already closed"));
      return FALSE;
  }
  if (stream->priv->pending) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_PENDING, _("Stream has outstanding operation"));
      return FALSE;
  }
  stream->priv->pending = TRUE;
  return TRUE;
}
void g_input_stream_clear_pending(GInputStream *stream) {
  g_return_if_fail(G_IS_INPUT_STREAM(stream));
  stream->priv->pending = FALSE;
}
typedef struct {
  void *buffer;
  gsize count_requested;
  gssize count_read;
} ReadData;
static void read_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  ReadData *op;
  GInputStreamClass *class;
  GError *error = NULL;
  op = g_simple_async_result_get_op_res_gpointer(res);
  class = G_INPUT_STREAM_GET_CLASS(object);
  op->count_read = class->read_fn(G_INPUT_STREAM(object), op->buffer, op->count_requested, cancellable, &error);
  if (op->count_read == -1) g_simple_async_result_take_error(res, error);
}
static void g_input_stream_real_read_async(GInputStream *stream, void *buffer, gsize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
				                           gpointer user_data) {
  GSimpleAsyncResult *res;
  ReadData *op;
  op = g_new(ReadData, 1);
  res = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_input_stream_real_read_async);
  g_simple_async_result_set_op_res_gpointer(res, op, g_free);
  op->buffer = buffer;
  op->count_requested = count;
  g_simple_async_result_run_in_thread(res, read_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static gssize g_input_stream_real_read_finish(GInputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
  ReadData *op;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_input_stream_real_read_async);
  op = g_simple_async_result_get_op_res_gpointer(simple);
  return op->count_read;
}
typedef struct {
  gsize count_requested;
  gssize count_skipped;
} SkipData;
static void skip_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  SkipData *op;
  GInputStreamClass *class;
  GError *error = NULL;
  class = G_INPUT_STREAM_GET_CLASS(object);
  op = g_simple_async_result_get_op_res_gpointer(res);
  op->count_skipped = class->skip(G_INPUT_STREAM(object), op->count_requested, cancellable, &error);
  if (op->count_skipped == -1) g_simple_async_result_take_error(res, error);
}
typedef struct {
  char buffer[8192];
  gsize count;
  gsize count_skipped;
  int io_prio;
  GCancellable *cancellable;
  gpointer user_data;
  GAsyncReadyCallback callback;
} SkipFallbackAsyncData;
static void skip_callback_wrapper(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GInputStreamClass *class;
  SkipFallbackAsyncData *data = user_data;
  SkipData *op;
  GSimpleAsyncResult *simple;
  GError *error = NULL;
  gssize ret;
  ret = g_input_stream_read_finish(G_INPUT_STREAM(source_object), res, &error);
  if (ret > 0) {
      data->count -= ret;
      data->count_skipped += ret;
      if (data->count > 0) {
          class = G_INPUT_STREAM_GET_CLASS(source_object);
          class->read_async(G_INPUT_STREAM(source_object), data->buffer, MIN(8192, data->count), data->io_prio, data->cancellable, skip_callback_wrapper, data);
          return;
	  }
  }
  op = g_new0(SkipData, 1);
  op->count_skipped = data->count_skipped;
  simple = g_simple_async_result_new(source_object, data->callback, data->user_data, g_input_stream_real_skip_async);
  g_simple_async_result_set_op_res_gpointer(simple, op, g_free);
  if (ret == -1) {
      if (data->count_skipped && g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) g_error_free(error);
      else g_simple_async_result_take_error(simple, error);
  }
  g_simple_async_result_complete(simple);
  g_object_unref(simple);
  g_free(data);
}
static void g_input_stream_real_skip_async(GInputStream *stream, gsize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
				                           gpointer user_data) {
  GInputStreamClass *class;
  SkipData *op;
  SkipFallbackAsyncData *data;
  GSimpleAsyncResult *res;
  class = G_INPUT_STREAM_GET_CLASS(stream);
  if (class->read_async == g_input_stream_real_read_async) {
      op = g_new0(SkipData, 1);
      res = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_input_stream_real_skip_async);
      g_simple_async_result_set_op_res_gpointer(res, op, g_free);
      op->count_requested = count;
      g_simple_async_result_run_in_thread(res, skip_async_thread, io_priority, cancellable);
      g_object_unref(res);
  } else {
      data = g_new(SkipFallbackAsyncData, 1);
      data->count = count;
      data->count_skipped = 0;
      data->io_prio = io_priority;
      data->cancellable = cancellable;
      data->callback = callback;
      data->user_data = user_data;
      class->read_async(stream, data->buffer, MIN(8192, count), io_priority, cancellable, skip_callback_wrapper, data);
  }
}
static gssize g_input_stream_real_skip_finish(GInputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
  SkipData *op;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_input_stream_real_skip_async);
  op = g_simple_async_result_get_op_res_gpointer(simple);
  return op->count_skipped;
}
static void close_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GInputStreamClass *class;
  GError *error = NULL;
  gboolean result;
  class = G_INPUT_STREAM_GET_CLASS(object);
  if (class->close_fn) {
      result = class->close_fn(G_INPUT_STREAM(object), cancellable, &error);
      if (!result) g_simple_async_result_take_error(res, error);
  }
}
static void g_input_stream_real_close_async(GInputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  res = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_input_stream_real_close_async);
  g_simple_async_result_set_handle_cancellation(res, FALSE);
  g_simple_async_result_run_in_thread(res, close_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static gboolean g_input_stream_real_close_finish(GInputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_input_stream_real_close_async);
  return TRUE;
}