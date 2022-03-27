#include <string.h>
#include <stdio.h>
#include "../gio/config.h"
#include "../glib/glib.h"
#include "gmodule.h"

#ifndef	RTLD_GLOBAL
#define	RTLD_GLOBAL	0
#endif
#ifndef	RTLD_LAZY
#define	RTLD_LAZY	1
#endif
#ifndef	RTLD_NOW
#define	RTLD_NOW	0
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
static gpointer _g_module_open (const gchar *file_name, gboolean bind_lazy, gboolean bind_local) {
    /*image_id handle;
    handle = load_add_on (file_name);
    if (handle < B_OK) {
        gchar *msg = g_strdup_printf("failed to load_add_on(%s): %s", file_name, strerror(handle));
        g_module_set_error(msg);
        g_free(msg);
        return NULL;
    }
    return (gpointer)handle;*/
    return NULL;
}
static gpointer _g_module_self (void) {
    /*image_info info;
    int32 cookie = 0;
    status_t status;
    status = get_next_image_info(0, &cookie, &info);
    if (status == B_OK) return(gpointer) info.id;
    else {
        gchar *msg = g_strdup_printf("failed to get_next_image_info(self): %s", strerror(status));
        g_module_set_error(msg);
        g_free(msg);
        return NULL;
    }*/
    return NULL;
}
static void _g_module_close(gpointer handle, gboolean is_unref) {
    /*image_info info;
    gchar *name;
    if (unload_add_on((image_id)handle) != B_OK) {
        gchar *msg;
        if (get_image_info((image_id)handle, &info) != B_OK) name = g_strdup("unknown");
        else name = g_strdup(info.name);
        msg = g_strdup_printf("failed to unload_add_on(%s): %s", name, strerror(status));
        g_module_set_error(msg);
        g_free(msg);
        g_free(name);
    }*/
}
static gpointer _g_module_symbol(gpointer handle, const gchar *symbol_name) {
    /*image_id id;
    status_t status;
    image_info info;
    int32 type, name_len;
    void *p;
    gchar *msg, name[256];
    gint n, l;
    id = (image_id)handle;
    status = get_image_info(id, &info);
    if (status != B_OK) {
        msg = g_strdup_printf("failed to get_image_info(): %s", strerror(status));
        g_module_set_error(msg);
        g_free(msg);
        return NULL;
    }
    l = strlen(symbol_name);
    name_len = 256;
    type = B_SYMBOL_TYPE_ANY;
    n = 0;
    status = get_nth_image_symbol(id, n, name, &name_len, &type, &p);
    while(status == B_OK) {
        if (p && strncmp(name, symbol_name, l) == 0) return p;
        if (strcmp(name, "_end") == 0) {
            msg = g_strdup_printf("unmatched symbol name `%s'", symbol_name);
            g_module_set_error(msg);
            g_free(msg);
            return NULL;
        }
        name_len = 256;
        type = B_SYMBOL_TYPE_ANY;
        n++;
        status = get_nth_image_symbol(id, n, name, &name_len, &type, &p);
    }
    msg = g_strdup_printf("failed to get_image_symbol(%s): %s", symbol_name, strerror (status));
    g_module_set_error (msg);
    g_free(msg);*/
    return NULL;
}
static gchar* _g_module_build_path (const gchar *directory, const gchar *module_name) {
    g_warning ("_g_module_build_path() untested for BeOS!");
    if (directory && *directory) {
        if (strncmp (module_name, "lib", 3) == 0) return g_strconcat (directory, "/", module_name, NULL);
        else return g_strconcat (directory, "/lib", module_name, "." G_MODULE_SUFFIX, NULL);
    } else if (strncmp (module_name, "lib", 3) == 0) return g_strdup (module_name);
    else return g_strconcat ("lib", module_name, "." G_MODULE_SUFFIX, NULL);
}