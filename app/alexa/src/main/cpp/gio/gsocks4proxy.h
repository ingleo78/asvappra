#ifndef __G_SOCKS4_PROXY_H__
#define __G_SOCKS4_PROXY_H__

#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_SOCKS4_PROXY  (_g_socks4_proxy_get_type())
#define G_SOCKS4_PROXY(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_SOCKS4_PROXY, GSocks4Proxy))
#define G_SOCKS4_PROXY_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_SOCKS4_PROXY, GSocks4ProxyClass))
#define G_IS_SOCKS4_PROXY(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_SOCKS4_PROXY))
#define G_IS_SOCKS4_PROXY_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_SOCKS4_PROXY))
#define G_SOCKS4_PROXY_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_SOCKS4_PROXY, GSocks4ProxyClass))
typedef struct _GSocks4Proxy GSocks4Proxy;
typedef struct _GSocks4ProxyClass GSocks4ProxyClass;
GType _g_socks4_proxy_get_type(void);
G_END_DECLS

#endif