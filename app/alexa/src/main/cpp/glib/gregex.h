#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_REGEX_H__
#define __G_REGEX_H__

#include "gerror.h"
#include "gstring.h"

G_BEGIN_DECLS
typedef float gfloat;
typedef double gdouble;
typedef long glong;
typedef unsigned long gulong;
typedef enum {
  G_REGEX_ERROR_COMPILE,
  G_REGEX_ERROR_OPTIMIZE,
  G_REGEX_ERROR_REPLACE,
  G_REGEX_ERROR_MATCH,
  G_REGEX_ERROR_INTERNAL,
  G_REGEX_ERROR_STRAY_BACKSLASH = 101,
  G_REGEX_ERROR_MISSING_CONTROL_CHAR = 102,
  G_REGEX_ERROR_UNRECOGNIZED_ESCAPE = 103,
  G_REGEX_ERROR_QUANTIFIERS_OUT_OF_ORDER = 104,
  G_REGEX_ERROR_QUANTIFIER_TOO_BIG = 105,
  G_REGEX_ERROR_UNTERMINATED_CHARACTER_CLASS = 106,
  G_REGEX_ERROR_INVALID_ESCAPE_IN_CHARACTER_CLASS = 107,
  G_REGEX_ERROR_RANGE_OUT_OF_ORDER = 108,
  G_REGEX_ERROR_NOTHING_TO_REPEAT = 109,
  G_REGEX_ERROR_UNRECOGNIZED_CHARACTER = 112,
  G_REGEX_ERROR_POSIX_NAMED_CLASS_OUTSIDE_CLASS = 113,
  G_REGEX_ERROR_UNMATCHED_PARENTHESIS = 114,
  G_REGEX_ERROR_INEXISTENT_SUBPATTERN_REFERENCE = 115,
  G_REGEX_ERROR_UNTERMINATED_COMMENT = 118,
  G_REGEX_ERROR_EXPRESSION_TOO_LARGE = 120,
  G_REGEX_ERROR_MEMORY_ERROR = 121,
  G_REGEX_ERROR_VARIABLE_LENGTH_LOOKBEHIND = 125,
  G_REGEX_ERROR_MALFORMED_CONDITION = 126,
  G_REGEX_ERROR_TOO_MANY_CONDITIONAL_BRANCHES = 127,
  G_REGEX_ERROR_ASSERTION_EXPECTED = 128,
  G_REGEX_ERROR_UNKNOWN_POSIX_CLASS_NAME = 130,
  G_REGEX_ERROR_POSIX_COLLATING_ELEMENTS_NOT_SUPPORTED = 131,
  G_REGEX_ERROR_HEX_CODE_TOO_LARGE = 134,
  G_REGEX_ERROR_INVALID_CONDITION = 135,
  G_REGEX_ERROR_SINGLE_BYTE_MATCH_IN_LOOKBEHIND = 136,
  G_REGEX_ERROR_INFINITE_LOOP = 140,
  G_REGEX_ERROR_MISSING_SUBPATTERN_NAME_TERMINATOR = 142,
  G_REGEX_ERROR_DUPLICATE_SUBPATTERN_NAME = 143,
  G_REGEX_ERROR_MALFORMED_PROPERTY = 146,
  G_REGEX_ERROR_UNKNOWN_PROPERTY = 147,
  G_REGEX_ERROR_SUBPATTERN_NAME_TOO_LONG = 148,
  G_REGEX_ERROR_TOO_MANY_SUBPATTERNS = 149,
  G_REGEX_ERROR_INVALID_OCTAL_VALUE = 151,
  G_REGEX_ERROR_TOO_MANY_BRANCHES_IN_DEFINE = 154,
  G_REGEX_ERROR_DEFINE_REPETION = 155,
  G_REGEX_ERROR_INCONSISTENT_NEWLINE_OPTIONS = 156,
  G_REGEX_ERROR_MISSING_BACK_REFERENCE = 157
} GRegexError;
#define G_REGEX_ERROR g_regex_error_quark ()
GQuark g_regex_error_quark (void);
typedef enum {
  G_REGEX_CASELESS = 1 << 0,
  G_REGEX_MULTILINE = 1 << 1,
  G_REGEX_DOTALL = 1 << 2,
  G_REGEX_EXTENDED = 1 << 3,
  G_REGEX_ANCHORED = 1 << 4,
  G_REGEX_DOLLAR_ENDONLY = 1 << 5,
  G_REGEX_UNGREEDY = 1 << 9,
  G_REGEX_RAW = 1 << 11,
  G_REGEX_NO_AUTO_CAPTURE = 1 << 12,
  G_REGEX_OPTIMIZE = 1 << 13,
  G_REGEX_DUPNAMES = 1 << 19,
  G_REGEX_NEWLINE_CR = 1 << 20,
  G_REGEX_NEWLINE_LF = 1 << 21,
  G_REGEX_NEWLINE_CRLF = G_REGEX_NEWLINE_CR | G_REGEX_NEWLINE_LF
} GRegexCompileFlags;
typedef enum {
  G_REGEX_MATCH_ANCHORED = 1 << 4,
  G_REGEX_MATCH_NOTBOL = 1 << 7,
  G_REGEX_MATCH_NOTEOL = 1 << 8,
  G_REGEX_MATCH_NOTEMPTY = 1 << 10,
  G_REGEX_MATCH_PARTIAL = 1 << 15,
  G_REGEX_MATCH_NEWLINE_CR = 1 << 20,
  G_REGEX_MATCH_NEWLINE_LF = 1 << 21,
  G_REGEX_MATCH_NEWLINE_CRLF = G_REGEX_MATCH_NEWLINE_CR | G_REGEX_MATCH_NEWLINE_LF,
  G_REGEX_MATCH_NEWLINE_ANY = 1 << 22
} GRegexMatchFlags;
typedef struct _GRegex GRegex;
typedef struct _GMatchInfo GMatchInfo;
typedef int (*GRegexEvalCallback)(const GMatchInfo *match_info, GString *result, gpointer user_data);
GRegex *g_regex_new(const gchar *pattern, GRegexCompileFlags compile_options, GRegexMatchFlags match_options, GError **error);
GRegex *g_regex_ref(GRegex *regex);
void g_regex_unref(GRegex *regex);
const gchar *g_regex_get_pattern(const GRegex *regex);
gint g_regex_get_max_backref(const GRegex *regex);
gint g_regex_get_capture_count(const GRegex *regex);
gint g_regex_get_string_number(const GRegex *regex, const gchar *name);
gchar *g_regex_escape_string(const gchar *string, gint length);
GRegexCompileFlags g_regex_get_compile_flags(const GRegex *regex);
GRegexMatchFlags g_regex_get_match_flags(const GRegex *regex);
int g_regex_match_simple(const gchar *pattern, const gchar *string, GRegexCompileFlags compile_options, GRegexMatchFlags match_options);
int g_regex_match(const GRegex *regex, const gchar *string, GRegexMatchFlags match_options, GMatchInfo **match_info);
int g_regex_match_full(const GRegex *regex, const gchar *string, gssize string_len, gint start_position, GRegexMatchFlags match_options, GMatchInfo **match_info,
	  				   GError **error);
int g_regex_match_all(const GRegex *regex, const gchar *string, GRegexMatchFlags match_options, GMatchInfo **match_info);
int g_regex_match_all_full(const GRegex *regex, const gchar *string, gssize string_len, gint start_position, GRegexMatchFlags  match_options,
						   GMatchInfo **match_info, GError **error);
gchar **g_regex_split_simple(const gchar *pattern, const gchar *string, GRegexCompileFlags compile_options, GRegexMatchFlags match_options);
gchar **g_regex_split(const GRegex *regex, const gchar *string, GRegexMatchFlags match_options);
gchar **g_regex_split_full(const GRegex *regex, const gchar *string, gssize string_len, gint start_position, GRegexMatchFlags match_options, gint max_tokens,
						   GError **error);
gchar *g_regex_replace(const GRegex *regex, const gchar *string, gssize string_len, gint start_position, const gchar *replacement, GRegexMatchFlags match_options,
					   GError **error);
gchar *g_regex_replace_literal(const GRegex *regex, const gchar *string, gssize string_len, gint start_position, const gchar *replacement,
							   GRegexMatchFlags match_options, GError **error);
gchar *g_regex_replace_eval(const GRegex *regex, const gchar *string, gssize string_len, gint start_position, GRegexMatchFlags match_options,
							GRegexEvalCallback eval, gpointer user_data, GError **error);
int g_regex_check_replacement(const gchar *replacement, int *has_references, GError **error);
GRegex *g_match_info_get_regex(const GMatchInfo *match_info);
const gchar *g_match_info_get_string(const GMatchInfo *match_info);
void g_match_info_free(GMatchInfo *match_info);
int g_match_info_next(GMatchInfo *match_info, GError **error);
int g_match_info_matches(const GMatchInfo *match_info);
gint g_match_info_get_match_count(const GMatchInfo *match_info);
int g_match_info_is_partial_match(const GMatchInfo *match_info);
gchar *g_match_info_expand_references(const GMatchInfo *match_info, const gchar *string_to_expand, GError **error);
gchar *g_match_info_fetch(const GMatchInfo *match_info, gint match_num);
int g_match_info_fetch_pos(const GMatchInfo *match_info, gint match_num, gint *start_pos, gint *end_pos);
gchar *g_match_info_fetch_named(const GMatchInfo *match_info, const gchar *name);
int g_match_info_fetch_named_pos(const GMatchInfo *match_info, const gchar *name, gint *start_pos, gint *end_pos);
gchar **g_match_info_fetch_all(const GMatchInfo *match_info);
G_END_DECLS

#endif