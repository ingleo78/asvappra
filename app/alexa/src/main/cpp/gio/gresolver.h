#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_RESOLVER_H__
#define __G_RESOLVER_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_RESOLVER  (g_resolver_get_type())
#define G_RESOLVER(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_RESOLVER, GResolver))
#define G_RESOLVER_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_RESOLVER, GResolverClass))
#define G_IS_RESOLVER(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_RESOLVER))
#define G_IS_RESOLVER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_RESOLVER))
#define G_RESOLVER_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_RESOLVER, GResolverClass))
typedef struct _GResolverPrivate GResolverPrivate;
typedef struct _GResolverClass GResolverClass;
struct _GResolver {
  GObject parent_instance;
  GResolverPrivate *priv;
};
struct _GResolverClass {
  GObjectClass parent_class;
  void (*reload)(GResolver *resolver);
  GList *(*lookup_by_name)(GResolver *resolver, const gchar *hostname, GCancellable *cancellable, GError **error);
  void (*lookup_by_name_async)(GResolver *resolver, const gchar *hostname, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  GList *(*lookup_by_name_finish)(GResolver *resolver, GAsyncResult *result, GError **error);
  gchar *(*lookup_by_address)(GResolver *resolver, GInetAddress *address, GCancellable *cancellable, GError **error);
  void (*lookup_by_address_async)(GResolver *resolver, GInetAddress *address, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gchar *(*lookup_by_address_finish)(GResolver *resolver, GAsyncResult *result, GError **error);
  GList *(*lookup_service)(GResolver *resolver, const gchar *rrname, GCancellable *cancellable, GError **error);
  void (*lookup_service_async)(GResolver *resolver, const gchar *rrname, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  GList *(*lookup_service_finish)(GResolver *resolver, GAsyncResult *result, GError **error);
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
  void (*_g_reserved6)(void);
};
GType g_resolver_get_type(void) G_GNUC_CONST;
GResolver *g_resolver_get_default(void);
void g_resolver_set_default(GResolver *resolver);
GList *g_resolver_lookup_by_name(GResolver *resolver, const gchar *hostname, GCancellable *cancellable, GError **error);
void g_resolver_lookup_by_name_async(GResolver *resolver, const gchar *hostname, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GList  *g_resolver_lookup_by_name_finish(GResolver *resolver, GAsyncResult *result, GError **error);
void g_resolver_free_addresses(GList *addresses);
gchar *g_resolver_lookup_by_address(GResolver *resolver, GInetAddress *address, GCancellable *cancellable, GError **error);
void g_resolver_lookup_by_address_async(GResolver *resolver, GInetAddress *address, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gchar *g_resolver_lookup_by_address_finish(GResolver *resolver, GAsyncResult *result, GError **error);
GList *g_resolver_lookup_service(GResolver *resolver, const gchar *service, const gchar *protocol, const gchar *domain, GCancellable *cancellable, GError **error);
void g_resolver_lookup_service_async(GResolver *resolver, const gchar *service, const gchar *protocol, const gchar *domain, GCancellable *cancellable,
						 			 GAsyncReadyCallback callback, gpointer user_data);
GList *g_resolver_lookup_service_finish(GResolver *resolver, GAsyncResult *result, GError **error);
void g_resolver_free_targets(GList *targets);
#define G_RESOLVER_ERROR  (g_resolver_error_quark())
GQuark g_resolver_error_quark(void);
G_END_DECLS

#endif