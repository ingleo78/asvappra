#include "../../glib/glib.h"
#include "../../glib/glibintl.h"
#include "../config.h"
#include "../gcancellable.h"
#include "../gioerror.h"
#include "gwinhttpfileoutputstream.h"

struct _GWinHttpFileOutputStream {
  GFileOutputStream parent_instance;
  GWinHttpFile *file;
  HINTERNET connection;
  goffset offset;
};
struct _GWinHttpFileOutputStreamClass {
  GFileOutputStreamClass parent_class;
};
#define g_winhttp_file_output_stream_get_type _g_winhttp_file_output_stream_get_type
//G_DEFINE_TYPE(GWinHttpFileOutputStream, g_winhttp_file_output_stream, G_TYPE_FILE_OUTPUT_STREAM);
static gssize g_winhttp_file_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error);
static void g_winhttp_file_output_stream_finalize(GObject *object) {
  /*GWinHttpFileOutputStream *winhttp_stream;
  winhttp_stream = G_WINHTTP_FILE_OUTPUT_STREAM (object);
  if (winhttp_stream->connection != NULL) G_WINHTTP_VFS_GET_CLASS (winhttp_stream->file->vfs)->funcs->pWinHttpCloseHandle (winhttp_stream->connection);
  G_OBJECT_CLASS (g_winhttp_file_output_stream_parent_class)->finalize (object);*/
}
static void g_winhttp_file_output_stream_class_init(GWinHttpFileOutputStreamClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GOutputStreamClass *stream_class = G_OUTPUT_STREAM_CLASS (klass);
  gobject_class->finalize = g_winhttp_file_output_stream_finalize;
  stream_class->write_fn = g_winhttp_file_output_stream_write;
}
static void g_winhttp_file_output_stream_init (GWinHttpFileOutputStream *info) {}
GFileOutputStream *_g_winhttp_file_output_stream_new(GWinHttpFile *file, HINTERNET connection) {
  GWinHttpFileOutputStream *stream;
  stream = g_object_new (G_TYPE_WINHTTP_FILE_OUTPUT_STREAM, NULL);
  stream->file = file;
  stream->connection = connection;
  stream->offset = 0;
  return G_FILE_OUTPUT_STREAM (stream);
}
static gssize g_winhttp_file_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error) {
  /*GWinHttpFileOutputStream *winhttp_stream = G_WINHTTP_FILE_OUTPUT_STREAM (stream);
  HINTERNET request;
  char *headers;
  wchar_t *wheaders;
  DWORD bytes_written;
  request = G_WINHTTP_VFS_GET_CLASS(winhttp_stream->file->vfs)->funcs->pWinHttpOpenRequest(winhttp_stream->connection, L"PUT", winhttp_stream->file->url.lpszUrlPath,
                                    NULL, WINHTTP_NO_REFERER, NULL, winhttp_stream->file->url.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);
  if (request == NULL) {
      _g_winhttp_set_error(error, GetLastError(), "PUT request");
      return -1;
  }
  headers = g_strdup_printf("Content-Range: bytes %" G_GINT64_FORMAT "-%" G_GINT64_FORMAT "/*\r\n", winhttp_stream->offset, winhttp_stream->offset + count);
  wheaders = g_utf8_to_utf16(headers, -1, NULL, NULL, NULL);
  g_free (headers);
  if (!G_WINHTTP_VFS_GET_CLASS(winhttp_stream->file->vfs)->funcs->pWinHttpSendRequest(request, wheaders, -1, NULL, 0, count, 0)) {
      _g_winhttp_set_error(error, GetLastError (), "PUT request");
      G_WINHTTP_VFS_GET_CLASS(winhttp_stream->file->vfs)->funcs->pWinHttpCloseHandle (request);
      g_free(wheaders);
      return -1;
  }
  g_free (wheaders);
  if (!G_WINHTTP_VFS_GET_CLASS(winhttp_stream->file->vfs)->funcs->pWinHttpWriteData(request, buffer, count, &bytes_written)) {
      _g_winhttp_set_error(error, GetLastError(), "PUT request");
      G_WINHTTP_VFS_GET_CLASS(winhttp_stream->file->vfs)->funcs->pWinHttpCloseHandle(request);
      return -1;
  }
  winhttp_stream->offset += bytes_written;
  if (!_g_winhttp_response(winhttp_stream->file->vfs, request, error,"PUT request")) {
      G_WINHTTP_VFS_GET_CLASS(winhttp_stream->file->vfs)->funcs->pWinHttpCloseHandle(request);
      return -1;
  }
  G_WINHTTP_VFS_GET_CLASS(winhttp_stream->file->vfs)->funcs->pWinHttpCloseHandle(request);
  return bytes_written;*/
  return 0;
}