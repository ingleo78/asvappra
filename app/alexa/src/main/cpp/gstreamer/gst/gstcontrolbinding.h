#ifndef __GST_CONTROL_BINDING_H__
#define __GST_CONTROL_BINDING_H__

#include <glib/glib-object.h>
#include "gstconfig.h"
#include "gstclock.h"

G_BEGIN_DECLS

#define GST_TYPE_CONTROL_BINDING  (gst_control_binding_get_type())
#define GST_CONTROL_BINDING(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_CONTROL_BINDING,GstControlBinding))
#define GST_CONTROL_BINDING_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_CONTROL_BINDING,GstControlBindingClass))
#define GST_IS_CONTROL_BINDING(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_CONTROL_BINDING))
#define GST_IS_CONTROL_BINDING_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_CONTROL_BINDING))
#define GST_CONTROL_BINDING_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_CONTOL_SOURCE, GstControlBindingClass))
typedef struct _GstControlBinding GstControlBinding;
typedef struct _GstControlBindingClass GstControlBindingClass;
typedef void (* GstControlBindingConvert) (GstControlBinding *binding, gdouble src_value, GValue *dest_value);
/*struct _GstControlBinding {
  GstObject parent;
  gchar *name;
  GParamSpec *pspec;
  GstObject *object;
  gboolean disabled;
  gpointer _gst_reserved[GST_PADDING];
};*/
struct _GstControlBindingClass {
  GstObjectClass parent_class;
  gboolean (* sync_values) (GstControlBinding *binding, GstObject *object, GstClockTime timestamp, GstClockTime last_sync);
  GValue * (* get_value) (GstControlBinding *binding, GstClockTime timestamp);
  gboolean (* get_value_array) (GstControlBinding *binding, GstClockTime timestamp,GstClockTime interval, guint n_values, gpointer values);
  gboolean (* get_g_value_array) (GstControlBinding *binding, GstClockTime timestamp,GstClockTime interval, guint n_values, GValue *values);
  gpointer _gst_reserved[GST_PADDING];
};
#define GST_CONTROL_BINDING_PSPEC(cb) (((GstControlBinding *) cb)->pspec)
GType gst_control_binding_get_type (void);
gboolean gst_control_binding_sync_values(GstControlBinding * binding, GstObject *object, GstClockTime timestamp, GstClockTime last_sync);
GValue *            gst_control_binding_get_value          (GstControlBinding *binding, GstClockTime timestamp);
gboolean gst_control_binding_get_value_array(GstControlBinding *binding, GstClockTime timestamp, GstClockTime interval, guint n_values,
                                             gpointer values);
gboolean gst_control_binding_get_g_value_array(GstControlBinding *binding, GstClockTime timestamp, GstClockTime interval, guint n_values,
                                               GValue *values);
void                gst_control_binding_set_disabled       (GstControlBinding * binding, gboolean disabled);
gboolean            gst_control_binding_is_disabled        (GstControlBinding * binding);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstControlBinding, gst_object_unref)
#endif
G_END_DECLS

#endif