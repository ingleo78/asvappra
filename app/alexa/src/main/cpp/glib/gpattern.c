#include <string.h>
#include "gpattern.h"
#include "gmacros.h"
#include "gmessages.h"
#include "gmem.h"
#include "gunicode.h"
#include "gutils.h" 

typedef enum {
  G_MATCH_ALL,
  G_MATCH_ALL_TAIL,
  G_MATCH_HEAD,
  G_MATCH_TAIL,
  G_MATCH_EXACT,
  G_MATCH_LAST
} GMatchType;
struct _GPatternSpec {
  GMatchType match_type;
  guint pattern_length;
  guint min_length;
  guint max_length;
  gchar *pattern;
};
static inline gboolean g_pattern_ph_match(const gchar *match_pattern, const gchar *match_string, gboolean *wildcard_reached_p) {
  register const gchar *pattern, *string;
  register gchar ch;
  pattern = match_pattern;
  string = match_string;
  ch = *pattern;
  pattern++;
  while(ch) {
      switch(ch) {
	      case '?':
              if (!*string) return FALSE;
              string = g_utf8_next_char(string);
              break;
          case '*':
              *wildcard_reached_p = TRUE;
              do {
                  ch = *pattern;
                  pattern++;
                  if (ch == '?'){
                  if (!*string) return FALSE;
                  string = g_utf8_next_char (string);
                  }
              } while(ch == '*' || ch == '?');
              if (!ch) return TRUE;
              do {
                  gboolean next_wildcard_reached = FALSE;
                  while(ch != *string) {
                      if (!*string) return FALSE;
                      string = g_utf8_next_char(string);
                  }
                  string++;
                  if (g_pattern_ph_match (pattern, string, &next_wildcard_reached)) return TRUE;
                      if (next_wildcard_reached) return FALSE;
              } while(*string);
              break;
          default:
              if (ch == *string) string++;
              else return FALSE;
              break;
	  }
      ch = *pattern;
      pattern++;
  }
  return *string == 0;
}
gboolean g_pattern_match(GPatternSpec *pspec, guint string_length, const gchar *string, const gchar *string_reversed) {
  g_return_val_if_fail(pspec != NULL, FALSE);
  g_return_val_if_fail(string != NULL, FALSE);
  if (string_length < pspec->min_length || string_length > pspec->max_length) return FALSE;
  gboolean dummy;
  switch(pspec->match_type) {
      case G_MATCH_ALL: return g_pattern_ph_match (pspec->pattern, string, &dummy);
      case G_MATCH_ALL_TAIL:
          if (string_reversed) return g_pattern_ph_match (pspec->pattern, string_reversed, &dummy);
          else {
              gboolean result;
              gchar *tmp;
              tmp = g_utf8_strreverse(string, string_length);
              result = g_pattern_ph_match(pspec->pattern, tmp, &dummy);
              g_free(tmp);
              return result;
          }
      case G_MATCH_HEAD:
          if (pspec->pattern_length == string_length) return strcmp (pspec->pattern, string) == 0;
          else if (pspec->pattern_length) return strncmp(pspec->pattern, string, pspec->pattern_length) == 0;
          else return TRUE;
          case G_MATCH_TAIL:
          if (pspec->pattern_length) return strcmp(pspec->pattern, string + (string_length - pspec->pattern_length)) == 0;
          else return TRUE;
          case G_MATCH_EXACT:
          if (pspec->pattern_length != string_length) return FALSE;
          else return strcmp(pspec->pattern, string) == 0;
      default:
          g_return_val_if_fail(pspec->match_type < G_MATCH_LAST, FALSE);
          return FALSE;
  }
}
GPatternSpec* g_pattern_spec_new(const gchar *pattern) {
  GPatternSpec *pspec;
  gboolean seen_joker = FALSE, seen_wildcard = FALSE, more_wildcards = FALSE;
  gint hw_pos = -1, tw_pos = -1, hj_pos = -1, tj_pos = -1;
  gboolean follows_wildcard = FALSE;
  guint pending_jokers = 0;
  const gchar *s;
  gchar *d;
  guint i;
  g_return_val_if_fail(pattern != NULL, NULL);
  pspec = g_new(GPatternSpec, 1);
  pspec->pattern_length = strlen(pattern);
  pspec->min_length = 0;
  pspec->max_length = 0;
  pspec->pattern = g_new(gchar, pspec->pattern_length + 1);
  d = pspec->pattern;
  for (i = 0, s = pattern; *s != 0; s++) {
      switch (*s){
          case '*':
              if (follows_wildcard) {
                  pspec->pattern_length--;
                  continue;
              }
              follows_wildcard = TRUE;
              if (hw_pos < 0) hw_pos = i;
              tw_pos = i;
              break;
          case '?':
              pending_jokers++;
              pspec->min_length++;
              pspec->max_length += 4;
              continue;
          default:
              for (; pending_jokers; pending_jokers--, i++) {
                  *d++ = '?';
                  if (hj_pos < 0) hj_pos = i;
                  tj_pos = i;
              }
              follows_wildcard = FALSE;
              pspec->min_length++;
              pspec->max_length++;
              break;
	  }
      *d++ = *s;
      i++;
  }
  for (; pending_jokers; pending_jokers--) {
    *d++ = '?';
    if (hj_pos < 0)
      hj_pos = i;
    tj_pos = i;
  }
  *d++ = 0;
  seen_joker = hj_pos >= 0;
  seen_wildcard = hw_pos >= 0;
  more_wildcards = seen_wildcard && hw_pos != tw_pos;
  if (seen_wildcard) pspec->max_length = G_MAXUINT;
  if (!seen_joker && !more_wildcards) {
      if (pspec->pattern[0] == '*') {
	      pspec->match_type = G_MATCH_TAIL;
          memmove (pspec->pattern, pspec->pattern + 1, --pspec->pattern_length);
          pspec->pattern[pspec->pattern_length] = 0;
          return pspec;
	  }
      if (pspec->pattern_length > 0 && pspec->pattern[pspec->pattern_length - 1] == '*') {
          pspec->match_type = G_MATCH_HEAD;
          pspec->pattern[--pspec->pattern_length] = 0;
          return pspec;
	  }
      if (!seen_wildcard) {
          pspec->match_type = G_MATCH_EXACT;
          return pspec;
	  }
  }
  tw_pos = pspec->pattern_length - 1 - tw_pos;
  tj_pos = pspec->pattern_length - 1 - tj_pos;
  if (seen_wildcard) pspec->match_type = tw_pos > hw_pos ? G_MATCH_ALL_TAIL : G_MATCH_ALL;
  else pspec->match_type = tj_pos > hj_pos ? G_MATCH_ALL_TAIL : G_MATCH_ALL;
  if (pspec->match_type == G_MATCH_ALL_TAIL) {
      gchar *tmp = pspec->pattern;
      pspec->pattern = g_utf8_strreverse(pspec->pattern, pspec->pattern_length);
      g_free(tmp);
  }
  return pspec;
}
void g_pattern_spec_free(GPatternSpec *pspec) {
  g_return_if_fail(pspec != NULL);
  g_free(pspec->pattern);
  g_free(pspec);
}
gboolean g_pattern_spec_equal(GPatternSpec *pspec1, GPatternSpec *pspec2) {
  g_return_val_if_fail(pspec1 != NULL, FALSE);
  g_return_val_if_fail(pspec2 != NULL, FALSE);
  return (pspec1->pattern_length == pspec2->pattern_length && pspec1->match_type == pspec2->match_type && strcmp(pspec1->pattern, pspec2->pattern) == 0);
}
gboolean g_pattern_match_string(GPatternSpec *pspec, const gchar  *string) {
  g_return_val_if_fail(pspec != NULL, FALSE);
  g_return_val_if_fail(string != NULL, FALSE);
  return g_pattern_match(pspec, strlen (string), string, NULL);
}
gboolean g_pattern_match_simple(const gchar *pattern, const gchar *string) {
  GPatternSpec *pspec;
  gboolean ergo;
  g_return_val_if_fail(pattern != NULL, FALSE);
  g_return_val_if_fail(string != NULL, FALSE);
  pspec = g_pattern_spec_new(pattern);
  ergo = g_pattern_match(pspec, strlen(string), string, NULL);
  g_pattern_spec_free(pspec);
  return ergo;
}