#include <string.h>
#include "../gio/config.h"
#include "../glib/glib.h"

#ifndef	DYNAMIC_PATH
#define	DYNAMIC_PATH	0
#endif
#ifndef	BIND_RESTRICTED
#define	BIND_RESTRICTED	0
#endif
#define	OPT_BIND_FLAGS	(BIND_NONFATAL | BIND_VERBOSE)
static gpointer _g_module_open(const gchar *file_name, gboolean bind_lazy, gboolean bind_local) {
    /*shl_t shl_handle;
    shl_handle = shl_load (file_name, (bind_lazy ? BIND_DEFERRED : BIND_IMMEDIATE) | OPT_BIND_FLAGS, 0);
    if (!shl_handle) g_module_set_error(g_strerror(errno));
    return (gpointer) shl_handle;*/
    return NULL;
}
static gpointer _g_module_self(void) {
    /*shl_t shl_handle;
    shl_handle = PROG_HANDLE;
    if (!shl_handle) g_module_set_error(g_strerror (errno));
    return shl_handle;*/
    return NULL;
}
static void _g_module_close(gpointer handle, gboolean is_unref) {
    /*if (!is_unref) {
        if (shl_unload((shl_t) handle) != 0) g_module_set_error(g_strerror(errno));
    }*/
}
static gpointer _g_module_symbol(gpointer handle, const gchar *symbol_name) {
  gpointer p = NULL;
  //if (handle == PROG_HANDLE) handle = NULL;
  //if (shl_findsym ((shl_t*) &handle, symbol_name, TYPE_UNDEFINED, &p) != 0 || handle == NULL || p == NULL)  g_module_set_error (g_strerror (errno));
  return p;
}
static gchar* _g_module_build_path(const gchar *directory, const gchar *module_name) {
    if (directory && *directory) {
        if (strncmp(module_name, "lib", 3) == 0) return g_strconcat(directory, "/", module_name, NULL);
        else return g_strconcat(directory, "/lib", module_name, ".sl", NULL);
    } else if (strncmp(module_name, "lib", 3) == 0) return g_strdup(module_name);
    else return g_strconcat("lib", module_name, ".sl", NULL);
}