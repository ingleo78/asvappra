#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#ifdef HAVE_CRT_EXTERNS_H
#include <crt_externs.h>
#endif
#undef G_DISABLE_DEPRECATED
#include "../glib/glibintl.h"
#include "../glib/gstdio.h"
#include "../glib/glib.h"
#include "config.h"
#include "gcontenttypeprivate.h"
#include "gdesktopappinfo.h"
#include "gfile.h"
#include "gioerror.h"
#include "gthemedicon.h"
#include "gfileicon.h"
#include "giomodule-priv.h"
#include "gappinfo.h"

#define DEFAULT_APPLICATIONS_GROUP  "Default Applications"
#define ADDED_ASSOCIATIONS_GROUP  "Added Associations"
#define REMOVED_ASSOCIATIONS_GROUP  "Removed Associations"
#define MIME_CACHE_GROUP  "MIME Cache"
#define FULL_NAME_KEY  "X-GNOME-FullName"
static void g_desktop_app_info_iface_init(GAppInfoIface *iface);
static GList *get_all_desktop_entries_for_mime_type(const char *base_mime_type, const char **except, gboolean include_fallback, char **explicit_default);
static void mime_info_cache_reload(const char *dir);
static gboolean g_desktop_app_info_ensure_saved(GDesktopAppInfo *info, GError **error);
struct _GDesktopAppInfo {
  GObject parent_instance;
  char *desktop_id;
  char *filename;
  char *name;
  char *fullname;
  char *comment;
  char *icon_name;
  GIcon *icon;
  char **only_show_in;
  char **not_show_in;
  char *try_exec;
  char *exec;
  char *binary;
  char *path;
  guint nodisplay : 1;
  guint hidden : 1;
  guint terminal : 1;
  guint startup_notify : 1;
  guint no_fuse : 1;
};
typedef enum {
  UPDATE_MIME_NONE = 1 << 0,
  UPDATE_MIME_SET_DEFAULT = 1 << 1,
  UPDATE_MIME_SET_NON_DEFAULT = 1 << 2,
  UPDATE_MIME_REMOVE = 1 << 3,
  UPDATE_MIME_SET_LAST_USED = 1 << 4,
} UpdateMimeFlags;
G_DEFINE_TYPE_WITH_CODE(GDesktopAppInfo, g_desktop_app_info, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE (G_TYPE_APP_INFO, g_desktop_app_info_iface_init));
static gpointer search_path_init(gpointer data) {
  char **args = NULL;
  const char * const *data_dirs;
  const char *user_data_dir;
  int i, length, j;
  data_dirs = g_get_system_data_dirs ();
  length = g_strv_length((char**)data_dirs);
  args = g_new(char*,length + 2);
  j = 0;
  user_data_dir = g_get_user_data_dir();
  args[j++] = g_build_filename(user_data_dir, "applications", NULL);
  for (i = 0; i < length; i++) args[j++] = g_build_filename(data_dirs[i], "applications", NULL);
  args[j++] = NULL;
  return args;
}
static const char * const *get_applications_search_path(void) {
  static GOnce once_init = G_ONCE_INIT;
  return g_once(&once_init, search_path_init, NULL);
}
static void g_desktop_app_info_finalize(GObject *object) {
  GDesktopAppInfo *info;
  info = G_DESKTOP_APP_INFO(object);
  g_free(info->desktop_id);
  g_free(info->filename);
  g_free(info->name);
  g_free(info->fullname);
  g_free(info->comment);
  g_free(info->icon_name);
  if (info->icon) g_object_unref(info->icon);
  g_strfreev(info->only_show_in);
  g_strfreev(info->not_show_in);
  g_free(info->try_exec);
  g_free(info->exec);
  g_free(info->binary);
  g_free(info->path);
  G_OBJECT_CLASS(g_desktop_app_info_parent_class)->finalize(object);
}
static void g_desktop_app_info_class_init(GDesktopAppInfoClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_desktop_app_info_finalize;
}
static void g_desktop_app_info_init(GDesktopAppInfo *local) {}
static char *binary_from_exec(const char *exec) {
  const char *p, *start;
  p = exec;
  while(*p == ' ') p++;
  start = p;
  while(*p != ' ' && *p != 0) p++;
  return g_strndup(start, p - start);
}
GDesktopAppInfo *g_desktop_app_info_new_from_keyfile(GKeyFile *key_file) {
  GDesktopAppInfo *info = NULL;
  char *start_group;
  char *type = NULL;
  char *try_exec;
  start_group = g_key_file_get_start_group(key_file);
  if (start_group == NULL || strcmp(start_group, G_KEY_FILE_DESKTOP_GROUP) != 0) {
      g_free(start_group);
      return NULL;
  }
  g_free(start_group);
  type = g_key_file_get_string(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TYPE, NULL);
  if (type == NULL || strcmp(type, G_KEY_FILE_DESKTOP_TYPE_APPLICATION) != 0) {
      g_free(type);
      return NULL;
  }
  g_free(type);
  try_exec = g_key_file_get_string(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TRY_EXEC, NULL);
  if (try_exec && try_exec[0] != '\0') {
      char *t;
      t = g_find_program_in_path(try_exec);
      if (t == NULL) {
          g_free(try_exec);
          return NULL;
	  }
      g_free(t);
  }
  info = g_object_new(G_TYPE_DESKTOP_APP_INFO, NULL);
  info->filename = NULL;
  info->name = g_key_file_get_locale_string(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
  info->fullname = g_key_file_get_locale_string(key_file, G_KEY_FILE_DESKTOP_GROUP, FULL_NAME_KEY, NULL, NULL);
  info->comment = g_key_file_get_locale_string(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_COMMENT, NULL, NULL);
  info->nodisplay = g_key_file_get_boolean(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NO_DISPLAY, NULL) != FALSE;
  info->icon_name =  g_key_file_get_locale_string(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL, NULL);
  info->only_show_in = g_key_file_get_string_list(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ONLY_SHOW_IN, NULL, NULL);
  info->not_show_in = g_key_file_get_string_list(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NOT_SHOW_IN, NULL, NULL);
  info->try_exec = try_exec;
  info->exec = g_key_file_get_string(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
  info->path = g_key_file_get_string(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_PATH, NULL);
  info->terminal = g_key_file_get_boolean(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TERMINAL, NULL) != FALSE;
  info->startup_notify = g_key_file_get_boolean(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_STARTUP_NOTIFY, NULL) != FALSE;
  info->no_fuse = g_key_file_get_boolean(key_file, G_KEY_FILE_DESKTOP_GROUP, "X-GIO-NoFuse", NULL) != FALSE;
  info->hidden = g_key_file_get_boolean(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_HIDDEN, NULL) != FALSE;
  info->icon = NULL;
  if (info->icon_name) {
      if (g_path_is_absolute(info->icon_name)) {
          GFile *file;
          file = g_file_new_for_path(info->icon_name);
          info->icon = g_file_icon_new(file);
          g_object_unref(file);
	  } else {
          char *p;
          if ((p = strrchr(info->icon_name, '.')) != NULL && (strcmp(p, ".png") == 0 || strcmp(p, ".xpm") == 0 || strcmp(p, ".svg") == 0)) *p = 0;
          info->icon = g_themed_icon_new(info->icon_name);
      }
  }
  if (info->exec) info->binary = binary_from_exec(info->exec);
  if (info->path && info->path[0] == '\0') {
      g_free(info->path);
      info->path = NULL;
  }
  return info;
}
GDesktopAppInfo *g_desktop_app_info_new_from_filename(const char *filename) {
  GKeyFile *key_file;
  GDesktopAppInfo *info = NULL;
  key_file = g_key_file_new();
  if (g_key_file_load_from_file(key_file, filename,G_KEY_FILE_NONE, NULL)) {
      info = g_desktop_app_info_new_from_keyfile(key_file);
      if (info) info->filename = g_strdup(filename);
  }
  g_key_file_free(key_file);
  return info;
}
GDesktopAppInfo *g_desktop_app_info_new(const char *desktop_id) {
  GDesktopAppInfo *appinfo;
  const char * const *dirs;
  char *basename;
  int i;
  dirs = get_applications_search_path();
  basename = g_strdup(desktop_id);
  for (i = 0; dirs[i] != NULL; i++) {
      char *filename;
      char *p;
      filename = g_build_filename(dirs[i], desktop_id, NULL);
      appinfo = g_desktop_app_info_new_from_filename(filename);
      g_free(filename);
      if (appinfo != NULL) goto found;
      p = basename;
      while((p = strchr(p, '-')) != NULL) {
          *p = '/';
          filename = g_build_filename(dirs[i], basename, NULL);
          appinfo = g_desktop_app_info_new_from_filename(filename);
          g_free(filename);
          if (appinfo != NULL) goto found;
          *p = '-';
          p++;
	  }
  }
  g_free(basename);
  return NULL;
found:
  g_free(basename);
  appinfo->desktop_id = g_strdup(desktop_id);
  if (g_desktop_app_info_get_is_hidden(appinfo)) {
      g_object_unref(appinfo);
      appinfo = NULL;
  }
  return appinfo;
}
static GAppInfo *g_desktop_app_info_dup(GAppInfo *appinfo) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  GDesktopAppInfo *new_info;
  new_info = g_object_new(G_TYPE_DESKTOP_APP_INFO, NULL);
  new_info->filename = g_strdup(info->filename);
  new_info->desktop_id = g_strdup(info->desktop_id);
  new_info->name = g_strdup(info->name);
  new_info->fullname = g_strdup(info->fullname);
  new_info->comment = g_strdup(info->comment);
  new_info->nodisplay = info->nodisplay;
  new_info->icon_name = g_strdup(info->icon_name);
  if (info->icon) new_info->icon = g_object_ref(info->icon);
  new_info->only_show_in = g_strdupv(info->only_show_in);
  new_info->not_show_in = g_strdupv(info->not_show_in);
  new_info->try_exec = g_strdup(info->try_exec);
  new_info->exec = g_strdup(info->exec);
  new_info->binary = g_strdup(info->binary);
  new_info->path = g_strdup(info->path);
  new_info->hidden = info->hidden;
  new_info->terminal = info->terminal;
  new_info->startup_notify = info->startup_notify;
  return G_APP_INFO(new_info);
}
static gboolean g_desktop_app_info_equal(GAppInfo *appinfo1, GAppInfo *appinfo2) {
  GDesktopAppInfo *info1 = G_DESKTOP_APP_INFO(appinfo1);
  GDesktopAppInfo *info2 = G_DESKTOP_APP_INFO(appinfo2);
  if (info1->desktop_id == NULL || info2->desktop_id == NULL) return info1 == info2;
  return strcmp(info1->desktop_id, info2->desktop_id) == 0;
}
static const char *g_desktop_app_info_get_id(GAppInfo *appinfo) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  return info->desktop_id;
}
static const char *g_desktop_app_info_get_name(GAppInfo *appinfo) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  if (info->name == NULL) return "Unnamed";
  return info->name;
}
static const char *g_desktop_app_info_get_display_name(GAppInfo *appinfo) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  if (info->fullname == NULL) return g_desktop_app_info_get_name(appinfo);
  return info->fullname;
}
gboolean g_desktop_app_info_get_is_hidden(GDesktopAppInfo *info) {
  return info->hidden;
}
const char *g_desktop_app_info_get_filename(GDesktopAppInfo *info) {
  return info->filename;
}
static const char *g_desktop_app_info_get_description(GAppInfo *appinfo) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  return info->comment;
}
static const char *g_desktop_app_info_get_executable(GAppInfo *appinfo) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  return info->binary;
}
static const char *g_desktop_app_info_get_commandline(GAppInfo *appinfo) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  return info->exec;
}
static GIcon *g_desktop_app_info_get_icon(GAppInfo *appinfo) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  return info->icon;
}
static char *expand_macro_single(char macro, char *uri) {
  GFile *file;
  char *result = NULL;
  char *path, *name;
  file = g_file_new_for_uri(uri);
  path = g_file_get_path(file);
  g_object_unref(file);
  switch(macro) {
      case 'u': case 'U': result = g_shell_quote(uri); break;
      case 'f': case 'F':
          if (path) result = g_shell_quote(path);
          break;
      case 'd': case 'D':
          if (path) {
              name = g_path_get_dirname(path);
              result = g_shell_quote(name);
              g_free(name);
          }
          break;
      case 'n': case 'N':
          if (path) {
              name = g_path_get_basename(path);
              result = g_shell_quote(name);
              g_free(name);
          }
          break;
  }
  g_free(path);
  return result;
}
static void expand_macro(char macro, GString *exec, GDesktopAppInfo *info, GList **uri_list) {
  GList *uris = *uri_list;
  char *expanded;
  gboolean force_file_uri;
  char force_file_uri_macro;
  char *uri;
  g_return_if_fail(exec != NULL);
  force_file_uri_macro = macro;
  force_file_uri = FALSE;
  if (!info->no_fuse) {
      switch(macro) {
          case 'u':
              force_file_uri_macro = 'f';
              force_file_uri = TRUE;
              break;
          case 'U':
              force_file_uri_macro = 'F';
              force_file_uri = TRUE;
              break;
	  }
  }
  switch(macro) {
      case 'u': case 'f': case 'd': case 'n':
          if (uris) {
              uri = uris->data;
              if (!force_file_uri || strchr (uri, '#') != NULL) expanded = expand_macro_single(macro, uri);
              else {
                  expanded = expand_macro_single(force_file_uri_macro, uri);
                  if (expanded == NULL) expanded = expand_macro_single(macro, uri);
              }
              if (expanded) {
                  g_string_append(exec, expanded);
                  g_free(expanded);
              }
              uris = uris->next;
          }
          break;
      case 'U': case 'F': case 'D': case 'N':
          while(uris) {
              uri = uris->data;
              if (!force_file_uri || strchr (uri, '#') != NULL) expanded = expand_macro_single(macro, uri);
              else {
                  expanded = expand_macro_single(force_file_uri_macro, uri);
                  if (expanded == NULL) expanded = expand_macro_single(macro, uri);
              }
              if (expanded) {
                  g_string_append(exec, expanded);
                  g_free(expanded);
              }
              uris = uris->next;
              if (uris != NULL && expanded) g_string_append_c(exec, ' ');
          }
          break;
      case 'i':
          if (info->icon_name) {
              g_string_append(exec, "--icon ");
              expanded = g_shell_quote(info->icon_name);
              g_string_append(exec, expanded);
              g_free(expanded);
          }
          break;
      case 'c':
          if (info->name) {
              expanded = g_shell_quote(info->name);
              g_string_append(exec, expanded);
              g_free(expanded);
          }
          break;
      case 'k':
          if (info->filename) {
              expanded = g_shell_quote(info->filename);
              g_string_append(exec, expanded);
              g_free(expanded);
          }
          break;
      case 'm': break;
      case '%': g_string_append_c(exec, '%'); break;
  }
  *uri_list = uris;
}
static gboolean expand_application_parameters(GDesktopAppInfo *info, GList **uris, int *argc, char ***argv, GError **error) {
  GList *uri_list = *uris;
  const char *p = info->exec;
  GString *expanded_exec;
  gboolean res;
  if (info->exec == NULL) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_FAILED,"Desktop file didn't specify Exec field");
      return FALSE;
  }
  expanded_exec = g_string_new (NULL);
  while(*p) {
      if (p[0] == '%' && p[1] != '\0') {
          expand_macro(p[1], expanded_exec, info, uris);
          p++;
	  } else g_string_append_c(expanded_exec, *p);
      p++;
  }
  if (uri_list == *uris && uri_list != NULL) {
      g_string_append_c(expanded_exec, ' ');
      expand_macro('f', expanded_exec, info, uris);
  }
  res = g_shell_parse_argv(expanded_exec->str, argc, argv, error);
  g_string_free(expanded_exec, TRUE);
  return res;
}
static gboolean prepend_terminal_to_vector(int *argc, char ***argv) {
#ifndef G_OS_WIN32
  char **real_argv;
  int real_argc;
  int i, j;
  char **term_argv = NULL;
  int term_argc = 0;
  char *check;
  char **the_argv;
  g_return_val_if_fail(argc != NULL, FALSE);
  g_return_val_if_fail(argv != NULL, FALSE);
  if(*argv == NULL) *argc = 0;
  the_argv = *argv;
  if (*argc < 0) {
      for (i = 0; the_argv[i] != NULL; i++);
      *argc = i;
  }
  term_argc = 2;
  term_argv = g_new0(char *, 3);
  check = g_find_program_in_path("gnome-terminal");
  if (check != NULL) {
      term_argv[0] = check;
      term_argv[1] = g_strdup("-x");
  } else {
      if (check == NULL) check = g_find_program_in_path("nxterm");
      if (check == NULL) check = g_find_program_in_path("color-xterm");
      if (check == NULL) check = g_find_program_in_path("rxvt");
      if (check == NULL) check = g_find_program_in_path("xterm");
      if (check == NULL) check = g_find_program_in_path("dtterm");
      if (check == NULL) {
          check = g_strdup("xterm");
          g_warning("couldn't find a terminal, falling back to xterm");
      }
      term_argv[0] = check;
      term_argv[1] = g_strdup ("-e");
  }
  real_argc = term_argc + *argc;
  real_argv = g_new (char *, real_argc + 1);
  for (i = 0; i < term_argc; i++) real_argv[i] = term_argv[i];
  for (j = 0; j < *argc; j++, i++) real_argv[i] = (char *)the_argv[j];
  real_argv[i] = NULL;
  g_free(*argv);
  *argv = real_argv;
  *argc = real_argc;
  g_free(term_argv);
  return TRUE;
#else
  return FALSE;
#endif
}
static GList *create_files_for_uris(GList *uris) {
  GList *res;
  GList *iter;
  res = NULL;
  for (iter = uris; iter; iter = iter->next) {
      GFile *file = g_file_new_for_uri((char*)iter->data);
      res = g_list_prepend(res, file);
  }
  return g_list_reverse(res);
}
typedef struct {
  GSpawnChildSetupFunc user_setup;
  gpointer user_setup_data;
  char *display;
  char *sn_id;
  char *desktop_file;
} ChildSetupData;
static void child_setup(gpointer user_data) {
  ChildSetupData *data = user_data;
  if (data->display) g_setenv ("DISPLAY", data->display, TRUE);
  if (data->sn_id) g_setenv ("DESKTOP_STARTUP_ID", data->sn_id, TRUE);
  if (data->desktop_file) {
      gchar pid[20];
      g_setenv ("GIO_LAUNCHED_DESKTOP_FILE", data->desktop_file, TRUE);
      g_snprintf (pid, 20, "%ld", (long)getpid ());
      g_setenv ("GIO_LAUNCHED_DESKTOP_FILE_PID", pid, TRUE);
  }
  if (data->user_setup) data->user_setup (data->user_setup_data);
}
static void notify_desktop_launch(GDBusConnection *session_bus, GDesktopAppInfo *info, long pid, const char *display, const char *sn_id, GList *uris) {
  GDBusMessage *msg;
  GVariantBuilder uri_variant;
  GVariantBuilder extras_variant;
  GList *iter;
  const char *desktop_file_id;
  const char *gio_desktop_file;
  if (session_bus == NULL) return;
  g_variant_builder_init(&uri_variant, G_VARIANT_TYPE("as"));
  for (iter = uris; iter; iter = iter->next) g_variant_builder_add(&uri_variant, "s", iter->data);
  g_variant_builder_init(&extras_variant, G_VARIANT_TYPE("a{sv}"));
  if (sn_id != NULL && g_utf8_validate(sn_id, -1, NULL))
      g_variant_builder_add(&extras_variant, "{sv}", "startup-id", g_variant_new("s", sn_id));
  gio_desktop_file = g_getenv("GIO_LAUNCHED_DESKTOP_FILE");
  if (gio_desktop_file != NULL)
      g_variant_builder_add(&extras_variant, "{sv}", "origin-desktop-file", g_variant_new_bytestring(gio_desktop_file));
  if (g_get_prgname() != NULL)
      g_variant_builder_add(&extras_variant, "{sv}", "origin-prgname", g_variant_new_bytestring(g_get_prgname()));
  g_variant_builder_add(&extras_variant, "{sv}", "origin-pid", g_variant_new("x", (gint64)getpid()));
  if (info->filename) desktop_file_id = info->filename;
  else if (info->desktop_id) desktop_file_id = info->desktop_id;
  else desktop_file_id = "";
  msg = g_dbus_message_new_signal("/org/gtk/gio/DesktopAppInfo","org.gtk.gio.DesktopAppInfo","Launched");
  g_dbus_message_set_body(msg, g_variant_new("(@aysxasa{sv})", g_variant_new_bytestring(desktop_file_id), display ? display : "", (gint64)pid,
					      &uri_variant, &extras_variant));
  g_dbus_connection_send_message(session_bus, msg,0,NULL,NULL);
  g_object_unref(msg);
}
#define _SPAWN_FLAGS_DEFAULT (G_SPAWN_SEARCH_PATH)
static gboolean _g_desktop_app_info_launch_uris_internal(GAppInfo *appinfo, GList *uris, GAppLaunchContext *launch_context, GSpawnFlags spawn_flags,
                                                         GSpawnChildSetupFunc user_setup, gpointer user_setup_data, GDesktopAppLaunchCallback pid_callback,
                                                         gpointer pid_callback_data, GError **error) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  GDBusConnection *session_bus;
  gboolean completed = FALSE;
  GList *old_uris;
  char **argv;
  int argc;
  ChildSetupData data;
  g_return_val_if_fail(appinfo != NULL, FALSE);
  argv = NULL;
  session_bus = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
  do {
      GPid pid;
      GList *launched_uris;
      GList *iter;
      old_uris = uris;
      if (!expand_application_parameters(info, &uris, &argc, &argv, error)) goto out;
      launched_uris = NULL;
      for (iter = old_uris; iter != NULL && iter != uris; iter = iter->next) launched_uris = g_list_prepend(launched_uris, iter->data);
      launched_uris = g_list_reverse(launched_uris);
      if (info->terminal && !prepend_terminal_to_vector(&argc, &argv)) {
          g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_FAILED,"Unable to find terminal required for application");
          goto out;
	  }
      data.user_setup = user_setup;
      data.user_setup_data = user_setup_data;
      data.display = NULL;
      data.sn_id = NULL;
      data.desktop_file = info->filename;
      if (launch_context) {
          GList *launched_files = create_files_for_uris(launched_uris);
          data.display = g_app_launch_context_get_display(launch_context, appinfo, launched_files);
          if (info->startup_notify) data.sn_id = g_app_launch_context_get_startup_notify_id(launch_context, appinfo, launched_files);
          g_list_foreach(launched_files, (GFunc)g_object_unref, NULL);
          g_list_free(launched_files);
	  }
      if (!g_spawn_async(info->path, argv, NULL, spawn_flags, child_setup, &data, &pid, error)) {
          if (data.sn_id) g_app_launch_context_launch_failed(launch_context, data.sn_id);
          g_free(data.sn_id);
          g_free(data.display);
          g_list_free(launched_uris);
          goto out;
	  }
      if (pid_callback != NULL) pid_callback(info, pid, pid_callback_data);
      notify_desktop_launch(session_bus, info, pid, data.display, data.sn_id, launched_uris);
      g_free(data.sn_id);
      g_free(data.display);
      g_list_free(launched_uris);
      g_strfreev(argv);
      argv = NULL;
  } while (uris != NULL);
  if (session_bus != NULL) {
      g_dbus_connection_flush(session_bus, NULL, NULL, NULL);
      g_object_unref(session_bus);
  }
  completed = TRUE;
out:
  g_strfreev(argv);
  return completed;
}
static gboolean g_desktop_app_info_launch_uris(GAppInfo *appinfo, GList *uris, GAppLaunchContext *launch_context, GError **error) {
  return _g_desktop_app_info_launch_uris_internal(appinfo, uris, launch_context, _SPAWN_FLAGS_DEFAULT, NULL, NULL, NULL, NULL, error);
}
static gboolean g_desktop_app_info_supports_uris(GAppInfo *appinfo) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  return info->exec && ((strstr(info->exec, "%u") != NULL) || (strstr(info->exec, "%U") != NULL));
}
static gboolean g_desktop_app_info_supports_files(GAppInfo *appinfo) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  return info->exec && ((strstr(info->exec, "%f") != NULL) || (strstr(info->exec, "%F") != NULL));
}
static gboolean g_desktop_app_info_launch(GAppInfo *appinfo, GList *files, GAppLaunchContext *launch_context, GError **error) {
  GList *uris;
  char *uri;
  gboolean res;
  uris = NULL;
  while(files) {
      uri = g_file_get_uri(files->data);
      uris = g_list_prepend(uris, uri);
      files = files->next;
  }
  uris = g_list_reverse(uris);
  res = g_desktop_app_info_launch_uris(appinfo, uris, launch_context, error);
  g_list_foreach(uris, (GFunc)g_free, NULL);
  g_list_free(uris);
  return res;
}
gboolean g_desktop_app_info_launch_uris_as_manager(GDesktopAppInfo *appinfo, GList *uris, GAppLaunchContext *launch_context, GSpawnFlags spawn_flags,
                                                   GSpawnChildSetupFunc user_setup, gpointer user_setup_data, GDesktopAppLaunchCallback pid_callback,
                                                   gpointer pid_callback_data, GError **error) {
  return _g_desktop_app_info_launch_uris_internal((GAppInfo*)appinfo, uris, launch_context, spawn_flags, user_setup, user_setup_data, pid_callback,
                                                  pid_callback_data, error);
}

G_LOCK_DEFINE_STATIC(g_desktop_env);
static gchar *g_desktop_env = NULL;
void g_desktop_app_info_set_desktop_env(const gchar *desktop_env) {
  G_LOCK(g_desktop_env);
  if (!g_desktop_env) g_desktop_env = g_strdup(desktop_env);
  G_UNLOCK(g_desktop_env);
}
static gboolean g_desktop_app_info_should_show(GAppInfo *appinfo) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  gboolean found;
  const gchar *desktop_env;
  int i;
  if (info->nodisplay) return FALSE;
  G_LOCK (g_desktop_env);
  desktop_env = g_desktop_env;
  G_UNLOCK (g_desktop_env);
  if (info->only_show_in) {
      if (desktop_env == NULL) return FALSE;
      found = FALSE;
      for (i = 0; info->only_show_in[i] != NULL; i++) {
          if (strcmp(info->only_show_in[i], desktop_env) == 0) {
              found = TRUE;
              break;
          }
      }
      if (!found)
	return FALSE;
  }
  if (info->not_show_in && desktop_env) {
      for (i = 0; info->not_show_in[i] != NULL; i++) {
	      if (strcmp(info->not_show_in[i], desktop_env) == 0) return FALSE;
	  }
  }
  return TRUE;
}
typedef enum {
  APP_DIR,
  MIMETYPE_DIR
} DirType;
static char *ensure_dir(DirType type, GError **error) {
  char *path, *display_name;
  int errsv;
  if (type == APP_DIR) path = g_build_filename(g_get_user_data_dir(), "applications", NULL);
  else path = g_build_filename(g_get_user_data_dir(), "mime", "packages", NULL);
  errno = 0;
  if (g_mkdir_with_parents(path, 0700) == 0) return path;
  errsv = errno;
  display_name = g_filename_display_name(path);
  if (type == APP_DIR) {
      g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv),"Can't create user application configuration folder %s: %s", display_name,
                  g_strerror(errsv));
  } else {
      g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv),"Can't create user MIME configuration folder %s: %s", display_name,
                  g_strerror(errsv));
  }
  g_free(display_name);
  g_free(path);
  return NULL;
}
static gboolean update_mimeapps_list(const char *desktop_id, const char *content_type, UpdateMimeFlags flags, GError **error) {
  char *dirname, *filename, *string;
  GKeyFile *key_file;
  gboolean load_succeeded, res, explicit_default;
  char **old_list, **list;
  GList *system_list;
  gsize length, data_size;
  char *data;
  int i, j, k;
  char **content_types;
  g_assert(!((flags & UPDATE_MIME_SET_DEFAULT) && (flags & UPDATE_MIME_SET_NON_DEFAULT)));
  dirname = ensure_dir(APP_DIR, error);
  if (!dirname) return FALSE;
  filename = g_build_filename(dirname, "mimeapps.list", NULL);
  g_free(dirname);
  key_file = g_key_file_new();
  load_succeeded = g_key_file_load_from_file(key_file, filename, G_KEY_FILE_NONE, NULL);
  if (!load_succeeded || !g_key_file_has_group(key_file, ADDED_ASSOCIATIONS_GROUP)) {
      g_key_file_free(key_file);
      key_file = g_key_file_new();
  }
  if (content_type) {
      content_types = g_new(char*,2);
      content_types[0] = g_strdup(content_type);
      content_types[1] = NULL;
  } else content_types = g_key_file_get_keys(key_file, DEFAULT_APPLICATIONS_GROUP, NULL, NULL);
  explicit_default = FALSE;
  for (k = 0; content_types && content_types[k]; k++) {
      string = g_key_file_get_string(key_file, DEFAULT_APPLICATIONS_GROUP, content_types[k],NULL);
      if (g_strcmp0(string, desktop_id) != 0 && (flags & UPDATE_MIME_SET_DEFAULT)) {
          g_free(string);
          string = g_strdup(desktop_id);
          flags |= UPDATE_MIME_SET_NON_DEFAULT;
      }
      if (string == NULL || desktop_id == NULL) g_key_file_remove_key(key_file, DEFAULT_APPLICATIONS_GROUP, content_types[k],NULL);
      else {
          g_key_file_set_string(key_file, DEFAULT_APPLICATIONS_GROUP, content_types[k], string);
          explicit_default = TRUE;
      }
      g_free(string);
  }
  if (content_type);
  else {
      g_strfreev(content_types);
      content_types = g_key_file_get_keys(key_file, ADDED_ASSOCIATIONS_GROUP, NULL, NULL);
  }
  for (k = 0; content_types && content_types[k]; k++) {
      length = 0;
      old_list = g_key_file_get_string_list(key_file, ADDED_ASSOCIATIONS_GROUP, content_types[k], &length, NULL);
      list = g_new(char *, 1 + length + 1);
      i = 0;
      if (flags & UPDATE_MIME_SET_LAST_USED) {
          if (flags & UPDATE_MIME_SET_NON_DEFAULT) flags ^= UPDATE_MIME_SET_NON_DEFAULT;
          list[i++] = g_strdup(desktop_id);
      }
      if (old_list) {
          for (j = 0; old_list[j] != NULL; j++) {
              if (g_strcmp0(old_list[j], desktop_id) != 0) list[i++] = g_strdup(old_list[j]);
              else if (flags & UPDATE_MIME_SET_NON_DEFAULT) {
                  flags ^= UPDATE_MIME_SET_NON_DEFAULT;
                  list[i++] = g_strdup(old_list[j]);
              }
	      }
      }
      if (flags & UPDATE_MIME_SET_NON_DEFAULT) list[i++] = g_strdup(desktop_id);
      list[i] = NULL;
      g_strfreev(old_list);
      if (list[0] == NULL || desktop_id == NULL) g_key_file_remove_key(key_file, ADDED_ASSOCIATIONS_GROUP, content_types[k],NULL);
      else {
          g_key_file_set_string_list(key_file, ADDED_ASSOCIATIONS_GROUP, content_types[k], (const char * const *)list, i);
          if (!explicit_default) {
              system_list = get_all_desktop_entries_for_mime_type(content_type, (const char**)list,FALSE,NULL);
              if (system_list != NULL) {
                  string = system_list->data;
                  g_key_file_set_string(key_file, DEFAULT_APPLICATIONS_GROUP, content_types[k], string);
              }
              g_list_free_full(system_list, g_free);
          }
      }
      g_strfreev(list);
  }
  if (content_type);
  else {
      g_strfreev(content_types);
      content_types = g_key_file_get_keys(key_file, REMOVED_ASSOCIATIONS_GROUP, NULL, NULL);
  }
  for (k = 0; content_types && content_types[k]; k++) {
      length = 0;
      old_list = g_key_file_get_string_list(key_file, REMOVED_ASSOCIATIONS_GROUP, content_types[k], &length, NULL);
      list = g_new(char *, 1 + length + 1);
      i = 0;
      if (flags & UPDATE_MIME_REMOVE) list[i++] = g_strdup(desktop_id);
      if (old_list) {
          for (j = 0; old_list[j] != NULL; j++) {
              if (g_strcmp0(old_list[j], desktop_id) != 0) list[i++] = g_strdup(old_list[j]);
	      }
      }
      list[i] = NULL;
      g_strfreev (old_list);
      if (list[0] == NULL || desktop_id == NULL) g_key_file_remove_key (key_file, REMOVED_ASSOCIATIONS_GROUP, content_types[k],NULL);
      else g_key_file_set_string_list(key_file, REMOVED_ASSOCIATIONS_GROUP, content_types[k], (const char * const *)list, i);
      g_strfreev(list);
  }
  g_strfreev(content_types);
  data = g_key_file_to_data(key_file, &data_size, error);
  g_key_file_free(key_file);
  res = g_file_set_contents(filename, data, data_size, error);
  mime_info_cache_reload(NULL);
  g_free(filename);
  g_free(data);
  return res;
}
static gboolean g_desktop_app_info_set_as_last_used_for_type(GAppInfo *appinfo, const char *content_type, GError **error) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  if (!g_desktop_app_info_ensure_saved(info, error)) return FALSE;
  return update_mimeapps_list(info->desktop_id, content_type,UPDATE_MIME_SET_NON_DEFAULT | UPDATE_MIME_SET_LAST_USED, error);
}
static gboolean g_desktop_app_info_set_as_default_for_type(GAppInfo *appinfo, const char *content_type, GError **error) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  if (!g_desktop_app_info_ensure_saved(info, error)) return FALSE;
  return update_mimeapps_list(info->desktop_id, content_type,UPDATE_MIME_SET_DEFAULT, error);
}
static void update_program_done(GPid pid, gint status, gpointer data) {}
static void run_update_command(char *command, char *subdir) {
	char *argv[3] = {
		NULL,
		NULL,
		NULL,
	};
	GPid pid = 0;
	GError *error = NULL;
	argv[0] = command;
	argv[1] = g_build_filename(g_get_user_data_dir(), subdir, NULL);
	if (g_spawn_async("/", argv, NULL, G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL | G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL,
		&pid, &error)) {
	    g_child_watch_add(pid, update_program_done, NULL);
	} else g_warning("%s", error->message);
	g_free(argv[1]);
}
static gboolean g_desktop_app_info_set_as_default_for_extension(GAppInfo *appinfo, const char *extension, GError **error) {
  char *filename, *basename, *mimetype;
  char *dirname;
  gboolean res;
  if (!g_desktop_app_info_ensure_saved(G_DESKTOP_APP_INFO(appinfo), error)) return FALSE;
  dirname = ensure_dir(MIMETYPE_DIR, error);
  if (!dirname) return FALSE;
  basename = g_strdup_printf("user-extension-%s.xml", extension);
  filename = g_build_filename(dirname, basename, NULL);
  g_free(basename);
  g_free(dirname);
  mimetype = g_strdup_printf("application/x-extension-%s", extension);
  if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
      char *contents;
      contents = g_strdup_printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<mime-info xmlns=\"http://www.freedesktop.org/standards/shared-mime-info"
                                 "\">\n <mime-type type=\"%s\">\n  <comment>%s document</comment>\n  <glob pattern=\"*.%s\"/>\n </mime-type>\n</mime-info>\n",
                                 mimetype, extension, extension);
      g_file_set_contents(filename, contents, -1, NULL);
      g_free(contents);
      run_update_command("update-mime-database", "mime");
  }
  g_free(filename);
  res = g_desktop_app_info_set_as_default_for_type(appinfo, mimetype, error);
  g_free(mimetype);
  return res;
}
static gboolean g_desktop_app_info_add_supports_type(GAppInfo *appinfo, const char *content_type, GError **error) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  if (!g_desktop_app_info_ensure_saved(G_DESKTOP_APP_INFO(info), error)) return FALSE;
  return update_mimeapps_list(info->desktop_id, content_type,UPDATE_MIME_SET_NON_DEFAULT, error);
}
static gboolean g_desktop_app_info_can_remove_supports_type(GAppInfo *appinfo) {
  return TRUE;
}
static gboolean g_desktop_app_info_remove_supports_type(GAppInfo *appinfo, const char *content_type, GError **error) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  if (!g_desktop_app_info_ensure_saved(G_DESKTOP_APP_INFO(info), error)) return FALSE;
  return update_mimeapps_list(info->desktop_id, content_type, UPDATE_MIME_REMOVE, error);
}
static gboolean g_desktop_app_info_ensure_saved(GDesktopAppInfo *info, GError **error) {
  GKeyFile *key_file;
  char *dirname;
  char *filename;
  char *data, *desktop_id;
  gsize data_size;
  int fd;
  gboolean res;
  if (info->filename != NULL) return TRUE;
  dirname = ensure_dir(APP_DIR, error);
  if (!dirname) return FALSE;
  key_file = g_key_file_new();
  g_key_file_set_string(key_file, G_KEY_FILE_DESKTOP_GROUP, "Encoding", "UTF-8");
  g_key_file_set_string(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_VERSION, "1.0");
  g_key_file_set_string(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TYPE, G_KEY_FILE_DESKTOP_TYPE_APPLICATION);
  if (info->terminal) g_key_file_set_boolean(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TERMINAL, TRUE);
  g_key_file_set_string(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, info->exec);
  g_key_file_set_string(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, info->name);
  if (info->fullname != NULL) g_key_file_set_string(key_file, G_KEY_FILE_DESKTOP_GROUP, FULL_NAME_KEY, info->fullname);
  g_key_file_set_string(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_COMMENT, info->comment);
  g_key_file_set_boolean(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NO_DISPLAY, TRUE);
  data = g_key_file_to_data(key_file, &data_size, NULL);
  g_key_file_free(key_file);
  desktop_id = g_strdup_printf("userapp-%s-XXXXXX.desktop", info->name);
  filename = g_build_filename(dirname, desktop_id, NULL);
  g_free(desktop_id);
  g_free(dirname);
  fd = g_mkstemp(filename);
  if (fd == -1) {
      char *display_name;
      display_name = g_filename_display_name(filename);
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED,"Can't create user desktop file %s", display_name);
      g_free(display_name);
      g_free(filename);
      g_free(data);
      return FALSE;
  }
  desktop_id = g_path_get_basename(filename);
  close(fd);
  res = g_file_set_contents(filename, data, data_size, error);
  if (!res) {
      g_free(desktop_id);
      g_free(filename);
      return FALSE;
  }
  info->filename = filename;
  info->desktop_id = desktop_id;
  run_update_command("update-desktop-database", "applications");
  return TRUE;
}
static gboolean g_desktop_app_info_can_delete(GAppInfo *appinfo) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO (appinfo);
  if (info->filename) {
      if (strstr(info->filename, "/userapp-")) return g_access(info->filename, W_OK) == 0;
  }
  return FALSE;
}
static gboolean g_desktop_app_info_delete(GAppInfo *appinfo) {
  GDesktopAppInfo *info = G_DESKTOP_APP_INFO(appinfo);
  if (info->filename) {
      if (g_remove (info->filename) == 0) {
          update_mimeapps_list(info->desktop_id,NULL,UPDATE_MIME_NONE,NULL);
          g_free(info->filename);
          info->filename = NULL;
          g_free(info->desktop_id);
          info->desktop_id = NULL;
          return TRUE;
      }
  }
  return FALSE;
}
GAppInfo *g_app_info_create_from_commandline(const char *commandline, const char *application_name, GAppInfoCreateFlags flags, GError **error) {
  char **split;
  char *basename;
  GDesktopAppInfo *info;
  g_return_val_if_fail(commandline, NULL);
  info = g_object_new(G_TYPE_DESKTOP_APP_INFO, NULL);
  info->filename = NULL;
  info->desktop_id = NULL;
  info->terminal = flags & G_APP_INFO_CREATE_NEEDS_TERMINAL;
  info->startup_notify = flags & G_APP_INFO_CREATE_SUPPORTS_STARTUP_NOTIFICATION;
  info->hidden = FALSE;
  if (flags & G_APP_INFO_CREATE_SUPPORTS_URIS) info->exec = g_strconcat(commandline, " %u", NULL);
  else info->exec = g_strconcat(commandline, " %f", NULL);
  info->nodisplay = TRUE;
  info->binary = binary_from_exec(info->exec);
  if (application_name) info->name = g_strdup(application_name);
  else {
      split = g_strsplit(commandline, " ", 2);
      basename = split[0] ? g_path_get_basename(split[0]) : NULL;
      g_strfreev(split);
      info->name = basename;
      if (info->name == NULL) info->name = g_strdup("custom");
  }
  info->comment = g_strdup_printf("Custom definition for %s", info->name);
  return G_APP_INFO(info);
}
static void g_desktop_app_info_iface_init(GAppInfoIface *iface) {
  iface->dup = g_desktop_app_info_dup;
  iface->equal = g_desktop_app_info_equal;
  iface->get_id = g_desktop_app_info_get_id;
  iface->get_name = g_desktop_app_info_get_name;
  iface->get_description = g_desktop_app_info_get_description;
  iface->get_executable = g_desktop_app_info_get_executable;
  iface->get_icon = g_desktop_app_info_get_icon;
  iface->launch = g_desktop_app_info_launch;
  iface->supports_uris = g_desktop_app_info_supports_uris;
  iface->supports_files = g_desktop_app_info_supports_files;
  iface->launch_uris = g_desktop_app_info_launch_uris;
  iface->should_show = g_desktop_app_info_should_show;
  iface->set_as_default_for_type = g_desktop_app_info_set_as_default_for_type;
  iface->set_as_default_for_extension = g_desktop_app_info_set_as_default_for_extension;
  iface->add_supports_type = g_desktop_app_info_add_supports_type;
  iface->can_remove_supports_type = g_desktop_app_info_can_remove_supports_type;
  iface->remove_supports_type = g_desktop_app_info_remove_supports_type;
  iface->can_delete = g_desktop_app_info_can_delete;
  iface->do_delete = g_desktop_app_info_delete;
  iface->get_commandline = g_desktop_app_info_get_commandline;
  iface->get_display_name = g_desktop_app_info_get_display_name;
  iface->set_as_last_used_for_type = g_desktop_app_info_set_as_last_used_for_type;
}
static gboolean app_info_in_list(GAppInfo *info, GList *list) {
  while(list != NULL) {
      if (g_app_info_equal(info, list->data)) return TRUE;
      list = list->next;
  }
  return FALSE;
}
GList *g_app_info_get_recommended_for_type(const gchar *content_type) {
  GList *desktop_entries, *l;
  GList *infos;
  GDesktopAppInfo *info;
  g_return_val_if_fail(content_type != NULL, NULL);
  desktop_entries = get_all_desktop_entries_for_mime_type(content_type, NULL, FALSE, NULL);
  infos = NULL;
  for (l = desktop_entries; l != NULL; l = l->next) {
      char *desktop_entry = l->data;
      info = g_desktop_app_info_new(desktop_entry);
      if (info) {
          if (app_info_in_list(G_APP_INFO(info), infos)) g_object_unref(info);
          else infos = g_list_prepend(infos, info);
	  }
      g_free(desktop_entry);
  }
  g_list_free(desktop_entries);
  return g_list_reverse(infos);
}
GList *g_app_info_get_fallback_for_type(const gchar *content_type) {
  GList *desktop_entries, *l;
  GList *infos, *recommended_infos;
  GDesktopAppInfo *info;
  g_return_val_if_fail(content_type != NULL, NULL);
  desktop_entries = get_all_desktop_entries_for_mime_type(content_type, NULL, TRUE, NULL);
  recommended_infos = g_app_info_get_recommended_for_type(content_type);
  infos = NULL;
  for (l = desktop_entries; l != NULL; l = l->next) {
      char *desktop_entry = l->data;
      info = g_desktop_app_info_new(desktop_entry);
      if (info) {
          if (app_info_in_list(G_APP_INFO(info), infos) || app_info_in_list(G_APP_INFO(info), recommended_infos)) g_object_unref(info);
          else infos = g_list_prepend(infos, info);
	  }
      g_free(desktop_entry);
  }
  g_list_free(desktop_entries);
  g_list_free_full(recommended_infos, g_object_unref);
  return g_list_reverse(infos);
}
GList *g_app_info_get_all_for_type(const char *content_type) {
  GList *desktop_entries, *l;
  GList *infos;
  char *user_default = NULL;
  GDesktopAppInfo *info;
  g_return_val_if_fail(content_type != NULL, NULL);
  desktop_entries = get_all_desktop_entries_for_mime_type(content_type, NULL, TRUE, &user_default);
  infos = NULL;
  if (user_default != NULL) {
      info = g_desktop_app_info_new(user_default);
      if (info != NULL) infos = g_list_prepend(infos, info);
  }
  g_free(user_default);
  for (l = desktop_entries; l != NULL; l = l->next) {
      char *desktop_entry = l->data;
      info = g_desktop_app_info_new(desktop_entry);
      if (info) {
          if (app_info_in_list(G_APP_INFO(info), infos)) g_object_unref(info);
          else infos = g_list_prepend(infos, info);
	  }
      g_free(desktop_entry);
  }
  g_list_free(desktop_entries);
  return g_list_reverse(infos);
}
void g_app_info_reset_type_associations(const char *content_type) {
  update_mimeapps_list(NULL, content_type,UPDATE_MIME_NONE,NULL);
}
GAppInfo *g_app_info_get_default_for_type(const char *content_type, gboolean must_support_uris) {
  GList *desktop_entries, *l;
  char *user_default = NULL;
  GAppInfo *info;
  g_return_val_if_fail(content_type != NULL, NULL);
  desktop_entries = get_all_desktop_entries_for_mime_type(content_type, NULL, TRUE, &user_default);
  info = NULL;
  if (user_default != NULL) {
      info = (GAppInfo*)g_desktop_app_info_new(user_default);
      if (info) {
          if (must_support_uris && !g_app_info_supports_uris(info)) {
              g_object_unref(info);
              info = NULL;
          }
      }
  }
  g_free(user_default);
  if (info != NULL) {
      g_list_free_full(desktop_entries, g_free);
      return info;
  }
  for (l = desktop_entries; l != NULL; l = l->next) {
      char *desktop_entry = l->data;
      info = (GAppInfo*)g_desktop_app_info_new(desktop_entry);
      if (info) {
          if (must_support_uris && !g_app_info_supports_uris(info)) {
              g_object_unref(info);
              info = NULL;
          } else break;
	  }
  }
  g_list_free_full(desktop_entries, g_free);
  return info;
}
GAppInfo *g_app_info_get_default_for_uri_scheme(const char *uri_scheme) {
  GAppInfo *app_info;
  char *content_type, *scheme_down;
  scheme_down = g_ascii_strdown(uri_scheme, -1);
  content_type = g_strdup_printf("x-scheme-handler/%s", scheme_down);
  g_free(scheme_down);
  app_info = g_app_info_get_default_for_type(content_type, FALSE);
  g_free(content_type);
  return app_info;
}
static void get_apps_from_dir(GHashTable *apps, const char *dirname, const char *prefix) {
  GDir *dir;
  const char *basename;
  char *filename, *subprefix, *desktop_id;
  gboolean hidden;
  GDesktopAppInfo *appinfo;
  dir = g_dir_open(dirname, 0, NULL);
  if (dir) {
      while((basename = g_dir_read_name(dir)) != NULL) {
          filename = g_build_filename(dirname, basename, NULL);
          if (g_str_has_suffix(basename, ".desktop")) {
              desktop_id = g_strconcat(prefix, basename, NULL);
              if (!g_hash_table_lookup_extended(apps, desktop_id, NULL, NULL)) {
                  appinfo = g_desktop_app_info_new_from_filename(filename);
                  hidden = FALSE;
                  if (appinfo && g_desktop_app_info_get_is_hidden(appinfo)) {
                      g_object_unref(appinfo);
                      appinfo = NULL;
                      hidden = TRUE;
                  }
                  if (appinfo || hidden) {
                      g_hash_table_insert(apps, g_strdup(desktop_id), appinfo);
                      if (appinfo) {
                          appinfo->desktop_id = desktop_id;
                          desktop_id = NULL;
                      }
                  }
              }
              g_free(desktop_id);
          } else {
              if (g_file_test(filename, G_FILE_TEST_IS_DIR)) {
                  subprefix = g_strconcat(prefix, basename, "-", NULL);
                  get_apps_from_dir(apps, filename, subprefix);
                  g_free(subprefix);
              }
          }
          g_free(filename);
      }
      g_dir_close(dir);
  }
}
GList *g_app_info_get_all(void) {
  const char * const *dirs;
  GHashTable *apps;
  GHashTableIter iter;
  gpointer value;
  int i;
  GList *infos;
  dirs = get_applications_search_path();
  apps = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  for (i = 0; dirs[i] != NULL; i++) get_apps_from_dir(apps, dirs[i], "");
  infos = NULL;
  g_hash_table_iter_init(&iter, apps);
  while(g_hash_table_iter_next(&iter, NULL, &value)) {
      if (value) infos = g_list_prepend(infos, value);
  }
  g_hash_table_destroy(apps);
  return g_list_reverse(infos);
}
typedef struct {
  char *path;
  GHashTable *mime_info_cache_map;
  GHashTable *defaults_list_map;
  GHashTable *mimeapps_list_added_map;
  GHashTable *mimeapps_list_removed_map;
  GHashTable *mimeapps_list_defaults_map;
  time_t mime_info_cache_timestamp;
  time_t defaults_list_timestamp;
  time_t mimeapps_list_timestamp;
} MimeInfoCacheDir;
typedef struct {
  GList *dirs;
  GHashTable *global_defaults_cache;
  time_t last_stat_time;
  guint should_ping_mime_monitor : 1;
} MimeInfoCache;
static MimeInfoCache *mime_info_cache = NULL;
G_LOCK_DEFINE_STATIC(mime_info_cache);
static void mime_info_cache_dir_add_desktop_entries(MimeInfoCacheDir  *dir, const char *mime_type, char **new_desktop_file_ids);
static MimeInfoCache *mime_info_cache_new(void);
static void destroy_info_cache_value(gpointer key, GList *value, gpointer data) {
  g_list_foreach(value, (GFunc)g_free, NULL);
  g_list_free(value);
}
static void destroy_info_cache_map(GHashTable *info_cache_map) {
  g_hash_table_foreach(info_cache_map, (GHFunc)destroy_info_cache_value, NULL);
  g_hash_table_destroy(info_cache_map);
}
static gboolean mime_info_cache_dir_out_of_date(MimeInfoCacheDir *dir, const char *cache_file, time_t *timestamp) {
  struct stat buf;
  char *filename;
  filename = g_build_filename(dir->path, cache_file, NULL);
  if (g_stat(filename, &buf) < 0) {
      g_free(filename);
      return TRUE;
  }
  g_free(filename);
  if (buf.st_mtime != *timestamp) return TRUE;
  return FALSE;
}
static gboolean remove_all(gpointer key, gpointer value, gpointer user_data) {
  return TRUE;
}
static void mime_info_cache_blow_global_cache(void) {
  g_hash_table_foreach_remove(mime_info_cache->global_defaults_cache, remove_all, NULL);
}
static void mime_info_cache_dir_init(MimeInfoCacheDir *dir) {
  GError *load_error;
  GKeyFile *key_file;
  gchar *filename, **mime_types;
  int i;
  struct stat buf;
  load_error = NULL;
  mime_types = NULL;
  if (dir->mime_info_cache_map != NULL && !mime_info_cache_dir_out_of_date(dir, "mimeinfo.cache", &dir->mime_info_cache_timestamp)) return;
  if (dir->mime_info_cache_map != NULL) destroy_info_cache_map(dir->mime_info_cache_map);
  dir->mime_info_cache_map = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)g_free,NULL);
  key_file = g_key_file_new();
  filename = g_build_filename(dir->path, "mimeinfo.cache", NULL);
  if (g_stat(filename, &buf) < 0) goto error;
  if (dir->mime_info_cache_timestamp > 0) mime_info_cache->should_ping_mime_monitor = TRUE;
  dir->mime_info_cache_timestamp = buf.st_mtime;
  g_key_file_load_from_file(key_file, filename, G_KEY_FILE_NONE, &load_error);
  g_free(filename);
  filename = NULL;
  if (load_error != NULL) goto error;
  mime_types = g_key_file_get_keys(key_file, MIME_CACHE_GROUP,NULL, &load_error);
  if (load_error != NULL) goto error;
  for (i = 0; mime_types[i] != NULL; i++) {
      gchar **desktop_file_ids;
      char *unaliased_type;
      desktop_file_ids = g_key_file_get_string_list(key_file, MIME_CACHE_GROUP, mime_types[i],NULL,NULL);
      if (desktop_file_ids == NULL) continue;
      unaliased_type = _g_unix_content_type_unalias(mime_types[i]);
      mime_info_cache_dir_add_desktop_entries(dir, unaliased_type, desktop_file_ids);
      g_free(unaliased_type);
      g_strfreev(desktop_file_ids);
  }
  g_strfreev(mime_types);
  g_key_file_free(key_file);
  return;
error:
  g_free(filename);
  g_key_file_free(key_file);
  if (mime_types != NULL) g_strfreev(mime_types);
  if (load_error) g_error_free(load_error);
}
static void mime_info_cache_dir_init_defaults_list(MimeInfoCacheDir *dir) {
  GKeyFile *key_file;
  GError *load_error;
  gchar *filename, **mime_types;
  char *unaliased_type;
  char **desktop_file_ids;
  int i;
  struct stat buf;
  load_error = NULL;
  mime_types = NULL;
  if (dir->defaults_list_map != NULL && !mime_info_cache_dir_out_of_date(dir, "defaults.list", &dir->defaults_list_timestamp)) return;
  if (dir->defaults_list_map != NULL) g_hash_table_destroy(dir->defaults_list_map);
  dir->defaults_list_map = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_strfreev);
  key_file = g_key_file_new();
  filename = g_build_filename(dir->path, "defaults.list", NULL);
  if (g_stat(filename, &buf) < 0) goto error;
  if (dir->defaults_list_timestamp > 0)mime_info_cache->should_ping_mime_monitor = TRUE;
  dir->defaults_list_timestamp = buf.st_mtime;
  g_key_file_load_from_file(key_file, filename, G_KEY_FILE_NONE, &load_error);
  g_free(filename);
  filename = NULL;
  if (load_error != NULL) goto error;
  mime_types = g_key_file_get_keys(key_file, DEFAULT_APPLICATIONS_GROUP,NULL, NULL);
  if (mime_types != NULL) {
      for (i = 0; mime_types[i] != NULL; i++) {
          desktop_file_ids = g_key_file_get_string_list(key_file, DEFAULT_APPLICATIONS_GROUP, mime_types[i],NULL,NULL);
          if (desktop_file_ids == NULL) continue;
          unaliased_type = _g_unix_content_type_unalias(mime_types[i]);
          g_hash_table_replace(dir->defaults_list_map, unaliased_type,desktop_file_ids);
	  }
      g_strfreev(mime_types);
  }
  g_key_file_free(key_file);
  return;
error:
  g_free(filename);
  g_key_file_free(key_file);
  if (mime_types != NULL) g_strfreev(mime_types);
  if (load_error) g_error_free(load_error);
}
static void mime_info_cache_dir_init_mimeapps_list(MimeInfoCacheDir *dir) {
  GKeyFile *key_file;
  GError *load_error;
  gchar *filename, **mime_types;
  char *unaliased_type;
  char **desktop_file_ids;
  char *desktop_id;
  int i;
  struct stat buf;
  load_error = NULL;
  mime_types = NULL;
  if (dir->mimeapps_list_added_map != NULL && !mime_info_cache_dir_out_of_date(dir, "mimeapps.list", &dir->mimeapps_list_timestamp)) return;
  if (dir->mimeapps_list_added_map != NULL) g_hash_table_destroy(dir->mimeapps_list_added_map);
  dir->mimeapps_list_added_map = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_strfreev);
  if (dir->mimeapps_list_removed_map != NULL) g_hash_table_destroy(dir->mimeapps_list_removed_map);
  dir->mimeapps_list_removed_map = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_strfreev);
  if (dir->mimeapps_list_defaults_map != NULL) g_hash_table_destroy(dir->mimeapps_list_defaults_map);
  dir->mimeapps_list_defaults_map = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  key_file = g_key_file_new();
  filename = g_build_filename (dir->path, "mimeapps.list", NULL);
  if (g_stat (filename, &buf) < 0) goto error;
  if (dir->mimeapps_list_timestamp > 0) mime_info_cache->should_ping_mime_monitor = TRUE;
  dir->mimeapps_list_timestamp = buf.st_mtime;
  g_key_file_load_from_file(key_file, filename, G_KEY_FILE_NONE, &load_error);
  g_free(filename);
  filename = NULL;
  if (load_error != NULL) goto error;
  mime_types = g_key_file_get_keys(key_file, ADDED_ASSOCIATIONS_GROUP, NULL, NULL);
  if (mime_types != NULL) {
      for (i = 0; mime_types[i] != NULL; i++) {
          desktop_file_ids = g_key_file_get_string_list(key_file, ADDED_ASSOCIATIONS_GROUP, mime_types[i],NULL,NULL);
          if (desktop_file_ids == NULL) continue;
          unaliased_type = _g_unix_content_type_unalias(mime_types[i]);
          g_hash_table_replace(dir->mimeapps_list_added_map, unaliased_type, desktop_file_ids);
	  }
      g_strfreev(mime_types);
  }
  mime_types = g_key_file_get_keys(key_file, REMOVED_ASSOCIATIONS_GROUP,NULL, NULL);
  if (mime_types != NULL) {
      for (i = 0; mime_types[i] != NULL; i++) {
          desktop_file_ids = g_key_file_get_string_list(key_file, REMOVED_ASSOCIATIONS_GROUP, mime_types[i],NULL, NULL);
          if (desktop_file_ids == NULL) continue;
          unaliased_type = _g_unix_content_type_unalias(mime_types[i]);
          g_hash_table_replace(dir->mimeapps_list_removed_map, unaliased_type, desktop_file_ids);
	  }
      g_strfreev(mime_types);
  }
  mime_types = g_key_file_get_keys(key_file, DEFAULT_APPLICATIONS_GROUP,NULL, NULL);
  if (mime_types != NULL) {
      for (i = 0; mime_types[i] != NULL; i++) {
          desktop_id = g_key_file_get_string(key_file, DEFAULT_APPLICATIONS_GROUP, mime_types[i],NULL);
          if (desktop_id == NULL) continue;
          unaliased_type = _g_unix_content_type_unalias(mime_types[i]);
          g_hash_table_replace(dir->mimeapps_list_defaults_map, unaliased_type, desktop_id);
      }
      g_strfreev(mime_types);
  }
  g_key_file_free(key_file);
  return;
error:
  g_free(filename);
  g_key_file_free(key_file);
  if (mime_types != NULL) g_strfreev(mime_types);
  if (load_error) g_error_free(load_error);
}
static MimeInfoCacheDir *mime_info_cache_dir_new(const char *path) {
  MimeInfoCacheDir *dir;
  dir = g_new0(MimeInfoCacheDir, 1);
  dir->path = g_strdup(path);
  return dir;
}
static void mime_info_cache_dir_free(MimeInfoCacheDir *dir) {
  if (dir == NULL) return;
  if (dir->mime_info_cache_map != NULL) {
      destroy_info_cache_map(dir->mime_info_cache_map);
      dir->mime_info_cache_map = NULL;
  }
  if (dir->defaults_list_map != NULL) {
      g_hash_table_destroy(dir->defaults_list_map);
      dir->defaults_list_map = NULL;
  }
  if (dir->mimeapps_list_added_map != NULL) {
      g_hash_table_destroy(dir->mimeapps_list_added_map);
      dir->mimeapps_list_added_map = NULL;
  }
  if (dir->mimeapps_list_removed_map != NULL) {
      g_hash_table_destroy(dir->mimeapps_list_removed_map);
      dir->mimeapps_list_removed_map = NULL;
  }
  if (dir->mimeapps_list_defaults_map != NULL) {
      g_hash_table_destroy(dir->mimeapps_list_defaults_map);
      dir->mimeapps_list_defaults_map = NULL;
  }
  g_free(dir);
}
static void mime_info_cache_dir_add_desktop_entries (MimeInfoCacheDir *dir, const char *mime_type, char **new_desktop_file_ids) {
  GList *desktop_file_ids;
  int i;
  desktop_file_ids = g_hash_table_lookup(dir->mime_info_cache_map, mime_type);
  for (i = 0; new_desktop_file_ids[i] != NULL; i++) {
      if (!g_list_find_custom(desktop_file_ids, new_desktop_file_ids[i], (GCompareFunc)strcmp))
	      desktop_file_ids = g_list_append(desktop_file_ids, g_strdup(new_desktop_file_ids[i]));
  }
  g_hash_table_insert(dir->mime_info_cache_map, g_strdup(mime_type), desktop_file_ids);
}
static void mime_info_cache_init_dir_lists(void) {
  const char * const *dirs;
  int i;
  mime_info_cache = mime_info_cache_new();
  dirs = get_applications_search_path();
  for (i = 0; dirs[i] != NULL; i++){
      MimeInfoCacheDir *dir;
      dir = mime_info_cache_dir_new(dirs[i]);
      if (dir != NULL) {
	  mime_info_cache_dir_init(dir);
	  mime_info_cache_dir_init_defaults_list(dir);
	  mime_info_cache_dir_init_mimeapps_list(dir);
	  mime_info_cache->dirs = g_list_append(mime_info_cache->dirs, dir);
	  }
  }
}
static void mime_info_cache_update_dir_lists(void) {
  GList *tmp;
  tmp = mime_info_cache->dirs;
  while(tmp != NULL) {
      MimeInfoCacheDir *dir = (MimeInfoCacheDir*)tmp->data;
      mime_info_cache_blow_global_cache();
      mime_info_cache_dir_init(dir);
      mime_info_cache_dir_init_defaults_list(dir);
      mime_info_cache_dir_init_mimeapps_list(dir);
      tmp = tmp->next;
  }
}
static void mime_info_cache_init(void) {
  G_LOCK(mime_info_cache);
  if (mime_info_cache == NULL) mime_info_cache_init_dir_lists();
  else {
      time_t now;
      time(&now);
      if (now >= mime_info_cache->last_stat_time + 10) {
          mime_info_cache_update_dir_lists();
          mime_info_cache->last_stat_time = now;
	  }
  }
  if (mime_info_cache->should_ping_mime_monitor) mime_info_cache->should_ping_mime_monitor = FALSE;
  G_UNLOCK(mime_info_cache);
}
static MimeInfoCache *mime_info_cache_new(void) {
  MimeInfoCache *cache;
  cache = g_new0(MimeInfoCache, 1);
  cache->global_defaults_cache = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)g_free, (GDestroyNotify)g_free);
  return cache;
}
static void mime_info_cache_free(MimeInfoCache *cache) {
  if (cache == NULL) return;
  g_list_foreach(cache->dirs, (GFunc)mime_info_cache_dir_free,NULL);
  g_list_free(cache->dirs);
  g_hash_table_destroy(cache->global_defaults_cache);
  g_free(cache);
}
static void mime_info_cache_reload(const char *dir) {
  if (mime_info_cache != NULL) {
      G_LOCK (mime_info_cache);
      mime_info_cache_free(mime_info_cache);
      mime_info_cache = NULL;
      G_UNLOCK(mime_info_cache);
  }
}
static GList *append_desktop_entry(GList *list, const char *desktop_entry, GList *removed_entries) {
  if (!g_list_find_custom(list, desktop_entry, (GCompareFunc)strcmp) && !g_list_find_custom(removed_entries, desktop_entry, (GCompareFunc)strcmp))
      list = g_list_prepend(list, g_strdup(desktop_entry));
  return list;
}
static GList *get_all_desktop_entries_for_mime_type(const char  *base_mime_type, const char **except, gboolean include_fallback, char **explicit_default) {
  GList *desktop_entries, *removed_entries, *list, *dir_list, *tmp;
  MimeInfoCacheDir *dir;
  char *mime_type, *default_entry = NULL;
  const char *entry;
  char **mime_types;
  char **default_entries;
  char **removed_associations;
  int i, j, k;
  GPtrArray *array;
  char **anc;
  mime_info_cache_init();
  if (include_fallback) {
      mime_types = _g_unix_content_type_get_parents(base_mime_type);
      array = g_ptr_array_new();
      for (i = 0; mime_types[i]; i++) g_ptr_array_add(array, mime_types[i]);
      g_free(mime_types);
      for (i = 0; i < array->len; i++) {
          anc = _g_unix_content_type_get_parents(g_ptr_array_index(array, i));
          for (j = 0; anc[j]; j++) {
              for (k = 0; k < array->len; k++) {
                  if (strcmp(anc[j], g_ptr_array_index(array, k)) == 0) break;
              }
              if (k == array->len) g_ptr_array_add(array, g_strdup(anc[j]));
          }
          g_strfreev(anc);
	  }
      g_ptr_array_add(array, NULL);
      mime_types = (char**)g_ptr_array_free(array,FALSE);
  } else {
      mime_types = g_malloc0(2 * sizeof(gchar*));
      mime_types[0] = g_strdup(base_mime_type);
      mime_types[1] = NULL;
  }
  G_LOCK(mime_info_cache);
  removed_entries = NULL;
  desktop_entries = NULL;
  for (i = 0; except != NULL && except[i] != NULL; i++) removed_entries = g_list_prepend(removed_entries, g_strdup(except[i]));
  for (i = 0; mime_types[i] != NULL; i++) {
      mime_type = mime_types[i];
      for (dir_list = mime_info_cache->dirs; dir_list != NULL; dir_list = dir_list->next) {
          dir = dir_list->data;
          if (desktop_entries == NULL) {
              entry = g_hash_table_lookup(dir->mimeapps_list_defaults_map, mime_type);
              if (entry != NULL) {
                  if (default_entry == NULL) default_entry = g_strdup(entry);
              }
          }
          default_entries = g_hash_table_lookup(dir->mimeapps_list_added_map, mime_type);
          for (j = 0; default_entries != NULL && default_entries[j] != NULL; j++)
              desktop_entries = append_desktop_entry(desktop_entries, default_entries[j], removed_entries);
          removed_associations = g_hash_table_lookup(dir->mimeapps_list_removed_map, mime_type);
          for (j = 0; removed_associations != NULL && removed_associations[j] != NULL; j++)
              removed_entries = append_desktop_entry(removed_entries, removed_associations[j], NULL);
          default_entries = g_hash_table_lookup(dir->defaults_list_map, mime_type);
          for (j = 0; default_entries != NULL && default_entries[j] != NULL; j++)
              desktop_entries = append_desktop_entry(desktop_entries, default_entries[j], removed_entries);
	  }
      for (dir_list = mime_info_cache->dirs; dir_list != NULL; dir_list = dir_list->next) {
          dir = dir_list->data;
          list = g_hash_table_lookup(dir->mime_info_cache_map, mime_type);
          for (tmp = list; tmp != NULL; tmp = tmp->next) desktop_entries = append_desktop_entry(desktop_entries, tmp->data, removed_entries);
      }
  }
  G_UNLOCK(mime_info_cache);
  g_strfreev(mime_types);
  if (explicit_default != NULL) *explicit_default = default_entry;
  else g_free(default_entry);
  g_list_foreach(removed_entries, (GFunc)g_free, NULL);
  g_list_free(removed_entries);
  desktop_entries = g_list_reverse(desktop_entries);
  return desktop_entries;
}
typedef GDesktopAppInfoLookupIface GDesktopAppInfoLookupInterface;
G_DEFINE_INTERFACE(GDesktopAppInfoLookup, g_desktop_app_info_lookup, G_TYPE_OBJECT);
static void g_desktop_app_info_lookup_default_init(GDesktopAppInfoLookupInterface *iface) {}
GAppInfo *g_desktop_app_info_lookup_get_default_for_uri_scheme(GDesktopAppInfoLookup *lookup, const char *uri_scheme) {
  GDesktopAppInfoLookupIface *iface;
  g_return_val_if_fail(G_IS_DESKTOP_APP_INFO_LOOKUP(lookup), NULL);
  iface = G_DESKTOP_APP_INFO_LOOKUP_GET_IFACE(lookup);
  return (*iface->get_default_for_uri_scheme)(lookup, uri_scheme);
}