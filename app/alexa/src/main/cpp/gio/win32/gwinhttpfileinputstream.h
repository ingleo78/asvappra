#ifndef __G_WINHTTP_FILE_INPUT_STREAM_H__
#define __G_WINHTTP_FILE_INPUT_STREAM_H__

#include "../gfileinputstream.h"
#include "gwinhttpfile.h"

G_BEGIN_DECLS
#define G_TYPE_WINHTTP_FILE_INPUT_STREAM  (_g_winhttp_file_input_stream_get_type ())
#define G_WINHTTP_FILE_INPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_WINHTTP_FILE_INPUT_STREAM, GWinHttpFileInputStream))
#define G_WINHTTP_FILE_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_WINHTTP_FILE_INPUT_STREAM, GWinHttpFileInputStreamClass))
#define G_IS_WINHTTP_FILE_INPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_WINHTTP_FILE_INPUT_STREAM))
#define G_IS_WINHTTP_FILE_INPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_WINHTTP_FILE_INPUT_STREAM))
#define G_WINHTTP_FILE_INPUT_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), G_TYPE_WINHTTP_FILE_INPUT_STREAM, GWinHttpFileInputStreamClass))
typedef struct _GWinHttpFileInputStream GWinHttpFileInputStream;
typedef struct _GWinHttpFileInputStreamClass GWinHttpFileInputStreamClass;
GType _g_winhttp_file_input_stream_get_type(void) G_GNUC_CONST;
GFileInputStream *_g_winhttp_file_input_stream_new(GWinHttpFile *file, HINTERNET connection, HINTERNET request);
G_END_DECLS

#endif