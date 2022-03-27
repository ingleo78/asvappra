#include "../glib/glib.h"
#include "../glib/gstdio.h"
#include "../glib/glibintl.h"
#include "../glib/pcre/pcre_internal.h"
#include "config.h"
#include "gio.h"
#include "gioerror.h"
#include "gwin32outputstream.h"
#include "gcancellable.h"
#include "gsimpleasyncresult.h"
#include "gasynchelper.h"
#include "gregistrysettingsbackend.h"

enum {
  PROP_0,
  PROP_HANDLE,
  PROP_CLOSE_HANDLE
};
G_DEFINE_TYPE(GWin32OutputStream, g_win32_output_stream, G_TYPE_OUTPUT_STREAM);
struct _GWin32OutputStreamPrivate {
  HANDLE handle;
  gboolean close_handle;
};
static void g_win32_output_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void g_win32_output_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static gssize g_win32_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error);
static gboolean g_win32_output_stream_close(GOutputStream *stream, GCancellable *cancellable, GError **error);
static void g_win32_output_stream_finalize(GObject *object) {
  G_OBJECT_CLASS (g_win32_output_stream_parent_class)->finalize (object);
}
static void g_win32_output_stream_class_init(GWin32OutputStreamClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GOutputStreamClass *stream_class = G_OUTPUT_STREAM_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GWin32OutputStreamPrivate));
  gobject_class->get_property = g_win32_output_stream_get_property;
  gobject_class->set_property = g_win32_output_stream_set_property;
  gobject_class->finalize = g_win32_output_stream_finalize;
  stream_class->write_fn = g_win32_output_stream_write;
  stream_class->close_fn = g_win32_output_stream_close;
  g_object_class_install_property(gobject_class,PROP_HANDLE,g_param_spec_pointer("handle", P_("File handle"),
							      P_("The file handle to write to"),G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME |
							      G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
  g_object_class_install_property(gobject_class,PROP_CLOSE_HANDLE,g_param_spec_boolean("close-handle", P_("Close file handle"),
							      P_("Whether to close the file handle when the stream is closed"), TRUE,G_PARAM_READABLE | G_PARAM_WRITABLE |
							      G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
}
static void g_win32_output_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GWin32OutputStream *win32_stream;
  win32_stream = G_WIN32_OUTPUT_STREAM(object);
  switch(prop_id) {
      case PROP_HANDLE: win32_stream->priv->handle = g_value_get_pointer(value); break;
      case PROP_CLOSE_HANDLE: win32_stream->priv->close_handle = g_value_get_boolean(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_win32_output_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GWin32OutputStream *win32_stream;
  win32_stream = G_WIN32_OUTPUT_STREAM(object);
  switch(prop_id) {
    case PROP_HANDLE: g_value_set_pointer(value, win32_stream->priv->handle); break;
    case PROP_CLOSE_HANDLE: g_value_set_boolean(value, win32_stream->priv->close_handle); break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_win32_output_stream_init(GWin32OutputStream *win32_stream) {
  win32_stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(win32_stream, G_TYPE_WIN32_OUTPUT_STREAM, GWin32OutputStreamPrivate);
  win32_stream->priv->handle = NULL;
  win32_stream->priv->close_handle = TRUE;
}
GOutputStream *g_win32_output_stream_new(void *handle, gboolean close_handle) {
  GWin32OutputStream *stream;
  g_return_val_if_fail(handle != NULL, NULL);
  stream = g_object_new(G_TYPE_WIN32_OUTPUT_STREAM, "handle", handle, "close-handle", close_handle, NULL);
  return G_OUTPUT_STREAM(stream);
}
void g_win32_output_stream_set_close_handle(GWin32OutputStream *stream, gboolean close_handle) {
  g_return_if_fail(G_IS_WIN32_OUTPUT_STREAM(stream));
  close_handle = close_handle != FALSE;
  if (stream->priv->close_handle != close_handle) {
      stream->priv->close_handle = close_handle;
      g_object_notify(G_OBJECT(stream), "close-handle");
    }
}
gboolean g_win32_output_stream_get_close_handle(GWin32OutputStream *stream) {
  g_return_val_if_fail(G_IS_WIN32_OUTPUT_STREAM(stream), FALSE);
  return stream->priv->close_handle;
}
void *g_win32_output_stream_get_handle(GWin32OutputStream *stream) {
  g_return_val_if_fail(G_IS_WIN32_OUTPUT_STREAM(stream), NULL);
  return stream->priv->handle;
}
static gssize g_win32_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error) {
  /*GWin32OutputStream *win32_stream;
  BOOL res;
  DWORD nbytes, nwritten;
  win32_stream = G_WIN32_OUTPUT_STREAM(stream);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return -1;
  if (count > G_MAXINT) nbytes = G_MAXINT;
  else nbytes = count;
  res = WriteFile(win32_stream->priv->handle, buffer, nbytes, &nwritten, NULL);
  if (!res) {
      int errsv = GetLastError();
      gchar *emsg = g_win32_error_message(errsv);
      if (errsv == ERROR_HANDLE_EOF) return 0;
      g_set_error(error, G_IO_ERROR, g_io_error_from_win32_error(errsv), _("Error writing to handle: %s"), emsg);
      g_free(emsg);
      return -1;
  }
  return nwritten;*/
  return 0;
}
static gboolean g_win32_output_stream_close(GOutputStream *stream, GCancellable *cancellable, GError **error) {
  /*GWin32OutputStream *win32_stream;
  BOOL res;
  win32_stream = G_WIN32_OUTPUT_STREAM(stream);
  if (!win32_stream->priv->close_handle) return TRUE;
  res = CloseHandle(win32_stream->priv->handle);
  if (!res) {
      int errsv = GetLastError();
      gchar *emsg = g_win32_error_message(errsv);
      g_set_error(error, G_IO_ERROR, g_io_error_from_win32_error(errsv), _("Error closing handle: %s"), emsg);
      g_free(emsg);
      return FALSE;
  }*/
  return TRUE;
}