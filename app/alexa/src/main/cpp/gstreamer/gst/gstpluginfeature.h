#ifndef __GST_PLUGIN_FEATURE_H__
#define __GST_PLUGIN_FEATURE_H__

#include <glib/glib-object.h>
#include "gstobject.h"
#include "gstplugin.h"

G_BEGIN_DECLS
#define GST_TYPE_PLUGIN_FEATURE                 (gst_plugin_feature_get_type())
#define GST_PLUGIN_FEATURE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_PLUGIN_FEATURE, GstPluginFeature))
#define GST_IS_PLUGIN_FEATURE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_PLUGIN_FEATURE))
#define GST_PLUGIN_FEATURE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_PLUGIN_FEATURE, GstPluginFeatureClass))
#define GST_IS_PLUGIN_FEATURE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_PLUGIN_FEATURE))
#define GST_PLUGIN_FEATURE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_PLUGIN_FEATURE, GstPluginFeatureClass))
#define GST_PLUGIN_FEATURE_CAST(obj)            ((GstPluginFeature*)(obj))
typedef struct _GstPluginFeature GstPluginFeature;
typedef struct _GstPluginFeatureClass GstPluginFeatureClass;
typedef enum {
  GST_RANK_NONE                 = 0,
  GST_RANK_MARGINAL             = 64,
  GST_RANK_SECONDARY            = 128,
  GST_RANK_PRIMARY              = 256
} GstRank;
#define                 gst_plugin_feature_get_name(feature)      GST_OBJECT_NAME(feature)
#define                 gst_plugin_feature_set_name(feature,name) gst_object_set_name(GST_OBJECT_CAST(feature),name)
typedef gboolean        (*GstPluginFeatureFilter)       (GstPluginFeature *feature, gpointer user_data);
GType           gst_plugin_feature_get_type             (void);
GstPluginFeature *gst_plugin_feature_load                 (GstPluginFeature *feature);
void            gst_plugin_feature_set_rank             (GstPluginFeature *feature, guint rank);
guint           gst_plugin_feature_get_rank             (GstPluginFeature *feature);
GstPlugin     * gst_plugin_feature_get_plugin           (GstPluginFeature *feature);
const gchar   * gst_plugin_feature_get_plugin_name      (GstPluginFeature *feature);
void            gst_plugin_feature_list_free            (GList *list);
GList          *gst_plugin_feature_list_copy            (GList *list) G_GNUC_MALLOC;
void            gst_plugin_feature_list_debug           (GList *list);
#ifndef GST_DISABLE_GST_DEBUG
#define GST_PLUGIN_FEATURE_LIST_DEBUG(list) gst_plugin_feature_list_debug(list)
#else
#define GST_PLUGIN_FEATURE_LIST_DEBUG(list)
#endif
gboolean        gst_plugin_feature_check_version(GstPluginFeature *feature, guint min_major, guint min_minor, guint min_micro);
gint            gst_plugin_feature_rank_compare_func(gconstpointer p1, gconstpointer p2);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstPluginFeature, gst_object_unref)
#endif
G_END_DECLS

#endif