#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_KEY_FILE_H__
#define __G_KEY_FILE_H__

#include "gmacros.h"
#include "gtypes.h"
#include "gquark.h"
#include "gerror.h"
#include "glist.h"

G_BEGIN_DECLS
typedef enum {
  G_KEY_FILE_ERROR_UNKNOWN_ENCODING,
  G_KEY_FILE_ERROR_PARSE,
  G_KEY_FILE_ERROR_NOT_FOUND,
  G_KEY_FILE_ERROR_KEY_NOT_FOUND,
  G_KEY_FILE_ERROR_GROUP_NOT_FOUND,
  G_KEY_FILE_ERROR_INVALID_VALUE
} GKeyFileError;
#define G_KEY_FILE_ERROR g_key_file_error_quark()
GQuark g_key_file_error_quark(void);
typedef struct _GKeyFile GKeyFile;
typedef struct _GKeyFileGroup GKeyFileGroup;
typedef struct _GKeyFileKeyValuePair GKeyFileKeyValuePair;
typedef enum {
  G_KEY_FILE_NONE = 0,
  G_KEY_FILE_KEEP_COMMENTS = 1 << 0,
  G_KEY_FILE_KEEP_TRANSLATIONS = 1 << 1
} GKeyFileFlags;
GKeyFile *g_key_file_new(void);
void g_key_file_free(GKeyFile *key_file);
void g_key_file_set_list_separator(GKeyFile *key_file, gchar separator);
int g_key_file_load_from_file(GKeyFile *key_file, const gchar *file, GKeyFileFlags flags, GError **error);
int g_key_file_load_from_data(GKeyFile *key_file, const gchar *data, gsize length, GKeyFileFlags flags, GError **error);
int g_key_file_load_from_dirs(GKeyFile *key_file, const gchar *file, const gchar **search_dirs, gchar **full_path, GKeyFileFlags flags, GError **error);
int g_key_file_load_from_data_dirs(GKeyFile *key_file, const gchar *file, gchar **full_path, GKeyFileFlags flags, GError **error);
gchar *g_key_file_to_data(GKeyFile *key_file, gsize *length, GError **error) G_GNUC_MALLOC;
gchar *g_key_file_get_start_group(GKeyFile *key_file) G_GNUC_MALLOC;
gchar **g_key_file_get_groups(GKeyFile *key_file, gsize *length) G_GNUC_MALLOC;
gchar **g_key_file_get_keys(GKeyFile *key_file, const gchar *group_name, gsize *length, GError **error) G_GNUC_MALLOC;
int g_key_file_has_group(GKeyFile *key_file, const gchar *group_name);
int g_key_file_has_key(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error);
gchar *g_key_file_get_value(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) G_GNUC_MALLOC;
void g_key_file_set_value(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *value);
gchar *g_key_file_get_string(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) G_GNUC_MALLOC;
void g_key_file_set_string(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *string);
gchar *g_key_file_get_locale_string(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *locale, GError **error) G_GNUC_MALLOC;
void g_key_file_set_locale_string(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *locale, const gchar *string);
int g_key_file_get_boolean(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error);
void g_key_file_set_boolean(GKeyFile *key_file, const gchar *group_name, const gchar *key, int value);
gint g_key_file_get_integer(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error);
void g_key_file_set_integer(GKeyFile *key_file, const gchar *group_name, const gchar *key, gint value);
gint64 g_key_file_get_int64(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error);
void g_key_file_set_int64(GKeyFile *key_file, const gchar *group_name, const gchar *key, gint64 value);
guint64 g_key_file_get_uint64(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error);
void g_key_file_set_uint64(GKeyFile *key_file, const gchar *group_name, const gchar *key, guint64 value);
gdouble g_key_file_get_double(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error);
void g_key_file_set_double(GKeyFile *key_file, const gchar *group_name, const gchar *key, gdouble value);
gchar **g_key_file_get_string_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, gsize *length, GError **error) G_GNUC_MALLOC;
void g_key_file_set_string_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar * const list[], gsize length);
gchar **g_key_file_get_locale_string_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *locale, gsize *length, GError **error) G_GNUC_MALLOC;
void g_key_file_set_locale_string_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *locale, const gchar* const list[], gsize length);
int *g_key_file_get_boolean_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, gsize *length, GError **error) G_GNUC_MALLOC;
void g_key_file_set_boolean_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, int list[], gsize length);
gint *g_key_file_get_integer_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, gsize *length, GError **error) G_GNUC_MALLOC;
void g_key_file_set_double_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, gdouble list[], gsize length);
gdouble *g_key_file_get_double_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, gsize *length, GError **error) G_GNUC_MALLOC;
void g_key_file_set_integer_list(GKeyFile *key_file, const gchar *group_name, const gchar *key, gint list[], gsize length);
int g_key_file_set_comment(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *comment, GError **error);
gchar *g_key_file_get_comment(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error) G_GNUC_MALLOC;
int g_key_file_remove_comment(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error);
int g_key_file_remove_key(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error);
int g_key_file_remove_group(GKeyFile *key_file, const gchar *group_name, GError **error);
gint g_key_file_parse_value_as_integer(GKeyFile *key_file, const gchar *value, GError **error);
int g_key_file_set_key_comment(GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *comment, GError **error);
int g_key_file_set_group_comment(GKeyFile *key_file, const gchar *group_name, const gchar *comment, GError **error);
int g_key_file_set_top_comment(GKeyFile *key_file, const gchar *comment, GError **error);
gchar* g_key_file_get_key_comment(GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error);
gchar* get_group_comment(GKeyFile *key_file, GKeyFileGroup *group, GError **error);
gchar* g_key_file_get_group_comment(GKeyFile *key_file, const gchar *group_name, GError **error);
gchar* g_key_file_get_top_comment(GKeyFile *key_file, GError **error);
void g_key_file_add_group(GKeyFile *key_file, const gchar *group_name);
void g_key_file_key_value_pair_free(GKeyFileKeyValuePair *pair);
void g_key_file_remove_key_value_pair_node(GKeyFile *key_file, GKeyFileGroup *group, GList *pair_node);
void g_key_file_remove_group_node(GKeyFile *key_file, GList *group_node);
void g_key_file_add_key(GKeyFile *key_file, GKeyFileGroup *group, const gchar *key, const gchar *value);
GList* g_key_file_lookup_group_node(GKeyFile *key_file, const gchar *group_name);
GKeyFileGroup* g_key_file_lookup_group(GKeyFile *key_file, const gchar *group_name);
GList* g_key_file_lookup_key_value_pair_node(GKeyFile *key_file, GKeyFileGroup *group, const gchar *key);
GKeyFileKeyValuePair* g_key_file_lookup_key_value_pair(GKeyFile *key_file, GKeyFileGroup *group, const gchar *key);
int g_key_file_line_is_comment(const gchar *line);
int g_key_file_is_group_name(const gchar *name);
int g_key_file_is_key_name(const gchar *name);
int g_key_file_line_is_group(const gchar *line);
int g_key_file_line_is_key_value_pair(const gchar *line);
gchar* g_key_file_parse_value_as_string(GKeyFile *key_file, const gchar *value, GSList **pieces, GError **error);
gchar* g_key_file_parse_string_as_value(GKeyFile *key_file, const gchar *string, int escape_separator);
gchar* g_key_file_parse_integer_as_value(GKeyFile *key_file, gint value);
gdouble g_key_file_parse_value_as_double(GKeyFile *key_file, const gchar *value, GError **error);
int g_key_file_parse_value_as_boolean(GKeyFile *key_file, const gchar *value, GError **error);
gchar* g_key_file_parse_boolean_as_value(GKeyFile *key_file, int  value);
gchar* g_key_file_parse_value_as_comment(GKeyFile *key_file, const gchar *value);
gchar* g_key_file_parse_comment_as_value(GKeyFile *key_file, const gchar *comment);
void g_key_file_parse_data(GKeyFile *key_file, const gchar *data, gsize length, GError **error);
void g_key_file_flush_parse_buffer(GKeyFile  *key_file, GError **error);
gchar* key_get_locale(const gchar *key);
void g_key_file_parse_key_value_pair(GKeyFile *key_file, const gchar *line, gsize length, GError **error);
void g_key_file_parse_group(GKeyFile *key_file, const gchar *line, gsize length, GError **error);
void g_key_file_parse_comment(GKeyFile *key_file, const gchar *line, gsize length, GError **error);
void g_key_file_parse_line(GKeyFile *key_file, const gchar *line, gsize length, GError **error);
int g_key_file_locale_is_interesting(GKeyFile *key_file, const gchar *locale);
int g_key_file_load_from_fd(GKeyFile *key_file, gint fd, GKeyFileFlags flags, GError **error);
void g_key_file_clear(GKeyFile *key_file);
void g_key_file_init(GKeyFile *key_file);
GKeyFileGroup *g_key_file_lookup_group(GKeyFile *key_file, const gchar *group_name);
GList *g_key_file_lookup_key_value_pair_node(GKeyFile *key_file, GKeyFileGroup *group, const gchar *key);
gint find_file_in_data_dirs(const gchar *file, const gchar **dirs, gchar **output_file, GError **error);
#define G_KEY_FILE_DESKTOP_GROUP                "Desktop Entry"
#define G_KEY_FILE_DESKTOP_KEY_TYPE             "Type"
#define G_KEY_FILE_DESKTOP_KEY_VERSION          "Version"
#define G_KEY_FILE_DESKTOP_KEY_NAME             "Name"
#define G_KEY_FILE_DESKTOP_KEY_GENERIC_NAME     "GenericName"
#define G_KEY_FILE_DESKTOP_KEY_NO_DISPLAY       "NoDisplay"
#define G_KEY_FILE_DESKTOP_KEY_COMMENT          "Comment"
#define G_KEY_FILE_DESKTOP_KEY_ICON             "Icon"
#define G_KEY_FILE_DESKTOP_KEY_HIDDEN           "Hidden"
#define G_KEY_FILE_DESKTOP_KEY_ONLY_SHOW_IN     "OnlyShowIn"
#define G_KEY_FILE_DESKTOP_KEY_NOT_SHOW_IN      "NotShowIn"
#define G_KEY_FILE_DESKTOP_KEY_TRY_EXEC         "TryExec"
#define G_KEY_FILE_DESKTOP_KEY_EXEC             "Exec"
#define G_KEY_FILE_DESKTOP_KEY_PATH             "Path"
#define G_KEY_FILE_DESKTOP_KEY_TERMINAL         "Terminal"
#define G_KEY_FILE_DESKTOP_KEY_MIME_TYPE        "MimeType"
#define G_KEY_FILE_DESKTOP_KEY_CATEGORIES       "Categories"
#define G_KEY_FILE_DESKTOP_KEY_STARTUP_NOTIFY   "StartupNotify"
#define G_KEY_FILE_DESKTOP_KEY_STARTUP_WM_CLASS "StartupWMClass"
#define G_KEY_FILE_DESKTOP_KEY_URL              "URL"
#define G_KEY_FILE_DESKTOP_TYPE_APPLICATION     "Application"
#define G_KEY_FILE_DESKTOP_TYPE_LINK            "Link"
#define G_KEY_FILE_DESKTOP_TYPE_DIRECTORY       "Directory"
G_END_DECLS

#endif