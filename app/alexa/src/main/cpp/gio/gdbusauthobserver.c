#include "../glib/glibintl.h"
#include "config.h"
#include "gdbusauthobserver.h"
#include "gcredentials.h"
#include "gioenumtypes.h"
#include "giostream.h"

typedef struct _GDBusAuthObserverClass GDBusAuthObserverClass;
struct _GDBusAuthObserverClass {
  GObjectClass parent_class;
  gboolean (*authorize_authenticated_peer)(GDBusAuthObserver *observer, GIOStream *stream, GCredentials *credentials);
};
struct _GDBusAuthObserver {
  GObject parent_instance;
};
enum {
  AUTHORIZE_AUTHENTICATED_PEER_SIGNAL,
  LAST_SIGNAL,
};
static guint signals[LAST_SIGNAL] = { 0 };
G_DEFINE_TYPE(GDBusAuthObserver, g_dbus_auth_observer, G_TYPE_OBJECT);
static void g_dbus_auth_observer_finalize(GObject *object) {
  G_OBJECT_CLASS(g_dbus_auth_observer_parent_class)->finalize (object);
}
static gboolean g_dbus_auth_observer_authorize_authenticated_peer_real(GDBusAuthObserver *observer, GIOStream *stream, GCredentials *credentials) {
  return TRUE;
}
gboolean _g_signal_accumulator_false_handled(GSignalInvocationHint *ihint, GValue *return_accu, const GValue *handler_return, gpointer dummy) {
  gboolean continue_emission;
  gboolean signal_handled;
  signal_handled = g_value_get_boolean(handler_return);
  g_value_set_boolean(return_accu, signal_handled);
  continue_emission = signal_handled;
  return continue_emission;
}
static void g_dbus_auth_observer_class_init(GDBusAuthObserverClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_dbus_auth_observer_finalize;
  klass->authorize_authenticated_peer = g_dbus_auth_observer_authorize_authenticated_peer_real;
  signals[AUTHORIZE_AUTHENTICATED_PEER_SIGNAL] = g_signal_new("authorize-authenticated-peer", G_TYPE_DBUS_AUTH_OBSERVER,
                                                              G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GDBusAuthObserverClass, authorize_authenticated_peer),
                                                              _g_signal_accumulator_false_handled, NULL, NULL,
                                                              G_TYPE_BOOLEAN,2, G_TYPE_IO_STREAM, G_TYPE_CREDENTIALS);
}
static void g_dbus_auth_observer_init(GDBusAuthObserver *observer) {}
GDBusAuthObserver *g_dbus_auth_observer_new(void) {
  return g_object_new(G_TYPE_DBUS_AUTH_OBSERVER, NULL);
}
gboolean g_dbus_auth_observer_authorize_authenticated_peer(GDBusAuthObserver *observer, GIOStream *stream, GCredentials *credentials) {
  gboolean denied;
  denied = FALSE;
  g_signal_emit(observer, signals[AUTHORIZE_AUTHENTICATED_PEER_SIGNAL], 0, stream, credentials, &denied);
  return denied;
}