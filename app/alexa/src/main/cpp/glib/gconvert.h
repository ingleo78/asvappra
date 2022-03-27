#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_CONVERT_H__
#define __G_CONVERT_H__

#include "gmacros.h"
#include "gtypes.h"
#include "gquark.h"
#include "gerror.h"

G_BEGIN_DECLS
typedef enum {
  G_CONVERT_ERROR_NO_CONVERSION,
  G_CONVERT_ERROR_ILLEGAL_SEQUENCE,
  G_CONVERT_ERROR_FAILED,
  G_CONVERT_ERROR_PARTIAL_INPUT,
  G_CONVERT_ERROR_BAD_URI,
  G_CONVERT_ERROR_NOT_ABSOLUTE_PATH
} GConvertError;
#define G_CONVERT_ERROR g_convert_error_quark()
GQuark g_convert_error_quark (void);
typedef struct _GIConv *GIConv;
GIConv g_iconv_open(const gchar *to_codeset, const gchar *from_codeset);
gsize g_iconv(GIConv converter, gchar **inbuf, gsize *inbytes_left, gchar **outbuf, gsize *outbytes_left);
gint g_iconv_close(GIConv converter);
gchar* g_convert(const gchar *str, gssize len, const gchar *to_codeset, const gchar *from_codeset, gsize *bytes_read, gsize *bytes_written, GError **error) G_GNUC_MALLOC;
gchar* g_convert_with_iconv(const gchar  *str, gssize len, GIConv converter, gsize *bytes_read, gsize *bytes_written, GError **error) G_GNUC_MALLOC;
gchar* g_convert_with_fallback(const gchar  *str, gssize len, const gchar *to_codeset, const gchar *from_codeset, const gchar *fallback, gsize *bytes_read,
							   gsize *bytes_written, GError **error) G_GNUC_MALLOC;
gchar* g_locale_to_utf8(const gchar *opsysstring, gssize len, gsize *bytes_read, gsize *bytes_written, GError **error) G_GNUC_MALLOC;
gchar* g_locale_from_utf8(const gchar  *utf8string, gssize len, gsize *bytes_read, gsize *bytes_written, GError **error) G_GNUC_MALLOC;
#ifdef G_OS_WIN32
#define g_filename_to_utf8 g_filename_to_utf8_utf8
#define g_filename_from_utf8 g_filename_from_utf8_utf8
#define g_filename_from_uri g_filename_from_uri_utf8
#define g_filename_to_uri g_filename_to_uri_utf8 
#endif
gchar* g_filename_to_utf8(const gchar *opsysstring, gssize len, gsize *bytes_read, gsize *bytes_written, GError **error) G_GNUC_MALLOC;
gchar* g_filename_from_utf8(const gchar *utf8string, gssize len, gsize *bytes_read, gsize *bytes_written, GError **error) G_GNUC_MALLOC;
gchar *g_filename_from_uri(const gchar *uri, gchar **hostname, GError **error) G_GNUC_MALLOC;
gchar *g_filename_to_uri(const gchar *filename, const gchar **hostname, GError **error) G_GNUC_MALLOC;
gchar *g_filename_display_name(const gchar *filename) G_GNUC_MALLOC;
gboolean g_get_filename_charsets(G_CONST_RETURN gchar ***charsets);
gchar *g_filename_display_basename(const gchar *filename) G_GNUC_MALLOC;
gchar **g_uri_list_extract_uris(const gchar *uri_list) G_GNUC_MALLOC;
G_END_DECLS

#endif