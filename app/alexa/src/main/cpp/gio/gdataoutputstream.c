#include <string.h>
#include "../glib/glibintl.h"
#include "config.h"
#include "gdataoutputstream.h"
#include "gioenumtypes.h"

struct _GDataOutputStreamPrivate {
  GDataStreamByteOrder byte_order;
};
enum {
  PROP_0,
  PROP_BYTE_ORDER
};
static void g_data_output_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void g_data_output_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
G_DEFINE_TYPE(GDataOutputStream, g_data_output_stream, G_TYPE_FILTER_OUTPUT_STREAM);
static void g_data_output_stream_class_init(GDataOutputStreamClass *klass) {
  GObjectClass *object_class;
  g_type_class_add_private (klass, sizeof(GDataOutputStreamPrivate));
  object_class = G_OBJECT_CLASS(klass);
  object_class->get_property = g_data_output_stream_get_property;
  object_class->set_property = g_data_output_stream_set_property;
  g_object_class_install_property(object_class, PROP_BYTE_ORDER, g_param_spec_enum("byte-order", P_("Byte order"),
                                  P_("The byte order"), G_TYPE_DATA_STREAM_BYTE_ORDER, G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN,
                                  G_PARAM_READWRITE | G_PARAM_STATIC_NAME|G_PARAM_STATIC_BLURB));
}
static void g_data_output_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GDataOutputStream *dstream;
  dstream = G_DATA_OUTPUT_STREAM(object);
  switch (prop_id) {
      case PROP_BYTE_ORDER: g_data_output_stream_set_byte_order(dstream, g_value_get_enum(value)); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_data_output_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GDataOutputStreamPrivate *priv;
  GDataOutputStream *dstream;
  dstream = G_DATA_OUTPUT_STREAM(object);
  priv = dstream->priv;
  switch(prop_id) {
      case PROP_BYTE_ORDER: g_value_set_enum(value, priv->byte_order); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_data_output_stream_init(GDataOutputStream *stream) {
  stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(stream, G_TYPE_DATA_OUTPUT_STREAM, GDataOutputStreamPrivate);
  stream->priv->byte_order = G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN;
}
GDataOutputStream *g_data_output_stream_new(GOutputStream *base_stream) {
  GDataOutputStream *stream;
  g_return_val_if_fail(G_IS_OUTPUT_STREAM(base_stream), NULL);
  stream = g_object_new(G_TYPE_DATA_OUTPUT_STREAM,"base-stream", base_stream, NULL);
  return stream;
}
void g_data_output_stream_set_byte_order(GDataOutputStream *stream, GDataStreamByteOrder order) {
  GDataOutputStreamPrivate *priv;
  g_return_if_fail(G_IS_DATA_OUTPUT_STREAM(stream));
  priv = stream->priv;
  if (priv->byte_order != order) {
      priv->byte_order = order;
      g_object_notify(G_OBJECT(stream), "byte-order");
  }
}
GDataStreamByteOrder g_data_output_stream_get_byte_order(GDataOutputStream *stream) {
  g_return_val_if_fail(G_IS_DATA_OUTPUT_STREAM(stream), G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN);
  return stream->priv->byte_order;
}
gboolean g_data_output_stream_put_byte(GDataOutputStream *stream, guchar data, GCancellable *cancellable, GError **error) {
  gsize bytes_written;
  g_return_val_if_fail(G_IS_DATA_OUTPUT_STREAM(stream), FALSE);
  return g_output_stream_write_all(G_OUTPUT_STREAM(stream), &data, 1, &bytes_written, cancellable, error);
}
gboolean g_data_output_stream_put_int16(GDataOutputStream *stream, gint16 data, GCancellable *cancellable, GError **error) {
  gsize bytes_written;
  g_return_val_if_fail(G_IS_DATA_OUTPUT_STREAM(stream), FALSE);
  switch(stream->priv->byte_order) {
      case G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN: data = GINT16_TO_BE(data); break;
      case G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN: data = GINT16_TO_LE(data); break;
  }
  return g_output_stream_write_all(G_OUTPUT_STREAM(stream), &data, 2, &bytes_written, cancellable, error);
}
gboolean g_data_output_stream_put_uint16(GDataOutputStream *stream, guint16 data, GCancellable *cancellable, GError **error) {
  gsize bytes_written;
  g_return_val_if_fail(G_IS_DATA_OUTPUT_STREAM(stream), FALSE);
  switch(stream->priv->byte_order) {
      case G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN: data = GUINT16_TO_BE(data); break;
      case G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN: data = GUINT16_TO_LE(data); break;
  }
  return g_output_stream_write_all(G_OUTPUT_STREAM(stream), &data,2, &bytes_written, cancellable, error);
}
gboolean g_data_output_stream_put_int32(GDataOutputStream *stream, gint32 data, GCancellable *cancellable, GError **error) {
  gsize bytes_written;
  g_return_val_if_fail(G_IS_DATA_OUTPUT_STREAM(stream), FALSE);
  switch(stream->priv->byte_order) {
      case G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN: data = GINT32_TO_BE(data); break;
      case G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN: data = GINT32_TO_LE(data); break;
  }
  return g_output_stream_write_all(G_OUTPUT_STREAM(stream), &data,4, &bytes_written, cancellable, error);
}
gboolean g_data_output_stream_put_uint32(GDataOutputStream *stream, guint32 data, GCancellable *cancellable, GError **error) {
  gsize bytes_written;
  g_return_val_if_fail(G_IS_DATA_OUTPUT_STREAM(stream), FALSE);
  switch(stream->priv->byte_order) {
      case G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN: data = GUINT32_TO_BE(data); break;
      case G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN: data = GUINT32_TO_LE(data); break;
  }
  return g_output_stream_write_all(G_OUTPUT_STREAM(stream), &data, 4, &bytes_written, cancellable, error);
}
gboolean g_data_output_stream_put_int64(GDataOutputStream *stream, gint64 data, GCancellable *cancellable, GError **error) {
  gsize bytes_written;
  g_return_val_if_fail(G_IS_DATA_OUTPUT_STREAM(stream), FALSE);
  switch(stream->priv->byte_order) {
      case G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN: data = GINT64_TO_BE(data); break;
      case G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN: data = GINT64_TO_LE(data); break;
  }
  return g_output_stream_write_all(G_OUTPUT_STREAM(stream), &data, 8, &bytes_written, cancellable, error);
}
gboolean g_data_output_stream_put_uint64(GDataOutputStream *stream, guint64 data, GCancellable *cancellable, GError **error) {
  gsize bytes_written;
  g_return_val_if_fail(G_IS_DATA_OUTPUT_STREAM(stream), FALSE);
  switch(stream->priv->byte_order) {
    case G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN: data = GUINT64_TO_BE(data); break;
    case G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN: data = GUINT64_TO_LE(data); break;
  }
  return g_output_stream_write_all(G_OUTPUT_STREAM(stream), &data,8, &bytes_written, cancellable, error);
}
gboolean g_data_output_stream_put_string(GDataOutputStream *stream, const char *str, GCancellable *cancellable, GError **error) {
  gsize bytes_written;
  g_return_val_if_fail(G_IS_DATA_OUTPUT_STREAM(stream), FALSE);
  g_return_val_if_fail(str != NULL, FALSE);
  return g_output_stream_write_all(G_OUTPUT_STREAM(stream), str, strlen(str), &bytes_written, cancellable, error);
}