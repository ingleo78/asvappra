#ifndef __G_SOCKET_OUTPUT_STREAM_H__
#define __G_SOCKET_OUTPUT_STREAM_H__

#include "goutputstream.h"
#include "gsocket.h"

G_BEGIN_DECLS
#define G_TYPE_SOCKET_OUTPUT_STREAM  (_g_socket_output_stream_get_type())
#define G_SOCKET_OUTPUT_STREAM(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_SOCKET_OUTPUT_STREAM, GSocketOutputStream))
#define G_SOCKET_OUTPUT_STREAM_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_SOCKET_OUTPUT_STREAM, GSocketOutputStreamClass))
#define G_IS_SOCKET_OUTPUT_STREAM(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_SOCKET_OUTPUT_STREAM))
#define G_IS_SOCKET_OUTPUT_STREAM_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_SOCKET_OUTPUT_STREAM))
#define G_SOCKET_OUTPUT_STREAM_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_SOCKET_OUTPUT_STREAM, GSocketOutputStreamClass))
typedef struct _GSocketOutputStreamPrivate GSocketOutputStreamPrivate;
typedef struct _GSocketOutputStreamClass GSocketOutputStreamClass;
typedef struct _GSocketOutputStream GSocketOutputStream;
struct _GSocketOutputStreamClass {
  GOutputStreamClass parent_class;
};
struct _GSocketOutputStream {
  GOutputStream parent_instance;
  GSocketOutputStreamPrivate *priv;
};
GType _g_socket_output_stream_get_type(void) G_GNUC_CONST;
GSocketOutputStream *_g_socket_output_stream_new(GSocket *socket);
G_END_DECLS

#endif