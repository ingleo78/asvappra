#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_ERROR_H__
#define __G_ERROR_H__

#include <stdarg.h>
#include "gmacros.h"
#include "gquark.h"

G_BEGIN_DECLS
typedef struct _GError GError;
struct _GError {
  GQuark domain;
  gint code;
  gchar *message;
};
GError* g_error_new(GQuark domain, gint code, const gchar *format, ...) G_GNUC_PRINTF (3, 4);
GError* g_error_new_literal(GQuark domain, gint code, const gchar *message);
GError* g_error_new_valist(GQuark domain, gint code, const gchar *format, va_list args);
void g_error_free(GError *error);
GError* g_error_copy(const GError *error);
gboolean g_error_matches(const GError *error, GQuark domain, gint code);
void g_set_error(GError **err, GQuark domain, gint code, const gchar *format, ...) G_GNUC_PRINTF (4, 5);
void g_set_error_literal(GError **err, GQuark domain, gint code, const gchar *message);
void g_propagate_error(GError **dest, GError *src);
void g_clear_error(GError **err);
void g_prefix_error(GError **err, const gchar *format, ...) G_GNUC_PRINTF (2, 3);
void g_propagate_prefixed_error(GError **dest, GError *src, const gchar *format, ...) G_GNUC_PRINTF (3, 4);
G_END_DECLS

#endif