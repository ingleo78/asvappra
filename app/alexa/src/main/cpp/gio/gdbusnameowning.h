#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_DBUS_NAME_OWNING_H__
#define __G_DBUS_NAME_OWNING_H__

#include "../gobject/gclosure.h"
#include "giotypes.h"
#include "gioenums.h"

G_BEGIN_DECLS
typedef void (*GBusAcquiredCallback)(GDBusConnection *connection, const gchar *name, gpointer user_data);
typedef void (*GBusNameAcquiredCallback)(GDBusConnection *connection, const gchar *name, gpointer user_data);
typedef void (*GBusNameLostCallback)(GDBusConnection *connection, const gchar *name, gpointer user_data);
guint g_bus_own_name(GBusType bus_type, const gchar *name, GBusNameOwnerFlags flags, GBusAcquiredCallback bus_acquired_handler,
                     GBusNameAcquiredCallback name_acquired_handler, GBusNameLostCallback name_lost_handler, gpointer user_data,
                     GDestroyNotify user_data_free_func);
guint g_bus_own_name_on_connection(GDBusConnection *connection, const gchar *name, GBusNameOwnerFlags flags,GBusNameAcquiredCallback name_acquired_handler,
                                   GBusNameLostCallback name_lost_handler, gpointer user_data, GDestroyNotify user_data_free_func);
guint g_bus_own_name_with_closures(GBusType bus_type, const gchar *name, GBusNameOwnerFlags flags, GClosure *bus_acquired_closure, GClosure *name_acquired_closure,
                                   GClosure*name_lost_closure);
guint g_bus_own_name_on_connection_with_closures(GDBusConnection *connection, const gchar *name, GBusNameOwnerFlags flags, GClosure *name_acquired_closure,
                                                 GClosure *name_lost_closure);
void  g_bus_unown_name(guint owner_id);

G_END_DECLS

#endif