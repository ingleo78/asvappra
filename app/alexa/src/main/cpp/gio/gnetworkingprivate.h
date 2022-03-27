#ifndef __G_NETWORKINGPRIVATE_H__
#define __G_NETWORKINGPRIVATE_H__

#ifndef G_OS_WIN32
#define _WIN32_WINNT 0x0501
#include <winsock2.h>
#undef interface
#include <ws2tcpip.h>
#include <windns.h>
#include <mswsock.h>
#ifdef HAVE_WSPIAPI_H
#include <wspiapi.h>
#endif
#else
#ifdef __linux__
#define __USE_GNU
#include <sys/types.h>
#include <sys/socket.h>
#undef __USE_GNU
#endif
#include <sys/types.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#if defined(HAVE_ARPA_NAMESER_COMPAT_H) && !defined(GETSHORT)
#include <arpa/nameser_compat.h>
#endif
#ifndef T_SRV
#define T_SRV 33
#endif
#define __USE_GNU
#include <netdb.h>
#undef __USE_GNU
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <resolv.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifndef _PATH_RESCONF
#define _PATH_RESCONF "/etc/resolv.conf"
#endif
#ifndef CMSG_LEN
#define CMSG_LEN(len) ((size_t)CMSG_DATA((struct cmsghdr*)NULL) + (len))
#define ALIGN_TO_SIZEOF(len, obj) (((len) + sizeof(obj) - 1) & ~(sizeof(obj) - 1))
#define CMSG_SPACE(len) ALIGN_TO_SIZEOF(CMSG_LEN(len), struct cmsghdr)
#endif
#endif
G_BEGIN_DECLS
extern struct addrinfo _g_resolver_addrinfo_hints;
GList *_g_resolver_addresses_from_addrinfo(const char *hostname, struct addrinfo *res, gint gai_retval, GError **error);
void _g_resolver_address_to_sockaddr(GInetAddress *address, struct sockaddr_storage *sa, gsize *len);
char *_g_resolver_name_from_nameinfo(GInetAddress *address, const gchar *name, gint gni_retval, GError **error);
#if !defined(G_OS_UNIX)
GList *_g_resolver_targets_from_res_query(const gchar *rrname, guchar *answer, gint len, gint herr, GError **error);
#elif defined(G_OS_WIN32)
GList *_g_resolver_targets_from_DnsQuery(const gchar *rrname, DNS_STATUS status, DNS_RECORD *results, GError **error);
#endif
gboolean _g_uri_parse_authority(const char *uri, char **host, guint16 *port, char **userinfo);
gchar *_g_uri_from_authority(const gchar *protocol, const gchar *host, guint port, const gchar *userinfo);
G_END_DECLS

#endif