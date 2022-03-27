#include "../glib/glibintl.h"
#include "config.h"
#include "ginitable.h"

typedef GInitableIface GInitableInterface;
G_DEFINE_INTERFACE(GInitable, g_initable, G_TYPE_OBJECT);
static void g_initable_default_init(GInitableInterface *iface) {}
gboolean g_initable_init(GInitable *initable, GCancellable *cancellable, GError **error) {
  GInitableIface *iface;
  g_return_val_if_fail(G_IS_INITABLE(initable), FALSE);
  iface = G_INITABLE_GET_IFACE(initable);
  return (*iface->init)(initable, cancellable, error);
}
gpointer g_initable_new(GType object_type, GCancellable *cancellable, GError **error, const gchar *first_property_name, ...) {
  GObject *object;
  va_list var_args;
  va_start(var_args, first_property_name);
  object = g_initable_new_valist(object_type, first_property_name, var_args, cancellable, error);
  va_end(var_args);
  return object;
}
gpointer g_initable_newv(GType object_type, guint n_parameters, GParameter *parameters, GCancellable *cancellable, GError **error) {
  GObject *obj;
  g_return_val_if_fail(G_TYPE_IS_INITABLE(object_type), NULL);
  obj = g_object_newv(object_type, n_parameters, parameters);
  if (!g_initable_init(G_INITABLE(obj), cancellable, error)) {
      g_object_unref(obj);
      return NULL;
  }
  return (gpointer)obj;
}
GObject* g_initable_new_valist(GType object_type, const gchar *first_property_name, va_list var_args, GCancellable *cancellable, GError **error) {
  GObject *obj;
  g_return_val_if_fail(G_TYPE_IS_INITABLE(object_type), NULL);
  obj = g_object_new_valist(object_type, first_property_name, var_args);
  if (!g_initable_init(G_INITABLE(obj), cancellable, error)) {
      g_object_unref(obj);
      return NULL;
  }
  return obj;
}