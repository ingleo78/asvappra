#ifndef __GST_OBJECT_H__
#define __GST_OBJECT_H__

#include <glib/glib-object.h>
#include <glib/glib.h>
#include "gstconfig.h"

G_BEGIN_DECLS
#define GST_TYPE_OBJECT			(gst_object_get_type ())
#define GST_IS_OBJECT(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_OBJECT))
#define GST_IS_OBJECT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_OBJECT))
#define GST_OBJECT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_OBJECT, GstObjectClass))
#define GST_OBJECT(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_OBJECT, GstObject))
#define GST_OBJECT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_OBJECT, GstObjectClass))
#define GST_OBJECT_CAST(obj)            ((GstObject*)(obj))
#define GST_OBJECT_CLASS_CAST(klass)    ((GstObjectClass*)(klass))
typedef enum {
  GST_OBJECT_FLAG_LAST = (1<<4)
} GstObjectFlags;
typedef guint64 GstClockTime;
typedef struct _GstControlBinding GstControlBinding;
#define GST_OBJECT_REFCOUNT(obj)                (((GObject*)(obj))->ref_count)
#define GST_OBJECT_REFCOUNT_VALUE(obj)          g_atomic_int_get ((gint *) &GST_OBJECT_REFCOUNT(obj))
#define GST_OBJECT_GET_LOCK(obj)               (&GST_OBJECT_CAST(obj)->lock)
#define GST_OBJECT_LOCK(obj)                   g_mutex_lock(GST_OBJECT_GET_LOCK(obj))
#define GST_OBJECT_TRYLOCK(obj)                g_mutex_trylock(GST_OBJECT_GET_LOCK(obj))
#define GST_OBJECT_UNLOCK(obj)                 g_mutex_unlock(GST_OBJECT_GET_LOCK(obj))
#define GST_OBJECT_NAME(obj)            (GST_OBJECT_CAST(obj)->name)
#define GST_OBJECT_PARENT(obj)          (GST_OBJECT_CAST(obj)->parent)
#define GST_OBJECT_FLAGS(obj)                  (GST_OBJECT_CAST (obj)->flags)
#define GST_OBJECT_FLAG_IS_SET(obj,flag)       ((GST_OBJECT_FLAGS (obj) & (flag)) == (flag))
#define GST_OBJECT_FLAG_SET(obj,flag)          (GST_OBJECT_FLAGS (obj) |= (flag))
#define GST_OBJECT_FLAG_UNSET(obj,flag)        (GST_OBJECT_FLAGS (obj) &= ~(flag))
typedef struct _GstObject GstObject;
typedef struct _GstObjectClass GstObjectClass;
struct _GstObject {
  GInitiallyUnowned object;
  GMutex         lock;
  gchar         *name;
  GstObject     *parent;
  guint32        flags;
  GList         *control_bindings;
  guint64        control_rate;
  guint64        last_sync;
  gpointer _gst_reserved;
};
struct _GstObjectClass {
  GInitiallyUnownedClass parent_class;
  const gchar	*path_string_separator;
  void          (*deep_notify)      (GstObject * object, GstObject * orig, GParamSpec * pspec);
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstControlBinding {
    GstObject parent;
    gchar *name;
    GParamSpec *pspec;
    GstObject *object;
    gboolean disabled;
    gpointer _gst_reserved[GST_PADDING];
};
gsize		gst_object_get_type		(void);
gboolean	gst_object_set_name		(GstObject *object, const gchar *name);
gchar*		gst_object_get_name		(GstObject *object);
gboolean	gst_object_set_parent		(GstObject *object, GstObject *parent);
GstObject*	gst_object_get_parent		(GstObject *object);
void		gst_object_unparent		(GstObject *object);
gboolean	gst_object_has_as_parent		(GstObject *object, GstObject *parent);
gboolean	gst_object_has_as_ancestor	(GstObject *object, GstObject *ancestor);
#ifndef GST_DISABLE_DEPRECATED
gboolean	gst_object_has_ancestor		(GstObject *object, GstObject *ancestor);
#endif
void            gst_object_default_deep_notify  (GObject *object, GstObject *orig, GParamSpec *pspec, gchar **excluded_props);
gpointer	gst_object_ref			(gpointer object);
void		gst_object_unref		(gpointer object);
gpointer        gst_object_ref_sink		(gpointer object);
gboolean        gst_object_replace		(GstObject **oldobj, GstObject *newobj);
gchar *		gst_object_get_path_string	(GstObject *object);
gboolean	gst_object_check_uniqueness	(GList *list, const gchar *name);
GstClockTime    gst_object_suggest_next_sync      (GstObject * object);
gboolean        gst_object_sync_values            (GstObject * object, GstClockTime timestamp);
gboolean        gst_object_has_active_control_bindings   (GstObject *object);
void            gst_object_set_control_bindings_disabled (GstObject *object, gboolean disabled);
void            gst_object_set_control_binding_disabled  (GstObject *object, const gchar * property_name, gboolean disabled);
gboolean        gst_object_add_control_binding    (GstObject * object, GstControlBinding * binding);
GstControlBinding *gst_object_get_control_binding    (GstObject *object, const gchar * property_name);
gboolean        gst_object_remove_control_binding (GstObject * object, GstControlBinding * binding);
GValue *        gst_object_get_value              (GstObject * object, const gchar * property_name, GstClockTime timestamp);
gboolean        gst_object_get_value_array        (GstObject * object, const gchar * property_name, GstClockTime timestamp, GstClockTime interval,
                                                   guint n_values, gpointer values);
gboolean        gst_object_get_g_value_array      (GstObject * object, const gchar * property_name, GstClockTime timestamp, GstClockTime interval,
                                                   guint n_values, GValue *values);
GstClockTime    gst_object_get_control_rate       (GstObject * object);
void            gst_object_set_control_rate       (GstObject * object, GstClockTime control_rate);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstObject, gst_object_unref)
#endif
G_END_DECLS

#endif