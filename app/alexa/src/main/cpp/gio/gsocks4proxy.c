#include "config.h"
#include "gsocks4proxy.h"
#include "giomodule.h"
#include "giomodule-priv.h"
#include "gproxy.h"
#include "gsocks4aproxy.h"

struct _GSocks4Proxy {
  GSocks4aProxy parent;
};
struct _GSocks4ProxyClass {
  GSocks4aProxyClass parent_class;
};
#define g_socks4_proxy_get_type _g_socks4_proxy_get_type
G_DEFINE_TYPE_WITH_CODE(GSocks4Proxy, g_socks4_proxy, G_TYPE_SOCKS4A_PROXY,_g_io_modules_ensure_extension_points_registered ();
			            g_io_extension_point_implement(G_PROXY_EXTENSION_POINT_NAME, g_define_type_id, "socks4", 0))

static void g_socks4_proxy_finalize(GObject *object) {
  G_OBJECT_CLASS (g_socks4_proxy_parent_class)->finalize (object);
}
static void g_socks4_proxy_init(GSocks4Proxy *proxy) {
  G_SOCKS4A_PROXY(proxy)->supports_hostname = FALSE;
}
static void g_socks4_proxy_class_init(GSocks4ProxyClass *class) {
  GObjectClass *object_class;
  object_class = (GObjectClass *) class;
  object_class->finalize = g_socks4_proxy_finalize;
}