#ifndef __GST_DIRECT_CONTROL_BINDING_H__
#define __GST_DIRECT_CONTROL_BINDING_H__

#include <glib/glib-object.h>
#include <gstreamer/gst/gstconfig.h>
#include <gstreamer/gst/gstcontrolsource.h>
#include <gstreamer/gst/gstcontrolbinding.h>

G_BEGIN_DECLS
#define GST_TYPE_DIRECT_CONTROL_BINDING  (gst_direct_control_binding_get_type())
#define GST_DIRECT_CONTROL_BINDING(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_DIRECT_CONTROL_BINDING,GstDirectControlBinding))
#define GST_DIRECT_CONTROL_BINDING_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_DIRECT_CONTROL_BINDING,GstDirectControlBindingClass))
#define GST_IS_DIRECT_CONTROL_BINDING(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_DIRECT_CONTROL_BINDING))
#define GST_IS_DIRECT_CONTROL_BINDING_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_DIRECT_CONTROL_BINDING))
#define GST_DIRECT_CONTROL_BINDING_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_CONTOL_SOURCE, GstDirectControlBindingClass))
typedef struct _GstDirectControlBinding GstDirectControlBinding;
typedef struct _GstDirectControlBindingClass GstDirectControlBindingClass;
typedef void (* GstDirectControlBindingConvertValue) (GstDirectControlBinding *self, gdouble src_value, gpointer dest_value);
typedef void (* GstDirectControlBindingConvertGValue) (GstDirectControlBinding *self, gdouble src_value, GValue *dest_value);
struct _GstDirectControlBinding {
  GstControlBinding parent;
  GstControlSource *cs;
  GValue cur_value;
  gdouble last_value;
  gint byte_size;
  GstDirectControlBindingConvertValue convert_value;
  GstDirectControlBindingConvertGValue convert_g_value;
  union {
    gpointer _gst_reserved[GST_PADDING];
    struct {
      gboolean want_absolute;
    } abi;
  } ABI;
};
struct _GstDirectControlBindingClass {
  GstControlBindingClass parent_class;
  gpointer _gst_reserved[GST_PADDING];
};
GType gst_direct_control_binding_get_type(void);
GstControlBinding *gst_direct_control_binding_new(GstObject *object, const gchar *property_name, GstControlSource *cs);
GstControlBinding *gst_direct_control_binding_new_absolute(GstObject *object, const gchar *property_name, GstControlSource *cs);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstDirectControlBinding, gst_object_unref)
#endif
G_END_DECLS

#endif