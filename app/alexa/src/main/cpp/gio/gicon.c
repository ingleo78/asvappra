#include <stdlib.h>
#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "config.h"
#include "gicon.h"
#include "gthemedicon.h"
#include "gfileicon.h"
#include "gemblemedicon.h"
#include "gfile.h"
#include "gioerror.h"

#define G_ICON_SERIALIZATION_MAGIC0 ". "
typedef GIconIface GIconInterface;
G_DEFINE_INTERFACE(GIcon, g_icon, G_TYPE_OBJECT);
static void g_icon_default_init(GIconInterface *iface) {}
guint g_icon_hash(gconstpointer icon) {
  GIconIface *iface;
  g_return_val_if_fail(G_IS_ICON(icon), 0);
  iface = G_ICON_GET_IFACE(icon);
  return (*iface->hash)((GIcon*)icon);
}
gboolean g_icon_equal(GIcon *icon1, GIcon *icon2) {
  GIconIface *iface;
  if (icon1 == NULL && icon2 == NULL) return TRUE;
  if (icon1 == NULL || icon2 == NULL) return FALSE;
  if (G_TYPE_FROM_INSTANCE(icon1) != G_TYPE_FROM_INSTANCE(icon2)) return FALSE;
  iface = G_ICON_GET_IFACE(icon1);
  return (*iface->equal)(icon1, icon2);
}
static gboolean g_icon_to_string_tokenized(GIcon *icon, GString *s) {
  char *ret;
  GPtrArray *tokens;
  gint version;
  GIconIface *icon_iface;
  int i;
  g_return_val_if_fail(icon != NULL, FALSE);
  g_return_val_if_fail(G_IS_ICON(icon), FALSE);
  ret = NULL;
  icon_iface = G_ICON_GET_IFACE(icon);
  if (icon_iface->to_tokens == NULL) return FALSE;
  tokens = g_ptr_array_new();
  if (!icon_iface->to_tokens(icon, tokens, &version)) {
      g_ptr_array_free(tokens, TRUE);
      return FALSE;
  }
  g_string_append(s, g_type_name_from_instance((GTypeInstance*)icon));
  if (version != 0) g_string_append_printf(s, ".%d", version);
  for (i = 0; i < tokens->len; i++) {
      char *token;
      token = g_ptr_array_index(tokens, i);
      g_string_append_c(s, ' ');
      g_string_append_uri_escaped(s, token, G_URI_RESERVED_CHARS_ALLOWED_IN_PATH, TRUE);
      g_free(token);
  }
  g_ptr_array_free(tokens, TRUE);
  return TRUE;
}
gchar *g_icon_to_string(GIcon *icon) {
  gchar *ret;
  g_return_val_if_fail(icon != NULL, NULL);
  g_return_val_if_fail(G_IS_ICON(icon), NULL);
  ret = NULL;
  if (G_IS_FILE_ICON(icon)) {
      GFile *file;
      file = g_file_icon_get_file(G_FILE_ICON(icon));
      if (g_file_is_native(file)) {
          ret = g_file_get_path(file);
          if (!g_utf8_validate(ret, -1, NULL)) {
              g_free(ret);
              ret = NULL;
          }
	  } else ret = g_file_get_uri(file);
  } else if (G_IS_THEMED_ICON(icon)) {
      const char * const *names;
      names = g_themed_icon_get_names(G_THEMED_ICON(icon));
      if (names != NULL && names[0] != NULL && names[0][0] != '.' && g_utf8_validate (names[0], -1, NULL) && names[1] == NULL) ret = g_strdup(names[0]);
  }
  if (ret == NULL) {
      GString *s;
      s = g_string_new(G_ICON_SERIALIZATION_MAGIC0);
      if (g_icon_to_string_tokenized(icon, s)) ret = g_string_free(s, FALSE);
      else g_string_free(s, TRUE);
  }
  return ret;
}
static GIcon *g_icon_new_from_tokens(char **tokens, GError **error) {
  GIcon *icon;
  char *typename, *version_str;
  GType type;
  gpointer klass;
  GIconIface *icon_iface;
  gint version;
  char *endp;
  int num_tokens;
  int i;
  icon = NULL;
  klass = NULL;
  num_tokens = g_strv_length(tokens);
  if (num_tokens < 1) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Wrong number of tokens (%d)", num_tokens);
      goto out;
  }
  typename = tokens[0];
  version_str = strchr(typename, '.');
  if (version_str) {
      *version_str = 0;
      version_str += 1;
  }
  type = g_type_from_name(tokens[0]);
  if (type == 0) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"No type for class name %s", tokens[0]);
      goto out;
  }
  if (!g_type_is_a(type, G_TYPE_ICON)) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Type %s does not implement the GIcon interface", tokens[0]);
      goto out;
  }
  klass = g_type_class_ref(type);
  if (klass == NULL) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Type %s is not classed", tokens[0]);
      goto out;
  }
  version = 0;
  if (version_str) {
      version = strtol(version_str, &endp, 10);
      if (endp == NULL || *endp != '\0') {
          g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Malformed version number: %s", version_str);
          goto out;
      }
  }
  icon_iface = g_type_interface_peek(klass, G_TYPE_ICON);
  g_assert(icon_iface != NULL);
  if (icon_iface->from_tokens == NULL) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Type %s does not implement from_tokens() on the GIcon interface", tokens[0]);
      goto out;
  }
  for (i = 1;  i < num_tokens; i++) {
      char *escaped;
      escaped = tokens[i];
      tokens[i] = g_uri_unescape_string(escaped, NULL);
      g_free(escaped);
  }
  icon = icon_iface->from_tokens(tokens + 1, num_tokens - 1, version, error);
out:
  if (klass != NULL) g_type_class_unref(klass);
  return icon;
}
static void ensure_builtin_icon_types(void) {
  static volatile GType t;
  t = g_themed_icon_get_type();
  t = g_file_icon_get_type();
  t = g_emblemed_icon_get_type();
  t = g_emblem_get_type();
}
GIcon *g_icon_new_for_string(const gchar *str, GError **error) {
  GIcon *icon;
  g_return_val_if_fail(str != NULL, NULL);
  ensure_builtin_icon_types();
  icon = NULL;
  if (*str == '.') {
      if (g_str_has_prefix(str, G_ICON_SERIALIZATION_MAGIC0)) {
          gchar **tokens;
          tokens = g_strsplit(str + sizeof(G_ICON_SERIALIZATION_MAGIC0) - 1, " ", 0);
          icon = g_icon_new_from_tokens(tokens, error);
          g_strfreev(tokens);
	  } else g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Can't handle the supplied version the icon encoding");
  } else {
      gchar *scheme;
      scheme = g_uri_parse_scheme(str);
      if (scheme != NULL || str[0] == '/') {
          GFile *location;
          location = g_file_new_for_commandline_arg(str);
          icon = g_file_icon_new(location);
          g_object_unref(location);
      } else icon = g_themed_icon_new(str);
      g_free(scheme);
  }
  return icon;
}