#ifndef __GST_PIPELINE_H__
#define __GST_PIPELINE_H__

#include "gstbin.h"

G_BEGIN_DECLS
#define GST_TYPE_PIPELINE               (gst_pipeline_get_type ())
#define GST_PIPELINE(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_PIPELINE, GstPipeline))
#define GST_IS_PIPELINE(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_PIPELINE))
#define GST_PIPELINE_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_PIPELINE, GstPipelineClass))
#define GST_IS_PIPELINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_PIPELINE))
#define GST_PIPELINE_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_PIPELINE, GstPipelineClass))
#define GST_PIPELINE_CAST(obj)          ((GstPipeline*)(obj))
typedef struct _GstPipeline GstPipeline;
typedef struct _GstPipelineClass GstPipelineClass;
typedef struct _GstPipelinePrivate GstPipelinePrivate;
typedef enum {
  GST_PIPELINE_FLAG_FIXED_CLOCK        = (GST_BIN_FLAG_LAST << 0),
  GST_PIPELINE_FLAG_LAST               = (GST_BIN_FLAG_LAST << 4)
} GstPipelineFlags;
struct _GstPipeline {
  GstBin         bin;
  GstClock      *fixed_clock;
  GstClockTime   stream_time;
  GstClockTime   delay;
  GstPipelinePrivate *priv;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstPipelineClass {
  GstBinClass parent_class;
  gpointer _gst_reserved[GST_PADDING];
};
GType           gst_pipeline_get_type           (void);
GstElement*     gst_pipeline_new                (const gchar *name) G_GNUC_MALLOC;
GstBus*         gst_pipeline_get_bus            (GstPipeline *pipeline);
void            gst_pipeline_use_clock          (GstPipeline *pipeline, GstClock *clock);
gboolean        gst_pipeline_set_clock          (GstPipeline *pipeline, GstClock *clock);
GstClock*       gst_pipeline_get_clock          (GstPipeline *pipeline);
GstClock*       gst_pipeline_get_pipeline_clock (GstPipeline *pipeline);
void            gst_pipeline_auto_clock         (GstPipeline *pipeline);
void            gst_pipeline_set_delay          (GstPipeline *pipeline, GstClockTime delay);
GstClockTime    gst_pipeline_get_delay          (GstPipeline *pipeline);
void            gst_pipeline_set_latency        (GstPipeline *pipeline, GstClockTime latency);
GstClockTime    gst_pipeline_get_latency        (GstPipeline *pipeline);
void            gst_pipeline_set_auto_flush_bus (GstPipeline *pipeline, gboolean auto_flush);
gboolean        gst_pipeline_get_auto_flush_bus (GstPipeline *pipeline);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstPipeline, gst_object_unref)
#endif
G_END_DECLS

#endif