#ifndef __GST_TRACER_RECORD_H__
#define __GST_TRACER_RECORD_H__

#include "gstobject.h"

G_BEGIN_DECLS
typedef struct _GstTracerRecord GstTracerRecord;
typedef struct _GstTracerRecordClass GstTracerRecordClass;
#define GST_TYPE_TRACER_RECORD            (gst_tracer_record_get_type())
#define GST_TRACER_RECORD(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TRACER_RECORD,GstTracerRecord))
#define GST_TRACER_RECORD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TRACER_RECORD,GstTracerRecordClass))
#define GST_IS_TRACER_RECORD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TRACER_RECORD))
#define GST_IS_TRACER_RECORD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TRACER_RECORD))
#define GST_TRACER_RECORD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),GST_TYPE_TRACER_RECORD,GstTracerRecordClass))
#define GST_TRACER_RECORD_CAST(obj)       ((GstTracerRecord *)(obj))
GType gst_tracer_record_get_type          (void);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstTracerRecord, gst_object_unref)
#endif
typedef enum {
  GST_TRACER_VALUE_SCOPE_PROCESS,
  GST_TRACER_VALUE_SCOPE_THREAD,
  GST_TRACER_VALUE_SCOPE_ELEMENT,
  GST_TRACER_VALUE_SCOPE_PAD
} GstTracerValueScope;
typedef enum {
  GST_TRACER_VALUE_FLAGS_NONE = 0,
  GST_TRACER_VALUE_FLAGS_OPTIONAL = (1 << 0),
  GST_TRACER_VALUE_FLAGS_AGGREGATED = (1 << 1),
} GstTracerValueFlags;
#ifdef GST_USE_UNSTABLE_API
GstTracerRecord * gst_tracer_record_new (const gchar * name, const gchar * firstfield, ...);
#ifndef GST_DISABLE_GST_DEBUG
void              gst_tracer_record_log (GstTracerRecord *self, ...);
#else
#define gst_tracer_record_log(...) G_STMT_START {} G_STMT_END
#endif
#endif
G_END_DECLS

#endif