#ifndef __G_LOCAL_FILE_OUTPUT_STREAM_H__
#define __G_LOCAL_FILE_OUTPUT_STREAM_H__

#include "gfileoutputstream.h"

G_BEGIN_DECLS
#define G_TYPE_LOCAL_FILE_OUTPUT_STREAM  (_g_local_file_output_stream_get_type())
#define G_LOCAL_FILE_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_LOCAL_FILE_OUTPUT_STREAM, GLocalFileOutputStream))
#define G_LOCAL_FILE_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_LOCAL_FILE_OUTPUT_STREAM, GLocalFileOutputStreamClass))
#define G_IS_LOCAL_FILE_OUTPUT_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_LOCAL_FILE_OUTPUT_STREAM))
#define G_IS_LOCAL_FILE_OUTPUT_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_LOCAL_FILE_OUTPUT_STREAM))
#define G_LOCAL_FILE_OUTPUT_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_LOCAL_FILE_OUTPUT_STREAM, GLocalFileOutputStreamClass))
typedef struct _GLocalFileOutputStream GLocalFileOutputStream;
typedef struct _GLocalFileOutputStreamClass GLocalFileOutputStreamClass;
typedef struct _GLocalFileOutputStreamPrivate GLocalFileOutputStreamPrivate;
struct _GLocalFileOutputStream {
  GFileOutputStream parent_instance;
  GLocalFileOutputStreamPrivate *priv;
};
struct _GLocalFileOutputStreamClass {
  GFileOutputStreamClass parent_class;
};
GType _g_local_file_output_stream_get_type(void) G_GNUC_CONST;
void _g_local_file_output_stream_set_do_close(GLocalFileOutputStream *out, gboolean do_close);
gboolean _g_local_file_output_stream_really_close(GLocalFileOutputStream *out, GCancellable *cancellable, GError **error);
GFileOutputStream *_g_local_file_output_stream_open(const char *filename, gboolean readable, GCancellable *cancellable, GError **error);
GFileOutputStream *_g_local_file_output_stream_create(const char *filename, gboolean readable, GFileCreateFlags flags, GCancellable *cancellable, GError **error);
GFileOutputStream *_g_local_file_output_stream_append(const char *filename, GFileCreateFlags flags, GCancellable *cancellable, GError **error);
GFileOutputStream *_g_local_file_output_stream_replace(const char *filename, gboolean readable, const char *etag, gboolean create_backup, GFileCreateFlags flags,
                                                       GCancellable *cancellable, GError **error);
gint _g_local_file_output_stream_get_fd(GLocalFileOutputStream *output_stream);
G_END_DECLS

#endif