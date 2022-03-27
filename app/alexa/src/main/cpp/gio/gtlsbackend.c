#include "../glib/glib.h"
#include "config.h"
#include "gtlsbackend.h"
#include "gdummytlsbackend.h"
#include "gioenumtypes.h"
#include "giomodule-priv.h"

G_DEFINE_INTERFACE (GTlsBackend, g_tls_backend, G_TYPE_OBJECT);
static void g_tls_backend_default_init(GTlsBackendInterface *iface) {}
static gpointer get_default_tls_backend(gpointer arg) {
  const char *use_this;
  GList *extensions;
  GIOExtensionPoint *ep;
  GIOExtension *extension;
  _g_io_modules_ensure_loaded();
  ep = g_io_extension_point_lookup(G_TLS_BACKEND_EXTENSION_POINT_NAME);
  use_this = g_getenv("GIO_USE_TLS");
  if (use_this) {
      extension = g_io_extension_point_get_extension_by_name(ep, use_this);
      if (extension) return g_object_new(g_io_extension_get_type(extension), NULL);
  }
  extensions = g_io_extension_point_get_extensions(ep);
  if (extensions) {
      extension = extensions->data;
      return g_object_new(g_io_extension_get_type(extension), NULL);
  }
  return NULL;
}
GTlsBackend *g_tls_backend_get_default(void) {
  static GOnce once_init = G_ONCE_INIT;
  return g_once(&once_init, get_default_tls_backend, NULL);
}
gboolean g_tls_backend_supports_tls(GTlsBackend *backend) {
  if (G_TLS_BACKEND_GET_INTERFACE(backend)->supports_tls) return G_TLS_BACKEND_GET_INTERFACE(backend)->supports_tls(backend);
  else if (G_IS_DUMMY_TLS_BACKEND(backend)) return FALSE;
  else return TRUE;
}
GType g_tls_backend_get_certificate_type(GTlsBackend *backend) {
  return G_TLS_BACKEND_GET_INTERFACE(backend)->get_certificate_type();
}
GType g_tls_backend_get_client_connection_type(GTlsBackend *backend) {
  return G_TLS_BACKEND_GET_INTERFACE(backend)->get_client_connection_type();
}
GType g_tls_backend_get_server_connection_type(GTlsBackend *backend) {
  return G_TLS_BACKEND_GET_INTERFACE(backend)->get_server_connection_type();
}