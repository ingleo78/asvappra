#include <string.h>
#include "../glib/glib.h"

#define STRINFO_MAX_WORDS   17
G_GNUC_UNUSED static guint strinfo_string_to_words(const gchar *string, guint32 *words, gboolean alias) {
  guint n_words;
  gsize size;
  size = strlen(string);
  n_words = MAX(2, (size + 6) >> 2);
  if (n_words > STRINFO_MAX_WORDS) return FALSE;
  words[0] = GUINT32_TO_LE(alias ? 0xfe : 0xff);
  words[n_words - 1] = GUINT32_TO_BE(0xff);
  memcpy(((gchar*)words) + 1, string, size + 1);
  return n_words;
}
G_GNUC_UNUSED static gint strinfo_scan(const guint32 *strinfo, guint length, const guint32 *words, guint n_words) {
  guint i = 0;
  if (length < n_words) return -1;
  while(i <= length - n_words) {
      guint j = 0;
      for (j = 0; j < n_words; j++)
          if (strinfo[i + j] != words[j]) break;
      if (j == n_words) return i;
      i += j ? j : 1;
  }
  return -1;
}
G_GNUC_UNUSED static gint strinfo_find_string(const guint32 *strinfo, guint length, const gchar *string, gboolean alias) {
  guint32 words[STRINFO_MAX_WORDS];
  guint n_words;
  if (length == 0) return -1;
  n_words = strinfo_string_to_words(string, words, alias);
  return strinfo_scan(strinfo + 1, length - 1, words, n_words);
}
G_GNUC_UNUSED static gint strinfo_find_integer(const guint32 *strinfo, guint length, guint32 value) {
  guint i;
  for (i = 0; i < length; i++)
      if (strinfo[i] == GUINT32_TO_LE(value)) {
          const guchar *charinfo = (const guchar*)&strinfo[i];
          if ((i == 0 || charinfo[-1] == 0xff) && charinfo[4] == 0xff) return i;
      }
  return -1;
}
G_GNUC_UNUSED static gboolean strinfo_is_string_valid(const guint32 *strinfo, guint length, const gchar *string) {
  return strinfo_find_string(strinfo, length, string, FALSE) != -1;
}
G_GNUC_UNUSED static gboolean strinfo_enum_from_string(const guint32 *strinfo, guint length, const gchar *string, guint *result) {
  gint index;
  index = strinfo_find_string(strinfo, length, string, FALSE);
  if (index < 0) return FALSE;
  *result = GUINT32_FROM_LE(strinfo[index]);
  return TRUE;
}
G_GNUC_UNUSED static const gchar *strinfo_string_from_enum(const guint32 *strinfo, guint length, guint value) {
  gint index;
  index = strinfo_find_integer (strinfo, length, value);
  if (index < 0) return NULL;
  return 1 + (const gchar*)&strinfo[index + 1];
}
G_GNUC_UNUSED static const gchar *strinfo_string_from_alias(const guint32 *strinfo, guint length, const gchar *alias) {
  gint index;
  index = strinfo_find_string (strinfo, length, alias, TRUE);
  if (index < 0) return NULL;
  return 1 + (const gchar*)&strinfo[GUINT32_TO_LE (strinfo[index]) + 1];
}
G_GNUC_UNUSED static GVariant *strinfo_enumerate(const guint32 *strinfo, guint length) {
  GVariantBuilder builder;
  const gchar *ptr, *end;
  ptr = (gpointer)strinfo;
  end = ptr + 4 * length;
  ptr += 4;
  g_variant_builder_init(&builder, G_VARIANT_TYPE_STRING_ARRAY);
  while(ptr < end) {
      if (*ptr == '\xff') g_variant_builder_add(&builder, "s", ptr + 1);
      ptr = memchr(ptr, '\xff', end - ptr);
      g_assert(ptr != NULL);
      ptr += 5;
  }
  return g_variant_builder_end(&builder);
}
G_GNUC_UNUSED static void strinfo_builder_append_item(GString *builder, const gchar *string, guint value) {
  guint32 words[STRINFO_MAX_WORDS];
  guint n_words;
  value = GUINT32_TO_LE(value);
  n_words = strinfo_string_to_words(string, words, FALSE);
  g_string_append_len(builder, (void*)&value, sizeof value);
  g_string_append_len(builder, (void*)words, 4 * n_words);
}
G_GNUC_UNUSED static gboolean strinfo_builder_append_alias(GString *builder, const gchar *alias, const gchar *target) {
  guint32 words[STRINFO_MAX_WORDS];
  guint n_words;
  guint value;
  gint index;
  index = strinfo_find_string((const guint32*)builder->str,builder->len / 4, target, FALSE);
  if (index == -1) return FALSE;
  value = GUINT32_TO_LE(index);
  n_words = strinfo_string_to_words(alias, words, TRUE);
  g_string_append_len(builder, (void*)&value, sizeof value);
  g_string_append_len(builder, (void*)words, 4 * n_words);
  return TRUE;
}
G_GNUC_UNUSED static gboolean strinfo_builder_contains(GString *builder, const gchar *string) {
  return strinfo_find_string((const guint32*)builder->str,builder->len / 4, string, FALSE) != -1 ||
         strinfo_find_string((const guint32*)builder->str,builder->len / 4, string, TRUE) != -1;
}
G_GNUC_UNUSED static gboolean strinfo_builder_contains_value(GString *builder, guint value) {
  return strinfo_string_from_enum((const guint32*)builder->str,builder->len / 4, value) != NULL;
}