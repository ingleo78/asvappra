#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_SOCKET_ADDRESS_H__
#define __G_SOCKET_ADDRESS_H__

#include "../gobject/gobject.h"
#include "gioenums.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_SOCKET_ADDRESS  (g_socket_address_get_type())
#define G_SOCKET_ADDRESS(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_SOCKET_ADDRESS, GSocketAddress))
#define G_SOCKET_ADDRESS_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_SOCKET_ADDRESS, GSocketAddressClass))
#define G_IS_SOCKET_ADDRESS(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_SOCKET_ADDRESS))
#define G_IS_SOCKET_ADDRESS_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_SOCKET_ADDRESS))
#define G_SOCKET_ADDRESS_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_SOCKET_ADDRESS, GSocketAddressClass))
typedef struct _GSocketAddressClass GSocketAddressClass;
struct _GSocketAddress {
  GObject parent_instance;
};
struct _GSocketAddressClass {
  GObjectClass parent_class;
  GSocketFamily (*get_family)(GSocketAddress *address);
  gssize (*get_native_size)(GSocketAddress *address);
  gboolean (*to_native)(GSocketAddress *address, gpointer dest, gsize destlen, GError **error);
};
GType g_socket_address_get_type(void) G_GNUC_CONST;
GSocketFamily g_socket_address_get_family(GSocketAddress *address);
GSocketAddress *g_socket_address_new_from_native(gpointer native, gsize len);
gboolean g_socket_address_to_native(GSocketAddress *address, gpointer dest, gsize destlen, GError **error);
gssize g_socket_address_get_native_size(GSocketAddress *address);
G_END_DECLS

#endif