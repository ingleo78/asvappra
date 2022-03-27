#ifndef __G_UNIX_OUTPUT_STREAM_H__
#define __G_UNIX_OUTPUT_STREAM_H__

G_BEGIN_DECLS
#define G_TYPE_UNIX_OUTPUT_STREAM  (g_unix_output_stream_get_type())
#define G_UNIX_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_UNIX_OUTPUT_STREAM, GUnixOutputStream))
#define G_UNIX_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_UNIX_OUTPUT_STREAM, GUnixOutputStreamClass))
#define G_IS_UNIX_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_UNIX_OUTPUT_STREAM))
#define G_IS_UNIX_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_UNIX_OUTPUT_STREAM))
#define G_UNIX_OUTPUT_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_UNIX_OUTPUT_STREAM, GUnixOutputStreamClass))
typedef struct _GUnixOutputStream GUnixOutputStream;
typedef struct _GUnixOutputStreamClass GUnixOutputStreamClass;
typedef struct _GUnixOutputStreamPrivate GUnixOutputStreamPrivate;
struct _GUnixOutputStream {
  GOutputStream parent_instance;
  GUnixOutputStreamPrivate *priv;
};
struct _GUnixOutputStreamClass {
  GOutputStreamClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
GType g_unix_output_stream_get_type(void) G_GNUC_CONST;
GOutputStream *g_unix_output_stream_new(gint fd, gboolean close_fd);
void g_unix_output_stream_set_close_fd(GUnixOutputStream *stream, gboolean close_fd);
gboolean g_unix_output_stream_get_close_fd(GUnixOutputStream *stream);
gint g_unix_output_stream_get_fd(GUnixOutputStream *stream);
G_END_DECLS

#endif