#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "../glib/glib-object.h"
#include "config.h"
#include "gmemoryoutputstream.h"
#include "goutputstream.h"
#include "gseekable.h"
#include "gsimpleasyncresult.h"
#include "gioerror.h"
#include "string.h"

#define MIN_ARRAY_SIZE  16
enum {
  PROP_0,
  PROP_DATA,
  PROP_SIZE,
  PROP_DATA_SIZE,
  PROP_REALLOC_FUNCTION,
  PROP_DESTROY_FUNCTION
};
struct _GMemoryOutputStreamPrivate {
  gpointer data;
  gsize len;
  gsize valid_len;
  gsize pos;
  GReallocFunc realloc_fn;
  GDestroyNotify destroy;
};
static void g_memory_output_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void g_memory_output_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void g_memory_output_stream_finalize(GObject *object);
static gssize g_memory_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error);
static gboolean g_memory_output_stream_close(GOutputStream *stream, GCancellable *cancellable, GError **error);
static void g_memory_output_stream_write_async(GOutputStream *stream, const void *buffer, gsize count, int io_priority, GCancellable *cancellable,
                                               GAsyncReadyCallback callback, gpointer data);
static gssize g_memory_output_stream_write_finish(GOutputStream *stream, GAsyncResult *result, GError **error);
static void g_memory_output_stream_close_async(GOutputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
static gboolean g_memory_output_stream_close_finish(GOutputStream *stream, GAsyncResult *result, GError **error);
static void g_memory_output_stream_seekable_iface_init(GSeekableIface *iface);
static goffset g_memory_output_stream_tell(GSeekable *seekable);
static gboolean g_memory_output_stream_can_seek(GSeekable *seekable);
static gboolean g_memory_output_stream_seek(GSeekable *seekable, goffset offset, GSeekType type, GCancellable *cancellable, GError **error);
static gboolean g_memory_output_stream_can_truncate(GSeekable *seekable);
static gboolean g_memory_output_stream_truncate(GSeekable *seekable, goffset offset, GCancellable *cancellable, GError **error);
G_DEFINE_TYPE_WITH_CODE(GMemoryOutputStream, g_memory_output_stream, G_TYPE_OUTPUT_STREAM,G_IMPLEMENT_INTERFACE(G_TYPE_SEEKABLE, g_memory_output_stream_seekable_iface_init));
static void g_memory_output_stream_class_init(GMemoryOutputStreamClass *klass) {
  GOutputStreamClass *ostream_class;
  GObjectClass *gobject_class;
  g_type_class_add_private(klass, sizeof(GMemoryOutputStreamPrivate));
  gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->set_property = g_memory_output_stream_set_property;
  gobject_class->get_property = g_memory_output_stream_get_property;
  gobject_class->finalize = g_memory_output_stream_finalize;
  ostream_class = G_OUTPUT_STREAM_CLASS (klass);
  ostream_class->write_fn = g_memory_output_stream_write;
  ostream_class->close_fn = g_memory_output_stream_close;
  ostream_class->write_async  = g_memory_output_stream_write_async;
  ostream_class->write_finish = g_memory_output_stream_write_finish;
  ostream_class->close_async  = g_memory_output_stream_close_async;
  ostream_class->close_finish = g_memory_output_stream_close_finish;
  g_object_class_install_property(gobject_class,PROP_DATA,g_param_spec_pointer("data", P_("Data Buffer"), P_("Pointer to buffer"
                                  " where data will be written."),G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class,PROP_SIZE,g_param_spec_ulong("size", P_("Data Buffer Size"), P_("Current size "
                                  "of the data buffer."),0,G_MAXULONG,0,G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                                  G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class,PROP_DATA_SIZE,g_param_spec_ulong("data-size", P_("Data Size"), P_("Size of "
                                  "data written to the buffer."),0,G_MAXULONG,0,G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class,PROP_REALLOC_FUNCTION,g_param_spec_pointer("realloc-function", P_("Memory Reallocation"
                                  " Function"), P_("Function with realloc semantics called to enlarge the buffer."),G_PARAM_READWRITE |
                                  G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class,PROP_DESTROY_FUNCTION,g_param_spec_pointer ("destroy-function", P_("Destroy Notification"
                                  " Function"), P_("Function called with the buffer as argument when the stream is destroyed."),G_PARAM_READWRITE |
                                  G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
}
static void g_memory_output_stream_set_property(GObject *object, guint prop_id, const GValue *value,GParamSpec *pspec) {
  GMemoryOutputStream        *stream;
  GMemoryOutputStreamPrivate *priv;
  stream = G_MEMORY_OUTPUT_STREAM(object);
  priv = stream->priv;
  switch(prop_id) {
      case PROP_DATA: priv->data = g_value_get_pointer(value); break;
      case PROP_SIZE: priv->len = g_value_get_ulong(value); break;
      case PROP_REALLOC_FUNCTION: priv->realloc_fn = g_value_get_pointer(value); break;
      case PROP_DESTROY_FUNCTION: priv->destroy = g_value_get_pointer(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_memory_output_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GMemoryOutputStream *stream;
  GMemoryOutputStreamPrivate *priv;
  stream = G_MEMORY_OUTPUT_STREAM(object);
  priv = stream->priv;
  switch(prop_id) {
      case PROP_DATA: g_value_set_pointer(value, priv->data); break;
      case PROP_SIZE: g_value_set_ulong(value, priv->len); break;
      case PROP_DATA_SIZE: g_value_set_ulong(value, priv->valid_len); break;
      case PROP_REALLOC_FUNCTION: g_value_set_pointer(value, priv->realloc_fn); break;
      case PROP_DESTROY_FUNCTION: g_value_set_pointer(value, priv->destroy); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_memory_output_stream_finalize(GObject *object) {
  GMemoryOutputStream *stream;
  GMemoryOutputStreamPrivate *priv;
  stream = G_MEMORY_OUTPUT_STREAM(object);
  priv = stream->priv;
  if (priv->destroy) priv->destroy(priv->data);
  //G_OBJECT_CLASS(g_memory_output_stream_parent_class)->finalize(object);
}
static void g_memory_output_stream_seekable_iface_init(GSeekableIface *iface) {
  iface->tell         = g_memory_output_stream_tell;
  iface->can_seek     = g_memory_output_stream_can_seek;
  iface->seek         = g_memory_output_stream_seek;
  iface->can_truncate = g_memory_output_stream_can_truncate;
  iface->truncate_fn  = g_memory_output_stream_truncate;
}
static void g_memory_output_stream_init(GMemoryOutputStream *stream) {
  stream->priv = G_TYPE_INSTANCE_GET_PRIVATE (stream, G_TYPE_MEMORY_OUTPUT_STREAM ,GMemoryOutputStreamPrivate);
  stream->priv->pos = 0;
  stream->priv->valid_len = 0;
}
GOutputStream *g_memory_output_stream_new(gpointer data, gsize size, GReallocFunc realloc_function, GDestroyNotify destroy_function) {
  GOutputStream *stream;
  stream = g_object_new(G_TYPE_MEMORY_OUTPUT_STREAM,"data", data, "size", size, "realloc-function", realloc_function, "destroy-function",
                        destroy_function, NULL);
  return stream;
}
gpointer g_memory_output_stream_get_data(GMemoryOutputStream *ostream) {
  g_return_val_if_fail(G_IS_MEMORY_OUTPUT_STREAM(ostream), NULL);
  return ostream->priv->data;
}
gsize g_memory_output_stream_get_size(GMemoryOutputStream *ostream) {
  g_return_val_if_fail(G_IS_MEMORY_OUTPUT_STREAM(ostream), 0);
  return ostream->priv->len;
}
gsize g_memory_output_stream_get_data_size(GMemoryOutputStream *ostream) {
  g_return_val_if_fail(G_IS_MEMORY_OUTPUT_STREAM(ostream), 0);
  return ostream->priv->valid_len;
}
gpointer g_memory_output_stream_steal_data(GMemoryOutputStream *ostream) {
  gpointer data;
  g_return_val_if_fail(G_IS_MEMORY_OUTPUT_STREAM(ostream), NULL);
  g_return_val_if_fail(g_output_stream_is_closed(G_OUTPUT_STREAM(ostream)), NULL);
  data = ostream->priv->data;
  ostream->priv->data = NULL;
  return data;
}
static gboolean array_resize(GMemoryOutputStream *ostream, gsize size, gboolean allow_partial, GError **error) {
  GMemoryOutputStreamPrivate *priv;
  gpointer data;
  gsize len;
  priv = ostream->priv;
  if (priv->len == size) return TRUE;
  if (!priv->realloc_fn) {
      if (allow_partial && priv->pos < priv->len) return TRUE;
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NO_SPACE, _("Memory output stream not resizable"));
      return FALSE;
  }
  len = priv->len;
  data = priv->realloc_fn(priv->data, size);
  if (size > 0 && !data) {
      if (allow_partial && priv->pos < priv->len) return TRUE;
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NO_SPACE, _("Failed to resize memory output stream"));
      return FALSE;
  }
  if (size > len) memset((guint8 *)data + len, 0, size - len);
  priv->data = data;
  priv->len = size;
  if (priv->len < priv->valid_len) priv->valid_len = priv->len;
  return TRUE;
}
static gint g_nearest_pow(gint num) {
  gint n = 1;
  while(n < num) n <<= 1;
  return n;
}
static gssize g_memory_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error) {
  GMemoryOutputStream *ostream;
  GMemoryOutputStreamPrivate *priv;
  guint8 *dest;
  gsize new_size;
  ostream = G_MEMORY_OUTPUT_STREAM(stream);
  priv = ostream->priv;
  if (count == 0) return 0;
  if (priv->realloc_fn && priv->pos + count < priv->pos) goto overflow;
  if (priv->pos + count > priv->len) {
      new_size = g_nearest_pow(priv->pos + count);
      if (new_size < priv->len) goto overflow;
      new_size = MAX(new_size, MIN_ARRAY_SIZE);
      if (!array_resize(ostream, new_size, TRUE, error)) return -1;
  }
  count = MIN(count, priv->len - priv->pos);
  dest = (guint8*)priv->data + priv->pos;
  memcpy(dest, buffer, count);
  priv->pos += count;
  if (priv->pos > priv->valid_len) priv->valid_len = priv->pos;
  return count;
overflow:
  g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NO_SPACE, _("Amount of memory required to process the write is larger than available address space"));
  return -1;
}
static gboolean g_memory_output_stream_close(GOutputStream *stream, GCancellable *cancellable, GError **error) {
  return TRUE;
}
static void g_memory_output_stream_write_async(GOutputStream *stream, const void *buffer, gsize count, int io_priority, GCancellable *cancellable,
                                               GAsyncReadyCallback callback, gpointer data) {
  GSimpleAsyncResult *simple;
  gssize nwritten;
  nwritten = g_memory_output_stream_write(stream, buffer, count, cancellable,NULL);
  simple = g_simple_async_result_new(G_OBJECT(stream), callback, data, g_memory_output_stream_write_async);
  g_simple_async_result_set_op_res_gssize(simple, nwritten);
  g_simple_async_result_complete_in_idle(simple);
  g_object_unref(simple);
}
static gssize g_memory_output_stream_write_finish(GOutputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  gssize nwritten;
  simple = G_SIMPLE_ASYNC_RESULT(result);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_memory_output_stream_write_async);
  nwritten = g_simple_async_result_get_op_res_gssize(simple);
  return nwritten;
}
static void g_memory_output_stream_close_async(GOutputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data) {
  GSimpleAsyncResult *simple;
  simple = g_simple_async_result_new(G_OBJECT(stream), callback, data, g_memory_output_stream_close_async);
  g_memory_output_stream_close(stream, cancellable, NULL);
  g_simple_async_result_complete_in_idle(simple);
  g_object_unref(simple);
}
static gboolean g_memory_output_stream_close_finish(GOutputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  simple = G_SIMPLE_ASYNC_RESULT(result);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_memory_output_stream_close_async);
  return TRUE;
}
static goffset g_memory_output_stream_tell(GSeekable *seekable) {
  GMemoryOutputStream *stream;
  GMemoryOutputStreamPrivate *priv;
  stream = G_MEMORY_OUTPUT_STREAM(seekable);
  priv = stream->priv;
  return priv->pos;
}
static gboolean g_memory_output_stream_can_seek(GSeekable *seekable) {
  return TRUE;
}
static gboolean g_memory_output_stream_seek(GSeekable *seekable, goffset offset, GSeekType type, GCancellable *cancellable, GError **error) {
  GMemoryOutputStream *stream;
  GMemoryOutputStreamPrivate *priv;
  goffset absolute;
  stream = G_MEMORY_OUTPUT_STREAM(seekable);
  priv = stream->priv;
  switch(type) {
      case G_SEEK_CUR: absolute = priv->pos + offset; break;
      case G_SEEK_SET: absolute = offset; break;
      case G_SEEK_END: absolute = priv->len + offset; break;
      default:
          g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Invalid GSeekType supplied"));
          return FALSE;
  }
  if (absolute < 0) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, _("Requested seek before the beginning of the stream"));
      return FALSE;
  }
  if (absolute > priv->len) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Requested seek beyond the end of the stream"));
      return FALSE;
  }
  priv->pos = absolute;
  return TRUE;
}
static gboolean g_memory_output_stream_can_truncate(GSeekable *seekable) {
  GMemoryOutputStream *ostream;
  GMemoryOutputStreamPrivate *priv;
  ostream = G_MEMORY_OUTPUT_STREAM(seekable);
  priv = ostream->priv;
  return priv->realloc_fn != NULL;
}
static gboolean g_memory_output_stream_truncate(GSeekable *seekable, goffset offset, GCancellable *cancellable, GError **error) {
  GMemoryOutputStream *ostream = G_MEMORY_OUTPUT_STREAM(seekable);
  if (!array_resize(ostream, offset, FALSE, error)) return FALSE;
  return TRUE;
}