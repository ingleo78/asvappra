#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_DBUS_NAME_WATCHING_H__
#define __G_DBUS_NAME_WATCHING_H__

#include "../gobject/gclosure.h"
#include "giotypes.h"
#include "gioenums.h"

G_BEGIN_DECLS
typedef void (*GBusNameAppearedCallback)(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer user_data);
typedef void (*GBusNameVanishedCallback)(GDBusConnection *connection, const gchar *name, gpointer user_data);
guint g_bus_watch_name(GBusType bus_type, const gchar *name, GBusNameWatcherFlags flags, GBusNameAppearedCallback name_appeared_handler,
                       GBusNameVanishedCallback name_vanished_handler, gpointer user_data, GDestroyNotify user_data_free_func);
guint g_bus_watch_name_on_connection(GDBusConnection *connection, const gchar *name, GBusNameWatcherFlags flags, GBusNameAppearedCallback name_appeared_handler,
                                     GBusNameVanishedCallback name_vanished_handler, gpointer user_data, GDestroyNotify user_data_free_func);
guint g_bus_watch_name_with_closures(GBusType bus_type, const gchar *name, GBusNameWatcherFlags flags, GClosure *name_appeared_closure,
                                     GClosure *name_vanished_closure);
guint g_bus_watch_name_on_connection_with_closures(GDBusConnection *connection, const gchar *name, GBusNameWatcherFlags flags, GClosure *name_appeared_closure,
                                                   GClosure *name_vanished_closure);
void  g_bus_unwatch_name(guint watcher_id);
G_END_DECLS

#endif