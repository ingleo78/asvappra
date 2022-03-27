#include "config.h"
#include "gsocketservice.h"
#include "gio.h"
#include "gsocketlistener.h"
#include "gsocketconnection.h"

static guint g_socket_service_incoming_signal;
G_DEFINE_TYPE (GSocketService, g_socket_service, G_TYPE_SOCKET_LISTENER);
G_LOCK_DEFINE_STATIC(active);
struct _GSocketServicePrivate {
  GCancellable *cancellable;
  guint active : 1;
  guint outstanding_accept : 1;
};
static void g_socket_service_ready(GObject *object, GAsyncResult *result, gpointer user_data);
static gboolean
g_socket_service_real_incoming(GSocketService *service, GSocketConnection *connection, GObject *source_object) {
  return FALSE;
}
static void g_socket_service_init(GSocketService *service) {
  service->priv = G_TYPE_INSTANCE_GET_PRIVATE(service, G_TYPE_SOCKET_SERVICE, GSocketServicePrivate);
  service->priv->cancellable = g_cancellable_new();
  service->priv->active = TRUE;
}
static void g_socket_service_finalize(GObject *object) {
  GSocketService *service = G_SOCKET_SERVICE(object);
  g_object_unref(service->priv->cancellable);
  G_OBJECT_CLASS(g_socket_service_parent_class)->finalize(object);
}
static void do_accept(GSocketService *service) {
  g_socket_listener_accept_async(G_SOCKET_LISTENER(service), service->priv->cancellable, g_socket_service_ready, NULL);
  service->priv->outstanding_accept = TRUE;
}
static void g_socket_service_changed(GSocketListener *listener) {
  GSocketService  *service = G_SOCKET_SERVICE(listener);
  G_LOCK(active);
  if (service->priv->active) {
      if (service->priv->outstanding_accept) g_cancellable_cancel(service->priv->cancellable);
      else {
	  g_socket_listener_accept_async(listener, service->priv->cancellable, g_socket_service_ready, NULL);
	  service->priv->outstanding_accept = TRUE;
	  }
  }
  G_UNLOCK(active);
}
gboolean g_socket_service_is_active(GSocketService *service) {
  gboolean active;
  G_LOCK(active);
  active = service->priv->active;
  G_UNLOCK(active);
  return active;
}
void g_socket_service_start(GSocketService *service) {
  G_LOCK(active);
  if (!service->priv->active) {
      service->priv->active = TRUE;
      if (service->priv->outstanding_accept) g_cancellable_cancel(service->priv->cancellable);
      else do_accept(service);
  }
  G_UNLOCK(active);
}
void g_socket_service_stop(GSocketService *service) {
  G_LOCK(active);
  if (service->priv->active) {
      service->priv->active = FALSE;
      if (service->priv->outstanding_accept) g_cancellable_cancel(service->priv->cancellable);
  }
  G_UNLOCK(active);
}
static gboolean g_socket_service_incoming(GSocketService *service, GSocketConnection *connection, GObject *source_object) {
  gboolean result;
  g_signal_emit(service, g_socket_service_incoming_signal,0, connection, source_object, &result);
  return result;
}
static void g_socket_service_class_init(GSocketServiceClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GSocketListenerClass *listener_class = G_SOCKET_LISTENER_CLASS(class);
  g_type_class_add_private(class, sizeof(GSocketServicePrivate));
  gobject_class->finalize = g_socket_service_finalize;
  listener_class->changed = g_socket_service_changed;
  class->incoming = g_socket_service_real_incoming;
  g_socket_service_incoming_signal = g_signal_new("incoming", G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GSocketServiceClass, incoming),
                                                  g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 2,
                                                  G_TYPE_SOCKET_CONNECTION, G_TYPE_OBJECT);
}
static void g_socket_service_ready(GObject *object, GAsyncResult *result, gpointer user_data) {
  GSocketListener *listener = G_SOCKET_LISTENER(object);
  GSocketService *service = G_SOCKET_SERVICE(object);
  GSocketConnection *connection;
  GObject *source_object;
  GError *error = NULL;
  connection = g_socket_listener_accept_finish(listener, result, &source_object, &error);
  if (error) {
      if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) g_warning("fail: %s", error->message);
      g_error_free(error);
  } else {
      g_socket_service_incoming(service, connection, source_object);
      g_object_unref(connection);
  }
  G_LOCK (active);
  g_cancellable_reset (service->priv->cancellable);
  service->priv->outstanding_accept = FALSE;
  if (service->priv->active) do_accept(service);
  G_UNLOCK (active);
}
GSocketService *g_socket_service_new(void) {
  return g_object_new(G_TYPE_SOCKET_SERVICE, NULL);
}