#include "../glib/glibintl.h"
#include "gio.h"
#include "gioerror.h"
#include "gfile.h"
#include "config.h"
#include "gappinfo.h"

typedef GAppInfoIface GAppInfoInterface;
G_DEFINE_INTERFACE (GAppInfo, g_app_info, G_TYPE_OBJECT);
static void g_app_info_default_init(GAppInfoInterface *iface) {}
GAppInfo *g_app_info_dup(GAppInfo *appinfo) {
  GAppInfoIface *iface;
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), NULL);
  iface = G_APP_INFO_GET_IFACE (appinfo);
  return (* iface->dup) (appinfo);
}
gboolean g_app_info_equal(GAppInfo *appinfo1, GAppInfo *appinfo2) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo1), FALSE);
  g_return_val_if_fail(G_IS_APP_INFO(appinfo2), FALSE);
  if (G_TYPE_FROM_INSTANCE(appinfo1) != G_TYPE_FROM_INSTANCE(appinfo2)) return FALSE;
  iface = G_APP_INFO_GET_IFACE(appinfo1);
  return (* iface->equal)(appinfo1, appinfo2);
}
const char *g_app_info_get_id(GAppInfo *appinfo) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), NULL);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  return (*iface->get_id)(appinfo);
}
const char *g_app_info_get_name(GAppInfo *appinfo) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), NULL);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  return (*iface->get_name)(appinfo);
}
const char *g_app_info_get_display_name(GAppInfo *appinfo) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), NULL);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  if (iface->get_display_name == NULL) return (*iface->get_name)(appinfo);
  return (*iface->get_display_name)(appinfo);
}
const char *g_app_info_get_description(GAppInfo *appinfo) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), NULL);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  return (*iface->get_description)(appinfo);
}
const char *g_app_info_get_executable(GAppInfo *appinfo) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), NULL);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  return (*iface->get_executable)(appinfo);
}
const char *g_app_info_get_commandline(GAppInfo *appinfo) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), NULL);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  if (iface->get_commandline) return (*iface->get_commandline)(appinfo);
  return NULL;
}
gboolean g_app_info_set_as_default_for_type(GAppInfo *appinfo, const char *content_type, GError **error) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), FALSE);
  g_return_val_if_fail(content_type != NULL, FALSE);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  return (*iface->set_as_default_for_type)(appinfo, content_type, error);
}
gboolean g_app_info_set_as_last_used_for_type(GAppInfo *appinfo, const char *content_type, GError **error) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO (appinfo), FALSE);
  g_return_val_if_fail(content_type != NULL, FALSE);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  return (*iface->set_as_last_used_for_type)(appinfo, content_type, error);
}
gboolean g_app_info_set_as_default_for_extension(GAppInfo *appinfo, const char *extension, GError **error) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), FALSE);
  g_return_val_if_fail(extension != NULL, FALSE);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  if (iface->set_as_default_for_extension) return (*iface->set_as_default_for_extension)(appinfo, extension, error);
  g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"g_app_info_set_as_default_for_extension not supported yet");
  return FALSE;
}
gboolean
g_app_info_add_supports_type(GAppInfo *appinfo, const char *content_type, GError **error) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), FALSE);
  g_return_val_if_fail(content_type != NULL, FALSE);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  if (iface->add_supports_type) return (*iface->add_supports_type)(appinfo, content_type, error);
  g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"g_app_info_add_supports_type not supported yet");
  return FALSE;
}
gboolean g_app_info_can_remove_supports_type(GAppInfo *appinfo) {
  GAppInfoIface *iface;
  g_return_val_if_fail (G_IS_APP_INFO (appinfo), FALSE);
  iface = G_APP_INFO_GET_IFACE (appinfo);
  if (iface->can_remove_supports_type) return (*iface->can_remove_supports_type)(appinfo);
  return FALSE;
}
gboolean g_app_info_remove_supports_type(GAppInfo *appinfo, const char *content_type, GError **error) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), FALSE);
  g_return_val_if_fail(content_type != NULL, FALSE);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  if (iface->remove_supports_type) return (*iface->remove_supports_type)(appinfo, content_type, error);
  g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED,"g_app_info_remove_supports_type not supported yet");
  return FALSE;
}
GIcon *g_app_info_get_icon(GAppInfo *appinfo) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), NULL);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  return (*iface->get_icon)(appinfo);
}
gboolean g_app_info_launch(GAppInfo *appinfo, GList *files, GAppLaunchContext *launch_context, GError **error) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), FALSE);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  return (*iface->launch)(appinfo, files, launch_context, error);
}
gboolean g_app_info_supports_uris(GAppInfo *appinfo) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), FALSE);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  return (*iface->supports_uris)(appinfo);
}
gboolean g_app_info_supports_files(GAppInfo *appinfo) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), FALSE);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  return (*iface->supports_files)(appinfo);
}
gboolean g_app_info_launch_uris(GAppInfo *appinfo, GList *uris, GAppLaunchContext *launch_context, GError **error) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), FALSE);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  return (*iface->launch_uris)(appinfo, uris, launch_context, error);
}
gboolean g_app_info_should_show(GAppInfo *appinfo) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), FALSE);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  return (*iface->should_show)(appinfo);
}
gboolean g_app_info_launch_default_for_uri(const char *uri, GAppLaunchContext *launch_context, GError **error) {
  GAppInfo *app_info;
  GFile *file;
  GList l;
  gboolean res;
  file = g_file_new_for_uri(uri);
  app_info = g_file_query_default_handler(file, NULL, error);
  g_object_unref(file);
  if (app_info == NULL) return FALSE;
  l.data = (char*)uri;
  l.next = l.prev = NULL;
  res = g_app_info_launch_uris(app_info, &l, launch_context, error);
  g_object_unref(app_info);
  return res;
}
gboolean g_app_info_can_delete(GAppInfo *appinfo) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), FALSE);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  if (iface->can_delete) return (*iface->can_delete)(appinfo);
  return FALSE; 
}
gboolean g_app_info_delete(GAppInfo *appinfo) {
  GAppInfoIface *iface;
  g_return_val_if_fail(G_IS_APP_INFO(appinfo), FALSE);
  iface = G_APP_INFO_GET_IFACE(appinfo);
  if (iface->do_delete) return (* iface->do_delete)(appinfo);
  return FALSE; 
}
G_DEFINE_TYPE (GAppLaunchContext, g_app_launch_context, G_TYPE_OBJECT);
GAppLaunchContext *g_app_launch_context_new(void) {
  return g_object_new (G_TYPE_APP_LAUNCH_CONTEXT, NULL);
}
static void g_app_launch_context_class_init(GAppLaunchContextClass *klass) {}
static void g_app_launch_context_init(GAppLaunchContext *launch_context) {}
char *g_app_launch_context_get_display(GAppLaunchContext *context, GAppInfo *info, GList *files) {
  GAppLaunchContextClass *class;
  g_return_val_if_fail(G_IS_APP_LAUNCH_CONTEXT(context), NULL);
  g_return_val_if_fail(G_IS_APP_INFO(info), NULL);
  class = G_APP_LAUNCH_CONTEXT_GET_CLASS(context);
  if (class->get_display == NULL) return NULL;
  return class->get_display(context, info, files);
}
char *g_app_launch_context_get_startup_notify_id(GAppLaunchContext *context, GAppInfo *info, GList *files) {
  GAppLaunchContextClass *class;
  g_return_val_if_fail(G_IS_APP_LAUNCH_CONTEXT(context), NULL);
  g_return_val_if_fail(G_IS_APP_INFO(info), NULL);
  class = G_APP_LAUNCH_CONTEXT_GET_CLASS(context);
  if (class->get_startup_notify_id == NULL) return NULL;
  return class->get_startup_notify_id(context, info, files);
}
void g_app_launch_context_launch_failed(GAppLaunchContext *context, const char *startup_notify_id) {
  GAppLaunchContextClass *class;
  g_return_if_fail(G_IS_APP_LAUNCH_CONTEXT(context));
  g_return_if_fail(startup_notify_id != NULL);
  class = G_APP_LAUNCH_CONTEXT_GET_CLASS(context);
  if (class->launch_failed != NULL) class->launch_failed(context, startup_notify_id);
}