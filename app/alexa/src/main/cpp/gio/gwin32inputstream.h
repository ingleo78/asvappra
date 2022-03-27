#ifndef __G_WIN32_INPUT_STREAM_H__
#define __G_WIN32_INPUT_STREAM_H__

G_BEGIN_DECLS
#define G_TYPE_WIN32_INPUT_STREAM  (g_win32_input_stream_get_type ())
#define G_WIN32_INPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_WIN32_INPUT_STREAM, GWin32InputStream))
#define G_WIN32_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_WIN32_INPUT_STREAM, GWin32InputStreamClass))
#define G_IS_WIN32_INPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_WIN32_INPUT_STREAM))
#define G_IS_WIN32_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_WIN32_INPUT_STREAM))
#define G_WIN32_INPUT_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), G_TYPE_WIN32_INPUT_STREAM, GWin32InputStreamClass))
typedef struct _GWin32InputStream GWin32InputStream;
typedef struct _GWin32InputStreamClass GWin32InputStreamClass;
typedef struct _GWin32InputStreamPrivate GWin32InputStreamPrivate;
struct _GWin32InputStream {
  GInputStream parent_instance;
  GWin32InputStreamPrivate *priv;
};
struct _GWin32InputStreamClass {
  GInputStreamClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
GType g_win32_input_stream_get_type(void) G_GNUC_CONST;
GInputStream *g_win32_input_stream_new(void *handle, gboolean close_handle);
void g_win32_input_stream_set_close_handle(GWin32InputStream *stream, gboolean close_handle);
gboolean g_win32_input_stream_get_close_handle(GWin32InputStream *stream);
void *g_win32_input_stream_get_handle(GWin32InputStream *stream);
G_END_DECLS

#endif