#ifndef __G_UNIX_RESOLVER_H__
#define __G_UNIX_RESOLVER_H__

#include "libasyncns/asyncns.h"
#include "gthreadedresolver.h"

G_BEGIN_DECLS
#define G_TYPE_UNIX_RESOLVER   (g_unix_resolver_get_type())
#define G_UNIX_RESOLVER(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_UNIX_RESOLVER, GUnixResolver))
#define G_UNIX_RESOLVER_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_UNIX_RESOLVER, GUnixResolverClass))
#define G_IS_UNIX_RESOLVER(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_UNIX_RESOLVER))
#define G_IS_UNIX_RESOLVER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_UNIX_RESOLVER))
#define G_UNIX_RESOLVER_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_UNIX_RESOLVER, GUnixResolverClass))
typedef struct {
  GThreadedResolver parent_instance;
  _g_asyncns_t *asyncns;
  guint watch;
} GUnixResolver;
typedef struct {
  GThreadedResolverClass parent_class;
} GUnixResolverClass;
GType g_unix_resolver_get_type(void) G_GNUC_CONST;
G_END_DECLS

#endif