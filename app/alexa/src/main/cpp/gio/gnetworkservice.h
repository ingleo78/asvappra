#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_NETWORK_SERVICE_H__
#define __G_NETWORK_SERVICE_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_NETWORK_SERVICE  (g_network_service_get_type())
#define G_NETWORK_SERVICE(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_NETWORK_SERVICE, GNetworkService))
#define G_NETWORK_SERVICE_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_NETWORK_SERVICE, GNetworkServiceClass))
#define G_IS_NETWORK_SERVICE(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_NETWORK_SERVICE))
#define G_IS_NETWORK_SERVICE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_NETWORK_SERVICE))
#define G_NETWORK_SERVICE_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_NETWORK_SERVICE, GNetworkServiceClass))
typedef struct _GNetworkServiceClass GNetworkServiceClass;
typedef struct _GNetworkServicePrivate GNetworkServicePrivate;
struct _GNetworkService {
  GObject parent_instance;
  GNetworkServicePrivate *priv;
};
struct _GNetworkServiceClass {
  GObjectClass parent_class;
};
GType g_network_service_get_type(void) G_GNUC_CONST;
GSocketConnectable *g_network_service_new(const gchar *service, const gchar *protocol, const gchar *domain);
const gchar *g_network_service_get_service(GNetworkService *srv);
const gchar *g_network_service_get_protocol(GNetworkService *srv);
const gchar *g_network_service_get_domain(GNetworkService *srv);
const gchar *g_network_service_get_scheme(GNetworkService *srv);
void g_network_service_set_scheme(GNetworkService *srv, const gchar *scheme);
G_END_DECLS

#endif