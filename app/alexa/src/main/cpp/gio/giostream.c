#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "../glib/glib-object.h"
#include "../glib/gi18n.h"
#include "config.h"
#include "giostream.h"
#include "gsimpleasyncresult.h"
#include "gasyncresult.h"

G_DEFINE_ABSTRACT_TYPE(GIOStream, g_io_stream, G_TYPE_OBJECT);
enum {
  PROP_0,
  PROP_INPUT_STREAM,
  PROP_OUTPUT_STREAM,
  PROP_CLOSED
};
struct _GIOStreamPrivate {
  guint closed : 1;
  guint pending : 1;
  GAsyncReadyCallback outstanding_callback;
};
static gboolean g_io_stream_real_close(GIOStream *stream, GCancellable *cancellable, GError **error);
static void g_io_stream_real_close_async(GIOStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
static gboolean g_io_stream_real_close_finish(GIOStream *stream, GAsyncResult *result, GError **error);
static void g_io_stream_finalize(GObject *object) {
  G_OBJECT_CLASS(g_io_stream_parent_class)->finalize(object);
}
static void g_io_stream_dispose(GObject *object) {
  GIOStream *stream;
  stream = G_IO_STREAM(object);
  if (!stream->priv->closed) g_io_stream_close(stream, NULL, NULL);
  G_OBJECT_CLASS (g_io_stream_parent_class)->dispose(object);
}
static void g_io_stream_init(GIOStream *stream) {
  stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(stream, G_TYPE_IO_STREAM, GIOStreamPrivate);
}
static void g_io_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GIOStream *stream = G_IO_STREAM(object);
  switch(prop_id) {
      case PROP_CLOSED: g_value_set_boolean(value, stream->priv->closed); break;
      case PROP_INPUT_STREAM: g_value_set_object(value, g_io_stream_get_input_stream(stream)); break;
      case PROP_OUTPUT_STREAM: g_value_set_object(value, g_io_stream_get_output_stream(stream)); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_io_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  switch(prop_id) {
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_io_stream_class_init(GIOStreamClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GIOStreamPrivate));
  gobject_class->finalize = g_io_stream_finalize;
  gobject_class->dispose = g_io_stream_dispose;
  gobject_class->set_property = g_io_stream_set_property;
  gobject_class->get_property = g_io_stream_get_property;
  klass->close_fn = g_io_stream_real_close;
  klass->close_async = g_io_stream_real_close_async;
  klass->close_finish = g_io_stream_real_close_finish;
  g_object_class_install_property(gobject_class, PROP_CLOSED,g_param_spec_boolean("closed", P_("Closed"), P_("Is the stream "
                                  "closed"),FALSE,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_INPUT_STREAM, g_param_spec_object("input-stream", P_("Input stream"), P_("The GInputStream to "
                                  "read from"), G_TYPE_INPUT_STREAM, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_OUTPUT_STREAM, g_param_spec_object("output-stream", P_("Output stream"), P_("The GOutputStream "
                                  "to write to"), G_TYPE_OUTPUT_STREAM, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
}
gboolean g_io_stream_is_closed(GIOStream *stream) {
  g_return_val_if_fail(G_IS_IO_STREAM(stream), TRUE);
  return stream->priv->closed;
}
GInputStream *g_io_stream_get_input_stream(GIOStream *stream) {
  GIOStreamClass *klass = NULL;
  klass = G_IO_STREAM_GET_CLASS(stream);
  g_assert(klass->get_input_stream != NULL);
  return klass->get_input_stream(stream);
}
GOutputStream *g_io_stream_get_output_stream(GIOStream *stream) {
  GIOStreamClass *klass = NULL;
  klass = G_IO_STREAM_GET_CLASS(stream);
  g_assert(klass->get_output_stream != NULL);
  return klass->get_output_stream(stream);
}
gboolean g_io_stream_has_pending(GIOStream *stream) {
  g_return_val_if_fail(G_IS_IO_STREAM(stream), FALSE);
  return stream->priv->pending;
}
gboolean g_io_stream_set_pending(GIOStream  *stream, GError **error) {
  g_return_val_if_fail(G_IS_IO_STREAM(stream), FALSE);
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
void g_io_stream_clear_pending(GIOStream *stream) {
  g_return_if_fail(G_IS_IO_STREAM(stream));
  stream->priv->pending = FALSE;
}
static gboolean g_io_stream_real_close(GIOStream *stream, GCancellable *cancellable, GError **error) {
  gboolean res;
  res = g_output_stream_close(g_io_stream_get_output_stream(stream), cancellable, error);
  if (error != NULL && *error != NULL) error = NULL;
  res &= g_input_stream_close(g_io_stream_get_input_stream(stream), cancellable, error);
  return res;
}
gboolean g_io_stream_close(GIOStream *stream, GCancellable *cancellable, GError **error) {
  GIOStreamClass *class;
  gboolean res;
  g_return_val_if_fail(G_IS_IO_STREAM(stream), FALSE);
  class = G_IO_STREAM_GET_CLASS(stream);
  if (stream->priv->closed) return TRUE;
  if (!g_io_stream_set_pending(stream, error)) return FALSE;
  if (cancellable) g_cancellable_push_current(cancellable);
  res = TRUE;
  if (class->close_fn) res = class->close_fn(stream, cancellable, error);
  if (cancellable) g_cancellable_pop_current(cancellable);
  stream->priv->closed = TRUE;
  g_io_stream_clear_pending(stream);
  return res;
}
static void async_ready_close_callback_wrapper(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GIOStream *stream = G_IO_STREAM(source_object);
  stream->priv->closed = TRUE;
  g_io_stream_clear_pending(stream);
  if (stream->priv->outstanding_callback) (*stream->priv->outstanding_callback)(source_object, res, user_data);
  g_object_unref(stream);
}
void g_io_stream_close_async(GIOStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GIOStreamClass *class;
  GSimpleAsyncResult *simple;
  GError *error = NULL;
  g_return_if_fail(G_IS_IO_STREAM(stream));
  if (stream->priv->closed) {
      simple = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_io_stream_close_async);
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  if (!g_io_stream_set_pending(stream, &error)) {
      g_simple_async_report_take_gerror_in_idle(G_OBJECT(stream), callback, user_data, error);
      return;
  }
  class = G_IO_STREAM_GET_CLASS(stream);
  stream->priv->outstanding_callback = callback;
  g_object_ref(stream);
  class->close_async(stream, io_priority, cancellable, async_ready_close_callback_wrapper, user_data);
}
gboolean g_io_stream_close_finish(GIOStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  GIOStreamClass *class;
  g_return_val_if_fail(G_IS_IO_STREAM(stream), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
      if (g_simple_async_result_get_source_tag(simple) == g_io_stream_close_async) return TRUE;
  }
  class = G_IO_STREAM_GET_CLASS(stream);
  return class->close_finish(stream, result, error);
}
static void close_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GIOStreamClass *class;
  GError *error = NULL;
  gboolean result;
  class = G_IO_STREAM_GET_CLASS(object);
  if (class->close_fn) {
      result = class->close_fn(G_IO_STREAM(object), cancellable, &error);
      if (!result) g_simple_async_result_take_error(res, error);
  }
}
static void g_io_stream_real_close_async(GIOStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  res = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_io_stream_real_close_async);
  g_simple_async_result_set_handle_cancellation(res, FALSE);
  g_simple_async_result_run_in_thread(res, close_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static gboolean g_io_stream_real_close_finish(GIOStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_io_stream_real_close_async);
  return TRUE;
}
typedef struct {
  GIOStream *stream1;
  GIOStream *stream2;
  GIOStreamSpliceFlags flags;
  gint io_priority;
  GCancellable *cancellable;
  gulong cancelled_id;
  GCancellable *op1_cancellable;
  GCancellable *op2_cancellable;
  guint completed;
  GError *error;
} SpliceContext;
static void splice_context_free(SpliceContext *ctx) {
  g_object_unref(ctx->stream1);
  g_object_unref(ctx->stream2);
  if (ctx->cancellable != NULL) g_object_unref(ctx->cancellable);
  g_object_unref(ctx->op1_cancellable);
  g_object_unref(ctx->op2_cancellable);
  g_clear_error(&ctx->error);
  g_slice_free(SpliceContext, ctx);
}
static void splice_complete(GSimpleAsyncResult *simple, SpliceContext *ctx) {
  if (ctx->cancelled_id != 0) g_cancellable_disconnect(ctx->cancellable, ctx->cancelled_id);
  ctx->cancelled_id = 0;
  if (ctx->error != NULL) g_simple_async_result_set_from_error(simple, ctx->error);
  g_simple_async_result_complete(simple);
}
static void splice_close_cb(GObject *iostream, GAsyncResult *res, gpointer user_data) {
  GSimpleAsyncResult *simple = user_data;
  SpliceContext *ctx;
  GError *error = NULL;
  g_io_stream_close_finish(G_IO_STREAM(iostream), res, &error);
  ctx = g_simple_async_result_get_op_res_gpointer(simple);
  ctx->completed++;
  if (error != NULL && ctx->error == NULL) ctx->error = error;
  else g_clear_error(&error);
  if (ctx->completed == 4) splice_complete(simple, ctx);
  g_object_unref(simple);
}
static void splice_cb(GObject *ostream, GAsyncResult *res, gpointer user_data) {
  GSimpleAsyncResult *simple = user_data;
  SpliceContext *ctx;
  GError *error = NULL;
  g_output_stream_splice_finish(G_OUTPUT_STREAM(ostream), res, &error);
  ctx = g_simple_async_result_get_op_res_gpointer(simple);
  ctx->completed++;
  if (error != NULL && g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED) && (ctx->cancellable == NULL || !g_cancellable_is_cancelled(ctx->cancellable)))
      g_clear_error(&error);
  if (error != NULL && ctx->error == NULL) ctx->error = error;
  else g_clear_error(&error);
  if (ctx->completed == 1 && (ctx->flags & G_IO_STREAM_SPLICE_WAIT_FOR_BOTH) == 0) {
      g_cancellable_cancel(ctx->op1_cancellable);
      g_cancellable_cancel(ctx->op2_cancellable);
  } else if (ctx->completed == 2) {
      if (ctx->cancellable == NULL || !g_cancellable_is_cancelled(ctx->cancellable)) {
          g_cancellable_reset(ctx->op1_cancellable);
          g_cancellable_reset(ctx->op2_cancellable);
      }
      if ((ctx->flags & G_IO_STREAM_SPLICE_CLOSE_STREAM1) != 0) {
          g_io_stream_close_async(ctx->stream1, ctx->io_priority, ctx->op1_cancellable, splice_close_cb, g_object_ref(simple));
      } else ctx->completed++;
      if ((ctx->flags & G_IO_STREAM_SPLICE_CLOSE_STREAM2) != 0) {
          g_io_stream_close_async(ctx->stream2, ctx->io_priority, ctx->op2_cancellable, splice_close_cb, g_object_ref(simple));
      } else ctx->completed++;
      if (ctx->completed == 4) splice_complete(simple, ctx);
  }
  g_object_unref(simple);
}
static void splice_cancelled_cb(GCancellable *cancellable, GSimpleAsyncResult *simple) {
  SpliceContext *ctx;
  ctx = g_simple_async_result_get_op_res_gpointer(simple);
  g_cancellable_cancel(ctx->op1_cancellable);
  g_cancellable_cancel(ctx->op2_cancellable);
}
void g_io_stream_splice_async(GIOStream *stream1, GIOStream *stream2, GIOStreamSpliceFlags flags, gint io_priority, GCancellable *cancellable,
                              GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *simple;
  SpliceContext *ctx;
  GInputStream *istream;
  GOutputStream *ostream;
  if (cancellable != NULL && g_cancellable_is_cancelled(cancellable)) {
      g_simple_async_report_error_in_idle(NULL, callback, user_data, G_IO_ERROR, G_IO_ERROR_CANCELLED,"Operation has been cancelled");
      return;
  }
  ctx = g_slice_new0(SpliceContext);
  ctx->stream1 = g_object_ref(stream1);
  ctx->stream2 = g_object_ref(stream2);
  ctx->flags = flags;
  ctx->io_priority = io_priority;
  ctx->op1_cancellable = g_cancellable_new();
  ctx->op2_cancellable = g_cancellable_new();
  ctx->completed = 0;
  simple = g_simple_async_result_new(NULL, callback, user_data, g_io_stream_splice_finish);
  g_simple_async_result_set_op_res_gpointer(simple, ctx, (GDestroyNotify)splice_context_free);
  if (cancellable != NULL) {
      ctx->cancellable = g_object_ref(cancellable);
      ctx->cancelled_id = g_cancellable_connect(cancellable, G_CALLBACK(splice_cancelled_cb), g_object_ref(simple), g_object_unref);
  }
  istream = g_io_stream_get_input_stream(stream1);
  ostream = g_io_stream_get_output_stream(stream2);
  g_output_stream_splice_async(ostream, istream, G_OUTPUT_STREAM_SPLICE_NONE, io_priority, ctx->op1_cancellable, splice_cb, g_object_ref (simple));
  istream = g_io_stream_get_input_stream(stream2);
  ostream = g_io_stream_get_output_stream(stream1);
  g_output_stream_splice_async(ostream, istream, G_OUTPUT_STREAM_SPLICE_NONE, io_priority, ctx->op2_cancellable, splice_cb, g_object_ref (simple));
  g_object_unref(simple);
}
gboolean g_io_stream_splice_finish(GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  g_return_val_if_fail(G_IS_SIMPLE_ASYNC_RESULT(result), FALSE);
  simple = G_SIMPLE_ASYNC_RESULT(result);
  if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  g_return_val_if_fail(g_simple_async_result_is_valid(result, NULL, g_io_stream_splice_finish), FALSE);
  return TRUE;
}