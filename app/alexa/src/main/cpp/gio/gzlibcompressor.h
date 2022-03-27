#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_ZLIB_COMPRESSOR_H__
#define __G_ZLIB_COMPRESSOR_H__

#include "gconverter.h"
#include "gfileinfo.h"

G_BEGIN_DECLS
#define G_TYPE_ZLIB_COMPRESSOR  (g_zlib_compressor_get_type())
#define G_ZLIB_COMPRESSOR(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_ZLIB_COMPRESSOR, GZlibCompressor))
#define G_ZLIB_COMPRESSOR_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_ZLIB_COMPRESSOR, GZlibCompressorClass))
#define G_IS_ZLIB_COMPRESSOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_ZLIB_COMPRESSOR))
#define G_IS_ZLIB_COMPRESSOR_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_ZLIB_COMPRESSOR))
#define G_ZLIB_COMPRESSOR_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_ZLIB_COMPRESSOR, GZlibCompressorClass))
typedef struct _GZlibCompressorClass   GZlibCompressorClass;
struct _GZlibCompressorClass {
  GObjectClass parent_class;
};
GType g_zlib_compressor_get_type(void) G_GNUC_CONST;
GZlibCompressor *g_zlib_compressor_new(GZlibCompressorFormat format, int level);
GFileInfo *g_zlib_compressor_get_file_info(GZlibCompressor *compressor);
void g_zlib_compressor_set_file_info(GZlibCompressor *compressor, GFileInfo *file_info);
G_END_DECLS

#endif