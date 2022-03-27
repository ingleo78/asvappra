#ifndef __G_THREADED_RESOLVER_H__
#define __G_THREADED_RESOLVER_H__

#include "gresolver.h"

G_BEGIN_DECLS
#define G_TYPE_THREADED_RESOLVER  (g_threaded_resolver_get_type())
#define G_THREADED_RESOLVER(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_THREADED_RESOLVER, GThreadedResolver))
#define G_THREADED_RESOLVER_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_THREADED_RESOLVER, GThreadedResolverClass))
#define G_IS_THREADED_RESOLVER(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_THREADED_RESOLVER))
#define G_IS_THREADED_RESOLVER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_THREADED_RESOLVER))
#define G_THREADED_RESOLVER_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_THREADED_RESOLVER, GThreadedResolverClass))
typedef struct {
  GResolver parent_instance;
  GThreadPool *thread_pool;
} GThreadedResolver;
typedef struct {
  GResolverClass parent_class;
} GThreadedResolverClass;
GType g_threaded_resolver_get_type(void) G_GNUC_CONST;
G_END_DECLS

#endif