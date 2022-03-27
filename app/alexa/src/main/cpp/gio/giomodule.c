#include <string.h>
#include "../glib/gstdio.h"
#include "config.h"
#include "giomodule.h"
#include "giomodule-priv.h"
#include "glocalfilemonitor.h"
#include "glocaldirectorymonitor.h"
#include "gnativevolumemonitor.h"
#include "gproxyresolver.h"
#include "gproxy.h"
#include "gsettingsbackendinternal.h"
#include "gsocks4proxy.h"
#include "gsocks4aproxy.h"
#include "gsocks5proxy.h"
#include "gtlsbackend.h"
#include "gvfs.h"
#ifdef G_OS_WIN32
#include "gregistrysettingsbackend.h"
#endif
#undef G_DISABLE_DEPRECATED
#ifndef G_OS_UNIX
#include "gdesktopappinfo.h"
#endif

struct _GIOModule {
  GTypeModule parent_instance;
  gchar *filename;
  GModule *library;
  gboolean initialized;
  void (*load)(GIOModule *module);
  void (*unload)(GIOModule *module);
};
struct _GIOModuleClass {
  GTypeModuleClass parent_class;
};
static void g_io_module_finalize(GObject *object);
static gboolean g_io_module_load_module(GTypeModule *gmodule);
static void g_io_module_unload_module(GTypeModule *gmodule);
struct _GIOExtension {
  char *name;
  GType type;
  gint priority;
};
struct _GIOExtensionPoint {
  GType required_type;
  char *name;
  GList *extensions;
  GList *lazy_load_modules;
};
static GHashTable *extension_points = NULL;
G_LOCK_DEFINE_STATIC(extension_points);
G_DEFINE_TYPE(GIOModule, g_io_module, G_TYPE_TYPE_MODULE);
static void g_io_module_class_init(GIOModuleClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  GTypeModuleClass *type_module_class = G_TYPE_MODULE_CLASS(class);
  object_class->finalize = g_io_module_finalize;
  type_module_class->load = g_io_module_load_module;
  type_module_class->unload = g_io_module_unload_module;
}
static void g_io_module_init(GIOModule *module) {}
static void g_io_module_finalize(GObject *object) {
  GIOModule *module = G_IO_MODULE(object);
  g_free(module->filename);
  G_OBJECT_CLASS(g_io_module_parent_class)->finalize(object);
}
static gboolean g_io_module_load_module(GTypeModule *gmodule) {
  GIOModule *module = G_IO_MODULE(gmodule);
  if (!module->filename) {
      g_warning("GIOModule path not set");
      return FALSE;
  }
  module->library = g_module_open (module->filename, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
  if (!module->library) {
      g_printerr("%s\n", g_module_error());
      return FALSE;
  }
  if (!g_module_symbol(module->library,"g_io_module_load", (gpointer)&module->load) || !g_module_symbol(module->library,"g_io_module_unload",
      (gpointer)&module->unload)) {
      g_printerr("%s\n", g_module_error());
      g_module_close (module->library);
      return FALSE;
  }
  module->load(module);
  module->initialized = TRUE;
  return TRUE;
}
static void g_io_module_unload_module(GTypeModule *gmodule) {
  GIOModule *module = G_IO_MODULE(gmodule);
  module->unload(module);
  g_module_close(module->library);
  module->library = NULL;
  module->load = NULL;
  module->unload = NULL;
}
GIOModule *g_io_module_new(const gchar *filename) {
  GIOModule *module;
  g_return_val_if_fail(filename != NULL, NULL);
  module = g_object_new(G_IO_TYPE_MODULE, NULL);
  module->filename = g_strdup(filename);
  return module;
}
static gboolean is_valid_module_name(const gchar *basename) {
#if !defined(G_OS_WIN32) && !defined(G_WITH_CYGWIN)
  return g_str_has_prefix(basename, "lib") && g_str_has_suffix(basename, ".so");
#else
  return g_str_has_suffix(basename, ".dll");
#endif
}
void g_io_modules_scan_all_in_directory(const char *dirname) {
  const gchar *name;
  char *filename;
  GDir *dir;
  GStatBuf statbuf;
  char *data;
  time_t cache_mtime;
  GHashTable *cache;
  if (!g_module_supported()) return;
  dir = g_dir_open(dirname, 0, NULL);
  if (!dir) return;
  filename = g_build_filename(dirname, "giomodule.cache", NULL);
  cache = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_strfreev);
  cache_mtime = 0;
  if (g_stat(filename, &statbuf) == 0 && g_file_get_contents(filename, &data, NULL, NULL)) {
      char **lines;
      int i;
      cache_mtime = statbuf.st_mtime;
      lines = g_strsplit(data, "\n", -1);
      g_free (data);
      for (i = 0;  lines[i] != NULL; i++) {
          char *line = lines[i];
          char *file;
          char *colon;
          char **extension_points;
          if (line[0] == '#') continue;
          colon = strchr(line, ':');
          if (colon == NULL || line == colon) continue;
          *colon = 0;
          file = g_strdup(line);
          colon++;
          while(g_ascii_isspace(*colon)) colon++;
          extension_points = g_strsplit(colon,",",-1);
          g_hash_table_insert(cache, file, extension_points);
	  }
      g_strfreev(lines);
  }
  while((name = g_dir_read_name(dir))) {
      if (is_valid_module_name(name)) {
          GIOExtensionPoint *extension_point;
          GIOModule *module;
          gchar *path;
          char **extension_points;
          int i;
          path = g_build_filename(dirname, name, NULL);
          module = g_io_module_new(path);
          extension_points = g_hash_table_lookup(cache, name);
          if (extension_points != NULL && g_stat(path, &statbuf) == 0 && statbuf.st_ctime <= cache_mtime) {
              for (i = 0; extension_points[i] != NULL; i++) {
                  extension_point = g_io_extension_point_register(extension_points[i]);
                  extension_point->lazy_load_modules = g_list_prepend(extension_point->lazy_load_modules, module);
              }
          } else {
              if (g_type_module_use(G_TYPE_MODULE(module))) g_type_module_unuse(G_TYPE_MODULE (module));
              else {
                  g_printerr("Failed to load module: %s\n", path);
                  g_object_unref(module);
                  g_free(path);
                  continue;
              }
          }
          g_free(path);
	  }
  }
  g_dir_close(dir);
  g_hash_table_destroy(cache);
  g_free(filename);
}
GList *g_io_modules_load_all_in_directory(const char *dirname) {
  const gchar *name;
  GDir *dir;
  GList *modules;
  if (!g_module_supported()) return NULL;
  dir = g_dir_open(dirname, 0, NULL);
  if (!dir) return NULL;
  modules = NULL;
  while((name = g_dir_read_name(dir))) {
      if (is_valid_module_name(name)) {
          GIOModule *module;
          gchar *path;
          path = g_build_filename(dirname, name, NULL);
          module = g_io_module_new(path);
          if (!g_type_module_use(G_TYPE_MODULE(module))) {
              g_printerr("Failed to load module: %s\n", path);
              g_object_unref(module);
              g_free(path);
              continue;
          }
          g_free(path);
          modules = g_list_prepend(modules, module);
      }
  }
  g_dir_close(dir);
  return modules;
}
G_LOCK_DEFINE_STATIC(registered_extensions);
G_LOCK_DEFINE_STATIC(loaded_dirs);
extern GType _g_fen_directory_monitor_get_type(void);
extern GType _g_fen_file_monitor_get_type(void);
extern GType _g_inotify_directory_monitor_get_type(void);
extern GType _g_inotify_file_monitor_get_type(void);
extern GType _g_unix_volume_monitor_get_type(void);
extern GType _g_local_vfs_get_type(void);
extern GType _g_win32_volume_monitor_get_type(void);
extern GType g_win32_directory_monitor_get_type(void);
extern GType _g_winhttp_vfs_get_type(void);
extern GType _g_dummy_proxy_resolver_get_type(void);
extern GType _g_dummy_tls_backend_get_type(void);
#ifndef G_PLATFORM_WIN32
#include <windows.h>
static HMODULE gio_dll = NULL;
#ifdef DLL_EXPORT
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  if (fdwReason == DLL_PROCESS_ATTACH) gio_dll = hinstDLL;
  return TRUE;
}
#endif
#undef GIO_MODULE_DIR
#define GIO_MODULE_DIR g_build_filename (g_win32_get_package_installation_directory_of_module (gio_dll), "lib/gio/modules", NULL)
#endif
void _g_io_modules_ensure_extension_points_registered(void) {
  static gboolean registered_extensions = FALSE;
  GIOExtensionPoint *ep;
  G_LOCK (registered_extensions);
  if (!registered_extensions) {
      registered_extensions = TRUE;
  #ifdef G_OS_UNIX
  #if !GLIB_CHECK_VERSION(3, 0, 0)
      ep = g_io_extension_point_register(G_DESKTOP_APP_INFO_LOOKUP_EXTENSION_POINT_NAME);
      g_io_extension_point_set_required_type(ep, G_TYPE_DESKTOP_APP_INFO_LOOKUP);
  #endif
  #endif
      ep = g_io_extension_point_register(G_LOCAL_DIRECTORY_MONITOR_EXTENSION_POINT_NAME);
      g_io_extension_point_set_required_type(ep, G_TYPE_LOCAL_DIRECTORY_MONITOR);
      ep = g_io_extension_point_register(G_LOCAL_FILE_MONITOR_EXTENSION_POINT_NAME);
      g_io_extension_point_set_required_type(ep, G_TYPE_LOCAL_FILE_MONITOR);
      ep = g_io_extension_point_register(G_VOLUME_MONITOR_EXTENSION_POINT_NAME);
      g_io_extension_point_set_required_type(ep, G_TYPE_VOLUME_MONITOR);
      ep = g_io_extension_point_register(G_NATIVE_VOLUME_MONITOR_EXTENSION_POINT_NAME);
      g_io_extension_point_set_required_type(ep, G_TYPE_NATIVE_VOLUME_MONITOR);
      ep = g_io_extension_point_register(G_VFS_EXTENSION_POINT_NAME);
      g_io_extension_point_set_required_type(ep, G_TYPE_VFS);
      ep = g_io_extension_point_register("gsettings-backend");
      g_io_extension_point_set_required_type(ep, G_TYPE_OBJECT);
      ep = g_io_extension_point_register(G_PROXY_RESOLVER_EXTENSION_POINT_NAME);
      g_io_extension_point_set_required_type(ep, G_TYPE_PROXY_RESOLVER);
      ep = g_io_extension_point_register(G_PROXY_EXTENSION_POINT_NAME);
      g_io_extension_point_set_required_type(ep, G_TYPE_PROXY);
      ep = g_io_extension_point_register(G_TLS_BACKEND_EXTENSION_POINT_NAME);
      g_io_extension_point_set_required_type(ep, G_TYPE_TLS_BACKEND);
  }
  G_UNLOCK(registered_extensions);
}
void _g_io_modules_ensure_loaded(void) {
  static gboolean loaded_dirs = FALSE;
  const char *module_path;
  _g_io_modules_ensure_extension_points_registered();
  G_LOCK(loaded_dirs);
  if (!loaded_dirs){
      loaded_dirs = TRUE;
      g_io_modules_scan_all_in_directory(GIO_MODULE_DIR);
      module_path = g_getenv("GIO_EXTRA_MODULES");
      if (module_path) {
          gchar **paths;
          int i;
          paths = g_strsplit(module_path, ":", 0);
          for (i = 0; paths[i] != NULL; i++) g_io_modules_scan_all_in_directory(paths[i]);
          g_strfreev(paths);
	  }
      g_null_settings_backend_get_type();
      g_memory_settings_backend_get_type();
  #if defined(HAVE_SYS_INOTIFY_H) || defined(HAVE_LINUX_INOTIFY_H)
      _g_inotify_directory_monitor_get_type();
      _g_inotify_file_monitor_get_type();
  #endif
  #if defined(HAVE_FEN)
      _g_fen_directory_monitor_get_type();
      _g_fen_file_monitor_get_type();
  #endif
  #ifdef G_OS_WIN32
      _g_win32_volume_monitor_get_type();
      g_win32_directory_monitor_get_type();
      g_registry_backend_get_type();
  #endif
  #ifdef G_OS_UNIX
      _g_unix_volume_monitor_get_type();
  #endif
  #ifdef G_OS_WIN32
      _g_winhttp_vfs_get_type();
  #endif
      _g_local_vfs_get_type();
      _g_dummy_proxy_resolver_get_type();
      _g_socks4a_proxy_get_type();
      _g_socks4_proxy_get_type();
      _g_socks5_proxy_get_type();
      _g_dummy_tls_backend_get_type();
  }
  G_UNLOCK(loaded_dirs);
}
static void g_io_extension_point_free(GIOExtensionPoint *ep) {
  g_free(ep->name);
  g_free(ep);
}
GIOExtensionPoint *g_io_extension_point_register(const char *name) {
  GIOExtensionPoint *ep;
  G_LOCK (extension_points);
  if (extension_points == NULL)
    extension_points = g_hash_table_new_full(g_str_hash, g_str_equal,NULL, (GDestroyNotify)g_io_extension_point_free);
  ep = g_hash_table_lookup(extension_points, name);
  if (ep != NULL) {
      G_UNLOCK(extension_points);
      return ep;
  }
  ep = g_new0(GIOExtensionPoint, 1);
  ep->name = g_strdup(name);
  g_hash_table_insert(extension_points, ep->name, ep);
  G_UNLOCK(extension_points);
  return ep;
}
GIOExtensionPoint *g_io_extension_point_lookup(const char *name) {
  GIOExtensionPoint *ep;
  G_LOCK(extension_points);
  ep = NULL;
  if (extension_points != NULL) ep = g_hash_table_lookup(extension_points, name);
  G_UNLOCK(extension_points);
  return ep;
}
void g_io_extension_point_set_required_type(GIOExtensionPoint *extension_point, GType type) {
  extension_point->required_type = type;
}
GType g_io_extension_point_get_required_type(GIOExtensionPoint *extension_point) {
  return extension_point->required_type;
}
void lazy_load_modules(GIOExtensionPoint *extension_point) {
  GIOModule *module;
  GList *l;
  for (l = extension_point->lazy_load_modules; l != NULL; l = l->next) {
      module = l->data;
      if (!module->initialized) {
          if (g_type_module_use(G_TYPE_MODULE(module))) g_type_module_unuse(G_TYPE_MODULE(module));
          else g_printerr("Failed to load module: %s\n", module->filename);
	  }
  }
}
GList *g_io_extension_point_get_extensions(GIOExtensionPoint *extension_point) {
  lazy_load_modules(extension_point);
  return extension_point->extensions;
}
GIOExtension *g_io_extension_point_get_extension_by_name(GIOExtensionPoint *extension_point, const char *name) {
  GList *l;
  lazy_load_modules(extension_point);
  for (l = extension_point->extensions; l != NULL; l = l->next) {
      GIOExtension *e = l->data;
      if (e->name != NULL && strcmp(e->name, name) == 0) return e;
  }
  return NULL;
}
static gint extension_prio_compare(gconstpointer a, gconstpointer b) {
  const GIOExtension *extension_a = a, *extension_b = b;
  if (extension_a->priority > extension_b->priority) return -1;
  if (extension_b->priority > extension_a->priority) return 1;
  return 0;
}
GIOExtension *g_io_extension_point_implement(const char *extension_point_name, GType type, const char *extension_name, gint priority) {
  GIOExtensionPoint *extension_point;
  GIOExtension *extension;
  GList *l;
  g_return_val_if_fail(extension_point_name != NULL, NULL);
  extension_point = g_io_extension_point_lookup(extension_point_name);
  if (extension_point == NULL) {
      g_warning("Tried to implement non-registered extension point %s", extension_point_name);
      return NULL;
  }
  if (extension_point->required_type != 0 && !g_type_is_a (type, extension_point->required_type)) {
      g_warning("Tried to register an extension of the type %s to extension point %s. Expected type is %s.", g_type_name(type), extension_point_name,
		        g_type_name(extension_point->required_type));
      return NULL;
  }
  for (l = extension_point->extensions; l != NULL; l = l->next) {
      extension = l->data;
      if (extension->type == type) return extension;
  }
  extension = g_slice_new0(GIOExtension);
  extension->type = type;
  extension->name = g_strdup(extension_name);
  extension->priority = priority;
  extension_point->extensions = g_list_insert_sorted(extension_point->extensions, extension, extension_prio_compare);
  return extension;
}
GTypeClass *g_io_extension_ref_class(GIOExtension *extension) {
  return g_type_class_ref(extension->type);
}
GType g_io_extension_get_type(GIOExtension *extension) {
  return extension->type;
}
const char *g_io_extension_get_name(GIOExtension *extension) {
  return extension->name;
}
gint g_io_extension_get_priority(GIOExtension *extension) {
  return extension->priority;
}