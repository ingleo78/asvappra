#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef G_OS_WIN32
#include <io.h>
#define fstat(a,b) _fstati64(a,b)
#define stat _stati64
#ifndef S_ISREG
#define S_ISREG(mode) ((mode)&_S_IFREG)
#endif
#endif
#include "gkeyfile.h"
#include "gutils.h"
#include "gtypes.h"
#include "gconvert.h"
#include "gdataset.h"
#include "gerror.h"
#include "gfileutils.h"
#include "ghash.h"
#include "glibintl.h"
#include "glist.h"
#include "gslist.h"
#include "gmem.h"
#include "gmessages.h"
#include "gstdio.h"
#include "gstring.h"
#include "gutils.h"
#include "gstrfuncs.h"

#define gchar char
#define gboolean gint
typedef struct _GKeyFileGroup GKeyFileGroup;
struct _GKeyFile {
  GList *groups;
  GHashTable *group_hash;
  GKeyFileGroup *start_group;
  GKeyFileGroup *current_group;
  GString *parse_buffer;
  gsize approximate_size;
  gchar list_separator;
  GKeyFileFlags flags;
  gchar **locales;
};
typedef struct _GKeyFileKeyValuePair GKeyFileKeyValuePair;
struct _GKeyFileGroup {
  const gchar *name;
  GKeyFileKeyValuePair *comment;
  gboolean has_trailing_blank_line;
  GList *key_value_pairs;
  GHashTable *lookup_map;
};
struct _GKeyFileKeyValuePair {
  gchar *key;
  gchar *value;
};
GQuark g_key_file_error_quark(void) {
  return g_quark_from_static_string("g-key-file-error-quark");
}
void g_key_file_init(GKeyFile *key_file) {
  key_file->current_group = g_slice_new0(GKeyFileGroup);
  key_file->groups = g_list_prepend(NULL, key_file->current_group);
  key_file->group_hash = g_hash_table_new(g_str_hash, g_str_equal);
  key_file->start_group = NULL;
  key_file->parse_buffer = g_string_sized_new(128);
  key_file->approximate_size = 0;
  key_file->list_separator = ';';
  key_file->flags = 0;
  key_file->locales = g_strdupv((gchar**)g_get_language_names ());
}
void g_key_file_clear(GKeyFile *key_file) {
  GList *tmp, *group_node;
  if (key_file->locales) {
      g_strfreev(key_file->locales);
      key_file->locales = NULL;
  }
  if (key_file->parse_buffer) {
      g_string_free(key_file->parse_buffer, TRUE);
      key_file->parse_buffer = NULL;
  }
  tmp = key_file->groups;
  while(tmp != NULL) {
      group_node = tmp;
      tmp = tmp->next;
      g_key_file_remove_group_node(key_file, group_node);
  }
  g_hash_table_destroy(key_file->group_hash);
  key_file->group_hash = NULL;
  g_warn_if_fail(key_file->groups == NULL);
}
GKeyFile* g_key_file_new(void) {
  GKeyFile *key_file;
  key_file = g_slice_new0(GKeyFile);
  g_key_file_init(key_file);
  return key_file;
}
void g_key_file_set_list_separator(GKeyFile *key_file, gchar separator) {
  g_return_if_fail(key_file != NULL);
  key_file->list_separator = separator;
}
gint find_file_in_data_dirs(const gchar *file, const gchar **dirs, gchar **output_file, GError **error) {
  const gchar **data_dirs, *data_dir;
  gchar *path;
  gint fd;
  path = NULL;
  fd = -1;
  if (dirs == NULL) return fd;
  data_dirs = dirs;
  while(data_dirs && (data_dir = *data_dirs) && fd < 0) {
      gchar *candidate_file, *sub_dir;
      candidate_file = (gchar*)file;
      sub_dir = g_strdup("");
      while(candidate_file != NULL && fd < 0) {
          gchar *p;
          path = g_build_filename(data_dir, sub_dir, candidate_file, NULL);
          fd = g_open(path, O_RDONLY, 0);
          if (fd < 0) {
              g_free(path);
              path = NULL;
          }
          candidate_file = strchr(candidate_file, '-');
          if (candidate_file == NULL) break;
          candidate_file++;
          g_free(sub_dir);
          sub_dir = g_strndup(file, candidate_file - file - 1);
          for (p = sub_dir; *p != '\0'; p++) {
              if (*p == '-') *p = G_DIR_SEPARATOR;
          }
      }
      g_free(sub_dir);
      data_dirs++;
  }
  if (fd < 0) g_set_error_literal(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_NOT_FOUND, _("Valid key file could not be found in search dirs"));
  if (output_file != NULL && fd > 0) *output_file = g_strdup(path);
  g_free(path);
  return fd;
}
gboolean g_key_file_load_from_fd(GKeyFile *key_file, gint fd, GKeyFileFlags flags, GError **error) {
  GError *key_file_error = NULL;
  gssize bytes_read;
  struct stat stat_buf;
  gchar read_buf[4096];
  if (fstat(fd, &stat_buf) < 0) {
      g_set_error_literal(error, G_FILE_ERROR, g_file_error_from_errno(errno), g_strerror(errno));
      return FALSE;
  }
  if (!S_ISREG(stat_buf.st_mode)) {
      g_set_error_literal(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_PARSE, _("Not a regular file"));
      return FALSE;
  }
  if (stat_buf.st_size == 0) {
      g_set_error_literal(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_PARSE, _("File is empty"));
      return FALSE;
  }
  if (key_file->approximate_size > 0) {
      g_key_file_clear(key_file);
      g_key_file_init(key_file);
  }
  key_file->flags = flags;
  do {
      bytes_read = read(fd, read_buf, 4096);
      if (bytes_read == 0) break;
      if (bytes_read < 0) {
          if (errno == EINTR || errno == EAGAIN) continue;
          g_set_error_literal(error, G_FILE_ERROR, g_file_error_from_errno (errno), g_strerror (errno));
          return FALSE;
      }
      g_key_file_parse_data(key_file, read_buf, bytes_read, &key_file_error);
  } while(!key_file_error);
  if (key_file_error) {
      g_propagate_error(error, key_file_error);
      return FALSE;
  }
  g_key_file_flush_parse_buffer(key_file, &key_file_error);
  if (key_file_error) {
      g_propagate_error(error, key_file_error);
      return FALSE;
  }
  return TRUE;
}
gboolean g_key_file_load_from_file(GKeyFile *key_file, const gchar *file, GKeyFileFlags flags, GError **error) {
  GError *key_file_error = NULL;
  gint fd;
  g_return_val_if_fail(key_file != NULL, FALSE);
  g_return_val_if_fail(file != NULL, FALSE);
  fd = g_open (file, O_RDONLY, 0);
  if (fd < 0) {
      g_set_error_literal(error, G_FILE_ERROR, g_file_error_from_errno(errno), g_strerror(errno));
      return FALSE;
  }
  g_key_file_load_from_fd(key_file, fd, flags, &key_file_error);
  close(fd);
  if (key_file_error) {
      g_propagate_error(error, key_file_error);
      return FALSE;
  }
  return TRUE;
}
gboolean g_key_file_load_from_data (GKeyFile *key_file, const gchar *data, gsize length, GKeyFileFlags flags, GError **error) {
  GError *key_file_error = NULL;
  g_return_val_if_fail(key_file != NULL, FALSE);
  g_return_val_if_fail(data != NULL, FALSE);
  g_return_val_if_fail(length != 0, FALSE);
  if (length == (gsize)-1) length = strlen (data);
  if (key_file->approximate_size > 0) {
      g_key_file_clear(key_file);
      g_key_file_init(key_file);
  }
  key_file->flags = flags;
  g_key_file_parse_data(key_file, data, length, &key_file_error);
  if (key_file_error) {
      g_propagate_error(error, key_file_error);
      return FALSE;
  }
  g_key_file_flush_parse_buffer(key_file, &key_file_error);
  if (key_file_error) {
      g_propagate_error(error, key_file_error);
      return FALSE;
  }
  return TRUE;
}
gboolean g_key_file_load_from_dirs(GKeyFile *key_file, const gchar *file, const gchar **search_dirs, gchar **full_path, GKeyFileFlags flags, GError **error) {
  GError *key_file_error = NULL;
  const gchar **data_dirs;
  gchar *output_path;
  gint fd;
  gboolean found_file;
  g_return_val_if_fail(key_file != NULL, FALSE);
  g_return_val_if_fail(!g_path_is_absolute (file), FALSE);
  g_return_val_if_fail(search_dirs != NULL, FALSE);
  found_file = FALSE;
  data_dirs = search_dirs;
  output_path = NULL;
  while(*data_dirs != NULL && !found_file) {
      g_free(output_path);
      fd = find_file_in_data_dirs(file, data_dirs, &output_path, &key_file_error);
      if (fd < 0) {
          if (key_file_error) g_propagate_error(error, key_file_error);
 	      break;
      }
      found_file = g_key_file_load_from_fd(key_file, fd, flags, &key_file_error);
      close (fd);
      if (key_file_error) {
          g_propagate_error(error, key_file_error);
          break;
      }
  }
  if (found_file && full_path) *full_path = output_path;
  else g_free(output_path);
  return found_file;
}
gboolean g_key_file_load_from_data_dirs(GKeyFile *key_file, const gchar *file, gchar **full_path, GKeyFileFlags flags, GError **error) {
  gchar **all_data_dirs;
  const gchar* user_data_dir;
  const gchar* const* system_data_dirs;
  gsize i, j;
  gboolean found_file;
  g_return_val_if_fail(key_file != NULL, FALSE);
  g_return_val_if_fail(!g_path_is_absolute (file), FALSE);
  user_data_dir = g_get_user_data_dir();
  system_data_dirs = g_get_system_data_dirs();
  all_data_dirs = g_new(gchar *, g_strv_length((gchar**)system_data_dirs) + 2);
  i = 0;
  all_data_dirs[i++] = g_strdup(user_data_dir);
  j = 0;
  while(system_data_dirs[j] != NULL) all_data_dirs[i++] = g_strdup(system_data_dirs[j++]);
  all_data_dirs[i] = NULL;
  found_file = g_key_file_load_from_dirs(key_file, file, (const gchar**)all_data_dirs, full_path, flags, error);
  g_strfreev(all_data_dirs);
  return found_file;
}
void g_key_file_free(GKeyFile *key_file) {
  g_return_if_fail(key_file != NULL);
  g_key_file_clear(key_file);
  g_slice_free(GKeyFile, key_file);
}
gboolean g_key_file_locale_is_interesting(GKeyFile *key_file, const gchar *locale) {
  gsize i;
  if (key_file->flags & G_KEY_FILE_KEEP_TRANSLATIONS) return TRUE;
  for (i = 0; key_file->locales[i] != NULL; i++) {
      if (g_ascii_strcasecmp(key_file->locales[i], locale) == 0) return TRUE;
  }
  return FALSE;
}
void g_key_file_parse_line(GKeyFile *key_file, const gchar *line, gsize length, GError **error) {
  GError *parse_error = NULL;
  gchar *line_start;
  g_return_if_fail(key_file != NULL);
  g_return_if_fail(line != NULL);
  line_start = (gchar*)line;
  while(g_ascii_isspace(*line_start)) line_start++;
  if (g_key_file_line_is_comment (line_start)) g_key_file_parse_comment(key_file, line, length, &parse_error);
  else if (g_key_file_line_is_group(line_start)) g_key_file_parse_group(key_file, line_start,length - (line_start - line), &parse_error);
  else if (g_key_file_line_is_key_value_pair(line_start)) g_key_file_parse_key_value_pair(key_file, line_start,length - (line_start - line), &parse_error);
  else {
      gchar *line_utf8 = _g_utf8_make_valid (line);
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_PARSE, ("Key file contains line '%s' which is not a key-value pair, group, or comment"), line_utf8);
      g_free(line_utf8);
      return;
  }
  if (parse_error) g_propagate_error (error, parse_error);
}
void g_key_file_parse_comment(GKeyFile *key_file, const gchar *line, gsize length, GError **error) {
  GKeyFileKeyValuePair *pair;
  if (!(key_file->flags & G_KEY_FILE_KEEP_COMMENTS)) return;
  g_warn_if_fail(key_file->current_group != NULL);
  pair = g_slice_new(GKeyFileKeyValuePair);
  pair->key = NULL;
  pair->value = g_strndup(line, length);
  key_file->current_group->key_value_pairs = g_list_prepend(key_file->current_group->key_value_pairs, pair);
  if (length == 0 || line[0] != '#') key_file->current_group->has_trailing_blank_line = TRUE;
}
void g_key_file_parse_group(GKeyFile *key_file, const gchar *line, gsize length, GError **error) {
  gchar *group_name;
  const gchar *group_name_start, *group_name_end;
  group_name_start = line + 1;
  group_name_end = line + length - 1;
  while(*group_name_end != ']') group_name_end--;
  group_name = g_strndup(group_name_start, group_name_end - group_name_start);
  if (!g_key_file_is_group_name (group_name)) {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_PARSE, _("Invalid group name: %s"), group_name);
      g_free(group_name);
      return;
  }
  g_key_file_add_group(key_file, group_name);
  g_free(group_name);
}
void g_key_file_parse_key_value_pair(GKeyFile *key_file, const gchar *line, gsize length, GError **error) {
  gchar *key, *value, *key_end, *value_start, *locale;
  gsize key_len, value_len;
  if (key_file->current_group == NULL || key_file->current_group->name == NULL) {
      g_set_error_literal(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND, _("Key file does not start with a group"));
      return;
  }
  key_end = value_start = strchr(line, '=');
  g_warn_if_fail(key_end != NULL);
  key_end--;
  value_start++;
  while (g_ascii_isspace (*key_end)) key_end--;
  key_len = key_end - line + 2;
  g_warn_if_fail(key_len <= length);
  key = g_strndup(line, key_len - 1);
  if (!g_key_file_is_key_name(key)) {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_PARSE, _("Invalid key name: %s"), key);
      g_free(key);
      return; 
  }
  while(g_ascii_isspace(*value_start)) value_start++;
  value_len = line + length - value_start + 1;
  value = g_strndup(value_start, value_len);
  g_warn_if_fail(key_file->start_group != NULL);
  if (key_file->current_group && key_file->current_group->name && strcmp(key_file->start_group->name, key_file->current_group->name) == 0 &&
      strcmp(key, "Encoding") == 0) {
      if (g_ascii_strcasecmp(value, "UTF-8") != 0) {
	      gchar *value_utf8 = _g_utf8_make_valid(value);
          g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_UNKNOWN_ENCODING, _("Key file contains unsupported encoding '%s'"), value_utf8);
	      g_free(value_utf8);
          g_free(key);
          g_free(value);
          return;
      }
  }
  locale = key_get_locale(key);
  if (locale == NULL || g_key_file_locale_is_interesting(key_file, locale)) g_key_file_add_key(key_file, key_file->current_group, key, value);
  g_free(locale);
  g_free(key);
  g_free(value);
}
gchar* key_get_locale(const gchar *key) {
  gchar *locale;
  locale = g_strrstr(key, "[");
  if (locale && strlen(locale) <= 2)
    locale = NULL;
  if (locale) locale = g_strndup(locale + 1, strlen(locale) - 2);
  return locale;
}
void g_key_file_parse_data(GKeyFile *key_file, const gchar *data, gsize length, GError **error) {
  GError *parse_error;
  gsize i;
  g_return_if_fail(key_file != NULL);
  g_return_if_fail(data != NULL);
  parse_error = NULL;
  for (i = 0; i < length; i++) {
      if (data[i] == '\n') {
	  if (key_file->parse_buffer->len > 0 && (key_file->parse_buffer->str[key_file->parse_buffer->len - 1] == '\r'))
	    g_string_erase(key_file->parse_buffer,key_file->parse_buffer->len - 1,1);
          if (key_file->parse_buffer->len > 0) g_key_file_flush_parse_buffer(key_file, &parse_error);
          else g_key_file_parse_comment(key_file, "", 1, &parse_error);
          if (parse_error) {
              g_propagate_error(error, parse_error);
              return;
          }
      } else g_string_append_c(key_file->parse_buffer, data[i]);
  }
  key_file->approximate_size += length;
}
void g_key_file_flush_parse_buffer(GKeyFile  *key_file, GError **error) {
  GError *file_error = NULL;
  g_return_if_fail(key_file != NULL);
  file_error = NULL;
  if (key_file->parse_buffer->len > 0) {
      g_key_file_parse_line(key_file, key_file->parse_buffer->str, key_file->parse_buffer->len, &file_error);
      g_string_erase(key_file->parse_buffer, 0, -1);
      if (file_error) {
          g_propagate_error(error, file_error);
          return;
      }
  }
}
gchar* g_key_file_to_data (GKeyFile *key_file, gsize *length, GError **error) {
  GString *data_string;
  GList *group_node, *key_file_node;
  gboolean has_blank_line = TRUE;
  g_return_val_if_fail(key_file != NULL, NULL);
  data_string = g_string_sized_new(2 * key_file->approximate_size);
  for (group_node = g_list_last(key_file->groups); group_node != NULL; group_node = group_node->prev) {
      GKeyFileGroup *group;
      group = (GKeyFileGroup*)group_node->data;
      if (!has_blank_line) g_string_append_c(data_string, '\n');
      has_blank_line = group->has_trailing_blank_line;
      if (group->comment != NULL) g_string_append_printf(data_string, "%s\n", group->comment->value);
      if (group->name != NULL) g_string_append_printf(data_string, "[%s]\n", group->name);
      for (key_file_node = g_list_last(group->key_value_pairs); key_file_node != NULL; key_file_node = key_file_node->prev) {
          GKeyFileKeyValuePair *pair;
          pair = (GKeyFileKeyValuePair*)key_file_node->data;
          if (pair->key != NULL) g_string_append_printf(data_string, "%s=%s\n", pair->key, pair->value);
          else g_string_append_printf(data_string, "%s\n", pair->value);
      }
  }
  if (length) *length = data_string->len;
  return g_string_free (data_string, FALSE);
}
gchar** g_key_file_get_keys(GKeyFile *key_file, const gchar *group_name, gsize *length, GError **error) {
  GKeyFileGroup *group;
  GList *tmp;
  gchar **keys;
  gsize i, num_keys;
  g_return_val_if_fail(key_file != NULL, NULL);
  g_return_val_if_fail(group_name != NULL, NULL);
  group = g_key_file_lookup_group(key_file, group_name);
  if (!group) {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND, _("Key file does not have group '%s'"), group_name ? group_name : "(null)");
      return NULL;
  }
  num_keys = 0;
  for (tmp = group->key_value_pairs; tmp; tmp = tmp->next) {
      GKeyFileKeyValuePair *pair;
      pair = (GKeyFileKeyValuePair*)tmp->data;
      if (pair->key) num_keys++;
  }
  keys = g_new(gchar *, num_keys + 1);
  i = num_keys - 1;
  for (tmp = group->key_value_pairs; tmp; tmp = tmp->next) {
      GKeyFileKeyValuePair *pair;
      pair = (GKeyFileKeyValuePair*)tmp->data;
      if (pair->key) {
          keys[i] = g_strdup(pair->key);
          i--;
	  }
  }
  keys[num_keys] = NULL;
  if (length) *length = num_keys;
  return keys;
}
gchar* g_key_file_get_start_group(GKeyFile *key_file) {
  g_return_val_if_fail (key_file != NULL, NULL);
  if (key_file->start_group) return g_strdup(key_file->start_group->name);
  return NULL;
}
gchar** g_key_file_get_groups(GKeyFile *key_file, gsize *length) {
  GList *group_node;
  gchar **groups;
  gsize i, num_groups;
  g_return_val_if_fail(key_file != NULL, NULL);
  num_groups = g_list_length(key_file->groups);
  g_return_val_if_fail(num_groups > 0, NULL);
  group_node = g_list_last(key_file->groups);
  g_return_val_if_fail(((GKeyFileGroup*)group_node->data)->name == NULL, NULL);
  groups = g_new (gchar *, num_groups);
  i = 0;
  for (group_node = group_node->prev; group_node != NULL; group_node = group_node->prev) {
      GKeyFileGroup *group;
      group = (GKeyFileGroup*)group_node->data;
      g_warn_if_fail(group->name != NULL);
      groups[i++] = g_strdup(group->name);
  }
  groups[i] = NULL;
  if (length) *length = i;
  return groups;
}
gchar* g_key_file_get_value(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) {
  GKeyFileGroup *group;
  GKeyFileKeyValuePair *pair;
  gchar *value = NULL;
  g_return_val_if_fail(key_file != NULL, NULL);
  g_return_val_if_fail(group_name != NULL, NULL);
  g_return_val_if_fail(key != NULL, NULL);
  group = g_key_file_lookup_group(key_file, group_name);
  if (!group) {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND, _("Key file does not have group '%s'"), group_name ? group_name : "(null)");
      return NULL;
  }
  pair = g_key_file_lookup_key_value_pair(key_file, group, key);
  if (pair) value = g_strdup(pair->value);
  else g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND, _("Key file does not have key '%s'"), key);
  return value;
}
void g_key_file_set_value(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *value) {
  GKeyFileGroup *group;
  GKeyFileKeyValuePair *pair;
  g_return_if_fail(key_file != NULL);
  g_return_if_fail(g_key_file_is_group_name (group_name));
  g_return_if_fail(g_key_file_is_key_name (key));
  g_return_if_fail(value != NULL);
  group = g_key_file_lookup_group(key_file, group_name);
  if (!group) {
      g_key_file_add_group(key_file, group_name);
      group = (GKeyFileGroup*)key_file->groups->data;
      g_key_file_add_key(key_file, group, key, value);
  } else {
      pair = g_key_file_lookup_key_value_pair(key_file, group, key);
      if (!pair) g_key_file_add_key(key_file, group, key, value);
      else {
          g_free(pair->value);
          pair->value = g_strdup(value);
      }
  }
}
gchar* g_key_file_get_string(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) {
  gchar *value, *string_value;
  GError *key_file_error;
  g_return_val_if_fail(key_file != NULL, NULL);
  g_return_val_if_fail(group_name != NULL, NULL);
  g_return_val_if_fail(key != NULL, NULL);
  key_file_error = NULL;
  value = g_key_file_get_value(key_file, group_name, key, &key_file_error);
  if (key_file_error) {
      g_propagate_error(error, key_file_error);
      return NULL;
  }
  if (!g_utf8_validate(value, -1, NULL)) {
      gchar *value_utf8 = _g_utf8_make_valid(value);
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_UNKNOWN_ENCODING, _("Key file contains key '%s' with value '%s' which is not UTF-8"), key, value_utf8);
      g_free(value_utf8);
      g_free(value);
      return NULL;
  }
  string_value = g_key_file_parse_value_as_string(key_file, value, NULL, &key_file_error);
  g_free(value);
  if (key_file_error) {
      if (g_error_matches(key_file_error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE)) {
          g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE, _("Key file contains key '%s' which has value that cannot be interpreted."), key);
          g_error_free(key_file_error);
      } else g_propagate_error (error, key_file_error);
  }
  return string_value;
}
void g_key_file_set_string(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *string) {
  gchar *value;
  g_return_if_fail(key_file != NULL);
  g_return_if_fail(string != NULL);
  value = g_key_file_parse_string_as_value(key_file, string, FALSE);
  g_key_file_set_value(key_file, group_name, key, value);
  g_free(value);
}
gchar** g_key_file_get_string_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, gsize *length, GError **error) {
  GError *key_file_error = NULL;
  gchar *value, *string_value, **values;
  gint i, len;
  GSList *p, *pieces = NULL;
  g_return_val_if_fail(key_file != NULL, NULL);
  g_return_val_if_fail(group_name != NULL, NULL);
  g_return_val_if_fail(key != NULL, NULL);
  if (length) *length = 0;
  value = g_key_file_get_value(key_file, group_name, key, &key_file_error);
  if (key_file_error) {
      g_propagate_error(error, key_file_error);
      return NULL;
  }
  if (!g_utf8_validate(value, -1, NULL)) {
      gchar *value_utf8 = _g_utf8_make_valid(value);
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_UNKNOWN_ENCODING, _("Key file contains key '%s' with value '%s' which is not UTF-8"), key, value_utf8);
      g_free(value_utf8);
      g_free(value);
      return NULL;
  }
  string_value = g_key_file_parse_value_as_string(key_file, value, &pieces, &key_file_error);
  g_free(value);
  g_free(string_value);
  if (key_file_error) {
      if (g_error_matches(key_file_error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE)) {
          g_set_error (error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE, _("Key file contains key '%s' which has a value that cannot be interpreted."),
                       key);
          g_error_free(key_file_error);
      } else g_propagate_error(error, key_file_error);
      return NULL;
  }
  len = g_slist_length(pieces);
  values = g_new(gchar *, len + 1);
  for (p = pieces, i = 0; p; p = p->next) values[i++] = p->data;
  values[len] = NULL;
  g_slist_free(pieces);
  if (length) *length = len;
  return values;
}
void g_key_file_set_string_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar* const list[], gsize length) {
  GString *value_list;
  gsize i;
  g_return_if_fail(key_file != NULL);
  g_return_if_fail(list != NULL || length == 0);
  value_list = g_string_sized_new (length * 128);
  for (i = 0; i < length && list[i] != NULL; i++) {
      gchar *value;
      value = g_key_file_parse_string_as_value(key_file, list[i], TRUE);
      g_string_append(value_list, value);
      g_string_append_c(value_list, key_file->list_separator);
      g_free(value);
  }
  g_key_file_set_value(key_file, group_name, key, value_list->str);
  g_string_free(value_list, TRUE);
}
void g_key_file_set_locale_string(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *locale, const gchar *string) {
  gchar *full_key, *value;
  g_return_if_fail(key_file != NULL);
  g_return_if_fail(key != NULL);
  g_return_if_fail(locale != NULL);
  g_return_if_fail(string != NULL);
  value = g_key_file_parse_string_as_value(key_file, string, FALSE);
  full_key = g_strdup_printf("%s[%s]", key, locale);
  g_key_file_set_value(key_file, group_name, full_key, value);
  g_free(full_key);
  g_free(value);
}
gchar* g_key_file_get_locale_string(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *locale, GError **error) {
  gchar *candidate_key, *translated_value;
  GError *key_file_error;
  gchar **languages;
  gboolean free_languages = FALSE;
  gint i;
  g_return_val_if_fail(key_file != NULL, NULL);
  g_return_val_if_fail(group_name != NULL, NULL);
  g_return_val_if_fail(key != NULL, NULL);
  candidate_key = NULL;
  translated_value = NULL;
  key_file_error = NULL;
  if (locale) {
      languages = g_get_locale_variants(locale);
      free_languages = TRUE;
  } else {
      languages = (gchar**)g_get_language_names();
      free_languages = FALSE;
  }
  for (i = 0; languages[i]; i++) {
      candidate_key = g_strdup_printf("%s[%s]", key, languages[i]);
      translated_value = g_key_file_get_string(key_file, group_name, candidate_key, NULL);
      g_free(candidate_key);
      if (translated_value) break;
      g_free(translated_value);
      translated_value = NULL;
  }
  if (!translated_value) {
      translated_value = g_key_file_get_string(key_file, group_name, key, &key_file_error);
      if (!translated_value) g_propagate_error(error, key_file_error);
  }
  if (free_languages) g_strfreev(languages);
  return translated_value;
}
gchar** g_key_file_get_locale_string_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *locale, gsize *length, GError **error) {
  GError *key_file_error;
  gchar **values, *value;
  char list_separator[2];
  gsize len;
  g_return_val_if_fail(key_file != NULL, NULL);
  g_return_val_if_fail(group_name != NULL, NULL);
  g_return_val_if_fail(key != NULL, NULL);
  key_file_error = NULL;
  value = g_key_file_get_locale_string(key_file, group_name, key, locale, &key_file_error);
  if (key_file_error) g_propagate_error(error, key_file_error);
  if (!value) {
      if (length) *length = 0;
      return NULL;
  }
  len = strlen(value);
  if (value[len - 1] == key_file->list_separator) value[len - 1] = '\0';
  list_separator[0] = key_file->list_separator;
  list_separator[1] = '\0';
  values = g_strsplit(value, list_separator, 0);
  g_free(value);
  if (length) *length = g_strv_length(values);
  return values;
}
void g_key_file_set_locale_string_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *locale, const gchar* const list[], gsize length) {
  GString *value_list;
  gchar *full_key;
  gsize i;
  g_return_if_fail(key_file != NULL);
  g_return_if_fail(key != NULL);
  g_return_if_fail(locale != NULL);
  g_return_if_fail(length != 0);
  value_list = g_string_sized_new(length * 128);
  for (i = 0; i < length && list[i] != NULL; i++) {
      gchar *value;
      value = g_key_file_parse_string_as_value(key_file, list[i], TRUE);
      g_string_append(value_list, value);
      g_string_append_c(value_list, key_file->list_separator);
      g_free(value);
  }
  full_key = g_strdup_printf("%s[%s]", key, locale);
  g_key_file_set_value(key_file, group_name, full_key, value_list->str);
  g_free(full_key);
  g_string_free(value_list, TRUE);
}
gboolean g_key_file_get_boolean(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) {
  GError *key_file_error = NULL;
  gchar *value;
  gboolean bool_value;
  g_return_val_if_fail(key_file != NULL, FALSE);
  g_return_val_if_fail(group_name != NULL, FALSE);
  g_return_val_if_fail(key != NULL, FALSE);
  value = g_key_file_get_value(key_file, group_name, key, &key_file_error);
  if (!value) {
      g_propagate_error(error, key_file_error);
      return FALSE;
  }
  bool_value = g_key_file_parse_value_as_boolean(key_file, value, &key_file_error);
  g_free(value);
  if (key_file_error) {
      if (g_error_matches(key_file_error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE)) {
          g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE, _("Key file contains key '%s' which has value that cannot be interpreted."),
                      key);
          g_error_free(key_file_error);
      } else g_propagate_error(error, key_file_error);
  }
  return bool_value;
}
void g_key_file_set_boolean(GKeyFile *key_file, const gchar *group_name, const gchar *key, gboolean value) {
  gchar *result;
  g_return_if_fail(key_file != NULL);
  result = g_key_file_parse_boolean_as_value(key_file, value);
  g_key_file_set_value(key_file, group_name, key, result);
  g_free(result);
}
gboolean* g_key_file_get_boolean_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, gsize *length, GError **error) {
  GError *key_file_error;
  gchar **values;
  gboolean *bool_values;
  gsize i, num_bools;
  g_return_val_if_fail(key_file != NULL, NULL);
  g_return_val_if_fail(group_name != NULL, NULL);
  g_return_val_if_fail(key != NULL, NULL);
  if (length) *length = 0;
  key_file_error = NULL;
  values = g_key_file_get_string_list(key_file, group_name, key, &num_bools, &key_file_error);
  if (key_file_error) g_propagate_error(error, key_file_error);
  if (!values) return NULL;
  bool_values = g_new(gboolean, num_bools);
  for (i = 0; i < num_bools; i++) {
      bool_values[i] = g_key_file_parse_value_as_boolean(key_file, values[i], &key_file_error);
      if (key_file_error) {
          g_propagate_error(error, key_file_error);
          g_strfreev(values);
          g_free(bool_values);
          return NULL;
      }
  }
  g_strfreev(values);
  if (length) *length = num_bools;
  return bool_values;
}
void g_key_file_set_boolean_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, gboolean list[], gsize length) {
  GString *value_list;
  gsize i;
  g_return_if_fail(key_file != NULL);
  g_return_if_fail(list != NULL);
  value_list = g_string_sized_new(length * 8);
  for (i = 0; i < length; i++) {
      gchar *value;
      value = g_key_file_parse_boolean_as_value(key_file, list[i]);
      g_string_append(value_list, value);
      g_string_append_c(value_list, key_file->list_separator);
      g_free(value);
  }
  g_key_file_set_value(key_file, group_name, key, value_list->str);
  g_string_free(value_list, TRUE);
}
gint g_key_file_get_integer(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) {
  GError *key_file_error;
  gchar *value;
  gint int_value;
  g_return_val_if_fail(key_file != NULL, -1);
  g_return_val_if_fail(group_name != NULL, -1);
  g_return_val_if_fail(key != NULL, -1);
  key_file_error = NULL;
  value = g_key_file_get_value(key_file, group_name, key, &key_file_error);
  if (key_file_error) {
      g_propagate_error(error, key_file_error);
      return 0;
  }
  int_value = g_key_file_parse_value_as_integer(key_file, value, &key_file_error);
  g_free(value);
  if (key_file_error) {
      if (g_error_matches(key_file_error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE)) {
          g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE, _("Key file contains key '%s' in group '%s' which has value that cannot "
                      "be interpreted."), key, group_name);
          g_error_free(key_file_error);
      } else g_propagate_error(error, key_file_error);
  }
  return int_value;
}
void g_key_file_set_integer(GKeyFile *key_file, const gchar *group_name, const gchar *key, gint value) {
  gchar *result;
  g_return_if_fail(key_file != NULL);
  result = g_key_file_parse_integer_as_value(key_file, value);
  g_key_file_set_value(key_file, group_name, key, result);
  g_free(result);
}
gint64 g_key_file_get_int64(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) {
  gchar *s, *end;
  gint64 v;
  g_return_val_if_fail(key_file != NULL, -1);
  g_return_val_if_fail(group_name != NULL, -1);
  g_return_val_if_fail(key != NULL, -1);
  s = g_key_file_get_value(key_file, group_name, key, error);
  if (s == NULL) return 0;
  v = g_ascii_strtoll(s, &end, 10);
  if (*s == '\0' || *end != '\0') {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE, "Key '%s' in group '%s' has value '%s' where int64 was expected", key, group_name, s);
      return 0;
  }
  g_free(s);
  return v;
}
void g_key_file_set_int64(GKeyFile *key_file, const gchar *group_name, const gchar *key, gint64 value) {
  gchar *result;
  g_return_if_fail(key_file != NULL);
  result = g_strdup_printf("%" G_GINT64_FORMAT, value);
  g_key_file_set_value(key_file, group_name, key, result);
  g_free(result);
}
unsigned long long g_key_file_get_uint64(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) {
  gchar *s, *end;
  unsigned long long v;
  g_return_val_if_fail(key_file != NULL, -1);
  g_return_val_if_fail(group_name != NULL, -1);
  g_return_val_if_fail(key != NULL, -1);
  s = g_key_file_get_value(key_file, group_name, key, error);
  if (s == NULL) return 0;
  v = g_ascii_strtoull(s, &end, 10);
  if (*s == '\0' || *end != '\0') {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE, "Key '%s' in group '%s' has value '%s' where uint64 was expected", key, group_name, s);
      return 0;
  }
  g_free(s);
  return v;
}
void g_key_file_set_uint64(GKeyFile *key_file, const gchar *group_name, const gchar *key, unsigned long long value) {
  gchar *result;
  g_return_if_fail(key_file != NULL);
  result = g_strdup_printf("%" G_GUINT64_FORMAT, value);
  g_key_file_set_value(key_file, group_name, key, result);
  g_free(result);
}
gint* g_key_file_get_integer_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, gsize *length, GError **error) {
  GError *key_file_error = NULL;
  gchar **values;
  gint *int_values;
  gsize i, num_ints;
  g_return_val_if_fail(key_file != NULL, NULL);
  g_return_val_if_fail(group_name != NULL, NULL);
  g_return_val_if_fail(key != NULL, NULL);
  if (length) *length = 0;
  values = g_key_file_get_string_list(key_file, group_name, key, &num_ints, &key_file_error);
  if (key_file_error) g_propagate_error(error, key_file_error);
  if (!values) return NULL;
  int_values = g_new(gint, num_ints);
  for (i = 0; i < num_ints; i++) {
      int_values[i] = g_key_file_parse_value_as_integer(key_file, values[i], &key_file_error);
      if (key_file_error) {
          g_propagate_error(error, key_file_error);
          g_strfreev(values);
          g_free(int_values);
          return NULL;
      }
  }
  g_strfreev(values);
  if (length) *length = num_ints;
  return int_values;
}
void g_key_file_set_integer_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, gint list[], gsize length) {
  GString *values;
  gsize i;
  g_return_if_fail(key_file != NULL);
  g_return_if_fail(list != NULL);
  values = g_string_sized_new(length * 16);
  for (i = 0; i < length; i++) {
      gchar *value;
      value = g_key_file_parse_integer_as_value(key_file, list[i]);
      g_string_append(values, value);
      g_string_append_c(values, key_file->list_separator);
      g_free (value);
  }
  g_key_file_set_value (key_file, group_name, key, values->str);
  g_string_free (values, TRUE);
}
gdouble g_key_file_get_double(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) {
  GError *key_file_error;
  gchar *value;
  gdouble double_value;
  g_return_val_if_fail(key_file != NULL, -1);
  g_return_val_if_fail(group_name != NULL, -1);
  g_return_val_if_fail(key != NULL, -1);
  key_file_error = NULL;
  value = g_key_file_get_value(key_file, group_name, key, &key_file_error);
  if (key_file_error) {
      g_propagate_error(error, key_file_error);
      return 0;
  }
  double_value = g_key_file_parse_value_as_double(key_file, value, &key_file_error);
  g_free(value);
  if (key_file_error) {
      if (g_error_matches(key_file_error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE)) {
          g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE, _("Key file contains key '%s' in group '%s' "
                      "which has value that cannot be interpreted."), key, group_name);
          g_error_free(key_file_error);
      } else g_propagate_error(error, key_file_error);
  }
  return double_value;
}
void g_key_file_set_double(GKeyFile *key_file, const gchar *group_name, const gchar *key, gdouble value) {
  gchar result[G_ASCII_DTOSTR_BUF_SIZE];
  g_return_if_fail(key_file != NULL);
  g_ascii_dtostr(result, sizeof (result), value);
  g_key_file_set_value(key_file, group_name, key, result);
}
gdouble* g_key_file_get_double_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, gsize *length, GError **error) {
  GError *key_file_error = NULL;
  gchar **values;
  gdouble *double_values;
  gsize i, num_doubles;
  g_return_val_if_fail(key_file != NULL, NULL);
  g_return_val_if_fail(group_name != NULL, NULL);
  g_return_val_if_fail(key != NULL, NULL);
  if (length) *length = 0;
  values = g_key_file_get_string_list(key_file, group_name, key, &num_doubles, &key_file_error);
  if (key_file_error) g_propagate_error(error, key_file_error);
  if (!values) return NULL;
  double_values = g_new(gdouble, num_doubles);
  for (i = 0; i < num_doubles; i++) {
      double_values[i] = g_key_file_parse_value_as_double(key_file, values[i], &key_file_error);
      if (key_file_error) {
          g_propagate_error(error, key_file_error);
          g_strfreev(values);
          g_free(double_values);
          return NULL;
      }
  }
  g_strfreev(values);
  if (length) *length = num_doubles;
  return double_values;
}
void g_key_file_set_double_list (GKeyFile *key_file, const gchar *group_name, const gchar *key, gdouble list[], gsize length) {
  GString *values;
  gsize i;
  g_return_if_fail(key_file != NULL);
  g_return_if_fail(list != NULL);
  values = g_string_sized_new(length * 16);
  for (i = 0; i < length; i++) {
      gchar result[G_ASCII_DTOSTR_BUF_SIZE];
      g_ascii_dtostr(result, sizeof(result), list[i] );
      g_string_append(values, result);
      g_string_append_c(values, key_file->list_separator);
  }
  g_key_file_set_value(key_file, group_name, key, values->str);
  g_string_free(values, TRUE);
}
gboolean g_key_file_set_key_comment(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *comment, GError **error) {
  GKeyFileGroup *group;
  GKeyFileKeyValuePair *pair;
  GList *key_node, *comment_node, *tmp;
  group = g_key_file_lookup_group (key_file, group_name);
  if (!group) {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND, _("Key file does not have group '%s'"), group_name ? group_name : "(null)");
      return FALSE;
  }
  key_node = g_key_file_lookup_key_value_pair_node (key_file, group, key);
  if (key_node == NULL) {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND, _("Key file does not have key '%s' in group '%s'"), key, group->name);
      return FALSE;
  }
  tmp = key_node->next;
  while(tmp != NULL) {
      pair = (GKeyFileKeyValuePair*)tmp->data;
      if (pair->key != NULL) break;
      comment_node = tmp;
      tmp = tmp->next;
      g_key_file_remove_key_value_pair_node(key_file, group, comment_node);
  }
  if (comment == NULL) return TRUE;
  pair = g_slice_new (GKeyFileKeyValuePair);
  pair->key = NULL;
  pair->value = g_key_file_parse_comment_as_value(key_file, comment);
  key_node = g_list_insert(key_node, pair, 1);
  return TRUE;
}
gboolean g_key_file_set_group_comment(GKeyFile *key_file, const gchar *group_name, const gchar *comment, GError **error) {
  GKeyFileGroup *group;
  g_return_val_if_fail (g_key_file_is_group_name (group_name), FALSE);
  group = g_key_file_lookup_group (key_file, group_name);
  if (!group) {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND, _("Key file does not have group '%s'"), group_name ? group_name : "(null)");
      return FALSE;
  }
  if (group->comment) {
      g_key_file_key_value_pair_free(group->comment);
      group->comment = NULL;
  }
  if (comment == NULL) return TRUE;
  group->comment = g_slice_new (GKeyFileKeyValuePair);
  group->comment->key = NULL;
  group->comment->value = g_key_file_parse_comment_as_value(key_file, comment);
  return TRUE;
}
gboolean g_key_file_set_top_comment(GKeyFile *key_file, const gchar *comment, GError **error) {
  GList *group_node;
  GKeyFileGroup *group;
  GKeyFileKeyValuePair *pair;
  g_warn_if_fail (key_file->groups != NULL);
  group_node = g_list_last (key_file->groups);
  group = (GKeyFileGroup *) group_node->data;
  g_warn_if_fail (group->name == NULL);
  if (group->key_value_pairs != NULL) {
      g_list_foreach(group->key_value_pairs, (GFunc)g_key_file_key_value_pair_free, NULL);
      g_list_free(group->key_value_pairs);
      group->key_value_pairs = NULL;
  }
  if (comment == NULL) return TRUE;
  pair = g_slice_new(GKeyFileKeyValuePair);
  pair->key = NULL;
  pair->value = g_key_file_parse_comment_as_value(key_file, comment);
  group->key_value_pairs = g_list_prepend(group->key_value_pairs, pair);
  return TRUE;
}
gboolean g_key_file_set_comment(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *comment, GError **error) {
  g_return_val_if_fail(key_file != NULL, FALSE);
  if (group_name != NULL && key != NULL) {
      if (!g_key_file_set_key_comment(key_file, group_name, key, comment, error)) return FALSE;
  } else if (group_name != NULL) {
      if (!g_key_file_set_group_comment(key_file, group_name, comment, error)) return FALSE;
  } else {
      if (!g_key_file_set_top_comment(key_file, comment, error)) return FALSE;
  }
  if (comment != NULL) key_file->approximate_size += strlen (comment);
  return TRUE;
}
gchar* g_key_file_get_key_comment(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) {
  GKeyFileGroup *group;
  GKeyFileKeyValuePair *pair;
  GList *key_node, *tmp;
  GString *string;
  gchar *comment;
  g_return_val_if_fail(g_key_file_is_group_name (group_name), NULL);
  group = g_key_file_lookup_group(key_file, group_name);
  if (!group) {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND, _("Key file does not have group '%s'"), group_name ? group_name : "(null)");
      return NULL;
  }
  key_node = g_key_file_lookup_key_value_pair_node(key_file, group, key);
  if (key_node == NULL) {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND, _("Key file does not have key '%s' in group '%s'"), key, group->name);
      return NULL;
  }
  string = NULL;
  tmp = key_node->next;
  if (!key_node->next) return NULL;
  pair = (GKeyFileKeyValuePair*)tmp->data;
  if (pair->key != NULL) return NULL;
  while(tmp->next) {
      pair = (GKeyFileKeyValuePair*)tmp->next->data;
      if (pair->key != NULL) break;
      tmp = tmp->next;
  }
  while(tmp != key_node) {
      pair = (GKeyFileKeyValuePair*)tmp->data;
      if (string == NULL) string = g_string_sized_new(512);
      comment = g_key_file_parse_value_as_comment(key_file, pair->value);
      g_string_append(string, comment);
      g_free(comment);
      tmp = tmp->prev;
  }
  if (string != NULL) {
      comment = string->str;
      g_string_free(string, FALSE);
  } else comment = NULL;
  return comment;
}
gchar* get_group_comment(GKeyFile *key_file, GKeyFileGroup *group, GError **error) {
  GString *string;
  GList *tmp;
  gchar *comment;
  string = NULL;
  tmp = group->key_value_pairs;
  while(tmp) {
      GKeyFileKeyValuePair *pair;
      pair = (GKeyFileKeyValuePair*)tmp->data;
      if (pair->key != NULL) {
          tmp = tmp->prev;
          break;
	  }
      if (tmp->next == NULL) break;
      tmp = tmp->next;
  }
  while(tmp != NULL) {
      GKeyFileKeyValuePair *pair;
      pair = (GKeyFileKeyValuePair*)tmp->data;
      if (string == NULL) string = g_string_sized_new(512);
      comment = g_key_file_parse_value_as_comment(key_file, pair->value);
      g_string_append(string, comment);
      g_free(comment);
      tmp = tmp->prev;
  }
  if (string != NULL) return g_string_free (string, FALSE);
  return NULL;
}
gchar* g_key_file_get_group_comment(GKeyFile *key_file, const gchar *group_name, GError **error) {
  GList *group_node;
  GKeyFileGroup *group;
  group = g_key_file_lookup_group (key_file, group_name);
  if (!group) {
      g_set_error (error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND, _("Key file does not have group '%s'"), group_name ? group_name : "(null)");
      return NULL;
  }
  if (group->comment) return g_strdup (group->comment->value);
  group_node = g_key_file_lookup_group_node(key_file, group_name);
  group_node = group_node->next;
  group = (GKeyFileGroup*)group_node->data;
  return get_group_comment(key_file, group, error);
}
gchar* g_key_file_get_top_comment(GKeyFile *key_file, GError **error) {
  GList *group_node;
  GKeyFileGroup *group;
  g_warn_if_fail(key_file->groups != NULL);
  group_node = g_list_last(key_file->groups);
  group = (GKeyFileGroup*)group_node->data;
  g_warn_if_fail(group->name == NULL);
  return get_group_comment(key_file, group, error);
}
gchar* g_key_file_get_comment(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) {
  g_return_val_if_fail(key_file != NULL, NULL);
  if (group_name != NULL && key != NULL) return g_key_file_get_key_comment(key_file, group_name, key, error);
  else if (group_name != NULL) return g_key_file_get_group_comment(key_file, group_name, error);
  else return g_key_file_get_top_comment(key_file, error);
}
gboolean g_key_file_remove_comment(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) {
  g_return_val_if_fail (key_file != NULL, FALSE);
  if (group_name != NULL && key != NULL) return g_key_file_set_key_comment(key_file, group_name, key, NULL, error);
  else if (group_name != NULL) return g_key_file_set_group_comment(key_file, group_name, NULL, error);
  else return g_key_file_set_top_comment(key_file, NULL, error);
}
gboolean g_key_file_has_group(GKeyFile *key_file, const gchar *group_name) {
  g_return_val_if_fail(key_file != NULL, FALSE);
  g_return_val_if_fail(group_name != NULL, FALSE);
  return g_key_file_lookup_group(key_file, group_name) != NULL;
}
gboolean g_key_file_has_key(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) {
  GKeyFileKeyValuePair *pair;
  GKeyFileGroup *group;
  g_return_val_if_fail(key_file != NULL, FALSE);
  g_return_val_if_fail(group_name != NULL, FALSE);
  g_return_val_if_fail(key != NULL, FALSE);
  group = g_key_file_lookup_group(key_file, group_name);
  if (!group) {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND, _("Key file does not have group '%s'"), group_name ? group_name : "(null)");
      return FALSE;
  }
  pair = g_key_file_lookup_key_value_pair(key_file, group, key);
  return pair != NULL;
}
void g_key_file_add_group(GKeyFile *key_file, const gchar *group_name) {
  GKeyFileGroup *group;
  g_return_if_fail(key_file != NULL);
  g_return_if_fail(g_key_file_is_group_name (group_name));
  group = g_key_file_lookup_group (key_file, group_name);
  if (group != NULL) {
      key_file->current_group = group;
      return;
  }
  group = g_slice_new0(GKeyFileGroup);
  group->name = g_strdup(group_name);
  group->lookup_map = g_hash_table_new(g_str_hash, g_str_equal);
  key_file->groups = g_list_prepend(key_file->groups, group);
  key_file->approximate_size += strlen(group_name) + 3;
  key_file->current_group = group;
  if (key_file->start_group == NULL) key_file->start_group = group;
  g_hash_table_insert(key_file->group_hash, (gpointer)group->name, group);
}
void g_key_file_key_value_pair_free(GKeyFileKeyValuePair *pair) {
  if (pair != NULL) {
      g_free (pair->key);
      g_free (pair->value);
      g_slice_free(GKeyFileKeyValuePair, pair);
  }
}
void g_key_file_remove_key_value_pair_node(GKeyFile *key_file, GKeyFileGroup *group, GList *pair_node) {
  GKeyFileKeyValuePair *pair;
  pair = (GKeyFileKeyValuePair *) pair_node->data;
  group->key_value_pairs = g_list_remove_link (group->key_value_pairs, pair_node);
  if (pair->key != NULL) key_file->approximate_size -= strlen(pair->key) + 1;
  g_warn_if_fail(pair->value != NULL);
  key_file->approximate_size -= strlen(pair->value);
  g_key_file_key_value_pair_free(pair);
  g_list_free_1(pair_node);
}
void g_key_file_remove_group_node(GKeyFile *key_file, GList *group_node) {
  GKeyFileGroup *group;
  GList *tmp;
  group = (GKeyFileGroup*)group_node->data;
  if (group->name) g_hash_table_remove(key_file->group_hash, group->name);
  if (key_file->current_group == group) {
      if (key_file->groups) key_file->current_group = (GKeyFileGroup*)key_file->groups->data;
      else key_file->current_group = NULL;
  }
  if (key_file->start_group == group) {
      tmp = g_list_last (key_file->groups);
      while(tmp != NULL) {
          if (tmp != group_node && ((GKeyFileGroup*)tmp->data)->name != NULL) break;
          tmp = tmp->prev;
      }
      if (tmp) key_file->start_group = (GKeyFileGroup*)tmp->data;
      else key_file->start_group = NULL;
  }
  key_file->groups = g_list_remove_link(key_file->groups, group_node);
  if (group->name != NULL) key_file->approximate_size -= strlen(group->name) + 3;
  tmp = group->key_value_pairs;
  while (tmp != NULL) {
      GList *pair_node;
      pair_node = tmp;
      tmp = tmp->next;
      g_key_file_remove_key_value_pair_node(key_file, group, pair_node);
  }
  g_warn_if_fail(group->key_value_pairs == NULL);
  if (group->lookup_map) {
      g_hash_table_destroy(group->lookup_map);
      group->lookup_map = NULL;
  }
  g_free((gchar*)group->name);
  g_slice_free(GKeyFileGroup, group);
  g_list_free_1(group_node);
}
gboolean g_key_file_remove_group(GKeyFile *key_file, const gchar *group_name, GError **error) {
  GList *group_node;
  g_return_val_if_fail(key_file != NULL, FALSE);
  g_return_val_if_fail(group_name != NULL, FALSE);
  group_node = g_key_file_lookup_group_node(key_file, group_name);
  if (!group_node) {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND, _("Key file does not have group '%s'"), group_name);
      return FALSE;
  }
  g_key_file_remove_group_node(key_file, group_node);
  return TRUE;  
}
void g_key_file_add_key(GKeyFile *key_file, GKeyFileGroup *group, const gchar *key, const gchar *value) {
  GKeyFileKeyValuePair *pair;
  pair = g_slice_new(GKeyFileKeyValuePair);
  pair->key = g_strdup(key);
  pair->value = g_strdup(value);
  g_hash_table_replace(group->lookup_map, pair->key, pair);
  group->key_value_pairs = g_list_prepend(group->key_value_pairs, pair);
  group->has_trailing_blank_line = FALSE;
  key_file->approximate_size += strlen(key) + strlen(value) + 2;
}
gboolean g_key_file_remove_key(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) {
  GKeyFileGroup *group;
  GKeyFileKeyValuePair *pair;
  g_return_val_if_fail(key_file != NULL, FALSE);
  g_return_val_if_fail(group_name != NULL, FALSE);
  g_return_val_if_fail(key != NULL, FALSE);
  pair = NULL;
  group = g_key_file_lookup_group(key_file, group_name);
  if (!group) {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND, _("Key file does not have group '%s'"), group_name ? group_name : "(null)");
      return FALSE;
  }
  pair = g_key_file_lookup_key_value_pair(key_file, group, key);
  if (!pair) {
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND, _("Key file does not have key '%s' in group '%s'"), key, group->name);
      return FALSE;
  }
  key_file->approximate_size -= strlen(pair->key) + strlen(pair->value) + 2;
  group->key_value_pairs = g_list_remove(group->key_value_pairs, pair);
  g_hash_table_remove (group->lookup_map, pair->key);  
  g_key_file_key_value_pair_free(pair);
  return TRUE;
}
GList* g_key_file_lookup_group_node(GKeyFile *key_file, const gchar *group_name) {
  GKeyFileGroup *group;
  GList *tmp;
  for (tmp = key_file->groups; tmp != NULL; tmp = tmp->next) {
      group = (GKeyFileGroup*)tmp->data;
      if (group && group->name && strcmp (group->name, group_name) == 0) break;
  }
  return tmp;
}
GKeyFileGroup* g_key_file_lookup_group(GKeyFile *key_file, const gchar *group_name) {
  return (GKeyFileGroup *)g_hash_table_lookup (key_file->group_hash, group_name);
}
GList* g_key_file_lookup_key_value_pair_node(GKeyFile *key_file, GKeyFileGroup *group, const gchar *key) {
  GList *key_node;
  for (key_node = group->key_value_pairs; key_node != NULL; key_node = key_node->next) {
      GKeyFileKeyValuePair *pair;
      pair = (GKeyFileKeyValuePair*)key_node->data;
      if (pair->key && strcmp (pair->key, key) == 0) break;
  }
  return key_node;
}
GKeyFileKeyValuePair* g_key_file_lookup_key_value_pair(GKeyFile *key_file, GKeyFileGroup *group, const gchar *key) {
  return (GKeyFileKeyValuePair *) g_hash_table_lookup (group->lookup_map, key);
}
gboolean g_key_file_line_is_comment(const gchar *line) {
  return (*line == '#' || *line == '\0' || *line == '\n');
}
gboolean g_key_file_is_group_name(const gchar *name) {
  gchar *p, *q;
  if (name == NULL) return FALSE;
  p = q = (gchar *) name;
  while (*q && *q != ']' && *q != '[' && !g_ascii_iscntrl (*q)) q = g_utf8_find_next_char (q, NULL);
  if (*q != '\0' || q == p) return FALSE;
  return TRUE;
}
gboolean g_key_file_is_key_name(const gchar *name) {
  gchar *p, *q;
  if (name == NULL) return FALSE;
  p = q = (gchar *) name;
  while (*q && *q != '=' && *q != '[' && *q != ']') q = g_utf8_find_next_char(q, NULL);
  if (q == p) return FALSE;
  if (*p == ' ' || q[-1] == ' ') return FALSE;
  if (*q == '[') {
      q++;
      while(*q && (g_unichar_isalnum(g_utf8_get_char_validated(q, -1)) || *q == '-' || *q == '_' || *q == '.' || *q == '@'))
        q = g_utf8_find_next_char(q, NULL);
      if (*q != ']') return FALSE;
      q++;
  }
  if (*q != '\0') return FALSE;
  return TRUE;
}
gboolean g_key_file_line_is_group(const gchar *line) {
  gchar *p;
  p = (gchar *) line;
  if (*p != '[') return FALSE;
  p++;
  while (*p && *p != ']') p = g_utf8_find_next_char(p, NULL);
  if (*p != ']') return FALSE;
  p = g_utf8_find_next_char(p, NULL);
  while (*p == ' ' || *p == '\t') p = g_utf8_find_next_char(p, NULL);
  if (*p) return FALSE;
  return TRUE;
}
gboolean g_key_file_line_is_key_value_pair(const gchar *line) {
  gchar *p;
  p = (gchar*)g_utf8_strchr(line, -1, '=');
  if (!p) return FALSE;
  if (*p == line[0]) return FALSE;
  return TRUE;
}
gchar* g_key_file_parse_value_as_string(GKeyFile *key_file, const gchar *value, GSList **pieces, GError **error) {
  gchar *string_value, *p, *q0, *q;
  string_value = g_new(gchar, strlen(value) + 1);
  p = (gchar*)value;
  q0 = q = string_value;
  while(*p) {
      if (*p == '\\') {
          p++;
          switch (*p){
              case 's': *q = ' '; break;
              case 'n': *q = '\n'; break;
              case 't': *q = '\t'; break;
              case 'r': *q = '\r'; break;
              case '\\': *q = '\\'; break;
	          case '\0':
	              g_set_error_literal(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE, _("Key file contains escape character at end of line"));
	              break;
              default:
                  if (pieces && *p == key_file->list_separator) *q = key_file->list_separator;
                  else {
                      *q++ = '\\';
                      *q = *p;
                      if (*error == NULL) {
                          gchar sequence[3];
                          sequence[0] = '\\';
                          sequence[1] = *p;
                          sequence[2] = '\0';
                          g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE, _("Key file contains invalid escape "
                                      "sequence '%s'"), sequence);
                      }
                  }
                  break;
          }
      } else {
          *q = *p;
          if (pieces && (*p == key_file->list_separator)) {
              *pieces = g_slist_prepend(*pieces, g_strndup(q0, q - q0));
              q0 = q + 1;
	      }
	  }
      if (*p == '\0') break;
      q++;
      p++;
  }
  *q = '\0';
  if (pieces) {
    if (q0 < q) *pieces = g_slist_prepend(*pieces, g_strndup(q0, q - q0));
    *pieces = g_slist_reverse(*pieces);
  }
  return string_value;
}
gchar* g_key_file_parse_string_as_value(GKeyFile *key_file, const gchar *string, gboolean escape_separator) {
  gchar *value, *p, *q;
  gsize length;
  gboolean parsing_leading_space;
  length = strlen(string) + 1;
  value = g_new(gchar, 2 * length);
  p = (gchar*)string;
  q = value;
  parsing_leading_space = TRUE;
  while(p < (string + length - 1)) {
      gchar escaped_character[3] = { '\\', 0, 0 };
      switch (*p) {
          case ' ':
              if (parsing_leading_space) {
                  escaped_character[1] = 's';
                  strcpy(q, escaped_character);
                  q += 2;
              } else {
                  *q = *p;
                  q++;
              }
              break;
          case '\t':
              if (parsing_leading_space) {
                  escaped_character[1] = 't';
                  strcpy(q, escaped_character);
                  q += 2;
              } else {
                  *q = *p;
                  q++;
              }
              break;
          case '\n':
              escaped_character[1] = 'n';
              strcpy(q, escaped_character);
              q += 2;
              break;
          case '\r':
              escaped_character[1] = 'r';
              strcpy(q, escaped_character);
              q += 2;
              break;
          case '\\':
              escaped_character[1] = '\\';
              strcpy(q, escaped_character);
              q += 2;
              parsing_leading_space = FALSE;
              break;
          default:
              if (escape_separator && *p == key_file->list_separator) {
                  escaped_character[1] = key_file->list_separator;
                  strcpy(q, escaped_character);
                  q += 2;
                  parsing_leading_space = TRUE;
              } else {
                  *q = *p;
                  q++;
                  parsing_leading_space = FALSE;
              }
              break;
      }
      p++;
  }
  *q = '\0';
  return value;
}
gint g_key_file_parse_value_as_integer(GKeyFile *key_file, const gchar *value, GError **error) {
  gchar *end_of_valid_int;
 glong long_value;
  gint int_value;
  errno = 0;
  long_value = strtol (value, &end_of_valid_int, 10);
  if (*value == '\0' || *end_of_valid_int != '\0') {
      gchar *value_utf8 = _g_utf8_make_valid(value);
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE, _("Value '%s' cannot be interpreted as a number."), value_utf8);
      g_free(value_utf8);
      return 0;
  }
  int_value = long_value;
  if (int_value != long_value || errno == ERANGE) {
      gchar *value_utf8 = _g_utf8_make_valid(value);
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE, _("Integer value '%s' out of range"), value_utf8);
      g_free(value_utf8);
      return 0;
  }
  return int_value;
}
gchar* g_key_file_parse_integer_as_value(GKeyFile *key_file, gint value) {
  return g_strdup_printf ("%d", value);
}
gdouble g_key_file_parse_value_as_double(GKeyFile *key_file, const gchar *value, GError **error) {
  gchar *end_of_valid_d;
  gdouble double_value = 0;
  double_value = g_ascii_strtod(value, &end_of_valid_d);
  if (*end_of_valid_d != '\0' || end_of_valid_d == value) {
      gchar *value_utf8 = _g_utf8_make_valid(value);
      g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE, _("Value '%s' cannot be interpreted as a float number."), value_utf8);
      g_free(value_utf8);
  }
  return double_value;
}
gboolean g_key_file_parse_value_as_boolean(GKeyFile *key_file, const gchar *value, GError **error) {
  gchar *value_utf8;
  if (strcmp (value, "true") == 0 || strcmp (value, "1") == 0) return TRUE;
  else if (strcmp (value, "false") == 0 || strcmp (value, "0") == 0) return FALSE;
  value_utf8 = _g_utf8_make_valid (value);
  g_set_error(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE, _("Value '%s' cannot be interpreted as a boolean."), value_utf8);
  g_free(value_utf8);
  return FALSE;
}
gchar* g_key_file_parse_boolean_as_value(GKeyFile *key_file, gboolean  value) {
  if (value) return g_strdup ("true");
  else return g_strdup ("false");
}
gchar* g_key_file_parse_value_as_comment(GKeyFile *key_file, const gchar *value) {
  GString *string;
  gchar **lines;
  gsize i;
  string = g_string_sized_new(512);
  lines = g_strsplit(value, "\n", 0);
  for (i = 0; lines[i] != NULL; i++) {
        if (lines[i][0] != '#') g_string_append_printf(string, "%s\n", lines[i]);
        else g_string_append_printf(string, "%s\n", lines[i] + 1);
  }
  g_strfreev(lines);
  return g_string_free(string, FALSE);
}
gchar* g_key_file_parse_comment_as_value(GKeyFile *key_file, const gchar *comment) {
  GString *string;
  gchar **lines;
  gsize i;
  string = g_string_sized_new(512);
  lines = g_strsplit(comment, "\n", 0);
  for (i = 0; lines[i] != NULL; i++) g_string_append_printf(string, "#%s%s", lines[i], lines[i + 1] == NULL? "" : "\n");
  g_strfreev(lines);
  return g_string_free(string, FALSE);
}