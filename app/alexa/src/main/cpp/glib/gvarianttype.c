#include <string.h>
#include "gvarianttype.h"
#include "gtestutils.h"
#include "gstrfuncs.h"

static gboolean g_variant_type_check(const GVariantType *type) {
  const gchar *type_string;
  if (type == NULL) return FALSE;
  type_string = (const gchar*)type;
#ifndef G_DISABLE_CHECKS
  return g_variant_type_string_scan(type_string, NULL, NULL);
#else
  return TRUE;
#endif
}
gboolean g_variant_type_string_scan(const gchar *string, const gchar *limit, const gchar **endptr) {
  g_return_val_if_fail(string != NULL, FALSE);
  if (string == limit || *string == '\0') return FALSE;
  switch (*string++) {
      case '(':
          while (string == limit || *string != ')') if (!g_variant_type_string_scan (string, limit, &string)) return FALSE;
          string++;
          break;
      case '{':
          if (string == limit || *string == '\0' || !strchr ("bynqihuxtdsog?", *string++) || !g_variant_type_string_scan (string, limit, &string) ||
              string == limit || *string++ != '}')
              return FALSE;
          break;
      case 'm': case 'a': return g_variant_type_string_scan (string, limit, endptr);
      case 'b': case 'y': case 'n': case 'q': case 'i': case 'u': case 'x': case 't': case 'd': case 's': case 'o': case 'g':case 'v': case 'r': case '*':
      case '?': case 'h':
          break;
      default: return FALSE;
  }
  if (endptr != NULL) *endptr = string;
  return TRUE;
}
gboolean g_variant_type_string_is_valid(const gchar *type_string) {
  const gchar *endptr;
  g_return_val_if_fail(type_string != NULL, FALSE);
  if (!g_variant_type_string_scan(type_string, NULL, &endptr)) return FALSE;
  return *endptr == '\0';
}
void g_variant_type_free(GVariantType *type) {
  g_return_if_fail(type == NULL || g_variant_type_check (type));
  g_free(type);
}
GVariantType *g_variant_type_copy(const GVariantType *type) {
  gsize length;
  gchar *new;
  g_return_val_if_fail(g_variant_type_check(type), NULL);
  length = g_variant_type_get_string_length(type);
  new = g_malloc(length + 1);
  memcpy(new, type, length);
  new[length] = '\0';
  return (GVariantType*)new;
}
GVariantType *g_variant_type_new(const gchar *type_string) {
  g_return_val_if_fail(type_string != NULL, NULL);
  return g_variant_type_copy(G_VARIANT_TYPE(type_string));
}
gsize g_variant_type_get_string_length(const GVariantType *type) {
  const gchar *type_string = (const gchar*)type;
  gint brackets = 0;
  gsize index = 0;
  g_return_val_if_fail(g_variant_type_check(type), 0);
  do {
      while(type_string[index] == 'a' || type_string[index] == 'm') index++;
      if (type_string[index] == '(' || type_string[index] == '{') brackets++;
      else if (type_string[index] == ')' || type_string[index] == '}') brackets--;
      index++;
  } while(brackets);
  return index;
}
const gchar *g_variant_type_peek_string(const GVariantType *type) {
  g_return_val_if_fail(g_variant_type_check (type), NULL);
  return (const gchar*)type;
}
gchar *g_variant_type_dup_string(const GVariantType *type) {
  g_return_val_if_fail(g_variant_type_check (type), NULL);
  return g_strndup(g_variant_type_peek_string(type), g_variant_type_get_string_length(type));
}
gboolean g_variant_type_is_definite(const GVariantType *type) {
  const gchar *type_string;
  gsize type_length;
  gsize i;
  g_return_val_if_fail(g_variant_type_check(type), FALSE);
  type_length = g_variant_type_get_string_length(type);
  type_string = g_variant_type_peek_string(type);
  for (i = 0; i < type_length; i++) if (type_string[i] == '*' || type_string[i] == '?' || type_string[i] == 'r') return FALSE;
  return TRUE;
}
gboolean g_variant_type_is_container(const GVariantType *type) {
  gchar first_char;
  g_return_val_if_fail(g_variant_type_check(type), FALSE);
  first_char = g_variant_type_peek_string(type)[0];
  switch(first_char) {
      case 'a': case 'm': case 'r': case '(': case '{': case 'v': return TRUE;
      default: return FALSE;
  }
}
gboolean g_variant_type_is_basic(const GVariantType *type) {
  gchar first_char;
  g_return_val_if_fail(g_variant_type_check(type), FALSE);
  first_char = g_variant_type_peek_string(type)[0];
  switch(first_char) {
      case 'b': case 'y': case 'n': case 'q': case 'i': case 'h': case 'u': case 't': case 'x': case 'd': case 's': case 'o': case 'g': case '?': return TRUE;
      default: return FALSE;
  }
}
gboolean g_variant_type_is_maybe(const GVariantType *type) {
  g_return_val_if_fail(g_variant_type_check(type), FALSE);
  return g_variant_type_peek_string(type)[0] == 'm';
}
gboolean g_variant_type_is_array(const GVariantType *type) {
  g_return_val_if_fail(g_variant_type_check(type), FALSE);
  return g_variant_type_peek_string(type)[0] == 'a';
}
gboolean g_variant_type_is_tuple(const GVariantType *type) {
  gchar type_char;
  g_return_val_if_fail(g_variant_type_check(type), FALSE);
  type_char = g_variant_type_peek_string(type)[0];
  return type_char == 'r' || type_char == '(';
}
gboolean g_variant_type_is_dict_entry(const GVariantType *type) {
  g_return_val_if_fail(g_variant_type_check(type), FALSE);
  return g_variant_type_peek_string(type)[0] == '{';
}
gboolean g_variant_type_is_variant(const GVariantType *type) {
  g_return_val_if_fail(g_variant_type_check(type), FALSE);
  return g_variant_type_peek_string(type)[0] == 'v';
}
guint g_variant_type_hash(gconstpointer type) {
  const gchar *type_string;
  guint value = 0;
  gsize length;
  gsize i;
  g_return_val_if_fail(g_variant_type_check (type), 0);
  type_string = g_variant_type_peek_string(type);
  length = g_variant_type_get_string_length(type);
  for (i = 0; i < length; i++) value = (value << 5) - value + type_string[i];
  return value;
}
gboolean g_variant_type_equal(gconstpointer type1, gconstpointer type2) {
  const gchar *string1, *string2;
  gsize size1, size2;
  g_return_val_if_fail(g_variant_type_check(type1), FALSE);
  g_return_val_if_fail(g_variant_type_check(type2), FALSE);
  if (type1 == type2) return TRUE;
  size1 = g_variant_type_get_string_length(type1);
  size2 = g_variant_type_get_string_length(type2);
  if (size1 != size2) return FALSE;
  string1 = g_variant_type_peek_string(type1);
  string2 = g_variant_type_peek_string(type2);
  return memcmp(string1, string2, size1) == 0;
}
gboolean g_variant_type_is_subtype_of(const GVariantType *type, const GVariantType *supertype) {
  const gchar *supertype_string;
  const gchar *supertype_end;
  const gchar *type_string;
  g_return_val_if_fail(g_variant_type_check(type), FALSE);
  g_return_val_if_fail(g_variant_type_check(supertype), FALSE);
  supertype_string = g_variant_type_peek_string(supertype);
  type_string = g_variant_type_peek_string(type);
  supertype_end = supertype_string + g_variant_type_get_string_length(supertype);
  while(supertype_string < supertype_end) {
      char supertype_char = *supertype_string++;
      if (supertype_char == *type_string) type_string++;
      else if (*type_string == ')') return FALSE;
      else {
          const GVariantType *target_type = (GVariantType*)type_string;
          switch(supertype_char) {
            case 'r':
              if (!g_variant_type_is_tuple(target_type)) return FALSE;
              break;
            case '*': break;
            case '?':
              if (!g_variant_type_is_basic(target_type)) return FALSE;
              break;
            default: return FALSE;
          }
          type_string += g_variant_type_get_string_length(target_type);
      }
  }
  return TRUE;
}
const GVariantType *g_variant_type_element(const GVariantType *type) {
  const gchar *type_string;
  g_return_val_if_fail(g_variant_type_check(type), NULL);
  type_string = g_variant_type_peek_string(type);
  g_assert (type_string[0] == 'a' || type_string[0] == 'm');
  return (const GVariantType*)&type_string[1];
}
const GVariantType *g_variant_type_first(const GVariantType *type) {
  const gchar *type_string;
  g_return_val_if_fail(g_variant_type_check(type), NULL);
  type_string = g_variant_type_peek_string(type);
  g_assert (type_string[0] == '(' || type_string[0] == '{');
  if (type_string[1] == ')') return NULL;
  return (const GVariantType*)&type_string[1];
}
const GVariantType *g_variant_type_next(const GVariantType *type) {
  const gchar *type_string;
  g_return_val_if_fail(g_variant_type_check(type), NULL);
  type_string = g_variant_type_peek_string(type);
  type_string += g_variant_type_get_string_length(type);
  if (*type_string == ')' || *type_string == '}') return NULL;
  return (const GVariantType*)type_string;
}
gsize g_variant_type_n_items(const GVariantType *type) {
  gsize count = 0;
  g_return_val_if_fail(g_variant_type_check(type), 0);
  for (type = g_variant_type_first(type); type; type = g_variant_type_next (type)) count++;
  return count;
}
const GVariantType *g_variant_type_key(const GVariantType *type) {
  const gchar *type_string;
  g_return_val_if_fail(g_variant_type_check(type), NULL);
  type_string = g_variant_type_peek_string(type);
  g_assert(type_string[0] == '{');
  return (const GVariantType*)&type_string[1];
}
const GVariantType *g_variant_type_value(const GVariantType *type) {
  const gchar *type_string;
  g_return_val_if_fail(g_variant_type_check(type), NULL);
  type_string = g_variant_type_peek_string(type);
  g_assert(type_string[0] == '{');
  return g_variant_type_next(g_variant_type_key(type));
}
static GVariantType *g_variant_type_new_tuple_slow(const GVariantType * const *items, gint length) {
  GString *string;
  gsize i;
  string = g_string_new ("(");
  for (i = 0; i < length; i++) {
      const GVariantType *type;
      gsize size;
      g_return_val_if_fail(g_variant_type_check(items[i]), NULL);
      type = items[i];
      size = g_variant_type_get_string_length(type);
      g_string_append_len(string, (const gchar*)type, size);
  }
  g_string_append_c(string, ')');
  return (GVariantType*)g_string_free(string, FALSE);
}
GVariantType *g_variant_type_new_tuple(const GVariantType * const *items, gint length) {
  char buffer[1024];
  gsize offset;
  gsize i;
  g_return_val_if_fail(length == 0 || items != NULL, NULL);
  if (length < 0) for (length = 0; items[length] != NULL; length++);
  offset = 0;
  buffer[offset++] = '(';
  for (i = 0; i < length; i++) {
      const GVariantType *type;
      gsize size;
      g_return_val_if_fail(g_variant_type_check(items[i]), NULL);
      type = items[i];
      size = g_variant_type_get_string_length(type);
      if (offset + size >= sizeof buffer) return g_variant_type_new_tuple_slow(items, length);
      memcpy(&buffer[offset], type, size);
      offset += size;
  }
  g_assert(offset < sizeof buffer);
  buffer[offset++] = ')';
  return (GVariantType*)g_memdup(buffer, offset);
}
GVariantType *g_variant_type_new_array(const GVariantType *element) {
  gsize size;
  gchar *new;
  g_return_val_if_fail(g_variant_type_check(element), NULL);
  size = g_variant_type_get_string_length(element);
  new = g_malloc(size + 1);
  new[0] = 'a';
  memcpy(new + 1, element, size);
  return (GVariantType*)new;
}
GVariantType *g_variant_type_new_maybe(const GVariantType *element) {
  gsize size;
  gchar *new;
  g_return_val_if_fail(g_variant_type_check(element), NULL);
  size = g_variant_type_get_string_length(element);
  new = g_malloc(size + 1);
  new[0] = 'm';
  memcpy(new + 1, element, size);
  return (GVariantType*)new;
}
GVariantType *g_variant_type_new_dict_entry(const GVariantType *key, const GVariantType *value) {
  gsize keysize, valsize;
  gchar *new;
  g_return_val_if_fail(g_variant_type_check(key), NULL);
  g_return_val_if_fail(g_variant_type_check(value), NULL);
  keysize = g_variant_type_get_string_length(key);
  valsize = g_variant_type_get_string_length(value);
  new = g_malloc(1 + keysize + valsize + 1);
  new[0] = '{';
  memcpy(new + 1, key, keysize);
  memcpy(new + 1 + keysize, value, valsize);
  new[1 + keysize + valsize] = '}';
  return (GVariantType*)new;
}
const GVariantType *g_variant_type_checked_(const gchar *type_string) {
  g_return_val_if_fail(g_variant_type_string_is_valid(type_string), NULL);
  return (const GVariantType*)type_string;
}