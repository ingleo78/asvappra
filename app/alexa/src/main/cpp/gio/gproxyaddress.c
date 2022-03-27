#include <string.h>
#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "../glib/glib-object.h"
#include "config.h"
#include "gsocketaddress.h"
#include "gproxyaddress.h"

G_DEFINE_TYPE(GProxyAddress, g_proxy_address, G_TYPE_INET_SOCKET_ADDRESS);
enum {
  PROP_0,
  PROP_PROTOCOL,
  PROP_DESTINATION_HOSTNAME,
  PROP_DESTINATION_PORT,
  PROP_USERNAME,
  PROP_PASSWORD
};
struct _GProxyAddressPrivate {
  gchar *protocol;
  gchar	*username;
  gchar *password;
  gchar *dest_hostname;
  guint16 dest_port;
};
static void g_proxy_address_finalize(GObject *object) {
  GProxyAddress *proxy = G_PROXY_ADDRESS(object);
  g_free(proxy->priv->protocol);
  g_free(proxy->priv->username);
  g_free(proxy->priv->password);
  g_free(proxy->priv->dest_hostname);
  G_OBJECT_CLASS(g_proxy_address_parent_class)->finalize(object);
}
static void g_proxy_address_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GProxyAddress *proxy = G_PROXY_ADDRESS(object);
  switch(prop_id) {
      case PROP_PROTOCOL:
		  g_free(proxy->priv->protocol);
		  proxy->priv->protocol = g_value_dup_string(value);
		  break;
      case PROP_DESTINATION_HOSTNAME:
		  g_free(proxy->priv->dest_hostname);
		  proxy->priv->dest_hostname = g_value_dup_string(value);
		  break;
      case PROP_DESTINATION_PORT: proxy->priv->dest_port = g_value_get_uint(value); break;
      case PROP_USERNAME:
		  g_free(proxy->priv->username);
		  proxy->priv->username = g_value_dup_string(value);
		  break;
      case PROP_PASSWORD:
		  g_free(proxy->priv->password);
		  proxy->priv->password = g_value_dup_string(value);
		  break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_proxy_address_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GProxyAddress *proxy = G_PROXY_ADDRESS(object);
  switch(prop_id) {
      case PROP_PROTOCOL: g_value_set_string(value, proxy->priv->protocol); break;
      case PROP_DESTINATION_HOSTNAME: g_value_set_string(value, proxy->priv->dest_hostname); break;
      case PROP_DESTINATION_PORT: g_value_set_uint(value, proxy->priv->dest_port); break;
      case PROP_USERNAME: g_value_set_string(value, proxy->priv->username); break;
      case PROP_PASSWORD: g_value_set_string(value, proxy->priv->password); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_proxy_address_class_init(GProxyAddressClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GProxyAddressPrivate));
  gobject_class->finalize = g_proxy_address_finalize;
  gobject_class->set_property = g_proxy_address_set_property;
  gobject_class->get_property = g_proxy_address_get_property;
  g_object_class_install_property(gobject_class,PROP_PROTOCOL,g_param_spec_string("protocol", P_("Protocol"), P_("The proxy protocol"),
						          NULL,G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(gobject_class,PROP_USERNAME,g_param_spec_string("username", P_("Username"), P_("The proxy username"),
						          NULL,G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class,PROP_PASSWORD,g_param_spec_string("password", P_("Password"), P_("The proxy password"),
						          NULL,G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class,PROP_DESTINATION_HOSTNAME,g_param_spec_string("destination-hostname", P_("Destination Hostname"),
						          P_("The proxy destination hostname"),NULL,G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
						          G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class,PROP_DESTINATION_PORT,g_param_spec_uint("destination-port", P_("Destination Port"),
						          P_("The proxy destination port"),0,65535,0,G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
						      	  G_PARAM_STATIC_STRINGS));
}
static void g_proxy_address_init(GProxyAddress *proxy) {
  proxy->priv = G_TYPE_INSTANCE_GET_PRIVATE(proxy, G_TYPE_PROXY_ADDRESS, GProxyAddressPrivate);
  proxy->priv->protocol = NULL;
  proxy->priv->username = NULL;
  proxy->priv->password = NULL;
  proxy->priv->dest_hostname = NULL;
  proxy->priv->dest_port = 0;
}
GSocketAddress *g_proxy_address_new(GInetAddress *inetaddr, guint16 port, const gchar *protocol, const gchar *dest_hostname, guint16 dest_port, const gchar *username,
		     						const gchar *password) {
  return g_object_new(G_TYPE_PROXY_ADDRESS,"address", inetaddr, "port", port, "protocol", protocol, "destination-hostname", dest_hostname,
		       		  "destination-port", dest_port, "username", username, "password", password, NULL);
}
const gchar *g_proxy_address_get_protocol(GProxyAddress *proxy) {
  return proxy->priv->protocol;
}
const gchar *g_proxy_address_get_destination_hostname(GProxyAddress *proxy) {
  return proxy->priv->dest_hostname;
}
guint16 g_proxy_address_get_destination_port(GProxyAddress *proxy) {
  return proxy->priv->dest_port;
}
const gchar *g_proxy_address_get_username(GProxyAddress *proxy) {
  return proxy->priv->username;
}
const gchar *g_proxy_address_get_password(GProxyAddress *proxy) {
  return proxy->priv->password;
}