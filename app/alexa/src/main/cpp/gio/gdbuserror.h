#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_DBUS_ERROR_H__
#define __G_DBUS_ERROR_H__

#include <stdarg.h>
#include "giotypes.h"

G_BEGIN_DECLS
#define G_DBUS_ERROR g_dbus_error_quark()
GQuark g_dbus_error_quark(void);
gboolean g_dbus_error_is_remote_error(const GError *error);
gchar *g_dbus_error_get_remote_error(const GError *error);
gboolean g_dbus_error_strip_remote_error(GError *error);
struct _GDBusErrorEntry {
  gint error_code;
  const gchar *dbus_error_name;
};
gboolean g_dbus_error_register_error(GQuark error_domain, gint error_code, const gchar *dbus_error_name);
gboolean g_dbus_error_unregister_error(GQuark error_domain, gint error_code, const gchar *dbus_error_name);
void g_dbus_error_register_error_domain(const gchar *error_domain_quark_name, volatile gsize *quark_volatile, const GDBusErrorEntry *entries, guint num_entries);
GError *g_dbus_error_new_for_dbus_error(const gchar *dbus_error_name, const gchar *dbus_error_message);
void g_dbus_error_set_dbus_error(GError **error, const gchar *dbus_error_name, const gchar *dbus_error_message, const gchar *format, ...);
void g_dbus_error_set_dbus_error_valist(GError **error, const gchar *dbus_error_name, const gchar *dbus_error_message, const gchar *format, va_list var_args);
gchar *g_dbus_error_encode_gerror(const GError *error);
G_END_DECLS

#endif