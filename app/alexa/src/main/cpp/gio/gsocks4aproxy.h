#ifndef __G_SOCKS4A_PROXY_H__
#define __G_SOCKS4A_PROXY_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_SOCKS4A_PROXY  (_g_socks4a_proxy_get_type())
#define G_SOCKS4A_PROXY(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_SOCKS4A_PROXY, GSocks4aProxy))
#define G_SOCKS4A_PROXY_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_SOCKS4A_PROXY, GSocks4aProxyClass))
#define G_IS_SOCKS4A_PROXY(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_SOCKS4A_PROXY))
#define G_IS_SOCKS4A_PROXY_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_SOCKS4A_PROXY))
#define G_SOCKS4A_PROXY_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_SOCKS4A_PROXY, GSocks4aProxyClass))
typedef struct _GSocks4aProxy GSocks4aProxy;
typedef struct _GSocks4aProxyClass GSocks4aProxyClass;
struct _GSocks4aProxy {
  GObject parent;
  gboolean supports_hostname;
};
struct _GSocks4aProxyClass {
  GObjectClass parent_class;
};
GType _g_socks4a_proxy_get_type(void);
G_END_DECLS

#endif