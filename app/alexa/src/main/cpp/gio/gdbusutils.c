#include <stdlib.h>
#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/gstring.h"
#include "../glib/glib.h"
#include "config.h"
#include "gdbusutils.h"

static gboolean is_valid_bus_name_character(gint c, gboolean allow_hyphen) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_') || (allow_hyphen && c == '-');
}
static gboolean is_valid_initial_bus_name_character(gint c, gboolean allow_initial_digit, gboolean allow_hyphen) {
  if (allow_initial_digit) return is_valid_bus_name_character (c, allow_hyphen);
  else return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_') || (allow_hyphen && c == '-');
}
static gboolean is_valid_name(const gchar *start, guint len, gboolean allow_initial_digit, gboolean allow_hyphen) {
  gboolean ret;
  const gchar *s;
  const gchar *end;
  gboolean has_dot;
  ret = FALSE;
  if (len == 0) goto out;
  s = start;
  end = s + len;
  has_dot = FALSE;
  while(s != end) {
      if (*s == '.') {
          s += 1;
          if (G_UNLIKELY(!is_valid_initial_bus_name_character(*s, allow_initial_digit, allow_hyphen))) goto out;
          has_dot = TRUE;
      } else if (G_UNLIKELY(!is_valid_bus_name_character(*s, allow_hyphen))) goto out;
      s += 1;
  }
  if (G_UNLIKELY(!has_dot)) goto out;
  ret = TRUE;
out:
  return ret;
}
gboolean g_dbus_is_name(const gchar *string) {
  guint len;
  gboolean ret;
  const gchar *s;
  const gchar *end;
  g_return_val_if_fail(string != NULL, FALSE);
  ret = FALSE;
  len = strlen(string);
  if (G_UNLIKELY(len == 0 || len > 255)) goto out;
  s = string;
  end = s + len;
  if (*s == ':') {
      if (!is_valid_name(s + 1, len - 1, TRUE, TRUE)) goto out;
      ret = TRUE;
      goto out;
  } else if (G_UNLIKELY(*s == '.')) goto out;
  else if (G_UNLIKELY(!is_valid_initial_bus_name_character(*s, FALSE, TRUE))) goto out;
  ret = is_valid_name(s + 1, len - 1, FALSE, TRUE);
 out:
  return ret;
}
gboolean g_dbus_is_unique_name(const gchar *string) {
  gboolean ret;
  guint len;
  g_return_val_if_fail(string != NULL, FALSE);
  ret = FALSE;
  len = strlen(string);
  if (G_UNLIKELY(len == 0 || len > 255)) goto out;
  if (G_UNLIKELY(*string != ':')) goto out;
  if (G_UNLIKELY(!is_valid_name (string + 1, len - 1, TRUE, TRUE))) goto out;
  ret = TRUE;
out:
  return ret;
}
gboolean g_dbus_is_member_name(const gchar *string) {
  gboolean ret;
  guint n;
  ret = FALSE;
  if (G_UNLIKELY(string == NULL)) goto out;
  if (G_UNLIKELY(!is_valid_initial_bus_name_character(string[0], FALSE, FALSE))) goto out;
  for (n = 1; string[n] != '\0'; n++) {
      if (G_UNLIKELY(!is_valid_bus_name_character(string[n], FALSE))) goto out;
  }
  ret = TRUE;
out:
  return ret;
}
gboolean g_dbus_is_interface_name(const gchar *string) {
  guint len;
  gboolean ret;
  const gchar *s;
  const gchar *end;
  g_return_val_if_fail(string != NULL, FALSE);
  ret = FALSE;
  len = strlen(string);
  if (G_UNLIKELY(len == 0 || len > 255)) goto out;
  s = string;
  end = s + len;
  if (G_UNLIKELY(*s == '.')) goto out;
  else if (G_UNLIKELY(!is_valid_initial_bus_name_character(*s, FALSE, FALSE))) goto out;
  ret = is_valid_name(s + 1, len - 1, FALSE, FALSE);
out:
  return ret;
}
gchar *g_dbus_generate_guid(void) {
  GString *s;
  GTimeVal now;
  guint32 r1;
  guint32 r2;
  guint32 r3;
  s = g_string_new(NULL);
  r1 = g_random_int();
  r2 = g_random_int();
  r3 = g_random_int();
  g_get_current_time(&now);
  g_string_append_printf(s, "%08x", r1);
  g_string_append_printf(s, "%08x", r2);
  g_string_append_printf(s, "%08x", r3);
  g_string_append_printf(s, "%08x", (guint32)now.tv_sec);
  return g_string_free(s, FALSE);
}
gboolean g_dbus_is_guid(const gchar *string) {
  gboolean ret;
  guint n;
  g_return_val_if_fail(string != NULL, FALSE);
  ret = FALSE;
  for (n = 0; n < 32; n++) {
      if (!g_ascii_isxdigit(string[n])) goto out;
  }
  if (string[32] != '\0') goto out;
  ret = TRUE;
out:
  return ret;
}