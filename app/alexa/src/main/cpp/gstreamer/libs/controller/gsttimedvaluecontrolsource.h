#ifndef __GST_TIMED_VALUE_CONTROL_SOURCE_H__
#define __GST_TIMED_VALUE_CONTROL_SOURCE_H__

#include <glib/glib-object.h>
#include <gstreamer/gst/gst.h>
#include <gstreamer/gst/gstcontrolsource.h>

G_BEGIN_DECLS
#define GST_TYPE_TIMED_VALUE_CONTROL_SOURCE  (gst_timed_value_control_source_get_type ())
#define GST_TIMED_VALUE_CONTROL_SOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_TIMED_VALUE_CONTROL_SOURCE, GstTimedValueControlSource))
#define GST_TIMED_VALUE_CONTROL_SOURCE_CLASS(vtable) \
  (G_TYPE_CHECK_CLASS_CAST ((vtable), GST_TYPE_TIMED_VALUE_CONTROL_SOURCE, GstTimedValueControlSourceClass))
#define GST_IS_TIMED_VALUE_CONTROL_SOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_TIMED_VALUE_CONTROL_SOURCE))
#define GST_IS_TIMED_VALUE_CONTROL_SOURCE_CLASS(vtable) \
  (G_TYPE_CHECK_CLASS_TYPE ((vtable), GST_TYPE_TIMED_VALUE_CONTROL_SOURCE))
#define GST_TIMED_VALUE_CONTROL_SOURCE_GET_CLASS(inst) \
  (G_TYPE_INSTANCE_GET_CLASS ((inst), GST_TYPE_TIMED_VALUE_CONTROL_SOURCE, GstTimedValueControlSourceClass))
typedef struct _GstTimedValueControlSource GstTimedValueControlSource;
typedef struct _GstTimedValueControlSourceClass GstTimedValueControlSourceClass;
typedef struct _GstTimedValueControlSourcePrivate GstTimedValueControlSourcePrivate;
typedef struct _GstControlPoint GstControlPoint;
struct _GstControlPoint {
  GstClockTime timestamp;
  gdouble value;
  union {
    struct {
      gdouble h;
      gdouble z;
    } cubic;
    struct {
      gdouble c1s, c2s, c3s;
    } cubic_monotonic;
    guint8 _gst_reserved[64];
  } cache;
};
GType gst_control_point_get_type (void);
struct _GstTimedValueControlSource {
  GstControlSource parent;
  GMutex lock;
  GSequence *values;
  gint nvalues;
  gboolean valid_cache;
  GstTimedValueControlSourcePrivate *priv;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstTimedValueControlSourceClass {
  GstControlSourceClass parent_class;
  gpointer _gst_reserved[GST_PADDING];
};
#define GST_TIMED_VALUE_CONTROL_SOURCE_LOCK(o)  g_mutex_lock(&((GstTimedValueControlSource *)o)->lock)
#define GST_TIMED_VALUE_CONTROL_SOURCE_UNLOCK(o)  g_mutex_unlock(&((GstTimedValueControlSource *)o)->lock)
GType gst_timed_value_control_source_get_type(void);
GSequenceIter *gst_timed_value_control_source_find_control_point_iter(GstTimedValueControlSource * self, GstClockTime timestamp);
gboolean gst_timed_value_control_source_set(GstTimedValueControlSource *self, GstClockTime timestamp, const gdouble value);
gboolean gst_timed_value_control_source_set_from_list(GstTimedValueControlSource *self, const GSList *timedvalues);
gboolean gst_timed_value_control_source_unset(GstTimedValueControlSource *self, GstClockTime timestamp);
void gst_timed_value_control_source_unset_all(GstTimedValueControlSource *self);
GList *gst_timed_value_control_source_get_all(GstTimedValueControlSource *self);
gint gst_timed_value_control_source_get_count(GstTimedValueControlSource *self);
void gst_timed_value_control_invalidate_cache(GstTimedValueControlSource *self);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstTimedValueControlSource, gst_object_unref)
#endif
G_END_DECLS

#endif