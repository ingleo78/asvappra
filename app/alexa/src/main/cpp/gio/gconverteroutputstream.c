#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "config.h"
#include "gconverteroutputstream.h"
#include "gsimpleasyncresult.h"
#include "gcancellable.h"
#include "gioenumtypes.h"
#include "gioerror.h"

#define INITIAL_BUFFER_SIZE 4096
typedef struct {
  char *data;
  gsize start;
  gsize end;
  gsize size;
} Buffer;
struct _GConverterOutputStreamPrivate {
  gboolean at_output_end;
  gboolean finished;
  GConverter *converter;
  Buffer output_buffer;
  Buffer converted_buffer;
};
enum {
  PROP_0,
  PROP_CONVERTER
};
static void g_converter_output_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void g_converter_output_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void g_converter_output_stream_finalize(GObject *object);
static gssize g_converter_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error);
static gboolean g_converter_output_stream_flush(GOutputStream  *stream, GCancellable *cancellable, GError **error);
G_DEFINE_TYPE(GConverterOutputStream, g_converter_output_stream, G_TYPE_FILTER_OUTPUT_STREAM);
static void g_converter_output_stream_class_init(GConverterOutputStreamClass *klass) {
  GObjectClass *object_class;
  GOutputStreamClass *istream_class;
  g_type_class_add_private(klass, sizeof(GConverterOutputStreamPrivate));
  object_class = G_OBJECT_CLASS(klass);
  object_class->get_property = g_converter_output_stream_get_property;
  object_class->set_property = g_converter_output_stream_set_property;
  object_class->finalize = g_converter_output_stream_finalize;
  istream_class = G_OUTPUT_STREAM_CLASS(klass);
  istream_class->write_fn = g_converter_output_stream_write;
  istream_class->flush = g_converter_output_stream_flush;
  g_object_class_install_property(object_class, PROP_CONVERTER, g_param_spec_object("converter","Converter","The converter object",
							      G_TYPE_CONVERTER, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
}
static void g_converter_output_stream_finalize(GObject *object) {
  GConverterOutputStreamPrivate *priv;
  GConverterOutputStream *stream;
  stream = G_CONVERTER_OUTPUT_STREAM(object);
  priv = stream->priv;
  g_free(priv->output_buffer.data);
  g_free(priv->converted_buffer.data);
  if (priv->converter) g_object_unref(priv->converter);
  G_OBJECT_CLASS(g_converter_output_stream_parent_class)->finalize(object);
}
static void g_converter_output_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GConverterOutputStream *cstream;
  cstream = G_CONVERTER_OUTPUT_STREAM(object);
  switch(prop_id) {
      case PROP_CONVERTER: cstream->priv->converter = g_value_dup_object(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_converter_output_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GConverterOutputStreamPrivate *priv;
  GConverterOutputStream *cstream;
  cstream = G_CONVERTER_OUTPUT_STREAM(object);
  priv = cstream->priv;
  switch(prop_id) {
      case PROP_CONVERTER: g_value_set_object (value, priv->converter); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
  }
}
static void g_converter_output_stream_init(GConverterOutputStream *stream) {
  stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(stream, G_TYPE_CONVERTER_OUTPUT_STREAM, GConverterOutputStreamPrivate);
}
GOutputStream *g_converter_output_stream_new(GOutputStream *base_stream, GConverter *converter) {
  GOutputStream *stream;
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(base_stream), NULL);
  stream = g_object_new(G_TYPE_CONVERTER_OUTPUT_STREAM,"base-stream", base_stream, "converter", converter, NULL);
  return stream;
}
static gsize buffer_data_size(Buffer *buffer) {
  return buffer->end - buffer->start;
}
static gsize buffer_tailspace(Buffer *buffer) {
  return buffer->size - buffer->end;
}
static char *buffer_data(Buffer *buffer) {
  return buffer->data + buffer->start;
}
static void buffer_consumed(Buffer *buffer, gsize count) {
  buffer->start += count;
  if (buffer->start == buffer->end) buffer->start = buffer->end = 0;
}
static void compact_buffer(Buffer *buffer) {
  gsize in_buffer;
  in_buffer = buffer_data_size(buffer);
  memmove(buffer->data,buffer->data + buffer->start, in_buffer);
  buffer->end -= buffer->start;
  buffer->start = 0;
}
static void grow_buffer(Buffer *buffer) {
  char *data;
  gsize size, in_buffer;
  if (buffer->size == 0) size = INITIAL_BUFFER_SIZE;
  else size = buffer->size * 2;
  data = g_malloc(size);
  in_buffer = buffer_data_size(buffer);
  memcpy(data,buffer->data + buffer->start, in_buffer);
  g_free (buffer->data);
  buffer->data = data;
  buffer->end -= buffer->start;
  buffer->start = 0;
  buffer->size = size;
}
static void buffer_ensure_space(Buffer *buffer, gsize at_least_size) {
  gsize in_buffer, left_to_fill;
  in_buffer = buffer_data_size(buffer);
  if (in_buffer >= at_least_size) return;
  left_to_fill = buffer_tailspace(buffer);
  if (in_buffer + left_to_fill >= at_least_size) {
      if (in_buffer < 256) compact_buffer(buffer);
  } else if (buffer->size >= at_least_size) compact_buffer(buffer);
  else {
      while(buffer->size < at_least_size) grow_buffer(buffer);
  }
}
static void buffer_append(Buffer *buffer, const char *data, gsize data_size) {
  buffer_ensure_space(buffer,buffer_data_size(buffer) + data_size);
  memcpy(buffer->data + buffer->end, data, data_size);
  buffer->end += data_size;
}
static gboolean flush_buffer(GConverterOutputStream *stream, Buffer *buffer, GCancellable *cancellable, GError **error) {
  GConverterOutputStreamPrivate *priv;
  GOutputStream *base_stream;
  gsize nwritten;
  gsize available;
  gboolean res;
  priv = stream->priv;
  base_stream = G_FILTER_OUTPUT_STREAM(stream)->base_stream;
  available = buffer_data_size(&priv->converted_buffer);
  if (available > 0) {
      res = g_output_stream_write_all(base_stream, buffer_data (&priv->converted_buffer), available, &nwritten, cancellable, error);
      buffer_consumed(&priv->converted_buffer, nwritten);
      return res;
  }
  return TRUE;
}
static gssize g_converter_output_stream_write (GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error) {
  GConverterOutputStream *cstream;
  GConverterOutputStreamPrivate *priv;
  gssize retval;
  GConverterResult res;
  gsize bytes_read;
  gsize bytes_written;
  GError *my_error;
  const char *to_convert;
  gsize to_convert_size, converted_bytes;
  gboolean converting_from_buffer;
  cstream = G_CONVERTER_OUTPUT_STREAM(stream);
  priv = cstream->priv;
  if (!flush_buffer(cstream, &priv->converted_buffer, cancellable, error)) return -1;
  if (priv->finished) return 0;
  if (buffer_data_size(&priv->output_buffer) > 0) {
      converting_from_buffer = TRUE;
      buffer_append(&priv->output_buffer, buffer, count);
      to_convert = buffer_data(&priv->output_buffer);
      to_convert_size = buffer_data_size(&priv->output_buffer);
  } else {
      converting_from_buffer = FALSE;
      to_convert = buffer;
      to_convert_size = count;
  }
  buffer_ensure_space(&priv->converted_buffer, to_convert_size);
  converted_bytes = 0;
  while (!priv->finished && converted_bytes < to_convert_size) {
      if (buffer_tailspace(&priv->converted_buffer) == 0) grow_buffer(&priv->converted_buffer);
      my_error = NULL;
      res = g_converter_convert(priv->converter,to_convert + converted_bytes,to_convert_size - converted_bytes,
                                buffer_data(&priv->converted_buffer) + buffer_data_size(&priv->converted_buffer), buffer_tailspace(&priv->converted_buffer),
				          0, &bytes_read, &bytes_written, &my_error);
      if (res != G_CONVERTER_ERROR) {
          priv->converted_buffer.end += bytes_written;
          converted_bytes += bytes_read;
          if (res == G_CONVERTER_FINISHED) priv->finished = TRUE;
	  } else {
          if (g_error_matches(my_error, G_IO_ERROR,G_IO_ERROR_NO_SPACE)) {
              buffer_ensure_space(&priv->converted_buffer,priv->converted_buffer.size + 1);
              g_error_free(my_error);
              continue;
          }
          if (converted_bytes > 0) {
              g_error_free(my_error);
              break;
          }
          if (g_error_matches(my_error, G_IO_ERROR,G_IO_ERROR_PARTIAL_INPUT)) {
              if (!converting_from_buffer) buffer_append(&priv->output_buffer, buffer, count);
              g_error_free(my_error);
              return count;
          }
          g_propagate_error(error, my_error);
          return -1;
	  }
  }
  if (converting_from_buffer) {
      buffer_consumed(&priv->output_buffer, converted_bytes);
      retval = count;
  } else retval = converted_bytes;
  flush_buffer(cstream, &priv->converted_buffer, cancellable, NULL);
  return retval;
}
static gboolean g_converter_output_stream_flush(GOutputStream *stream, GCancellable *cancellable, GError **error) {
  GConverterOutputStream *cstream;
  GConverterOutputStreamPrivate *priv;
  GConverterResult res;
  GError *my_error;
  gboolean is_closing;
  gboolean flushed;
  gsize bytes_read;
  gsize bytes_written;
  cstream = G_CONVERTER_OUTPUT_STREAM(stream);
  priv = cstream->priv;
  is_closing = g_output_stream_is_closing(stream);
  if (!flush_buffer(cstream, &priv->converted_buffer, cancellable, error)) return FALSE;
  buffer_ensure_space(&priv->converted_buffer, 1);
  flushed = FALSE;
  while(!priv->finished && !flushed) {
      if (buffer_tailspace(&priv->converted_buffer) == 0) grow_buffer(&priv->converted_buffer);
      my_error = NULL;
      res = g_converter_convert(priv->converter, buffer_data (&priv->output_buffer), buffer_data_size (&priv->output_buffer),
				         buffer_data(&priv->converted_buffer) + buffer_data_size (&priv->converted_buffer), buffer_tailspace(&priv->converted_buffer),
				                is_closing ? G_CONVERTER_INPUT_AT_END : G_CONVERTER_FLUSH, &bytes_read, &bytes_written, &my_error);
      if (res != G_CONVERTER_ERROR) {
          priv->converted_buffer.end += bytes_written;
          buffer_consumed(&priv->output_buffer, bytes_read);
          if (res == G_CONVERTER_FINISHED) priv->finished = TRUE;
          if (!is_closing && res == G_CONVERTER_FLUSHED) {
              g_assert(buffer_data_size(&priv->output_buffer) == 0);
              flushed = TRUE;
          }
	  } else {
          if (g_error_matches(my_error, G_IO_ERROR,G_IO_ERROR_NO_SPACE)) {
              buffer_ensure_space(&priv->converted_buffer,priv->converted_buffer.size + 1);
              g_error_free(my_error);
              continue;
          }
          g_propagate_error(error, my_error);
          return FALSE;
	  }
  }
  if (!flush_buffer(cstream, &priv->converted_buffer, cancellable, error)) return FALSE;
  return TRUE;
}
GConverter *g_converter_output_stream_get_converter(GConverterOutputStream *converter_stream) {
  return converter_stream->priv->converter;
}