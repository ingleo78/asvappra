#ifndef __G_WINHTTP_FILE_OUTPUT_STREAM_H__
#define __G_WINHTTP_FILE_OUTPUT_STREAM_H__

#include "../gfileoutputstream.h"
#include "gwinhttpfile.h"

G_BEGIN_DECLS
#define G_TYPE_WINHTTP_FILE_OUTPUT_STREAM  (_g_winhttp_file_output_stream_get_type())
#define G_WINHTTP_FILE_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_WINHTTP_FILE_OUTPUT_STREAM, GWinHttpFileOutputStream))
#define G_WINHTTP_FILE_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_WINHTTP_FILE_OUTPUT_STREAM, GWinHttpFileOutputStreamClass))
#define G_IS_WINHTTP_FILE_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_WINHTTP_FILE_OUTPUT_STREAM))
#define G_IS_WINHTTP_FILE_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_WINHTTP_FILE_OUTPUT_STREAM))
#define G_WINHTTP_FILE_OUTPUT_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_WINHTTP_FILE_OUTPUT_STREAM, GWinHttpFileOutputStreamClass))
typedef struct _GWinHttpFileOutputStream GWinHttpFileOutputStream;
typedef struct _GWinHttpFileOutputStreamClass GWinHttpFileOutputStreamClass;
GType _g_winhttp_file_output_stream_get_type(void) G_GNUC_CONST;
GFileOutputStream *_g_winhttp_file_output_stream_new(GWinHttpFile *file, HINTERNET     connection);
G_END_DECLS

#endif