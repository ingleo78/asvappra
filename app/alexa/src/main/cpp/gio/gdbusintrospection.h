#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_DBUS_INTROSPECTION_H__
#define __G_DBUS_INTROSPECTION_H__

#include "../glib/gstring.h"
#include "../gobject/gobject.h"
#include "giotypes.h"
#include "gioenums.h"

G_BEGIN_DECLS
struct _GDBusAnnotationInfo {
  volatile gint ref_count;
  gchar *key;
  gchar *value;
  GDBusAnnotationInfo **annotations;
};
struct _GDBusArgInfo {
  volatile gint ref_count;
  gchar *name;
  gchar *signature;
  GDBusAnnotationInfo **annotations;
};
struct _GDBusMethodInfo {
  volatile gint ref_count;
  gchar *name;
  GDBusArgInfo **in_args;
  GDBusArgInfo **out_args;
  GDBusAnnotationInfo **annotations;
};
struct _GDBusSignalInfo {
  volatile gint ref_count;
  gchar *name;
  GDBusArgInfo **args;
  GDBusAnnotationInfo **annotations;
};
struct _GDBusPropertyInfo {
  volatile gint ref_count;
  gchar *name;
  gchar *signature;
  GDBusPropertyInfoFlags flags;
  GDBusAnnotationInfo **annotations;
};
struct _GDBusInterfaceInfo {
  volatile gint ref_count;
  gchar *name;
  GDBusMethodInfo **methods;
  GDBusSignalInfo **signals;
  GDBusPropertyInfo **properties;
  GDBusAnnotationInfo **annotations;
};
struct _GDBusNodeInfo {
  volatile gint ref_count;
  gchar *path;
  GDBusInterfaceInfo **interfaces;
  GDBusNodeInfo **nodes;
  GDBusAnnotationInfo **annotations;
};
const gchar *g_dbus_annotation_info_lookup(GDBusAnnotationInfo **annotations, const gchar *name);
GDBusMethodInfo *g_dbus_interface_info_lookup_method(GDBusInterfaceInfo *info, const gchar *name);
GDBusSignalInfo *g_dbus_interface_info_lookup_signal(GDBusInterfaceInfo *info, const gchar *name);
GDBusPropertyInfo *g_dbus_interface_info_lookup_property(GDBusInterfaceInfo *info, const gchar *name);
void g_dbus_interface_info_generate_xml(GDBusInterfaceInfo *info, guint indent, GString *string_builder);
GDBusNodeInfo *g_dbus_node_info_new_for_xml(const gchar *xml_data, GError **error);
GDBusInterfaceInfo *g_dbus_node_info_lookup_interface(GDBusNodeInfo *info, const gchar *name);
void g_dbus_node_info_generate_xml(GDBusNodeInfo *info, guint indent, GString *string_builder);
GDBusNodeInfo *g_dbus_node_info_ref(GDBusNodeInfo *info);
GDBusInterfaceInfo *g_dbus_interface_info_ref(GDBusInterfaceInfo *info);
GDBusMethodInfo *g_dbus_method_info_ref(GDBusMethodInfo *info);
GDBusSignalInfo *g_dbus_signal_info_ref(GDBusSignalInfo *info);
GDBusPropertyInfo *g_dbus_property_info_ref(GDBusPropertyInfo *info);
GDBusArgInfo *g_dbus_arg_info_ref(GDBusArgInfo *info);
GDBusAnnotationInfo *g_dbus_annotation_info_ref(GDBusAnnotationInfo *info);
void g_dbus_node_info_unref(GDBusNodeInfo *info);
void g_dbus_interface_info_unref(GDBusInterfaceInfo *info);
void g_dbus_method_info_unref(GDBusMethodInfo *info);
void g_dbus_signal_info_unref(GDBusSignalInfo *info);
void g_dbus_property_info_unref(GDBusPropertyInfo *info);
void g_dbus_arg_info_unref(GDBusArgInfo *info);
void g_dbus_annotation_info_unref(GDBusAnnotationInfo *info);
#define G_TYPE_DBUS_NODE_INFO  (g_dbus_node_info_get_type())
#define G_TYPE_DBUS_INTERFACE_INFO  (g_dbus_interface_info_get_type())
#define G_TYPE_DBUS_METHOD_INFO  (g_dbus_method_info_get_type())
#define G_TYPE_DBUS_SIGNAL_INFO  (g_dbus_signal_info_get_type())
#define G_TYPE_DBUS_PROPERTY_INFO  (g_dbus_property_info_get_type())
#define G_TYPE_DBUS_ARG_INFO  (g_dbus_arg_info_get_type())
#define G_TYPE_DBUS_ANNOTATION_INFO  (g_dbus_annotation_info_get_type())
GType g_dbus_node_info_get_type(void) G_GNUC_CONST;
GType g_dbus_interface_info_get_type(void) G_GNUC_CONST;
GType g_dbus_method_info_get_type(void) G_GNUC_CONST;
GType g_dbus_signal_info_get_type(void) G_GNUC_CONST;
GType g_dbus_property_info_get_type(void) G_GNUC_CONST;
GType g_dbus_arg_info_get_type(void) G_GNUC_CONST;
GType g_dbus_annotation_info_get_type(void) G_GNUC_CONST;
G_END_DECLS

#endif