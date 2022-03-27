#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "gmarkup.h"
#include "galloca.h"
#include "gstrfuncs.h"
#include "gstring.h"
#include "gtestutils.h"
#include "glibintl.h"
#include "gunicode.h"

GQuark g_markup_error_quark(void) {
  return g_quark_from_static_string ("g-markup-error-quark");
}
typedef enum {
  STATE_START,
  STATE_AFTER_OPEN_ANGLE,
  STATE_AFTER_CLOSE_ANGLE,
  STATE_AFTER_ELISION_SLASH,
  STATE_INSIDE_OPEN_TAG_NAME,
  STATE_INSIDE_ATTRIBUTE_NAME,
  STATE_AFTER_ATTRIBUTE_NAME,
  STATE_BETWEEN_ATTRIBUTES,
  STATE_AFTER_ATTRIBUTE_EQUALS_SIGN,
  STATE_INSIDE_ATTRIBUTE_VALUE_SQ,
  STATE_INSIDE_ATTRIBUTE_VALUE_DQ,
  STATE_INSIDE_TEXT,
  STATE_AFTER_CLOSE_TAG_SLASH,
  STATE_INSIDE_CLOSE_TAG_NAME,
  STATE_AFTER_CLOSE_TAG_NAME,
  STATE_INSIDE_PASSTHROUGH,
  STATE_ERROR
} GMarkupParseState;
typedef struct {
  const char *prev_element;
  const GMarkupParser *prev_parser;
  gpointer prev_user_data;
} GMarkupRecursionTracker;
struct _GMarkupParseContext {
  const GMarkupParser *parser;
  GMarkupParseFlags flags;
  gint line_number;
  gint char_number;
  gpointer user_data;
  GDestroyNotify dnotify;
  GString *partial_chunk;
  GSList *spare_chunks;
  GMarkupParseState state;
  GSList *tag_stack;
  GSList *tag_stack_gstr;
  GSList *spare_list_nodes;
  GString **attr_names;
  GString **attr_values;
  gint cur_attr;
  gint alloc_attrs;
  const gchar *current_text;
  gssize current_text_len;
  const gchar *current_text_end;
  const gchar *start;
  const gchar *iter;
  guint document_empty : 1;
  guint parsing : 1;
  guint awaiting_pop : 1;
  gint balance;
  GSList *subparser_stack;
  const char *subparser_element;
  gpointer held_user_data;
};
static GSList* get_list_node(GMarkupParseContext *context, gpointer data) {
  GSList *node;
  if (context->spare_list_nodes != NULL) {
      node = context->spare_list_nodes;
      context->spare_list_nodes = g_slist_remove_link(context->spare_list_nodes, node);
  } else node = g_slist_alloc();
  node->data = data;
  return node;
}
static void free_list_node(GMarkupParseContext *context, GSList *node) {
  node->data = NULL;
  context->spare_list_nodes = g_slist_concat(node, context->spare_list_nodes);
}
static inline void string_blank(GString *string) {
  string->str[0] = '\0';
  string->len = 0;
}
GMarkupParseContext* g_markup_parse_context_new(const GMarkupParser *parser, GMarkupParseFlags flags, gpointer user_data, GDestroyNotify user_data_dnotify) {
  GMarkupParseContext *context;
  g_return_val_if_fail(parser != NULL, NULL);
  context = g_new(GMarkupParseContext, 1);
  context->parser = parser;
  context->flags = flags;
  context->user_data = user_data;
  context->dnotify = user_data_dnotify;
  context->line_number = 1;
  context->char_number = 1;
  context->partial_chunk = NULL;
  context->spare_chunks = NULL;
  context->spare_list_nodes = NULL;
  context->state = STATE_START;
  context->tag_stack = NULL;
  context->tag_stack_gstr = NULL;
  context->attr_names = NULL;
  context->attr_values = NULL;
  context->cur_attr = -1;
  context->alloc_attrs = 0;
  context->current_text = NULL;
  context->current_text_len = -1;
  context->current_text_end = NULL;
  context->start = NULL;
  context->iter = NULL;
  context->document_empty = TRUE;
  context->parsing = FALSE;
  context->awaiting_pop = FALSE;
  context->subparser_stack = NULL;
  context->subparser_element = NULL;
  context->held_user_data = NULL;
  context->balance = 0;
  return context;
}
static void string_full_free(gpointer ptr) {
  g_string_free (ptr, TRUE);
}
static void clear_attributes(GMarkupParseContext *context);
void g_markup_parse_context_free(GMarkupParseContext *context) {
  g_return_if_fail(context != NULL);
  g_return_if_fail(!context->parsing);
  g_return_if_fail(!context->subparser_stack);
  g_return_if_fail(!context->awaiting_pop);
  if (context->dnotify) (*context->dnotify)(context->user_data);
  clear_attributes(context);
  g_free(context->attr_names);
  g_free(context->attr_values);
  g_slist_free_full(context->tag_stack_gstr, string_full_free);
  g_slist_free(context->tag_stack);
  g_slist_free_full(context->spare_chunks, string_full_free);
  g_slist_free(context->spare_list_nodes);
  if (context->partial_chunk) g_string_free(context->partial_chunk, TRUE);
  g_free(context);
}
static void pop_subparser_stack(GMarkupParseContext *context);
static void mark_error(GMarkupParseContext *context, GError *error) {
  context->state = STATE_ERROR;
  if (context->parser->error) (*context->parser->error)(context, error, context->user_data);
  while (context->subparser_stack) {
      pop_subparser_stack(context);
      context->awaiting_pop = FALSE;
      if (context->parser->error) (*context->parser->error)(context, error, context->user_data);
  }
}
static void set_error(GMarkupParseContext *context, GError **error, GMarkupError code, const gchar *format, ...) G_GNUC_PRINTF (4, 5);
static void set_error_literal(GMarkupParseContext  *context, GError **error, GMarkupError code, const gchar *message) {
  GError *tmp_error;
  tmp_error = g_error_new_literal(G_MARKUP_ERROR, code, message);
  g_prefix_error(&tmp_error, _("Error on line %d char %d: "), context->line_number, context->char_number);
  mark_error(context, tmp_error);
  g_propagate_error(error, tmp_error);
}
static void set_error(GMarkupParseContext *context, GError **error, GMarkupError code, const gchar *format, ...) {
  gchar *s;
  gchar *s_valid;
  va_list args;
  va_start(args, format);
  s = g_strdup_vprintf(format, args);
  va_end(args);
  s_valid = _g_utf8_make_valid(s);
  set_error_literal(context, error, code, s);
  g_free(s);
  g_free(s_valid);
}
static void propagate_error(GMarkupParseContext *context, GError **dest, GError *src) {
  if (context->flags & G_MARKUP_PREFIX_ERROR_POSITION) g_prefix_error(&src, _("Error on line %d char %d: "), context->line_number, context->char_number);
  mark_error(context, src);
  g_propagate_error(dest, src);
}
#define IS_COMMON_NAME_END_CHAR(c) ((c) == '=' || (c) == '/' || (c) == '>' || (c) == ' ')
static gboolean slow_name_validate(GMarkupParseContext *context, const gchar *name, GError **error) {
  const gchar *p = name;
  if (!g_utf8_validate(name, strlen (name), NULL)) {
      set_error(context, error, G_MARKUP_ERROR_BAD_UTF8, _("Invalid UTF-8 encoded text in name - not valid '%s'"), name);
      return FALSE;
  }
  if (!(g_ascii_isalpha(*p) || (!IS_COMMON_NAME_END_CHAR(*p) && (*p == '_' || *p == ':' || g_unichar_isalpha(g_utf8_get_char(p)))))) {
      set_error(context, error, G_MARKUP_ERROR_PARSE, _("'%s' is not a valid name "), name);
      return FALSE;
  }
  for (p = g_utf8_next_char(name); *p != '\0'; p = g_utf8_next_char(p)) {
      if (!(g_ascii_isalnum(*p) || (!IS_COMMON_NAME_END_CHAR (*p) && (*p == '.' || *p == '-' || *p == '_' || *p == ':' ||
          g_unichar_isalpha(g_utf8_get_char(p)))))) {
          set_error(context, error, G_MARKUP_ERROR_PARSE, _("'%s' is not a valid name: '%c' "), name, *p);
          return FALSE;
      }
  }
  return TRUE;
}
static gboolean name_validate(GMarkupParseContext *context, const gchar *name, GError **error) {
  char mask;
  const char *p;
  p = name;
  if (G_UNLIKELY(IS_COMMON_NAME_END_CHAR(*p) || !(g_ascii_isalpha(*p) || *p == '_' || *p == ':'))) goto slow_validate;
  for (mask = *p++; *p != '\0'; p++) {
      mask |= *p;
      if (G_UNLIKELY(!(g_ascii_isalnum (*p) || (!IS_COMMON_NAME_END_CHAR(*p) && (*p == '.' || *p == '-' || *p == '_' || *p == ':')))))
          goto slow_validate;
  }
  if (mask & 0x80) goto slow_validate;
  return TRUE;
  slow_validate:
  return slow_name_validate(context, name, error);
}
static gboolean text_validate(GMarkupParseContext *context, const gchar *p, gint len, GError **error) {
  if (!g_utf8_validate (p, len, NULL)) {
      set_error(context, error, G_MARKUP_ERROR_BAD_UTF8, _("Invalid UTF-8 encoded text in name - not valid '%s'"), p);
      return FALSE;
  } else return TRUE;
}
static gchar* char_str(gunichar c, gchar *buf) {
  memset(buf, 0, 8);
  g_unichar_to_utf8(c, buf);
  return buf;
}
static gchar* utf8_str(const gchar *utf8, gchar *buf) {
  char_str(g_utf8_get_char (utf8), buf);
  return buf;
}
static void set_unescape_error(GMarkupParseContext *context, GError **error, const gchar *remaining_text, GMarkupError code, const gchar *format, ...) {
  GError *tmp_error;
  gchar *s;
  va_list args;
  gint remaining_newlines;
  const gchar *p;
  remaining_newlines = 0;
  p = remaining_text;
  while(*p != '\0') {
      if (*p == '\n') ++remaining_newlines;
      ++p;
  }
  va_start(args, format);
  s = g_strdup_vprintf(format, args);
  va_end(args);
  tmp_error = g_error_new(G_MARKUP_ERROR, code, _("Error on line %d: %s"), context->line_number - remaining_newlines, s);
  g_free(s);
  mark_error(context, tmp_error);
  g_propagate_error(error, tmp_error);
}
static gboolean unescape_gstring_inplace(GMarkupParseContext *context, GString *string, gboolean *is_ascii, GError **error) {
  char mask, *to;
  int line_num = 1;
  const char *from;
  gboolean normalize_attribute;
  *is_ascii = FALSE;
  if (context->state == STATE_INSIDE_ATTRIBUTE_VALUE_SQ || context->state == STATE_INSIDE_ATTRIBUTE_VALUE_DQ) normalize_attribute = TRUE;
  else normalize_attribute = FALSE;
  mask = 0;
  for (from = to = string->str; *from != '\0'; from++, to++) {
      *to = *from;
      mask |= *to;
      if (*to == '\n') line_num++;
      if (normalize_attribute && (*to == '\t' || *to == '\n')) *to = ' ';
      if (*to == '\r') {
          *to = normalize_attribute ? ' ' : '\n';
          if (from[1] == '\n') from++;
      }
      if (*from == '&') {
          from++;
          if (*from == '#') {
              gboolean is_hex = FALSE;
              gulong l;
              gchar *end = NULL;
              from++;
              if (*from == 'x') {
                  is_hex = TRUE;
                  from++;
              }
              errno = 0;
              if (is_hex) l = strtoul(from, &end, 16);
              else l = strtoul(from, &end, 10);
              if (end == from || errno != 0) {
                  set_unescape_error(context, error, from, G_MARKUP_ERROR_PARSE, _("Failed to parse '%-.*s', which should have been a digit "
                                     "inside a character reference (&#234; for example) - perhaps the digit is too large"), end - from, from);
                  return FALSE;
              } else if (*end != ';') {
                  set_unescape_error(context, error, from, G_MARKUP_ERROR_PARSE, _("Character reference did not end with a semicolon; "
                                     "most likely you used an ampersand character without intending to start an entity - escape ampersand as &amp;"));
                  return FALSE;
              } else {
                  if ((0 < l && l <= 0xD7FF) || (0xE000 <= l && l <= 0xFFFD) || (0x10000 <= l && l <= 0x10FFFF)) {
                      gchar buf[8];
                      char_str(l, buf);
                      strcpy(to, buf);
                      to += strlen(buf) - 1;
                      from = end;
                      if (l >= 0x80) mask |= 0x80;
                  } else {
                      set_unescape_error(context, error, from, G_MARKUP_ERROR_PARSE, _("Character reference '%-.*s' does not "
                                         "encode a permitted character"), end - from, from);
                      return FALSE;
                  }
              }
          } else if (strncmp(from, "lt;", 3) == 0) {
              *to = '<';
              from += 2;
          } else if (strncmp(from, "gt;", 3) == 0) {
              *to = '>';
              from += 2;
          } else if (strncmp(from, "amp;", 4) == 0) {
              *to = '&';
              from += 3;
          } else if (strncmp(from, "quot;", 5) == 0) {
              *to = '"';
              from += 4;
          } else if (strncmp(from, "apos;", 5) == 0) {
              *to = '\'';
              from += 4;
          } else {
              if (*from == ';') {
                  set_unescape_error(context, error, from, G_MARKUP_ERROR_PARSE, _("Empty entity '&;' seen; valid entities are: &amp; &quot; "
                                     "&lt; &gt; &apos;"));
              } else {
                  const char *end = strchr (from, ';');
                  if (end) {
                      set_unescape_error(context, error, from, G_MARKUP_ERROR_PARSE, _("Entity name '%-.*s' is not known"), end - from, from);
                  } else {
                      set_unescape_error(context, error, from, G_MARKUP_ERROR_PARSE, _("Entity did not end with a semicolon; most likely you used "
                                         "an ampersand character without intending to start an entity - escape ampersand as &amp;"));
                  }
              }
              return FALSE;
          }
      }
  }
  g_assert(to - string->str <= string->len);
  if (to - string->str != string->len) g_string_truncate (string, to - string->str);
  *is_ascii = !(mask & 0x80);
  return TRUE;
}
static inline gboolean advance_char(GMarkupParseContext *context) {
  context->iter++;
  context->char_number++;
  if (G_UNLIKELY(context->iter == context->current_text_end)) return FALSE;
  else if (G_UNLIKELY(*context->iter == '\n')) {
      context->line_number++;
      context->char_number = 1;
  }
  return TRUE;
}
static inline gboolean xml_isspace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
static void skip_spaces(GMarkupParseContext *context) {
  do {
      if (!xml_isspace(*context->iter)) return;
  } while(advance_char(context));
}
static void advance_to_name_end(GMarkupParseContext *context) {
  do {
      if (IS_COMMON_NAME_END_CHAR(*(context->iter))) return;
      if (xml_isspace(*(context->iter))) return;
  } while(advance_char(context));
}
static void release_chunk(GMarkupParseContext *context, GString *str) {
  GSList *node;
  if (!str) return;
  if (str->allocated_len > 256) {
      g_string_free(str, TRUE);
      return;
  }
  string_blank(str);
  node = get_list_node(context, str);
  context->spare_chunks = g_slist_concat(node, context->spare_chunks);
}
static void add_to_partial(GMarkupParseContext *context, const gchar *text_start, const gchar *text_end) {
  if (context->partial_chunk == NULL) {
      if (context->spare_chunks != NULL) {
          GSList *node = context->spare_chunks;
          context->spare_chunks = g_slist_remove_link(context->spare_chunks, node);
          context->partial_chunk = node->data;
          free_list_node(context, node);
      } else context->partial_chunk = g_string_sized_new(MAX (28, text_end - text_start));
  }
  if (text_start != text_end) g_string_insert_len(context->partial_chunk, -1, text_start, text_end - text_start);
}
static inline void truncate_partial(GMarkupParseContext *context) {
  if (context->partial_chunk != NULL) string_blank(context->partial_chunk);
}
static inline const gchar* current_element(GMarkupParseContext *context) {
  return context->tag_stack->data;
}
static void pop_subparser_stack(GMarkupParseContext *context) {
  GMarkupRecursionTracker *tracker;
  g_assert(context->subparser_stack);
  tracker = context->subparser_stack->data;
  context->awaiting_pop = TRUE;
  context->held_user_data = context->user_data;
  context->user_data = tracker->prev_user_data;
  context->parser = tracker->prev_parser;
  context->subparser_element = tracker->prev_element;
  g_slice_free(GMarkupRecursionTracker, tracker);
  context->subparser_stack = g_slist_delete_link(context->subparser_stack, context->subparser_stack);
}
static void push_partial_as_tag(GMarkupParseContext *context) {
  GString *str = context->partial_chunk;
  context->tag_stack = g_slist_concat(get_list_node(context, str->str), context->tag_stack);
  context->tag_stack_gstr = g_slist_concat(get_list_node(context, str), context->tag_stack_gstr);
  context->partial_chunk = NULL;
}
static void pop_tag(GMarkupParseContext *context) {
  GSList *nodea, *nodeb;
  nodea = context->tag_stack;
  nodeb = context->tag_stack_gstr;
  release_chunk(context, nodeb->data);
  context->tag_stack = g_slist_remove_link(context->tag_stack, nodea);
  context->tag_stack_gstr = g_slist_remove_link(context->tag_stack_gstr, nodeb);
  free_list_node(context, nodea);
  free_list_node(context, nodeb);
}
static void possibly_finish_subparser(GMarkupParseContext *context) {
  if (current_element(context) == context->subparser_element)
    pop_subparser_stack(context);
}
static void ensure_no_outstanding_subparser(GMarkupParseContext *context) {
  if (context->awaiting_pop)
      g_critical("During the first end_element call after invoking a subparser you must pop the subparser stack and handle the freeing of the subparser user_data.  "
                 "This can be done by calling the end function of the subparser.  Very probably, your program just leaked memory.");
  context->held_user_data = NULL;
  context->awaiting_pop = FALSE;
}
static const gchar* current_attribute(GMarkupParseContext *context) {
  g_assert(context->cur_attr >= 0);
  return context->attr_names[context->cur_attr]->str;
}
static void add_attribute(GMarkupParseContext *context, GString *str) {
  if (context->cur_attr + 2 >= context->alloc_attrs) {
      context->alloc_attrs += 5;
      context->attr_names = g_realloc(context->attr_names, sizeof(GString*)*context->alloc_attrs);
      context->attr_values = g_realloc(context->attr_values, sizeof(GString*)*context->alloc_attrs);
  }
  context->cur_attr++;
  context->attr_names[context->cur_attr] = str;
  context->attr_values[context->cur_attr] = NULL;
  context->attr_names[context->cur_attr+1] = NULL;
  context->attr_values[context->cur_attr+1] = NULL;
}
static void clear_attributes(GMarkupParseContext *context) {
  for (; context->cur_attr >= 0; context->cur_attr--) {
      int pos = context->cur_attr;
      release_chunk(context, context->attr_names[pos]);
      release_chunk(context, context->attr_values[pos]);
      context->attr_names[pos] = context->attr_values[pos] = NULL;
  }
  g_assert(context->cur_attr == -1);
  g_assert(context->attr_names == NULL || context->attr_names[0] == NULL);
  g_assert(context->attr_values == NULL || context->attr_values[0] == NULL);
}
static inline void emit_start_element(GMarkupParseContext *context, GError **error) {
  int i;
  const gchar *start_name;
  const gchar **attr_names;
  const gchar **attr_values;
  GError *tmp_error;
  attr_names = g_newa(const gchar *, context->cur_attr + 2);
  attr_values = g_newa(const gchar *, context->cur_attr + 2);
  for (i = 0; i < context->cur_attr + 1; i++) {
      attr_names[i] = context->attr_names[i]->str;
      attr_values[i] = context->attr_values[i]->str;
  }
  attr_names[i] = NULL;
  attr_values[i] = NULL;
  tmp_error = NULL;
  start_name = current_element(context);
  if (context->parser->start_element && name_validate(context, start_name, error))
      (*context->parser->start_element)(context, start_name, (const gchar**)attr_names, (const gchar**)attr_values, context->user_data, &tmp_error);
  clear_attributes(context);
  if (tmp_error != NULL) propagate_error(context, error, tmp_error);
}
gboolean g_markup_parse_context_parse(GMarkupParseContext *context, const gchar *text, gssize text_len, GError **error) {
  g_return_val_if_fail(context != NULL, FALSE);
  g_return_val_if_fail(text != NULL, FALSE);
  g_return_val_if_fail(context->state != STATE_ERROR, FALSE);
  g_return_val_if_fail(!context->parsing, FALSE);
  if (text_len < 0) text_len = strlen(text);
  if (text_len == 0) return TRUE;
  context->parsing = TRUE;
  context->current_text = text;
  context->current_text_len = text_len;
  context->current_text_end = context->current_text + text_len;
  context->iter = context->current_text;
  context->start = context->iter;
  if (context->current_text_len == 0) goto finished;
  while(context->iter != context->current_text_end) {
      switch(context->state) {
        case STATE_START:
          g_assert(context->tag_stack == NULL);
          skip_spaces(context);
          if (context->iter != context->current_text_end) {
              if (*context->iter == '<') {
                  advance_char(context);
                  context->state = STATE_AFTER_OPEN_ANGLE;
                  context->start = context->iter;
                  context->document_empty = FALSE;
              } else {
                  set_error_literal(context, error,G_MARKUP_ERROR_PARSE, _("Document must begin with an element (e.g. <book>)"));
              }
          }
          break;
        case STATE_AFTER_OPEN_ANGLE:
          if (*context->iter == '?' || *context->iter == '!') {
              const gchar *openangle = "<";
              add_to_partial(context, openangle, openangle + 1);
              context->start = context->iter;
              context->balance = 1;
              context->state = STATE_INSIDE_PASSTHROUGH;
          } else if (*context->iter == '/') {
              advance_char (context);
              context->state = STATE_AFTER_CLOSE_TAG_SLASH;
          } else if (!IS_COMMON_NAME_END_CHAR(*(context->iter))) {
              context->state = STATE_INSIDE_OPEN_TAG_NAME;
              context->start = context->iter;
          } else {
              gchar buf[8];
              set_error(context, error,G_MARKUP_ERROR_PARSE, _("'%s' is not a valid character following a '<' character; it may not begin an "
                        "element name"), utf8_str (context->iter, buf));
          }
          break;
        case STATE_AFTER_CLOSE_ANGLE:
          if (context->tag_stack == NULL) {
              context->start = NULL;
              context->state = STATE_START;
          } else {
              context->start = context->iter;
              context->state = STATE_INSIDE_TEXT;
          }
          break;
        case STATE_AFTER_ELISION_SLASH: {
            GError *tmp_error = NULL;
            g_assert(context->tag_stack != NULL);
            possibly_finish_subparser(context);
            tmp_error = NULL;
            if (context->parser->end_element)
                (*context->parser->end_element)(context, current_element(context), context->user_data, &tmp_error);
            ensure_no_outstanding_subparser(context);
            if (tmp_error) {
                mark_error(context, tmp_error);
                g_propagate_error(error, tmp_error);
            } else {
                if (*context->iter == '>') {
                    advance_char(context);
                    context->state = STATE_AFTER_CLOSE_ANGLE;
                } else {
                    gchar buf[8];
                    set_error(context, error,G_MARKUP_ERROR_PARSE, _("Odd character '%s', expected a '>' character to end the empty-element tag '%s'"),
                              utf8_str (context->iter, buf), current_element (context));
                }
            }
            pop_tag(context);
          }
          break;
        case STATE_INSIDE_OPEN_TAG_NAME:
          advance_to_name_end (context);
          if (context->iter == context->current_text_end) add_to_partial(context, context->start, context->iter);
          else {
              add_to_partial(context, context->start, context->iter);
              push_partial_as_tag(context);
              context->state = STATE_BETWEEN_ATTRIBUTES;
              context->start = NULL;
          }
          break;
        case STATE_INSIDE_ATTRIBUTE_NAME:
          advance_to_name_end(context);
          add_to_partial(context, context->start, context->iter);
          if (context->iter != context->current_text_end) context->state = STATE_AFTER_ATTRIBUTE_NAME;
          break;
        case STATE_AFTER_ATTRIBUTE_NAME:
          skip_spaces(context);
          if (context->iter != context->current_text_end) {
              if (!name_validate (context, context->partial_chunk->str, error)) break;
              add_attribute(context, context->partial_chunk);
              context->partial_chunk = NULL;
              context->start = NULL;
              if (*context->iter == '=') {
                  advance_char(context);
                  context->state = STATE_AFTER_ATTRIBUTE_EQUALS_SIGN;
              } else {
                  gchar buf[8];
                  set_error(context, error,G_MARKUP_ERROR_PARSE, _("Odd character '%s', expected a '=' after attribute name '%s' of element '%s'"),
                            utf8_str (context->iter, buf), current_attribute(context), current_element(context));

              }
          }
          break;
        case STATE_BETWEEN_ATTRIBUTES:
          skip_spaces (context);
          if (context->iter != context->current_text_end) {
              if (*context->iter == '/') {
                  advance_char (context);
                  context->state = STATE_AFTER_ELISION_SLASH;
              } else if (*context->iter == '>') {
                  advance_char (context);
                  context->state = STATE_AFTER_CLOSE_ANGLE;
              } else if (!IS_COMMON_NAME_END_CHAR (*(context->iter))) {
                  context->state = STATE_INSIDE_ATTRIBUTE_NAME;
                  context->start = context->iter;
              } else {
                  gchar buf[8];
                  set_error(context, error,G_MARKUP_ERROR_PARSE, _("Odd character '%s', expected a '>' or '/' character to end the start tag of "
                            "element '%s', or optionally an attribute; perhaps you used an invalid character in an attribute name"), utf8_str (context->iter, buf),
                            current_element (context));
              }
              if (context->state == STATE_AFTER_ELISION_SLASH || context->state == STATE_AFTER_CLOSE_ANGLE) emit_start_element(context, error);
          }
          break;
        case STATE_AFTER_ATTRIBUTE_EQUALS_SIGN:
          skip_spaces (context);
          if (context->iter != context->current_text_end) {
              if (*context->iter == '"') {
                  advance_char (context);
                  context->state = STATE_INSIDE_ATTRIBUTE_VALUE_DQ;
                  context->start = context->iter;
              } else if (*context->iter == '\'') {
                  advance_char (context);
                  context->state = STATE_INSIDE_ATTRIBUTE_VALUE_SQ;
                  context->start = context->iter;
              } else {
                  gchar buf[8];
                  set_error(context, error,G_MARKUP_ERROR_PARSE, _("Odd character '%s', expected an open quote mark after the equals sign when giving "
                            "value for attribute '%s' of element '%s'"), utf8_str(context->iter, buf), current_attribute(context), current_element(context));
              }
          }
          break;
        case STATE_INSIDE_ATTRIBUTE_VALUE_SQ:
        case STATE_INSIDE_ATTRIBUTE_VALUE_DQ: {
            gchar delim;
            if (context->state == STATE_INSIDE_ATTRIBUTE_VALUE_SQ) delim = '\'';
            else delim = '"';
            do {
                if (*context->iter == delim) break;
            } while(advance_char (context));
          }
          if (context->iter == context->current_text_end) add_to_partial (context, context->start, context->iter);
          else {
              gboolean is_ascii;
              add_to_partial(context, context->start, context->iter);
              g_assert (context->cur_attr >= 0);
              if (unescape_gstring_inplace(context, context->partial_chunk, &is_ascii, error) && (is_ascii || text_validate(context, context->partial_chunk->str,
                  context->partial_chunk->len, error))) {
                  context->attr_values[context->cur_attr] = context->partial_chunk;
                  context->partial_chunk = NULL;
                  advance_char(context);
                  context->state = STATE_BETWEEN_ATTRIBUTES;
                  context->start = NULL;
              }
              truncate_partial (context);
          }
          break;
        case STATE_INSIDE_TEXT:
          do {
              if (*context->iter == '<') break;
          } while(advance_char (context));
          add_to_partial(context, context->start, context->iter);
          if (context->iter != context->current_text_end) {
              gboolean is_ascii;
              if (unescape_gstring_inplace(context, context->partial_chunk, &is_ascii, error) && (is_ascii || text_validate(context, context->partial_chunk->str,
                  context->partial_chunk->len, error))) {
                  GError *tmp_error = NULL;
                  if (context->parser->text)
                      (*context->parser->text)(context, context->partial_chunk->str, context->partial_chunk->len, context->user_data, &tmp_error);
                  if (tmp_error == NULL) {
                      advance_char (context);
                      context->state = STATE_AFTER_OPEN_ANGLE;
                      context->start = context->iter;
                  } else propagate_error(context, error, tmp_error);
              }
              truncate_partial (context);
          }
          break;
        case STATE_AFTER_CLOSE_TAG_SLASH:
          if (!IS_COMMON_NAME_END_CHAR(*(context->iter))) {
              context->state = STATE_INSIDE_CLOSE_TAG_NAME;
              context->start = context->iter;
          } else {
              gchar buf[8];
              set_error(context, error,G_MARKUP_ERROR_PARSE, _("'%s' is not a valid character following the characters '</'; '%s' may not begin an "
                        "element name"), utf8_str(context->iter, buf), utf8_str(context->iter, buf));
          }
          break;
        case STATE_INSIDE_CLOSE_TAG_NAME:
          advance_to_name_end(context);
          add_to_partial(context, context->start, context->iter);
          if (context->iter != context->current_text_end) context->state = STATE_AFTER_CLOSE_TAG_NAME;
          break;
        case STATE_AFTER_CLOSE_TAG_NAME:
          skip_spaces (context);
          if (context->iter != context->current_text_end) {
              GString *close_name;
              close_name = context->partial_chunk;
              context->partial_chunk = NULL;
              if (*context->iter != '>') {
                  gchar buf[8];
                  set_error(context, error, G_MARKUP_ERROR_PARSE, _("'%s' is not a valid character following the close element name '%s'; the allowed "
                            "character is '>'"), utf8_str(context->iter, buf), close_name->str);
              } else if (context->tag_stack == NULL) {
                  set_error(context, error, G_MARKUP_ERROR_PARSE, _("Element '%s' was closed, no element is currently open"), close_name->str);
              } else if (strcmp(close_name->str, current_element(context)) != 0) {
                  set_error(context, error, G_MARKUP_ERROR_PARSE, _("Element '%s' was closed, but the currently open element is '%s'"), close_name->str,
                            current_element(context));
              } else {
                  GError *tmp_error;
                  advance_char(context);
                  context->state = STATE_AFTER_CLOSE_ANGLE;
                  context->start = NULL;
                  possibly_finish_subparser(context);
                  tmp_error = NULL;
                  if (context->parser->end_element) (*context->parser->end_element)(context, close_name->str, context->user_data, &tmp_error);
                  ensure_no_outstanding_subparser(context);
                  pop_tag(context);
                  if (tmp_error) propagate_error(context, error, tmp_error);
              }
              context->partial_chunk = close_name;
              truncate_partial(context);
          }
          break;
        case STATE_INSIDE_PASSTHROUGH:
          do {
              if (*context->iter == '<') context->balance++;
              if (*context->iter == '>') {
                  gchar *str;
                  gsize len;
                  context->balance--;
                  add_to_partial(context, context->start, context->iter);
                  context->start = context->iter;
                  str = context->partial_chunk->str;
                  len = context->partial_chunk->len;
                  if (str[1] == '?' && str[len - 1] == '?') break;
                  if (strncmp(str, "<!--", 4) == 0 && strcmp(str + len - 2, "--") == 0) break;
                  if (strncmp(str, "<![CDATA[", 9) == 0 && strcmp(str + len - 2, "]]") == 0) break;
                  if (strncmp(str, "<!DOCTYPE", 9) == 0 && context->balance == 0) break;
              }
          } while(advance_char (context));
          if (context->iter == context->current_text_end) add_to_partial(context, context->start, context->iter);
          else {
              GError *tmp_error = NULL;
              advance_char(context);
              add_to_partial(context, context->start, context->iter);
              if (context->flags & G_MARKUP_TREAT_CDATA_AS_TEXT && strncmp(context->partial_chunk->str, "<![CDATA[", 9) == 0) {
                  if (context->parser->text && text_validate(context, context->partial_chunk->str + 9, context->partial_chunk->len - 12, error))
                    (*context->parser->text)(context, context->partial_chunk->str + 9, context->partial_chunk->len - 12, context->user_data, &tmp_error);
              } else if (context->parser->passthrough && text_validate(context, context->partial_chunk->str, context->partial_chunk->len, error))
                  (*context->parser->passthrough)(context, context->partial_chunk->str, context->partial_chunk->len, context->user_data, &tmp_error);
              truncate_partial(context);
              if (tmp_error == NULL) {
                  context->state = STATE_AFTER_CLOSE_ANGLE;
                  context->start = context->iter;
              } else propagate_error(context, error, tmp_error);
          }
          break;
        case STATE_ERROR: goto finished;
        default: g_assert_not_reached(); break;
      }
  }
  finished:
  context->parsing = FALSE;
  return context->state != STATE_ERROR;
}
gboolean g_markup_parse_context_end_parse(GMarkupParseContext *context, GError **error) {
  g_return_val_if_fail(context != NULL, FALSE);
  g_return_val_if_fail(!context->parsing, FALSE);
  g_return_val_if_fail(context->state != STATE_ERROR, FALSE);
  if (context->partial_chunk != NULL) {
      g_string_free(context->partial_chunk, TRUE);
      context->partial_chunk = NULL;
  }
  if (context->document_empty) {
      set_error_literal(context, error, G_MARKUP_ERROR_EMPTY, _("Document was empty or contained only whitespace"));
      return FALSE;
  }
  context->parsing = TRUE;
  switch (context->state) {
    case STATE_START: break;
    case STATE_AFTER_OPEN_ANGLE:
        set_error_literal(context, error, G_MARKUP_ERROR_PARSE, _("Document ended unexpectedly just after an open angle bracket '<'"));
        break;
    case STATE_AFTER_CLOSE_ANGLE:
        if (context->tag_stack != NULL) {
            set_error(context, error, G_MARKUP_ERROR_PARSE, _("Document ended unexpectedly with elements still open - '%s' was the last element opened"),
                      current_element(context));
        }
        break;
    case STATE_AFTER_ELISION_SLASH:
        set_error(context, error, G_MARKUP_ERROR_PARSE, _("Document ended unexpectedly, expected to see a close angle bracket ending the tag <%s/>"),
                  current_element(context));
        break;
    case STATE_INSIDE_OPEN_TAG_NAME:
        set_error_literal(context, error, G_MARKUP_ERROR_PARSE, _("Document ended unexpectedly inside an element name"));
        break;
    case STATE_INSIDE_ATTRIBUTE_NAME: case STATE_AFTER_ATTRIBUTE_NAME:
        set_error_literal(context, error, G_MARKUP_ERROR_PARSE, _("Document ended unexpectedly inside an attribute name"));
        break;
    case STATE_BETWEEN_ATTRIBUTES:
        set_error_literal(context, error, G_MARKUP_ERROR_PARSE, _("Document ended unexpectedly inside an element-opening tag."));
        break;
    case STATE_AFTER_ATTRIBUTE_EQUALS_SIGN:
        set_error_literal(context, error, G_MARKUP_ERROR_PARSE, _("Document ended unexpectedly after the equals sign following an attribute name; "
                          "no attribute value"));
      break;
    case STATE_INSIDE_ATTRIBUTE_VALUE_SQ: case STATE_INSIDE_ATTRIBUTE_VALUE_DQ:
        set_error_literal(context, error, G_MARKUP_ERROR_PARSE, _("Document ended unexpectedly while inside an attribute value"));
        break;
    case STATE_INSIDE_TEXT:
        g_assert(context->tag_stack != NULL);
        set_error(context, error, G_MARKUP_ERROR_PARSE, _("Document ended unexpectedly with elements still open - '%s' was the last element opened"),
                  current_element(context));
        break;
    case STATE_AFTER_CLOSE_TAG_SLASH: case STATE_INSIDE_CLOSE_TAG_NAME: case STATE_AFTER_CLOSE_TAG_NAME:
        set_error(context, error, G_MARKUP_ERROR_PARSE, _("Document ended unexpectedly inside the close tag for element '%s'"),
                  current_element(context));
        break;
    case STATE_INSIDE_PASSTHROUGH:
        set_error_literal(context, error, G_MARKUP_ERROR_PARSE, _("Document ended unexpectedly inside a comment or processing instruction"));
        break;
    default: g_assert_not_reached(); break;
  }
  context->parsing = FALSE;
  return context->state != STATE_ERROR;
}
G_CONST_RETURN gchar* g_markup_parse_context_get_element(GMarkupParseContext *context) {
  g_return_val_if_fail(context != NULL, NULL);
  if (context->tag_stack == NULL) return NULL;
  else return current_element(context);
}
G_CONST_RETURN GSList* g_markup_parse_context_get_element_stack(GMarkupParseContext *context) {
  g_return_val_if_fail(context != NULL, NULL);
  return context->tag_stack;
}
void g_markup_parse_context_get_position(GMarkupParseContext *context, gint *line_number, gint *char_number) {
  g_return_if_fail(context != NULL);
  if (line_number) *line_number = context->line_number;
  if (char_number) *char_number = context->char_number;
}
gpointer g_markup_parse_context_get_user_data(GMarkupParseContext *context) {
  return context->user_data;
}
void g_markup_parse_context_push(GMarkupParseContext *context, const GMarkupParser *parser, gpointer user_data) {
  GMarkupRecursionTracker *tracker;
  tracker = g_slice_new(GMarkupRecursionTracker);
  tracker->prev_element = context->subparser_element;
  tracker->prev_parser = context->parser;
  tracker->prev_user_data = context->user_data;
  context->subparser_element = current_element(context);
  context->parser = parser;
  context->user_data = user_data;
  context->subparser_stack = g_slist_prepend(context->subparser_stack, tracker);
}
gpointer g_markup_parse_context_pop(GMarkupParseContext *context) {
  gpointer user_data;
  if (!context->awaiting_pop) possibly_finish_subparser(context);
  g_assert(context->awaiting_pop);
  context->awaiting_pop = FALSE;
  user_data = context->held_user_data;
  context->held_user_data = NULL;
  return user_data;
}
static void append_escaped_text(GString *str, const gchar *text, gssize length) {
  const gchar *p;
  const gchar *end;
  gunichar c;
  p = text;
  end = text + length;
  while(p != end) {
      const gchar *next;
      next = g_utf8_next_char(p);
      switch(*p) {
        case '&': g_string_append(str, "&amp;"); break;
        case '<': g_string_append(str, "&lt;"); break;
        case '>': g_string_append(str, "&gt;"); break;
        case '\'': g_string_append(str, "&apos;"); break;
        case '"': g_string_append(str, "&quot;"); break;
        default:
          c = g_utf8_get_char(p);
          if ((0x1 <= c && c <= 0x8) || (0xb <= c && c  <= 0xc) || (0xe <= c && c <= 0x1f) || (0x7f <= c && c <= 0x84) || (0x86 <= c && c <= 0x9f)) {
            g_string_append_printf(str, "&#x%x;", c);
          } else g_string_append_len(str, p, next - p);
          break;
      }
      p = next;
  }
}
gchar* g_markup_escape_text(const gchar *text, gssize length) {
  GString *str;
  g_return_val_if_fail(text != NULL, NULL);
  if (length < 0) length = strlen(text);
  str = g_string_sized_new(length);
  append_escaped_text(str, text, length);
  return g_string_free(str, FALSE);
}
static const char* find_conversion(const char  *format, const char **after) {
  const char *start = format;
  const char *cp;
  while(*start != '\0' && *start != '%') start++;
  if (*start == '\0') {
      *after = start;
      return NULL;
  }
  cp = start + 1;
  if (*cp == '\0') {
      *after = cp;
      return NULL;
  }
  if (*cp >= '0' && *cp <= '9') {
      const char *np;
      for (np = cp; *np >= '0' && *np <= '9'; np++);
      if (*np == '$') cp = np + 1;
  }
  for ( ; ; ) {
      if (*cp == '\'' || *cp == '-' || *cp == '+' || *cp == ' ' || *cp == '#' || *cp == '0') cp++;
      else break;
  }
  if (*cp == '*') {
      cp++;
      if (*cp >= '0' && *cp <= '9') {
          const char *np;
          for (np = cp; *np >= '0' && *np <= '9'; np++);
          if (*np == '$') cp = np + 1;
      }
  } else for (; *cp >= '0' && *cp <= '9'; cp++);
  if (*cp == '.') {
      cp++;
      if (*cp == '*') {
          if (*cp >= '0' && *cp <= '9') {
              const char *np;
              for (np = cp; *np >= '0' && *np <= '9'; np++);
              if (*np == '$')cp = np + 1;
          }
      } else for (; *cp >= '0' && *cp <= '9'; cp++);
  }
  while(*cp == 'h' || *cp == 'L' || *cp == 'l' || *cp == 'j' || *cp == 'z' || *cp == 'Z' || *cp == 't') cp++;
  cp++;
  *after = cp;
  return start;
}
gchar* g_markup_vprintf_escaped(const gchar *format, va_list args) {
  GString *format1;
  GString *format2;
  GString *result = NULL;
  gchar *output1 = NULL;
  gchar *output2 = NULL;
  const char *p, *op1, *op2;
  va_list args2;
  format1 = g_string_new(NULL);
  format2 = g_string_new(NULL);
  p = format;
  while(TRUE) {
      const char *after;
      const char *conv = find_conversion(p, &after);
      if (!conv) break;
      g_string_append_len(format1, conv, after - conv);
      g_string_append_c(format1, 'X');
      g_string_append_len(format2, conv, after - conv);
      g_string_append_c(format2, 'Y');
      p = after;
  }
  G_VA_COPY(args2, args);
  output1 = g_strdup_vprintf(format1->str, args);
  if (!output1) {
      va_end(args2);
      goto cleanup;
  }
  output2 = g_strdup_vprintf(format2->str, args2);
  va_end(args2);
  if (!output2) goto cleanup;
  result = g_string_new(NULL);
  op1 = output1;
  op2 = output2;
  p = format;
  while(TRUE) {
      const char *after;
      const char *output_start;
      const char *conv = find_conversion(p, &after);
      char *escaped;
      if (!conv) {
          g_string_append_len(result, p, after - p);
          break;
      }
      g_string_append_len(result, p, conv - p);
      output_start = op1;
      while (*op1 == *op2) {
          op1++;
          op2++;
      }
      escaped = g_markup_escape_text(output_start, op1 - output_start);
      g_string_append(result, escaped);
      g_free(escaped);
      p = after;
      op1++;
      op2++;
  }
  cleanup:
  g_string_free(format1, TRUE);
  g_string_free(format2, TRUE);
  g_free(output1);
  g_free(output2);
  if (result) return g_string_free(result, FALSE);
  else return NULL;
}
gchar* g_markup_printf_escaped(const gchar *format, ...) {
  char *result;
  va_list args;
  va_start(args, format);
  result = g_markup_vprintf_escaped(format, args);
  va_end(args);
  return result;
}
static gboolean g_markup_parse_boolean(const char *string, gboolean *value) {
  char const* const falses[] = { "false", "f", "no", "n", "0" };
  char const* const trues[] = { "true", "t", "yes", "y", "1" };
  int i;
  for (i = 0; i < G_N_ELEMENTS(falses); i++) {
      if (g_ascii_strcasecmp(string, falses[i]) == 0) {
          if (value != NULL) *value = FALSE;
          return TRUE;
      }
  }
  for (i = 0; i < G_N_ELEMENTS (trues); i++) {
      if (g_ascii_strcasecmp(string, trues[i]) == 0) {
          if (value != NULL) *value = TRUE;
          return TRUE;
      }
  }
  return FALSE;
}
gboolean g_markup_collect_attributes(const gchar *element_name, const gchar **attribute_names, const gchar **attribute_values, GError **error,
                                     GMarkupCollectType first_type, const gchar *first_attr, ...) {
  GMarkupCollectType type;
  const gchar *attr;
  unsigned long long collected;
  int written;
  va_list ap;
  int i;
  type = first_type;
  attr = first_attr;
  collected = 0;
  written = 0;
  va_start(ap, first_attr);
  while(type != G_MARKUP_COLLECT_INVALID) {
      gboolean mandatory;
      const gchar *value;
      mandatory = !(type & G_MARKUP_COLLECT_OPTIONAL);
      type &= (G_MARKUP_COLLECT_OPTIONAL - 1);
      if (type == G_MARKUP_COLLECT_TRISTATE) mandatory = FALSE;
      for (i = 0; attribute_names[i]; i++)
        if (i >= 40 || !(collected & (G_GUINT64_CONSTANT(1) << i)))
          if (!strcmp (attribute_names[i], attr)) break;
      if (i < 40) collected |= (G_GUINT64_CONSTANT(1) << i);
      value = attribute_values[i];
      if (value == NULL && mandatory) {
          g_set_error(error, G_MARKUP_ERROR,G_MARKUP_ERROR_MISSING_ATTRIBUTE,"element '%s' requires attribute '%s'", element_name, attr);
          va_end(ap);
          goto failure;
      }
      switch(type) {
        case G_MARKUP_COLLECT_STRING: {
            const char **str_ptr;
            str_ptr = va_arg(ap, const char **);
            if (str_ptr != NULL) *str_ptr = value;
          }
          break;
        case G_MARKUP_COLLECT_STRDUP: {
            char **str_ptr;
            str_ptr = va_arg(ap, char **);
            if (str_ptr != NULL) *str_ptr = g_strdup(value);
          }
          break;
        case G_MARKUP_COLLECT_BOOLEAN: case G_MARKUP_COLLECT_TRISTATE:
          if (value == NULL) {
              gboolean *bool_ptr;
              bool_ptr = va_arg(ap, gboolean *);
              if (bool_ptr != NULL) {
                  if (type == G_MARKUP_COLLECT_TRISTATE) *bool_ptr = -1;
                  else *bool_ptr = FALSE;
              }
          } else {
              if (!g_markup_parse_boolean(value, va_arg(ap, gboolean *))) {
                  g_set_error(error, G_MARKUP_ERROR,G_MARKUP_ERROR_INVALID_CONTENT,"element '%s', attribute '%s', value '%s' "
                              "cannot be parsed as a boolean value", element_name, attr, value);
                  va_end(ap);
                  goto failure;
              }
          }
          break;
        default: g_assert_not_reached();
      }
      type = va_arg(ap, GMarkupCollectType);
      attr = va_arg(ap, const char *);
      written++;
  }
  va_end(ap);
  for (i = 0; attribute_names[i]; i++) {
      if ((collected & (G_GUINT64_CONSTANT(1) << i)) == 0) {
          int j;
          for (j = 0; j < i; j++) if (strcmp(attribute_names[i], attribute_names[j]) == 0) break;
          if (i == j) {
              g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE,"attribute '%s' invalid for element '%s'", attribute_names[i],
                          element_name);
          } else {
              g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,"attribute '%s' given multiple times for element '%s'",
                          attribute_names[i], element_name);
          }
          goto failure;
      }
  }
  return TRUE;
failure:
  type = first_type;
  attr = first_attr;
  va_start (ap, first_attr);
  while(type != G_MARKUP_COLLECT_INVALID) {
      gpointer ptr;
      ptr = va_arg(ap, gpointer);
      if (ptr == NULL) continue;
      switch(type & (G_MARKUP_COLLECT_OPTIONAL - 1)) {
          case G_MARKUP_COLLECT_STRDUP: if (written) g_free (*(char**)ptr);
          case G_MARKUP_COLLECT_STRING: *(char**)ptr = NULL; break;
          case G_MARKUP_COLLECT_BOOLEAN: *(gboolean*)ptr = FALSE; break;
          case G_MARKUP_COLLECT_TRISTATE: *(gboolean*)ptr = -1; break;
      }
      type = va_arg(ap, GMarkupCollectType);
      attr = va_arg(ap, const char *);
      if (written) written--;
  }
  va_end(ap);
  return FALSE;
}