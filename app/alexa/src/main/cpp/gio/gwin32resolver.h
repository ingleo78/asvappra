#ifndef __G_WIN32_RESOLVER_H__
#define __G_WIN32_RESOLVER_H__

#include "gthreadedresolver.h"

G_BEGIN_DECLS
#define G_TYPE_WIN32_RESOLVER  (g_win32_resolver_get_type())
#define G_WIN32_RESOLVER(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_WIN32_RESOLVER, GWin32Resolver))
#define G_WIN32_RESOLVER_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_WIN32_RESOLVER, GWin32ResolverClass))
#define G_IS_WIN32_RESOLVER(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_WIN32_RESOLVER))
#define G_IS_WIN32_RESOLVER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_WIN32_RESOLVER))
#define G_WIN32_RESOLVER_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_WIN32_RESOLVER, GWin32ResolverClass))
typedef struct {
  GThreadedResolver parent_instance;
} GWin32Resolver;
typedef struct {
  GThreadedResolverClass parent_class;
} GWin32ResolverClass;
GType g_win32_resolver_get_type(void) G_GNUC_CONST;
G_END_DECLS

#endif