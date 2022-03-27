#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "goption.h"
#include "gprintf.h"
#include "glibintl.h"

#define TRANSLATE(group, str) (((group)->translate_func ? (* (group)->translate_func) ((str), (group)->translate_data) : (str)))
#define NO_ARG(entry) ((entry)->arg == G_OPTION_ARG_NONE || ((entry)->arg == G_OPTION_ARG_CALLBACK && ((entry)->flags & G_OPTION_FLAG_NO_ARG)))
#define OPTIONAL_ARG(entry) ((entry)->arg == G_OPTION_ARG_CALLBACK && (entry)->flags & G_OPTION_FLAG_OPTIONAL_ARG)
typedef struct {
  GOptionArg arg_type;
  gpointer arg_data;
  union {
      gboolean bool;
      gint integer;
      gchar *str;
      gchar **array;
      gdouble dbl;
      gint64 int64;
  } prev;
  union {
      gchar *str;
      struct {
          gint len;
          gchar **data;
      } array;
  } allocated;
} Change;
typedef struct {
  gchar **ptr;
  gchar *value;
} PendingNull;
struct _GOptionContext {
  GList           *groups;
  gchar           *parameter_string;
  gchar           *summary;
  gchar           *description;
  GTranslateFunc   translate_func;
  GDestroyNotify   translate_notify;
  gpointer         translate_data;
  guint            help_enabled   : 1;
  guint            ignore_unknown : 1;
  GOptionGroup    *main_group;
  GList           *changes;
  GList           *pending_nulls;
};
struct _GOptionGroup {
  gchar           *name;
  gchar           *description;
  gchar           *help_description;
  GDestroyNotify   destroy_notify;
  gpointer         user_data;
  GTranslateFunc   translate_func;
  GDestroyNotify   translate_notify;
  gpointer         translate_data;
  GOptionEntry    *entries;
  gint             n_entries;
  GOptionParseFunc pre_parse_func;
  GOptionParseFunc post_parse_func;
  GOptionErrorFunc error_func;
};
static void free_changes_list(GOptionContext *context, gboolean revert);
static void free_pending_nulls(GOptionContext *context, gboolean perform_nulls);
static int _g_unichar_get_width(gunichar c) {
  if (G_UNLIKELY(g_unichar_iszerowidth(c))) return 0;
  if (g_unichar_iswide(c)) return 2;
  return 1;
}
static glong _g_utf8_strwidth(const gchar *p, gssize max) {
  glong len = 0;
  const gchar *start = p;
  g_return_val_if_fail(p != NULL || max == 0, 0);
  if (max < 0) {
      while(*p) {
          len += _g_unichar_get_width(g_utf8_get_char (p));
          p = g_utf8_next_char(p);
      }
  } else {
      if (max == 0 || !*p) return 0;
      len += _g_unichar_get_width(g_utf8_get_char(p));
      p = g_utf8_next_char(p);
      while(p - start < max && *p) {
          len += _g_unichar_get_width(g_utf8_get_char(p));
          p = g_utf8_next_char(p);
      }
  }
  return len;
}
GQuark g_option_error_quark(void) {
  return g_quark_from_static_string("g-option-context-error-quark");
}
GOptionContext* g_option_context_new(const gchar *parameter_string) {
  GOptionContext *context;
  context = g_new0(GOptionContext, 1);
  context->parameter_string = g_strdup(parameter_string);
  context->help_enabled = TRUE;
  context->ignore_unknown = FALSE;
  return context;
}
void g_option_context_free(GOptionContext *context) {
  g_return_if_fail(context != NULL);
  g_list_foreach(context->groups, (GFunc)g_option_group_free, NULL);
  g_list_free(context->groups);
  if (context->main_group) g_option_group_free(context->main_group);
  free_changes_list(context, FALSE);
  free_pending_nulls(context, FALSE);
  g_free(context->parameter_string);
  g_free(context->summary);
  g_free(context->description);
  if (context->translate_notify) (*context->translate_notify)(context->translate_data);
  g_free(context);
}
void g_option_context_set_help_enabled(GOptionContext *context, gboolean help_enabled) {
  g_return_if_fail(context != NULL);
  context->help_enabled = help_enabled;
}
gboolean g_option_context_get_help_enabled(GOptionContext *context) {
  g_return_val_if_fail(context != NULL, FALSE);
  return context->help_enabled;
}
void
g_option_context_set_ignore_unknown_options(GOptionContext *context, gboolean ignore_unknown) {
  g_return_if_fail(context != NULL);
  context->ignore_unknown = ignore_unknown;
}
gboolean g_option_context_get_ignore_unknown_options(GOptionContext *context) {
  g_return_val_if_fail(context != NULL, FALSE);
  return context->ignore_unknown;
}
void g_option_context_add_group(GOptionContext *context, GOptionGroup *group) {
  GList *list;
  g_return_if_fail(context != NULL);
  g_return_if_fail(group != NULL);
  g_return_if_fail(group->name != NULL);
  g_return_if_fail(group->description != NULL);
  g_return_if_fail(group->help_description != NULL);
  for (list = context->groups; list; list = list->next) {
      GOptionGroup *g = (GOptionGroup *)list->data;
      if ((group->name == NULL && g->name == NULL) || (group->name && g->name && strcmp(group->name, g->name) == 0))
          g_warning("A group named \"%s\" is already part of this GOptionContext", group->name);
  }
  context->groups = g_list_append(context->groups, group);
}
void g_option_context_set_main_group(GOptionContext *context, GOptionGroup *group) {
  g_return_if_fail(context != NULL);
  g_return_if_fail(group != NULL);
  if (context->main_group) {
      g_warning("This GOptionContext already has a main group");
      return;
  }
  context->main_group = group;
}
GOptionGroup* g_option_context_get_main_group(GOptionContext *context) {
  g_return_val_if_fail(context != NULL, NULL);
  return context->main_group;
}
void g_option_context_add_main_entries(GOptionContext *context, const GOptionEntry *entries, const gchar *translation_domain) {
  g_return_if_fail(entries != NULL);
  if (!context->main_group) context->main_group = g_option_group_new(NULL, NULL, NULL, NULL, NULL);
  g_option_group_add_entries(context->main_group, entries);
  g_option_group_set_translation_domain(context->main_group, translation_domain);
}
static gint calculate_max_length(GOptionGroup *group) {
  GOptionEntry *entry;
  gint i, len, max_length;
  max_length = 0;
  for (i = 0; i < group->n_entries; i++) {
      entry = &group->entries[i];
      if (entry->flags & G_OPTION_FLAG_HIDDEN) continue;
      len = _g_utf8_strwidth(entry->long_name, -1);
      if (entry->short_name) len += 4;
      if (!NO_ARG(entry) && entry->arg_description) len += 1 + _g_utf8_strwidth(TRANSLATE(group, entry->arg_description), -1);
      max_length = MAX(max_length, len);
  }
  return max_length;
}
static void print_entry(GOptionGroup *group, gint max_length, const GOptionEntry *entry, GString *string) {
  GString *str;
  if (entry->flags & G_OPTION_FLAG_HIDDEN) return;
  if (entry->long_name[0] == 0) return;
  str = g_string_new(NULL);
  if (entry->short_name) g_string_append_printf(str, "  -%c, --%s", entry->short_name, entry->long_name);
  else g_string_append_printf(str, "  --%s", entry->long_name);
  if (entry->arg_description) g_string_append_printf(str, "=%s", TRANSLATE(group, entry->arg_description));
  g_string_append_printf(string, "%s%*s %s\n", str->str, (int)(max_length + 4 - _g_utf8_strwidth (str->str, -1)), "",
                         entry->description ? TRANSLATE(group, entry->description) : "");
  g_string_free(str, TRUE);
}
static gboolean group_has_visible_entries(GOptionContext *context, GOptionGroup *group, gboolean main_entries) {
  GOptionFlags reject_filter = G_OPTION_FLAG_HIDDEN;
  GOptionEntry *entry;
  gint i, l;
  gboolean main_group = group == context->main_group;
  if (!main_entries) reject_filter |= G_OPTION_FLAG_IN_MAIN;
  for (i = 0, l = (group ? group->n_entries : 0); i < l; i++) {
      entry = &group->entries[i];
      if (main_entries && !main_group && !(entry->flags & G_OPTION_FLAG_IN_MAIN)) continue;
      if (!(entry->flags & reject_filter)) return TRUE;
  }
  return FALSE;
}
static gboolean group_list_has_visible_entires(GOptionContext *context, GList *group_list, gboolean main_entries) {
  while(group_list) {
      if (group_has_visible_entries(context, group_list->data, main_entries)) return TRUE;
      group_list = group_list->next;
  }
  return FALSE;
}
static gboolean context_has_h_entry(GOptionContext *context) {
  gsize i;
  GList *list;
  if (context->main_group) {
      for (i = 0; i < context->main_group->n_entries; i++) {
          if (context->main_group->entries[i].short_name == 'h') return TRUE;
      }
  }
  for (list = context->groups; list != NULL; list = g_list_next (list)) {
     GOptionGroup *group;
      group = (GOptionGroup*)list->data;
      for (i = 0; i < group->n_entries; i++) {
          if (group->entries[i].short_name == 'h') return TRUE;
      }
  }
  return FALSE;
}
gchar* g_option_context_get_help(GOptionContext *context, gboolean  main_help, GOptionGroup *group) {
  GList *list;
  gint max_length, len;
  gint i;
  GOptionEntry *entry;
  GHashTable *shadow_map;
  gboolean seen[256];
  const gchar *rest_description;
  GString *string;
  guchar token;
  string = g_string_sized_new(1024);
  rest_description = NULL;
  if (context->main_group) {
      for (i = 0; i < context->main_group->n_entries; i++) {
          entry = &context->main_group->entries[i];
          if (entry->long_name[0] == 0) {
              rest_description = TRANSLATE(context->main_group, entry->arg_description);
              break;
          }
      }
  }
  g_string_append_printf(string, "%s\n  %s %s", _("Usage:"), g_get_prgname(), _("[OPTION...]"));
  if (rest_description) {
      g_string_append(string, " ");
      g_string_append(string, rest_description);
  }
  if (context->parameter_string) {
      g_string_append(string, " ");
      g_string_append(string, TRANSLATE(context, context->parameter_string));
  }
  g_string_append(string, "\n\n");
  if (context->summary) {
      g_string_append(string, TRANSLATE(context, context->summary));
      g_string_append(string, "\n\n");
  }
  memset(seen, 0, sizeof(gboolean) * 256);
  shadow_map = g_hash_table_new(g_str_hash, g_str_equal);

  if (context->main_group) {
      for (i = 0; i < context->main_group->n_entries; i++) {
          entry = &context->main_group->entries[i];
          g_hash_table_insert(shadow_map, (gpointer)entry->long_name, entry);
          if (seen[(guchar)entry->short_name]) entry->short_name = 0;
          else seen[(guchar)entry->short_name] = TRUE;
      }
  }
  list = context->groups;
  while (list != NULL) {
      GOptionGroup *g = list->data;
      for (i = 0; i < g->n_entries; i++) {
          entry = &g->entries[i];
          if (g_hash_table_lookup (shadow_map, entry->long_name) && !(entry->flags & G_OPTION_FLAG_NOALIAS)) {
              entry->long_name = g_strdup_printf ("%s-%s", g->name, entry->long_name);
          } else g_hash_table_insert(shadow_map,(gpointer)entry->long_name, entry);
          if (seen[(guchar)entry->short_name] && !(entry->flags & G_OPTION_FLAG_NOALIAS)) entry->short_name = 0;
          else seen[(guchar)entry->short_name] = TRUE;
      }
      list = list->next;
  }
  g_hash_table_destroy(shadow_map);
  list = context->groups;
  max_length = _g_utf8_strwidth("-?, --help", -1);
  if (list) {
      len = _g_utf8_strwidth("--help-all", -1);
      max_length = MAX(max_length, len);
  }
  if (context->main_group) {
      len = calculate_max_length(context->main_group);
      max_length = MAX(max_length, len);
  }
  while (list != NULL) {
      GOptionGroup *g = list->data;
      len = _g_utf8_strwidth("--help-", -1) + _g_utf8_strwidth(g->name, -1);
      max_length = MAX(max_length, len);
      len = calculate_max_length(g);
      max_length = MAX(max_length, len);
      list = list->next;
  }
  max_length += 4;
  if (!group) {
      list = context->groups;
      token = context_has_h_entry(context) ? '?' : 'h';
      g_string_append_printf(string, "%s\n  -%c, --%-*s %s\n", _("Help Options:"), token, max_length - 4, "help", _("Show help options"));
      if (list) g_string_append_printf(string, "  --%-*s %s\n", max_length, "help-all", _("Show all help options"));
      while (list) {
          GOptionGroup *g = list->data;
          if (group_has_visible_entries(context, g, FALSE))
              g_string_append_printf(string, "  --help-%-*s %s\n", max_length - 5, g->name, TRANSLATE (g, g->help_description));
          list = list->next;
      }
      g_string_append(string, "\n");
  }
  if (group) {
      if (group_has_visible_entries (context, group, FALSE)) {
          g_string_append(string, TRANSLATE(group, group->description));
          g_string_append(string, "\n");
          for (i = 0; i < group->n_entries; i++) print_entry(group, max_length, &group->entries[i], string);
          g_string_append (string, "\n");
      }
  } else if (!main_help) {
      list = context->groups;
      while(list) {
          GOptionGroup *g = list->data;
          if (group_has_visible_entries(context, g, FALSE)) {
              g_string_append(string, g->description);
              g_string_append(string, "\n");
              for (i = 0; i < g->n_entries; i++)
                  if (!(g->entries[i].flags & G_OPTION_FLAG_IN_MAIN)) print_entry(g, max_length, &g->entries[i], string);
              g_string_append(string, "\n");
          }
          list = list->next;
      }
  }
  if ((main_help || !group) && (group_has_visible_entries(context, context->main_group, TRUE) || group_list_has_visible_entires(context, context->groups, TRUE))) {
      list = context->groups;
      g_string_append(string,  _("Application Options:"));
      g_string_append(string, "\n");
      if (context->main_group)
          for (i = 0; i < context->main_group->n_entries; i++) print_entry(context->main_group, max_length, &context->main_group->entries[i], string);
      while(list != NULL) {
          GOptionGroup *g = list->data;
          for (i = 0; i < g->n_entries; i++)
              if (g->entries[i].flags & G_OPTION_FLAG_IN_MAIN) print_entry(g, max_length, &g->entries[i], string);
          list = list->next;
      }
      g_string_append(string, "\n");
  }
  if (context->description) {
      g_string_append(string, TRANSLATE(context, context->description));
      g_string_append(string, "\n");
  }
  return g_string_free(string, FALSE);
}
G_GNUC_NORETURN static void print_help(GOptionContext *context,  gboolean main_help, GOptionGroup *group) {
  gchar *help;
  help = g_option_context_get_help(context, main_help, group);
  g_print("%s", help);
  g_free(help);
  exit(0);
}
static gboolean parse_int(const gchar *arg_name, const gchar *arg, gint *result, GError **error) {
  gchar *end;
  glong tmp;
  errno = 0;
  tmp = strtol(arg, &end, 0);
  if (*arg == '\0' || *end != '\0') {
      g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, _("Cannot parse integer value '%s' for %s"), arg, arg_name);
      return FALSE;
  }
  *result = tmp;
  if (*result != tmp || errno == ERANGE) {
      g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, _("Integer value '%s' for %s out of range"), arg, arg_name);
      return FALSE;
  }
  return TRUE;
}
static gboolean
parse_double (const gchar *arg_name, const gchar *arg, gdouble *result, GError **error) {
  gchar *end;
  gdouble tmp;
  errno = 0;
  tmp = g_strtod (arg, &end);
  if (*arg == '\0' || *end != '\0') {
      g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, _("Cannot parse double value '%s' for %s"), arg, arg_name);
      return FALSE;
  }
  if (errno == ERANGE) {
      g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, _("Double value '%s' for %s out of range"), arg, arg_name);
      return FALSE;
  }
  *result = tmp;
  return TRUE;
}
static gboolean parse_int64(const gchar *arg_name, const gchar *arg, gint64 *result, GError **error) {
  gchar *end;
  gint64 tmp;
  errno = 0;
  tmp = g_ascii_strtoll (arg, &end, 0);
  if (*arg == '\0' || *end != '\0') {
      g_set_error (error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, _("Cannot parse integer value '%s' for %s"), arg, arg_name);
      return FALSE;
  }
  if (errno == ERANGE) {
      g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, _("Integer value '%s' for %s out of range"), arg, arg_name);
      return FALSE;
  }
  *result = tmp;
  return TRUE;
}
static Change* get_change(GOptionContext *context, GOptionArg arg_type, gpointer arg_data) {
  GList *list;
  Change *change = NULL;
  for (list = context->changes; list != NULL; list = list->next) {
      change = list->data;
      if (change->arg_data == arg_data) goto found;
  }
  change = g_new0(Change, 1);
  change->arg_type = arg_type;
  change->arg_data = arg_data;
  context->changes = g_list_prepend(context->changes, change);
  found:
  return change;
}
static void add_pending_null(GOptionContext *context, gchar **ptr, gchar *value) {
  PendingNull *n;
  n = g_new0 (PendingNull, 1);
  n->ptr = ptr;
  n->value = value;
  context->pending_nulls = g_list_prepend(context->pending_nulls, n);
}
static gboolean parse_arg(GOptionContext *context, GOptionGroup *group, GOptionEntry *entry, const gchar *value, const gchar *option_name, GError **error) {
  Change *change;
  g_assert(value || OPTIONAL_ARG (entry) || NO_ARG (entry));
  switch (entry->arg) {
      case G_OPTION_ARG_NONE: {
              change = get_change (context, G_OPTION_ARG_NONE, entry->arg_data);
              *(gboolean*)entry->arg_data = !(entry->flags & G_OPTION_FLAG_REVERSE);
              break;
          }
      case G_OPTION_ARG_STRING: {
              gchar *data;
              data = g_locale_to_utf8(value, -1, NULL, NULL, error);
              if (!data) return FALSE;
              change = get_change(context, G_OPTION_ARG_STRING, entry->arg_data);
              g_free(change->allocated.str);
              change->prev.str = *(gchar **)entry->arg_data;
              change->allocated.str = data;
              *(gchar**)entry->arg_data = data;
              break;
          }
      case G_OPTION_ARG_STRING_ARRAY: {
              gchar *data;
              data = g_locale_to_utf8 (value, -1, NULL, NULL, error);
              if (!data) return FALSE;
              change = get_change (context, G_OPTION_ARG_STRING_ARRAY, entry->arg_data);
              if (change->allocated.array.len == 0) {
                  change->prev.array = *(gchar ***)entry->arg_data;
                  change->allocated.array.data = g_new(gchar*, 2);
              } else change->allocated.array.data = g_renew(gchar*, change->allocated.array.data,change->allocated.array.len + 2);
              change->allocated.array.data[change->allocated.array.len] = data;
              change->allocated.array.data[change->allocated.array.len + 1] = NULL;
              change->allocated.array.len ++;
              *(gchar***)entry->arg_data = change->allocated.array.data;
              break;
          }
      case G_OPTION_ARG_FILENAME: {
              gchar *data;
          #ifdef G_OS_WIN32
              data = g_locale_to_utf8(value, -1, NULL, NULL, error);
              if (!data) return FALSE;
          #else
              data = g_strdup(value);
          #endif
              change = get_change(context, G_OPTION_ARG_FILENAME, entry->arg_data);
              g_free (change->allocated.str);
              change->prev.str = *(gchar**)entry->arg_data;
              change->allocated.str = data;
              *(gchar**)entry->arg_data = data;
              break;
          }
      case G_OPTION_ARG_FILENAME_ARRAY: {
              gchar *data;
          #ifdef G_OS_WIN32
              data = g_locale_to_utf8(value, -1, NULL, NULL, error);
              if (!data) return FALSE;
          #else
              data = g_strdup (value);
          #endif
              change = get_change(context, G_OPTION_ARG_STRING_ARRAY, entry->arg_data);
              if (change->allocated.array.len == 0) {
                  change->prev.array = *(gchar ***)entry->arg_data;
                  change->allocated.array.data = g_new(gchar *, 2);
              } else change->allocated.array.data = g_renew(gchar*, change->allocated.array.data, change->allocated.array.len + 2);
              change->allocated.array.data[change->allocated.array.len] = data;
              change->allocated.array.data[change->allocated.array.len + 1] = NULL;
              change->allocated.array.len++;
              *(gchar***)entry->arg_data = change->allocated.array.data;
              break;
          }
      case G_OPTION_ARG_INT: {
              gint data;
              if (!parse_int(option_name, value, data, error)) return FALSE;
              change = get_change(context, G_OPTION_ARG_INT, entry->arg_data);
              change->prev.integer = *(gint*)entry->arg_data;
              *(gint*)entry->arg_data = data;
              break;
          }
      case G_OPTION_ARG_CALLBACK: {
              gchar *data;
              gboolean retval;
              if (!value && entry->flags & G_OPTION_FLAG_OPTIONAL_ARG) data = NULL;
              else if (entry->flags & G_OPTION_FLAG_NO_ARG) data = NULL;
              else if (entry->flags & G_OPTION_FLAG_FILENAME) {
              #ifdef G_OS_WIN32
                  data = g_locale_to_utf8(value, -1, NULL, NULL, error);
              #else
                  data = g_strdup (value);
              #endif
              } else data = g_locale_to_utf8(value, -1, NULL, NULL, error);
              if (!(entry->flags & (G_OPTION_FLAG_NO_ARG|G_OPTION_FLAG_OPTIONAL_ARG)) && !data) return FALSE;
              retval = (*(GOptionArgFunc)entry->arg_data)(option_name, data, group->user_data, error);
              if (!retval && error != NULL && *error == NULL)
                  g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_FAILED, _("Error parsing option %s"), option_name);
              g_free(data);
              return retval;
              break;
          }
      case G_OPTION_ARG_DOUBLE: {
              gdouble data;
              if (!parse_double (option_name, value, &data, error)) return FALSE;
              change = get_change(context, G_OPTION_ARG_DOUBLE, entry->arg_data);
              change->prev.dbl = *(gdouble*)entry->arg_data;
              *(gdouble*)entry->arg_data = data;
              break;
          }
      case G_OPTION_ARG_INT64: {
              gint64 data;
              if (!parse_int64(option_name, value, &data, error)) return FALSE;
              change = get_change(context, G_OPTION_ARG_INT64, entry->arg_data);
              change->prev.int64 = *(gint64*)entry->arg_data;
              *(gint64*)entry->arg_data = data;
              break;
          }
      default: g_assert_not_reached();
  }
  return TRUE;
}
static gboolean parse_short_option(GOptionContext *context, GOptionGroup *group, gint idx, gint *new_idx, gchar arg, gint *argc, gchar ***argv, GError **error,
                                   gboolean *parsed) {
  gint j;
  for (j = 0; j < group->n_entries; j++) {
      if (arg == group->entries[j].short_name) {
          gchar *option_name;
          gchar *value = NULL;
          option_name = g_strdup_printf("-%c", group->entries[j].short_name);
          if (NO_ARG(&group->entries[j])) value = NULL;
          else {
              if (*new_idx > idx) {
                  g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_FAILED, _("Error parsing option %s"), option_name);
                  g_free(option_name);
                  return FALSE;
              }
              if (idx < *argc - 1) {
                  if (!OPTIONAL_ARG(&group->entries[j])) {
                      value = (*argv)[idx + 1];
                      add_pending_null(context, &((*argv)[idx + 1]), NULL);
                      *new_idx = idx + 1;
                  } else {
                      if ((*argv)[idx + 1][0] == '-') value = NULL;
                      else {
                          value = (*argv)[idx + 1];
                          add_pending_null(context, &((*argv)[idx + 1]), NULL);
                          *new_idx = idx + 1;
                      }
                  }
              } else if (idx >= *argc - 1 && OPTIONAL_ARG(&group->entries[j])) value = NULL;
              else {
                  g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, _("Missing argument for %s"), option_name);
                  g_free(option_name);
                  return FALSE;
              }
          }
          if (!parse_arg(context, group, &group->entries[j], value, option_name, error)) {
              g_free(option_name);
              return FALSE;
          }
          g_free(option_name);
          *parsed = TRUE;
      }
  }
  return TRUE;
}
static gboolean parse_long_option(GOptionContext *context, GOptionGroup *group, gint *idx, gchar *arg, gboolean aliased, gint *argc, gchar ***argv,
                                  GError **error, gboolean *parsed) {
  gint j;
  for (j = 0; j < group->n_entries; j++) {
      if (*idx >= *argc) return TRUE;
      if (aliased && (group->entries[j].flags & G_OPTION_FLAG_NOALIAS)) continue;
      if (NO_ARG (&group->entries[j]) && strcmp (arg, group->entries[j].long_name) == 0) {
          gchar *option_name;
          gboolean retval;
          option_name = g_strconcat("--", group->entries[j].long_name, NULL);
          retval = parse_arg(context, group, &group->entries[j],NULL, option_name, error);
          g_free(option_name);
          add_pending_null(context, &((*argv)[*idx]), NULL);
          *parsed = TRUE;
          return retval;
      } else {
          gint len = strlen(group->entries[j].long_name);
          if (strncmp(arg, group->entries[j].long_name, len) == 0 && (arg[len] == '=' || arg[len] == 0)) {
              gchar *value = NULL;
              gchar *option_name;
              add_pending_null(context, &((*argv)[*idx]), NULL);
              option_name = g_strconcat("--", group->entries[j].long_name, NULL);
              if (arg[len] == '=') value = arg + len + 1;
              else if (*idx < *argc - 1) {
                  if (!(group->entries[j].flags & G_OPTION_FLAG_OPTIONAL_ARG)) {
                      value = (*argv)[*idx + 1];
                      add_pending_null(context, &((*argv)[*idx + 1]), NULL);
                      (*idx)++;
                  } else {
                      if ((*argv)[*idx + 1][0] == '-') {
                          gboolean retval;
                          retval = parse_arg(context, group, &group->entries[j], NULL, option_name, error);
                          *parsed = TRUE;
                          g_free(option_name);
                          return retval;
                      } else {
                          value = (*argv)[*idx + 1];
                          add_pending_null(context, &((*argv)[*idx + 1]), NULL);
                          (*idx)++;
                      }
                  }
              } else if (*idx >= *argc - 1 && group->entries[j].flags & G_OPTION_FLAG_OPTIONAL_ARG) {
                    gboolean retval;
                    retval = parse_arg(context, group, &group->entries[j],NULL, option_name, error);
                    *parsed = TRUE;
                    g_free(option_name);
                    return retval;
              } else{
                  g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, _("Missing argument for %s"), option_name);
                  g_free(option_name);
                  return FALSE;
              }
              if (!parse_arg(context, group, &group->entries[j], value, option_name, error)) {
                  g_free(option_name);
                  return FALSE;
              }
              g_free(option_name);
              *parsed = TRUE;
          }
      }
  }
  return TRUE;
}
static gboolean parse_remaining_arg(GOptionContext *context, GOptionGroup *group, gint *idx, gint *argc, gchar ***argv, GError **error, gboolean *parsed) {
  gint j;
  for (j = 0; j < group->n_entries; j++) {
      if (*idx >= *argc) return TRUE;
      if (group->entries[j].long_name[0]) continue;
      g_return_val_if_fail(group->entries[j].arg == G_OPTION_ARG_CALLBACK || group->entries[j].arg == G_OPTION_ARG_STRING_ARRAY ||
                            group->entries[j].arg == G_OPTION_ARG_FILENAME_ARRAY, FALSE);
      add_pending_null(context, &((*argv)[*idx]), NULL);
      if (!parse_arg(context, group, &group->entries[j], (*argv)[*idx], "", error)) return FALSE;
      *parsed = TRUE;
      return TRUE;
  }
  return TRUE;
}
static void free_changes_list(GOptionContext *context, gboolean revert) {
  GList *list;
  for (list = context->changes; list != NULL; list = list->next) {
      Change *change = list->data;
      if (revert) {
          switch(change->arg_type) {
              case G_OPTION_ARG_NONE: *(gboolean*)change->arg_data = change->prev.bool; break;
              case G_OPTION_ARG_INT: *(gint*)change->arg_data = change->prev.integer; break;
              case G_OPTION_ARG_STRING:
              case G_OPTION_ARG_FILENAME:
                  g_free (change->allocated.str);
                  *(gchar**)change->arg_data = change->prev.str;
                  break;
              case G_OPTION_ARG_STRING_ARRAY:
              case G_OPTION_ARG_FILENAME_ARRAY:
                  g_strfreev (change->allocated.array.data);
                  *(gchar***)change->arg_data = change->prev.array;
                  break;
              case G_OPTION_ARG_DOUBLE: *(gdouble*)change->arg_data = change->prev.dbl; break;
              case G_OPTION_ARG_INT64: *(gint64*)change->arg_data = change->prev.int64; break;
              default: g_assert_not_reached();
          }
      }
      g_free (change);
  }
  g_list_free (context->changes);
  context->changes = NULL;
}
static void free_pending_nulls(GOptionContext *context, gboolean perform_nulls) {
  GList *list;
  for (list = context->pending_nulls; list != NULL; list = list->next) {
      PendingNull *n = list->data;
      if (perform_nulls) {
          if (n->value) {
              *(n->ptr)[0] = '-';
              strcpy (*n->ptr + 1, n->value);
          } else *n->ptr = NULL;
      }
      g_free(n->value);
      g_free(n);
  }
  g_list_free(context->pending_nulls);
  context->pending_nulls = NULL;
}
gboolean g_option_context_parse(GOptionContext *context, gint *argc, gchar ***argv, GError **error) {
  gint i, j, k;
  GList *list;
  if (!g_get_prgname()) {
      if (argc && argv && *argc) {
          gchar *prgname;
          prgname = g_path_get_basename((*argv)[0]);
          g_set_prgname(prgname);
          g_free(prgname);
      } else g_set_prgname("<unknown>");
  }
  list = context->groups;
  while(list) {
      GOptionGroup *group = list->data;
      if (group->pre_parse_func) {
          if (!(* group->pre_parse_func)(context, group, group->user_data, error)) goto fail;
      }
      list = list->next;
  }
  if (context->main_group && context->main_group->pre_parse_func) {
      if (!(*context->main_group->pre_parse_func)(context, context->main_group, context->main_group->user_data, error)) goto fail;
  }
  if (argc && argv) {
      gboolean stop_parsing = FALSE;
      gboolean has_unknown = FALSE;
      gint separator_pos = 0;
      for (i = 1; i < *argc; i++) {
          gchar *arg, *dash;
          gboolean parsed = FALSE;
          if ((*argv)[i][0] == '-' && (*argv)[i][1] != '\0' && !stop_parsing) {
              if ((*argv)[i][1] == '-') {
                  arg = (*argv)[i] + 2;
                  if (*arg == 0) {
                      separator_pos = i;
                      stop_parsing = TRUE;
                      continue;
                  }
                  if (context->help_enabled) {
                      if (strcmp(arg, "help") == 0) print_help (context, TRUE, NULL);
                      else if (strcmp(arg, "help-all") == 0) print_help(context, FALSE, NULL);
                      else if (strncmp(arg, "help-", 5) == 0) {
                          list = context->groups;
                          while(list) {
                              GOptionGroup *group = list->data;
                              if (strcmp(arg + 5, group->name) == 0) print_help(context, FALSE, group);
                              list = list->next;
                          }
                      }
                  }
                  if (context->main_group && !parse_long_option(context, context->main_group, &i, arg, FALSE, argc, argv, error, &parsed)) goto fail;
                  if (parsed) continue;
                  list = context->groups;
                  while(list) {
                      GOptionGroup *group = list->data;
                      if (!parse_long_option(context, group, &i, arg, FALSE, argc, argv, error, &parsed)) goto fail;
                      if (parsed) break;
                      list = list->next;
                  }
                  if (parsed) continue;
                  dash = strchr (arg, '-');
                  if (dash) {
                      list = context->groups;
                      while(list) {
                          GOptionGroup *group = list->data;
                          if (strncmp(group->name, arg, dash - arg) == 0) {
                              if (!parse_long_option(context, group, &i, dash + 1, TRUE, argc, argv, error, &parsed)) goto fail;
                              if (parsed) break;
                          }
                          list = list->next;
                      }
                  }
                  if (context->ignore_unknown) continue;
              } else {
                  gint new_i = i, arg_length;
                  gboolean *nulled_out = NULL;
                  gboolean has_h_entry = context_has_h_entry(context);
                  arg = (*argv)[i] + 1;
                  arg_length = strlen(arg);
                  nulled_out = g_newa(gboolean, arg_length);
                  memset(nulled_out, 0, arg_length * sizeof(gboolean));
                  for (j = 0; j < arg_length; j++) {
                      if (context->help_enabled && (arg[j] == '?' || (arg[j] == 'h' && !has_h_entry))) print_help(context, TRUE, NULL);
                      parsed = FALSE;
                      if (context->main_group && !parse_short_option(context, context->main_group, i, &new_i, arg[j], argc, argv, error, &parsed)) goto fail;
                      if (!parsed) {
                          list = context->groups;
                          while(list) {
                              GOptionGroup *group = list->data;
                              if (!parse_short_option(context, group, i, &new_i, arg[j], argc, argv, error, &parsed)) goto fail;
                              if (parsed) break;
                              list = list->next;
                          }
                      }
                      if (context->ignore_unknown && parsed) nulled_out[j] = TRUE;
                      else if (context->ignore_unknown) continue;
                      else if (!parsed) break;
                  }
                  if (context->ignore_unknown) {
                      gchar *new_arg = NULL;
                      gint arg_index = 0;
                      for (j = 0; j < arg_length; j++) {
                          if (!nulled_out[j]) {
                              if (!new_arg) new_arg = g_malloc(arg_length + 1);
                              new_arg[arg_index++] = arg[j];
                          }
                      }
                      if (new_arg) new_arg[arg_index] = '\0';
                      add_pending_null(context, &((*argv)[i]), new_arg);
                  } else if (parsed) {
                      add_pending_null(context, &((*argv)[i]), NULL);
                      i = new_i;
                  }
              }
              if (!parsed) has_unknown = TRUE;
              if (!parsed && !context->ignore_unknown) {
                  g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_UNKNOWN_OPTION, _("Unknown option %s"), (*argv)[i]);
                  goto fail;
              }
          } else {
              if (context->main_group && !parse_remaining_arg (context, context->main_group, &i, argc, argv, error, &parsed)) goto fail;
              if (!parsed && (has_unknown || (*argv)[i][0] == '-')) separator_pos = 0;
          }
      }
      if (separator_pos > 0) add_pending_null(context, &((*argv)[separator_pos]), NULL);
  }
  list = context->groups;
  while(list) {
      GOptionGroup *group = list->data;
      if (group->post_parse_func) {
          if (!(* group->post_parse_func)(context, group, group->user_data, error)) goto fail;
      }
      list = list->next;
  }
  if (context->main_group && context->main_group->post_parse_func) {
      if (!(* context->main_group->post_parse_func)(context, context->main_group, context->main_group->user_data, error)) goto fail;
  }
  if (argc && argv) {
      free_pending_nulls(context, TRUE);
      for (i = 1; i < *argc; i++) {
          for (k = i; k < *argc; k++)
            if ((*argv)[k] != NULL) break;
          if (k > i) {
              k -= i;
              for (j = i + k; j < *argc; j++) {
                  (*argv)[j-k] = (*argv)[j];
                  (*argv)[j] = NULL;
              }
              *argc -= k;
          }
      }
  }
  return TRUE;
  fail:
  list = context->groups;
  while(list) {
      GOptionGroup *group = list->data;
      if (group->error_func) (*group->error_func)(context, group, group->user_data, error);
      list = list->next;
  }
  if (context->main_group && context->main_group->error_func) (*context->main_group->error_func)(context, context->main_group, context->main_group->user_data, error);
  free_changes_list(context, TRUE);
  free_pending_nulls(context, FALSE);
  return FALSE;
}
GOptionGroup* g_option_group_new(const gchar *name, const gchar *description, const gchar *help_description, gpointer user_data, GDestroyNotify  destroy) {
  GOptionGroup *group;
  group = g_new0(GOptionGroup, 1);
  group->name = g_strdup(name);
  group->description = g_strdup(description);
  group->help_description = g_strdup(help_description);
  group->user_data = user_data;
  group->destroy_notify = destroy;
  return group;
}
void g_option_group_free(GOptionGroup *group) {
  g_return_if_fail(group != NULL);
  g_free(group->name);
  g_free(group->description);
  g_free(group->help_description);
  g_free(group->entries);
  if (group->destroy_notify) (*group->destroy_notify)(group->user_data);
  if (group->translate_notify) (*group->translate_notify)(group->translate_data);
  g_free(group);
}
void g_option_group_add_entries(GOptionGroup *group, const GOptionEntry *entries) {
  gint i, n_entries;
  g_return_if_fail(entries != NULL);
  for (n_entries = 0; entries[n_entries].long_name != NULL; n_entries++);
  group->entries = g_renew(GOptionEntry, group->entries, group->n_entries + n_entries);
  memcpy(group->entries + group->n_entries, entries, sizeof(GOptionEntry) * n_entries);
  for (i = group->n_entries; i < group->n_entries + n_entries; i++) {
      gchar c = group->entries[i].short_name;
      if (c) {
          if (c == '-' || !g_ascii_isprint(c)) {
              g_warning(G_STRLOC": ignoring invalid short option '%c' (%d)", c, c);
              group->entries[i].short_name = 0;
          }
      }
  }
  group->n_entries += n_entries;
}
void g_option_group_set_parse_hooks(GOptionGroup *group, GOptionParseFunc pre_parse_func, GOptionParseFunc post_parse_func) {
  g_return_if_fail(group != NULL);
  group->pre_parse_func = pre_parse_func;
  group->post_parse_func = post_parse_func;
}
void g_option_group_set_error_hook(GOptionGroup *group, GOptionErrorFunc  error_func) {
  g_return_if_fail(group != NULL);
  group->error_func = error_func;
}
void g_option_group_set_translate_func(GOptionGroup *group, GTranslateFunc func, gpointer data, GDestroyNotify destroy_notify) {
  g_return_if_fail(group != NULL);
  if (group->translate_notify) group->translate_notify(group->translate_data);
  group->translate_func = func;
  group->translate_data = data;
  group->translate_notify = destroy_notify;
}
static const gchar* dgettext_swapped(const gchar *msgid, const gchar *domainname) {
  return g_dgettext(domainname, msgid);
}
void g_option_group_set_translation_domain(GOptionGroup *group, const gchar *domain) {
  g_return_if_fail(group != NULL);
  g_option_group_set_translate_func(group, (GTranslateFunc)dgettext_swapped, g_strdup(domain), g_free);
}
void g_option_context_set_translate_func(GOptionContext *context, GTranslateFunc func, gpointer data, GDestroyNotify destroy_notify) {
  g_return_if_fail(context != NULL);
  if (context->translate_notify) context->translate_notify(context->translate_data);
  context->translate_func = func;
  context->translate_data = data;
  context->translate_notify = destroy_notify;
}
void
g_option_context_set_translation_domain(GOptionContext *context, const gchar *domain) {
  g_return_if_fail(context != NULL);
  g_option_context_set_translate_func(context, (GTranslateFunc)dgettext_swapped, g_strdup(domain), g_free);
}
void g_option_context_set_summary(GOptionContext *context, const gchar *summary) {
  g_return_if_fail(context != NULL);
  g_free(context->summary);
  context->summary = g_strdup(summary);
}
G_CONST_RETURN gchar* g_option_context_get_summary(GOptionContext *context) {
  g_return_val_if_fail(context != NULL, NULL);
  return context->summary;
}
void g_option_context_set_description(GOptionContext *context, const gchar *description) {
  g_return_if_fail(context != NULL);
  g_free (context->description);
  context->description = g_strdup(description);
}
G_CONST_RETURN gchar* g_option_context_get_description(GOptionContext *context) {
  g_return_val_if_fail(context != NULL, NULL);
  return context->description;
}