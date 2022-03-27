#include <stdio.h>
#include <string.h>
#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gthreadedresolver.h"
#include "gnetworkingprivate.h"
#include "gcancellable.h"
#include "gsimpleasyncresult.h"
#include "gsocketaddress.h"

G_DEFINE_TYPE(GThreadedResolver, g_threaded_resolver, G_TYPE_RESOLVER);
static void threaded_resolver_thread(gpointer thread_data, gpointer pool_data);
static void g_threaded_resolver_init(GThreadedResolver *gtr) {
  if (g_thread_supported()) gtr->thread_pool = g_thread_pool_new(threaded_resolver_thread, gtr, -1, FALSE, NULL);
}
static void finalize(GObject *object) {
  GThreadedResolver *gtr = G_THREADED_RESOLVER(object);
  g_thread_pool_free(gtr->thread_pool, FALSE, FALSE);
  G_OBJECT_CLASS(g_threaded_resolver_parent_class)->finalize(object);
}
typedef struct _GThreadedResolverRequest GThreadedResolverRequest;
typedef void (*GThreadedResolverResolveFunc)(GThreadedResolverRequest *, GError **);
typedef void (*GThreadedResolverFreeFunc)(GThreadedResolverRequest *);
struct _GThreadedResolverRequest {
  GThreadedResolverResolveFunc resolve_func;
  GThreadedResolverFreeFunc free_func;
  union {
      struct {
          gchar *hostname;
          GList *addresses;
      } name;
      struct {
          GInetAddress *address;
          gchar *name;
      } address;
      struct {
          gchar *rrname;
          GList *targets;
      } service;
  } u;
  GCancellable *cancellable;
  GError *error;
  GMutex *mutex;
  guint ref_count;
  GCond *cond;
  GSimpleAsyncResult *async_result;
  gboolean complete;
};
static void g_threaded_resolver_request_unref(GThreadedResolverRequest *req);
static void request_cancelled(GCancellable *cancellable, gpointer req);
static void request_cancelled_disconnect_notify(gpointer req, GClosure *closure);
static GThreadedResolverRequest *g_threaded_resolver_request_new(GThreadedResolverResolveFunc resolve_func, GThreadedResolverFreeFunc free_func,
				                                                 GCancellable *cancellable) {
  GThreadedResolverRequest *req;
  req = g_slice_new0(GThreadedResolverRequest);
  req->resolve_func = resolve_func;
  req->free_func = free_func;
  req->ref_count = 2;
  if (g_thread_supported()) req->mutex = g_mutex_new();
  g_mutex_lock(req->mutex);
  if (cancellable) {
      req->ref_count++;
      req->cancellable = g_object_ref(cancellable);
      g_signal_connect_data(cancellable, "cancelled", G_CALLBACK(request_cancelled), req, request_cancelled_disconnect_notify, 0);
  }
  return req;
}
static void g_threaded_resolver_request_unref(GThreadedResolverRequest *req) {
  guint ref_count;
  g_mutex_lock(req->mutex);
  ref_count = --req->ref_count;
  g_mutex_unlock(req->mutex);
  if (ref_count > 0) return;
  g_mutex_free(req->mutex);
  if (req->cond) g_cond_free(req->cond);
  if (req->error) g_error_free(req->error);
  if (req->free_func) req->free_func(req);
  g_slice_free(GThreadedResolverRequest, req);
}
static void g_threaded_resolver_request_complete(GThreadedResolverRequest *req, gboolean cancelled) {
  g_mutex_lock(req->mutex);
  if (req->complete) {
      g_mutex_unlock(req->mutex);
      return;
  }
  req->complete = TRUE;
  g_mutex_unlock(req->mutex);
  if (req->cancellable) {
      if (cancelled && !req->error) g_cancellable_set_error_if_cancelled(req->cancellable, &req->error);
      g_signal_handlers_disconnect_by_func(req->cancellable, request_cancelled, req);
      g_object_unref(req->cancellable);
      req->cancellable = NULL;
  }
  if (req->cond) { g_cond_signal(req->cond); }
  else if (req->async_result) {
      if (req->error) g_simple_async_result_set_from_error(req->async_result, req->error);
      g_simple_async_result_complete_in_idle(req->async_result);
      g_object_unref(req->async_result);
      req->async_result = NULL;
  }
}
static void request_cancelled(GCancellable *cancellable, gpointer user_data) {
  GThreadedResolverRequest *req = user_data;
  g_threaded_resolver_request_complete(req, TRUE);
}
static void request_cancelled_disconnect_notify(gpointer req, GClosure *closure) {
  g_threaded_resolver_request_unref(req);
}
static void threaded_resolver_thread(gpointer thread_data, gpointer pool_data) {
  GThreadedResolverRequest *req = thread_data;
  req->resolve_func(req, &req->error);
  g_threaded_resolver_request_complete(req, FALSE);
  g_threaded_resolver_request_unref(req);
}
static void resolve_sync(GThreadedResolver *gtr, GThreadedResolverRequest *req, GError **error) {
  if (!req->cancellable || !gtr->thread_pool) {
      req->resolve_func(req, error);
      g_mutex_unlock(req->mutex);
      g_threaded_resolver_request_complete(req, FALSE);
      g_threaded_resolver_request_unref(req);
      return;
  }
  req->cond = g_cond_new();
  g_thread_pool_push(gtr->thread_pool, req, NULL);
  g_cond_wait(req->cond, req->mutex);
  g_mutex_unlock(req->mutex);
  if (req->error) {
      g_propagate_error(error, req->error);
      req->error = NULL;
  }
}
static void resolve_async(GThreadedResolver *gtr, GThreadedResolverRequest *req, GAsyncReadyCallback callback, gpointer user_data, gpointer tag) {
  req->async_result = g_simple_async_result_new(G_OBJECT(gtr), callback, user_data, tag);
  g_simple_async_result_set_op_res_gpointer(req->async_result, req, (GDestroyNotify)g_threaded_resolver_request_unref);
  g_thread_pool_push(gtr->thread_pool, req, NULL);
  g_mutex_unlock(req->mutex);
}
static GThreadedResolverRequest *resolve_finish(GResolver *resolver, GAsyncResult *result, gpointer tag, GError **error) {
  g_return_val_if_fail(g_simple_async_result_is_valid(result, G_OBJECT(resolver), tag), NULL);
  return g_simple_async_result_get_op_res_gpointer(G_SIMPLE_ASYNC_RESULT(result));
}
static void do_lookup_by_name(GThreadedResolverRequest *req, GError **error) {
  struct addrinfo *res = NULL;
  gint retval;
  retval = getaddrinfo(req->u.name.hostname, NULL, &_g_resolver_addrinfo_hints, &res);
  req->u.name.addresses = _g_resolver_addresses_from_addrinfo(req->u.name.hostname, res, retval, error);
  if (res) freeaddrinfo(res);
}
static GList *lookup_by_name(GResolver *resolver, const gchar *hostname, GCancellable *cancellable, GError **error) {
  GThreadedResolver *gtr = G_THREADED_RESOLVER(resolver);
  GThreadedResolverRequest *req;
  GList *addresses;
  req = g_threaded_resolver_request_new(do_lookup_by_name, NULL, cancellable);
  req->u.name.hostname = (gchar*)hostname;
  resolve_sync(gtr, req, error);
  addresses = req->u.name.addresses;
  g_threaded_resolver_request_unref(req);
  return addresses;
}
static void free_lookup_by_name(GThreadedResolverRequest *req) {
  g_free (req->u.name.hostname);
  if (req->u.name.addresses) g_resolver_free_addresses(req->u.name.addresses);
}
static void lookup_by_name_async(GResolver *resolver, const gchar *hostname, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GThreadedResolver *gtr = G_THREADED_RESOLVER(resolver);
  GThreadedResolverRequest *req;
  req = g_threaded_resolver_request_new(do_lookup_by_name, free_lookup_by_name, cancellable);
  req->u.name.hostname = g_strdup(hostname);
  resolve_async(gtr, req, callback, user_data, lookup_by_name_async);
}
static GList *lookup_by_name_finish(GResolver *resolver, GAsyncResult *result, GError **error) {
  GThreadedResolverRequest *req;
  GList *addresses;
  req = resolve_finish(resolver, result, lookup_by_name_async, error);
  addresses = req->u.name.addresses;
  req->u.name.addresses = NULL;
  return addresses;
}
static void do_lookup_by_address(GThreadedResolverRequest *req, GError **error) {
  struct sockaddr_storage sockaddr;
  gsize sockaddr_size;
  gchar name[NI_MAXHOST];
  gint retval;
  _g_resolver_address_to_sockaddr(req->u.address.address, &sockaddr, &sockaddr_size);
  retval = getnameinfo((struct sockaddr*)&sockaddr, sockaddr_size, name, sizeof(name), NULL, 0, NI_NAMEREQD);
  req->u.address.name = _g_resolver_name_from_nameinfo(req->u.address.address, name, retval, error);
}
static gchar *lookup_by_address(GResolver *resolver, GInetAddress *address, GCancellable *cancellable, GError **error) {
  GThreadedResolver *gtr = G_THREADED_RESOLVER(resolver);
  GThreadedResolverRequest *req;
  gchar *name;
  req = g_threaded_resolver_request_new(do_lookup_by_address, NULL, cancellable);
  req->u.address.address = address;
  resolve_sync(gtr, req, error);
  name = req->u.address.name;
  g_threaded_resolver_request_unref(req);
  return name;
}
static void free_lookup_by_address(GThreadedResolverRequest *req) {
  g_object_unref(req->u.address.address);
  if (req->u.address.name) g_free(req->u.address.name);
}
static void lookup_by_address_async(GResolver *resolver, GInetAddress *address, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GThreadedResolver *gtr = G_THREADED_RESOLVER(resolver);
  GThreadedResolverRequest *req;
  req = g_threaded_resolver_request_new(do_lookup_by_address, free_lookup_by_address, cancellable);
  req->u.address.address = g_object_ref(address);
  resolve_async(gtr, req, callback, user_data, lookup_by_address_async);
}
static gchar *lookup_by_address_finish(GResolver *resolver, GAsyncResult *result, GError **error) {
  GThreadedResolverRequest *req;
  gchar *name;
  req = resolve_finish(resolver, result, lookup_by_address_async, error);
  name = req->u.address.name;
  req->u.address.name = NULL;
  return name;
}
static void do_lookup_service(GThreadedResolverRequest *req, GError **error) {
#if !defined(G_OS_UNIX)
  gint len, herr;
  guchar answer[1024];
#elif defined(G_OS_WIN32)
  DNS_STATUS status;
  DNS_RECORD *results;
#endif
#if !defined(G_OS_UNIX)
  len = res_query(req->u.service.rrname, C_IN, T_SRV, answer, sizeof(answer));
  herr = h_errno;
  req->u.service.targets = _g_resolver_targets_from_res_query(req->u.service.rrname, answer, len, herr, error);
#elif defined(G_OS_WIN32)
  status = DnsQuery_A(req->u.service.rrname, DNS_TYPE_SRV, DNS_QUERY_STANDARD, NULL, &results, NULL);
  req->u.service.targets = _g_resolver_targets_from_DnsQuery(req->u.service.rrname, status, results, error);
  DnsRecordListFree(results, DnsFreeRecordList);
#endif
}
static GList *lookup_service(GResolver *resolver, const gchar *rrname, GCancellable *cancellable, GError **error) {
  GThreadedResolver *gtr = G_THREADED_RESOLVER(resolver);
  GThreadedResolverRequest *req;
  GList *targets;
  req = g_threaded_resolver_request_new(do_lookup_service, NULL, cancellable);
  req->u.service.rrname = (char*)rrname;
  resolve_sync(gtr, req, error);
  targets = req->u.service.targets;
  g_threaded_resolver_request_unref(req);
  return targets;
}
static void free_lookup_service(GThreadedResolverRequest *req) {
  g_free (req->u.service.rrname);
  if (req->u.service.targets) g_resolver_free_targets(req->u.service.targets);
}
static void lookup_service_async(GResolver *resolver, const char *rrname, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GThreadedResolver *gtr = G_THREADED_RESOLVER(resolver);
  GThreadedResolverRequest *req;
  req = g_threaded_resolver_request_new(do_lookup_service, free_lookup_service, cancellable);
  req->u.service.rrname = g_strdup(rrname);
  resolve_async(gtr, req, callback, user_data, lookup_service_async);
}
static GList *lookup_service_finish(GResolver *resolver, GAsyncResult *result, GError **error) {
  GThreadedResolverRequest *req;
  GList *targets;
  req = resolve_finish(resolver, result, lookup_service_async, error);
  targets = req->u.service.targets;
  req->u.service.targets = NULL;
  return targets;
}
static void g_threaded_resolver_class_init(GThreadedResolverClass *threaded_class) {
  GResolverClass *resolver_class = G_RESOLVER_CLASS(threaded_class);
  GObjectClass *object_class = G_OBJECT_CLASS(threaded_class);
  resolver_class->lookup_by_name = lookup_by_name;
  resolver_class->lookup_by_name_async = lookup_by_name_async;
  resolver_class->lookup_by_name_finish = lookup_by_name_finish;
  resolver_class->lookup_by_address = lookup_by_address;
  resolver_class->lookup_by_address_async = lookup_by_address_async;
  resolver_class->lookup_by_address_finish = lookup_by_address_finish;
  resolver_class->lookup_service = lookup_service;
  resolver_class->lookup_service_async = lookup_service_async;
  resolver_class->lookup_service_finish = lookup_service_finish;
  object_class->finalize = finalize;
}