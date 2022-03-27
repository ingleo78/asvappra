#ifndef __GST_DEVICE_MONITOR_H__
#define __GST_DEVICE_MONITOR_H__

#include "gstobject.h"
#include "gstdevice.h"
#include "gstdeviceprovider.h"
#include "gstdeviceproviderfactory.h"

G_BEGIN_DECLS
typedef struct _GstDeviceMonitor GstDeviceMonitor;
typedef struct _GstDeviceMonitorPrivate GstDeviceMonitorPrivate;
typedef struct _GstDeviceMonitorClass GstDeviceMonitorClass;
#define GST_TYPE_DEVICE_MONITOR                 (gst_device_monitor_get_type())
#define GST_IS_DEVICE_MONITOR(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_DEVICE_MONITOR))
#define GST_IS_DEVICE_MONITOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_DEVICE_MONITOR))
#define GST_DEVICE_MONITOR_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_DEVICE_MONITOR, GstDeviceMonitorClass))
#define GST_DEVICE_MONITOR(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_DEVICE_MONITOR, GstDeviceMonitor))
#define GST_DEVICE_MONITOR_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_DEVICE_MONITOR, GstDeviceMonitorClass))
#define GST_DEVICE_MONITOR_CAST(obj)            ((GstDeviceMonitor *)(obj))
struct _GstDeviceMonitor {
  GstObject                parent;
  GstDeviceMonitorPrivate *priv;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstDeviceMonitorClass {
  GstObjectClass           parent_class;
  gpointer _gst_reserved[GST_PADDING];
};
GType     gst_device_monitor_get_type (void);
GstDeviceMonitor * gst_device_monitor_new  (void);
GstBus *  gst_device_monitor_get_bus (GstDeviceMonitor * monitor);
GList *   gst_device_monitor_get_devices (GstDeviceMonitor * monitor);
gboolean  gst_device_monitor_start (GstDeviceMonitor * monitor);
void      gst_device_monitor_stop  (GstDeviceMonitor * monitor);
guint     gst_device_monitor_add_filter (GstDeviceMonitor *monitor, const gchar *classes, GstCaps *caps);
gboolean  gst_device_monitor_remove_filter (GstDeviceMonitor * monitor, guint filter_id);
gchar **  gst_device_monitor_get_providers (GstDeviceMonitor * monitor);
void      gst_device_monitor_set_show_all_devices (GstDeviceMonitor * monitor, gboolean show_all);
gboolean  gst_device_monitor_get_show_all_devices (GstDeviceMonitor * monitor);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstDeviceMonitor, gst_object_unref)
#endif
G_END_DECLS

#endif