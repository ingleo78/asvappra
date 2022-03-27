#ifndef __GMODULE_H__
#define __GMODULE_H__

#include "../glib/gtypes.h"

G_BEGIN_DECLS
#define	G_MODULE_IMPORT		extern
#ifdef G_PLATFORM_WIN32
#define	G_MODULE_EXPORT		__declspec(dllexport)
#else
#define	G_MODULE_EXPORT
#endif
typedef enum {
    G_MODULE_BIND_LAZY	= 1 << 0,
    G_MODULE_BIND_LOCAL	= 1 << 1,
    G_MODULE_BIND_MASK	= 0x03
} GModuleFlags;
typedef	struct _GModule GModule;
typedef const gchar* (*GModuleCheckInit)(GModule *module);
typedef void (*GModuleUnload)(GModule *module);
#ifdef G_OS_WIN32
#define g_module_open g_module_open_utf8
#define g_module_name g_module_name_utf8
#endif
gboolean g_module_supported(void) G_GNUC_CONST;
GModule* g_module_open(const gchar *file_name, GModuleFlags flags);
gboolean g_module_close(GModule *module);
void g_module_make_resident(GModule *module);
G_CONST_RETURN gchar* g_module_error(void);
gboolean g_module_symbol(GModule *module, const gchar *symbol_name, gpointer *symbol);
G_CONST_RETURN gchar* g_module_name(GModule *module);
gchar* g_module_build_path(const gchar *directory, const gchar  *module_name);
G_END_DECLS
#endif