#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_DBUS_METHOD_INVOCATION_H__
#define __G_DBUS_METHOD_INVOCATION_H__

#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_DBUS_METHOD_INVOCATION  (g_dbus_method_invocation_get_type())
#define G_DBUS_METHOD_INVOCATION(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_DBUS_METHOD_INVOCATION, GDBusMethodInvocation))
#define G_IS_DBUS_METHOD_INVOCATION(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_DBUS_METHOD_INVOCATION))
GType g_dbus_method_invocation_get_type(void) G_GNUC_CONST;
const gchar *g_dbus_method_invocation_get_sender(GDBusMethodInvocation *invocation);
const gchar *g_dbus_method_invocation_get_object_path(GDBusMethodInvocation *invocation);
const gchar *g_dbus_method_invocation_get_interface_name(GDBusMethodInvocation *invocation);
const gchar *g_dbus_method_invocation_get_method_name(GDBusMethodInvocation *invocation);
const GDBusMethodInfo *g_dbus_method_invocation_get_method_info(GDBusMethodInvocation *invocation);
GDBusConnection *g_dbus_method_invocation_get_connection(GDBusMethodInvocation *invocation);
GDBusMessage *g_dbus_method_invocation_get_message(GDBusMethodInvocation *invocation);
GVariant *g_dbus_method_invocation_get_parameters(GDBusMethodInvocation *invocation);
gpointer g_dbus_method_invocation_get_user_data(GDBusMethodInvocation *invocation);
void g_dbus_method_invocation_return_value(GDBusMethodInvocation *invocation, GVariant *parameters);
void g_dbus_method_invocation_return_error(GDBusMethodInvocation *invocation, GQuark domain, gint code, const gchar *format, ...);
void g_dbus_method_invocation_return_error_valist(GDBusMethodInvocation *invocation, GQuark domain, gint code, const gchar *format, va_list var_args);
void g_dbus_method_invocation_return_error_literal(GDBusMethodInvocation *invocation, GQuark domain, gint code, const gchar *message);
void g_dbus_method_invocation_return_gerror(GDBusMethodInvocation *invocation, const GError *error);
void g_dbus_method_invocation_return_dbus_error(GDBusMethodInvocation *invocation, const gchar *error_name, const gchar *error_message);
G_END_DECLS

#endif