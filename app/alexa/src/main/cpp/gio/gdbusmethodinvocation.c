#include <stdlib.h>
#include "../glib/glibintl.h"
#include "config.h"
#include "gdbusutils.h"
#include "gdbusconnection.h"
#include "gdbusmessage.h"
#include "gdbusmethodinvocation.h"
#include "gdbusintrospection.h"
#include "gdbuserror.h"
#include "gdbusprivate.h"

typedef struct _GDBusMethodInvocationClass GDBusMethodInvocationClass;
struct _GDBusMethodInvocationClass {
  GObjectClass parent_class;
};
struct _GDBusMethodInvocation {
  GObject parent_instance;
  gchar *sender;
  gchar *object_path;
  gchar *interface_name;
  gchar *method_name;
  const GDBusMethodInfo *method_info;
  GDBusConnection *connection;
  GDBusMessage *message;
  GVariant *parameters;
  gpointer user_data;
};
G_DEFINE_TYPE(GDBusMethodInvocation, g_dbus_method_invocation, G_TYPE_OBJECT);
static void g_dbus_method_invocation_finalize(GObject *object) {
  GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(object);
  g_free(invocation->sender);
  g_free(invocation->object_path);
  g_free(invocation->interface_name);
  g_free(invocation->method_name);
  g_object_unref(invocation->connection);
  g_object_unref(invocation->message);
  g_variant_unref(invocation->parameters);
  G_OBJECT_CLASS(g_dbus_method_invocation_parent_class)->finalize(object);
}
static void g_dbus_method_invocation_class_init(GDBusMethodInvocationClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_dbus_method_invocation_finalize;
}
static void g_dbus_method_invocation_init(GDBusMethodInvocation *invocation) {}
const gchar *g_dbus_method_invocation_get_sender(GDBusMethodInvocation *invocation) {
  g_return_val_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation), NULL);
  return invocation->sender;
}
const gchar *g_dbus_method_invocation_get_object_path(GDBusMethodInvocation *invocation) {
  g_return_val_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation), NULL);
  return invocation->object_path;
}
const gchar *g_dbus_method_invocation_get_interface_name(GDBusMethodInvocation *invocation) {
  g_return_val_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation), NULL);
  return invocation->interface_name;
}
const GDBusMethodInfo *g_dbus_method_invocation_get_method_info(GDBusMethodInvocation *invocation) {
  g_return_val_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation), NULL);
  return invocation->method_info;
}
const gchar *g_dbus_method_invocation_get_method_name(GDBusMethodInvocation *invocation) {
  g_return_val_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation), NULL);
  return invocation->method_name;
}
GDBusConnection *g_dbus_method_invocation_get_connection(GDBusMethodInvocation *invocation) {
  g_return_val_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation), NULL);
  return invocation->connection;
}
GDBusMessage *g_dbus_method_invocation_get_message(GDBusMethodInvocation *invocation) {
  g_return_val_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation), NULL);
  return invocation->message;
}
GVariant *g_dbus_method_invocation_get_parameters(GDBusMethodInvocation *invocation) {
  g_return_val_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation), NULL);
  return invocation->parameters;
}
gpointer g_dbus_method_invocation_get_user_data(GDBusMethodInvocation *invocation) {
  g_return_val_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation), NULL);
  return invocation->user_data;
}
GDBusMethodInvocation *_g_dbus_method_invocation_new(const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name,
                                                     const GDBusMethodInfo *method_info, GDBusConnection *connection, GDBusMessage *message, GVariant *parameters,
                                                     gpointer user_data) {
  GDBusMethodInvocation *invocation;
  g_return_val_if_fail(sender == NULL || g_dbus_is_name(sender), NULL);
  g_return_val_if_fail(g_variant_is_object_path(object_path), NULL);
  g_return_val_if_fail(interface_name == NULL || g_dbus_is_interface_name(interface_name), NULL);
  g_return_val_if_fail(g_dbus_is_member_name(method_name), NULL);
  g_return_val_if_fail(G_IS_DBUS_CONNECTION(connection), NULL);
  g_return_val_if_fail(G_IS_DBUS_MESSAGE(message), NULL);
  g_return_val_if_fail(g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE), NULL);
  invocation = G_DBUS_METHOD_INVOCATION(g_object_new(G_TYPE_DBUS_METHOD_INVOCATION, NULL));
  invocation->sender = g_strdup(sender);
  invocation->object_path = g_strdup(object_path);
  invocation->interface_name = g_strdup(interface_name);
  invocation->method_name = g_strdup(method_name);
  invocation->method_info = g_dbus_method_info_ref((GDBusMethodInfo*)method_info);
  invocation->connection = g_object_ref(connection);
  invocation->message = g_object_ref(message);
  invocation->parameters = g_variant_ref(parameters);
  invocation->user_data = user_data;
  return invocation;
}
void g_dbus_method_invocation_return_value(GDBusMethodInvocation *invocation, GVariant *parameters) {
  GDBusMessage *reply;
  GError *error;
  g_return_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation));
  g_return_if_fail((parameters == NULL) || g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE));
  if (parameters == NULL) parameters = g_variant_new_tuple(NULL, 0);
  if (invocation->method_info != NULL) {
      GVariantType *type;
      type = _g_dbus_compute_complete_signature(invocation->method_info->out_args);
      if (!g_variant_is_of_type(parameters, type)) {
          gchar *type_string = g_variant_type_dup_string(type);
          g_warning("Type of return value is incorrect, got `%s', expected `%s'", g_variant_get_type_string(parameters), type_string);
          g_variant_type_free(type);
          g_free(type_string);
          goto out;
      }
      g_variant_type_free(type);
  }
  if (G_UNLIKELY(_g_dbus_debug_return())) {
      _g_dbus_debug_print_lock();
      g_print("========================================================================\nGDBus-debug:Return:\n >>>> METHOD RETURN\n      in response to "
              "%s.%s()\n      on object %s\n      to name %s\n      reply-serial %d\n", invocation->interface_name, invocation->method_name,
              invocation->object_path, invocation->sender, g_dbus_message_get_serial(invocation->message));
      _g_dbus_debug_print_unlock();
  }
  reply = g_dbus_message_new_method_reply(invocation->message);
  g_dbus_message_set_body(reply, parameters);
  error = NULL;
  if (!g_dbus_connection_send_message(g_dbus_method_invocation_get_connection(invocation), reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, &error)) {
      g_warning("Error sending message: %s", error->message);
      g_error_free(error);
  }
  g_object_unref(reply);
out:
  g_object_unref(invocation);
}
void g_dbus_method_invocation_return_error(GDBusMethodInvocation *invocation, GQuark domain, gint code, const gchar *format, ...) {
  va_list var_args;
  g_return_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation));
  g_return_if_fail(format != NULL);
  va_start(var_args, format);
  g_dbus_method_invocation_return_error_valist(invocation, domain, code, format, var_args);
  va_end(var_args);
}
void g_dbus_method_invocation_return_error_valist(GDBusMethodInvocation *invocation, GQuark domain, gint code, const gchar *format, va_list var_args) {
  gchar *literal_message;
  g_return_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation));
  g_return_if_fail(format != NULL);
  literal_message = g_strdup_vprintf(format, var_args);
  g_dbus_method_invocation_return_error_literal(invocation, domain, code, literal_message);
  g_free(literal_message);
}
void g_dbus_method_invocation_return_error_literal(GDBusMethodInvocation *invocation, GQuark domain, gint code, const gchar *message) {
  GError *error;
  g_return_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation));
  g_return_if_fail(message != NULL);
  error = g_error_new_literal(domain, code, message);
  g_dbus_method_invocation_return_gerror(invocation, error);
  g_error_free(error);
}
void g_dbus_method_invocation_return_gerror(GDBusMethodInvocation *invocation, const GError *error) {
  gchar *dbus_error_name;
  g_return_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation));
  g_return_if_fail(error != NULL);
  dbus_error_name = g_dbus_error_encode_gerror(error);
  g_dbus_method_invocation_return_dbus_error(invocation, dbus_error_name, error->message);
  g_free(dbus_error_name);
}
void g_dbus_method_invocation_return_dbus_error(GDBusMethodInvocation *invocation, const gchar *error_name, const gchar *error_message) {
  GDBusMessage *reply;
  g_return_if_fail(G_IS_DBUS_METHOD_INVOCATION(invocation));
  g_return_if_fail(error_name != NULL && g_dbus_is_name(error_name));
  g_return_if_fail(error_message != NULL);
  if (G_UNLIKELY(_g_dbus_debug_return())) {
      _g_dbus_debug_print_lock();
      g_print("========================================================================\nGDBus-debug:Return:\n >>>> METHOD ERROR %s\n      message `%s'\n"
              "      in response to %s.%s()\n      on object %s\n      to name %s\n      reply-serial %d\n", error_name, error_message,
              invocation->interface_name, invocation->method_name, invocation->object_path, invocation->sender, g_dbus_message_get_serial(invocation->message));
      _g_dbus_debug_print_unlock();
  }
  reply = g_dbus_message_new_method_error_literal(invocation->message, error_name, error_message);
  g_dbus_connection_send_message(g_dbus_method_invocation_get_connection(invocation), reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
  g_object_unref(reply);
  g_object_unref(invocation);
}