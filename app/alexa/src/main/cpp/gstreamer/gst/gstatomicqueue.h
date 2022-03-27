#include <glib/glib.h>
#include <glib/glib-object.h>

#ifndef __GST_ATOMIC_QUEUE_H__
#define __GST_ATOMIC_QUEUE_H__

G_BEGIN_DECLS

#define GST_TYPE_ATOMIC_QUEUE (gst_atomic_queue_get_type())
typedef struct _GstAtomicQueue GstAtomicQueue;
gsize              gst_atomic_queue_get_type    (void);
GstAtomicQueue *   gst_atomic_queue_new         (guint initial_size) G_GNUC_MALLOC;
void               gst_atomic_queue_ref         (GstAtomicQueue * queue);
void               gst_atomic_queue_unref       (GstAtomicQueue * queue);
void               gst_atomic_queue_push        (GstAtomicQueue* queue, gpointer data);
gpointer           gst_atomic_queue_pop         (GstAtomicQueue* queue);
gpointer           gst_atomic_queue_peek        (GstAtomicQueue* queue);
guint              gst_atomic_queue_length      (GstAtomicQueue * queue);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstAtomicQueue, gst_atomic_queue_unref)
#endif
G_END_DECLS

#endif