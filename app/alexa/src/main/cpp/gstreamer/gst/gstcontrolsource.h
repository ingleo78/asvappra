#ifndef __GST_CONTROL_SOURCE_H__
#define __GST_CONTROL_SOURCE_H__

#include <glib/glib-object.h>
#include "gstconfig.h"
#include "gstclock.h"

G_BEGIN_DECLS
#define GST_TYPE_CONTROL_SOURCE  (gst_control_source_get_type())
#define GST_CONTROL_SOURCE(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_CONTROL_SOURCE,GstControlSource))
#define GST_CONTROL_SOURCE_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_CONTROL_SOURCE,GstControlSourceClass))
#define GST_IS_CONTROL_SOURCE(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_CONTROL_SOURCE))
#define GST_IS_CONTROL_SOURCE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_CONTROL_SOURCE))
#define GST_CONTROL_SOURCE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_CONTOL_SOURCE, GstControlSourceClass))
typedef struct _GstControlSource GstControlSource;
typedef struct _GstControlSourceClass GstControlSourceClass;
typedef struct _GstTimedValue GstTimedValue;
typedef struct _GstValueArray GstValueArray;
struct _GstTimedValue {
  GstClockTime timestamp;
  gdouble      value;
};
typedef gboolean (* GstControlSourceGetValue) (GstControlSource *self, 
    GstClockTime timestamp, gdouble *value);
typedef gboolean (* GstControlSourceGetValueArray) (GstControlSource *self, 
    GstClockTime timestamp, GstClockTime interval, guint n_values, gdouble *values);
struct _GstControlSource {
  GstObject parent;
  GstControlSourceGetValue get_value;
  GstControlSourceGetValueArray get_value_array;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstControlSourceClass {
  GstObjectClass parent_class;
  gpointer _gst_reserved[GST_PADDING];
};
GType gst_control_source_get_type(void);
gboolean gst_control_source_get_value(GstControlSource *self, GstClockTime timestamp, gdouble *value);
gboolean gst_control_source_get_value_array(GstControlSource *self, GstClockTime timestamp, GstClockTime interval, guint n_values,
                                            gdouble *values);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstControlSource, gst_object_unref)
#endif
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstValueArray, gst_object_unref)
#endif
G_END_DECLS

#endif