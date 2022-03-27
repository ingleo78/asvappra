#include "../../glib/glib.h"
#include "../../glib/glibintl.h"
#include "../config.h"
#include "../gcancellable.h"
#include "../gioerror.h"
#include "gwinhttpfileinputstream.h"

struct _GWinHttpFileInputStream {
  GFileInputStream parent_instance;
  GWinHttpFile *file;
  gboolean request_sent;
  HINTERNET connection;
  HINTERNET request;
};
struct _GWinHttpFileInputStreamClass {
  GFileInputStreamClass parent_class;
};
#define g_winhttp_file_input_stream_get_type _g_winhttp_file_input_stream_get_type
G_DEFINE_TYPE (GWinHttpFileInputStream, g_winhttp_file_input_stream, G_TYPE_FILE_INPUT_STREAM);
static gssize g_winhttp_file_input_stream_read(GInputStream *stream, void *buffer, gsize count, GCancellable *cancellable, GError **error);
static gboolean g_winhttp_file_input_stream_close(GInputStream *stream, GCancellable *cancellable, GError **error);
static void g_winhttp_file_input_stream_finalize(GObject *object) {
  /*GWinHttpFileInputStream *winhttp_stream;
  winhttp_stream = G_WINHTTP_FILE_INPUT_STREAM(object);
  if (winhttp_stream->request != NULL) G_WINHTTP_VFS_GET_CLASS(winhttp_stream->file->vfs)->funcs->pWinHttpCloseHandle(winhttp_stream->request);
  if (winhttp_stream->connection != NULL) G_WINHTTP_VFS_GET_CLASS(winhttp_stream->file->vfs)->funcs->pWinHttpCloseHandle(winhttp_stream->connection);
  g_object_unref(winhttp_stream->file);
  winhttp_stream->file = NULL;
  G_OBJECT_CLASS(g_winhttp_file_input_stream_parent_class)->finalize(object);*/
}
static void g_winhttp_file_input_stream_class_init(GWinHttpFileInputStreamClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GInputStreamClass *stream_class = G_INPUT_STREAM_CLASS(klass);
  gobject_class->finalize = g_winhttp_file_input_stream_finalize;
  stream_class->read_fn = g_winhttp_file_input_stream_read;
  stream_class->close_fn = g_winhttp_file_input_stream_close;
}
static void g_winhttp_file_input_stream_init(GWinHttpFileInputStream *info) {}
GFileInputStream *_g_winhttp_file_input_stream_new(GWinHttpFile *file, HINTERNET connection, HINTERNET request) {
  GWinHttpFileInputStream *stream;
  stream = g_object_new(G_TYPE_WINHTTP_FILE_INPUT_STREAM, NULL);
  stream->file = g_object_ref(file);
  stream->request_sent = FALSE;
  stream->connection = connection;
  stream->request = request;
  return G_FILE_INPUT_STREAM(stream);
}
static gssize g_winhttp_file_input_stream_read(GInputStream *stream, void *buffer, gsize count, GCancellable *cancellable, GError **error) {
  /*GWinHttpFileInputStream *winhttp_stream = G_WINHTTP_FILE_INPUT_STREAM (stream);
  DWORD bytes_read;
  if (!winhttp_stream->request_sent) {
      if (!G_WINHTTP_VFS_GET_CLASS (winhttp_stream->file->vfs)->funcs->pWinHttpSendRequest(winhttp_stream->request, NULL, 0, NULL, 0, 0, 0)) {
          _g_winhttp_set_error (error, GetLastError (), "GET request");
          return -1;
      }
      if (!_g_winhttp_response(winhttp_stream->file->vfs, winhttp_stream->request, error,"GET request")) return -1;
      winhttp_stream->request_sent = TRUE;
  }
  if (!G_WINHTTP_VFS_GET_CLASS(winhttp_stream->file->vfs)->funcs->pWinHttpReadData(winhttp_stream->request, buffer, count, &bytes_read)) {
      _g_winhttp_set_error (error, GetLastError (), "GET request");
      return -1;
  }
  return bytes_read;*/
  return 0;
}
static gboolean g_winhttp_file_input_stream_close(GInputStream *stream, GCancellable *cancellable, GError **error) {
  GWinHttpFileInputStream *winhttp_stream = G_WINHTTP_FILE_INPUT_STREAM(stream);
  //if (winhttp_stream->connection != NULL) G_WINHTTP_VFS_GET_CLASS(winhttp_stream->file->vfs)->funcs->pWinHttpCloseHandle(winhttp_stream->connection);
  winhttp_stream->connection = NULL;
  return TRUE;
}