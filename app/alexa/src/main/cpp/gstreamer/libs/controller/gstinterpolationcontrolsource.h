#ifndef __GST_INTERPOLATION_CONTROL_SOURCE_H__
#define __GST_INTERPOLATION_CONTROL_SOURCE_H__

#include <glib/glib-object.h>
#include <gstreamer/gst/gst.h>
#include "gsttimedvaluecontrolsource.h"

G_BEGIN_DECLS
#define GST_TYPE_INTERPOLATION_CONTROL_SOURCE  (gst_interpolation_control_source_get_type ())
#define GST_INTERPOLATION_CONTROL_SOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_INTERPOLATION_CONTROL_SOURCE, GstInterpolationControlSource))
#define GST_INTERPOLATION_CONTROL_SOURCE_CLASS(vtable) \
  (G_TYPE_CHECK_CLASS_CAST ((vtable), GST_TYPE_INTERPOLATION_CONTROL_SOURCE, GstInterpolationControlSourceClass))
#define GST_IS_INTERPOLATION_CONTROL_SOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_INTERPOLATION_CONTROL_SOURCE))
#define GST_IS_INTERPOLATION_CONTROL_SOURCE_CLASS(vtable) \
  (G_TYPE_CHECK_CLASS_TYPE ((vtable), GST_TYPE_INTERPOLATION_CONTROL_SOURCE))
#define GST_INTERPOLATION_CONTROL_SOURCE_GET_CLASS(inst) \
  (G_TYPE_INSTANCE_GET_CLASS ((inst), GST_TYPE_INTERPOLATION_CONTROL_SOURCE, GstInterpolationControlSourceClass))
#define GST_TYPE_INTERPOLATION_MODE (gst_interpolation_mode_get_type ())
typedef struct _GstInterpolationControlSource GstInterpolationControlSource;
typedef struct _GstInterpolationControlSourceClass GstInterpolationControlSourceClass;
typedef struct _GstInterpolationControlSourcePrivate GstInterpolationControlSourcePrivate;
typedef enum {
  GST_INTERPOLATION_MODE_NONE,
  GST_INTERPOLATION_MODE_LINEAR,
  GST_INTERPOLATION_MODE_CUBIC,
  GST_INTERPOLATION_MODE_CUBIC_MONOTONIC,
} GstInterpolationMode;
struct _GstInterpolationControlSource {
  GstTimedValueControlSource parent;
  GstInterpolationControlSourcePrivate *priv;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstInterpolationControlSourceClass {
  GstTimedValueControlSourceClass parent_class;
  gpointer _gst_reserved[GST_PADDING];
};
GType gst_interpolation_control_source_get_type (void);
GType gst_interpolation_mode_get_type (void);
GstControlSource * gst_interpolation_control_source_new (void);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstInterpolationControlSource, gst_object_unref)
#endif
G_END_DECLS

#endif