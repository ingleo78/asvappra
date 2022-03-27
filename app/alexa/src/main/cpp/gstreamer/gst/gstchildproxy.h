#ifndef __GST_CHILD_PROXY_H__
#define __GST_CHILD_PROXY_H__

#include <glib/glib-object.h>

G_BEGIN_DECLS
#define GST_TYPE_CHILD_PROXY               (gst_child_proxy_get_type ())
#define GST_CHILD_PROXY(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_CHILD_PROXY, GstChildProxy))
#define GST_IS_CHILD_PROXY(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_CHILD_PROXY))
#define GST_CHILD_PROXY_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), GST_TYPE_CHILD_PROXY, GstChildProxyInterface))
typedef struct _GstChildProxy GstChildProxy;
typedef struct _GstChildProxyInterface GstChildProxyInterface;
struct _GstChildProxyInterface {
  GTypeInterface parent;
  GObject * (*get_child_by_name)  (GstChildProxy * parent, const gchar * name);
  GObject * (*get_child_by_index) (GstChildProxy * parent, guint index);
  guint     (*get_children_count) (GstChildProxy * parent);
  void      (*child_added)        (GstChildProxy * parent, GObject * child, const gchar * name);
  void      (*child_removed)      (GstChildProxy * parent, GObject * child, const gchar * name);
  gpointer _gst_reserved[GST_PADDING];
};
GType     gst_child_proxy_get_type (void);
GObject * gst_child_proxy_get_child_by_name  (GstChildProxy * parent, const gchar * name);
guint     gst_child_proxy_get_children_count (GstChildProxy * parent);
GObject * gst_child_proxy_get_child_by_index (GstChildProxy * parent, guint index);
gboolean  gst_child_proxy_lookup             (GstChildProxy *object, const gchar *name, GObject **target, GParamSpec **pspec);
void      gst_child_proxy_get_property       (GstChildProxy * object, const gchar *name, GValue *value);
void      gst_child_proxy_get_valist         (GstChildProxy * object, const gchar * first_property_name, va_list var_args);
void      gst_child_proxy_get                (GstChildProxy * object, const gchar * first_property_name, ...) G_GNUC_NULL_TERMINATED;
void      gst_child_proxy_set_property       (GstChildProxy * object, const gchar *name, const GValue *value);
void      gst_child_proxy_set_valist         (GstChildProxy* object, const gchar * first_property_name, va_list var_args);
void      gst_child_proxy_set                (GstChildProxy * object, const gchar * first_property_name, ...) G_GNUC_NULL_TERMINATED;
void      gst_child_proxy_child_added        (GstChildProxy * parent, GObject * child, const gchar *name);
void      gst_child_proxy_child_removed      (GstChildProxy * parent, GObject * child, const gchar *name);
G_END_DECLS

#endif