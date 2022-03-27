#ifndef __GST_DEVICE_H__
#define __GST_DEVICE_H__

//#include "gstelement.h"
#include "gstcaps.h"

typedef struct _GstDevice GstDevice;
typedef struct _GstDeviceClass GstDeviceClass;
G_BEGIN_DECLS
typedef struct _GstDevicePrivate GstDevicePrivate;
#define GST_TYPE_DEVICE                 (gst_device_get_type())
#define GST_IS_DEVICE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_DEVICE))
#define GST_IS_DEVICE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_DEVICE))
#define GST_DEVICE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_DEVICE, GstDeviceClass))
#define GST_DEVICE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_DEVICE, GstDevice))
#define GST_DEVICE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_DEVICE, GstDeviceClass))
#define GST_DEVICE_CAST(obj)            ((GstDevice *)(obj))
struct _GstDevice {
  GstObject         parent;
  GstDevicePrivate *priv;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstDeviceClass {
  GstObjectClass    parent_class;
  GstElement * (*create_element)      (GstDevice * device, const gchar * name);
  gboolean     (*reconfigure_element) (GstDevice * device, GstElement * element);
  gpointer _gst_reserved[GST_PADDING];
};
GType          gst_device_get_type (void);
GstElement *   gst_device_create_element      (GstDevice * device, const gchar * name);
GstCaps *      gst_device_get_caps            (GstDevice * device);
gchar *        gst_device_get_display_name    (GstDevice * device);
gchar *        gst_device_get_device_class    (GstDevice * device);
GstStructure * gst_device_get_properties      (GstDevice * device);
gboolean       gst_device_reconfigure_element (GstDevice * device, GstElement * element);
gboolean        gst_device_has_classesv       (GstDevice * device, gchar ** classes);
gboolean        gst_device_has_classes        (GstDevice * device, const gchar * classes);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstDevice, gst_object_unref)
#endif
G_END_DECLS

#endif