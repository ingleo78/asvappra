#ifndef __G_WIN32_OUTPUT_STREAM_H__
#define __G_WIN32_OUTPUT_STREAM_H__

G_BEGIN_DECLS
#define G_TYPE_WIN32_OUTPUT_STREAM  (g_win32_output_stream_get_type())
#define G_WIN32_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_WIN32_OUTPUT_STREAM, GWin32OutputStream))
#define G_WIN32_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_WIN32_OUTPUT_STREAM, GWin32OutputStreamClass))
#define G_IS_WIN32_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_WIN32_OUTPUT_STREAM))
#define G_IS_WIN32_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_WIN32_OUTPUT_STREAM))
#define G_WIN32_OUTPUT_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_WIN32_OUTPUT_STREAM, GWin32OutputStreamClass))
typedef struct _GWin32OutputStream GWin32OutputStream;
typedef struct _GWin32OutputStreamClass GWin32OutputStreamClass;
typedef struct _GWin32OutputStreamPrivate GWin32OutputStreamPrivate;
struct _GWin32OutputStream {
  GOutputStream parent_instance;
  GWin32OutputStreamPrivate *priv;
};
struct _GWin32OutputStreamClass {
  GOutputStreamClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
GType g_win32_output_stream_get_type(void) G_GNUC_CONST;
GOutputStream *g_win32_output_stream_new(void *handle, gboolean close_handle);
void g_win32_output_stream_set_close_handle(GWin32OutputStream *stream, gboolean close_handle);
gboolean g_win32_output_stream_get_close_handle(GWin32OutputStream *stream);
void *g_win32_output_stream_get_handle(GWin32OutputStream *stream);
G_END_DECLS

#endif