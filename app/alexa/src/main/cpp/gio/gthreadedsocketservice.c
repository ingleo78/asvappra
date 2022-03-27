#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "config.h"
#include "gsocketconnection.h"
#include "gthreadedsocketservice.h"

static guint g_threaded_socket_service_run_signal;
G_DEFINE_TYPE(GThreadedSocketService, g_threaded_socket_service, G_TYPE_SOCKET_SERVICE);
enum {
  PROP_0,
  PROP_MAX_THREADS
};
G_LOCK_DEFINE_STATIC(job_count);
struct _GThreadedSocketServicePrivate {
  GThreadPool *thread_pool;
  int max_threads;
  gint job_count;
};
typedef struct {
  GThreadedSocketService *service;
  GSocketConnection *connection;
  GObject *source_object;
} GThreadedSocketServiceData;
static void g_threaded_socket_service_func(gpointer _data, gpointer user_data) {
  GThreadedSocketService *threaded = user_data;
  GThreadedSocketServiceData *data = _data;
  gboolean result;
  g_signal_emit(data->service, g_threaded_socket_service_run_signal,0, data->connection, data->source_object, &result);
  g_object_unref(data->service);
  g_object_unref(data->connection);
  if (data->source_object) g_object_unref(data->source_object);
  g_slice_free(GThreadedSocketServiceData, data);
  G_LOCK(job_count);
  if (threaded->priv->job_count-- == threaded->priv->max_threads) g_socket_service_start(G_SOCKET_SERVICE(threaded));
  G_UNLOCK(job_count);
}
static gboolean g_threaded_socket_service_incoming(GSocketService *service, GSocketConnection *connection, GObject *source_object) {
  GThreadedSocketService *threaded;
  GThreadedSocketServiceData *data;
  threaded = G_THREADED_SOCKET_SERVICE(service);
  data = g_slice_new(GThreadedSocketServiceData);
  data->service = g_object_ref(service);
  data->connection = g_object_ref(connection);
  if (source_object) data->source_object = g_object_ref(source_object);
  else data->source_object = NULL;
  G_LOCK(job_count);
  if (++threaded->priv->job_count == threaded->priv->max_threads) g_socket_service_stop(service);
  G_UNLOCK(job_count);
  g_thread_pool_push(threaded->priv->thread_pool, data, NULL);
  return FALSE;
}
static void g_threaded_socket_service_init(GThreadedSocketService *service) {
  service->priv = G_TYPE_INSTANCE_GET_PRIVATE(service, G_TYPE_THREADED_SOCKET_SERVICE, GThreadedSocketServicePrivate);
  service->priv->max_threads = 10;
}
static void g_threaded_socket_service_constructed(GObject *object) {
  GThreadedSocketService *service = G_THREADED_SOCKET_SERVICE(object);
  service->priv->thread_pool = g_thread_pool_new(g_threaded_socket_service_func, service, service->priv->max_threads,FALSE,NULL);
}
static void g_threaded_socket_service_finalize(GObject *object) {
  GThreadedSocketService *service = G_THREADED_SOCKET_SERVICE(object);
  g_thread_pool_free(service->priv->thread_pool, FALSE, TRUE);
  G_OBJECT_CLASS(g_threaded_socket_service_parent_class)->finalize(object);
}
static void g_threaded_socket_service_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GThreadedSocketService *service = G_THREADED_SOCKET_SERVICE(object);
  switch(prop_id) {
      case PROP_MAX_THREADS: g_value_set_int(value, service->priv->max_threads); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_threaded_socket_service_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GThreadedSocketService *service = G_THREADED_SOCKET_SERVICE(object);
  switch(prop_id) {
      case PROP_MAX_THREADS: service->priv->max_threads = g_value_get_int(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_threaded_socket_service_class_init(GThreadedSocketServiceClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GSocketServiceClass *ss_class = &class->parent_class;
  g_type_class_add_private(class, sizeof(GThreadedSocketServicePrivate));
  gobject_class->constructed = g_threaded_socket_service_constructed;
  gobject_class->finalize = g_threaded_socket_service_finalize;
  gobject_class->set_property = g_threaded_socket_service_set_property;
  gobject_class->get_property = g_threaded_socket_service_get_property;
  ss_class->incoming = g_threaded_socket_service_incoming;
  g_threaded_socket_service_run_signal = g_signal_new("run", G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GThreadedSocketServiceClass, run),
                                                      g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 2,
                                                      G_TYPE_SOCKET_CONNECTION, G_TYPE_OBJECT);
  g_object_class_install_property(gobject_class, PROP_MAX_THREADS,g_param_spec_int("max-threads", P_("Max threads"),
						          P_("The max number of threads handling clients for this service"),-1, G_MAXINT,10,
						    G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
GSocketService * g_threaded_socket_service_new(int max_threads) {
  return g_object_new (G_TYPE_THREADED_SOCKET_SERVICE,"max-threads", max_threads, NULL);
}