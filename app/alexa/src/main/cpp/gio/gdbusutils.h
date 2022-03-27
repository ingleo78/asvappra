#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_DBUS_UTILS_H__
#define __G_DBUS_UTILS_H__

#include "giotypes.h"

G_BEGIN_DECLS
gboolean g_dbus_is_guid(const gchar *string);
gchar *g_dbus_generate_guid(void);
gboolean g_dbus_is_name(const gchar *string);
gboolean g_dbus_is_unique_name(const gchar *string);
gboolean g_dbus_is_member_name(const gchar *string);
gboolean g_dbus_is_interface_name(const gchar *string);
G_END_DECLS

#endif