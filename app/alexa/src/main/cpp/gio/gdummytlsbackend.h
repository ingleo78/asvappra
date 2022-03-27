#ifndef __G_DUMMY_TLS_BACKEND_H__
#define __G_DUMMY_TLS_BACKEND_H__

#include "../glib/glib.h"
#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_DUMMY_TLS_BACKEND  (_g_dummy_tls_backend_get_type())
#define G_DUMMY_TLS_BACKEND(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_DUMMY_TLS_BACKEND, GDummyTlsBackend))
#define G_DUMMY_TLS_BACKEND_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_DUMMY_TLS_BACKEND, GDummyTlsBackendClass))
#define G_IS_DUMMY_TLS_BACKEND(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_DUMMY_TLS_BACKEND))
#define G_IS_DUMMY_TLS_BACKEND_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_DUMMY_TLS_BACKEND))
#define G_DUMMY_TLS_BACKEND_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_DUMMY_TLS_BACKEND, GDummyTlsBackendClass))
typedef struct _GDummyTlsBackend GDummyTlsBackend;
typedef struct _GDummyTlsBackendClass GDummyTlsBackendClass;
struct _GDummyTlsBackendClass {
  GObjectClass parent_class;
};
GType _g_dummy_tls_backend_get_type(void);
G_END_DECLS

#endif