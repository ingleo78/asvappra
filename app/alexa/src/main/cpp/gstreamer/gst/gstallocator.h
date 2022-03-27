#ifndef __GST_ALLOCATOR_H__
#define __GST_ALLOCATOR_H__

#include "gstmemory.h"
#include "gstobject.h"

G_BEGIN_DECLS
typedef struct _GstAllocatorPrivate GstAllocatorPrivate;
typedef struct _GstAllocatorClass GstAllocatorClass;
#define GST_TYPE_ALLOCATOR                 (gst_allocator_get_type())
#define GST_IS_ALLOCATOR(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_ALLOCATOR))
#define GST_IS_ALLOCATOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_ALLOCATOR))
#define GST_ALLOCATOR_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_ALLOCATOR, GstAllocatorClass))
#define GST_ALLOCATOR(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_ALLOCATOR, GstAllocator))
#define GST_ALLOCATOR_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_ALLOCATOR, GstAllocatorClass))
#define GST_ALLOCATOR_CAST(obj)            ((GstAllocator *)(obj))
#define GST_TYPE_ALLOCATION_PARAMS (gst_allocation_params_get_type())
GType gst_allocation_params_get_type(void);
typedef struct _GstAllocationParams GstAllocationParams;
GST_EXPORT gsize gst_memory_alignment;
#define GST_ALLOCATOR_SYSMEM   "SystemMemory"
struct _GstAllocationParams {
  GstMemoryFlags flags;
  gsize          align;
  gsize          prefix;
  gsize          padding;
  gpointer _gst_reserved[GST_PADDING];
};
typedef enum {
  GST_ALLOCATOR_FLAG_CUSTOM_ALLOC  = (GST_OBJECT_FLAG_LAST << 0),
  GST_ALLOCATOR_FLAG_LAST          = (GST_OBJECT_FLAG_LAST << 16)
} GstAllocatorFlags;
struct _GstAllocator {
  GstObject  object;
  const gchar               *mem_type;
  GstMemoryMapFunction       mem_map;
  GstMemoryUnmapFunction     mem_unmap;
  GstMemoryCopyFunction      mem_copy;
  GstMemoryShareFunction     mem_share;
  GstMemoryIsSpanFunction    mem_is_span;
  GstMemoryMapFullFunction   mem_map_full;
  GstMemoryUnmapFullFunction mem_unmap_full;
  gpointer _gst_reserved[GST_PADDING - 2];
  GstAllocatorPrivate *priv;
};
struct _GstAllocatorClass {
  GstObjectClass object_class;
  GstMemory *  (*alloc)      (GstAllocator *allocator, gsize size, GstAllocationParams *params);
  void         (*free)       (GstAllocator *allocator, GstMemory *memory);
  gpointer _gst_reserved[GST_PADDING];
};
GType gst_allocator_get_type(void);
void           gst_allocator_register        (const gchar *name, GstAllocator *allocator);
GstAllocator * gst_allocator_find            (const gchar *name);
void           gst_allocator_set_default     (GstAllocator * allocator);
void           gst_allocation_params_init    (GstAllocationParams *params);
GstAllocationParams *gst_allocation_params_copy    (const GstAllocationParams *params) G_GNUC_MALLOC;
void           gst_allocation_params_free    (GstAllocationParams *params);
GstMemory *    gst_allocator_alloc           (GstAllocator * allocator, gsize size, GstAllocationParams *params);
void           gst_allocator_free            (GstAllocator * allocator, GstMemory *memory);
GstMemory *    gst_memory_new_wrapped  (GstMemoryFlags flags, gpointer data, gsize maxsize, gsize offset, gsize size, gpointer user_data,
                                        GDestroyNotify notify);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstAllocationParams, gst_allocation_params_free)
#endif
G_END_DECLS

#endif