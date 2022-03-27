#include <dlfcn.h>
#include <string.h>
#include "../gio/config.h"
#include "../glib/glib.h"

#ifndef	G_MODULE_HAVE_DLERROR
#define dlerror()	"unknown dl-error"
#endif
#ifndef	RTLD_GLOBAL
#define	RTLD_GLOBAL	0
#endif
#ifndef	RTLD_LAZY
#define	RTLD_LAZY	1
#endif
#ifndef	RTLD_NOW
#define	RTLD_NOW	0
#endif
static gpointer _g_module_open(const gchar *file_name, gboolean bind_lazy, gboolean bind_local) {
    gpointer handle;
    handle = dlopen(file_name,(bind_local ? 0 : RTLD_GLOBAL) | (bind_lazy ? RTLD_LAZY : RTLD_NOW));
    //if (!handle) g_module_set_error(dlerror ());
    return handle;
}
static gpointer _g_module_self(void) {
    gpointer handle;
    handle = NULL;
    //g_module_set_error("module handle for self not supported");
    return handle;
}
static void _g_module_close(gpointer handle, gboolean is_unref) {
    is_unref |= 1;
    if (is_unref) dlclose(handle);
}
static gpointer _g_module_symbol(gpointer handle, const gchar *symbol_name) {
    gpointer p;
    p = dlsym(handle, symbol_name);
    //if (!p) g_module_set_error(dlerror ());
    return p;
}
static gchar* _g_module_build_path(const gchar *directory, const gchar *module_name) {
    gchar *suffix = strrchr(module_name, '.');
    if (directory && *directory) {
        if (suffix && (strcmp(suffix, ".dll") == 0)) return g_strconcat(directory, "/", module_name, NULL);
        else return g_strconcat(directory, "/", module_name, ".dll", NULL);
    } else if (suffix && (strcmp (suffix, ".dll") == 0)) return g_strdup (module_name);
    else return g_strconcat (module_name, ".dll", NULL);
}