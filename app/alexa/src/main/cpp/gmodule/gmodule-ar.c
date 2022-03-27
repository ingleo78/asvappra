#include <stdlib.h>
#include <dlfcn.h>
#include <asm/fcntl.h>
#include <fcntl.h>
#include <zconf.h>
#include <string.h>
#include "../gio/config.h"
#include "../glib/glib.h"
#include "gmodule.h"

#define AIAMAGBIG "<bigaf>\n"
#define AIAMAG "<aiaff>\n"
#define SAIAMAG 8
#define AIAFMAG "`\n"
#define FL_HDR struct fl_hdr
#define FL_HSZ sizeof(FL_HDR)
#define AR_HDR struct ar_hdr
#define AR_HSZ sizeof(AR_HDR)
#define ar_name _ar_name.an_name
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
struct fl_hdr {
    char fl_magic[SAIAMAG];
    char fl_memoff[12];
    char fl_gstoff[12];
    char fl_fstmoff[12];
    char fl_lstmoff[12];
    char fl_freeoff[12];
};
struct ar_hdr {
    char ar_size[12];
    char ar_nxtmem[12];
    char ar_prvmem[12];
    char ar_date[12];
    char ar_uid[12];
    char ar_gid[12];
    char ar_mode[12];
    char ar_namlen[4];
    union {
        char an_name[2];
        char an_fmag[2];
    } _ar_name;
};
static gpointer	_g_module_open(const gchar *file_name, gboolean	bind_lazy, gboolean	bind_local);
static void	 _g_module_close(gpointer handle, gboolean is_unref);
static gpointer _g_module_self(void);
static gpointer	_g_module_symbol(gpointer handle, const gchar *symbol_name);
static gchar* _g_module_build_path(const gchar *directory, const gchar *module_name);
static inline void g_module_set_error(const gchar *error);
static inline GModule* g_module_find_by_handle(gpointer	 handle);
static inline GModule* g_module_find_by_name(const gchar	*name);
static gchar* fetch_dlerror (gboolean replace_null) {
    gchar *msg = dlerror ();
    if (!msg && replace_null) return "unknown dl-error";
    return msg;
}
static gchar* _g_module_get_member(const gchar* file_name) {
    gchar* member = NULL;
    struct fl_hdr file_header;
    struct ar_hdr ar_header;
    long first_member;
    long name_len;
    int fd;
    fd = open(file_name, O_RDONLY);
    if (fd == -1) return NULL;
    if (read(fd, (void*)&file_header, FL_HSZ) != FL_HSZ) goto exit;
    if (strncmp(file_header.fl_magic, AIAMAGBIG, SAIAMAG) != 0) goto exit;
    first_member = atol(file_header.fl_fstmoff);
    if (lseek(fd, first_member, SEEK_SET) != first_member) goto exit;
    if (read(fd, (void*)&ar_header, AR_HSZ - 2) != AR_HSZ - 2) goto exit;
    name_len = atol(ar_header.ar_namlen);
    member = g_malloc(name_len+1);
    if (!member) goto exit;
    if (read(fd, (void*)member, name_len) != name_len) {
        g_free(member);
        member = NULL;
        goto exit;
    }
    member[name_len] = 0;
exit:
    close(fd);
    return member;
}
static gpointer _g_module_open(const gchar *file_name, gboolean     bind_lazy, gboolean     bind_local) {
    gpointer handle;
    gchar* member;
    gchar* full_name;
    member = _g_module_get_member(file_name);
    if (member != NULL) {
        full_name = g_strconcat(file_name, "(", member, ")", NULL);
        g_free (member);
    } else full_name = g_strdup(file_name);
    handle = dlopen(full_name,(bind_local ? RTLD_LOCAL : RTLD_GLOBAL) | (bind_lazy ? RTLD_LAZY : RTLD_NOW));
    g_free (full_name);
    if (!handle) g_module_set_error(fetch_dlerror (TRUE));
    return handle;
}
static gpointer _g_module_self(void) {
    gpointer handle;
    handle = dlopen(NULL, RTLD_GLOBAL | RTLD_LAZY);
    if (!handle) g_module_set_error(fetch_dlerror (TRUE));
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
    p = dlsym(handle, symbol_name);
    if (!p) g_module_set_error(fetch_dlerror(FALSE));
    return p;
}
static gchar* _g_module_build_path(const gchar *directory, const gchar *module_name) {
    if (directory && *directory) {
        if (strncmp(module_name, "lib", 3) == 0) return g_strconcat(directory, "/", module_name, NULL);
        else return g_strconcat(directory, "/lib", module_name, "." G_MODULE_SUFFIX, NULL);
    } else if (strncmp(module_name, "lib", 3) == 0) return g_strdup(module_name);
    else return g_strconcat("lib", module_name, "." G_MODULE_SUFFIX, NULL);
}