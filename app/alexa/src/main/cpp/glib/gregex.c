#include <string.h>
#ifdef USE_SYSTEM_PCRE
#include <pcre.h>
#else
#include "pcre/pcre.h"
#endif
#include "gtypes.h"
#include "gregex.h"
#include "glibintl.h"
#include "glist.h"
#include "gmessages.h"
#include "gstrfuncs.h"
#include "gatomic.h"
#include "gi18n-lib.h"

#define G_REGEX_COMPILE_MASK (G_REGEX_CASELESS | G_REGEX_MULTILINE | G_REGEX_DOTALL | G_REGEX_EXTENDED | G_REGEX_ANCHORED | G_REGEX_DOLLAR_ENDONLY | \
                              G_REGEX_UNGREEDY | G_REGEX_RAW |  G_REGEX_NO_AUTO_CAPTURE | G_REGEX_OPTIMIZE | G_REGEX_DUPNAMES | G_REGEX_NEWLINE_CR | \
                              G_REGEX_NEWLINE_LF | G_REGEX_NEWLINE_CRLF)
#define G_REGEX_MATCH_MASK (G_REGEX_MATCH_ANCHORED | G_REGEX_MATCH_NOTBOL | G_REGEX_MATCH_NOTEOL | G_REGEX_MATCH_NOTEMPTY | G_REGEX_MATCH_PARTIAL | \
                            G_REGEX_MATCH_NEWLINE_CR | G_REGEX_MATCH_NEWLINE_LF | G_REGEX_MATCH_NEWLINE_CRLF | G_REGEX_MATCH_NEWLINE_ANY)
#define NEXT_CHAR(re, s) (((re)->compile_opts & PCRE_UTF8) ? g_utf8_next_char (s) : ((s) + 1))
#define PREV_CHAR(re, s) (((re)->compile_opts & PCRE_UTF8) ? g_utf8_prev_char (s) : ((s) - 1))
struct _GMatchInfo {
  GRegex *regex;
  GRegexMatchFlags match_opts;
  gint matches;
  gint pos;
  gint *offsets;
  gint n_offsets;
  gint *workspace;
  gint n_workspace;
  const gchar *string;
  gssize string_len;
};
struct _GRegex {
  volatile gint ref_count;
  gchar *pattern;
  pcre *pcre_re;
  GRegexCompileFlags compile_opts;
  GRegexMatchFlags match_opts;
  pcre_extra *extra;
};
#define IS_PCRE_ERROR(ret) ((ret) < PCRE_ERROR_NOMATCH && (ret) != PCRE_ERROR_PARTIAL)
typedef struct _InterpolationData InterpolationData;
static gboolean interpolation_list_needs_match(GList *list);
static gboolean interpolate_replacement(const GMatchInfo *match_info, GString *result, gpointer data);
static GList *split_replacement(const gchar *replacement, GError **error);
static void free_interpolation_data(InterpolationData *data);
static const gchar * match_error (gint errcode) {
  switch (errcode) {
      case PCRE_ERROR_NOMATCH: break;
      case PCRE_ERROR_NULL: g_warning ("A NULL argument was passed to PCRE"); break;
      case PCRE_ERROR_BADOPTION: return "bad options";
      case PCRE_ERROR_BADMAGIC: return _("corrupted object");
      case PCRE_ERROR_UNKNOWN_OPCODE: return N_("internal error or corrupted object");
      case PCRE_ERROR_NOMEMORY: return _("out of memory");
      case PCRE_ERROR_NOSUBSTRING: break;
      case PCRE_ERROR_MATCHLIMIT: return _("backtracking limit reached");
      case PCRE_ERROR_CALLOUT: break;
      case PCRE_ERROR_BADUTF8: case PCRE_ERROR_BADUTF8_OFFSET: break;
      case PCRE_ERROR_PARTIAL: break;
      case PCRE_ERROR_BADPARTIAL: return _("the pattern contains items not supported for partial matching");
      case PCRE_ERROR_INTERNAL: return _("internal error");
      case PCRE_ERROR_BADCOUNT: g_warning ("A negative ovecsize was passed to PCRE"); break;
      case PCRE_ERROR_DFA_UITEM: return _("the pattern contains items not supported for partial matching");
      case PCRE_ERROR_DFA_UCOND: return _("back references as conditions are not supported for partial matching");
      case PCRE_ERROR_DFA_UMLIMIT: break;
      case PCRE_ERROR_DFA_WSSIZE: break;
      case PCRE_ERROR_DFA_RECURSE: case PCRE_ERROR_RECURSIONLIMIT: return _("recursion limit reached");
      case PCRE_ERROR_NULLWSLIMIT: return _("workspace limit for empty substrings reached");
      case PCRE_ERROR_BADNEWLINE: return _("invalid combination of newline flags");
      case PCRE_ERROR_BADOFFSET: return _("bad offset");
      case PCRE_ERROR_SHORTUTF8: return _("short utf8");
      default: break;
  }
  return _("unknown error");
}
static void translate_compile_error(gint *errcode, const gchar **errmsg) {
  *errcode += 100;
  switch (*errcode) {
      case G_REGEX_ERROR_STRAY_BACKSLASH: *errmsg = _("\\ at end of pattern"); break;
      case G_REGEX_ERROR_MISSING_CONTROL_CHAR: *errmsg = _("\\c at end of pattern"); break;
      case G_REGEX_ERROR_UNRECOGNIZED_ESCAPE: *errmsg = _("unrecognized character follows \\"); break;
      case 137:
          *errcode = G_REGEX_ERROR_UNRECOGNIZED_ESCAPE;
          *errmsg = _("case-changing escapes (\\l, \\L, \\u, \\U) are not allowed here");
          break;
      case G_REGEX_ERROR_QUANTIFIERS_OUT_OF_ORDER: *errmsg = _("numbers out of order in {} quantifier"); break;
      case G_REGEX_ERROR_QUANTIFIER_TOO_BIG: *errmsg = _("number too big in {} quantifier"); break;
      case G_REGEX_ERROR_UNTERMINATED_CHARACTER_CLASS: *errmsg = _("missing terminating ] for character class"); break;
      case G_REGEX_ERROR_INVALID_ESCAPE_IN_CHARACTER_CLASS: *errmsg = _("invalid escape sequence in character class"); break;
      case G_REGEX_ERROR_RANGE_OUT_OF_ORDER: *errmsg = _("range out of order in character class"); break;
      case G_REGEX_ERROR_NOTHING_TO_REPEAT: *errmsg = _("nothing to repeat"); break;
      case G_REGEX_ERROR_UNRECOGNIZED_CHARACTER: *errmsg = _("unrecognized character after (?"); break;
      case 124:
          *errcode = G_REGEX_ERROR_UNRECOGNIZED_CHARACTER;
          *errmsg = _("unrecognized character after (?<");
          break;
      case 141:
          *errcode = G_REGEX_ERROR_UNRECOGNIZED_CHARACTER;
          *errmsg = _("unrecognized character after (?P");
          break;
      case G_REGEX_ERROR_POSIX_NAMED_CLASS_OUTSIDE_CLASS: *errmsg = _("POSIX named classes are supported only within a class"); break;
      case G_REGEX_ERROR_UNMATCHED_PARENTHESIS: *errmsg = _("missing terminating )"); break;
      case 122:
          *errcode = G_REGEX_ERROR_UNMATCHED_PARENTHESIS;
          *errmsg = _(") without opening (");
          break;
      case 129:
          *errcode = G_REGEX_ERROR_UNMATCHED_PARENTHESIS;
          *errmsg = _("(?R or (?[+-]digits must be followed by )");
          break;
      case G_REGEX_ERROR_INEXISTENT_SUBPATTERN_REFERENCE: *errmsg = _("reference to non-existent subpattern"); break;
      case G_REGEX_ERROR_UNTERMINATED_COMMENT: *errmsg = _("missing ) after comment"); break;
      case G_REGEX_ERROR_EXPRESSION_TOO_LARGE: *errmsg = _("regular expression too large"); break;
      case G_REGEX_ERROR_MEMORY_ERROR: *errmsg = _("failed to get memory"); break;
      case G_REGEX_ERROR_VARIABLE_LENGTH_LOOKBEHIND: *errmsg = _("lookbehind assertion is not fixed length"); break;
      case G_REGEX_ERROR_MALFORMED_CONDITION: *errmsg = _("malformed number or name after (?("); break;
      case G_REGEX_ERROR_TOO_MANY_CONDITIONAL_BRANCHES: *errmsg = _("conditional group contains more than two branches"); break;
      case G_REGEX_ERROR_ASSERTION_EXPECTED: *errmsg = _("assertion expected after (?("); break;
      case G_REGEX_ERROR_UNKNOWN_POSIX_CLASS_NAME: *errmsg = _("unknown POSIX class name"); break;
      case G_REGEX_ERROR_POSIX_COLLATING_ELEMENTS_NOT_SUPPORTED: *errmsg = _("POSIX collating elements are not supported"); break;
      case G_REGEX_ERROR_HEX_CODE_TOO_LARGE: *errmsg = _("character value in \\x{...} sequence is too large"); break;
      case G_REGEX_ERROR_INVALID_CONDITION: *errmsg = _("invalid condition (?(0)"); break;
      case G_REGEX_ERROR_SINGLE_BYTE_MATCH_IN_LOOKBEHIND: *errmsg = _("\\C not allowed in lookbehind assertion"); break;
      case G_REGEX_ERROR_INFINITE_LOOP: *errmsg = _("recursive call could loop indefinitely"); break;
      case G_REGEX_ERROR_MISSING_SUBPATTERN_NAME_TERMINATOR: *errmsg = _("missing terminator in subpattern name"); break;
      case G_REGEX_ERROR_DUPLICATE_SUBPATTERN_NAME: *errmsg = _("two named subpatterns have the same name"); break;
      case G_REGEX_ERROR_MALFORMED_PROPERTY: *errmsg = _("malformed \\P or \\p sequence"); break;
      case G_REGEX_ERROR_UNKNOWN_PROPERTY: *errmsg = _("unknown property name after \\P or \\p"); break;
      case G_REGEX_ERROR_SUBPATTERN_NAME_TOO_LONG: *errmsg = _("subpattern name is too long (maximum 32 characters)"); break;
      case G_REGEX_ERROR_TOO_MANY_SUBPATTERNS: *errmsg = _("too many named subpatterns (maximum 10,000)"); break;
      case G_REGEX_ERROR_INVALID_OCTAL_VALUE: *errmsg = _("octal value is greater than \\377"); break;
      case G_REGEX_ERROR_TOO_MANY_BRANCHES_IN_DEFINE: *errmsg = _("DEFINE group contains more than one branch"); break;
      case G_REGEX_ERROR_DEFINE_REPETION: *errmsg = _("repeating a DEFINE group is not allowed"); break;
      case G_REGEX_ERROR_INCONSISTENT_NEWLINE_OPTIONS: *errmsg = _("inconsistent NEWLINE options"); break;
      case G_REGEX_ERROR_MISSING_BACK_REFERENCE: *errmsg = _("\\g is not followed by a braced name or an optionally braced non-zero number"); break;
      case 11:
          *errcode = G_REGEX_ERROR_INTERNAL;
          *errmsg = _("unexpected repeat");
          break;
      case 23:
          *errcode = G_REGEX_ERROR_INTERNAL;
          *errmsg = _("code overflow");
          break;
      case 52:
          *errcode = G_REGEX_ERROR_INTERNAL;
          *errmsg = _("overran compiling workspace");
          break;
      case 53:
          *errcode = G_REGEX_ERROR_INTERNAL;
          *errmsg = _("previously-checked referenced subpattern not found");
          break;
      case 16:
          g_warning("erroffset passed as NULL");
          *errcode = G_REGEX_ERROR_COMPILE;
          break;
      case 17:
          g_warning("unknown option bit(s) set");
          *errcode = G_REGEX_ERROR_COMPILE;
          break;
      case 32: case 44: case 45:
          g_warning ("%s", *errmsg);
          *errcode = G_REGEX_ERROR_COMPILE;
          break;
      default: *errcode = G_REGEX_ERROR_COMPILE;
  }
}
static GMatchInfo* match_info_new(const GRegex *regex, const gchar *string, gint string_len, gint start_position, gint match_options, gboolean is_dfa) {
  GMatchInfo *match_info;
  if (string_len < 0) string_len = strlen(string);
  match_info = g_new0(GMatchInfo, 1);
  match_info->regex = g_regex_ref((GRegex *)regex);
  match_info->string = string;
  match_info->string_len = string_len;
  match_info->matches = PCRE_ERROR_NOMATCH;
  match_info->pos = start_position;
  match_info->match_opts = match_options;
  if (is_dfa) {
      match_info->n_offsets = 24;
      match_info->n_workspace = 100;
      match_info->workspace = g_new(gint, match_info->n_workspace);
  } else {
      gint capture_count;
      pcre_fullinfo(regex->pcre_re, regex->extra, PCRE_INFO_CAPTURECOUNT, &capture_count);
      match_info->n_offsets = (capture_count + 1) * 3;
  }
  match_info->offsets = g_new0(gint, match_info->n_offsets);
  match_info->offsets[0] = -1;
  match_info->offsets[1] = -1;
  return match_info;
}
GRegex* g_match_info_get_regex(const GMatchInfo *match_info) {
  g_return_val_if_fail(match_info != NULL, NULL);
  return match_info->regex;
}
const gchar* g_match_info_get_string(const GMatchInfo *match_info) {
  g_return_val_if_fail(match_info != NULL, NULL);
  return match_info->string;
}
void g_match_info_free(GMatchInfo *match_info) {
  if (match_info) {
      g_regex_unref(match_info->regex);
      g_free(match_info->offsets);
      g_free(match_info->workspace);
      g_free(match_info);
  }
}
gboolean
g_match_info_next (GMatchInfo *match_info, GError **error) {
  gint prev_match_start;
  gint prev_match_end;
  g_return_val_if_fail(match_info != NULL, FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail(match_info->pos >= 0, FALSE);
  prev_match_start = match_info->offsets[0];
  prev_match_end = match_info->offsets[1];
  if (match_info->pos > match_info->string_len) {
      match_info->pos = -1;
      match_info->matches = PCRE_ERROR_NOMATCH;
      return FALSE;
  }
  match_info->matches = pcre_exec(match_info->regex->pcre_re, match_info->regex->extra, match_info->string, match_info->string_len, match_info->pos,
                                  match_info->regex->match_opts | match_info->match_opts, match_info->offsets, match_info->n_offsets);
  if (IS_PCRE_ERROR(match_info->matches)) {
      g_set_error(error, G_REGEX_ERROR, G_REGEX_ERROR_MATCH, _("Error while matching regular expression %s: %s"),
                  match_info->regex->pattern, match_error (match_info->matches));
      return FALSE;
  }
  if (match_info->pos == match_info->offsets[1]) {
      if (match_info->pos > match_info->string_len) {
          match_info->pos = -1;
          match_info->matches = PCRE_ERROR_NOMATCH;
          return FALSE;
      }
      match_info->pos = NEXT_CHAR(match_info->regex,&match_info->string[match_info->pos]) - match_info->string;
  } else match_info->pos = match_info->offsets[1];
  if (match_info->matches >= 0 && prev_match_start == match_info->offsets[0] && prev_match_end == match_info->offsets[1]) {
      return g_match_info_next (match_info, error);
  }
  return match_info->matches >= 0;
}
gboolean g_match_info_matches(const GMatchInfo *match_info) {
  g_return_val_if_fail(match_info != NULL, FALSE);
  return match_info->matches >= 0;
}
gint g_match_info_get_match_count(const GMatchInfo *match_info) {
  g_return_val_if_fail(match_info, -1);
  if (match_info->matches == PCRE_ERROR_NOMATCH) return 0;
  else if (match_info->matches < PCRE_ERROR_NOMATCH) return -1;
  else return match_info->matches;
}
gboolean g_match_info_is_partial_match(const GMatchInfo *match_info) {
  g_return_val_if_fail(match_info != NULL, FALSE);
  return match_info->matches == PCRE_ERROR_PARTIAL;
}
gchar* g_match_info_expand_references(const GMatchInfo *match_info, const gchar *string_to_expand, GError **error) {
  GString *result;
  GList *list;
  GError *tmp_error = NULL;
  g_return_val_if_fail(string_to_expand != NULL, NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  list = split_replacement (string_to_expand, &tmp_error);
  if (tmp_error != NULL) {
      g_propagate_error(error, tmp_error);
      return NULL;
  }
  if (!match_info && interpolation_list_needs_match (list)) {
      g_critical("String '%s' contains references to the match, can't expand references without GMatchInfo object", string_to_expand);
      return NULL;
  }
  result = g_string_sized_new(strlen (string_to_expand));
  interpolate_replacement(match_info, result, list);
  g_list_foreach(list, (GFunc)free_interpolation_data, NULL);
  g_list_free(list);
  return g_string_free(result, FALSE);
}
gchar* g_match_info_fetch(const GMatchInfo *match_info, gint match_num) {
  gchar *match = NULL;
  gint start, end;
  g_return_val_if_fail(match_info != NULL, NULL);
  g_return_val_if_fail(match_num >= 0, NULL);
  if (!g_match_info_fetch_pos(match_info, match_num, &start, &end)) match = NULL;
  else if (start == -1) match = g_strdup("");
  else match = g_strndup(&match_info->string[start], end - start);
  return match;
}
gboolean g_match_info_fetch_pos(const GMatchInfo *match_info, gint match_num, gint *start_pos, gint *end_pos) {
  g_return_val_if_fail(match_info != NULL, FALSE);
  g_return_val_if_fail(match_num >= 0, FALSE);
  if (match_num >= match_info->matches) return FALSE;
  if (start_pos != NULL) *start_pos = match_info->offsets[2 * match_num];
  if (end_pos != NULL) *end_pos = match_info->offsets[2 * match_num + 1];
  return TRUE;
}
static gint get_matched_substring_number(const GMatchInfo *match_info, const gchar *name) {
  gint entrysize;
  gchar *first, *last;
  guchar *entry;
  if (!(match_info->regex->compile_opts & G_REGEX_DUPNAMES)) return pcre_get_stringnumber(match_info->regex->pcre_re, name);
  entrysize = pcre_get_stringtable_entries(match_info->regex->pcre_re, name, &first, &last);
  if (entrysize <= 0) return entrysize;
  for (entry = (guchar*)first; entry <= (guchar*)last; entry += entrysize) {
      gint n = (entry[0] << 8) + entry[1];
      if (match_info->offsets[n*2] >= 0) return n;
  }
  return (first[0] << 8) + first[1];
}
gchar* g_match_info_fetch_named(const GMatchInfo *match_info, const gchar *name) {
  gint num;
  g_return_val_if_fail(match_info != NULL, NULL);
  g_return_val_if_fail(name != NULL, NULL);
  num = get_matched_substring_number(match_info, name);
  if (num < 0) return NULL;
  else return g_match_info_fetch(match_info, num);
}
gboolean g_match_info_fetch_named_pos(const GMatchInfo *match_info, const gchar *name, gint *start_pos, gint *end_pos) {
  gint num;
  g_return_val_if_fail(match_info != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);
  num = get_matched_substring_number(match_info, name);
  if (num < 0) return FALSE;
  return g_match_info_fetch_pos(match_info, num, start_pos, end_pos);
}
gchar** g_match_info_fetch_all(const GMatchInfo *match_info) {
  gchar **result;
  gint i;
  g_return_val_if_fail(match_info != NULL, NULL);
  if (match_info->matches < 0) return NULL;
  result = g_new(gchar *, match_info->matches + 1);
  for (i = 0; i < match_info->matches; i++) result[i] = g_match_info_fetch(match_info, i);
  result[i] = NULL;
  return result;
}
GQuark g_regex_error_quark(void) {
  static GQuark error_quark = 0;
  if (error_quark == 0) error_quark = g_quark_from_static_string("g-regex-error-quark");
  return error_quark;
}
GRegex* g_regex_ref(GRegex *regex) {
  g_return_val_if_fail(regex != NULL, NULL);
  g_atomic_int_inc(&regex->ref_count);
  return regex;
}
void g_regex_unref(GRegex *regex) {
  g_return_if_fail (regex != NULL);
  if (g_atomic_int_exchange_and_add(&regex->ref_count, -1) - 1 == 0) {
      g_free(regex->pattern);
      if (regex->pcre_re != NULL) pcre_free(regex->pcre_re);
      if (regex->extra != NULL) pcre_free(regex->extra);
      g_free(regex);
  }
}
GRegex* g_regex_new(const gchar *pattern, GRegexCompileFlags compile_options, GRegexMatchFlags match_options, GError **error) {
  GRegex *regex;
  pcre *re;
  const gchar *errmsg;
  gint erroffset;
  gint errcode;
  gboolean optimize = FALSE;
  static gboolean initialized = FALSE;
  unsigned long int pcre_compile_options;
  g_return_val_if_fail(pattern != NULL, NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  g_return_val_if_fail((compile_options & ~G_REGEX_COMPILE_MASK) == 0, NULL);
  g_return_val_if_fail((match_options & ~G_REGEX_MATCH_MASK) == 0, NULL);
  if (!initialized) {
      gint support;
      const gchar *msg;
      pcre_config (PCRE_CONFIG_UTF8, &support);
      if (!support) {
          msg = N_("PCRE library is compiled without UTF8 support");
          g_critical("%s", msg);
          g_set_error_literal(error, G_REGEX_ERROR, G_REGEX_ERROR_COMPILE, gettext(msg));
          return NULL;
      }
      pcre_config(PCRE_CONFIG_UNICODE_PROPERTIES, &support);
      if (!support) {
          msg = N_("PCRE library is compiled without UTF8 properties support");
          g_critical("%s", msg);
          g_set_error_literal(error, G_REGEX_ERROR, G_REGEX_ERROR_COMPILE, gettext(msg));
          return NULL;
      }
      initialized = TRUE;
  }
  if (compile_options & G_REGEX_OPTIMIZE) optimize = TRUE;
  if (compile_options & G_REGEX_RAW) compile_options &= ~G_REGEX_RAW;
  else {
      compile_options |= PCRE_UTF8 | PCRE_NO_UTF8_CHECK;
      match_options |= PCRE_NO_UTF8_CHECK;
  }
  if (!(compile_options & G_REGEX_NEWLINE_CR) && !(compile_options & G_REGEX_NEWLINE_LF)) compile_options |= PCRE_NEWLINE_ANY;
  compile_options |= PCRE_UCP;
  re = pcre_compile2(pattern, compile_options, &errcode, &errmsg, &erroffset, NULL);
  if (re == NULL) {
      GError *tmp_error;
      translate_compile_error(&errcode, &errmsg);
      erroffset = g_utf8_pointer_to_offset(pattern, &pattern[erroffset]);
      tmp_error = g_error_new(G_REGEX_ERROR, errcode, _("Error while compiling regular expression %s at char %d: %s"), pattern, erroffset, errmsg);
      g_propagate_error(error, tmp_error);
      return NULL;
  }
  pcre_fullinfo(re, NULL, PCRE_INFO_OPTIONS, &pcre_compile_options);
  compile_options = pcre_compile_options;
  if (!(compile_options & G_REGEX_DUPNAMES)) {
      gboolean jchanged = FALSE;
      pcre_fullinfo(re, NULL, PCRE_INFO_JCHANGED, &jchanged);
      if (jchanged) compile_options |= G_REGEX_DUPNAMES;
  }
  regex = g_new0 (GRegex, 1);
  regex->ref_count = 1;
  regex->pattern = g_strdup (pattern);
  regex->pcre_re = re;
  regex->compile_opts = compile_options;
  regex->match_opts = match_options;
  if (optimize) {
      regex->extra = pcre_study (regex->pcre_re, 0, &errmsg);
      if (errmsg != NULL) {
          GError *tmp_error = g_error_new(G_REGEX_ERROR, G_REGEX_ERROR_OPTIMIZE, _("Error while optimizing regular expression %s: %s"), regex->pattern,
                                          errmsg);
          g_propagate_error(error, tmp_error);
          g_regex_unref(regex);
          return NULL;
      }
  }
  return regex;
}
const gchar* g_regex_get_pattern(const GRegex *regex) {
  g_return_val_if_fail(regex != NULL, NULL);
  return regex->pattern;
}
gint g_regex_get_max_backref(const GRegex *regex) {
  gint value;
  pcre_fullinfo(regex->pcre_re, regex->extra, PCRE_INFO_BACKREFMAX, &value);
  return value;
}
gint g_regex_get_capture_count(const GRegex *regex) {
  gint value;
  pcre_fullinfo(regex->pcre_re, regex->extra, PCRE_INFO_CAPTURECOUNT, &value);
  return value;
}
GRegexCompileFlags g_regex_get_compile_flags(const GRegex *regex) {
  g_return_val_if_fail(regex != NULL, 0);
  return regex->compile_opts;
}
GRegexMatchFlags g_regex_get_match_flags(const GRegex *regex) {
  g_return_val_if_fail(regex != NULL, 0);
  return regex->match_opts;
}
gboolean g_regex_match_simple(const gchar *pattern, const gchar *string, GRegexCompileFlags compile_options, GRegexMatchFlags match_options) {
  GRegex *regex;
  gboolean result;
  regex = g_regex_new(pattern, compile_options, 0, NULL);
  if (!regex) return FALSE;
  result = g_regex_match_full(regex, string, -1, 0, match_options, NULL, NULL);
  g_regex_unref(regex);
  return result;
}
gboolean g_regex_match(const GRegex *regex, const gchar *string, GRegexMatchFlags match_options, GMatchInfo **match_info) {
  return g_regex_match_full(regex, string, -1, 0, match_options, match_info, NULL);
}
gboolean g_regex_match_full(const GRegex *regex, const gchar *string,gssize string_len, gint start_position, GRegexMatchFlags match_options,
                            GMatchInfo **match_info,  GError **error) {
  GMatchInfo *info;
  gboolean match_ok;
  g_return_val_if_fail(regex != NULL, FALSE);
  g_return_val_if_fail(string != NULL, FALSE);
  g_return_val_if_fail(start_position >= 0, FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail((match_options & ~G_REGEX_MATCH_MASK) == 0, FALSE);
  info = match_info_new(regex, string, string_len, start_position, match_options, FALSE);
  match_ok = g_match_info_next(info, error);
  if (match_info != NULL) *match_info = info;
  else g_match_info_free (info);
  return match_ok;
}
gboolean g_regex_match_all(const GRegex *regex, const gchar *string, GRegexMatchFlags match_options, GMatchInfo **match_info) {
  return g_regex_match_all_full(regex, string, -1, 0, match_options, match_info, NULL);
}
gboolean g_regex_match_all_full(const GRegex *regex, const gchar *string, gssize string_len, gint start_position, GRegexMatchFlags match_options,
                                GMatchInfo **match_info, GError **error) {
  GMatchInfo *info;
  gboolean done;
  g_return_val_if_fail(regex != NULL, FALSE);
  g_return_val_if_fail(string != NULL, FALSE);
  g_return_val_if_fail(start_position >= 0, FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail((match_options & ~G_REGEX_MATCH_MASK) == 0, FALSE);
  info = match_info_new(regex, string, string_len, start_position, match_options, TRUE);
  done = FALSE;
  while (!done) {
      done = TRUE;
      info->matches = pcre_dfa_exec(regex->pcre_re, regex->extra, info->string, info->string_len, info->pos, regex->match_opts | match_options,
                                    info->offsets, info->n_offsets, info->workspace, info->n_workspace);
      if (info->matches == PCRE_ERROR_DFA_WSSIZE) {
          info->n_workspace *= 2;
          info->workspace = g_realloc (info->workspace,info->n_workspace * sizeof (gint));
          done = FALSE;
      } else if (info->matches == 0) {
          info->n_offsets *= 2;
          info->offsets = g_realloc(info->offsets,info->n_offsets * sizeof (gint));
          done = FALSE;
      } else if (IS_PCRE_ERROR (info->matches)) {
          g_set_error(error, G_REGEX_ERROR, G_REGEX_ERROR_MATCH, _("Error while matching regular expression %s: %s"),regex->pattern,
                      match_error (info->matches));
      }
  }
  info->pos = -1;
  if (match_info != NULL) *match_info = info;
  else g_match_info_free (info);
  return info->matches >= 0;
}
gint g_regex_get_string_number(const GRegex *regex, const gchar *name) {
  gint num;
  g_return_val_if_fail(regex != NULL, -1);
  g_return_val_if_fail(name != NULL, -1);
  num = pcre_get_stringnumber(regex->pcre_re, name);
  if (num == PCRE_ERROR_NOSUBSTRING) num = -1;
  return num;
}
gchar** g_regex_split_simple(const gchar *pattern, const gchar *string, GRegexCompileFlags compile_options, GRegexMatchFlags match_options) {
  GRegex *regex;
  gchar **result;
  regex = g_regex_new(pattern, compile_options, 0, NULL);
  if (!regex) return NULL;
  result = g_regex_split_full(regex, string, -1, 0, match_options, 0, NULL);
  g_regex_unref(regex);
  return result;
}
gchar** g_regex_split(const GRegex *regex, const gchar *string, GRegexMatchFlags match_options) {
  return g_regex_split_full(regex, string, -1, 0, match_options, 0, NULL);
}
gchar** g_regex_split_full(const GRegex *regex, const gchar *string, gssize string_len, gint start_position, GRegexMatchFlags match_options, gint max_tokens,
                           GError **error) {
  GError *tmp_error = NULL;
  GMatchInfo *match_info;
  GList *list, *last;
  gint i;
  gint token_count;
  gboolean match_ok;
  gint last_separator_end;
  gboolean last_match_is_empty;
  gchar **string_list;
  g_return_val_if_fail(regex != NULL, NULL);
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(start_position >= 0, NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  g_return_val_if_fail((match_options & ~G_REGEX_MATCH_MASK) == 0, NULL);
  if (max_tokens <= 0) max_tokens = G_MAXINT;
  if (string_len < 0) string_len = strlen(string);
  if (string_len - start_position == 0) return g_new0(gchar *, 1);
  if (max_tokens == 1) {
      string_list = g_new0(gchar *, 2);
      string_list[0] = g_strndup(&string[start_position],string_len - start_position);
      return string_list;
  }
  list = NULL;
  token_count = 0;
  last_separator_end = start_position;
  last_match_is_empty = FALSE;
  match_ok = g_regex_match_full(regex, string, string_len, start_position, match_options, &match_info, &tmp_error);
  while (tmp_error == NULL) {
      if (match_ok) {
          last_match_is_empty = (match_info->offsets[0] == match_info->offsets[1]);
          if (last_separator_end != match_info->offsets[1]) {
              gchar *token;
              gint match_count;
              token = g_strndup(string + last_separator_end,match_info->offsets[0] - last_separator_end);
              list = g_list_prepend(list, token);
              token_count++;
              match_count = g_match_info_get_match_count (match_info);
              if (match_count > 1) {
                  for (i = 1; i < match_count; i++) list = g_list_prepend(list, g_match_info_fetch (match_info, i));
              }
          }
      } else {
          if (!last_match_is_empty) {
              gchar *token = g_strndup(string + last_separator_end,match_info->string_len - last_separator_end);
              list = g_list_prepend (list, token);
          }
          break;
      }
      if (token_count >= max_tokens - 1) {
          if (last_match_is_empty) match_info->pos = PREV_CHAR (regex, &string[match_info->pos]) - string;
          if (string_len > match_info->pos) {
              gchar *token = g_strndup (string + match_info->pos,string_len - match_info->pos);
              list = g_list_prepend (list, token);
          }
          break;
      }
      last_separator_end = match_info->pos;
      if (last_match_is_empty) last_separator_end = PREV_CHAR(regex, &string[last_separator_end]) - string;
      match_ok = g_match_info_next (match_info, &tmp_error);
  }
  g_match_info_free (match_info);
  if (tmp_error != NULL) {
      g_propagate_error (error, tmp_error);
      g_list_foreach (list, (GFunc)g_free, NULL);
      g_list_free (list);
      match_info->pos = -1;
      return NULL;
  }
  string_list = g_new(gchar*, g_list_length(list) + 1);
  i = 0;
  for (last = g_list_last (list); last; last = g_list_previous(last)) string_list[i++] = last->data;
  string_list[i] = NULL;
  g_list_free(list);
  return string_list;
}
enum {
  REPL_TYPE_STRING,
  REPL_TYPE_CHARACTER,
  REPL_TYPE_SYMBOLIC_REFERENCE,
  REPL_TYPE_NUMERIC_REFERENCE,
  REPL_TYPE_CHANGE_CASE
};
typedef enum {
  CHANGE_CASE_NONE = 1 << 0,
  CHANGE_CASE_UPPER = 1 << 1,
  CHANGE_CASE_LOWER = 1 << 2,
  CHANGE_CASE_UPPER_SINGLE = 1 << 3,
  CHANGE_CASE_LOWER_SINGLE = 1 << 4,
  CHANGE_CASE_SINGLE_MASK = CHANGE_CASE_UPPER_SINGLE | CHANGE_CASE_LOWER_SINGLE,
  CHANGE_CASE_LOWER_MASK = CHANGE_CASE_LOWER | CHANGE_CASE_LOWER_SINGLE,
  CHANGE_CASE_UPPER_MASK = CHANGE_CASE_UPPER | CHANGE_CASE_UPPER_SINGLE
} ChangeCase;
struct _InterpolationData {
  gchar *text;
  gint type;
  gint num;
  gchar c;
  ChangeCase change_case;
};
static void free_interpolation_data(InterpolationData *data) {
  g_free(data->text);
  g_free(data);
}
static const gchar* expand_escape(const gchar *replacement, const gchar *p, InterpolationData *data, GError **error) {
  const gchar *q, *r;
  gint x, d, h, i;
  const gchar *error_detail;
  gint base = 0;
  GError *tmp_error = NULL;
  p++;
  switch(*p) {
      case 't':
          p++;
          data->c = '\t';
          data->type = REPL_TYPE_CHARACTER;
          break;
      case 'n':
          p++;
          data->c = '\n';
          data->type = REPL_TYPE_CHARACTER;
          break;
      case 'v':
          p++;
          data->c = '\v';
          data->type = REPL_TYPE_CHARACTER;
          break;
      case 'r':
          p++;
          data->c = '\r';
          data->type = REPL_TYPE_CHARACTER;
          break;
      case 'f':
          p++;
          data->c = '\f';
          data->type = REPL_TYPE_CHARACTER;
          break;
      case 'a':
          p++;
          data->c = '\a';
          data->type = REPL_TYPE_CHARACTER;
          break;
      case 'b':
          p++;
          data->c = '\b';
          data->type = REPL_TYPE_CHARACTER;
          break;
      case '\\':
          p++;
          data->c = '\\';
          data->type = REPL_TYPE_CHARACTER;
          break;
      case 'x':
          p++;
          x = 0;
          if (*p == '{') {
              p++;
              do {
                  h = g_ascii_xdigit_value (*p);
                  if (h < 0) {
                      error_detail = _("hexadecimal digit or '}' expected");
                      goto error;
                  }
                  x = x * 16 + h;
                  p++;
              } while(*p != '}');
              p++;
          } else {
              for (i = 0; i < 2; i++) {
                  h = g_ascii_xdigit_value (*p);
                  if (h < 0) {
                      error_detail = _("hexadecimal digit expected");
                      goto error;
                  }
                  x = x * 16 + h;
                  p++;
              }
          }
          data->type = REPL_TYPE_STRING;
          data->text = g_new0(gchar, 8);
          g_unichar_to_utf8(x, data->text);
          break;
      case 'l':
          p++;
          data->type = REPL_TYPE_CHANGE_CASE;
          data->change_case = CHANGE_CASE_LOWER_SINGLE;
          break;
      case 'u':
          p++;
          data->type = REPL_TYPE_CHANGE_CASE;
          data->change_case = CHANGE_CASE_UPPER_SINGLE;
          break;
      case 'L':
          p++;
          data->type = REPL_TYPE_CHANGE_CASE;
          data->change_case = CHANGE_CASE_LOWER;
          break;
      case 'U':
          p++;
          data->type = REPL_TYPE_CHANGE_CASE;
          data->change_case = CHANGE_CASE_UPPER;
          break;
      case 'E':
          p++;
          data->type = REPL_TYPE_CHANGE_CASE;
          data->change_case = CHANGE_CASE_NONE;
          break;
      case 'g':
          p++;
          if (*p != '<') {
              error_detail = _("missing '<' in symbolic reference");
              goto error;
          }
          q = p + 1;
          do {
              p++;
              if (!*p) {
                  error_detail = _("unfinished symbolic reference");
                  goto error;
              }
          } while(*p != '>');
          if (p - q == 0) {
              error_detail = _("zero-length symbolic reference");
              goto error;
          }
          if (g_ascii_isdigit(*q)) {
              x = 0;
              do {
                  h = g_ascii_digit_value (*q);
                  if (h < 0) {
                      error_detail = _("digit expected");
                      p = q;
                      goto error;
                  }
                  x = x * 10 + h;
                  q++;
              } while(q != p);
              data->num = x;
              data->type = REPL_TYPE_NUMERIC_REFERENCE;
          } else {
              r = q;
              do {
                  if (!g_ascii_isalnum(*r)){
                      error_detail = _("illegal symbolic reference");
                      p = r;
                      goto error;
                  }
                  r++;
              } while(r != p);
              data->text = g_strndup(q, p - q);
              data->type = REPL_TYPE_SYMBOLIC_REFERENCE;
          }
          p++;
          break;
      case '0':
          if (g_ascii_digit_value (*g_utf8_next_char (p)) >= 0) {
              base = 8;
              p = g_utf8_next_char (p);
          }
      case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
          x = 0;
          d = 0;
          for (i = 0; i < 3; i++) {
              h = g_ascii_digit_value (*p);
              if (h < 0) break;
              if (h > 7) {
                  if (base == 8) break;
                  else base = 10;
              }
              if (i == 2 && base == 10) break;
              x = x * 8 + h;
              d = d * 10 + h;
              p++;
          }
          if (base == 8 || i == 3) {
              data->type = REPL_TYPE_STRING;
              data->text = g_new0(gchar, 8);
              g_unichar_to_utf8(x, data->text);
          } else {
              data->type = REPL_TYPE_NUMERIC_REFERENCE;
              data->num = d;
          }
          break;
      case 0:
          error_detail = _("stray final '\\'");
          goto error;
          break;
      default:
          error_detail = _("unknown escape sequence");
          goto error;
  }
  return p;
   error:
  tmp_error = g_error_new(G_REGEX_ERROR,G_REGEX_ERROR_REPLACE, _("Error while parsing replacement text \"%s\" at char %lu: %s"), replacement,
                          (gulong)(p - replacement), error_detail);
  g_propagate_error(error, tmp_error);
  return NULL;
}
static GList* split_replacement(const gchar *replacement, GError **error) {
  GList *list = NULL;
  InterpolationData *data;
  const gchar *p, *start;
  start = p = replacement;
  while(*p) {
      if (*p == '\\') {
          data = g_new0(InterpolationData, 1);
          start = p = expand_escape(replacement, p, data, error);
          if (p == NULL) {
              g_list_foreach(list, (GFunc)free_interpolation_data, NULL);
              g_list_free(list);
              free_interpolation_data(data);
              return NULL;
          }
          list = g_list_prepend(list, data);
      } else {
          p++;
          if (*p == '\\' || *p == '\0') {
              if (p - start > 0) {
                  data = g_new0(InterpolationData, 1);
                  data->text = g_strndup(start, p - start);
                  data->type = REPL_TYPE_STRING;
                  list = g_list_prepend(list, data);
              }
          }
      }
  }
  return g_list_reverse(list);
}
#define CHANGE_CASE(c, change_case)  (((change_case) & CHANGE_CASE_LOWER_MASK) ? g_unichar_tolower (c) : g_unichar_toupper (c))
static void string_append(GString *string, const gchar *text, ChangeCase *change_case) {
  gunichar c;
  if (text[0] == '\0') return;
  if (*change_case == CHANGE_CASE_NONE) g_string_append(string, text);
  else if (*change_case & CHANGE_CASE_SINGLE_MASK) {
      c = g_utf8_get_char(text);
      g_string_append_unichar(string, CHANGE_CASE(c, *change_case));
      g_string_append(string, g_utf8_next_char(text));
      *change_case = CHANGE_CASE_NONE;
  } else {
      while(*text != '\0') {
          c = g_utf8_get_char(text);
          g_string_append_unichar(string, CHANGE_CASE(c, *change_case));
          text = g_utf8_next_char(text);
      }
  }
}
static gboolean interpolate_replacement(const GMatchInfo *match_info, GString *result, gpointer data) {
  GList *list;
  InterpolationData *idata;
  gchar *match;
  ChangeCase change_case = CHANGE_CASE_NONE;
  for (list = data; list; list = list->next) {
      idata = list->data;
      switch (idata->type) {
          case REPL_TYPE_STRING: string_append(result, idata->text, &change_case); break;
          case REPL_TYPE_CHARACTER:
              g_string_append_c(result, CHANGE_CASE (idata->c, change_case));
              if (change_case & CHANGE_CASE_SINGLE_MASK) change_case = CHANGE_CASE_NONE;
              break;
          case REPL_TYPE_NUMERIC_REFERENCE:
              match = g_match_info_fetch(match_info, idata->num);
              if (match) {
                  string_append(result, match, &change_case);
                  g_free(match);
              }
              break;
          case REPL_TYPE_SYMBOLIC_REFERENCE:
              match = g_match_info_fetch_named(match_info, idata->text);
              if (match) {
                  string_append(result, match, &change_case);
                  g_free(match);
              }
              break;
          case REPL_TYPE_CHANGE_CASE: change_case = idata->change_case; break;
      }
  }
  return FALSE;
}
static gboolean interpolation_list_needs_match(GList *list) {
  while (list != NULL) {
      InterpolationData *data = list->data;
      if (data->type == REPL_TYPE_SYMBOLIC_REFERENCE || data->type == REPL_TYPE_NUMERIC_REFERENCE)  return TRUE;
      list = list->next;
  }
  return FALSE;
}
gchar* g_regex_replace(const GRegex *regex, const gchar *string,gssize string_len, gint start_position, const gchar *replacement, GRegexMatchFlags match_options,
                       GError **error) {
  gchar *result;
  GList *list;
  GError *tmp_error = NULL;
  g_return_val_if_fail(regex != NULL, NULL);
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(start_position >= 0, NULL);
  g_return_val_if_fail(replacement != NULL, NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  g_return_val_if_fail((match_options & ~G_REGEX_MATCH_MASK) == 0, NULL);
  list = split_replacement(replacement, &tmp_error);
  if (tmp_error != NULL) {
      g_propagate_error(error, tmp_error);
      return NULL;
  }
  result = g_regex_replace_eval(regex, string, string_len, start_position, match_options, interpolate_replacement, (gpointer)list, &tmp_error);
  if (tmp_error != NULL) g_propagate_error(error, tmp_error);
  g_list_foreach(list, (GFunc)free_interpolation_data, NULL);
  g_list_free(list);
  return result;
}
static gboolean literal_replacement(const GMatchInfo *match_info, GString *result, gpointer data) {
  g_string_append(result, data);
  return FALSE;
}
gchar* g_regex_replace_literal(const GRegex *regex, const gchar *string, gssize string_len, gint start_position, const gchar *replacement,
                               GRegexMatchFlags match_options, GError **error) {
  g_return_val_if_fail(replacement != NULL, NULL);
  g_return_val_if_fail((match_options & ~G_REGEX_MATCH_MASK) == 0, NULL);
  return g_regex_replace_eval(regex, string, string_len, start_position, match_options, literal_replacement, (gpointer)replacement, error);
}
gchar* g_regex_replace_eval(const GRegex *regex, const gchar *string, gssize string_len, gint start_position, GRegexMatchFlags match_options,
                            GRegexEvalCallback eval, gpointer user_data, GError **error) {
  GMatchInfo *match_info;
  GString *result;
  gint str_pos = 0;
  gboolean done = FALSE;
  GError *tmp_error = NULL;
  g_return_val_if_fail(regex != NULL, NULL);
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(start_position >= 0, NULL);
  g_return_val_if_fail(eval != NULL, NULL);
  g_return_val_if_fail((match_options & ~G_REGEX_MATCH_MASK) == 0, NULL);
  if (string_len < 0) string_len = strlen(string);
  result = g_string_sized_new(string_len);
  g_regex_match_full(regex, string, string_len, start_position, match_options, &match_info, &tmp_error);
  while (!done && g_match_info_matches(match_info)) {
      g_string_append_len(result,string + str_pos,match_info->offsets[0] - str_pos);
      done = (*eval)(match_info, result, user_data);
      str_pos = match_info->offsets[1];
      g_match_info_next(match_info, &tmp_error);
  }
  g_match_info_free (match_info);
  if (tmp_error != NULL) {
      g_propagate_error(error, tmp_error);
      g_string_free(result, TRUE);
      return NULL;
  }
  g_string_append_len(result, string + str_pos, string_len - str_pos);
  return g_string_free(result, FALSE);
}
gboolean g_regex_check_replacement(const gchar *replacement, gboolean *has_references, GError **error) {
  GList *list;
  GError *tmp = NULL;
  list = split_replacement(replacement, &tmp);
  if (tmp) {
      g_propagate_error(error, tmp);
      return FALSE;
  }
  if (has_references) *has_references = interpolation_list_needs_match(list);
  g_list_foreach (list, (GFunc)free_interpolation_data, NULL);
  g_list_free(list);
  return TRUE;
}
gchar* g_regex_escape_string(const gchar *string, gint length) {
  GString *escaped;
  const char *p, *piece_start, *end;
  g_return_val_if_fail(string != NULL, NULL);
  if (length < 0) length = strlen(string);
  end = string + length;
  p = piece_start = string;
  escaped = g_string_sized_new(length + 1);
  while(p < end) {
      switch(*p) {
          case '\0': case '\\': case '|': case '(': case ')': case '[': case ']': case '{': case '}': case '^': case '$': case '*': case '+': case '?': case '.':
              if (p != piece_start) g_string_append_len(escaped, piece_start, p - piece_start);
              g_string_append_c(escaped, '\\');
              if (*p == '\0') g_string_append_c(escaped, '0');
              else g_string_append_c(escaped, *p);
              piece_start = ++p;
              break;
          default: p = g_utf8_next_char(p); break;
      }
  }
  if (piece_start < end) g_string_append_len(escaped, piece_start, end - piece_start);
  return g_string_free(escaped, FALSE);
}