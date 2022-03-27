#ifndef __GST_PLUGIN_H__
#define __GST_PLUGIN_H__

#include "gstconfig.h"
#include "gstobject.h"
#include "gstmacros.h"
#include "gststructure.h"

G_BEGIN_DECLS
typedef struct _GstPlugin GstPlugin;
typedef struct _GstPluginClass GstPluginClass;
typedef struct _GstPluginDesc GstPluginDesc;
GQuark gst_plugin_error_quark (void);
#define GST_PLUGIN_ERROR gst_plugin_error_quark ()
typedef enum {
  GST_PLUGIN_ERROR_MODULE,
  GST_PLUGIN_ERROR_DEPENDENCIES,
  GST_PLUGIN_ERROR_NAME_MISMATCH
} GstPluginError;
typedef enum {
  GST_PLUGIN_FLAG_CACHED      = (GST_OBJECT_FLAG_LAST << 0),
  GST_PLUGIN_FLAG_BLACKLISTED = (GST_OBJECT_FLAG_LAST << 1)
} GstPluginFlags;
typedef enum {
  GST_PLUGIN_DEPENDENCY_FLAG_NONE = 0,
  GST_PLUGIN_DEPENDENCY_FLAG_RECURSE = (1 << 0),
  GST_PLUGIN_DEPENDENCY_FLAG_PATHS_ARE_DEFAULT_ONLY = (1 << 1),
  GST_PLUGIN_DEPENDENCY_FLAG_FILE_NAME_IS_SUFFIX = (1 << 2),
  GST_PLUGIN_DEPENDENCY_FLAG_FILE_NAME_IS_PREFIX = (1 << 3)
} GstPluginDependencyFlags;
typedef gboolean (*GstPluginInitFunc) (GstPlugin *plugin);
typedef gboolean (*GstPluginInitFullFunc) (GstPlugin *plugin, gpointer user_data);
struct _GstPluginDesc {
  gint major_version;
  gint minor_version;
  const gchar *name;
  const gchar *description;
  GstPluginInitFunc plugin_init;
  const gchar *version;
  const gchar *license;
  const gchar *source;
  const gchar *package;
  const gchar *origin;
  const gchar *release_datetime;
  gpointer _gst_reserved[GST_PADDING];
};
#define GST_TYPE_PLUGIN   (gst_plugin_get_type())
#define GST_IS_PLUGIN(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_PLUGIN))
#define GST_IS_PLUGIN_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_PLUGIN))
#define GST_PLUGIN_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_PLUGIN, GstPluginClass))
#define GST_PLUGIN(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_PLUGIN, GstPlugin))
#define GST_PLUGIN_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_PLUGIN, GstPluginClass))
#define GST_PLUGIN_CAST(obj)           ((GstPlugin*)(obj))
#ifdef GST_PACKAGE_RELEASE_DATETIME
#define __GST_PACKAGE_RELEASE_DATETIME GST_PACKAGE_RELEASE_DATETIME
#else
#define __GST_PACKAGE_RELEASE_DATETIME NULL
#endif
#define GST_PLUGIN_STATIC_DECLARE(name)  extern void G_PASTE(gst_plugin_, G_PASTE(name, _register)) (void)
#define GST_PLUGIN_STATIC_REGISTER(name) G_PASTE(gst_plugin_, G_PASTE(name, _register)) ()
#ifdef GST_PLUGIN_BUILD_STATIC
#define GST_PLUGIN_DEFINE(major,minor,name,description,init,version,license,package,origin)	\
G_BEGIN_DECLS						\
GST_PLUGIN_EXPORT void G_PASTE(gst_plugin_, G_PASTE(name, _register)) (void);			\
void G_PASTE(gst_plugin_, G_PASTE(name, _register)) (void) {							\
  gst_plugin_register_static (major, minor, G_STRINGIFY(name), description, init, version, license, PACKAGE, package, origin);			\
}							\
G_END_DECLS
#else
#define GST_PLUGIN_DEFINE(major,minor,name,description,init,version,license,package,origin)	\
G_BEGIN_DECLS \
GST_PLUGIN_EXPORT GstPluginDesc gst_plugin_desc = {	\
  major,						\
  minor,						\
  G_STRINGIFY(name),                                    \
  (gchar *) description,				\
  init,							\
  version,						\
  license,						\
  PACKAGE,						\
  package,						\
  origin,						\
  __GST_PACKAGE_RELEASE_DATETIME,                       \
  GST_PADDING_INIT				        \
}; \
G_END_DECLS
#endif
#define GST_LICENSE_UNKNOWN "unknown"
typedef gboolean        (*GstPluginFilter)              (GstPlugin *plugin, gpointer user_data);
GType                   gst_plugin_get_type             (void);
gboolean gst_plugin_register_static(gint major_version, gint minor_version, const gchar *name, const gchar *description,
                                    GstPluginInitFunc init_func, const gchar *version, const gchar *license, const gchar *source,
                                    const gchar *package, const gchar *origin);
gboolean gst_plugin_register_static_full(gint major_version, gint minor_version, const gchar *name, const gchar *description,
                                         GstPluginInitFullFunc init_full_func, const gchar *version, const gchar *license, const gchar *source,
                                         const gchar *package, const gchar *origin, gpointer user_data);
const gchar*		gst_plugin_get_name(GstPlugin *plugin);
const gchar*		gst_plugin_get_description	(GstPlugin *plugin);
const gchar*		gst_plugin_get_filename		(GstPlugin *plugin);
const gchar*		gst_plugin_get_version		(GstPlugin *plugin);
const gchar*		gst_plugin_get_license		(GstPlugin *plugin);
const gchar*		gst_plugin_get_source		(GstPlugin *plugin);
const gchar*		gst_plugin_get_package		(GstPlugin *plugin);
const gchar*		gst_plugin_get_origin		(GstPlugin *plugin);
const gchar*		gst_plugin_get_release_date_string (GstPlugin *plugin);
const GstStructure*	gst_plugin_get_cache_data	(GstPlugin * plugin);
void			gst_plugin_set_cache_data	(GstPlugin * plugin, GstStructure *cache_data);
gboolean		gst_plugin_is_loaded		(GstPlugin *plugin);
GstPlugin *		gst_plugin_load_file		(const gchar *filename, GError** error);
GstPlugin *             gst_plugin_load                 (GstPlugin *plugin);
GstPlugin *             gst_plugin_load_by_name         (const gchar *name);
void gst_plugin_add_dependency(GstPlugin *plugin, const gchar **env_vars, const gchar **paths, const gchar **names,
                               GstPluginDependencyFlags flags);
void gst_plugin_add_dependency_simple(GstPlugin *plugin, const gchar *env_vars, const gchar *paths, const gchar *names,
                                      GstPluginDependencyFlags flags);
void gst_plugin_list_free (GList *list);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstPlugin, gst_object_unref)
#endif
G_END_DECLS

#endif