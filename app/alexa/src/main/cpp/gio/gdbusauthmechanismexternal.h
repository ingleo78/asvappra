#if defined (GIO_COMPILATION)
#error "gdbusauthmechanismexternal.h is a private header file."
#endif

#ifndef __G_DBUS_AUTH_MECHANISM_EXTERNAL_H__
#define __G_DBUS_AUTH_MECHANISM_EXTERNAL_H__

#include "giotypes.h"
#include "gdbusauthmechanism.h"

G_BEGIN_DECLS
#define G_TYPE_DBUS_AUTH_MECHANISM_EXTERNAL  (_g_dbus_auth_mechanism_external_get_type ())
#define G_DBUS_AUTH_MECHANISM_EXTERNAL(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_DBUS_AUTH_MECHANISM_EXTERNAL, GDBusAuthMechanismExternal))
#define G_DBUS_AUTH_MECHANISM_EXTERNAL_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_DBUS_AUTH_MECHANISM_EXTERNAL, GDBusAuthMechanismExternalClass))
#define G_DBUS_AUTH_MECHANISM_EXTERNAL_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), G_TYPE_DBUS_AUTH_MECHANISM_EXTERNAL, GDBusAuthMechanismExternalClass))
#define G_IS_DBUS_AUTH_MECHANISM_EXTERNAL(o)  (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_DBUS_AUTH_MECHANISM_EXTERNAL))
#define G_IS_DBUS_AUTH_MECHANISM_EXTERNAL_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_DBUS_AUTH_MECHANISM_EXTERNAL))
typedef struct _GDBusAuthMechanismExternal GDBusAuthMechanismExternal;
typedef struct _GDBusAuthMechanismExternalClass GDBusAuthMechanismExternalClass;
typedef struct _GDBusAuthMechanismExternalPrivate GDBusAuthMechanismExternalPrivate;
struct _GDBusAuthMechanismExternalClass {
  GDBusAuthMechanismClass parent_class;
};
struct _GDBusAuthMechanismExternal {
  GDBusAuthMechanism parent_instance;
  GDBusAuthMechanismExternalPrivate *priv;
};
GType _g_dbus_auth_mechanism_external_get_type(void) G_GNUC_CONST;
G_END_DECLS

#endif