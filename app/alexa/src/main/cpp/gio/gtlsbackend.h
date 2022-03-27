#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_TLS_BACKEND_H__
#define __G_TLS_BACKEND_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TLS_BACKEND_EXTENSION_POINT_NAME "gio-tls-backend"
#define G_TYPE_TLS_BACKEND  (g_tls_backend_get_type())
#define G_TLS_BACKEND(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_TLS_BACKEND, GTlsBackend))
#define G_IS_TLS_BACKEND(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_TLS_BACKEND))
#define G_TLS_BACKEND_GET_INTERFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_TLS_BACKEND, GTlsBackendInterface))
typedef struct _GTlsBackend GTlsBackend;
typedef struct _GTlsBackendInterface GTlsBackendInterface;
struct _GTlsBackendInterface {
  GTypeInterface g_iface;
  gboolean (*supports_tls)(GTlsBackend *backend);
  GType (*get_certificate_type)(void);
  GType (*get_client_connection_type)(void);
  GType (*get_server_connection_type)(void);
};
GType g_tls_backend_get_type(void) G_GNUC_CONST;
GTlsBackend *g_tls_backend_get_default(void);
gboolean g_tls_backend_supports_tls(GTlsBackend *backend);
GType g_tls_backend_get_certificate_type(GTlsBackend *backend);
GType g_tls_backend_get_client_connection_type(GTlsBackend *backend);
GType g_tls_backend_get_server_connection_type(GTlsBackend *backend);
G_END_DECLS

#endif