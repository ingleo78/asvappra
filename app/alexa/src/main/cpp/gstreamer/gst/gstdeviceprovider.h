#ifndef __GST_DEVICE_PROVIDER_H__
#define __GST_DEVICE_PROVIDER_H__

#include "gstelement.h"
#include "gstdeviceproviderfactory.h"

G_BEGIN_DECLS
typedef struct _GstDeviceProvider GstDeviceProvider;
typedef struct _GstDeviceProviderClass GstDeviceProviderClass;
typedef struct _GstDeviceProviderPrivate GstDeviceProviderPrivate;
#define GST_TYPE_DEVICE_PROVIDER                 (gst_device_provider_get_type())
#define GST_IS_DEVICE_PROVIDER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_DEVICE_PROVIDER))
#define GST_IS_DEVICE_PROVIDER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_DEVICE_PROVIDER))
#define GST_DEVICE_PROVIDER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_DEVICE_PROVIDER, GstDeviceProviderClass))
#define GST_DEVICE_PROVIDER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_DEVICE_PROVIDER, GstDeviceProvider))
#define GST_DEVICE_PROVIDER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_DEVICE_PROVIDER, GstDeviceProviderClass))
#define GST_DEVICE_PROVIDER_CAST(obj)            ((GstDeviceProvider *)(obj))
struct _GstDeviceProvider {
  GstObject         parent;
  GList *devices;
  GstDeviceProviderPrivate *priv;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstDeviceProviderClass {
  GstObjectClass    parent_class;
  GstDeviceProviderFactory     *factory;
  GList*      (*probe) (GstDeviceProvider * provider);
  gboolean    (*start) (GstDeviceProvider * provider);
  void        (*stop)  (GstDeviceProvider * provider);
  gpointer metadata;
  gpointer _gst_reserved[GST_PADDING];
};
GType       gst_device_provider_get_type (void);
GList *     gst_device_provider_get_devices    (GstDeviceProvider * provider);
gboolean    gst_device_provider_start          (GstDeviceProvider * provider);
void        gst_device_provider_stop           (GstDeviceProvider * provider);
gboolean    gst_device_provider_can_monitor    (GstDeviceProvider * provider);
GstBus *    gst_device_provider_get_bus        (GstDeviceProvider * provider);
void        gst_device_provider_device_add     (GstDeviceProvider * provider, GstDevice * device);
void        gst_device_provider_device_remove  (GstDeviceProvider * provider, GstDevice * device);
gchar **    gst_device_provider_get_hidden_providers (GstDeviceProvider * provider);
void        gst_device_provider_hide_provider        (GstDeviceProvider * provider, const gchar       * name);
void        gst_device_provider_unhide_provider      (GstDeviceProvider * provider, const gchar       * name);
void gst_device_provider_class_set_metadata(GstDeviceProviderClass *klass, const gchar *longname, const gchar *classification,
                                                            const gchar *description, const gchar *author);
void gst_device_provider_class_set_static_metadata(GstDeviceProviderClass *klass, const gchar *longname, const gchar *classification,
                                                            const gchar *description, const gchar *author);
void        gst_device_provider_class_add_metadata(GstDeviceProviderClass * klass, const gchar * key, const gchar * value);
void        gst_device_provider_class_add_static_metadata(GstDeviceProviderClass * klass, const gchar * key, const gchar * value);
const gchar * gst_device_provider_class_get_metadata(GstDeviceProviderClass * klass, const gchar * key);
GstDeviceProviderFactory * gst_device_provider_get_factory (GstDeviceProvider * provider);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstDeviceProvider, gst_object_unref)
#endif
G_END_DECLS

#endif