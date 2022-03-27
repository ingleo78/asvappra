#if defined (GIO_COMPILATION)
#error "gdbusauthmechanismanon.h is a private header file."
#endif

#ifndef __G_DBUS_AUTH_MECHANISM_ANON_H__
#define __G_DBUS_AUTH_MECHANISM_ANON_H__

#include "giotypes.h"
#include "gdbusauthmechanism.h"

G_BEGIN_DECLS
#define G_TYPE_DBUS_AUTH_MECHANISM_ANON  (_g_dbus_auth_mechanism_anon_get_type())
#define G_DBUS_AUTH_MECHANISM_ANON(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_DBUS_AUTH_MECHANISM_ANON, GDBusAuthMechanismAnon))
#define G_DBUS_AUTH_MECHANISM_ANON_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_DBUS_AUTH_MECHANISM_ANON, GDBusAuthMechanismAnonClass))
#define G_DBUS_AUTH_MECHANISM_ANON_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_DBUS_AUTH_MECHANISM_ANON, GDBusAuthMechanismAnonClass))
#define G_IS_DBUS_AUTH_MECHANISM_ANON(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_DBUS_AUTH_MECHANISM_ANON))
#define G_IS_DBUS_AUTH_MECHANISM_ANON_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_DBUS_AUTH_MECHANISM_ANON))
typedef struct _GDBusAuthMechanismAnon GDBusAuthMechanismAnon;
typedef struct _GDBusAuthMechanismAnonClass GDBusAuthMechanismAnonClass;
typedef struct _GDBusAuthMechanismAnonPrivate GDBusAuthMechanismAnonPrivate;
struct _GDBusAuthMechanismAnonClass {
  GDBusAuthMechanismClass parent_class;
};
struct _GDBusAuthMechanismAnon {
  GDBusAuthMechanism parent_instance;
  GDBusAuthMechanismAnonPrivate *priv;
};
GType _g_dbus_auth_mechanism_anon_get_type(void) G_GNUC_CONST;
G_END_DECLS

#endif