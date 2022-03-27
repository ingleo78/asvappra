#ifndef __GST_BUFFER_H__
#define __GST_BUFFER_H__

#include "gstminiobject.h"
#include "gstclock.h"
#include "gstallocator.h"

G_BEGIN_DECLS
GST_EXPORT GType _gst_buffer_type;
typedef struct _GstBuffer GstBuffer;
typedef struct _GstBufferPool GstBufferPool;
#define GST_TYPE_BUFFER                         (_gst_buffer_type)
#define GST_IS_BUFFER(obj)                      (GST_IS_MINI_OBJECT_TYPE(obj, GST_TYPE_BUFFER))
#define GST_BUFFER_CAST(obj)                    ((GstBuffer *)(obj))
#define GST_BUFFER(obj)                         (GST_BUFFER_CAST(obj))
#define GST_BUFFER_FLAGS(buf)                   GST_MINI_OBJECT_FLAGS(buf)
#define GST_BUFFER_FLAG_IS_SET(buf,flag)        GST_MINI_OBJECT_FLAG_IS_SET (buf, flag)
#define GST_BUFFER_FLAG_SET(buf,flag)           GST_MINI_OBJECT_FLAG_SET (buf, flag)
#define GST_BUFFER_FLAG_UNSET(buf,flag)         GST_MINI_OBJECT_FLAG_UNSET (buf, flag)
#define GST_BUFFER_PTS(buf)                     (GST_BUFFER_CAST(buf)->pts)
#define GST_BUFFER_DTS(buf)                     (GST_BUFFER_CAST(buf)->dts)
#define GST_BUFFER_DTS_OR_PTS(buf)              (GST_BUFFER_DTS_IS_VALID(buf) ? GST_BUFFER_DTS(buf) : GST_BUFFER_PTS (buf))
#define GST_BUFFER_DURATION(buf)                (GST_BUFFER_CAST(buf)->duration)
#define GST_BUFFER_OFFSET(buf)                  (GST_BUFFER_CAST(buf)->offset)
#define GST_BUFFER_OFFSET_END(buf)              (GST_BUFFER_CAST(buf)->offset_end)
#define GST_BUFFER_OFFSET_NONE  ((guint64)-1)
#define GST_BUFFER_DURATION_IS_VALID(buffer)    (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_DURATION (buffer)))
#define GST_BUFFER_PTS_IS_VALID(buffer)   (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_PTS (buffer)))
#define GST_BUFFER_DTS_IS_VALID(buffer)   (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_DTS (buffer)))
#define GST_BUFFER_OFFSET_IS_VALID(buffer)      (GST_BUFFER_OFFSET (buffer) != GST_BUFFER_OFFSET_NONE)
#define GST_BUFFER_OFFSET_END_IS_VALID(buffer)  (GST_BUFFER_OFFSET_END (buffer) != GST_BUFFER_OFFSET_NONE)
#define GST_BUFFER_IS_DISCONT(buffer)   (GST_BUFFER_FLAG_IS_SET (buffer, GST_BUFFER_FLAG_DISCONT))
typedef enum {
  GST_BUFFER_FLAG_LIVE        = (GST_MINI_OBJECT_FLAG_LAST << 0),
  GST_BUFFER_FLAG_DECODE_ONLY = (GST_MINI_OBJECT_FLAG_LAST << 1),
  GST_BUFFER_FLAG_DISCONT     = (GST_MINI_OBJECT_FLAG_LAST << 2),
  GST_BUFFER_FLAG_RESYNC      = (GST_MINI_OBJECT_FLAG_LAST << 3),
  GST_BUFFER_FLAG_CORRUPTED   = (GST_MINI_OBJECT_FLAG_LAST << 4),
  GST_BUFFER_FLAG_MARKER      = (GST_MINI_OBJECT_FLAG_LAST << 5),
  GST_BUFFER_FLAG_HEADER      = (GST_MINI_OBJECT_FLAG_LAST << 6),
  GST_BUFFER_FLAG_GAP         = (GST_MINI_OBJECT_FLAG_LAST << 7),
  GST_BUFFER_FLAG_DROPPABLE   = (GST_MINI_OBJECT_FLAG_LAST << 8),
  GST_BUFFER_FLAG_DELTA_UNIT  = (GST_MINI_OBJECT_FLAG_LAST << 9),
  GST_BUFFER_FLAG_TAG_MEMORY  = (GST_MINI_OBJECT_FLAG_LAST << 10),
  GST_BUFFER_FLAG_SYNC_AFTER  = (GST_MINI_OBJECT_FLAG_LAST << 11),
  GST_BUFFER_FLAG_LAST        = (GST_MINI_OBJECT_FLAG_LAST << 16)
} GstBufferFlags;
struct _GstBuffer {
  GstMiniObject          mini_object;
  GstBufferPool         *pool;
  GstClockTime           pts;
  GstClockTime           dts;
  GstClockTime           duration;
  guint64                offset;
  guint64                offset_end;
};
GType       gst_buffer_get_type            (void);
guint       gst_buffer_get_max_memory      (void);
GstBuffer * gst_buffer_new                 (void);
GstBuffer * gst_buffer_new_allocate        (GstAllocator * allocator, gsize size,
                                            GstAllocationParams * params);
GstBuffer * gst_buffer_new_wrapped_full    (GstMemoryFlags flags, gpointer data, gsize maxsize,
                                            gsize offset, gsize size, gpointer user_data,
                                            GDestroyNotify notify);
GstBuffer * gst_buffer_new_wrapped         (gpointer data, gsize size);
guint       gst_buffer_n_memory             (GstBuffer *buffer);
void        gst_buffer_insert_memory        (GstBuffer *buffer, gint idx, GstMemory *mem);
void        gst_buffer_replace_memory_range (GstBuffer *buffer, guint idx, gint length, GstMemory *mem);
GstMemory * gst_buffer_peek_memory          (GstBuffer *buffer, guint idx);
GstMemory * gst_buffer_get_memory_range     (GstBuffer *buffer, guint idx, gint length);
void        gst_buffer_remove_memory_range  (GstBuffer *buffer, guint idx, gint length);
void        gst_buffer_prepend_memory       (GstBuffer *buffer, GstMemory *mem);
void        gst_buffer_append_memory        (GstBuffer *buffer, GstMemory *mem);
void        gst_buffer_replace_memory       (GstBuffer *buffer, guint idx, GstMemory *mem);
void        gst_buffer_replace_all_memory   (GstBuffer *buffer, GstMemory *mem);
GstMemory * gst_buffer_get_memory           (GstBuffer *buffer, guint idx);
GstMemory * gst_buffer_get_all_memory       (GstBuffer *buffer);
void        gst_buffer_remove_memory        (GstBuffer *buffer, guint idx);
void        gst_buffer_remove_all_memory    (GstBuffer *buffer);
gboolean    gst_buffer_find_memory         (GstBuffer *buffer, gsize offset, gsize size, guint *idx, guint *length, gsize *skip);
gboolean    gst_buffer_is_memory_range_writable  (GstBuffer *buffer, guint idx, gint length);
gboolean    gst_buffer_is_all_memory_writable    (GstBuffer *buffer);
gsize       gst_buffer_fill                (GstBuffer *buffer, gsize offset, gconstpointer src, gsize size);
gsize       gst_buffer_extract             (GstBuffer *buffer, gsize offset, gpointer dest, gsize size);
gint        gst_buffer_memcmp              (GstBuffer *buffer, gsize offset, gconstpointer mem, gsize size);
gsize       gst_buffer_memset              (GstBuffer *buffer, gsize offset, guint8 val, gsize size);
gsize       gst_buffer_get_sizes_range     (GstBuffer *buffer, guint idx, gint length, gsize *offset, gsize *maxsize);
gboolean    gst_buffer_resize_range        (GstBuffer *buffer, guint idx, gint length, gssize offset, gssize size);
gsize       gst_buffer_get_sizes           (GstBuffer *buffer, gsize *offset, gsize *maxsize);
gsize       gst_buffer_get_size            (GstBuffer *buffer);
void        gst_buffer_resize              (GstBuffer *buffer, gssize offset, gssize size);
void        gst_buffer_set_size            (GstBuffer *buffer, gssize size);
gboolean    gst_buffer_map_range           (GstBuffer *buffer, guint idx, gint length, GstMapInfo *info, GstMapFlags flags);
gboolean    gst_buffer_map                 (GstBuffer *buffer, GstMapInfo *info, GstMapFlags flags);
void        gst_buffer_unmap               (GstBuffer *buffer, GstMapInfo *info);
void        gst_buffer_extract_dup         (GstBuffer *buffer, gsize offset, gsize size, gpointer *dest, gsize *dest_size);
static inline GstBuffer *gst_buffer_ref (GstBuffer * buf) {
  return (GstBuffer *) gst_mini_object_ref (GST_MINI_OBJECT_CAST (buf));
}
static inline void gst_buffer_unref (GstBuffer * buf) {
  gst_mini_object_unref (GST_MINI_OBJECT_CAST (buf));
}
static inline GstBuffer *gst_buffer_copy (const GstBuffer * buf) {
  return GST_BUFFER (gst_mini_object_copy (GST_MINI_OBJECT_CONST_CAST (buf)));
}
GstBuffer * gst_buffer_copy_deep (const GstBuffer * buf);
typedef enum {
  GST_BUFFER_COPY_NONE           = 0,
  GST_BUFFER_COPY_FLAGS          = (1 << 0),
  GST_BUFFER_COPY_TIMESTAMPS     = (1 << 1),
  GST_BUFFER_COPY_META           = (1 << 2),
  GST_BUFFER_COPY_MEMORY         = (1 << 3),
  GST_BUFFER_COPY_MERGE          = (1 << 4),
  GST_BUFFER_COPY_DEEP           = (1 << 5)
} GstBufferCopyFlags;
#define GST_BUFFER_COPY_METADATA       (GST_BUFFER_COPY_FLAGS | GST_BUFFER_COPY_TIMESTAMPS | GST_BUFFER_COPY_META)
#define GST_BUFFER_COPY_ALL  ((GstBufferCopyFlags)(GST_BUFFER_COPY_METADATA | GST_BUFFER_COPY_MEMORY))
gboolean        gst_buffer_copy_into            (GstBuffer *dest, GstBuffer *src, GstBufferCopyFlags flags, gsize offset, gsize size);
#define         gst_buffer_is_writable(buf)     gst_mini_object_is_writable (GST_MINI_OBJECT_CAST (buf))
#define         gst_buffer_make_writable(buf)   GST_BUFFER_CAST (gst_mini_object_make_writable (GST_MINI_OBJECT_CAST (buf)))
static inline gboolean gst_buffer_replace (GstBuffer **obuf, GstBuffer *nbuf) {
  return gst_mini_object_replace ((GstMiniObject **) obuf, (GstMiniObject *) nbuf);
}
GstBuffer*      gst_buffer_copy_region          (GstBuffer *parent, GstBufferCopyFlags flags, gsize offset, gsize size);
GstBuffer*      gst_buffer_append_region        (GstBuffer *buf1, GstBuffer *buf2, gssize offset, gssize size);
GstBuffer*      gst_buffer_append               (GstBuffer *buf1, GstBuffer *buf2);
#include "gstmeta.h"
typedef gboolean (*GstBufferForeachMetaFunc)    (GstBuffer *buffer, GstMeta **meta, gpointer user_data);
GstMeta *       gst_buffer_get_meta             (GstBuffer *buffer, GType api);
GstMeta *       gst_buffer_add_meta             (GstBuffer *buffer, const GstMetaInfo *info, gpointer params);
gboolean        gst_buffer_remove_meta          (GstBuffer *buffer, GstMeta *meta);
GstMeta *       gst_buffer_iterate_meta         (GstBuffer *buffer, gpointer *state);
gboolean        gst_buffer_foreach_meta         (GstBuffer *buffer, GstBufferForeachMetaFunc func, gpointer user_data);
#define         gst_value_set_buffer(v,b)       g_value_set_boxed((v),(b))
#define         gst_value_take_buffer(v,b)      g_value_take_boxed(v,(b))
#define         gst_value_get_buffer(v)         GST_BUFFER_CAST (g_value_get_boxed(v))
typedef struct _GstParentBufferMeta GstParentBufferMeta;
struct _GstParentBufferMeta {
  GstMeta parent;
  GstBuffer *buffer;
};
GType gst_parent_buffer_meta_api_get_type (void);
#ifndef GST_DISABLE_DEPRECATED
#define GST_TYPE_PARENT_BUFFER_META_API_TYPE GST_PARENT_BUFFER_META_API_TYPE
#endif
#define GST_PARENT_BUFFER_META_API_TYPE (gst_parent_buffer_meta_api_get_type())
#define gst_buffer_get_parent_buffer_meta(b)  ((GstParentBufferMeta*)gst_buffer_get_meta((b),GST_PARENT_BUFFER_META_API_TYPE))
const GstMetaInfo *gst_parent_buffer_meta_get_info (void);
#define GST_PARENT_BUFFER_META_INFO (gst_parent_buffer_meta_get_info())
GstParentBufferMeta *gst_buffer_add_parent_buffer_meta (GstBuffer *buffer, GstBuffer *ref);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstBuffer, gst_buffer_unref)
#endif
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstBufferPool, gst_object_unref)
#endif
G_END_DECLS

#endif