#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_SOCKET_ADDRESS_ENUMERATOR_H__
#define __G_SOCKET_ADDRESS_ENUMERATOR_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_SOCKET_ADDRESS_ENUMERATOR  (g_socket_address_enumerator_get_type())
#define G_SOCKET_ADDRESS_ENUMERATOR(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_SOCKET_ADDRESS_ENUMERATOR, GSocketAddressEnumerator))
#define G_SOCKET_ADDRESS_ENUMERATOR_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_SOCKET_ADDRESS_ENUMERATOR, GSocketAddressEnumeratorClass))
#define G_IS_SOCKET_ADDRESS_ENUMERATOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_SOCKET_ADDRESS_ENUMERATOR))
#define G_IS_SOCKET_ADDRESS_ENUMERATOR_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_SOCKET_ADDRESS_ENUMERATOR))
#define G_SOCKET_ADDRESS_ENUMERATOR_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_SOCKET_ADDRESS_ENUMERATOR, GSocketAddressEnumeratorClass))
typedef struct _GSocketAddressEnumeratorClass GSocketAddressEnumeratorClass;
struct _GSocketAddressEnumerator {
  GObject parent_instance;
};
struct _GSocketAddressEnumeratorClass {
  GObjectClass parent_class;
  GSocketAddress *(*next)(GSocketAddressEnumerator *enumerator, GCancellable *cancellable, GError **error);
  void (*next_async)(GSocketAddressEnumerator *enumerator, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  GSocketAddress *(*next_finish)(GSocketAddressEnumerator *enumerator, GAsyncResult *result, GError **error);
};
GType g_socket_address_enumerator_get_type(void) G_GNUC_CONST;
GSocketAddress *g_socket_address_enumerator_next(GSocketAddressEnumerator *enumerator, GCancellable *cancellable, GError **error);
void g_socket_address_enumerator_next_async(GSocketAddressEnumerator *enumerator, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GSocketAddress *g_socket_address_enumerator_next_finish(GSocketAddressEnumerator *enumerator, GAsyncResult *result, GError **error);
G_END_DECLS

#endif