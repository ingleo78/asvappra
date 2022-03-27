#ifndef __GST_META_H__
#define __GST_META_H__

#include <glib/glib.h>
#include "gstbuffer.h"

G_BEGIN_DECLS
typedef struct _GstMeta GstMeta;
typedef struct _GstMetaInfo GstMetaInfo;
#define GST_META_CAST(meta)   ((GstMeta *)(meta))
typedef enum {
  GST_META_FLAG_NONE        = 0,
  GST_META_FLAG_READONLY    = (1 << 0),
  GST_META_FLAG_POOLED      = (1 << 1),
  GST_META_FLAG_LOCKED      = (1 << 2),
  GST_META_FLAG_LAST        = (1 << 16)
} GstMetaFlags;
#define GST_META_FLAGS(meta)  (GST_META_CAST (meta)->flags)
#define GST_META_FLAG_IS_SET(meta,flag)        !!(GST_META_FLAGS (meta) & (flag))
#define GST_META_FLAG_SET(meta,flag)           (GST_META_FLAGS (meta) |= (flag))
#define GST_META_FLAG_UNSET(meta,flag)         (GST_META_FLAGS (meta) &= ~(flag))
#define GST_META_TAG_MEMORY_STR "memory"
struct _GstMeta {
  GstMetaFlags       flags;
  const GstMetaInfo *info;
};
typedef gboolean (*GstMetaInitFunction) (GstMeta *meta, gpointer params, GstBuffer *buffer);
typedef void (*GstMetaFreeFunction)     (GstMeta *meta, GstBuffer *buffer);
GST_EXPORT GQuark _gst_meta_transform_copy;
#define GST_META_TRANSFORM_IS_COPY(type) ((type) == _gst_meta_transform_copy)
typedef struct {
  gboolean region;
  gsize offset;
  gsize size;
} GstMetaTransformCopy;
typedef gboolean (*GstMetaTransformFunction) (GstBuffer *transbuf, GstMeta *meta, GstBuffer *buffer, GQuark type, gpointer data);
struct _GstMetaInfo {
  GType                      api;
  GType                      type;
  gsize                      size;
  GstMetaInitFunction        init_func;
  GstMetaFreeFunction        free_func;
  GstMetaTransformFunction   transform_func;
  gpointer _gst_reserved[GST_PADDING];
};
GType                gst_meta_api_type_register (const gchar *api, const gchar **tags);
gboolean             gst_meta_api_type_has_tag  (GType api, GQuark tag);
const GstMetaInfo *gst_meta_register(GType api, const gchar *impl, gsize size, GstMetaInitFunction init_func, GstMetaFreeFunction free_func,
                                     GstMetaTransformFunction transform_func);
const GstMetaInfo *  gst_meta_get_info          (const gchar * impl);
const gchar* const*  gst_meta_api_type_get_tags (GType api);
GST_EXPORT GQuark _gst_meta_tag_memory;
#ifndef GST_DISABLE_DEPRECATED
#define GST_META_TAG_MEMORY (_gst_meta_tag_memory)
#endif
G_END_DECLS

#endif