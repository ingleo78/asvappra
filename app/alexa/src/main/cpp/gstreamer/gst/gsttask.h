#ifndef __GST_TASK_H__
#define __GST_TASK_H__

#include "gstobject.h"
#include "gsttaskpool.h"

G_BEGIN_DECLS
typedef void         (*GstTaskFunction)          (gpointer user_data);
#define GST_TYPE_TASK                   (gst_task_get_type ())
#define GST_TASK(task)                  (G_TYPE_CHECK_INSTANCE_CAST ((task), GST_TYPE_TASK, GstTask))
#define GST_IS_TASK(task)               (G_TYPE_CHECK_INSTANCE_TYPE ((task), GST_TYPE_TASK))
#define GST_TASK_CLASS(tclass)          (G_TYPE_CHECK_CLASS_CAST ((tclass), GST_TYPE_TASK, GstTaskClass))
#define GST_IS_TASK_CLASS(tclass)       (G_TYPE_CHECK_CLASS_TYPE ((tclass), GST_TYPE_TASK))
#define GST_TASK_GET_CLASS(task)        (G_TYPE_INSTANCE_GET_CLASS ((task), GST_TYPE_TASK, GstTaskClass))
#define GST_TASK_CAST(task)             ((GstTask*)(task))
typedef struct _GstTask GstTask;
typedef struct _GstTaskClass GstTaskClass;
typedef struct _GstTaskPrivate GstTaskPrivate;
typedef enum {
  GST_TASK_STARTED,
  GST_TASK_STOPPED,
  GST_TASK_PAUSED
} GstTaskState;
#define GST_TASK_STATE(task)            (GST_TASK_CAST(task)->state)
#define GST_TASK_GET_COND(task)         (&GST_TASK_CAST(task)->cond)
#define GST_TASK_WAIT(task)             g_cond_wait(GST_TASK_GET_COND (task), GST_OBJECT_GET_LOCK (task))
#define GST_TASK_SIGNAL(task)           g_cond_signal(GST_TASK_GET_COND (task))
#define GST_TASK_BROADCAST(task)        g_cond_broadcast(GST_TASK_GET_COND (task))
#define GST_TASK_GET_LOCK(task)         (GST_TASK_CAST(task)->lock)
typedef void (*GstTaskThreadFunc) (GstTask *task, GThread *thread, gpointer user_data);
struct _GstTask {
  GstObject      object;
  GstTaskState     state;
  GCond            cond;
  GRecMutex       *lock;
  GstTaskFunction  func;
  gpointer         user_data;
  GDestroyNotify   notify;
  gboolean         running;
  GThread         *thread;
  GstTaskPrivate  *priv;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstTaskClass {
  GstObjectClass parent_class;
  GstTaskPool *pool;
  gpointer _gst_reserved[GST_PADDING];
};
void            gst_task_cleanup_all    (void);
GType           gst_task_get_type       (void);
GstTask*        gst_task_new            (GstTaskFunction func, gpointer user_data, GDestroyNotify notify);
void            gst_task_set_lock       (GstTask *task, GRecMutex *mutex);
GstTaskPool *   gst_task_get_pool       (GstTask *task);
void            gst_task_set_pool       (GstTask *task, GstTaskPool *pool);
void            gst_task_set_enter_callback  (GstTask *task, GstTaskThreadFunc enter_func, gpointer user_data, GDestroyNotify notify);
void            gst_task_set_leave_callback  (GstTask *task, GstTaskThreadFunc leave_func, gpointer user_data, GDestroyNotify notify);
GstTaskState    gst_task_get_state      (GstTask *task);
gboolean        gst_task_set_state      (GstTask *task, GstTaskState state);
gboolean        gst_task_start          (GstTask *task);
gboolean        gst_task_stop           (GstTask *task);
gboolean        gst_task_pause          (GstTask *task);
gboolean        gst_task_join           (GstTask *task);
gboolean        gst_task_get_scheduleable(GstTask *task);
gboolean        gst_task_set_scheduleable(GstTask *task, gboolean scheduleable);
void            gst_task_schedule       (GstTask *task);
void            gst_task_unschedule     (GstTask *task);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstTask, gst_object_unref)
#endif
G_END_DECLS

#endif