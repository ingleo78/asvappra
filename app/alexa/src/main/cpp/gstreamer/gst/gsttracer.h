#ifndef __GST_TRACER_H__
#define __GST_TRACER_H__

#include <glib/glib.h>
#include <glib/glib-object.h>
#include "gstconfig.h"

G_BEGIN_DECLS
typedef struct _GstTracer GstTracer;
typedef struct _GstTracerPrivate GstTracerPrivate;
typedef struct _GstTracerClass GstTracerClass;
#define GST_TYPE_TRACER            (gst_tracer_get_type())
#define GST_TRACER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TRACER,GstTracer))
#define GST_TRACER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TRACER,GstTracerClass))
#define GST_IS_TRACER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TRACER))
#define GST_IS_TRACER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TRACER))
#define GST_TRACER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),GST_TYPE_TRACER,GstTracerClass))
#define GST_TRACER_CAST(obj)       ((GstTracer *)(obj))
struct _GstTracer {
  GstObject        parent;
  GstTracerPrivate *priv;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstTracerClass {
  GstObjectClass parent_class;
  gpointer _gst_reserved[GST_PADDING];
};
GType gst_tracer_get_type          (void);
#ifdef GST_USE_UNSTABLE_API
void gst_tracing_register_hook (GstTracer *tracer, const gchar *detail, GCallback func);
gboolean gst_tracer_register (GstPlugin * plugin, const gchar * name, GType type);
#endif
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstTracer, gst_object_unref)
#endif
G_END_DECLS

#endif