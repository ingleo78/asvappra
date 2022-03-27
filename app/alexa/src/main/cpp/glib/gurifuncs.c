#include <string.h>
#include "gurifuncs.h"
#include "gstrfuncs.h"
#include "gmessages.h"
#include "gstring.h"
#include "gmem.h"

static int unescape_character(const char *scanner) {
  int first_digit;
  int second_digit;
  first_digit = g_ascii_xdigit_value (*scanner++);
  if (first_digit < 0) return -1;
  second_digit = g_ascii_xdigit_value (*scanner++);
  if (second_digit < 0) return -1;
  return (first_digit << 4) | second_digit;
}
char* g_uri_unescape_segment(const char *escaped_string, const char *escaped_string_end, const char *illegal_characters) {
  const char *in;
  char *out, *result;
  gint character;
  if (escaped_string == NULL) return NULL;
  if (escaped_string_end == NULL) escaped_string_end = escaped_string + strlen(escaped_string);
  result = g_malloc(escaped_string_end - escaped_string + 1);
  out = result;
  for (in = escaped_string; in < escaped_string_end; in++) {
      character = *in;
      if (*in == '%') {
          in++;
          if (escaped_string_end - in < 2) {
              g_free(result);
              return NULL;
          }
          character = unescape_character(in);
          if (character <= 0 || (illegal_characters != NULL && strchr(illegal_characters, (char)character) != NULL)) {
              g_free(result);
              return NULL;
          }
          in++;
	  }
      *out++ = (char)character;
  }
  *out = '\0';
  return result;
}
char* g_uri_unescape_string(const char *escaped_string, const char *illegal_characters) {
  return g_uri_unescape_segment(escaped_string, NULL, illegal_characters);
}
char* g_uri_parse_scheme(const char  *uri) {
  const char *p;
  char c;
  g_return_val_if_fail(uri != NULL, NULL);
  p = uri;
  if (!g_ascii_isalpha(*p)) return NULL;
  while(1) {
      c = *p++;
      if (c == ':') break;
      if (!(g_ascii_isalnum(c) || c == '+' || c == '-' || c == '.')) return NULL;
  }
  return g_strndup(uri, p - uri - 1);
}
char* g_uri_escape_string(const char *unescaped, const char *reserved_chars_allowed, gboolean allow_utf8) {
  GString *s;
  g_return_val_if_fail(unescaped != NULL, NULL);
  s = g_string_sized_new(strlen(unescaped) + 10);
  g_string_append_uri_escaped(s, unescaped, reserved_chars_allowed, allow_utf8);
  return g_string_free(s, FALSE);
}