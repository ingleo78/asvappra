#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_APP_INFO_H__
#define __G_APP_INFO_H__

#include "../gobject/gtype.h"
#include "../gobject/gobject.h"
#include "giotypes.h"
#include "gioenums.h"

G_BEGIN_DECLS
#define G_TYPE_APP_INFO  (g_app_info_get_type())
#define G_APP_INFO(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_APP_INFO, GAppInfo))
#define G_IS_APP_INFO(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_APP_INFO))
#define G_APP_INFO_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_APP_INFO, GAppInfoIface))
#define G_TYPE_APP_LAUNCH_CONTEXT  (g_app_launch_context_get_type())
#define G_APP_LAUNCH_CONTEXT(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_APP_LAUNCH_CONTEXT, GAppLaunchContext))
#define G_APP_LAUNCH_CONTEXT_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_APP_LAUNCH_CONTEXT, GAppLaunchContextClass))
#define G_IS_APP_LAUNCH_CONTEXT(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_APP_LAUNCH_CONTEXT))
#define G_IS_APP_LAUNCH_CONTEXT_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_APP_LAUNCH_CONTEXT))
#define G_APP_LAUNCH_CONTEXT_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_APP_LAUNCH_CONTEXT, GAppLaunchContextClass))
typedef struct _GAppLaunchContextClass GAppLaunchContextClass;
typedef struct _GAppLaunchContextPrivate GAppLaunchContextPrivate;
typedef struct _GAppInfoIface    GAppInfoIface;
struct _GAppInfoIface {
  GTypeInterface g_iface;
  GAppInfo *(*dup)(GAppInfo *appinfo);
  gboolean (*equal)(GAppInfo *appinfo1, GAppInfo *appinfo2);
  const char *(*get_id)(GAppInfo *appinfo);
  const char *(*get_name)(GAppInfo *appinfo);
  const char *(*get_description)(GAppInfo *appinfo);
  const char *(*get_executable)(GAppInfo *appinfo);
  GIcon *(*get_icon)(GAppInfo *appinfo);
  gboolean (*launch)(GAppInfo *appinfo, GList *files, GAppLaunchContext *launch_context, GError **error);
  gboolean (*supports_uris)(GAppInfo *appinfo);
  gboolean (*supports_files)(GAppInfo *appinfo);
  gboolean (*launch_uris)(GAppInfo *appinfo, GList *uris, GAppLaunchContext *launch_context, GError **error);
  gboolean (*should_show)(GAppInfo *appinfo);
  gboolean (*set_as_default_for_type)(GAppInfo *appinfo, const char *content_type, GError **error);
  gboolean (*set_as_default_for_extension)(GAppInfo *appinfo, const char *extension, GError **error);
  gboolean (*add_supports_type)(GAppInfo *appinfo, const char *content_type, GError **error);
  gboolean (*can_remove_supports_type)(GAppInfo *appinfo);
  gboolean (*remove_supports_type)(GAppInfo *appinfo, const char *content_type, GError **error);
  gboolean (*can_delete)(GAppInfo *appinfo);
  gboolean (*do_delete)(GAppInfo *appinfo);
  const char *(*get_commandline)(GAppInfo *appinfo);
  const char *(*get_display_name)(GAppInfo *appinfo);
  gboolean (*set_as_last_used_for_type)(GAppInfo *appinfo, const char *content_type, GError **error);
};
GType g_app_info_get_type(void) G_GNUC_CONST;
GAppInfo *g_app_info_create_from_commandline(const char *commandline, const char *application_name, GAppInfoCreateFlags flags, GError **error);
GAppInfo *g_app_info_dup(GAppInfo *appinfo);
gboolean  g_app_info_equal(GAppInfo *appinfo1, GAppInfo *appinfo2);
const char *g_app_info_get_id(GAppInfo *appinfo);
const char *g_app_info_get_name(GAppInfo *appinfo);
const char *g_app_info_get_display_name(GAppInfo *appinfo);
const char *g_app_info_get_description(GAppInfo *appinfo);
const char *g_app_info_get_executable(GAppInfo *appinfo);
const char *g_app_info_get_commandline(GAppInfo *appinfo);
GIcon *g_app_info_get_icon(GAppInfo *appinfo);
gboolean g_app_info_launch(GAppInfo *appinfo, GList *files, GAppLaunchContext *launch_context, GError **error);
gboolean g_app_info_supports_uris(GAppInfo *appinfo);
gboolean g_app_info_supports_files(GAppInfo *appinfo);
gboolean g_app_info_launch_uris(GAppInfo *appinfo, GList *uris, GAppLaunchContext *launch_context, GError **error);
gboolean g_app_info_should_show(GAppInfo *appinfo);
gboolean g_app_info_set_as_default_for_type(GAppInfo *appinfo, const char *content_type, GError **error);
gboolean g_app_info_set_as_default_for_extension(GAppInfo *appinfo, const char *extension, GError **error);
gboolean g_app_info_add_supports_type(GAppInfo *appinfo, const char *content_type, GError **error);
gboolean g_app_info_can_remove_supports_type(GAppInfo *appinfo);
gboolean g_app_info_remove_supports_type(GAppInfo *appinfo, const char *content_type, GError **error);
gboolean g_app_info_can_delete(GAppInfo *appinfo);
gboolean g_app_info_delete(GAppInfo *appinfo);
gboolean g_app_info_set_as_last_used_for_type(GAppInfo *appinfo, const char *content_type, GError **error);
GList *g_app_info_get_all(void);
GList *g_app_info_get_all_for_type(const char *content_type);
GList *g_app_info_get_recommended_for_type(const gchar *content_type);
GList *g_app_info_get_fallback_for_type(const gchar *content_type);
void g_app_info_reset_type_associations(const char *content_type);
GAppInfo *g_app_info_get_default_for_type(const char *content_type, gboolean must_support_uris);
GAppInfo *g_app_info_get_default_for_uri_scheme(const char *uri_scheme);
gboolean  g_app_info_launch_default_for_uri(const char *uri, GAppLaunchContext *launch_context, GError **error);
struct _GAppLaunchContext {
  GObject parent_instance;
  GAppLaunchContextPrivate *priv;
};
struct _GAppLaunchContextClass {
  GObjectClass parent_class;
  char *(*get_display)(GAppLaunchContext *context, GAppInfo *info, GList *files);
  char *(*get_startup_notify_id)(GAppLaunchContext *context, GAppInfo *info, GList *files);
  void (*launch_failed)(GAppLaunchContext *context, const char *startup_notify_id);
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
GType g_app_launch_context_get_type(void) G_GNUC_CONST;
GAppLaunchContext *g_app_launch_context_new(void);
char *g_app_launch_context_get_display(GAppLaunchContext *context, GAppInfo *info, GList *files);
char *g_app_launch_context_get_startup_notify_id(GAppLaunchContext *context, GAppInfo *info, GList *files);
void g_app_launch_context_launch_failed(GAppLaunchContext *context, const char *startup_notify_id);
G_END_DECLS

#endif