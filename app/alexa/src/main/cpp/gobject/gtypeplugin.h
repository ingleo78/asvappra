#if defined (__GLIB_GOBJECT_H_INSIDE__) && defined (GOBJECT_COMPILATION)
//#error "Only <glib-object.h> can be included directly."
#endif

#ifndef __G_TYPE_PLUGIN_H__
#define __G_TYPE_PLUGIN_H__

#include "gtype.h"

G_BEGIN_DECLS
#define G_TYPE_TYPE_PLUGIN	(g_type_plugin_get_type())
#define G_TYPE_PLUGIN(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_TYPE_PLUGIN, GTypePlugin))
#define G_TYPE_PLUGIN_CLASS(vtable)  (G_TYPE_CHECK_CLASS_CAST((vtable), G_TYPE_TYPE_PLUGIN, GTypePluginClass))
#define G_IS_TYPE_PLUGIN(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_TYPE_PLUGIN))
#define G_IS_TYPE_PLUGIN_CLASS(vtable)	(G_TYPE_CHECK_CLASS_TYPE((vtable), G_TYPE_TYPE_PLUGIN))
#define G_TYPE_PLUGIN_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE((inst), G_TYPE_TYPE_PLUGIN, GTypePluginClass))
typedef struct _GTypePluginClass  GTypePluginClass;
typedef void (*GTypePluginUse)(GTypePlugin *plugin);
typedef void (*GTypePluginUnuse)(GTypePlugin *plugin);
typedef void (*GTypePluginCompleteTypeInfo)(GTypePlugin *plugin, GType g_type, GTypeInfo *info, GTypeValueTable *value_table);
typedef void (*GTypePluginCompleteInterfaceInfo)(GTypePlugin *plugin, GType instance_type, GType interface_type, GInterfaceInfo *info);
struct _GTypePluginClass {
  GTypeInterface base_iface;
  GTypePluginUse use_plugin;
  GTypePluginUnuse unuse_plugin;
  GTypePluginCompleteTypeInfo complete_type_info;
  GTypePluginCompleteInterfaceInfo complete_interface_info;
};
GType g_type_plugin_get_type(void) G_GNUC_CONST;
void g_type_plugin_use(GTypePlugin *plugin);
void g_type_plugin_unuse(GTypePlugin *plugin);
void g_type_plugin_complete_type_info(GTypePlugin *plugin, GType g_type, GTypeInfo *info, GTypeValueTable *value_table);
void g_type_plugin_complete_interface_info(GTypePlugin *plugin, GType instance_type, GType interface_type, GInterfaceInfo *info);
G_END_DECLS
#endif