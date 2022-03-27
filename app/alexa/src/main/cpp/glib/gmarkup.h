#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_MARKUP_H__
#define __G_MARKUP_H__

#include <stdarg.h>
#include "gerror.h"
#include "gslist.h"

G_BEGIN_DECLS
typedef enum {
  G_MARKUP_ERROR_BAD_UTF8,
  G_MARKUP_ERROR_EMPTY,
  G_MARKUP_ERROR_PARSE,
  G_MARKUP_ERROR_UNKNOWN_ELEMENT,
  G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE,
  G_MARKUP_ERROR_INVALID_CONTENT,
  G_MARKUP_ERROR_MISSING_ATTRIBUTE
} GMarkupError;
#define G_MARKUP_ERROR g_markup_error_quark ()
GQuark g_markup_error_quark (void);
typedef enum {
  G_MARKUP_DO_NOT_USE_THIS_UNSUPPORTED_FLAG = 1 << 0,
  G_MARKUP_TREAT_CDATA_AS_TEXT              = 1 << 1,
  G_MARKUP_PREFIX_ERROR_POSITION            = 1 << 2
} GMarkupParseFlags;
typedef struct _GMarkupParseContext GMarkupParseContext;
typedef struct _GMarkupParser GMarkupParser;
struct _GMarkupParser {
  void (*start_element)(GMarkupParseContext *context, const gchar *element_name, const gchar **attribute_names, const gchar **attribute_values, gpointer user_data,
                        GError **error);
  void (*end_element)(GMarkupParseContext *context, const gchar *element_name, gpointer user_data, GError **error);
  void (*text)(GMarkupParseContext *context, const gchar *text, gsize text_len, gpointer user_data, GError **error);
  void (*passthrough)(GMarkupParseContext *context, const gchar *passthrough_text, gsize text_len, gpointer user_data, GError **error);
  void (*error)(GMarkupParseContext *context, GError *error, gpointer user_data);
};
GMarkupParseContext *g_markup_parse_context_new(const GMarkupParser *parser, GMarkupParseFlags flags, gpointer user_data, GDestroyNotify user_data_dnotify);
void g_markup_parse_context_free(GMarkupParseContext *context);
gboolean g_markup_parse_context_parse(GMarkupParseContext *context, const gchar *text, gssize text_len, GError **error);
void g_markup_parse_context_push(GMarkupParseContext *context, const GMarkupParser *parser, gpointer user_data);
gpointer g_markup_parse_context_pop(GMarkupParseContext *context);
gboolean g_markup_parse_context_end_parse(GMarkupParseContext *context, GError **error);
G_CONST_RETURN gchar *g_markup_parse_context_get_element(GMarkupParseContext *context);
G_CONST_RETURN GSList *g_markup_parse_context_get_element_stack(GMarkupParseContext *context);
void g_markup_parse_context_get_position(GMarkupParseContext *context, gint *line_number, gint *char_number);
gpointer g_markup_parse_context_get_user_data(GMarkupParseContext *context);
gchar* g_markup_escape_text(const gchar *text, gssize length);
gchar *g_markup_printf_escaped(const char *format, ...) G_GNUC_PRINTF (1, 2);
gchar *g_markup_vprintf_escaped(const char *format, va_list args);
typedef enum {
  G_MARKUP_COLLECT_INVALID,
  G_MARKUP_COLLECT_STRING,
  G_MARKUP_COLLECT_STRDUP,
  G_MARKUP_COLLECT_BOOLEAN,
  G_MARKUP_COLLECT_TRISTATE,
  G_MARKUP_COLLECT_OPTIONAL = (1 << 16)
} GMarkupCollectType;
gboolean g_markup_collect_attributes(const gchar *element_name, const gchar **attribute_names, const gchar **attribute_values, GError **error,
                                     GMarkupCollectType first_type, const gchar *first_attr, ...);
G_END_DECLS

#endif