#ifndef __GST_MEMORY_H__
#define __GST_MEMORY_H__

#include <glib/glib-object.h>
#include "gstconfig.h"
#include "gstminiobject.h"

G_BEGIN_DECLS
GST_EXPORT GType _gst_memory_type;
#define GST_TYPE_MEMORY (_gst_memory_type)
GType gst_memory_get_type(void);
typedef struct _GstMemory GstMemory;
typedef struct _GstAllocator GstAllocator;
#define GST_MEMORY_CAST(mem)   ((GstMemory *)(mem))
typedef enum {
  GST_MEMORY_FLAG_READONLY      = GST_MINI_OBJECT_FLAG_LOCK_READONLY,
  GST_MEMORY_FLAG_NO_SHARE      = (GST_MINI_OBJECT_FLAG_LAST << 0),
  GST_MEMORY_FLAG_ZERO_PREFIXED = (GST_MINI_OBJECT_FLAG_LAST << 1),
  GST_MEMORY_FLAG_ZERO_PADDED   = (GST_MINI_OBJECT_FLAG_LAST << 2),
  GST_MEMORY_FLAG_PHYSICALLY_CONTIGUOUS = (GST_MINI_OBJECT_FLAG_LAST << 3),
  GST_MEMORY_FLAG_NOT_MAPPABLE  = (GST_MINI_OBJECT_FLAG_LAST << 4),
  GST_MEMORY_FLAG_LAST          = (GST_MINI_OBJECT_FLAG_LAST << 16)
} GstMemoryFlags;
#define GST_MEMORY_FLAGS(mem)  GST_MINI_OBJECT_FLAGS (mem)
#define GST_MEMORY_FLAG_IS_SET(mem,flag)   GST_MINI_OBJECT_FLAG_IS_SET (mem,flag)
#define GST_MEMORY_FLAG_UNSET(mem,flag)   GST_MINI_OBJECT_FLAG_UNSET (mem, flag)
#define GST_MEMORY_IS_READONLY(mem)        GST_MEMORY_FLAG_IS_SET(mem,GST_MEMORY_FLAG_READONLY)
#define GST_MEMORY_IS_NO_SHARE(mem)        GST_MEMORY_FLAG_IS_SET(mem,GST_MEMORY_FLAG_NO_SHARE)
#define GST_MEMORY_IS_ZERO_PREFIXED(mem)   GST_MEMORY_FLAG_IS_SET(mem,GST_MEMORY_FLAG_ZERO_PREFIXED)
#define GST_MEMORY_IS_ZERO_PADDED(mem)     GST_MEMORY_FLAG_IS_SET(mem,GST_MEMORY_FLAG_ZERO_PADDED)
#define GST_MEMORY_IS_PHYSICALLY_CONTIGUOUS(mem)     GST_MEMORY_FLAG_IS_SET(mem,GST_MEMORY_FLAG_PHYSICALLY_CONTIGUOUS)
#define GST_MEMORY_IS_NOT_MAPPABLE(mem)     GST_MEMORY_FLAG_IS_SET(mem,GST_MEMORY_FLAG_NOT_MAPPABLE)
struct _GstMemory {
  GstMiniObject   mini_object;
  GstAllocator   *allocator;
  GstMemory      *parent;
  gsize           maxsize;
  gsize           align;
  gsize           offset;
  gsize           size;
};
typedef enum {
  GST_MAP_READ      = GST_LOCK_FLAG_READ,
  GST_MAP_WRITE     = GST_LOCK_FLAG_WRITE,
  GST_MAP_FLAG_LAST = (1 << 16)
} GstMapFlags;
#define GST_MAP_READWRITE      (GST_MAP_READ | GST_MAP_WRITE)
typedef struct {
  GstMemory *memory;
  GstMapFlags flags;
  guint8 *data;
  gsize size;
  gsize maxsize;
  gpointer user_data[4];
  gpointer _gst_reserved[GST_PADDING];
} GstMapInfo;
#define GST_MAP_INFO_INIT { NULL, 0, NULL, 0, 0, {0, }, {0, }}
typedef gpointer    (*GstMemoryMapFunction)       (GstMemory *mem, gsize maxsize, GstMapFlags flags);
typedef gpointer    (*GstMemoryMapFullFunction)       (GstMemory *mem, GstMapInfo * info, gsize maxsize);
typedef void        (*GstMemoryUnmapFunction)     (GstMemory *mem);
typedef void        (*GstMemoryUnmapFullFunction)     (GstMemory *mem, GstMapInfo * info);
typedef GstMemory * (*GstMemoryCopyFunction)      (GstMemory *mem, gssize offset, gssize size);
typedef GstMemory * (*GstMemoryShareFunction)     (GstMemory *mem, gssize offset, gssize size);
typedef gboolean    (*GstMemoryIsSpanFunction)    (GstMemory *mem1, GstMemory *mem2, gsize *offset);
void           gst_memory_init(GstMemory *mem, GstMemoryFlags flags, GstAllocator *allocator, GstMemory *parent, gsize maxsize, gsize align,
                                        gsize offset, gsize size);
gboolean       gst_memory_is_type      (GstMemory *mem, const gchar *mem_type);
static inline GstMemory *gst_memory_ref (GstMemory * memory) {
  return (GstMemory *) gst_mini_object_ref (GST_MINI_OBJECT_CAST (memory));
}
static inline void gst_memory_unref (GstMemory * memory) {
  gst_mini_object_unref (GST_MINI_OBJECT_CAST (memory));
}
gsize          gst_memory_get_sizes    (GstMemory *mem, gsize *offset, gsize *maxsize);
void           gst_memory_resize       (GstMemory *mem, gssize offset, gsize size);
#define        gst_memory_lock(m,f)        gst_mini_object_lock (GST_MINI_OBJECT_CAST (m), (f))
#define        gst_memory_unlock(m,f)      gst_mini_object_unlock (GST_MINI_OBJECT_CAST (m), (f))
#define        gst_memory_is_writable(m)   gst_mini_object_is_writable (GST_MINI_OBJECT_CAST (m))
#define        gst_memory_make_writable(m) GST_MEMORY_CAST (gst_mini_object_make_writable (GST_MINI_OBJECT_CAST (m)))
GstMemory *    gst_memory_make_mapped  (GstMemory *mem, GstMapInfo *info, GstMapFlags flags);
gboolean       gst_memory_map          (GstMemory *mem, GstMapInfo *info, GstMapFlags flags);
void           gst_memory_unmap        (GstMemory *mem, GstMapInfo *info);
GstMemory *    gst_memory_copy         (GstMemory *mem, gssize offset, gssize size);
GstMemory *    gst_memory_share        (GstMemory *mem, gssize offset, gssize size);
gboolean       gst_memory_is_span      (GstMemory *mem1, GstMemory *mem2, gsize *offset);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstMemory, gst_memory_unref)
#endif
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstAllocator, gst_object_unref)
#endif
G_END_DECLS

#endif