#include "../gio/config.h"
#include "../glib/glib.h"
#include "gtypeplugin.h"

GType g_type_plugin_get_type(void) {
  static GType type_plugin_type = 0;
  if (!type_plugin_type) {
      static const GTypeInfo type_plugin_info = {
          sizeof(GTypePluginClass),
          NULL,
          NULL
      };
      type_plugin_type = g_type_register_static(G_TYPE_INTERFACE, g_intern_static_string("GTypePlugin"), &type_plugin_info, 0);
  }
  return type_plugin_type;
}
void g_type_plugin_use(GTypePlugin *plugin) {
  GTypePluginClass *iface;
  g_return_if_fail(G_IS_TYPE_PLUGIN(plugin));
  iface = G_TYPE_PLUGIN_GET_CLASS(plugin);
  iface->use_plugin(plugin);
}
void g_type_plugin_unuse(GTypePlugin *plugin) {
  GTypePluginClass *iface;
  g_return_if_fail(G_IS_TYPE_PLUGIN(plugin));
  iface = G_TYPE_PLUGIN_GET_CLASS(plugin);
  iface->unuse_plugin(plugin);
}
void g_type_plugin_complete_type_info(GTypePlugin *plugin, GType g_type, GTypeInfo *info, GTypeValueTable *value_table) {
  GTypePluginClass *iface;
  g_return_if_fail(G_IS_TYPE_PLUGIN (plugin));
  g_return_if_fail(info != NULL);
  g_return_if_fail(value_table != NULL);
  iface = G_TYPE_PLUGIN_GET_CLASS(plugin);
  iface->complete_type_info(plugin, g_type, info, value_table);
}
void g_type_plugin_complete_interface_info(GTypePlugin *plugin, GType instance_type, GType interface_type, GInterfaceInfo *info) {
  GTypePluginClass *iface;
  g_return_if_fail(G_IS_TYPE_PLUGIN (plugin));
  g_return_if_fail(info != NULL);
  iface = G_TYPE_PLUGIN_GET_CLASS(plugin);
  iface->complete_interface_info(plugin, instance_type, interface_type, info);
}