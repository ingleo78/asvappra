#ifndef __GST_BUFFER_POOL_H__
#define __GST_BUFFER_POOL_H__

#include "gstminiobject.h"
#include "gstpad.h"
#include "gstbuffer.h"

G_BEGIN_DECLS
typedef struct _GstBufferPoolPrivate GstBufferPoolPrivate;
typedef struct _GstBufferPoolClass GstBufferPoolClass;
#define GST_TYPE_BUFFER_POOL                 (gst_buffer_pool_get_type())
#define GST_IS_BUFFER_POOL(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_BUFFER_POOL))
#define GST_IS_BUFFER_POOL_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_BUFFER_POOL))
#define GST_BUFFER_POOL_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_BUFFER_POOL, GstBufferPoolClass))
#define GST_BUFFER_POOL(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_BUFFER_POOL, GstBufferPool))
#define GST_BUFFER_POOL_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_BUFFER_POOL, GstBufferPoolClass))
#define GST_BUFFER_POOL_CAST(obj)            ((GstBufferPool *)(obj))
typedef enum {
  GST_BUFFER_POOL_ACQUIRE_FLAG_NONE     = 0,
  GST_BUFFER_POOL_ACQUIRE_FLAG_KEY_UNIT = (1 << 0),
  GST_BUFFER_POOL_ACQUIRE_FLAG_DONTWAIT = (1 << 1),
  GST_BUFFER_POOL_ACQUIRE_FLAG_DISCONT  = (1 << 2),
  GST_BUFFER_POOL_ACQUIRE_FLAG_LAST     = (1 << 16),
} GstBufferPoolAcquireFlags;
typedef struct _GstBufferPoolAcquireParams GstBufferPoolAcquireParams;
struct _GstBufferPoolAcquireParams {
  GstFormat                 format;
  gint64                    start;
  gint64                    stop;
  GstBufferPoolAcquireFlags flags;
  gpointer _gst_reserved[GST_PADDING];
};
#define GST_BUFFER_POOL_IS_FLUSHING(pool)  (g_atomic_int_get (&pool->flushing))
struct _GstBufferPool {
  GstObject            object;
  gint                 flushing;
  GstBufferPoolPrivate *priv;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstBufferPoolClass {
  GstObjectClass    object_class;
  const gchar ** (*get_options)    (GstBufferPool *pool);
  gboolean       (*set_config)     (GstBufferPool *pool, GstStructure *config);
  gboolean       (*start)          (GstBufferPool *pool);
  gboolean       (*stop)           (GstBufferPool *pool);
  GstFlowReturn  (*acquire_buffer) (GstBufferPool *pool, GstBuffer **buffer, GstBufferPoolAcquireParams *params);
  GstFlowReturn  (*alloc_buffer)   (GstBufferPool *pool, GstBuffer **buffer, GstBufferPoolAcquireParams *params);
  void           (*reset_buffer)   (GstBufferPool *pool, GstBuffer *buffer);
  void           (*release_buffer) (GstBufferPool *pool, GstBuffer *buffer);
  void           (*free_buffer)    (GstBufferPool *pool, GstBuffer *buffer);
  void           (*flush_start)    (GstBufferPool *pool);
  void           (*flush_stop)     (GstBufferPool *pool);
  gpointer _gst_reserved[GST_PADDING - 2];
};
GType       gst_buffer_pool_get_type (void);
GstBufferPool *  gst_buffer_pool_new  (void);
gboolean         gst_buffer_pool_set_active      (GstBufferPool *pool, gboolean active);
gboolean         gst_buffer_pool_is_active       (GstBufferPool *pool);
gboolean         gst_buffer_pool_set_config      (GstBufferPool *pool, GstStructure *config);
GstStructure *   gst_buffer_pool_get_config      (GstBufferPool *pool);
const gchar **   gst_buffer_pool_get_options     (GstBufferPool *pool);
gboolean         gst_buffer_pool_has_option      (GstBufferPool *pool, const gchar *option);
void             gst_buffer_pool_set_flushing    (GstBufferPool *pool, gboolean flushing);
void             gst_buffer_pool_config_set_params    (GstStructure *config, GstCaps *caps, guint size, guint min_buffers, guint max_buffers);
gboolean         gst_buffer_pool_config_get_params(GstStructure *config, GstCaps **caps, guint *size, guint *min_buffers, guint *max_buffers);
void             gst_buffer_pool_config_set_allocator (GstStructure *config, GstAllocator *allocator, const GstAllocationParams *params);
gboolean         gst_buffer_pool_config_get_allocator (GstStructure *config, GstAllocator **allocator, GstAllocationParams *params);
guint            gst_buffer_pool_config_n_options   (GstStructure *config);
void             gst_buffer_pool_config_add_option  (GstStructure *config, const gchar *option);
const gchar *    gst_buffer_pool_config_get_option  (GstStructure *config, guint index);
gboolean         gst_buffer_pool_config_has_option  (GstStructure *config, const gchar *option);
gboolean gst_buffer_pool_config_validate_params(GstStructure *config, GstCaps *caps, guint size, guint min_buffers, guint max_buffers);
GstFlowReturn    gst_buffer_pool_acquire_buffer  (GstBufferPool *pool, GstBuffer **buffer, GstBufferPoolAcquireParams *params);
void             gst_buffer_pool_release_buffer  (GstBufferPool *pool, GstBuffer *buffer);
G_END_DECLS

#endif