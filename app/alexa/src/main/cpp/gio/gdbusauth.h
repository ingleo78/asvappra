#if defined (GIO_COMPILATION)
#error "gdbusauth.h is a private header file."
#endif

#ifndef __G_DBUS_AUTH_H__
#define __G_DBUS_AUTH_H__

#include "../gobject/gobject.h"
#include "giotypes.h"
#include "gioenums.h"

G_BEGIN_DECLS
#define G_TYPE_DBUS_AUTH  (_g_dbus_auth_get_type ())
#define G_DBUS_AUTH(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_DBUS_AUTH, GDBusAuth))
#define G_DBUS_AUTH_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_DBUS_AUTH, GDBusAuthClass))
#define G_DBUS_AUTH_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), G_TYPE_DBUS_AUTH, GDBusAuthClass))
#define G_IS_DBUS_AUTH(o)  (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_DBUS_AUTH))
#define G_IS_DBUS_AUTH_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_DBUS_AUTH))
typedef struct _GDBusAuth GDBusAuth;
typedef struct _GDBusAuthClass GDBusAuthClass;
typedef struct _GDBusAuthPrivate GDBusAuthPrivate;
struct _GDBusAuthClass {
  GObjectClass parent_class;
};
struct _GDBusAuth {
  GObject parent_instance;
  GDBusAuthPrivate *priv;
};
GType _g_dbus_auth_get_type(void) G_GNUC_CONST;
GDBusAuth *_g_dbus_auth_new(GIOStream *stream);
gboolean _g_dbus_auth_run_server(GDBusAuth *auth, GDBusAuthObserver *observer, const gchar *guid, gboolean allow_anonymous, GDBusCapabilityFlags offered_capabilities,
                                 GDBusCapabilityFlags *out_negotiated_capabilities, GCredentials **out_received_credentials, GCancellable *cancellable,
                                 GError **error);
gchar *_g_dbus_auth_run_client(GDBusAuth *auth, GDBusCapabilityFlags offered_capabilities, GDBusCapabilityFlags *out_negotiated_capabilities,
                               GCancellable *cancellable, GError **error);
G_END_DECLS

#endif