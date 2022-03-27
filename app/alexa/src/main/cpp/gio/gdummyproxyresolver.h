#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_DUMMY_PROXY_RESOLVER_H__
#define __G_DUMMY_PROXY_RESOLVER_H__

#include "../glib/glib.h"
#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_DUMMY_PROXY_RESOLVER  (_g_dummy_proxy_resolver_get_type())
#define G_DUMMY_PROXY_RESOLVER(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_DUMMY_PROXY_RESOLVER, GDummyProxyResolver))
#define G_DUMMY_PROXY_RESOLVER_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_DUMMY_PROXY_RESOLVER, GDummyProxyResolverClass))
#define G_IS_DUMMY_PROXY_RESOLVER(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_DUMMY_PROXY_RESOLVER))
#define G_IS_DUMMY_PROXY_RESOLVER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_DUMMY_PROXY_RESOLVER))
#define G_DUMMY_PROXY_RESOLVER_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_DUMMY_PROXY_RESOLVER, GDummyProxyResolverClass))
typedef struct _GDummyProxyResolver GDummyProxyResolver;
typedef struct _GDummyProxyResolverClass GDummyProxyResolverClass;
struct _GDummyProxyResolverClass {
  GObjectClass parent_class;
};
GType _g_dummy_proxy_resolver_get_type(void);
G_END_DECLS

#endif