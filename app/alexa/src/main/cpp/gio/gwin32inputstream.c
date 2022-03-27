#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gio.h"
#include "gioerror.h"
#include "gsimpleasyncresult.h"
#include "gwin32inputstream.h"
#include "gcancellable.h"
#include "gasynchelper.h"
#include "gregistrysettingsbackend.h"

enum {
  PROP_0,
  PROP_HANDLE,
  PROP_CLOSE_HANDLE
};
G_DEFINE_TYPE(GWin32InputStream, g_win32_input_stream, G_TYPE_INPUT_STREAM);
struct _GWin32InputStreamPrivate {
  HANDLE handle;
  gboolean close_handle;
};
static void g_win32_input_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void g_win32_input_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static gssize g_win32_input_stream_read(GInputStream *stream, void *buffer, gsize count, GCancellable *cancellable, GError **error);
static gboolean g_win32_input_stream_close(GInputStream *stream, GCancellable *cancellable, GError **error);
static void g_win32_input_stream_finalize(GObject *object) {
  G_OBJECT_CLASS(g_win32_input_stream_parent_class)->finalize (object);
}
static void g_win32_input_stream_class_init(GWin32InputStreamClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GInputStreamClass *stream_class = G_INPUT_STREAM_CLASS(klass);
  g_type_class_add_private (klass, sizeof(GWin32InputStreamPrivate));
  gobject_class->get_property = g_win32_input_stream_get_property;
  gobject_class->set_property = g_win32_input_stream_set_property;
  gobject_class->finalize = g_win32_input_stream_finalize;
  stream_class->read_fn = g_win32_input_stream_read;
  stream_class->close_fn = g_win32_input_stream_close;
  g_object_class_install_property(gobject_class,PROP_HANDLE,g_param_spec_pointer("handle", P_("File handle"),
							      P_("The file handle to read from"),G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME |
							      G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
  g_object_class_install_property(gobject_class,PROP_CLOSE_HANDLE,g_param_spec_boolean("close-handle", P_("Close file handle"),
							      P_("Whether to close the file handle when the stream is closed"), TRUE,G_PARAM_READABLE | G_PARAM_WRITABLE |
							      G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
}
static void g_win32_input_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GWin32InputStream *win32_stream;
  win32_stream = G_WIN32_INPUT_STREAM(object);
  switch(prop_id) {
      case PROP_HANDLE: win32_stream->priv->handle = g_value_get_pointer(value); break;
      case PROP_CLOSE_HANDLE: win32_stream->priv->close_handle = g_value_get_boolean(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_win32_input_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GWin32InputStream *win32_stream;
  win32_stream = G_WIN32_INPUT_STREAM(object);
  switch(prop_id) {
    case PROP_HANDLE: g_value_set_pointer(value, win32_stream->priv->handle); break;
    case PROP_CLOSE_HANDLE: g_value_set_boolean(value, win32_stream->priv->close_handle); break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_win32_input_stream_init(GWin32InputStream *win32_stream) {
  win32_stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(win32_stream, G_TYPE_WIN32_INPUT_STREAM, GWin32InputStreamPrivate);
  win32_stream->priv->handle = NULL;
  win32_stream->priv->close_handle = TRUE;
}
GInputStream *g_win32_input_stream_new(void *handle, gboolean close_handle) {
  GWin32InputStream *stream;
  g_return_val_if_fail(handle != NULL, NULL);
  stream = g_object_new(G_TYPE_WIN32_INPUT_STREAM,"handle", handle, "close-handle", close_handle, NULL);
  return G_INPUT_STREAM(stream);
}
void g_win32_input_stream_set_close_handle(GWin32InputStream *stream, gboolean close_handle) {
  g_return_if_fail(G_IS_WIN32_INPUT_STREAM(stream));
  close_handle = close_handle != FALSE;
  if (stream->priv->close_handle != close_handle) {
      stream->priv->close_handle = close_handle;
      g_object_notify(G_OBJECT(stream), "close-handle");
  }
}
gboolean g_win32_input_stream_get_close_handle(GWin32InputStream *stream) {
  g_return_val_if_fail(G_IS_WIN32_INPUT_STREAM(stream), FALSE);
  return stream->priv->close_handle;
}
void *g_win32_input_stream_get_handle(GWin32InputStream *stream) {
  g_return_val_if_fail(G_IS_WIN32_INPUT_STREAM (stream), NULL);
  return stream->priv->handle;
}
static gssize g_win32_input_stream_read(GInputStream  *stream, void *buffer, gsize count,GCancellable *cancellable, GError **error) {
  /*GWin32InputStream *win32_stream;
  BOOL res;
  DWORD nbytes, nread;
  win32_stream = G_WIN32_INPUT_STREAM(stream);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return -1;
  if (count > G_MAXINT) nbytes = G_MAXINT;
  else nbytes = count;
  res = ReadFile(win32_stream->priv->handle, buffer, nbytes, &nread, NULL);
  if (!res) {
      int errsv = GetLastError();
      gchar *emsg;
      if (errsv == ERROR_HANDLE_EOF || errsv == ERROR_BROKEN_PIPE) return 0;
      emsg = g_win32_error_message(errsv);
      g_set_error(error, G_IO_ERROR, g_io_error_from_win32_error(errsv), _("Error reading from handle: %s"), emsg);
      g_free(emsg);
      return -1;
  }
  return nread;*/
  return 0;
}
static gboolean g_win32_input_stream_close(GInputStream *stream, GCancellable *cancellable, GError **error) {
  /*GWin32InputStream *win32_stream;
  BOOL res;
  win32_stream = G_WIN32_INPUT_STREAM(stream);
  if (!win32_stream->priv->close_handle) return TRUE;
  res = CloseHandle(win32_stream->priv->handle);
  if (!res) {
      int errsv = GetLastError();
      gchar *emsg = g_win32_error_message(errsv);
      g_set_error(error, G_IO_ERROR, g_io_error_from_win32_error (errsv), _("Error closing handle: %s"), emsg);
      g_free(emsg);
      return FALSE;
  }*/
  return TRUE;
}