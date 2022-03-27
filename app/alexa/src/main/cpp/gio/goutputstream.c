#include "../glib/glibintl.h"
#include "config.h"
#include "goutputstream.h"
#include "gcancellable.h"
#include "gasyncresult.h"
#include "gsimpleasyncresult.h"
#include "ginputstream.h"
#include "gioerror.h"

G_DEFINE_ABSTRACT_TYPE(GOutputStream, g_output_stream, G_TYPE_OBJECT);
struct _GOutputStreamPrivate {
  guint closed : 1;
  guint pending : 1;
  guint closing : 1;
  GAsyncReadyCallback outstanding_callback;
};
static gssize g_output_stream_real_splice(GOutputStream *stream, GInputStream *source, GOutputStreamSpliceFlags flags, GCancellable *cancellable,
						                  GError **error);
static void g_output_stream_real_write_async(GOutputStream *stream, const void *buffer, gsize count, int io_priority, GCancellable *cancellable,
						                     GAsyncReadyCallback callback, gpointer data);
static gssize g_output_stream_real_write_finish(GOutputStream *stream, GAsyncResult *result, GError **error);
static void g_output_stream_real_splice_async(GOutputStream *stream, GInputStream *source, GOutputStreamSpliceFlags flags, int io_priority, GCancellable *cancellable,
						                      GAsyncReadyCallback callback, gpointer data);
static gssize g_output_stream_real_splice_finish(GOutputStream *stream, GAsyncResult *result, GError **error);
static void g_output_stream_real_flush_async(GOutputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
static gboolean g_output_stream_real_flush_finish(GOutputStream *stream, GAsyncResult *result, GError **error);
static void g_output_stream_real_close_async(GOutputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
static gboolean g_output_stream_real_close_finish(GOutputStream *stream, GAsyncResult *result, GError **error);
static void g_output_stream_finalize(GObject *object) {
  G_OBJECT_CLASS(g_output_stream_parent_class)->finalize(object);
}
static void g_output_stream_dispose(GObject *object) {
  GOutputStream *stream;
  stream = G_OUTPUT_STREAM (object);
  if (!stream->priv->closed) g_output_stream_close(stream, NULL, NULL);
  G_OBJECT_CLASS(g_output_stream_parent_class)->dispose(object);
}
static void g_output_stream_class_init(GOutputStreamClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GOutputStreamPrivate));
  gobject_class->finalize = g_output_stream_finalize;
  gobject_class->dispose = g_output_stream_dispose;
  klass->splice = g_output_stream_real_splice;
  klass->write_async = g_output_stream_real_write_async;
  klass->write_finish = g_output_stream_real_write_finish;
  klass->splice_async = g_output_stream_real_splice_async;
  klass->splice_finish = g_output_stream_real_splice_finish;
  klass->flush_async = g_output_stream_real_flush_async;
  klass->flush_finish = g_output_stream_real_flush_finish;
  klass->close_async = g_output_stream_real_close_async;
  klass->close_finish = g_output_stream_real_close_finish;
}
static void g_output_stream_init(GOutputStream *stream) {
  stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(stream, G_TYPE_OUTPUT_STREAM, GOutputStreamPrivate);
}
gssize g_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error) {
  GOutputStreamClass *class;
  gssize res;
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), -1);
  g_return_val_if_fail(buffer != NULL, 0);
  if (count == 0) return 0;
  if (((gssize)count) < 0) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, _("Too large count value passed to %s"), G_STRFUNC);
      return -1;
  }
  class = G_OUTPUT_STREAM_GET_CLASS(stream);
  if (class->write_fn == NULL) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Output stream doesn't implement write"));
      return -1;
  }
  if (!g_output_stream_set_pending(stream, error)) return -1;
  if (cancellable) g_cancellable_push_current(cancellable);
  res = class->write_fn(stream, buffer, count, cancellable, error);
  if (cancellable) g_cancellable_pop_current(cancellable);
  g_output_stream_clear_pending(stream);
  return res; 
}
gboolean g_output_stream_write_all(GOutputStream *stream, const void *buffer, gsize count, gsize *bytes_written, GCancellable *cancellable, GError **error) {
  gsize _bytes_written;
  gssize res;
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), FALSE);
  g_return_val_if_fail(buffer != NULL, FALSE);
  _bytes_written = 0;
  while(_bytes_written < count) {
      res = g_output_stream_write(stream,(char*)buffer + _bytes_written, count - _bytes_written, cancellable, error);
      if (res == -1) {
          if (bytes_written) *bytes_written = _bytes_written;
          return FALSE;
	  }
      if (res == 0) g_warning("Write returned zero without error");
      _bytes_written += res;
  }
  if (bytes_written) *bytes_written = _bytes_written;
  return TRUE;
}
gboolean g_output_stream_flush(GOutputStream *stream, GCancellable *cancellable, GError **error) {
  GOutputStreamClass *class;
  gboolean res;
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), FALSE);
  if (!g_output_stream_set_pending(stream, error)) return FALSE;
  class = G_OUTPUT_STREAM_GET_CLASS(stream);
  res = TRUE;
  if (class->flush) {
      if (cancellable) g_cancellable_push_current(cancellable);
      res = class->flush(stream, cancellable, error);
      if (cancellable) g_cancellable_pop_current(cancellable);
  }
  g_output_stream_clear_pending(stream);
  return res;
}
gssize g_output_stream_splice(GOutputStream *stream, GInputStream *source, GOutputStreamSpliceFlags flags, GCancellable *cancellable, GError **error) {
  GOutputStreamClass *class;
  gssize bytes_copied;
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), -1);
  g_return_val_if_fail(G_IS_INPUT_STREAM(source), -1);
  if (g_input_stream_is_closed(source)) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_CLOSED, _("Source stream is already closed"));
      return -1;
  }
  if (!g_output_stream_set_pending(stream, error)) return -1;
  class = G_OUTPUT_STREAM_GET_CLASS(stream);
  if (cancellable) g_cancellable_push_current(cancellable);
  bytes_copied = class->splice(stream, source, flags, cancellable, error);
  if (cancellable) g_cancellable_pop_current(cancellable);
  g_output_stream_clear_pending(stream);
  return bytes_copied;
}
static gssize g_output_stream_real_splice(GOutputStream *stream, GInputStream *source, GOutputStreamSpliceFlags flags, GCancellable *cancellable, GError **error) {
  GOutputStreamClass *class = G_OUTPUT_STREAM_GET_CLASS(stream);
  gssize n_read, n_written;
  gssize bytes_copied;
  char buffer[8192], *p;
  gboolean res;
  bytes_copied = 0;
  if (class->write_fn == NULL) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Output stream doesn't implement write"));
      res = FALSE;
      goto notsupported;
  }
  res = TRUE;
  do {
      n_read = g_input_stream_read(source, buffer, sizeof(buffer), cancellable, error);
      if (n_read == -1) {
          res = FALSE;
          break;
	  }
      if (n_read == 0) break;
      p = buffer;
      while(n_read > 0) {
          n_written = class->write_fn(stream, p, n_read, cancellable, error);
          if (n_written == -1) {
              res = FALSE;
              break;
          }

          p += n_written;
          n_read -= n_written;
          bytes_copied += n_written;
	  }
  } while(res);
notsupported:
  if (!res) error = NULL;
  if (flags & G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE) g_input_stream_close (source, cancellable, NULL);
  if (flags & G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET) {
      if (class->close_fn && !class->close_fn(stream, cancellable, error)) res = FALSE;
  }
  if (res) return bytes_copied;
  return -1;
}
gboolean g_output_stream_close(GOutputStream *stream, GCancellable *cancellable, GError **error) {
  GOutputStreamClass *class;
  gboolean res;
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), FALSE);
  class = G_OUTPUT_STREAM_GET_CLASS(stream);
  if (stream->priv->closed) return TRUE;
  if (!g_output_stream_set_pending(stream, error)) return FALSE;
  stream->priv->closing = TRUE;
  if (cancellable) g_cancellable_push_current(cancellable);
  if (class->flush) res = class->flush(stream, cancellable, error);
  else res = TRUE;
  if (!res) {
      if (class->close_fn) class->close_fn(stream, cancellable, NULL);
  } else {
      res = TRUE;
      if (class->close_fn) res = class->close_fn(stream, cancellable, error);
  }
  if (cancellable) g_cancellable_pop_current(cancellable);
  stream->priv->closing = FALSE;
  stream->priv->closed = TRUE;
  g_output_stream_clear_pending(stream);
  return res;
}
static void async_ready_callback_wrapper(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GOutputStream *stream = G_OUTPUT_STREAM(source_object);
  g_output_stream_clear_pending(stream);
  if (stream->priv->outstanding_callback) (*stream->priv->outstanding_callback)(source_object, res, user_data);
  g_object_unref(stream);
}
typedef struct {
  gint io_priority;
  GCancellable *cancellable;
  GError *flush_error;
  gpointer user_data;
} CloseUserData;
static void async_ready_close_callback_wrapper(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GOutputStream *stream = G_OUTPUT_STREAM(source_object);
  CloseUserData *data = user_data;
  stream->priv->closing = FALSE;
  stream->priv->closed = TRUE;
  g_output_stream_clear_pending(stream);
  if (stream->priv->outstanding_callback) {
      if (data->flush_error != NULL) {
          GSimpleAsyncResult *err;
          err = g_simple_async_result_new_take_error(source_object, stream->priv->outstanding_callback, data->user_data, data->flush_error);
          data->flush_error = NULL;
          (*stream->priv->outstanding_callback)(source_object, G_ASYNC_RESULT(err), data->user_data);
          g_object_unref(err);
      } else (*stream->priv->outstanding_callback)(source_object, res, data->user_data);
  }
  g_object_unref(stream);
  if (data->cancellable) g_object_unref(data->cancellable);
  if (data->flush_error) g_error_free(data->flush_error);
  g_slice_free(CloseUserData, data);
}
static void async_ready_close_flushed_callback_wrapper(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GOutputStream *stream = G_OUTPUT_STREAM(source_object);
  GOutputStreamClass *class;
  CloseUserData *data = user_data;
  GSimpleAsyncResult *simple;
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      simple = G_SIMPLE_ASYNC_RESULT(res);
      g_simple_async_result_propagate_error(simple, &data->flush_error);
  }
  class = G_OUTPUT_STREAM_GET_CLASS(stream);
  class->close_async(stream, data->io_priority, data->cancellable, async_ready_close_callback_wrapper, user_data);
}
void g_output_stream_write_async(GOutputStream *stream, const void *buffer, gsize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
			                     gpointer user_data) {
  GOutputStreamClass *class;
  GSimpleAsyncResult *simple;
  GError *error = NULL;
  g_return_if_fail(G_IS_OUTPUT_STREAM(stream));
  g_return_if_fail(buffer != NULL);
  if (count == 0) {
      simple = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_output_stream_write_async);
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  if (((gssize)count) < 0) {
      g_simple_async_report_error_in_idle(G_OBJECT(stream), callback, user_data, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, _("Too large count "
                                          "value passed to %s"), G_STRFUNC);
      return;
  }
  if (!g_output_stream_set_pending(stream, &error)) {
      g_simple_async_report_take_gerror_in_idle(G_OBJECT (stream), callback, user_data, error);
      return;
  }
  class = G_OUTPUT_STREAM_GET_CLASS(stream);
  stream->priv->outstanding_callback = callback;
  g_object_ref(stream);
  class->write_async(stream, buffer, count, io_priority, cancellable, async_ready_callback_wrapper, user_data);
}
gssize g_output_stream_write_finish(GOutputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  GOutputStreamClass *class;
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), -1);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), -1);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return -1;
      if (g_simple_async_result_get_source_tag(simple) == g_output_stream_write_async) return 0;
  }
  class = G_OUTPUT_STREAM_GET_CLASS(stream);
  return class->write_finish(stream, result, error);
}
typedef struct {
  GInputStream *source;
  gpointer user_data;
  GAsyncReadyCallback callback;
} SpliceUserData;
static void async_ready_splice_callback_wrapper(GObject *source_object, GAsyncResult *res, gpointer _data) {
  GOutputStream *stream = G_OUTPUT_STREAM(source_object);
  SpliceUserData *data = _data;
  g_output_stream_clear_pending(stream);
  if (data->callback) (*data->callback)(source_object, res, data->user_data);
  g_object_unref(stream);
  g_object_unref(data->source);
  g_free(data);
}
void g_output_stream_splice_async(GOutputStream *stream, GInputStream *source, GOutputStreamSpliceFlags flags, int io_priority, GCancellable *cancellable,
			                      GAsyncReadyCallback callback, gpointer user_data) {
  GOutputStreamClass *class;
  SpliceUserData *data;
  GError *error = NULL;
  g_return_if_fail(G_IS_OUTPUT_STREAM(stream));
  g_return_if_fail(G_IS_INPUT_STREAM(source));
  if (g_input_stream_is_closed(source)) {
      g_simple_async_report_error_in_idle(G_OBJECT(stream), callback, user_data, G_IO_ERROR, G_IO_ERROR_CLOSED, _("Source stream is already closed"));
      return;
  }
  if (!g_output_stream_set_pending(stream, &error)) {
      g_simple_async_report_take_gerror_in_idle(G_OBJECT(stream), callback, user_data, error);
      return;
  }
  class = G_OUTPUT_STREAM_GET_CLASS(stream);
  data = g_new0(SpliceUserData, 1);
  data->callback = callback;
  data->user_data = user_data;
  data->source = g_object_ref(source);
  g_object_ref(stream);
  class->splice_async(stream, source, flags, io_priority, cancellable, async_ready_splice_callback_wrapper, data);
}
gssize g_output_stream_splice_finish(GOutputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  GOutputStreamClass *class;
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), -1);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), -1);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return -1;
  }
  class = G_OUTPUT_STREAM_GET_CLASS(stream);
  return class->splice_finish(stream, result, error);
}
void g_output_stream_flush_async(GOutputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GOutputStreamClass *class;
  GSimpleAsyncResult *simple;
  GError *error = NULL;
  g_return_if_fail(G_IS_OUTPUT_STREAM(stream));
  if (!g_output_stream_set_pending(stream, &error)) {
      g_simple_async_report_take_gerror_in_idle(G_OBJECT(stream), callback, user_data, error);
      return;
  }
  stream->priv->outstanding_callback = callback;
  g_object_ref(stream);
  class = G_OUTPUT_STREAM_GET_CLASS(stream);
  if (class->flush_async == NULL) {
      simple = g_simple_async_result_new(G_OBJECT (stream), async_ready_callback_wrapper, user_data, g_output_stream_flush_async);
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  class->flush_async(stream, io_priority, cancellable, async_ready_callback_wrapper, user_data);
}
gboolean g_output_stream_flush_finish(GOutputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  GOutputStreamClass *klass;
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
      if (g_simple_async_result_get_source_tag(simple) == g_output_stream_flush_async) return TRUE;
  }
  klass = G_OUTPUT_STREAM_GET_CLASS(stream);
  return klass->flush_finish(stream, result, error);
}
void g_output_stream_close_async(GOutputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GOutputStreamClass *class;
  GSimpleAsyncResult *simple;
  GError *error = NULL;
  CloseUserData *data;
  g_return_if_fail(G_IS_OUTPUT_STREAM(stream));
  if (stream->priv->closed) {
      simple = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_output_stream_close_async);
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  if (!g_output_stream_set_pending(stream, &error)) {
      g_simple_async_report_take_gerror_in_idle(G_OBJECT(stream), callback, user_data, error);
      return;
  }
  class = G_OUTPUT_STREAM_GET_CLASS(stream);
  stream->priv->closing = TRUE;
  stream->priv->outstanding_callback = callback;
  g_object_ref(stream);
  data = g_slice_new0(CloseUserData);
  if (cancellable != NULL) data->cancellable = g_object_ref(cancellable);
  data->io_priority = io_priority;
  data->user_data = user_data;
  if (class->flush_async == NULL || (class->flush_async == g_output_stream_real_flush_async && (class->flush == NULL ||
      class->close_async == g_output_stream_real_close_async))) {
      class->close_async(stream, io_priority, cancellable, async_ready_close_callback_wrapper, data);
  } else class->flush_async(stream, io_priority, cancellable, async_ready_close_flushed_callback_wrapper, data);
}
gboolean g_output_stream_close_finish(GOutputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  GOutputStreamClass *class;
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
      if (g_simple_async_result_get_source_tag(simple) == g_output_stream_close_async)
	  return TRUE;
  }
  class = G_OUTPUT_STREAM_GET_CLASS(stream);
  return class->close_finish(stream, result, error);
}
gboolean g_output_stream_is_closed(GOutputStream *stream) {
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), TRUE);
  return stream->priv->closed;
}
gboolean g_output_stream_is_closing(GOutputStream *stream) {
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), TRUE);
  return stream->priv->closing;
}
gboolean g_output_stream_has_pending(GOutputStream *stream) {
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), FALSE);
  return stream->priv->pending;
}
gboolean g_output_stream_set_pending(GOutputStream *stream, GError **error) {
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), FALSE);
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
void g_output_stream_clear_pending(GOutputStream *stream) {
  g_return_if_fail(G_IS_OUTPUT_STREAM(stream));
  stream->priv->pending = FALSE;
}
typedef struct {
  const void *buffer;
  gsize count_requested;
  gssize count_written;
} WriteData;
static void write_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  WriteData *op;
  GOutputStreamClass *class;
  GError *error = NULL;
  class = G_OUTPUT_STREAM_GET_CLASS(object);
  op = g_simple_async_result_get_op_res_gpointer(res);
  op->count_written = class->write_fn(G_OUTPUT_STREAM(object), op->buffer, op->count_requested, cancellable, &error);
  if (op->count_written == -1) g_simple_async_result_take_error(res, error);
}
static void g_output_stream_real_write_async(GOutputStream *stream, const void *buffer, gsize count, int io_priority, GCancellable *cancellable,
                                             GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  WriteData *op;
  op = g_new0(WriteData, 1);
  res = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_output_stream_real_write_async);
  g_simple_async_result_set_op_res_gpointer(res, op, g_free);
  op->buffer = buffer;
  op->count_requested = count;
  g_simple_async_result_run_in_thread(res, write_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static gssize g_output_stream_real_write_finish(GOutputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
  WriteData *op;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_output_stream_real_write_async);
  op = g_simple_async_result_get_op_res_gpointer(simple);
  return op->count_written;
}
typedef struct {
  GInputStream *source;
  GOutputStreamSpliceFlags flags;
  gssize bytes_copied;
} SpliceData;
static void splice_async_thread(GSimpleAsyncResult *result, GObject *object, GCancellable *cancellable) {
  SpliceData *op;
  GOutputStreamClass *class;
  GError *error = NULL;
  GOutputStream *stream;
  stream = G_OUTPUT_STREAM(object);
  class = G_OUTPUT_STREAM_GET_CLASS(object);
  op = g_simple_async_result_get_op_res_gpointer(result);
  op->bytes_copied = class->splice(stream, op->source, op->flags, cancellable, &error);
  if (op->bytes_copied == -1) g_simple_async_result_take_error(result, error);
}
static void g_output_stream_real_splice_async(GOutputStream *stream, GInputStream *source, GOutputStreamSpliceFlags flags, int io_priority, GCancellable *cancellable,
                                              GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  SpliceData *op;
  op = g_new0(SpliceData, 1);
  res = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_output_stream_real_splice_async);
  g_simple_async_result_set_op_res_gpointer(res, op, g_free);
  op->flags = flags;
  op->source = source;
  g_simple_async_result_run_in_thread(res, splice_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static gssize g_output_stream_real_splice_finish(GOutputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
  SpliceData *op;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_output_stream_real_splice_async);
  op = g_simple_async_result_get_op_res_gpointer(simple);
  return op->bytes_copied;
}
static void flush_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GOutputStreamClass *class;
  gboolean result;
  GError *error = NULL;
  class = G_OUTPUT_STREAM_GET_CLASS(object);
  result = TRUE;
  if (class->flush) result = class->flush(G_OUTPUT_STREAM(object), cancellable, &error);
  if (!result) g_simple_async_result_take_error(res, error);
}
static void g_output_stream_real_flush_async(GOutputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  res = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_output_stream_real_write_async);
  g_simple_async_result_run_in_thread(res, flush_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static gboolean g_output_stream_real_flush_finish(GOutputStream *stream, GAsyncResult *result, GError **error) {
  return TRUE;
}
static void close_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GOutputStreamClass *class;
  GError *error = NULL;
  gboolean result = TRUE;
  class = G_OUTPUT_STREAM_GET_CLASS(object);
  if (class->flush != NULL && (class->flush_async == NULL || class->flush_async == g_output_stream_real_flush_async)) {
      result = class->flush(G_OUTPUT_STREAM(object), cancellable, &error);
  }
  if (class->close_fn) {
      if (!result) class->close_fn(G_OUTPUT_STREAM(object), cancellable, NULL);
      else result = class->close_fn(G_OUTPUT_STREAM(object), cancellable, &error);
      if (!result) g_simple_async_result_take_error(res, error);
  }
}
static void g_output_stream_real_close_async(GOutputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  res = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_output_stream_real_close_async);
  g_simple_async_result_set_handle_cancellation(res, FALSE);
  g_simple_async_result_run_in_thread(res, close_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static gboolean g_output_stream_real_close_finish(GOutputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_output_stream_real_close_async);
  return TRUE;
}