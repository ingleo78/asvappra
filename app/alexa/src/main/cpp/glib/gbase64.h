#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_BASE64_H__
#define __G_BASE64_H__

#include "gmacros.h"
#include "gtypes.h"
#include "glib-basic-types.h"

G_BEGIN_DECLS
gsize g_base64_encode_step(const guchar *in, gsize len, gboolean break_lines, gchar *out, gint *state, gint *save);
gsize g_base64_encode_close(gboolean break_lines, gchar *out, gint *state, gint *save);
gchar* g_base64_encode(const guchar *data, gsize len) G_GNUC_MALLOC;
gsize g_base64_decode_step(const gchar *in, gsize len, guchar *out, gint *state, guint *save);
guchar *g_base64_decode(const gchar *text, gsize *out_len) G_GNUC_MALLOC;
guchar *g_base64_decode_inplace(gchar *text, gsize *out_len);
G_END_DECLS
#endif