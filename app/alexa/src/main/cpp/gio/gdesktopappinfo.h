#ifndef __G_DESKTOP_APP_INFO_H__
#define __G_DESKTOP_APP_INFO_H__

#include "../gobject/gobject.h"
#include "gio.h"

G_BEGIN_DECLS
#define G_TYPE_DESKTOP_APP_INFO  (g_desktop_app_info_get_type ())
#define G_DESKTOP_APP_INFO(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_DESKTOP_APP_INFO, GDesktopAppInfo))
#define G_DESKTOP_APP_INFO_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_DESKTOP_APP_INFO, GDesktopAppInfoClass))
#define G_IS_DESKTOP_APP_INFO(o)  (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_DESKTOP_APP_INFO))
#define G_IS_DESKTOP_APP_INFO_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_DESKTOP_APP_INFO))
#define G_DESKTOP_APP_INFO_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), G_TYPE_DESKTOP_APP_INFO, GDesktopAppInfoClass))
typedef struct _GDesktopAppInfo GDesktopAppInfo;
typedef struct _GDesktopAppInfoClass GDesktopAppInfoClass;
struct _GDesktopAppInfoClass {
  GObjectClass parent_class;
};
GType g_desktop_app_info_get_type(void) G_GNUC_CONST;
GDesktopAppInfo *g_desktop_app_info_new_from_filename(const char *filename);
GDesktopAppInfo *g_desktop_app_info_new_from_keyfile(GKeyFile *key_file);
const char *g_desktop_app_info_get_filename(GDesktopAppInfo *info);
GDesktopAppInfo *g_desktop_app_info_new(const char *desktop_id);
gboolean g_desktop_app_info_get_is_hidden(GDesktopAppInfo *info);
void g_desktop_app_info_set_desktop_env(const char *desktop_env);
#ifndef G_DISABLE_DEPRECATED
#define G_TYPE_DESKTOP_APP_INFO_LOOKUP  (g_desktop_app_info_lookup_get_type())
#define G_DESKTOP_APP_INFO_LOOKUP(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_DESKTOP_APP_INFO_LOOKUP, GDesktopAppInfoLookup))
#define G_IS_DESKTOP_APP_INFO_LOOKUP(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_DESKTOP_APP_INFO_LOOKUP))
#define G_DESKTOP_APP_INFO_LOOKUP_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_DESKTOP_APP_INFO_LOOKUP, GDesktopAppInfoLookupIface))
#define G_DESKTOP_APP_INFO_LOOKUP_EXTENSION_POINT_NAME "gio-desktop-app-info-lookup"
typedef struct _GDesktopAppInfoLookup GDesktopAppInfoLookup;
typedef struct _GDesktopAppInfoLookupIface GDesktopAppInfoLookupIface;
struct _GDesktopAppInfoLookupIface {
  GTypeInterface g_iface;
  GAppInfo *(*get_default_for_uri_scheme)(GDesktopAppInfoLookup *lookup, const char *uri_scheme);
};
GType g_desktop_app_info_lookup_get_type(void) G_GNUC_CONST;
GAppInfo *g_desktop_app_info_lookup_get_default_for_uri_scheme(GDesktopAppInfoLookup *lookup, const char *uri_scheme);
typedef void (*GDesktopAppLaunchCallback)(GDesktopAppInfo *appinfo, GPid pid, gpointer user_data);
gboolean g_desktop_app_info_launch_uris_as_manager(GDesktopAppInfo *appinfo, GList *uris, GAppLaunchContext *launch_context, GSpawnFlags spawn_flags,
						                           GSpawnChildSetupFunc user_setup, gpointer user_setup_data, GDesktopAppLaunchCallback pid_callback,
						                           gpointer pid_callback_data, GError **error);
#endif
G_END_DECLS

#endif