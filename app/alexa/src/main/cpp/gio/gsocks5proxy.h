#ifndef __G_SOCKS5_PROXY_H__
#define __G_SOCKS5_PROXY_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_SOCKS5_PROXY  (_g_socks5_proxy_get_type())
#define G_SOCKS5_PROXY(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_SOCKS5_PROXY, GSocks5Proxy))
#define G_SOCKS5_PROXY_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_SOCKS5_PROXY, GSocks5ProxyClass))
#define G_IS_SOCKS5_PROXY(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_SOCKS5_PROXY))
#define G_IS_SOCKS5_PROXY_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_SOCKS5_PROXY))
#define G_SOCKS5_PROXY_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_SOCKS5_PROXY, GSocks5ProxyClass))
typedef struct _GSocks5Proxy GSocks5Proxy;
typedef struct _GSocks5ProxyClass GSocks5ProxyClass;
GType _g_socks5_proxy_get_type(void);
G_END_DECLS

#endif