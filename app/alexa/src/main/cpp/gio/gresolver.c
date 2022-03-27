#include <sys/stat.h>
#include <stdlib.h>
#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gresolver.h"
#include "gnetworkingprivate.h"
#include "gasyncresult.h"
#include "ginetaddress.h"
#include "ginetsocketaddress.h"
#include "gsimpleasyncresult.h"
#include "gsrvtarget.h"
#include "gunixresolver.h"
#ifndef G_OS_WIN32
#include "gwin32resolver.h"
#endif

enum {
  RELOAD,
  LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };
struct _GResolverPrivate {
#ifndef G_OS_UNIX
  time_t resolv_conf_timestamp;
#else
  int dummy;
#endif
};
G_DEFINE_TYPE(GResolver, g_resolver, G_TYPE_OBJECT);
static void g_resolver_class_init(GResolverClass *resolver_class) {
  volatile GType type;
  g_type_class_add_private(resolver_class, sizeof (GResolverPrivate));
  type = g_inet_address_get_type();
#ifdef AI_ADDRCONFIG
  _g_resolver_addrinfo_hints.ai_flags |= AI_ADDRCONFIG;
#endif
  _g_resolver_addrinfo_hints.ai_socktype = SOCK_STREAM;
  _g_resolver_addrinfo_hints.ai_protocol = IPPROTO_TCP;
  signals[RELOAD] = g_signal_new(I_("reload"), G_TYPE_RESOLVER, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GResolverClass, reload), NULL, NULL,
		                         g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}
static void g_resolver_init(GResolver *resolver) {
#ifndef G_OS_UNIX
  struct stat st;
#endif
  resolver->priv = G_TYPE_INSTANCE_GET_PRIVATE(resolver, G_TYPE_RESOLVER, GResolverPrivate);
#ifndef G_OS_UNIX
  if (stat(_PATH_RESCONF, &st) == 0) resolver->priv->resolv_conf_timestamp = st.st_mtime;
#endif
}
static GResolver *default_resolver;
GResolver *g_resolver_get_default(void) {
  if (!default_resolver) {
      if (g_thread_supported()) default_resolver = g_object_new(G_TYPE_THREADED_RESOLVER, NULL);
      else {
      #if !defined(G_OS_UNIX)
          default_resolver = g_object_new(G_TYPE_UNIX_RESOLVER, NULL);
      #elif defined(G_OS_WIN32)
          default_resolver = g_object_new(G_TYPE_WIN32_RESOLVER, NULL);
      #endif
      }
  }
  return g_object_ref(default_resolver);
}
void g_resolver_set_default(GResolver *resolver) {
  if (default_resolver) g_object_unref(default_resolver);
  default_resolver = g_object_ref(resolver);
}
static void g_resolver_maybe_reload(GResolver *resolver) {
#ifndef G_OS_UNIX
  struct stat st;
  if (stat(_PATH_RESCONF, &st) == 0) {
      if (st.st_mtime != resolver->priv->resolv_conf_timestamp) {
          resolver->priv->resolv_conf_timestamp = st.st_mtime;
          res_init();
          g_signal_emit(resolver, signals[RELOAD], 0);
      }
  }
#endif
}
GList *g_resolver_lookup_by_name(GResolver *resolver, const gchar *hostname, GCancellable *cancellable, GError **error) {
  GInetAddress *addr;
  GList *addrs;
  gchar *ascii_hostname = NULL;
  g_return_val_if_fail(G_IS_RESOLVER(resolver), NULL);
  g_return_val_if_fail(hostname != NULL, NULL);
  addr = g_inet_address_new_from_string(hostname);
  if (addr) return g_list_append(NULL, addr);
  if (g_hostname_is_non_ascii(hostname)) hostname = ascii_hostname = g_hostname_to_ascii(hostname);
  g_resolver_maybe_reload(resolver);
  addrs = G_RESOLVER_GET_CLASS(resolver)->lookup_by_name(resolver, hostname, cancellable, error);
  g_free(ascii_hostname);
  return addrs;
}
void g_resolver_lookup_by_name_async(GResolver *resolver, const gchar *hostname, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GInetAddress *addr;
  gchar *ascii_hostname = NULL;
  g_return_if_fail(G_IS_RESOLVER (resolver));
  g_return_if_fail(hostname != NULL);
  addr = g_inet_address_new_from_string(hostname);
  if (addr) {
      GSimpleAsyncResult *simple;
      simple = g_simple_async_result_new(G_OBJECT(resolver), callback, user_data, g_resolver_lookup_by_name_async);
      g_simple_async_result_set_op_res_gpointer(simple, addr, g_object_unref);
      g_simple_async_result_complete_in_idle(simple);
      g_object_unref(simple);
      return;
  }
  if (g_hostname_is_non_ascii(hostname)) hostname = ascii_hostname = g_hostname_to_ascii(hostname);
  g_resolver_maybe_reload(resolver);
  G_RESOLVER_GET_CLASS(resolver)->lookup_by_name_async(resolver, hostname, cancellable, callback, user_data);
  g_free(ascii_hostname);
}
GList *g_resolver_lookup_by_name_finish(GResolver *resolver, GAsyncResult *result, GError **error) {
  g_return_val_if_fail(G_IS_RESOLVER(resolver), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
      if (g_simple_async_result_get_source_tag(simple) == g_resolver_lookup_by_name_async) {
          GInetAddress *addr;
          addr = g_simple_async_result_get_op_res_gpointer(simple);
          return g_list_append(NULL, g_object_ref(addr));
      }
  }
  return G_RESOLVER_GET_CLASS(resolver)->lookup_by_name_finish(resolver, result, error);
}
void g_resolver_free_addresses(GList *addresses) {
  GList *a;
  for (a = addresses; a; a = a->next) g_object_unref(a->data);
  g_list_free(addresses);
}
gchar *g_resolver_lookup_by_address(GResolver *resolver, GInetAddress *address, GCancellable *cancellable, GError **error) {
  g_return_val_if_fail(G_IS_RESOLVER(resolver), NULL);
  g_return_val_if_fail(G_IS_INET_ADDRESS(address), NULL);
  g_resolver_maybe_reload(resolver);
  return G_RESOLVER_GET_CLASS(resolver)->lookup_by_address(resolver, address, cancellable, error);
}
void g_resolver_lookup_by_address_async(GResolver *resolver, GInetAddress *address, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  g_return_if_fail(G_IS_RESOLVER(resolver));
  g_return_if_fail(G_IS_INET_ADDRESS(address));
  g_resolver_maybe_reload(resolver);
  G_RESOLVER_GET_CLASS(resolver)->lookup_by_address_async(resolver, address, cancellable, callback, user_data);
}
gchar *g_resolver_lookup_by_address_finish(GResolver *resolver, GAsyncResult *result, GError **error) {
  g_return_val_if_fail(G_IS_RESOLVER(resolver), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  return G_RESOLVER_GET_CLASS(resolver)->lookup_by_address_finish(resolver, result, error);
}
static gchar *g_resolver_get_service_rrname(const char *service, const char *protocol, const char *domain) {
  gchar *rrname, *ascii_domain = NULL;
  if (g_hostname_is_non_ascii(domain)) domain = ascii_domain = g_hostname_to_ascii(domain);
  rrname = g_strdup_printf("_%s._%s.%s", service, protocol, domain);
  g_free(ascii_domain);
  return rrname;
}
GList *g_resolver_lookup_service(GResolver *resolver, const gchar *service, const gchar *protocol, const gchar *domain, GCancellable *cancellable,
                                 GError **error) {
  GList *targets;
  gchar *rrname;
  g_return_val_if_fail(G_IS_RESOLVER(resolver), NULL);
  g_return_val_if_fail(service != NULL, NULL);
  g_return_val_if_fail(protocol != NULL, NULL);
  g_return_val_if_fail(domain != NULL, NULL);
  rrname = g_resolver_get_service_rrname(service, protocol, domain);
  g_resolver_maybe_reload(resolver);
  targets = G_RESOLVER_GET_CLASS(resolver)->lookup_service(resolver, rrname, cancellable, error);
  g_free(rrname);
  return targets;
}
void g_resolver_lookup_service_async(GResolver *resolver, const gchar *service, const gchar *protocol, const gchar *domain, GCancellable *cancellable,
                                     GAsyncReadyCallback callback, gpointer user_data) {
  gchar *rrname;
  g_return_if_fail(G_IS_RESOLVER(resolver));
  g_return_if_fail(service != NULL);
  g_return_if_fail(protocol != NULL);
  g_return_if_fail(domain != NULL);
  rrname = g_resolver_get_service_rrname(service, protocol, domain);
  g_resolver_maybe_reload(resolver);
  G_RESOLVER_GET_CLASS(resolver)->lookup_service_async(resolver, rrname, cancellable, callback, user_data);
  g_free(rrname);
}
GList *g_resolver_lookup_service_finish(GResolver *resolver, GAsyncResult *result, GError **error) {
  g_return_val_if_fail(G_IS_RESOLVER(resolver), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  return G_RESOLVER_GET_CLASS(resolver)->lookup_service_finish(resolver, result, error);
}
void g_resolver_free_targets(GList *targets) {
  GList *t;
  for (t = targets; t; t = t->next) g_srv_target_free(t->data);
  g_list_free(targets);
}
GQuark g_resolver_error_quark(void) {
  return g_quark_from_static_string("g-resolver-error-quark");
}
static GResolverError g_resolver_error_from_addrinfo_error(gint err) {
  switch(err) {
      case EAI_FAIL:
  #if defined(EAI_NODATA) && (EAI_NODATA != EAI_NONAME)
      case EAI_NODATA:
  #endif
      case EAI_NONAME: return G_RESOLVER_ERROR_NOT_FOUND;
      case EAI_AGAIN: return G_RESOLVER_ERROR_TEMPORARY_FAILURE;
      default: return G_RESOLVER_ERROR_INTERNAL;
  }
}
struct addrinfo _g_resolver_addrinfo_hints;
GList *_g_resolver_addresses_from_addrinfo(const char *hostname, struct addrinfo *res, gint gai_retval, GError **error) {
  struct addrinfo *ai;
  GSocketAddress *sockaddr;
  GInetAddress *addr;
  GList *addrs;
  if (gai_retval != 0) {
      g_set_error(error, G_RESOLVER_ERROR, g_resolver_error_from_addrinfo_error(gai_retval), _("Error resolving '%s': %s"), hostname,
		          gai_strerror(gai_retval));
      return NULL;
  }
  g_return_val_if_fail(res != NULL, NULL);
  addrs = NULL;
  for (ai = res; ai; ai = ai->ai_next) {
      sockaddr = g_socket_address_new_from_native(ai->ai_addr, ai->ai_addrlen);
      if (!sockaddr || !G_IS_INET_SOCKET_ADDRESS(sockaddr)) continue;
      addr = g_object_ref(g_inet_socket_address_get_address((GInetSocketAddress*)sockaddr));
      addrs = g_list_prepend(addrs, addr);
      g_object_unref(sockaddr);
  }
  return g_list_reverse(addrs);
}
void _g_resolver_address_to_sockaddr(GInetAddress *address, struct sockaddr_storage *sa, gsize *len) {
  GSocketAddress *sockaddr;
  sockaddr = g_inet_socket_address_new(address, 0);
  g_socket_address_to_native(sockaddr, (struct sockaddr*)sa, sizeof(*sa), NULL);
  *len = g_socket_address_get_native_size(sockaddr);
  g_object_unref(sockaddr);
}
char *_g_resolver_name_from_nameinfo(GInetAddress *address, const gchar *name, gint gni_retval, GError **error) {
  if (gni_retval != 0) {
      gchar *phys;
      phys = g_inet_address_to_string(address);
      g_set_error(error, G_RESOLVER_ERROR, g_resolver_error_from_addrinfo_error(gni_retval), _("Error reverse-resolving '%s': %s"),
                  phys ? phys : "(unknown)", gai_strerror(gni_retval));
      g_free(phys);
      return NULL;
  }
  return g_strdup(name);
}
#if !defined(G_OS_UNIX)
GList *_g_resolver_targets_from_res_query(const gchar *rrname, guchar *answer, gint len, gint herr, GError **error) {
  gint count;
  gchar namebuf[1024];
  guchar *end, *p;
  guint16 type, qclass, rdlength, priority, weight, port;
  guint32 ttl;
  HEADER *header;
  GSrvTarget *target;
  GList *targets;
  if (len <= 0) {
      GResolverError errnum;
      const gchar *format;
      if (len == 0 || herr == HOST_NOT_FOUND || herr == NO_DATA) {
          errnum = G_RESOLVER_ERROR_NOT_FOUND;
          format = _("No service record for '%s'");
      } else if (herr == TRY_AGAIN) {
          errnum = G_RESOLVER_ERROR_TEMPORARY_FAILURE;
          format = _("Temporarily unable to resolve '%s'");
      } else {
          errnum = G_RESOLVER_ERROR_INTERNAL;
          format = _("Error resolving '%s'");
      }
      g_set_error(error, G_RESOLVER_ERROR, errnum, format, rrname);
      return NULL;
  }
  targets = NULL;
  header = (HEADER*)answer;
  p = answer + sizeof(HEADER);
  end = answer + len;
  count = ntohs(header->qdcount);
  while(count-- && p < end) {
      p += dn_expand(answer, end, p, namebuf, sizeof(namebuf));
      p += 4;
  }
  count = ntohs(header->ancount);
  while(count-- && p < end) {
      p += dn_expand (answer, end, p, namebuf, sizeof(namebuf));
      GETSHORT(type, p);
      GETSHORT(qclass, p);
      GETLONG(ttl, p);
      GETSHORT(rdlength, p);
      if (type != T_SRV || qclass != C_IN) {
          p += rdlength;
          continue;
      }
      GETSHORT(priority, p);
      GETSHORT(weight, p);
      GETSHORT(port, p);
      p += dn_expand(answer, end, p, namebuf, sizeof(namebuf));
      target = g_srv_target_new(namebuf, port, priority, weight);
      targets = g_list_prepend(targets, target);
  }
  return g_srv_target_list_sort(targets);
}
#elif defined(G_OS_WIN32)
_g_resolver_targets_from_DnsQuery(const gchar *rrname, DNS_STATUS status, DNS_RECORD *results, GError **error) {
  DNS_RECORD *rec;
  GSrvTarget *target;
  GList *targets;
  if (status != ERROR_SUCCESS) {
      GResolverError errnum;
      const gchar *format;
      if (status == DNS_ERROR_RCODE_NAME_ERROR) {
          errnum = G_RESOLVER_ERROR_NOT_FOUND;
          format = _("No service record for '%s'");
      } else if (status == DNS_ERROR_RCODE_SERVER_FAILURE) {
          errnum = G_RESOLVER_ERROR_TEMPORARY_FAILURE;
          format = _("Temporarily unable to resolve '%s'");
      } else {
          errnum = G_RESOLVER_ERROR_INTERNAL;
          format = _("Error resolving '%s'");
      }
      g_set_error(error, G_RESOLVER_ERROR, errnum, format, rrname);
      return NULL;
  }
  targets = NULL;
  for (rec = results; rec; rec = rec->pNext) {
      if (rec->wType != DNS_TYPE_SRV) continue;
      target = g_srv_target_new(rec->Data.SRV.pNameTarget, rec->Data.SRV.wPort, rec->Data.SRV.wPriority, rec->Data.SRV.wWeight);
      targets = g_list_prepend(targets, target);
  }
  return g_srv_target_list_sort(targets);
}
#endif