#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_SHELL_H__
#define __G_SHELL_H__

#include "gerror.h"

G_BEGIN_DECLS
#define G_SHELL_ERROR g_shell_error_quark ()
typedef enum {
  G_SHELL_ERROR_BAD_QUOTING,
  G_SHELL_ERROR_EMPTY_STRING,
  G_SHELL_ERROR_FAILED
} GShellError;
GQuark g_shell_error_quark(void);
gchar* g_shell_quote(const gchar *unquoted_string);
gchar* g_shell_unquote(const gchar *quoted_string, GError **error);
gboolean g_shell_parse_argv(const gchar *command_line, gint *argcp, gchar ***argvp, GError **error);
G_END_DECLS

#endif