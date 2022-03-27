#include <string.h>
#include "gshell.h"
#include "gslist.h"
#include "gstrfuncs.h"
#include "gstring.h"
#include "gtestutils.h"
#include "glibintl.h"

GQuark g_shell_error_quark(void) {
  return g_quark_from_static_string("g-shell-error-quark");
}
static gboolean unquote_string_inplace(gchar* str, gchar** end, GError** err) {
  gchar* dest;
  gchar* s;
  gchar quote_char;
  g_return_val_if_fail(end != NULL, FALSE);
  g_return_val_if_fail(err == NULL || *err == NULL, FALSE);
  g_return_val_if_fail(str != NULL, FALSE);
  dest = s = str;
  quote_char = *s;
  if (!(*s == '"' || *s == '\'')) {
      g_set_error_literal(err, G_SHELL_ERROR,G_SHELL_ERROR_BAD_QUOTING, _("Quoted text doesn't begin with a quotation mark"));
      *end = str;
      return FALSE;
  }
  ++s;
  if (quote_char == '"') {
      while(*s) {
          g_assert(s > dest);
          switch(*s) {
              case '"':
                  *dest = '\0';
                  ++s;
                  *end = s;
                  return TRUE;
              case '\\':
                  ++s;
                  switch(*s) {
                      case '"': case '\\': case '`': case '$': case '\n':
                          *dest = *s;
                          ++s;
                          ++dest;
                          break;
                      default:
                          *dest = '\\';
                          ++dest;
                          break;
                  }
                  break;
              default:
                  *dest = *s;
                  ++dest;
                  ++s;
                  break;
          }
          g_assert(s > dest);
      }
  } else {
      while(*s) {
          g_assert(s > dest);
          if (*s == '\'') {
              *dest = '\0';
              ++s;
              *end = s;
              return TRUE;
          } else {
              *dest = *s;
              ++dest;
              ++s;
          }
          g_assert(s > dest);
      }
  }
  *dest = '\0';
  g_set_error_literal(err, G_SHELL_ERROR,G_SHELL_ERROR_BAD_QUOTING, _("Unmatched quotation mark in command line or other shell-quoted text"));
  *end = s;
  return FALSE;
}
gchar* g_shell_quote(const gchar *unquoted_string) {
  const gchar *p;
  GString *dest;
  g_return_val_if_fail(unquoted_string != NULL, NULL);
  dest = g_string_new("'");
  p = unquoted_string;
  while(*p) {
      if (*p == '\'') g_string_append(dest, "'\\''");
      else g_string_append_c(dest, *p);
      ++p;
  }
  g_string_append_c(dest, '\'');
  return g_string_free(dest, FALSE);
}
gchar* g_shell_unquote(const gchar *quoted_string, GError **error) {
  gchar *unquoted;
  gchar *end;
  gchar *start;
  GString *retval;
  g_return_val_if_fail(quoted_string != NULL, NULL);
  unquoted = g_strdup(quoted_string);
  start = unquoted;
  end = unquoted;
  retval = g_string_new(NULL);
  while(*start) {
      while (*start && !(*start == '"' || *start == '\'')) {
          if (*start == '\\') {
              ++start;
              if (*start) {
                  if (*start != '\n') g_string_append_c(retval, *start);
                  ++start;
              }
          } else {
              g_string_append_c(retval, *start);
              ++start;
          }
      }
      if (*start) {
          if (!unquote_string_inplace(start, &end, error)) goto error;
          else {
              g_string_append(retval, start);
              start = end;
          }
      }
  }
  g_free(unquoted);
  return g_string_free(retval, FALSE);
  error:
  g_assert(error == NULL || *error != NULL);
  g_free(unquoted);
  g_string_free(retval, TRUE);
  return NULL;
}
static inline void ensure_token(GString **token) {
  if (*token == NULL) *token = g_string_new(NULL);
}
static void delimit_token(GString **token, GSList **retval) {
  if (*token == NULL) return;
  *retval = g_slist_prepend(*retval, g_string_free(*token, FALSE));
  *token = NULL;
}
static GSList* tokenize_command_line(const gchar *command_line, GError **error) {
  gchar current_quote;
  const gchar *p;
  GString *current_token = NULL;
  GSList *retval = NULL;
  gboolean quoted;
  current_quote = '\0';
  quoted = FALSE;
  p = command_line;
  while(*p) {
      if (current_quote == '\\') {
          if (*p == '\n');
          else {
              ensure_token(&current_token);
              g_string_append_c(current_token, '\\');
              g_string_append_c(current_token, *p);
          }
          current_quote = '\0';
      } else if (current_quote == '#') {
          while(*p && *p != '\n') ++p;
          current_quote = '\0';
          if (*p == '\0') break;
      } else if (current_quote) {
          if (*p == current_quote && !(current_quote == '"' && quoted)) current_quote = '\0';
          ensure_token (&current_token);
          g_string_append_c(current_token, *p);
      } else {
          switch (*p) {
              case '\n': delimit_token(&current_token, &retval); break;
              case ' ': case '\t':
                  if (current_token && current_token->len > 0) delimit_token(&current_token, &retval);
                  break;
              case '\'': case '"':
                  ensure_token(&current_token);
                  g_string_append_c(current_token, *p);
              case '#': case '\\': current_quote = *p; break;
              default:
                  ensure_token(&current_token);
                  g_string_append_c(current_token, *p);
                  break;
          }
      }
      if (*p != '\\') quoted = FALSE;
      else quoted = !quoted;
      ++p;
  }
  delimit_token (&current_token, &retval);
  if (current_quote) {
      if (current_quote == '\\') {
          g_set_error(error, G_SHELL_ERROR,G_SHELL_ERROR_BAD_QUOTING, _("Text ended just after a '\\' character. (The text was '%s')"), command_line);
      } else {
          g_set_error(error, G_SHELL_ERROR,G_SHELL_ERROR_BAD_QUOTING, _("Text ended before matching quote was found for %c. (The text was '%s')"),
                      current_quote, command_line);
      }
      goto error;
  }
  if (retval == NULL) {
      g_set_error_literal(error, G_SHELL_ERROR,G_SHELL_ERROR_EMPTY_STRING, _("Text was empty (or contained only whitespace)"));
      goto error;
  }
  retval = g_slist_reverse(retval);
  return retval;
  error:
  g_assert(error == NULL || *error != NULL);
  if (retval) {
      g_slist_foreach(retval, (GFunc)g_free, NULL);
      g_slist_free(retval);
  }
  return NULL;
}
gboolean g_shell_parse_argv(const gchar *command_line, gint *argcp, gchar ***argvp, GError **error) {
  gint argc = 0;
  gchar **argv = NULL;
  GSList *tokens = NULL;
  gint i;
  GSList *tmp_list;
  g_return_val_if_fail(command_line != NULL, FALSE);
  tokens = tokenize_command_line(command_line, error);
  if (tokens == NULL) return FALSE;
  argc = g_slist_length(tokens);
  argv = g_new0(gchar*, argc + 1);
  i = 0;
  tmp_list = tokens;
  while(tmp_list) {
      argv[i] = g_shell_unquote(tmp_list->data, error);
      if (argv[i] == NULL) goto failed;
      tmp_list = g_slist_next(tmp_list);
      ++i;
  }
  g_slist_foreach(tokens, (GFunc)g_free, NULL);
  g_slist_free(tokens);
  if (argcp) *argcp = argc;
  if (argvp) *argvp = argv;
  else g_strfreev(argv);
  return TRUE;
  failed:
  g_assert(error == NULL || *error != NULL);
  g_strfreev(argv);
  g_slist_foreach(tokens, (GFunc)g_free, NULL);
  g_slist_free(tokens);
  return FALSE;
}