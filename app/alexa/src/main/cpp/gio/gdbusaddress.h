#ifndef __G_DBUS_ADDRESS_H__
#define __G_DBUS_ADDRESS_H__

#include "giotypes.h"
#include "gioenums.h"

G_BEGIN_DECLS
gboolean g_dbus_is_address(const gchar *string);
gboolean g_dbus_is_supported_address(const gchar  *string, GError **error);
void g_dbus_address_get_stream(const gchar *address, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GIOStream *g_dbus_address_get_stream_finish(GAsyncResult *res, gchar **out_guid, GError **error);
GIOStream *g_dbus_address_get_stream_sync(const gchar *address, gchar **out_guid, GCancellable *cancellable, GError **error);
gchar *g_dbus_address_get_for_bus_sync(GBusType bus_type, GCancellable *cancellable, GError **error);
G_END_DECLS

#endif