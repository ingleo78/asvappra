#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_CONTENT_TYPE_H__
#define __G_CONTENT_TYPE_H__

#include "giotypes.h"

G_BEGIN_DECLS
gboolean g_content_type_equals(const gchar *type1, const gchar *type2);
gboolean g_content_type_is_a(const gchar *type, const gchar *supertype);
gboolean g_content_type_is_unknown(const gchar *type);
gchar *g_content_type_get_description(const gchar *type);
gchar *g_content_type_get_mime_type(const gchar *type);
GIcon *g_content_type_get_icon(const gchar *type);
gboolean g_content_type_can_be_executable(const gchar *type);
gchar *g_content_type_from_mime_type(const gchar *mime_type);
gchar *g_content_type_guess(const gchar *filename, const guchar *data, gsize data_size, gboolean *result_uncertain);
gchar **g_content_type_guess_for_tree(GFile *root);
GList *g_content_types_get_registered(void);
G_END_DECLS

#endif