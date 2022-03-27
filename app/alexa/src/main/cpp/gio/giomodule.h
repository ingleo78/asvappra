#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_IO_MODULE_H__
#define __G_IO_MODULE_H__

#include "../gmodule/gmodule.h"
#include "../gobject/gtype.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_IO_TYPE_MODULE  (g_io_module_get_type())
#define G_IO_MODULE(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_IO_TYPE_MODULE, GIOModule))
#define G_IO_MODULE_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_IO_TYPE_MODULE, GIOModuleClass))
#define G_IO_IS_MODULE(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_IO_TYPE_MODULE))
#define G_IO_IS_MODULE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_IO_TYPE_MODULE))
#define G_IO_MODULE_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_IO_TYPE_MODULE, GIOModuleClass))
typedef struct _GIOModuleClass GIOModuleClass;
GType g_io_module_get_type(void) G_GNUC_CONST;
GIOModule *g_io_module_new(const gchar *filename);
void g_io_modules_scan_all_in_directory(const char *dirname);
GList *g_io_modules_load_all_in_directory(const gchar *dirname);
GIOExtensionPoint *g_io_extension_point_register(const char *name);
GIOExtensionPoint *g_io_extension_point_lookup(const char *name);
void g_io_extension_point_set_required_type(GIOExtensionPoint *extension_point, GType type);
GType g_io_extension_point_get_required_type(GIOExtensionPoint *extension_point);
GList *g_io_extension_point_get_extensions(GIOExtensionPoint *extension_point);
GIOExtension *g_io_extension_point_get_extension_by_name(GIOExtensionPoint *extension_point, const char *name);
GIOExtension *g_io_extension_point_implement(const char *extension_point_name, GType type, const char *extension_name, gint priority);
GType g_io_extension_get_type(GIOExtension *extension);
const char *g_io_extension_get_name(GIOExtension *extension);
gint g_io_extension_get_priority(GIOExtension *extension);
GTypeClass* g_io_extension_ref_class(GIOExtension *extension);
void g_io_module_load(GIOModule *module);
void g_io_module_unload(GIOModule *module);
char **g_io_module_query(void);
G_END_DECLS

#endif