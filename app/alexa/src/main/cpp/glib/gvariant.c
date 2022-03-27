#include <string.h>
#include "gvariant-serialiser.h"
#include "gvariant-internal.h"
#include "gvariant-core.h"
#include "gtestutils.h"
#include "gstrfuncs.h"
#include "gvariant.h"
#include "ghash.h"
#include "gmem.h"

#define guint64 unsigned long long
#define TYPE_CHECK(value, TYPE, val)                                                                       \
    if G_UNLIKELY(!g_variant_is_of_type(value, TYPE)) {                                                    \
        g_return_if_fail_warning(G_LOG_DOMAIN, G_STRFUNC, "g_variant_is_of_type (" #value ", " #TYPE ")"); \
        return val;                                                                                        \
    }
static GVariant* g_variant_new_from_trusted(const GVariantType *type, gconstpointer data, gsize size) {
  GVariant *value;
  GBuffer *buffer;
  buffer = g_buffer_new_from_data(data, size);
  value = g_variant_new_from_buffer(type, buffer, TRUE);
  g_buffer_unref(buffer);
  return value;
}
GVariant* g_variant_new_boolean(gboolean value) {
  guchar v = value;
  return g_variant_new_from_trusted(G_VARIANT_TYPE_BOOLEAN, &v, 1);
}
gboolean g_variant_get_boolean(GVariant *value) {
  const guchar *data;
  TYPE_CHECK(value, G_VARIANT_TYPE_BOOLEAN, FALSE);
  data = g_variant_get_data(value);
  return data != NULL ? *data != 0 : FALSE;
}
#define NUMERIC_TYPE(TYPE, type, ctype)                                               \
  GVariant *g_variant_new_##type(ctype value) {                                       \
      return g_variant_new_from_trusted(G_VARIANT_TYPE_##TYPE, &value, sizeof value); \
  }                                                                                   \
  ctype g_variant_get_##type (GVariant *value) {                                      \
      const ctype *data;                                                              \
      TYPE_CHECK(value, G_VARIANT_TYPE_ ## TYPE, 0);                                  \
      data = g_variant_get_data(value);                                               \
      return data != NULL ? *data : 0;                                                \
  }
NUMERIC_TYPE (BYTE, byte, guchar)
NUMERIC_TYPE (INT16, int16, gint16)
NUMERIC_TYPE (UINT16, uint16, guint16)
NUMERIC_TYPE (INT32, int32, gint32)
NUMERIC_TYPE (UINT32, uint32, guint32)
NUMERIC_TYPE (INT64, int64, gint64)
NUMERIC_TYPE (UINT64, uint64, guint64)
NUMERIC_TYPE (HANDLE, handle, gint32)
NUMERIC_TYPE (DOUBLE, double, gdouble)
GVariant* g_variant_new_maybe(const GVariantType *child_type, GVariant *child) {
  GVariantType *maybe_type;
  GVariant *value;
  g_return_val_if_fail(child_type == NULL || g_variant_type_is_definite(child_type), 0);
  g_return_val_if_fail(child_type != NULL || child != NULL, NULL);
  g_return_val_if_fail(child_type == NULL || child == NULL || g_variant_is_of_type(child, child_type), NULL);
  if (child_type == NULL) child_type = g_variant_get_type(child);
  maybe_type = g_variant_type_new_maybe(child_type);
  if (child != NULL) {
      GVariant **children;
      gboolean trusted;
      children = g_new(GVariant *, 1);
      children[0] = g_variant_ref_sink(child);
      trusted = g_variant_is_trusted(children[0]);
      value = g_variant_new_from_children(maybe_type, children, 1, trusted);
  } else value = g_variant_new_from_children(maybe_type, NULL, 0, TRUE);
  g_variant_type_free(maybe_type);
  return value;
}
GVariant *g_variant_get_maybe(GVariant *value) {
  TYPE_CHECK(value, G_VARIANT_TYPE_MAYBE, NULL);
  if (g_variant_n_children (value)) return g_variant_get_child_value(value, 0);
  return NULL;
}
GVariant * g_variant_new_variant(GVariant *value) {
  g_return_val_if_fail(value != NULL, NULL);
  g_variant_ref_sink(value);
  return g_variant_new_from_children(G_VARIANT_TYPE_VARIANT, g_memdup(&value, sizeof value),1, g_variant_is_trusted(value));
}
GVariant *g_variant_get_variant(GVariant *value) {
  TYPE_CHECK(value, G_VARIANT_TYPE_VARIANT, NULL);
  return g_variant_get_child_value(value, 0);
}
GVariant *g_variant_new_array(const GVariantType *child_type, GVariant * const *children, gsize n_children) {
  GVariantType *array_type;
  GVariant **my_children;
  gboolean trusted;
  GVariant *value;
  gsize i;
  g_return_val_if_fail(n_children > 0 || child_type != NULL, NULL);
  g_return_val_if_fail(n_children == 0 || children != NULL, NULL);
  g_return_val_if_fail(child_type == NULL || g_variant_type_is_definite (child_type), NULL);
  my_children = g_new(GVariant *, n_children);
  trusted = TRUE;
  if (child_type == NULL) child_type = g_variant_get_type (children[0]);
  array_type = g_variant_type_new_array (child_type);
  for (i = 0; i < n_children; i++) {
      TYPE_CHECK(children[i], child_type, NULL);
      my_children[i] = g_variant_ref_sink(children[i]);
      trusted &= g_variant_is_trusted(children[i]);
  }
  value = g_variant_new_from_children(array_type, my_children, n_children, trusted);
  g_variant_type_free (array_type);
  return value;
}
static GVariantType *g_variant_make_tuple_type(GVariant * const *children, gsize n_children) {
  const GVariantType **types;
  GVariantType *type;
  gsize i;
  types = g_new(const GVariantType *, n_children);
  for (i = 0; i < n_children; i++) types[i] = g_variant_get_type(children[i]);
  type = g_variant_type_new_tuple(types, n_children);
  g_free(types);
  return type;
}
GVariant *g_variant_new_tuple(GVariant * const *children, gsize n_children) {
  GVariantType *tuple_type;
  GVariant **my_children;
  gboolean trusted;
  GVariant *value;
  gsize i;
  g_return_val_if_fail(n_children == 0 || children != NULL, NULL);
  my_children = g_new(GVariant *, n_children);
  trusted = TRUE;
  for (i = 0; i < n_children; i++) {
      my_children[i] = g_variant_ref_sink(children[i]);
      trusted &= g_variant_is_trusted(children[i]);
  }
  tuple_type = g_variant_make_tuple_type(children, n_children);
  value = g_variant_new_from_children(tuple_type, my_children, n_children, trusted);
  g_variant_type_free(tuple_type);
  return value;
}
static GVariantType *g_variant_make_dict_entry_type(GVariant *key, GVariant *val) {
  return g_variant_type_new_dict_entry(g_variant_get_type(key), g_variant_get_type(val));
}
GVariant *g_variant_new_dict_entry (GVariant *key, GVariant *value) {
  GVariantType *dict_type;
  GVariant **children;
  gboolean trusted;
  g_return_val_if_fail(key != NULL && value != NULL, NULL);
  g_return_val_if_fail(!g_variant_is_container(key), NULL);
  children = g_new(GVariant *, 2);
  children[0] = g_variant_ref_sink(key);
  children[1] = g_variant_ref_sink(value);
  trusted = g_variant_is_trusted(key) && g_variant_is_trusted(value);
  dict_type = g_variant_make_dict_entry_type(key, value);
  value = g_variant_new_from_children(dict_type, children, 2, trusted);
  g_variant_type_free(dict_type);
  return value;
}
gboolean g_variant_lookup(GVariant *dictionary, const gchar *key, const gchar *format_string, ...) {
  GVariantType *type;
  GVariant *value;
  g_variant_get_data (dictionary);
  type = g_variant_format_string_scan_type (format_string, NULL, NULL);
  value = g_variant_lookup_value (dictionary, key, type);
  g_variant_type_free (type);
  if (value) {
      va_list ap;
      va_start (ap, format_string);
      g_variant_get_va (value, format_string, NULL, &ap);
      g_variant_unref (value);
      va_end (ap);
      return TRUE;
  } else return FALSE;
}
GVariant *g_variant_lookup_value(GVariant *dictionary, const gchar *key, const GVariantType *expected_type) {
  GVariantIter iter;
  GVariant *entry;
  GVariant *value;
  g_return_val_if_fail(g_variant_is_of_type(dictionary, G_VARIANT_TYPE("a{s*}")) || g_variant_is_of_type(dictionary,
                       G_VARIANT_TYPE("a{o*}")), NULL);
  g_variant_iter_init(&iter, dictionary);
  while((entry = g_variant_iter_next_value(&iter))) {
      GVariant *entry_key;
      gboolean matches;
      entry_key = g_variant_get_child_value(entry, 0);
      matches = strcmp(g_variant_get_string(entry_key, NULL), key) == 0;
      g_variant_unref(entry_key);
      if (matches) break;
      g_variant_unref(entry);
  }
  if (entry == NULL) return NULL;
  value = g_variant_get_child_value(entry, 1);
  g_variant_unref(entry);
  if (g_variant_is_of_type(value, G_VARIANT_TYPE_VARIANT)) {
      GVariant *tmp;
      tmp = g_variant_get_variant(value);
      g_variant_unref(value);
      if (expected_type && !g_variant_is_of_type(tmp, expected_type)) {
          g_variant_unref (tmp);
          tmp = NULL;
      }
      value = tmp;
  }
  g_return_val_if_fail(expected_type == NULL || value == NULL || g_variant_is_of_type(value, expected_type), NULL);
  return value;
}
gconstpointer g_variant_get_fixed_array(GVariant *value, gsize *n_elements, gsize element_size) {
  GVariantTypeInfo *array_info;
  gsize array_element_size;
  gconstpointer data;
  gsize size;
  TYPE_CHECK(value, G_VARIANT_TYPE_ARRAY, NULL);
  g_return_val_if_fail(n_elements != NULL, NULL);
  g_return_val_if_fail(element_size > 0, NULL);
  array_info = g_variant_get_type_info (value);
  g_variant_type_info_query_element (array_info, NULL, &array_element_size);
  g_return_val_if_fail(array_element_size, NULL);
  if G_UNLIKELY(array_element_size != element_size) {
      if (array_element_size) {
          g_critical("g_variant_get_fixed_array: assertion `g_variant_array_has_fixed_size (value, element_size)' failed: array size %"G_GSIZE_FORMAT
                     " does not match given element_size %"G_GSIZE_FORMAT".", array_element_size, element_size);
      } else {
          g_critical("g_variant_get_fixed_array: assertion `g_variant_array_has_fixed_size (value, element_size)' failed: array does not have fixed size.");
      }
  }
  data = g_variant_get_data(value);
  size = g_variant_get_size(value);
  if (size % element_size) *n_elements = 0;
  else *n_elements = size / element_size;
  if (*n_elements) return data;
  return NULL;
}
GVariant *g_variant_new_string(const gchar *string) {
  g_return_val_if_fail(string != NULL, NULL);
  g_return_val_if_fail(g_utf8_validate (string, -1, NULL), NULL);
  return g_variant_new_from_trusted(G_VARIANT_TYPE_STRING, string, strlen (string) + 1);
}
GVariant *g_variant_new_object_path(const gchar *object_path) {
  g_return_val_if_fail(g_variant_is_object_path(object_path), NULL);
  return g_variant_new_from_trusted(G_VARIANT_TYPE_OBJECT_PATH, object_path, strlen(object_path) + 1);
}
gboolean g_variant_is_object_path(const gchar *string) {
  g_return_val_if_fail(string != NULL, FALSE);
  return g_variant_serialiser_is_object_path(string, strlen(string) + 1);
}
GVariant *g_variant_new_signature(const gchar *signature) {
  g_return_val_if_fail(g_variant_is_signature(signature), NULL);
  return g_variant_new_from_trusted(G_VARIANT_TYPE_SIGNATURE, signature, strlen(signature) + 1);
}
gboolean g_variant_is_signature(const gchar *string) {
  g_return_val_if_fail(string != NULL, FALSE);
  return g_variant_serialiser_is_signature(string, strlen(string) + 1);
}
const gchar *g_variant_get_string(GVariant *value, gsize *length) {
  gconstpointer data;
  gsize size;
  g_return_val_if_fail(value != NULL, NULL);
  g_return_val_if_fail(g_variant_is_of_type(value, G_VARIANT_TYPE_STRING) || g_variant_is_of_type(value, G_VARIANT_TYPE_OBJECT_PATH) ||
                       g_variant_is_of_type(value, G_VARIANT_TYPE_SIGNATURE), NULL);
  data = g_variant_get_data(value);
  size = g_variant_get_size(value);
  if (!g_variant_is_trusted(value)) {
      switch(g_variant_classify(value)) {
          case G_VARIANT_CLASS_STRING:
              if (g_variant_serialiser_is_string(data, size)) break;
              data = "";
              size = 1;
              break;
          case G_VARIANT_CLASS_OBJECT_PATH:
              if (g_variant_serialiser_is_object_path (data, size)) break;
              data = "/";
              size = 2;
              break;
          case G_VARIANT_CLASS_SIGNATURE:
              if (g_variant_serialiser_is_signature (data, size)) break;
              data = "";
              size = 1;
              break;
          default: g_assert_not_reached();
      }
  }
  if (length) *length = size - 1;
  return data;
}
gchar *g_variant_dup_string(GVariant *value, gsize *length) {
  return g_strdup(g_variant_get_string(value, length));
}
GVariant *g_variant_new_strv(const gchar * const *strv, gssize length) {
  GVariant **strings;
  gsize i;
  g_return_val_if_fail(length == 0 || strv != NULL, NULL);
  if (length < 0) length = g_strv_length((gchar**)strv);
  strings = g_new(GVariant*, length);
  for (i = 0; i < length; i++) strings[i] = g_variant_ref_sink(g_variant_new_string(strv[i]));
  return g_variant_new_from_children(G_VARIANT_TYPE_STRING_ARRAY, strings, length, TRUE);
}
const gchar **g_variant_get_strv(GVariant *value, gsize *length) {
  const gchar **strv;
  gsize n;
  gsize i;
  TYPE_CHECK(value, G_VARIANT_TYPE_STRING_ARRAY, NULL);
  g_variant_get_data(value);
  n = g_variant_n_children(value);
  strv = g_new(const gchar *, n + 1);
  for (i = 0; i < n; i++) {
      GVariant *string;
      string = g_variant_get_child_value(value, i);
      strv[i] = g_variant_get_string(string, NULL);
      g_variant_unref(string);
  }
  strv[i] = NULL;
  if (length) *length = n;
  return strv;
}
gchar **g_variant_dup_strv(GVariant *value, gsize *length) {
  gchar **strv;
  gsize n;
  gsize i;
  TYPE_CHECK(value, G_VARIANT_TYPE_STRING_ARRAY, NULL);
  n = g_variant_n_children(value);
  strv = g_new(gchar *, n + 1);
  for (i = 0; i < n; i++) {
      GVariant *string;
      string = g_variant_get_child_value(value, i);
      strv[i] = g_variant_dup_string(string, NULL);
      g_variant_unref(string);
  }
  strv[i] = NULL;
  if (length) *length = n;
  return strv;
}
GVariant *g_variant_new_bytestring(const gchar *string) {
  g_return_val_if_fail(string != NULL, NULL);
  return g_variant_new_from_trusted(G_VARIANT_TYPE_BYTESTRING, string, strlen(string) + 1);
}
const gchar *g_variant_get_bytestring(GVariant *value) {
  const gchar *string;
  gsize size;
  TYPE_CHECK(value, G_VARIANT_TYPE_BYTESTRING, NULL);
  string = g_variant_get_data(value);
  size = g_variant_get_size(value);
  if (size && string[size - 1] == '\0') return string;
  else return "";
}
gchar *g_varant_dup_bytestring(GVariant *value, gsize *length) {
  const gchar *original = g_variant_get_bytestring(value);
  gsize size;
  if (original == NULL) return NULL;
  size = strlen(original);
  if (length) *length = size;
  return g_memdup(original, size + 1);
}
GVariant *g_variant_new_bytestring_array(const gchar * const *strv, gssize length) {
  GVariant **strings;
  gsize i;
  g_return_val_if_fail (length == 0 || strv != NULL, NULL);
  if (length < 0) length = g_strv_length((gchar **) strv);
  strings = g_new(GVariant *, length);
  for (i = 0; i < length; i++) strings[i] = g_variant_ref_sink(g_variant_new_bytestring (strv[i]));
  return g_variant_new_from_children(G_VARIANT_TYPE_BYTESTRING_ARRAY, strings, length, TRUE);
}
const gchar **g_variant_get_bytestring_array(GVariant *value, gsize *length) {
  const gchar **strv;
  gsize n;
  gsize i;
  TYPE_CHECK(value, G_VARIANT_TYPE_BYTESTRING_ARRAY, NULL);
  g_variant_get_data(value);
  n = g_variant_n_children(value);
  strv = g_new(const gchar*, n + 1);
  for (i = 0; i < n; i++) {
      GVariant *string;
      string = g_variant_get_child_value(value, i);
      strv[i] = g_variant_get_bytestring(string);
      g_variant_unref(string);
  }
  strv[i] = NULL;
  if (length) *length = n;
  return strv;
}
gchar **g_variant_dup_bytestring_array(GVariant *value, gsize *length) {
  gchar **strv;
  gsize n;
  gsize i;
  TYPE_CHECK(value, G_VARIANT_TYPE_BYTESTRING_ARRAY, NULL);
  g_variant_get_data(value);
  n = g_variant_n_children(value);
  strv = g_new(gchar *, n + 1);
  for (i = 0; i < n; i++) {
      GVariant *string;
      string = g_variant_get_child_value(value, i);
      strv[i] = g_variant_dup_bytestring(string, NULL);
      g_variant_unref(string);
  }
  strv[i] = NULL;
  if (length) *length = n;
  return strv;
}
const GVariantType *g_variant_get_type(GVariant *value) {
  GVariantTypeInfo *type_info;
  g_return_val_if_fail(value != NULL, NULL);
  type_info = g_variant_get_type_info(value);
  return (GVariantType*)g_variant_type_info_get_type_string(type_info);
}
const gchar *g_variant_get_type_string(GVariant *value) {
  GVariantTypeInfo *type_info;
  g_return_val_if_fail(value != NULL, NULL);
  type_info = g_variant_get_type_info(value);
  return g_variant_type_info_get_type_string(type_info);
}
gboolean g_variant_is_of_type(GVariant *value, const GVariantType *type) {
  return g_variant_type_is_subtype_of(g_variant_get_type(value), type);
}
gboolean g_variant_is_container(GVariant *value) {
  return g_variant_type_is_container(g_variant_get_type(value));
}
GVariantClass g_variant_classify(GVariant *value) {
  g_return_val_if_fail(value != NULL, 0);
  return *g_variant_get_type_string(value);
}
GString *g_variant_print_string(GVariant *value, GString *string, gboolean type_annotate) {
  if G_UNLIKELY(string == NULL) string = g_string_new(NULL);
  switch(g_variant_classify(value)) {
      case G_VARIANT_CLASS_MAYBE:
          if (type_annotate) g_string_append_printf(string, "@%s ", g_variant_get_type_string(value));
          if (g_variant_n_children(value)) {
              gchar *printed_child;
              GVariant *element;
              element = g_variant_get_child_value(value, 0);
              printed_child = g_variant_print(element, FALSE);
              g_variant_unref(element);
              if (g_str_has_suffix(printed_child, "nothing")) g_string_append(string, "just ");
              g_string_append(string, printed_child);
              g_free (printed_child);
          } else g_string_append(string, "nothing");
          break;
      case G_VARIANT_CLASS_ARRAY:
          if (g_variant_get_type_string (value)[1] == 'y') {
              const gchar *str;
              gsize size;
              gsize i;
              str = g_variant_get_data (value);
              size = g_variant_get_size (value);
              for (i = 0; i < size; i++) if (str[i] == '\0') break;
              if (i == size - 1) {
                  gchar *escaped = g_strescape (str, NULL);
                  if (strchr (str, '\'')) g_string_append_printf (string, "b\"%s\"", escaped);
                  else g_string_append_printf (string, "b'%s'", escaped);
                  g_free (escaped);
                  break;
              }
          }
          if (g_variant_get_type_string(value)[1] == '{') {
              const gchar *comma = "";
              gsize n, i;
              if ((n = g_variant_n_children(value)) == 0) {
                  if (type_annotate) g_string_append_printf(string, "@%s ", g_variant_get_type_string(value));
                  g_string_append(string, "{}");
                  break;
              }
              g_string_append_c(string, '{');
              for (i = 0; i < n; i++) {
                  GVariant *entry, *key, *val;
                  g_string_append(string, comma);
                  comma = ", ";
                  entry = g_variant_get_child_value(value, i);
                  key = g_variant_get_child_value(entry, 0);
                  val = g_variant_get_child_value(entry, 1);
                  g_variant_unref(entry);
                  g_variant_print_string(key, string, type_annotate);
                  g_variant_unref(key);
                  g_string_append(string, ": ");
                  g_variant_print_string(val, string, type_annotate);
                  g_variant_unref(val);
                  type_annotate = FALSE;
              }
              g_string_append_c(string, '}');
          } else {
              const gchar *comma = "";
              gsize n, i;
              if ((n = g_variant_n_children(value)) == 0) {
                  if (type_annotate) g_string_append_printf(string, "@%s ", g_variant_get_type_string(value));
                  g_string_append (string, "[]");
                  break;
              }
              g_string_append_c (string, '[');
              for (i = 0; i < n; i++) {
                  GVariant *element;
                  g_string_append (string, comma);
                  comma = ", ";
                  element = g_variant_get_child_value (value, i);
                  g_variant_print_string (element, string, type_annotate);
                  g_variant_unref (element);
                  type_annotate = FALSE;
              }
              g_string_append_c (string, ']');
          }
          break;
      case G_VARIANT_CLASS_TUPLE: {
              gsize n, i;
              n = g_variant_n_children(value);
              g_string_append_c(string, '(');
              for (i = 0; i < n; i++) {
                  GVariant *element;
                  element = g_variant_get_child_value(value, i);
                  g_variant_print_string(element, string, type_annotate);
                  g_string_append(string, ", ");
                  g_variant_unref(element);
              }
              g_string_truncate(string, string->len - (n > 0) - (n > 1));
              g_string_append_c(string, ')');
          }
          break;
      case G_VARIANT_CLASS_DICT_ENTRY: {
              GVariant *element;
              g_string_append_c(string, '{');
              element = g_variant_get_child_value(value, 0);
              g_variant_print_string(element, string, type_annotate);
              g_variant_unref(element);
              g_string_append(string, ", ");
              element = g_variant_get_child_value(value, 1);
              g_variant_print_string(element, string, type_annotate);
              g_variant_unref(element);
              g_string_append_c(string, '}');
          }
          break;
      case G_VARIANT_CLASS_VARIANT: {
              GVariant *child = g_variant_get_variant(value);
              g_string_append_c(string, '<');
              g_variant_print_string(child, string, TRUE);
              g_string_append_c(string, '>');
              g_variant_unref(child);
          }
          break;
      case G_VARIANT_CLASS_BOOLEAN:
          if (g_variant_get_boolean(value)) g_string_append(string, "true");
          else g_string_append(string, "false");
          break;
      case G_VARIANT_CLASS_STRING: {
              const gchar *str = g_variant_get_string(value, NULL);
              gunichar quote = strchr(str, '\'') ? '"' : '\'';
              g_string_append_c(string, quote);
              while(*str) {
                  gunichar c = g_utf8_get_char(str);
                  if (c == quote || c == '\\') g_string_append_c(string, '\\');
                  if (g_unichar_isprint (c)) g_string_append_unichar(string, c);
                  else {
                      g_string_append_c(string, '\\');
                      if (c < 0x10000) {
                          switch(c) {
                              case '\a': g_string_append_c(string, 'a'); break;
                              case '\b': g_string_append_c(string, 'b'); break;
                              case '\f': g_string_append_c(string, 'f'); break;
                              case '\n': g_string_append_c(string, 'n'); break;
                              case '\r': g_string_append_c(string, 'r'); break;
                              case '\t': g_string_append_c(string, 't'); break;
                              case '\v': g_string_append_c(string, 'v'); break;
                              default: g_string_append_printf(string, "u%04x", c); break;
                          }
                      } else g_string_append_printf(string, "U%08x", c);
                  }
                  str = g_utf8_next_char(str);
              }
              g_string_append_c(string, quote);
          }
          break;
      case G_VARIANT_CLASS_BYTE:
      if (type_annotate)
        g_string_append (string, "byte ");
      g_string_append_printf (string, "0x%02x",
                              g_variant_get_byte (value));
      break;
      case G_VARIANT_CLASS_INT16:
          if (type_annotate) g_string_append(string, "int16 ");
          g_string_append_printf(string, "%"G_GINT16_FORMAT, g_variant_get_int16(value));
          break;
      case G_VARIANT_CLASS_UINT16:
          if (type_annotate) g_string_append(string, "uint16 ");
          g_string_append_printf(string, "%"G_GUINT16_FORMAT, g_variant_get_uint16(value));
          break;
      case G_VARIANT_CLASS_INT32: g_string_append_printf(string, "%"G_GINT32_FORMAT, g_variant_get_int32(value)); break;
      case G_VARIANT_CLASS_HANDLE:
          if (type_annotate) g_string_append(string, "handle ");
          g_string_append_printf(string, "%"G_GINT32_FORMAT, g_variant_get_handle(value));
          break;
      case G_VARIANT_CLASS_UINT32:
          if (type_annotate) g_string_append(string, "uint32 ");
          g_string_append_printf(string, "%"G_GUINT32_FORMAT, g_variant_get_uint32(value));
          break;
      case G_VARIANT_CLASS_INT64:
          if (type_annotate) g_string_append(string, "int64 ");
          g_string_append_printf(string, "%"G_GINT64_FORMAT, g_variant_get_int64(value));
          break;
      case G_VARIANT_CLASS_UINT64:
          if (type_annotate) g_string_append(string, "uint64 ");
          g_string_append_printf(string, "%"G_GUINT64_FORMAT, g_variant_get_uint64(value));
          break;
      case G_VARIANT_CLASS_DOUBLE: {
              gchar buffer[100];
              gint i;
              g_ascii_dtostr(buffer, sizeof buffer, g_variant_get_double(value));
              for (i = 0; buffer[i]; i++)
              if (buffer[i] == '.' || buffer[i] == 'e' || buffer[i] == 'n' || buffer[i] == 'N') break;
              if (buffer[i] == '\0') {
                  buffer[i++] = '.';
                  buffer[i++] = '0';
                  buffer[i++] = '\0';
              }
              g_string_append(string, buffer);
          }
          break;
      case G_VARIANT_CLASS_OBJECT_PATH:
      if (type_annotate) g_string_append(string, "objectpath ");
      g_string_append_printf(string, "\'%s\'", g_variant_get_string(value, NULL));
      break;
      case G_VARIANT_CLASS_SIGNATURE:
      if (type_annotate) g_string_append(string, "signature ");
      g_string_append_printf(string, "\'%s\'", g_variant_get_string(value, NULL));
      break;
      default: g_assert_not_reached();
  }
  return string;
}
gchar *g_variant_print(GVariant *value, gboolean type_annotate) {
  return g_string_free(g_variant_print_string (value, NULL, type_annotate),FALSE);
};
guint g_variant_hash(gconstpointer value_) {
  GVariant *value = (GVariant*)value_;
  switch(g_variant_classify(value)) {
      case G_VARIANT_CLASS_STRING: case G_VARIANT_CLASS_OBJECT_PATH: case G_VARIANT_CLASS_SIGNATURE: return g_str_hash(g_variant_get_string(value, NULL));
      case G_VARIANT_CLASS_BOOLEAN: return g_variant_get_boolean(value);
      case G_VARIANT_CLASS_BYTE: return g_variant_get_byte(value);
      case G_VARIANT_CLASS_INT16: case G_VARIANT_CLASS_UINT16: {
          const guint16 *ptr;
          ptr = g_variant_get_data(value);
          if (ptr) return *ptr;
          else return 0;
      }
      case G_VARIANT_CLASS_INT32: case G_VARIANT_CLASS_UINT32: case G_VARIANT_CLASS_HANDLE: {
          const guint *ptr;
          ptr = g_variant_get_data(value);
          if (ptr) return *ptr;
          else return 0;
      }
      case G_VARIANT_CLASS_INT64: case G_VARIANT_CLASS_UINT64: case G_VARIANT_CLASS_DOUBLE: {
          const guint *ptr;
          ptr = g_variant_get_data(value);
          if (ptr) return ptr[0] + ptr[1];
          else return 0;
      }
      default:
          g_return_val_if_fail(!g_variant_is_container(value), 0);
          g_assert_not_reached();
  }
}
gboolean g_variant_equal(gconstpointer one, gconstpointer two) {
  gboolean equal;
  g_return_val_if_fail(one != NULL && two != NULL, FALSE);
  if (g_variant_get_type_info((GVariant*)one) != g_variant_get_type_info((GVariant*)two)) return FALSE;
  if (g_variant_is_trusted((GVariant*)one) && g_variant_is_trusted((GVariant*)two)) {
      const void *data_one, *data_two;
      gsize size_one, size_two;
      size_one = g_variant_get_size((GVariant*)one);
      size_two = g_variant_get_size((GVariant*)two);
      if (size_one != size_two) return FALSE;
      data_one = g_variant_get_data((GVariant*)one);
      data_two = g_variant_get_data((GVariant*)two);
      equal = memcmp(data_one, data_two, size_one) == 0;
  } else {
      gchar *strone, *strtwo;
      strone = g_variant_print((GVariant*)one, FALSE);
      strtwo = g_variant_print((GVariant*)two, FALSE);
      equal = strcmp(strone, strtwo) == 0;
      g_free(strone);
      g_free(strtwo);
  }
  return equal;
}
gint g_variant_compare(gconstpointer one, gconstpointer two) {
  GVariant *a = (GVariant*)one;
  GVariant *b = (GVariant*)two;
  g_return_val_if_fail(g_variant_classify(a) == g_variant_classify(b),0);
  switch(g_variant_classify(a)) {
      case G_VARIANT_CLASS_BYTE: return ((gint)g_variant_get_byte(a)) - ((gint)g_variant_get_byte(b));
      case G_VARIANT_CLASS_INT16: return ((gint)g_variant_get_int16(a)) - ((gint)g_variant_get_int16(b));
      case G_VARIANT_CLASS_UINT16: return ((gint)g_variant_get_uint16(a)) - ((gint)g_variant_get_uint16(b));
      case G_VARIANT_CLASS_INT32: {
              gint32 a_val = g_variant_get_int32(a);
              gint32 b_val = g_variant_get_int32(b);
              return (a_val == b_val) ? 0 : (a_val > b_val) ? 1 : -1;
          }
      case G_VARIANT_CLASS_UINT32: {
              guint32 a_val = g_variant_get_uint32(a);
              guint32 b_val = g_variant_get_uint32(b);
              return (a_val == b_val) ? 0 : (a_val > b_val) ? 1 : -1;
          }
      case G_VARIANT_CLASS_INT64: {
              gint64 a_val = g_variant_get_int64(a);
              gint64 b_val = g_variant_get_int64(b);
              return (a_val == b_val) ? 0 : (a_val > b_val) ? 1 : -1;
          }
      case G_VARIANT_CLASS_UINT64: {
              guint64 a_val = g_variant_get_int32(a);
              guint64 b_val = g_variant_get_int32(b);
              return (a_val == b_val) ? 0 : (a_val > b_val) ? 1 : -1;
          }
      case G_VARIANT_CLASS_DOUBLE: {
              gdouble a_val = g_variant_get_double(a);
              gdouble b_val = g_variant_get_double(b);
              return (a_val == b_val) ? 0 : (a_val > b_val) ? 1 : -1;
          }
      case G_VARIANT_CLASS_STRING: case G_VARIANT_CLASS_OBJECT_PATH: case G_VARIANT_CLASS_SIGNATURE:
          return strcmp(g_variant_get_string(a, NULL), g_variant_get_string(b, NULL));
      default:
          g_return_val_if_fail(!g_variant_is_container(a), 0);
          g_assert_not_reached();
  }
}
struct stack_iter {
  GVariant *value;
  gssize n, i;
  const gchar *loop_format;
  gsize padding[3];
  gsize magic;
};
G_STATIC_ASSERT(sizeof(struct stack_iter) <= sizeof(GVariantIter));
struct heap_iter {
  struct stack_iter iter;
  GVariant *value_ref;
  gsize magic;
};
#define GVSI(i)  ((struct stack_iter*)(i))
#define GVHI(i)  ((struct heap_iter*)(i))
#define GVSI_MAGIC  ((gsize) 3579507750u)
#define GVHI_MAGIC  ((gsize) 1450270775u)
#define is_valid_iter(i)  (i != NULL &&  GVSI(i)->magic == GVSI_MAGIC)
#define is_valid_heap_iter(i)  (GVHI(i)->magic == GVHI_MAGIC && is_valid_iter(i))
GVariantIter *g_variant_iter_new(GVariant *value) {
  GVariantIter *iter;
  iter = (GVariantIter*)g_slice_new(struct heap_iter);
  GVHI(iter)->value_ref = g_variant_ref(value);
  GVHI(iter)->magic = GVHI_MAGIC;
  g_variant_iter_init(iter, value);
  return iter;
}
gsize g_variant_iter_init(GVariantIter *iter, GVariant *value) {
  GVSI(iter)->magic = GVSI_MAGIC;
  GVSI(iter)->value = value;
  GVSI(iter)->n = g_variant_n_children(value);
  GVSI(iter)->i = -1;
  GVSI(iter)->loop_format = NULL;
  return GVSI(iter)->n;
}
GVariantIter *g_variant_iter_copy(GVariantIter *iter) {
  GVariantIter *copy;
  g_return_val_if_fail(is_valid_iter(iter), 0);
  copy = g_variant_iter_new(GVSI(iter)->value);
  GVSI(copy)->i = GVSI(iter)->i;
  return copy;
}
gsize g_variant_iter_n_children(GVariantIter *iter) {
  g_return_val_if_fail(is_valid_iter(iter), 0);
  return GVSI(iter)->n;
}
void g_variant_iter_free(GVariantIter *iter) {
  g_return_if_fail(is_valid_heap_iter(iter));
  g_variant_unref (GVHI(iter)->value_ref);
  GVHI(iter)->magic = 0;
  g_slice_free(struct heap_iter, GVHI(iter));
}
GVariant *g_variant_iter_next_value (GVariantIter *iter) {
  g_return_val_if_fail (is_valid_iter (iter), FALSE);
  if G_UNLIKELY (GVSI(iter)->i >= GVSI(iter)->n) {
      g_critical ("g_variant_iter_next_value: must not be called again after NULL has already been returned.");
      return NULL;
  }
  GVSI(iter)->i++;
  if (GVSI(iter)->i < GVSI(iter)->n) return g_variant_get_child_value(GVSI(iter)->value, GVSI(iter)->i);
  return NULL;
}
struct stack_builder {
  GVariantBuilder *parent;
  GVariantType *type;
  const GVariantType *expected_type;
  const GVariantType *prev_item_type;
  gsize min_items;
  gsize max_items;
  GVariant **children;
  gsize allocated_children;
  gsize offset;
  guint uniform_item_types : 1;
  guint trusted : 1;
  gsize magic;
};
G_STATIC_ASSERT(sizeof(struct stack_builder) <= sizeof(GVariantBuilder));
struct heap_builder {
  GVariantBuilder builder;
  gsize magic;
  gint ref_count;
};
#define GVSB(b)  ((struct stack_builder*)(b))
#define GVHB(b)  ((struct heap_builder*)(b))
#define GVSB_MAGIC  ((gsize)1033660112u)
#define GVHB_MAGIC  ((gsize)3087242682u)
#define is_valid_builder(b)  (b != NULL && GVSB(b)->magic == GVSB_MAGIC)
#define is_valid_heap_builder(b) (GVHB(b)->magic == GVHB_MAGIC)
GVariantBuilder * g_variant_builder_new(const GVariantType *type) {
  GVariantBuilder *builder;
  builder = (GVariantBuilder*)g_slice_new(struct heap_builder);
  g_variant_builder_init(builder, type);
  GVHB(builder)->magic = GVHB_MAGIC;
  GVHB(builder)->ref_count = 1;
  return builder;
}
void g_variant_builder_unref(GVariantBuilder *builder) {
  g_return_if_fail(is_valid_heap_builder (builder));
  if (--GVHB(builder)->ref_count) return;
  g_variant_builder_clear(builder);
  GVHB(builder)->magic = 0;
  g_slice_free(struct heap_builder, GVHB(builder));
}
GVariantBuilder *g_variant_builder_ref(GVariantBuilder *builder) {
  g_return_val_if_fail (is_valid_heap_builder (builder), NULL);
  GVHB(builder)->ref_count++;
  return builder;
}
void g_variant_builder_clear(GVariantBuilder *builder) {
  gsize i;
  if (GVSB(builder)->magic == 0) return;
  g_return_if_fail(is_valid_builder (builder));
  g_variant_type_free(GVSB(builder)->type);
  for (i = 0; i < GVSB(builder)->offset; i++) g_variant_unref(GVSB(builder)->children[i]);
  g_free (GVSB(builder)->children);
  if (GVSB(builder)->parent) {
      g_variant_builder_clear(GVSB(builder)->parent);
      g_slice_free(GVariantBuilder, GVSB(builder)->parent);
  }
  memset(builder, 0, sizeof(GVariantBuilder));
}
void g_variant_builder_init(GVariantBuilder *builder, const GVariantType *type) {
  g_return_if_fail(type != NULL);
  g_return_if_fail(g_variant_type_is_container(type));
  memset(builder, 0, sizeof(GVariantBuilder));
  GVSB(builder)->type = g_variant_type_copy(type);
  GVSB(builder)->magic = GVSB_MAGIC;
  GVSB(builder)->trusted = TRUE;
  switch(*(const gchar*)type) {
      case G_VARIANT_CLASS_VARIANT:
          GVSB(builder)->uniform_item_types = TRUE;
          GVSB(builder)->allocated_children = 1;
          GVSB(builder)->expected_type = NULL;
          GVSB(builder)->min_items = 1;
          GVSB(builder)->max_items = 1;
          break;
      case G_VARIANT_CLASS_ARRAY:
          GVSB(builder)->uniform_item_types = TRUE;
          GVSB(builder)->allocated_children = 8;
          GVSB(builder)->expected_type = g_variant_type_element(GVSB(builder)->type);
          GVSB(builder)->min_items = 0;
          GVSB(builder)->max_items = -1;
          break;
      case G_VARIANT_CLASS_MAYBE:
          GVSB(builder)->uniform_item_types = TRUE;
          GVSB(builder)->allocated_children = 1;
          GVSB(builder)->expected_type = g_variant_type_element(GVSB(builder)->type);
          GVSB(builder)->min_items = 0;
          GVSB(builder)->max_items = 1;
          break;
      case G_VARIANT_CLASS_DICT_ENTRY:
          GVSB(builder)->uniform_item_types = FALSE;
          GVSB(builder)->allocated_children = 2;
          GVSB(builder)->expected_type = g_variant_type_key(GVSB(builder)->type);
          GVSB(builder)->min_items = 2;
          GVSB(builder)->max_items = 2;
          break;
      case 'r':
          GVSB(builder)->uniform_item_types = FALSE;
          GVSB(builder)->allocated_children = 8;
          GVSB(builder)->expected_type = NULL;
          GVSB(builder)->min_items = 0;
          GVSB(builder)->max_items = -1;
          break;
      case G_VARIANT_CLASS_TUPLE: /* a definite tuple type was given */
          GVSB(builder)->allocated_children = g_variant_type_n_items(type);
          GVSB(builder)->expected_type = g_variant_type_first(GVSB(builder)->type);
          GVSB(builder)->min_items = GVSB(builder)->allocated_children;
          GVSB(builder)->max_items = GVSB(builder)->allocated_children;
          GVSB(builder)->uniform_item_types = FALSE;
          break;
      default: g_assert_not_reached();
   }
  GVSB(builder)->children = g_new(GVariant *,GVSB(builder)->allocated_children);
}
static void g_variant_builder_make_room(struct stack_builder *builder) {
  if (builder->offset == builder->allocated_children) {
      builder->allocated_children *= 2;
      builder->children = g_renew(GVariant *,builder->children,builder->allocated_children);
  }
}
void
g_variant_builder_add_value (GVariantBuilder *builder, GVariant *value) {
  g_return_if_fail(is_valid_builder(builder));
  g_return_if_fail(GVSB(builder)->offset < GVSB(builder)->max_items);
  g_return_if_fail(!GVSB(builder)->expected_type || g_variant_is_of_type(value, GVSB(builder)->expected_type));
  g_return_if_fail(!GVSB(builder)->prev_item_type || g_variant_is_of_type(value, GVSB(builder)->prev_item_type));
  GVSB(builder)->trusted &= g_variant_is_trusted(value);
  if (!GVSB(builder)->uniform_item_types) {
      if (GVSB(builder)->expected_type) GVSB(builder)->expected_type = g_variant_type_next(GVSB(builder)->expected_type);
      if (GVSB(builder)->prev_item_type) GVSB(builder)->prev_item_type = g_variant_type_next(GVSB(builder)->prev_item_type);
  } else GVSB(builder)->prev_item_type = g_variant_get_type(value);
  g_variant_builder_make_room(GVSB(builder));
  GVSB(builder)->children[GVSB(builder)->offset++] = g_variant_ref_sink(value);
}
void g_variant_builder_open(GVariantBuilder *builder, const GVariantType *type) {
  GVariantBuilder *parent;
  g_return_if_fail(is_valid_builder (builder));
  g_return_if_fail(GVSB(builder)->offset < GVSB(builder)->max_items);
  g_return_if_fail(!GVSB(builder)->expected_type || g_variant_type_is_subtype_of(type, GVSB(builder)->expected_type));
  g_return_if_fail(!GVSB(builder)->prev_item_type || g_variant_type_is_subtype_of(GVSB(builder)->prev_item_type, type));
  parent = g_slice_dup(GVariantBuilder, builder);
  g_variant_builder_init(builder, type);
  GVSB(builder)->parent = parent;
  if (GVSB(parent)->prev_item_type) {
      if (!GVSB(builder)->uniform_item_types) GVSB(builder)->prev_item_type = g_variant_type_first(GVSB(parent)->prev_item_type);
      else if (!g_variant_type_is_variant(GVSB(builder)->type)) GVSB(builder)->prev_item_type = g_variant_type_element(GVSB(parent)->prev_item_type);
    }
}
void g_variant_builder_close(GVariantBuilder *builder) {
  GVariantBuilder *parent;
  g_return_if_fail(is_valid_builder (builder));
  g_return_if_fail(GVSB(builder)->parent != NULL);
  parent = GVSB(builder)->parent;
  GVSB(builder)->parent = NULL;
  g_variant_builder_add_value(parent, g_variant_builder_end(builder));
  *builder = *parent;
  g_slice_free(GVariantBuilder, parent);
}
static GVariantType *g_variant_make_maybe_type(GVariant *element) {
  return g_variant_type_new_maybe(g_variant_get_type(element));
}
static GVariantType *g_variant_make_array_type(GVariant *element) {
  return g_variant_type_new_array(g_variant_get_type(element));
}
GVariant *g_variant_builder_end (GVariantBuilder *builder) {
  GVariantType *my_type;
  GVariant *value;
  g_return_val_if_fail(is_valid_builder (builder), NULL);
  g_return_val_if_fail(GVSB(builder)->offset >= GVSB(builder)->min_items, NULL);
  g_return_val_if_fail(!GVSB(builder)->uniform_item_types || GVSB(builder)->prev_item_type != NULL || g_variant_type_is_definite(GVSB(builder)->type), NULL);
  if (g_variant_type_is_definite(GVSB(builder)->type)) my_type = g_variant_type_copy(GVSB(builder)->type);
  else if (g_variant_type_is_maybe(GVSB(builder)->type)) my_type = g_variant_make_maybe_type(GVSB(builder)->children[0]);
  else if (g_variant_type_is_array (GVSB(builder)->type)) my_type = g_variant_make_array_type(GVSB(builder)->children[0]);
  else if (g_variant_type_is_tuple (GVSB(builder)->type)) my_type = g_variant_make_tuple_type(GVSB(builder)->children, GVSB(builder)->offset);
  else if (g_variant_type_is_dict_entry(GVSB(builder)->type)) my_type = g_variant_make_dict_entry_type(GVSB(builder)->children[0], GVSB(builder)->children[1]);
  else g_assert_not_reached();
  value = g_variant_new_from_children(my_type, g_renew(GVariant *,GVSB(builder)->children,GVSB(builder)->offset), GVSB(builder)->offset,
                                      GVSB(builder)->trusted);
  GVSB(builder)->children = NULL;
  GVSB(builder)->offset = 0;
  g_variant_builder_clear(builder);
  g_variant_type_free(my_type);
  return value;
}
gboolean g_variant_format_string_scan(const gchar *string, const gchar *limit, const gchar **endptr) {
#define next_char() (string == limit ? '\0' : *string++)
#define peek_char() (string == limit ? '\0' : *string)
  char c;
  switch (next_char()) {
      case 'b': case 'y': case 'n': case 'q': case 'i': case 'u': case 'x': case 't': case 'h': case 'd': case 's': case 'o':
      case 'g': case 'v': case '*': case '?': case 'r':
          break;
      case 'm': return g_variant_format_string_scan (string, limit, endptr);
      case 'a': case '@': return g_variant_type_string_scan (string, limit, endptr);
      case '(':
          while(peek_char() != ')') if (!g_variant_format_string_scan (string, limit, &string)) return FALSE;
          next_char();
          break;
      case '{':
          c = next_char();
          if (c == '&') {
              c = next_char ();
              if (c != 's' && c != 'o' && c != 'g') return FALSE;
          } else {
              if (c == '@') c = next_char();
              if (c != '\0' && strchr ("bynqiuxthdsog?", c) == NULL) return FALSE;
          }
          if (!g_variant_format_string_scan (string, limit, &string)) return FALSE;
          if (next_char() != '}') return FALSE;
          break;
      case '^':
          if ((c = next_char()) == 'a') {
              if ((c = next_char()) == '&') {
                  if ((c = next_char()) == 'a') {
                      if ((c = next_char()) == 'y') break;
                  } else if (c == 's') break;
              } else if (c == 'a') {
                  if ((c = next_char()) == 'y') break;
              } else if (c == 's') break;
              else if (c == 'y') break;
          } else if (c == '&') {
              if ((c = next_char()) == 'a') {
                  if ((c = next_char()) == 'y') break;
              }
          }
          return FALSE;
      case '&':
          c = next_char();
          if (c != 's' && c != 'o' && c != 'g') return FALSE;
          break;
      default: return FALSE;
  }
  if (endptr != NULL) *endptr = string;
#undef next_char
#undef peek_char
  return TRUE;
}
GVariantType *g_variant_format_string_scan_type(const gchar *string, const gchar *limit, const gchar **endptr) {
  const gchar *my_end;
  gchar *dest;
  gchar *new;
  if (endptr == NULL) endptr = &my_end;
  if (!g_variant_format_string_scan(string, limit, endptr)) return NULL;
  dest = new = g_malloc(*endptr - string + 1);
  while (string != *endptr) {
      if (*string != '@' && *string != '&' && *string != '^') *dest++ = *string;
      string++;
  }
  *dest = '\0';
  return (GVariantType*)G_VARIANT_TYPE(new);
}
static gboolean valid_format_string(const gchar *format_string, gboolean single, GVariant *value) {
  const gchar *endptr;
  GVariantType *type;
  type = g_variant_format_string_scan_type(format_string, NULL, &endptr);
  if G_UNLIKELY(type == NULL || (single && *endptr != '\0')) {
      if (single) g_critical("`%s' is not a valid GVariant format string", format_string);
      else g_critical("`%s' does not have a valid GVariant format string as a prefix", format_string);
      if (type != NULL) g_variant_type_free(type);
      return FALSE;
  }
  if G_UNLIKELY(value && !g_variant_is_of_type(value, type)) {
      gchar *fragment;
      gchar *typestr;
      fragment = g_strndup(format_string, endptr - format_string);
      typestr = g_variant_type_dup_string(type);
      g_critical("the GVariant format string `%s' has a type of `%s' but the given value has a type of `%s'", fragment, typestr, g_variant_get_type_string (value));
      g_variant_type_free(type);
      return FALSE;
  }
  g_variant_type_free(type);
  return TRUE;
}
static gboolean g_variant_format_string_is_leaf(const gchar *str) {
  return str[0] != 'm' && str[0] != '(' && str[0] != '{';
}
static gboolean g_variant_format_string_is_nnp(const gchar *str) {
  return str[0] == 'a' || str[0] == 's' || str[0] == 'o' || str[0] == 'g' || str[0] == '^' || str[0] == '@' || str[0] == '*' || str[0] == '?' ||
         str[0] == 'r' || str[0] == 'v' || str[0] == '&';
}
static void g_variant_valist_free_nnp(const gchar *str, gpointer ptr) {
  switch(*str) {
      case 'a': g_variant_iter_free(ptr); break;
      case '^':
          if (str[2] != '&') g_strfreev(ptr);
          else g_free(ptr);
          break;
      case 's': case 'o': case 'g': g_free(ptr); break;
      case '@': case '*': case '?': case 'v': g_variant_unref(ptr); break;
      case '&': break;
      default: g_assert_not_reached();
    }
}
static gchar g_variant_scan_convenience(const gchar **str, gboolean *constant, guint *arrays) {
  *constant = FALSE;
  *arrays = 0;
  for(;;) {
      char c = *(*str)++;
      if (c == '&') *constant = TRUE;
      else if (c == 'a') (*arrays)++;
      else return c;
  }
}
static GVariant *g_variant_valist_new_nnp(const gchar **str, gpointer ptr) {
  if (**str == '&') (*str)++;
  switch(*(*str)++)  {
      case 'a':
          if (ptr != NULL) {
              const GVariantType *type;
              GVariant *value;
              value = g_variant_builder_end(ptr);
              type = g_variant_get_type(value);
              if G_UNLIKELY(!g_variant_type_is_array(type))
                  g_error("g_variant_new: expected array GVariantBuilder but the built value has type `%s'", g_variant_get_type_string(value));
              type = g_variant_type_element (type);
              if G_UNLIKELY(!g_variant_type_is_subtype_of(type, (GVariantType*)*str))
                  g_error("g_variant_new: expected GVariantBuilder array element type `%s' but the built value has element type `%s'",
                          g_variant_type_dup_string((GVariantType*)*str), g_variant_get_type_string (value) + 1);
              g_variant_type_string_scan(*str, NULL, str);
              return value;
          } else {
              const GVariantType *type = (GVariantType*)*str;
              g_variant_type_string_scan(*str, NULL, str);
              if G_UNLIKELY(!g_variant_type_is_definite(type))
                  g_error("g_variant_new: NULL pointer given with indefinite array type; unable to determine which type of empty array to construct.");
              return g_variant_new_array (type, NULL, 0);
          }
      case 's': return g_variant_new_string(ptr);
      case 'o': return g_variant_new_object_path(ptr);
      case 'g': return g_variant_new_signature(ptr);
      case '^': {
              gboolean constant;
              guint arrays;
              if (g_variant_scan_convenience(str, &constant, &arrays) == 's') return g_variant_new_strv(ptr, -1);
              if (arrays > 1) return g_variant_new_bytestring_array(ptr, -1);
              return g_variant_new_bytestring(ptr);
          }
      case '@':
          if G_UNLIKELY(!g_variant_is_of_type(ptr, (GVariantType*)*str))
              g_error("g_variant_new: expected GVariant of type `%s' but " "received value has type `%s'", g_variant_type_dup_string((GVariantType*)*str),
                      g_variant_get_type_string(ptr));
          g_variant_type_string_scan(*str, NULL, str);
          return ptr;
      case '*': return ptr;
      case '?':
          if G_UNLIKELY (!g_variant_type_is_basic(g_variant_get_type(ptr)))
              g_error("g_variant_new: format string `?' expects basic-typed GVariant, but received value has type `%s'", g_variant_get_type_string(ptr));
          return ptr;
      case 'r':
          if G_UNLIKELY (!g_variant_type_is_tuple(g_variant_get_type(ptr)))
              g_error ("g_variant_new: format string `r` expects tuple-typed GVariant, but received value has type `%s'", g_variant_get_type_string(ptr));
          return ptr;
      case 'v': return g_variant_new_variant(ptr);
      default: g_assert_not_reached();
  }
}
static gpointer g_variant_valist_get_nnp(const gchar **str, GVariant *value) {
  switch (*(*str)++) {
      case 'a':
          g_variant_type_string_scan(*str, NULL, str);
          return g_variant_iter_new(value);
      case '&':
          (*str)++;
          return (gchar *)g_variant_get_string(value, NULL);
      case 's': case 'o': case 'g': return g_variant_dup_string(value, NULL);
      case '^': {
          gboolean constant;
          guint arrays;
          if (g_variant_scan_convenience(str, &constant, &arrays) == 's') {
              if (constant) return g_variant_get_strv(value, NULL);
              else return g_variant_dup_strv(value, NULL);
          } else if (arrays > 1) {
              if (constant) return g_variant_get_bytestring_array(value, NULL);
              else return g_variant_dup_bytestring_array(value, NULL);
          } else {
              if (constant) return (gchar*)g_variant_get_bytestring(value);
              else return g_variant_dup_bytestring(value, NULL);
          }
      }
      case '@': g_variant_type_string_scan(*str, NULL, str);
      case '?': case 'r': return g_variant_ref(value);
      case 'v': return g_variant_get_variant(value);
      default: g_assert_not_reached();
  }
}
static void g_variant_valist_skip_leaf(const gchar **str, va_list *app) {
  if (g_variant_format_string_is_nnp(*str)) {
      g_variant_format_string_scan(*str, NULL, str);
      va_arg(*app, gpointer);
      return;
  }
  switch (*(*str)++) {
      case 'b': case 'y': case 'n': case 'q': case 'i': case 'u': case 'h':
      va_arg(*app, int);
      return;
      case 'x': case 't':
      va_arg (*app, guint64);
      return;
      case 'd':
      va_arg(*app, gdouble);
      return;
      default: g_assert_not_reached ();
  }
}
static GVariant *g_variant_valist_new_leaf(const gchar **str, va_list *app) {
  if (g_variant_format_string_is_nnp(*str)) return g_variant_valist_new_nnp(str, va_arg(*app, gpointer));
  switch(*(*str)++) {
      case 'b': return g_variant_new_boolean(va_arg(*app, gboolean));
      case 'y': return g_variant_new_byte(va_arg(*app, guint));
      case 'n': return g_variant_new_int16(va_arg(*app, gint));
      case 'q': return g_variant_new_uint16(va_arg(*app, guint));
      case 'i': return g_variant_new_int32(va_arg(*app, gint));
      case 'u': return g_variant_new_uint32(va_arg(*app, guint));
      case 'x': return g_variant_new_int64(va_arg(*app, gint64));
      case 't': return g_variant_new_uint64(va_arg(*app, guint64));
      case 'h': return g_variant_new_handle(va_arg(*app, gint));
      case 'd': return g_variant_new_double(va_arg(*app, gdouble));
      default: g_assert_not_reached();
  }
}
G_STATIC_ASSERT(sizeof(gboolean) == sizeof(guint32));
G_STATIC_ASSERT(sizeof(gdouble) == sizeof(guint64));
static void g_variant_valist_get_leaf(const gchar **str, GVariant *value, gboolean free, va_list *app) {
  gpointer ptr = va_arg (*app, gpointer);
  if (ptr == NULL) {
      g_variant_format_string_scan(*str, NULL, str);
      return;
  }
  if (g_variant_format_string_is_nnp(*str)) {
      gpointer *nnp = (gpointer*)ptr;
      if (free && *nnp != NULL) g_variant_valist_free_nnp(*str, *nnp);
      *nnp = NULL;
      if (value != NULL) *nnp = g_variant_valist_get_nnp(str, value);
      else g_variant_format_string_scan(*str, NULL, str);
      return;
  }
  if (value != NULL) {
      switch (*(*str)++) {
          case 'b':
              *(gboolean*)ptr = g_variant_get_boolean(value);
              return;
          case 'y':
              *(guchar*)ptr = g_variant_get_byte(value);
              return;
          case 'n':
              *(gint16*)ptr = g_variant_get_int16(value);
              return;
          case 'q':
              *(guint16*)ptr = g_variant_get_uint16(value);
              return;
          case 'i':
              *(gint32*)ptr = g_variant_get_int32(value);
              return;
          case 'u':
              *(guint32*)ptr = g_variant_get_uint32(value);
              return;
          case 'x':
              *(gint64*)ptr = g_variant_get_int64(value);
              return;
          case 't':
              *(guint64*)ptr = g_variant_get_uint64(value);
              return;
          case 'h':
              *(gint32*)ptr = g_variant_get_handle(value);
              return;
          case 'd':
              *(gdouble*)ptr = g_variant_get_double(value);
              return;
      }
  } else {
      switch(*(*str)++) {
          case 'y':
              *(guchar*)ptr = 0;
              return;
          case 'n': case 'q':
              *(guint16*)ptr = 0;
              return;
          case 'i': case 'u': case 'h': case 'b':
              *(guint32*)ptr = 0;
              return;
          case 'x': case 't': case 'd':
              *(guint64*)ptr = 0;
              return;
      }
  }
  g_assert_not_reached();
}
static void g_variant_valist_skip(const gchar **str, va_list *app) {
  if (g_variant_format_string_is_leaf(*str)) g_variant_valist_skip_leaf (str, app);
  else if (**str == 'm') {
      (*str)++;
      if (!g_variant_format_string_is_nnp(*str)) va_arg(*app, gboolean);
      g_variant_valist_skip(str, app);
  } else {
      g_assert(**str == '(' || **str == '{');
      (*str)++;
      while (**str != ')' && **str != '}') g_variant_valist_skip (str, app);
      (*str)++;
  }
}
static GVariant *g_variant_valist_new(const gchar **str, va_list *app) {
  if (g_variant_format_string_is_leaf(*str)) return g_variant_valist_new_leaf(str, app);
  if (**str == 'm') {
      GVariantType *type = NULL;
      GVariant *value = NULL;
      (*str)++;
      if (g_variant_format_string_is_nnp(*str)) {
          gpointer nnp = va_arg (*app, gpointer);
          if (nnp != NULL) value = g_variant_valist_new_nnp (str, nnp);
          else type = g_variant_format_string_scan_type (*str, NULL, str);
      } else {
          gboolean just = va_arg(*app, gboolean);
          if (just) value = g_variant_valist_new (str, app);
          else {
              type = g_variant_format_string_scan_type(*str, NULL, NULL);
              g_variant_valist_skip(str, app);
          }
      }
      value = g_variant_new_maybe (type, value);
      if (type != NULL) g_variant_type_free(type);
      return value;
  } else {
      GVariantBuilder b;
      if (**str == '(') g_variant_builder_init(&b, G_VARIANT_TYPE_TUPLE);
      else {
          g_assert(**str == '{');
          g_variant_builder_init(&b, G_VARIANT_TYPE_DICT_ENTRY);
      }
      (*str)++;
      while(**str != ')' && **str != '}') g_variant_builder_add_value(&b, g_variant_valist_new(str, app));
      (*str)++;
      return g_variant_builder_end(&b);
    }
}
static void g_variant_valist_get(const gchar **str, GVariant *value, gboolean free, va_list *app) {
  if (g_variant_format_string_is_leaf (*str)) g_variant_valist_get_leaf (str, value, free, app);
  else if (**str == 'm') {
      (*str)++;
      if (value != NULL) value = g_variant_get_maybe (value);
      if (!g_variant_format_string_is_nnp (*str)) {
          gboolean *ptr = va_arg (*app, gboolean *);
          if (ptr != NULL) *ptr = value != NULL;
      }
      g_variant_valist_get (str, value, free, app);
      if (value != NULL) g_variant_unref (value);
  } else {
      gint index = 0;
      g_assert(**str == '(' || **str == '{');
      (*str)++;
      while(**str != ')' && **str != '}') {
          if (value != NULL) {
              GVariant *child = g_variant_get_child_value(value, index++);
              g_variant_valist_get(str, child, free, app);
              g_variant_unref(child);
          } else g_variant_valist_get(str, NULL, free, app);
      }
      (*str)++;
  }
}
GVariant *g_variant_new(const gchar *format_string, ...) {
  GVariant *value;
  va_list ap;
  g_return_val_if_fail(valid_format_string (format_string, TRUE, NULL) && format_string[0] != '?' && format_string[0] != '@' &&
                        format_string[0] != '*' && format_string[0] != 'r', NULL);
  va_start(ap, format_string);
  value = g_variant_new_va(format_string, NULL, &ap);
  va_end(ap);
  return value;
}
GVariant *g_variant_new_va(const gchar  *format_string, const gchar **endptr, va_list *app) {
  GVariant *value;
  g_return_val_if_fail(valid_format_string (format_string, !endptr, NULL), NULL);
  g_return_val_if_fail(app != NULL, NULL);
  value = g_variant_valist_new(&format_string, app);
  if (endptr != NULL) *endptr = format_string;
  return value;
}
void g_variant_get(GVariant *value, const gchar *format_string, ...) {
  va_list ap;
  g_return_if_fail(valid_format_string(format_string, TRUE, value));
  if (strchr(format_string, '&')) g_variant_get_data(value);
  va_start(ap, format_string);
  g_variant_get_va(value, format_string, NULL, &ap);
  va_end(ap);
}
void g_variant_get_va(GVariant *value, const gchar *format_string, const gchar **endptr, va_list *app) {
  g_return_if_fail(valid_format_string (format_string, !endptr, value));
  g_return_if_fail(value != NULL);
  g_return_if_fail(app != NULL);
  if (strchr(format_string, '&')) g_variant_get_data(value);
  g_variant_valist_get(&format_string, value, FALSE, app);
  if (endptr != NULL) *endptr = format_string;
}
void g_variant_builder_add(GVariantBuilder *builder, const gchar *format_string, ...) {
  GVariant *variant;
  va_list ap;
  va_start(ap, format_string);
  variant = g_variant_new_va(format_string, NULL, &ap);
  va_end(ap);
  g_variant_builder_add_value(builder, variant);
}
void g_variant_get_child(GVariant *value, gsize index_, const gchar *format_string, ...) {
  GVariant *child;
  va_list ap;
  child = g_variant_get_child_value(value, index_);
  g_return_if_fail(valid_format_string(format_string, TRUE, child));
  va_start(ap, format_string);
  g_variant_get_va(child, format_string, NULL, &ap);
  va_end(ap);
  g_variant_unref(child);
}
gboolean g_variant_iter_next(GVariantIter *iter, const gchar *format_string, ...) {
  GVariant *value;
  value = g_variant_iter_next_value (iter);
  g_return_val_if_fail(valid_format_string (format_string, TRUE, value), FALSE);
  if (value != NULL) {
      va_list ap;
      va_start(ap, format_string);
      g_variant_valist_get(&format_string, value, FALSE, &ap);
      va_end(ap);
      g_variant_unref(value);
  }
  return value != NULL;
}
gboolean g_variant_iter_loop (GVariantIter *iter, const gchar *format_string, ...) {
  gboolean first_time = GVSI(iter)->loop_format == NULL;
  GVariant *value;
  va_list ap;
  g_return_val_if_fail(first_time || format_string == GVSI(iter)->loop_format, FALSE);
  if (first_time) {
      TYPE_CHECK(GVSI(iter)->value, G_VARIANT_TYPE_ARRAY, FALSE);
      GVSI(iter)->loop_format = format_string;
      if (strchr(format_string, '&')) g_variant_get_data(GVSI(iter)->value);
  }
  value = g_variant_iter_next_value (iter);
  g_return_val_if_fail(!first_time || valid_format_string(format_string, TRUE, value), FALSE);
  va_start(ap, format_string);
  g_variant_valist_get(&format_string, value, !first_time, &ap);
  va_end(ap);
  if (value != NULL) g_variant_unref(value);
  return value != NULL;
}
static GVariant *g_variant_deep_copy(GVariant *value) {
  switch (g_variant_classify (value)) {
      case G_VARIANT_CLASS_MAYBE: case G_VARIANT_CLASS_ARRAY: case G_VARIANT_CLASS_TUPLE: case G_VARIANT_CLASS_DICT_ENTRY: case G_VARIANT_CLASS_VARIANT: {
              GVariantBuilder builder;
              GVariantIter iter;
              GVariant *child;
              g_variant_builder_init(&builder, g_variant_get_type(value));
              g_variant_iter_init(&iter, value);
              while((child = g_variant_iter_next_value(&iter))) {
                  g_variant_builder_add_value(&builder, g_variant_deep_copy(child));
                  g_variant_unref(child);
              }
              return g_variant_builder_end(&builder);
          }
      case G_VARIANT_CLASS_BOOLEAN: return g_variant_new_boolean(g_variant_get_boolean(value));
      case G_VARIANT_CLASS_BYTE: return g_variant_new_byte(g_variant_get_byte(value));
      case G_VARIANT_CLASS_INT16: return g_variant_new_int16(g_variant_get_int16(value));
      case G_VARIANT_CLASS_UINT16: return g_variant_new_uint16(g_variant_get_uint16(value));
      case G_VARIANT_CLASS_INT32: return g_variant_new_int32(g_variant_get_int32(value));
      case G_VARIANT_CLASS_UINT32: return g_variant_new_uint32(g_variant_get_uint32(value));
      case G_VARIANT_CLASS_INT64: return g_variant_new_int64(g_variant_get_int64(value));
      case G_VARIANT_CLASS_UINT64: return g_variant_new_uint64(g_variant_get_uint64(value));
      case G_VARIANT_CLASS_HANDLE: return g_variant_new_handle(g_variant_get_handle(value));
      case G_VARIANT_CLASS_DOUBLE: return g_variant_new_double(g_variant_get_double(value));
      case G_VARIANT_CLASS_STRING: return g_variant_new_string(g_variant_get_string(value, NULL));
      case G_VARIANT_CLASS_OBJECT_PATH: return g_variant_new_object_path (g_variant_get_string(value, NULL));
      case G_VARIANT_CLASS_SIGNATURE: return g_variant_new_signature (g_variant_get_string(value, NULL));
  }
  g_assert_not_reached();
}
GVariant *
g_variant_get_normal_form(GVariant *value) {
  GVariant *trusted;
  if (g_variant_is_normal_form(value)) return g_variant_ref(value);
  trusted = g_variant_deep_copy(value);
  g_assert(g_variant_is_trusted(trusted));
  return g_variant_ref_sink(trusted);
}
GVariant *g_variant_byteswap(GVariant *value) {
  GVariantTypeInfo *type_info;
  guint alignment;
  GVariant *new;
  type_info = g_variant_get_type_info (value);
  g_variant_type_info_query (type_info, &alignment, NULL);
  if (alignment) {
      GVariantSerialised serialised;
      GVariant *trusted;
      GBuffer *buffer;
      trusted = g_variant_get_normal_form(value);
      serialised.type_info = g_variant_get_type_info(trusted);
      serialised.size = g_variant_get_size (trusted);
      serialised.data = g_malloc(serialised.size);
      g_variant_store(trusted, serialised.data);
      g_variant_unref(trusted);
      g_variant_serialised_byteswap(serialised);
      buffer = g_buffer_new_take_data(serialised.data, serialised.size);
      new = g_variant_new_from_buffer(g_variant_get_type(value), buffer, TRUE);
      g_buffer_unref(buffer);
  } else new = value;
  return g_variant_ref_sink(new);
}
GVariant *g_variant_new_from_data(const GVariantType *type, gconstpointer data, gsize size, gboolean trusted, GDestroyNotify notify, gpointer user_data) {
  GVariant *value;
  GBuffer *buffer;
  g_return_val_if_fail(g_variant_type_is_definite (type), NULL);
  g_return_val_if_fail(data != NULL || size == 0, NULL);
  if (notify) buffer = g_buffer_new_from_pointer(data, size, notify, user_data);
  else buffer = g_buffer_new_from_static_data(data, size);
  value = g_variant_new_from_buffer(type, buffer, trusted);
  g_buffer_unref(buffer);
  return value;
}