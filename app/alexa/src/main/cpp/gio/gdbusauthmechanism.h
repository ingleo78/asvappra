#if defined (GIO_COMPILATION)
#error "gdbusauthmechanism.h is a private header file."
#endif

#ifndef __G_DBUS_AUTH_MECHANISM_H__
#define __G_DBUS_AUTH_MECHANISM_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_DBUS_AUTH_MECHANISM  (_g_dbus_auth_mechanism_get_type())
#define G_DBUS_AUTH_MECHANISM(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_DBUS_AUTH_MECHANISM, GDBusAuthMechanism))
#define G_DBUS_AUTH_MECHANISM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_DBUS_AUTH_MECHANISM, GDBusAuthMechanismClass))
#define G_DBUS_AUTH_MECHANISM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_DBUS_AUTH_MECHANISM, GDBusAuthMechanismClass))
#define G_IS_DBUS_AUTH_MECHANISM(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_DBUS_AUTH_MECHANISM))
#define G_IS_DBUS_AUTH_MECHANISM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_DBUS_AUTH_MECHANISM))
typedef struct _GDBusAuthMechanism GDBusAuthMechanism;
typedef struct _GDBusAuthMechanismClass GDBusAuthMechanismClass;
typedef struct _GDBusAuthMechanismPrivate GDBusAuthMechanismPrivate;
typedef enum {
  G_DBUS_AUTH_MECHANISM_STATE_INVALID,
  G_DBUS_AUTH_MECHANISM_STATE_WAITING_FOR_DATA,
  G_DBUS_AUTH_MECHANISM_STATE_HAVE_DATA_TO_SEND,
  G_DBUS_AUTH_MECHANISM_STATE_REJECTED,
  G_DBUS_AUTH_MECHANISM_STATE_ACCEPTED,
} GDBusAuthMechanismState;
struct _GDBusAuthMechanismClass {
  GObjectClass parent_class;
  gint (*get_priority)(void);
  const gchar *(*get_name)(void);
  gboolean (*is_supported)(GDBusAuthMechanism *mechanism);
  gchar *(*encode_data)(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len, gsize *out_data_len);
  gchar *(*decode_data)(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len, gsize *out_data_len);
  GDBusAuthMechanismState (*server_get_state)(GDBusAuthMechanism *mechanism);
  void (*server_initiate)(GDBusAuthMechanism *mechanism, const gchar *initial_response, gsize initial_response_len);
  void (*server_data_receive)(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len);
  gchar *(*server_data_send)(GDBusAuthMechanism *mechanism, gsize *out_data_len);
  gchar *(*server_get_reject_reason)(GDBusAuthMechanism *mechanism);
  void (*server_shutdown)(GDBusAuthMechanism *mechanism);
  GDBusAuthMechanismState (*client_get_state)(GDBusAuthMechanism *mechanism);
  gchar *(*client_initiate)(GDBusAuthMechanism *mechanism, gsize *out_initial_response_len);
  void (*client_data_receive)(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len);
  gchar *(*client_data_send)(GDBusAuthMechanism *mechanism, gsize *out_data_len);
  void (*client_shutdown)(GDBusAuthMechanism *mechanism);
};
struct _GDBusAuthMechanism {
  GObject parent_instance;
  GDBusAuthMechanismPrivate *priv;
};
GType _g_dbus_auth_mechanism_get_type(void) G_GNUC_CONST;
gint _g_dbus_auth_mechanism_get_priority(GType mechanism_type);
const gchar *_g_dbus_auth_mechanism_get_name(GType mechanism_type);
GIOStream  *_g_dbus_auth_mechanism_get_stream(GDBusAuthMechanism *mechanism);
GCredentials *_g_dbus_auth_mechanism_get_credentials(GDBusAuthMechanism *mechanism);
gboolean _g_dbus_auth_mechanism_is_supported(GDBusAuthMechanism *mechanism);
gchar *_g_dbus_auth_mechanism_encode_data(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len, gsize *out_data_len);
gchar *_g_dbus_auth_mechanism_decode_data(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len, gsize *out_data_len);
GDBusAuthMechanismState _g_dbus_auth_mechanism_server_get_state(GDBusAuthMechanism *mechanism);
void _g_dbus_auth_mechanism_server_initiate(GDBusAuthMechanism *mechanism, const gchar *initial_response, gsize initial_response_len);
void _g_dbus_auth_mechanism_server_data_receive(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len);
gchar *_g_dbus_auth_mechanism_server_data_send(GDBusAuthMechanism *mechanism, gsize *out_data_len);
gchar *_g_dbus_auth_mechanism_server_get_reject_reason(GDBusAuthMechanism *mechanism);
void _g_dbus_auth_mechanism_server_shutdown(GDBusAuthMechanism *mechanism);
GDBusAuthMechanismState _g_dbus_auth_mechanism_client_get_state(GDBusAuthMechanism *mechanism);
gchar *_g_dbus_auth_mechanism_client_initiate(GDBusAuthMechanism *mechanism, gsize *out_initial_response_len);
void _g_dbus_auth_mechanism_client_data_receive(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len);
gchar *_g_dbus_auth_mechanism_client_data_send(GDBusAuthMechanism *mechanism, gsize *out_data_len);
void _g_dbus_auth_mechanism_client_shutdown(GDBusAuthMechanism *mechanism);
G_END_DECLS

#endif