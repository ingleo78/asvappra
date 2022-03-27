#ifndef __GST_SAMPLE_H__
#define __GST_SAMPLE_H__

#include "gstbufferlist.h"
#include "gstcaps.h"
#include "gstsegment.h"

G_BEGIN_DECLS
GST_EXPORT GType _gst_sample_type;
#define GST_TYPE_SAMPLE      (_gst_sample_type)
#define GST_IS_SAMPLE(obj)   (GST_IS_MINI_OBJECT_TYPE(obj, GST_TYPE_SAMPLE))
#define GST_SAMPLE_CAST(obj) ((GstSample *)obj)
#define GST_SAMPLE(obj)      (GST_SAMPLE_CAST(obj))
typedef struct _GstSample GstSample;
GType gst_sample_get_type            (void);
GstSample *          gst_sample_new           (GstBuffer *buffer, GstCaps *caps, const GstSegment *segment, GstStructure *info);
GstBuffer *          gst_sample_get_buffer    (GstSample *sample);
GstCaps *            gst_sample_get_caps      (GstSample *sample);
GstSegment *         gst_sample_get_segment   (GstSample *sample);
const GstStructure * gst_sample_get_info      (GstSample *sample);
GstBufferList *      gst_sample_get_buffer_list (GstSample *sample);
void                 gst_sample_set_buffer_list (GstSample *sample, GstBufferList *buffer_list);
static inline GstSample *gst_sample_ref (GstSample * sample) {
  return GST_SAMPLE_CAST (gst_mini_object_ref (GST_MINI_OBJECT_CAST (
      sample)));
}
static inline void gst_sample_unref (GstSample * sample) {
  gst_mini_object_unref (GST_MINI_OBJECT_CAST (sample));
}
static inline GstSample *gst_sample_copy (const GstSample * buf) {
  return GST_SAMPLE_CAST (gst_mini_object_copy (GST_MINI_OBJECT_CONST_CAST (buf)));
}
#define         gst_value_set_sample(v,b)       g_value_set_boxed((v),(b))
#define         gst_value_take_sample(v,b)      g_value_take_boxed(v,(b))
#define         gst_value_get_sample(v)         GST_SAMPLE_CAST (g_value_get_boxed(v))
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstSample, gst_sample_unref)
#endif
G_END_DECLS

#endif