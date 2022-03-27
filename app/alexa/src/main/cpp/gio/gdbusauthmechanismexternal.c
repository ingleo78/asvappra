#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "../glib/glibintl.h"
#include "config.h"
#include "gdbusauthmechanismexternal.h"
#include "gcredentials.h"
#include "gdbuserror.h"
#include "gioenumtypes.h"

struct _GDBusAuthMechanismExternalPrivate {
  gboolean is_client;
  gboolean is_server;
  GDBusAuthMechanismState state;
};
static gint mechanism_get_priority(void);
static const gchar *mechanism_get_name(void);
static gboolean mechanism_is_supported(GDBusAuthMechanism *mechanism);
static gchar *mechanism_encode_data(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len, gsize *out_data_len);
static gchar *mechanism_decode_data(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len, gsize *out_data_len);
static GDBusAuthMechanismState mechanism_server_get_state(GDBusAuthMechanism *mechanism);
static void mechanism_server_initiate(GDBusAuthMechanism *mechanism, const gchar *initial_response, gsize initial_response_len);
static void mechanism_server_data_receive(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len);
static gchar *mechanism_server_data_send(GDBusAuthMechanism *mechanism, gsize *out_data_len);
static gchar *mechanism_server_get_reject_reason(GDBusAuthMechanism *mechanism);
static void mechanism_server_shutdown(GDBusAuthMechanism *mechanism);
static GDBusAuthMechanismState mechanism_client_get_state(GDBusAuthMechanism *mechanism);
static gchar *mechanism_client_initiate(GDBusAuthMechanism *mechanism, gsize *out_initial_response_len);
static void mechanism_client_data_receive(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len);
static gchar *mechanism_client_data_send(GDBusAuthMechanism *mechanism, gsize *out_data_len);
static void mechanism_client_shutdown(GDBusAuthMechanism *mechanism);
G_DEFINE_TYPE(GDBusAuthMechanismExternal, _g_dbus_auth_mechanism_external, G_TYPE_DBUS_AUTH_MECHANISM);
static void _g_dbus_auth_mechanism_external_finalize(GObject *object) {
  if (G_OBJECT_CLASS(_g_dbus_auth_mechanism_external_parent_class)->finalize != NULL) G_OBJECT_CLASS(_g_dbus_auth_mechanism_external_parent_class)->finalize(object);
}
static void _g_dbus_auth_mechanism_external_class_init(GDBusAuthMechanismExternalClass *klass) {
  GObjectClass *gobject_class;
  GDBusAuthMechanismClass *mechanism_class;
  g_type_class_add_private(klass, sizeof(GDBusAuthMechanismExternalPrivate));
  gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = _g_dbus_auth_mechanism_external_finalize;
  mechanism_class = G_DBUS_AUTH_MECHANISM_CLASS(klass);
  mechanism_class->get_name = mechanism_get_name;
  mechanism_class->get_priority = mechanism_get_priority;
  mechanism_class->is_supported = mechanism_is_supported;
  mechanism_class->encode_data = mechanism_encode_data;
  mechanism_class->decode_data = mechanism_decode_data;
  mechanism_class->server_get_state = mechanism_server_get_state;
  mechanism_class->server_initiate = mechanism_server_initiate;
  mechanism_class->server_data_receive = mechanism_server_data_receive;
  mechanism_class->server_data_send = mechanism_server_data_send;
  mechanism_class->server_get_reject_reason = mechanism_server_get_reject_reason;
  mechanism_class->server_shutdown = mechanism_server_shutdown;
  mechanism_class->client_get_state = mechanism_client_get_state;
  mechanism_class->client_initiate = mechanism_client_initiate;
  mechanism_class->client_data_receive = mechanism_client_data_receive;
  mechanism_class->client_data_send = mechanism_client_data_send;
  mechanism_class->client_shutdown = mechanism_client_shutdown;
}
static void _g_dbus_auth_mechanism_external_init(GDBusAuthMechanismExternal *mechanism) {
  mechanism->priv = G_TYPE_INSTANCE_GET_PRIVATE(mechanism, G_TYPE_DBUS_AUTH_MECHANISM_EXTERNAL, GDBusAuthMechanismExternalPrivate);
}
static gboolean mechanism_is_supported(GDBusAuthMechanism *mechanism) {
  g_return_val_if_fail(G_IS_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism), FALSE);
  if (_g_dbus_auth_mechanism_get_credentials(mechanism) != NULL) return TRUE;
  else return FALSE;
}
static gint mechanism_get_priority(void) {
  return 100;
}
static const gchar *mechanism_get_name(void) {
  return "EXTERNAL";
}
static gchar *mechanism_encode_data(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len, gsize *out_data_len) {
  return NULL;
}
static gchar *mechanism_decode_data(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len, gsize *out_data_len) {
  return NULL;
}
static GDBusAuthMechanismState mechanism_server_get_state(GDBusAuthMechanism *mechanism) {
  GDBusAuthMechanismExternal *m = G_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism);
  g_return_val_if_fail(G_IS_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism), G_DBUS_AUTH_MECHANISM_STATE_INVALID);
  g_return_val_if_fail(m->priv->is_server && !m->priv->is_client, G_DBUS_AUTH_MECHANISM_STATE_INVALID);
  return m->priv->state;
}
static gboolean data_matches_credentials(const gchar *data, GCredentials *credentials) {
  gboolean match;
  match = FALSE;
  if (credentials == NULL) return match;
  if (data == NULL || strlen(data) == 0) return match;
  gint64 alleged_uid;
  gchar *endp;
  alleged_uid = g_ascii_strtoll(data, &endp, 10);
  if (*endp == '\0') {
      if (g_credentials_get_unix_user(credentials, NULL) == alleged_uid) match = TRUE;
  }
  return match;
}
static void mechanism_server_initiate(GDBusAuthMechanism *mechanism, const gchar *initial_response, gsize initial_response_len) {
  GDBusAuthMechanismExternal *m = G_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism);
  g_return_if_fail(G_IS_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism));
  g_return_if_fail(!m->priv->is_server && !m->priv->is_client);
  m->priv->is_server = TRUE;
  if (initial_response != NULL) {
      if (data_matches_credentials(initial_response, _g_dbus_auth_mechanism_get_credentials(mechanism))) m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_ACCEPTED;
      else m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_REJECTED;
  } else m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_WAITING_FOR_DATA;
}
static void mechanism_server_data_receive(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len) {
  GDBusAuthMechanismExternal *m = G_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism);
  g_return_if_fail(G_IS_DBUS_AUTH_MECHANISM_EXTERNAL (mechanism));
  g_return_if_fail(m->priv->is_server && !m->priv->is_client);
  g_return_if_fail(m->priv->state == G_DBUS_AUTH_MECHANISM_STATE_WAITING_FOR_DATA);
  if (data_matches_credentials(data, _g_dbus_auth_mechanism_get_credentials(mechanism))) m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_ACCEPTED;
  else m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_REJECTED;
}
static gchar *mechanism_server_data_send(GDBusAuthMechanism *mechanism, gsize *out_data_len) {
  GDBusAuthMechanismExternal *m = G_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism);
  g_return_val_if_fail(G_IS_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism), NULL);
  g_return_val_if_fail(m->priv->is_server && !m->priv->is_client, NULL);
  g_return_val_if_fail(m->priv->state == G_DBUS_AUTH_MECHANISM_STATE_HAVE_DATA_TO_SEND, NULL);
  g_assert_not_reached();
  return NULL;
}
static gchar *mechanism_server_get_reject_reason(GDBusAuthMechanism *mechanism) {
  GDBusAuthMechanismExternal *m = G_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism);
  g_return_val_if_fail(G_IS_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism), NULL);
  g_return_val_if_fail(m->priv->is_server && !m->priv->is_client, NULL);
  g_return_val_if_fail(m->priv->state == G_DBUS_AUTH_MECHANISM_STATE_REJECTED, NULL);
  g_assert_not_reached();
  return NULL;
}
static void mechanism_server_shutdown(GDBusAuthMechanism *mechanism) {
  GDBusAuthMechanismExternal *m = G_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism);
  g_return_if_fail(G_IS_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism));
  g_return_if_fail(m->priv->is_server && !m->priv->is_client);
  m->priv->is_server = FALSE;
}
static GDBusAuthMechanismState mechanism_client_get_state(GDBusAuthMechanism *mechanism) {
  GDBusAuthMechanismExternal *m = G_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism);
  g_return_val_if_fail(G_IS_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism), G_DBUS_AUTH_MECHANISM_STATE_INVALID);
  g_return_val_if_fail(m->priv->is_client && !m->priv->is_server, G_DBUS_AUTH_MECHANISM_STATE_INVALID);
  return m->priv->state;
}
static gchar *mechanism_client_initiate(GDBusAuthMechanism *mechanism, gsize *out_initial_response_len) {
  GDBusAuthMechanismExternal *m = G_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism);
  gchar *initial_response;
  GCredentials *credentials;
  g_return_val_if_fail(G_IS_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism), NULL);
  g_return_val_if_fail(!m->priv->is_server && !m->priv->is_client, NULL);
  m->priv->is_client = TRUE;
  m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_ACCEPTED;
  *out_initial_response_len = -1;
  credentials = _g_dbus_auth_mechanism_get_credentials(mechanism);
  g_assert(credentials != NULL);
#if !defined(G_OS_UNIX)
  initial_response = g_strdup_printf("%" G_GINT64_FORMAT, (gint64)g_credentials_get_unix_user(credentials, NULL));
#elif defined(G_OS_WIN32)
#ifdef __GNUC__
#warning Dont know how to send credentials on this OS. The EXTERNAL D-Bus authentication mechanism will not work.
#endif
  m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_REJECTED;
#endif
  return initial_response;
}
static void mechanism_client_data_receive(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len) {
  GDBusAuthMechanismExternal *m = G_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism);
  g_return_if_fail(G_IS_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism));
  g_return_if_fail(m->priv->is_client && !m->priv->is_server);
  g_return_if_fail(m->priv->state == G_DBUS_AUTH_MECHANISM_STATE_WAITING_FOR_DATA);
  g_assert_not_reached();
}
static gchar *mechanism_client_data_send(GDBusAuthMechanism *mechanism, gsize *out_data_len) {
  GDBusAuthMechanismExternal *m = G_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism);
  g_return_val_if_fail(G_IS_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism), NULL);
  g_return_val_if_fail(m->priv->is_client && !m->priv->is_server, NULL);
  g_return_val_if_fail(m->priv->state == G_DBUS_AUTH_MECHANISM_STATE_HAVE_DATA_TO_SEND, NULL);
  g_assert_not_reached();
  return NULL;
}
static void mechanism_client_shutdown(GDBusAuthMechanism *mechanism) {
  GDBusAuthMechanismExternal *m = G_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism);
  g_return_if_fail(G_IS_DBUS_AUTH_MECHANISM_EXTERNAL(mechanism));
  g_return_if_fail(m->priv->is_client && !m->priv->is_server);
  m->priv->is_client = FALSE;
}