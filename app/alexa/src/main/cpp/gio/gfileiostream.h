#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_FILE_IO_STREAM_H__
#define __G_FILE_IO_STREAM_H__

#include "giostream.h"

G_BEGIN_DECLS
#define G_TYPE_FILE_IO_STREAM  (g_file_io_stream_get_type())
#define G_FILE_IO_STREAM(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_FILE_IO_STREAM, GFileIOStream))
#define G_FILE_IO_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_FILE_IO_STREAM, GFileIOStreamClass))
#define G_IS_FILE_IO_STREAM(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_FILE_IO_STREAM))
#define G_IS_FILE_IO_STREAM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_FILE_IO_STREAM))
#define G_FILE_IO_STREAM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_FILE_IO_STREAM, GFileIOStreamClass))
typedef struct _GFileIOStreamClass GFileIOStreamClass;
typedef struct _GFileIOStreamPrivate GFileIOStreamPrivate;
struct _GFileIOStream {
  GIOStream parent_instance;
  GFileIOStreamPrivate *priv;
};
struct _GFileIOStreamClass {
  GIOStreamClass parent_class;
  goffset (*tell)(GFileIOStream *stream);
  gboolean (*can_seek)(GFileIOStream *stream);
  gboolean (*seek)(GFileIOStream *stream, goffset offset, GSeekType type, GCancellable *cancellable, GError **error);
  gboolean (*can_truncate)(GFileIOStream *stream);
  gboolean (*truncate_fn)(GFileIOStream *stream, goffset size, GCancellable *cancellable, GError **error);
  GFileInfo *(*query_info)(GFileIOStream *stream, const char *attributes, GCancellable *cancellable, GError **error);
  void (*query_info_async)(GFileIOStream *stream, const char *attributes, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  GFileInfo *(*query_info_finish)(GFileIOStream *stream, GAsyncResult *result, GError **error);
  char *(*get_etag)(GFileIOStream *stream);
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
GType g_file_io_stream_get_type(void) G_GNUC_CONST;
GFileInfo *g_file_io_stream_query_info(GFileIOStream *stream, const char *attributes, GCancellable *cancellable, GError **error);
void g_file_io_stream_query_info_async(GFileIOStream *stream, const char *attributes, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
					       			   gpointer user_data);
GFileInfo *g_file_io_stream_query_info_finish(GFileIOStream *stream, GAsyncResult *result, GError **error);
char *g_file_io_stream_get_etag(GFileIOStream *stream);
G_END_DECLS

#endif