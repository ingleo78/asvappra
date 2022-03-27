#ifndef __GST_CAPS_FEATURES_H__
#define __GST_CAPS_FEATURES_H__

#include <glib/glib-object.h>
#include <glib/glib.h>
#include "gstconfig.h"
#include "glib-compat.h"

G_BEGIN_DECLS
typedef struct _GstCapsFeatures GstCapsFeatures;
GST_EXPORT GType _gst_caps_features_type;
#define GST_TYPE_CAPS_FEATURES (_gst_caps_features_type)
#define GST_IS_CAPS_FEATURES(object)       (gst_is_caps_features(object))
#define GST_CAPS_FEATURES_CAST(object)     ((GstCapsFeatures *)(object))
#define GST_CAPS_FEATURES(object)          (GST_CAPS_FEATURES_CAST(object))
#define GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY "memory:SystemMemory"
GST_EXPORT GstCapsFeatures *_gst_caps_features_any;
#define GST_CAPS_FEATURES_ANY (_gst_caps_features_any)
GST_EXPORT GstCapsFeatures *_gst_caps_features_memory_system_memory;
#define GST_CAPS_FEATURES_MEMORY_SYSTEM_MEMORY (_gst_caps_features_memory_system_memory)
GType             gst_caps_features_get_type (void);
gboolean          gst_is_caps_features (gconstpointer obj);
GstCapsFeatures * gst_caps_features_new_empty (void);
GstCapsFeatures * gst_caps_features_new_any (void);
GstCapsFeatures * gst_caps_features_new (const gchar *feature1, ...);
GstCapsFeatures * gst_caps_features_new_valist (const gchar *feature1, va_list varargs);
GstCapsFeatures * gst_caps_features_new_id (GQuark feature1, ...);
GstCapsFeatures * gst_caps_features_new_id_valist (GQuark feature1, va_list varargs);
gboolean          gst_caps_features_set_parent_refcount  (GstCapsFeatures *features, gint * refcount);
GstCapsFeatures * gst_caps_features_copy (const GstCapsFeatures * features);
void              gst_caps_features_free (GstCapsFeatures * features);
gchar *           gst_caps_features_to_string (const GstCapsFeatures * features);
GstCapsFeatures * gst_caps_features_from_string (const gchar * features);
guint             gst_caps_features_get_size (const GstCapsFeatures * features);
const gchar *     gst_caps_features_get_nth (const GstCapsFeatures * features, guint i);
GQuark            gst_caps_features_get_nth_id (const GstCapsFeatures * features, guint i);
gboolean          gst_caps_features_contains (const GstCapsFeatures * features, const gchar * feature);
gboolean          gst_caps_features_contains_id (const GstCapsFeatures * features, GQuark feature);
gboolean          gst_caps_features_is_equal (const GstCapsFeatures * features1, const GstCapsFeatures * features2);
gboolean          gst_caps_features_is_any (const GstCapsFeatures * features);
void              gst_caps_features_add (GstCapsFeatures * features, const gchar * feature);
void              gst_caps_features_add_id ( GstCapsFeatures * features, GQuark feature);
void              gst_caps_features_remove (GstCapsFeatures * features, const gchar * feature);
void              gst_caps_features_remove_id (GstCapsFeatures * features, GQuark feature);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstCapsFeatures, gst_caps_features_free)
#endif
G_END_DECLS

#endif