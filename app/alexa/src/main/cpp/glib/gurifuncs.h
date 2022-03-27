#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_URI_FUNCS_H__
#define __G_URI_FUNCS_H__

#include "gmacros.h"
#include "gtypes.h"

G_BEGIN_DECLS
#define G_URI_RESERVED_CHARS_GENERIC_DELIMITERS ":/?#[]@"
#define G_URI_RESERVED_CHARS_SUBCOMPONENT_DELIMITERS "!$&'()*+,;="
#define G_URI_RESERVED_CHARS_ALLOWED_IN_PATH_ELEMENT G_URI_RESERVED_CHARS_SUBCOMPONENT_DELIMITERS ":@"
#define G_URI_RESERVED_CHARS_ALLOWED_IN_PATH G_URI_RESERVED_CHARS_ALLOWED_IN_PATH_ELEMENT "/"
#define G_URI_RESERVED_CHARS_ALLOWED_IN_USERINFO G_URI_RESERVED_CHARS_SUBCOMPONENT_DELIMITERS ":"
char* g_uri_unescape_string(const char *escaped_string,const char *illegal_characters);
char* g_uri_unescape_segment(const char *escaped_string, const char *escaped_string_end, const char *illegal_characters);
char* g_uri_parse_scheme(const char *uri);
char* g_uri_escape_string(const char *unescaped, const char *reserved_chars_allowed, gboolean allow_utf8);
G_END_DECLS

#endif