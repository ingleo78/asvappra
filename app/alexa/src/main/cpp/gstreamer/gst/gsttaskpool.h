#ifndef __GST_TASK_POOL_H__
#define __GST_TASK_POOL_H__

#include "gstobject.h"

G_BEGIN_DECLS
#define GST_TYPE_TASK_POOL             (gst_task_pool_get_type ())
#define GST_TASK_POOL(pool)            (G_TYPE_CHECK_INSTANCE_CAST ((pool), GST_TYPE_TASK_POOL, GstTaskPool))
#define GST_IS_TASK_POOL(pool)         (G_TYPE_CHECK_INSTANCE_TYPE ((pool), GST_TYPE_TASK_POOL))
#define GST_TASK_POOL_CLASS(pclass)    (G_TYPE_CHECK_CLASS_CAST ((pclass), GST_TYPE_TASK_POOL, GstTaskPoolClass))
#define GST_IS_TASK_POOL_CLASS(pclass) (G_TYPE_CHECK_CLASS_TYPE ((pclass), GST_TYPE_TASK_POOL))
#define GST_TASK_POOL_GET_CLASS(pool)  (G_TYPE_INSTANCE_GET_CLASS ((pool), GST_TYPE_TASK_POOL, GstTaskPoolClass))
#define GST_TASK_POOL_CAST(pool)       ((GstTaskPool*)(pool))
typedef struct _GstTaskPool GstTaskPool;
typedef struct _GstTaskPoolClass GstTaskPoolClass;
typedef struct _GstTaskPoolPrivate GstTaskPoolPrivate;
typedef void   (*GstTaskPoolFunction)          (void *user_data);
struct _GstTaskPool {
  GstObject      object;
  GThreadPool   *pool;
  GstTaskPoolPrivate *priv;
  gpointer _gst_reserved[GST_PADDING-1];
};
struct _GstTaskPoolClass {
  GstObjectClass parent_class;
  void      (*prepare)  (GstTaskPool *pool, GError **error);
  void      (*cleanup)  (GstTaskPool *pool);
  gpointer  (*push)     (GstTaskPool *pool, GstTaskPoolFunction func, gpointer user_data, GError **error);
  void      (*join)     (GstTaskPool *pool, gpointer id);
  gpointer _gst_reserved[GST_PADDING];
};
GType           gst_task_pool_get_type    (void);
GstTaskPool *   gst_task_pool_new         (void);
GstTaskPool *   gst_task_pool_new_full    (gint max_threads, gboolean exclusive);
void            gst_task_pool_prepare     (GstTaskPool *pool, GError **error);
gpointer        gst_task_pool_push        (GstTaskPool *pool, GstTaskPoolFunction func, gpointer user_data, GError **error);
void            gst_task_pool_join        (GstTaskPool *pool, gpointer id);
void            gst_task_pool_cleanup     (GstTaskPool *pool);
GstTaskPool *   gst_task_pool_get_default (void);
gboolean        gst_task_pool_need_schedule_thread (GstTaskPool *pool, gboolean needed);
GMainContext *  gst_task_pool_get_schedule_context (GstTaskPool *pool);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstTaskPool, gst_object_unref)
#endif
G_END_DECLS

#endif