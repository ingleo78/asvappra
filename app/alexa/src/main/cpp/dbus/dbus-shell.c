#include <string.h>
#include "config.h"
#include "dbus-internals.h"
#include "dbus-list.h"
#include "dbus-memory.h"
#include "dbus-protocol.h"
#include "dbus-shell.h"
#include "dbus-string.h"

static dbus_bool_t unquote_string_inplace(char* str, char** end) {
  char* dest;
  char* s;
  char quote_char;
  dest = s = str;
  quote_char = *s;
  if (!(*s == '"' || *s == '\'')) {
      *end = str;
      return FALSE;
  }
  ++s;
  if (quote_char == '"') {
      while(*s) {
          _dbus_assert(s > dest);
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
          _dbus_assert(s > dest);
      }
  } else {
      while(*s) {
          _dbus_assert(s > dest);
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
          _dbus_assert(s > dest);
      }
  }
  *dest = '\0';
  *end = s;
  return FALSE;
}
char* _dbus_shell_unquote(const char *quoted_string) {
  char *unquoted;
  char *end;
  char *start;
  char *ret;
  DBusString retval;
  unquoted = _dbus_strdup(quoted_string);
  if (unquoted == NULL) return NULL;
  start = unquoted;
  end = unquoted;
  if (!_dbus_string_init(&retval)) {
      dbus_free(unquoted);
      return NULL;
  }
  while(*start) {
      while(*start && !(*start == '"' || *start == '\'')) {
          if (*start == '\\') {
              ++start;
              if (*start) {
                  if (*start != '\n') {
                      if (!_dbus_string_append_byte(&retval, *start)) goto error;
                  }
                  ++start;
              }
          } else {
              if (!_dbus_string_append_byte(&retval, *start)) goto error;
              ++start;
          }
      }
      if (*start) {
          if (!unquote_string_inplace(start, &end)) goto error;
          else {
              if (!_dbus_string_append(&retval, start)) goto error;
              start = end;
          }
      }
  }
  ret = _dbus_strdup(_dbus_string_get_data(&retval));
  if (!ret) goto error;
  dbus_free(unquoted);
  _dbus_string_free(&retval);
  return ret;
error:
  dbus_free(unquoted);
  _dbus_string_free(&retval);
  return NULL;
}
static dbus_bool_t delimit_token(DBusString *token, DBusList **retval, DBusError *error) {
  char *str;
  str = _dbus_strdup(_dbus_string_get_data(token));
  if (!str) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (!_dbus_list_append(retval, str)) {
      dbus_free(str);
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  return TRUE;
}
static DBusList* tokenize_command_line(const char *command_line, DBusError *error) {
  char current_quote;
  const char *p;
  DBusString current_token;
  DBusList *retval = NULL;
  dbus_bool_t quoted;;
  current_quote = '\0';
  quoted = FALSE;
  p = command_line;
  if (!_dbus_string_init(&current_token)) {
      _DBUS_SET_OOM(error);
      return NULL;
  }
  while(*p) {
      if (current_quote == '\\') {
          if (*p == '\n');
          else {
              if (!_dbus_string_append_byte(&current_token, '\\') || !_dbus_string_append_byte(&current_token, *p)) {
                  _DBUS_SET_OOM(error);
                  goto error;
              }
          }
          current_quote = '\0';
      } else if (current_quote == '#') {
          while(*p && *p != '\n') ++p;
          current_quote = '\0';
          if (*p == '\0') break;
      } else if (current_quote) {
          if (*p == current_quote && !(current_quote == '"' && quoted)) current_quote = '\0';
          if (!_dbus_string_append_byte(&current_token, *p)) {
              _DBUS_SET_OOM(error);
              goto error;
	      }
      } else {
          switch(*p) {
              case '\n':
                  if (!delimit_token(&current_token, &retval, error)) goto error;
                  _dbus_string_free(&current_token);
                  if (!_dbus_string_init(&current_token)) {
                      _DBUS_SET_OOM(error);
                      goto init_error;
                  }
                  break;
              case ' ': case '\t':
                  if (_dbus_string_get_length(&current_token) > 0) {
                      if (!delimit_token(&current_token, &retval, error)) goto error;
                      _dbus_string_free(&current_token);
                      if (!_dbus_string_init(&current_token)) {
                          _DBUS_SET_OOM(error);
                          goto init_error;
		              }

                  }
                  break;
              case '\'': case '"':
                  if (!_dbus_string_append_byte(&current_token, *p)) {
                      _DBUS_SET_OOM(error);
                      goto error;
                  }
              case '#': case '\\': current_quote = *p; break;
              default:
                  if (!_dbus_string_append_byte(&current_token, *p)) {
                      _DBUS_SET_OOM(error);
                      goto error;
                  }
                  break;
          }
      }
      if (*p != '\\') quoted = FALSE;
      else quoted = !quoted;
      ++p;
  }
  if (!delimit_token(&current_token, &retval, error)) goto error;
  if (current_quote) {
      dbus_set_error_const(error, DBUS_ERROR_INVALID_ARGS, "Unclosed quotes in command line");
      goto error;
  }
  if (retval == NULL) {
      dbus_set_error_const(error, DBUS_ERROR_INVALID_ARGS, "No tokens found in command line");
      goto error;
  }
  _dbus_string_free(&current_token);
  return retval;
error:
  _dbus_string_free(&current_token);
init_error:
  if (retval) {
      _dbus_list_foreach(&retval, (DBusForeachFunction)dbus_free, NULL);
      _dbus_list_clear(&retval);
  }
  return NULL;
}
dbus_bool_t _dbus_shell_parse_argv(const char *command_line, int *argcp, char ***argvp, DBusError *error) {
  int argc = 0;
  char **argv = NULL;
  DBusList *tokens = NULL;
  int i;
  DBusList *tmp_list;
  if (!command_line) {
      _dbus_verbose("Command line is NULL\n");
      return FALSE;
  }
  tokens = tokenize_command_line(command_line, error);
  if (tokens == NULL) {
      _dbus_verbose("No tokens for command line '%s'\n", command_line);
      return FALSE;
  }
  argc = _dbus_list_get_length(&tokens);
  argv = dbus_new(char*, argc + 1);
  if (!argv) {
      _DBUS_SET_OOM(error);
      goto error;
  }
  i = 0;
  tmp_list = tokens;
  while(tmp_list) {
      argv[i] = _dbus_shell_unquote(tmp_list->data);
      if (!argv[i]) {
          int j;
	      for (j = 0; j < i; j++) dbus_free(argv[j]);
          dbus_free(argv);
          _DBUS_SET_OOM(error);
          goto error;
      }
      tmp_list = _dbus_list_get_next_link(&tokens, tmp_list);
      ++i;
  }
  argv[argc] = NULL;
  _dbus_list_foreach(&tokens, (DBusForeachFunction)dbus_free, NULL);
  _dbus_list_clear(&tokens);
  if (argcp) *argcp = argc;
  if (argvp) *argvp = argv;
  else dbus_free_string_array(argv);
  return TRUE;
error:
  _dbus_list_foreach(&tokens, (DBusForeachFunction)dbus_free, NULL);
  _dbus_list_clear(&tokens);
  return FALSE;
}