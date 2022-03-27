#if defined (__GLIB_H_INSIDE__) && defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_HOST_UTILS_H__
#define __G_HOST_UTILS_H__

#include "gmacros.h"
#include "gtypes.h"

G_BEGIN_DECLS
gboolean g_hostname_is_non_ascii(const gchar *hostname);
gboolean g_hostname_is_ascii_encoded(const gchar *hostname);
gboolean g_hostname_is_ip_address(const gchar *hostname);
gchar *g_hostname_to_ascii(const gchar *hostname);
gchar *g_hostname_to_unicode(const gchar *hostname);
G_END_DECLS

#endif