#include <dlfcn.h>
#include <string.h>
#include "../gio/config.h"
#include "../glib/glib.h"
#include "gmodule.h"

#ifndef	G_MODULE_HAVE_DLERROR
#  ifdef __NetBSD__
#    define dlerror()	g_strerror (errno)
#  else
#    define dlerror()	"unknown dl-error"
#  endif
#endif
#ifndef	RTLD_LAZY
#define	RTLD_LAZY	1
#endif
#ifndef	RTLD_NOW
#define	RTLD_NOW	0
#endif
#ifdef G_MODULE_BROKEN_RTLD_GLOBAL
#undef	RTLD_GLOBAL
#endif
#ifndef	RTLD_GLOBAL
#define	RTLD_GLOBAL	0
#endif
struct _GModule {
    gchar	*file_name;
#if defined (G_OS_WIN32) && !defined(_WIN64)
    gchar *cp_file_name;
#endif
    gpointer handle;
    guint ref_count : 31;
    guint is_resident : 1;
    GModuleUnload unload;
    GModule *next;
};
static gpointer	_g_module_open(const gchar *file_name, gboolean	bind_lazy, gboolean	bind_local);
static void	 _g_module_close(gpointer handle, gboolean is_unref);
static gpointer _g_module_self(void);
static gpointer	_g_module_symbol(gpointer handle, const gchar *symbol_name);
static gchar* _g_module_build_path(const gchar *directory, const gchar *module_name);
static inline void g_module_set_error(const gchar *error);
static inline GModule* g_module_find_by_handle(gpointer	 handle);
static inline GModule* g_module_find_by_name(const gchar	*name);
static gchar* fetch_dlerror(gboolean replace_null) {
  gchar *msg = dlerror ();
  if (!msg && replace_null) return "unknown dl-error";
  return msg;
}
static gpointer _g_module_open(const gchar *file_name, gboolean bind_lazy, gboolean bind_local) {
  gpointer handle;
  handle = dlopen(file_name,(bind_local ? 0 : RTLD_GLOBAL) | (bind_lazy ? RTLD_LAZY : RTLD_NOW));
  if (!handle) g_module_set_error(fetch_dlerror(TRUE));
  return handle;
}
static gpointer _g_module_self(void) {
  gpointer handle;
  handle = dlopen(NULL, RTLD_GLOBAL | RTLD_LAZY);
  if (!handle) g_module_set_error(fetch_dlerror(TRUE));
  return handle;
}
static void _g_module_close(gpointer handle, gboolean is_unref) {
    is_unref |= 1;
    if (is_unref) {
        if (dlclose(handle) != 0) g_module_set_error(fetch_dlerror (TRUE));
    }
}
static gpointer _g_module_symbol(gpointer handle, const gchar *symbol_name) {
    gpointer p;
    gchar *msg;
    fetch_dlerror(FALSE);
    p = dlsym(handle, symbol_name);
    msg = fetch_dlerror(FALSE);
    if (msg) g_module_set_error(msg);
    return p;
}
static gchar* _g_module_build_path (const gchar *directory, const gchar *module_name) {
    if (directory && *directory) {
    if (strncmp(module_name, "lib", 3) == 0) return g_strconcat(directory, "/", module_name, NULL);
    else return g_strconcat(directory, "/lib", module_name, "." G_MODULE_SUFFIX, NULL);
    } else if (strncmp(module_name, "lib", 3) == 0) return g_strdup(module_name);
    else return g_strconcat("lib", module_name, "." G_MODULE_SUFFIX, NULL);
}