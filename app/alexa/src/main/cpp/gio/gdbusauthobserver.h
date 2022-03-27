#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_DBUS_AUTH_OBSERVER_H__
#define __G_DBUS_AUTH_OBSERVER_H__

#include "../gobject/gtype.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_DBUS_AUTH_OBSERVER  (g_dbus_auth_observer_get_type())
#define G_DBUS_AUTH_OBSERVER(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_DBUS_AUTH_OBSERVER, GDBusAuthObserver))
#define G_IS_DBUS_AUTH_OBSERVER(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_DBUS_AUTH_OBSERVER))
GType g_dbus_auth_observer_get_type(void) G_GNUC_CONST;
GDBusAuthObserver *g_dbus_auth_observer_new(void);
gboolean g_dbus_auth_observer_authorize_authenticated_peer(GDBusAuthObserver *observer, GIOStream *stream, GCredentials *credentials);
G_END_DECLS

#endif