#include <string.h>
#include "../glib/glib-object.h"
#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "config.h"
#include "gfileattribute.h"
#include "gfileattribute-priv.h"

void _g_file_attribute_value_free(GFileAttributeValue *attr) {
  g_return_if_fail(attr != NULL);
  _g_file_attribute_value_clear(attr);
  g_free(attr);
}
void _g_file_attribute_value_clear(GFileAttributeValue *attr) {
  g_return_if_fail(attr != NULL);
  if (attr->type == G_FILE_ATTRIBUTE_TYPE_STRING || attr->type == G_FILE_ATTRIBUTE_TYPE_BYTE_STRING) g_free(attr->u.string);
  if (attr->type == G_FILE_ATTRIBUTE_TYPE_STRINGV) g_strfreev(attr->u.stringv);
  if (attr->type == G_FILE_ATTRIBUTE_TYPE_OBJECT && attr->u.obj != NULL) g_object_unref(attr->u.obj);
  attr->type = G_FILE_ATTRIBUTE_TYPE_INVALID;
}
void _g_file_attribute_value_set(GFileAttributeValue *attr, const GFileAttributeValue *new_value) {
  g_return_if_fail(attr != NULL);
  g_return_if_fail(new_value != NULL);
  _g_file_attribute_value_clear(attr);
  *attr = *new_value;
  if (attr->type == G_FILE_ATTRIBUTE_TYPE_STRING || attr->type == G_FILE_ATTRIBUTE_TYPE_BYTE_STRING) attr->u.string = g_strdup(attr->u.string);
  if (attr->type == G_FILE_ATTRIBUTE_TYPE_STRINGV) attr->u.stringv = g_strdupv(attr->u.stringv);
  if (attr->type == G_FILE_ATTRIBUTE_TYPE_OBJECT && attr->u.obj != NULL) g_object_ref(attr->u.obj);
}
GFileAttributeValue *_g_file_attribute_value_new(void) {
  GFileAttributeValue *attr;
  attr = g_new(GFileAttributeValue, 1);
  attr->type = G_FILE_ATTRIBUTE_TYPE_INVALID;
  return attr;
}
gpointer _g_file_attribute_value_peek_as_pointer(GFileAttributeValue *attr) {
  switch(attr->type) {
      case G_FILE_ATTRIBUTE_TYPE_STRING: case G_FILE_ATTRIBUTE_TYPE_BYTE_STRING: return attr->u.string;
      case G_FILE_ATTRIBUTE_TYPE_STRINGV: return attr->u.stringv;
      case G_FILE_ATTRIBUTE_TYPE_OBJECT: return attr->u.obj;
      default: return (gpointer) &attr->u;
  }
}
GFileAttributeValue *_g_file_attribute_value_dup(const GFileAttributeValue *other) {
  GFileAttributeValue *attr;
  g_return_val_if_fail(other != NULL, NULL);
  attr = g_new(GFileAttributeValue, 1);
  attr->type = G_FILE_ATTRIBUTE_TYPE_INVALID;
  _g_file_attribute_value_set(attr, other);
  return attr;
}
G_DEFINE_BOXED_TYPE(GFileAttributeInfoList, g_file_attribute_info_list, g_file_attribute_info_list_dup, g_file_attribute_info_list_unref);
static gboolean valid_char(char c) {
  return c >= 32 && c <= 126 && c != '\\';
}
static char *escape_byte_string(const char *str) {
  size_t len;
  int num_invalid, i;
  char *escaped_val, *p;
  unsigned char c;
  const char hex_digits[] = "0123456789abcdef";
  len = strlen(str);
  num_invalid = 0;
  for (i = 0; i < len; i++) {
      if (!valid_char(str[i])) num_invalid++;
  }
  if (num_invalid == 0) return g_strdup(str);
  else {
      escaped_val = g_malloc(len + num_invalid*3 + 1);
      p = escaped_val;
      for (i = 0; i < len; i++) {
          c = str[i];
          if (valid_char(c)) *p++ = c;
          else {
              *p++ = '\\';
              *p++ = 'x';
              *p++ = hex_digits[(c >> 4) & 0xf];
              *p++ = hex_digits[c & 0xf];
          }
	  }
      *p++ = 0;
      return escaped_val;
  }
}
char *_g_file_attribute_value_as_string(const GFileAttributeValue *attr) {
  GString *s;
  int i;
  char *str;
  g_return_val_if_fail(attr != NULL, NULL);
  switch (attr->type) {
      case G_FILE_ATTRIBUTE_TYPE_STRING: str = g_strdup(attr->u.string); break;
      case G_FILE_ATTRIBUTE_TYPE_STRINGV:
          s = g_string_new("[");
          for (i = 0; attr->u.stringv[i] != NULL; i++) {
              g_string_append(s, attr->u.stringv[i]);
              if (attr->u.stringv[i+1] != NULL) g_string_append(s, ", ");
          }
          g_string_append(s, "]");
          str = g_string_free(s, FALSE);
          break;
      case G_FILE_ATTRIBUTE_TYPE_BYTE_STRING: str = escape_byte_string(attr->u.string); break;
      case G_FILE_ATTRIBUTE_TYPE_BOOLEAN: str = g_strdup_printf("%s", attr->u.boolean?"TRUE":"FALSE"); break;
      case G_FILE_ATTRIBUTE_TYPE_UINT32: str = g_strdup_printf("%u", (unsigned int)attr->u.uint32); break;
      case G_FILE_ATTRIBUTE_TYPE_INT32: str = g_strdup_printf("%i", (int)attr->u.int32); break;
      case G_FILE_ATTRIBUTE_TYPE_UINT64: str = g_strdup_printf("%"G_GUINT64_FORMAT, attr->u.uint64); break;
      case G_FILE_ATTRIBUTE_TYPE_INT64: str = g_strdup_printf("%"G_GINT64_FORMAT, attr->u.int64); break;
      case G_FILE_ATTRIBUTE_TYPE_OBJECT:
          str = g_strdup_printf("%s:%p", g_type_name_from_instance((GTypeInstance*)attr->u.obj), attr->u.obj);
          break;
      case G_FILE_ATTRIBUTE_TYPE_INVALID: str = g_strdup("<unset>"); break;
      default:
          g_warning("Invalid type in GFileInfo attribute");
          str = g_strdup("<invalid>");
          break;
  }
  return str;
}
const char *_g_file_attribute_value_get_string(const GFileAttributeValue *attr) {
  if (attr == NULL) return NULL;
  g_return_val_if_fail(attr->type == G_FILE_ATTRIBUTE_TYPE_STRING, NULL);
  return attr->u.string;
}
const char *_g_file_attribute_value_get_byte_string(const GFileAttributeValue *attr) {
  if (attr == NULL) return NULL;
  g_return_val_if_fail(attr->type == G_FILE_ATTRIBUTE_TYPE_BYTE_STRING, NULL);
  return attr->u.string;
}
char **_g_file_attribute_value_get_stringv(const GFileAttributeValue *attr) {
  if (attr == NULL) return NULL;
  g_return_val_if_fail(attr->type == G_FILE_ATTRIBUTE_TYPE_STRINGV, NULL);
  return attr->u.stringv;
}
gboolean _g_file_attribute_value_get_boolean(const GFileAttributeValue *attr) {
  if (attr == NULL) return FALSE;
  g_return_val_if_fail(attr->type == G_FILE_ATTRIBUTE_TYPE_BOOLEAN, FALSE);
  return attr->u.boolean;
}
guint32 _g_file_attribute_value_get_uint32(const GFileAttributeValue *attr) {
  if (attr == NULL) return 0;
  g_return_val_if_fail(attr->type == G_FILE_ATTRIBUTE_TYPE_UINT32, 0);
  return attr->u.uint32;
}
gint32 _g_file_attribute_value_get_int32(const GFileAttributeValue *attr) {
  if (attr == NULL) return 0;
  g_return_val_if_fail(attr->type == G_FILE_ATTRIBUTE_TYPE_INT32, 0);
  return attr->u.int32;
}
guint64 _g_file_attribute_value_get_uint64(const GFileAttributeValue *attr) {
  if (attr == NULL) return 0;
  g_return_val_if_fail (attr->type == G_FILE_ATTRIBUTE_TYPE_UINT64, 0);
  return attr->u.uint64;
}
gint64 _g_file_attribute_value_get_int64(const GFileAttributeValue *attr) {
  if (attr == NULL) return 0;
  g_return_val_if_fail(attr->type == G_FILE_ATTRIBUTE_TYPE_INT64, 0);
  return attr->u.int64;
}
GObject *_g_file_attribute_value_get_object(const GFileAttributeValue *attr) {
  if (attr == NULL) return NULL;
  g_return_val_if_fail(attr->type == G_FILE_ATTRIBUTE_TYPE_OBJECT, NULL);
  return attr->u.obj;
}
void _g_file_attribute_value_set_from_pointer (GFileAttributeValue *value, GFileAttributeType type, gpointer value_p, gboolean dup) {
  _g_file_attribute_value_clear(value);
  value->type = type;
  switch(type) {
      case G_FILE_ATTRIBUTE_TYPE_STRING: case G_FILE_ATTRIBUTE_TYPE_BYTE_STRING:
          if (dup) value->u.string = g_strdup(value_p);
          else value->u.string = value_p;
          break;
      case G_FILE_ATTRIBUTE_TYPE_STRINGV:
          if (dup) value->u.stringv = g_strdupv(value_p);
          else value->u.stringv = value_p;
          break;
      case G_FILE_ATTRIBUTE_TYPE_OBJECT:
          if (dup) value->u.obj = g_object_ref(value_p);
          else value->u.obj = value_p;
          break;
      case G_FILE_ATTRIBUTE_TYPE_BOOLEAN: value->u.boolean = *(gboolean*)value_p; break;
      case G_FILE_ATTRIBUTE_TYPE_UINT32: value->u.uint32 = *(guint32*)value_p; break;
      case G_FILE_ATTRIBUTE_TYPE_INT32: value->u.int32 = *(gint32*)value_p; break;
      case G_FILE_ATTRIBUTE_TYPE_UINT64: value->u.uint64 = *(guint64*)value_p; break;
      case G_FILE_ATTRIBUTE_TYPE_INT64: value->u.int64 = *(gint64*)value_p; break;
      case G_FILE_ATTRIBUTE_TYPE_INVALID: break;
      default: g_warning("Unknown type specified in g_file_info_set_attribute\n"); break;
  }
}
void _g_file_attribute_value_set_string(GFileAttributeValue *attr, const char *string) {
  g_return_if_fail(attr != NULL);
  g_return_if_fail(string != NULL);
  _g_file_attribute_value_clear(attr);
  attr->type = G_FILE_ATTRIBUTE_TYPE_STRING;
  attr->u.string = g_strdup(string);
}
void _g_file_attribute_value_set_byte_string(GFileAttributeValue *attr, const char *string) {
  g_return_if_fail(attr != NULL);
  g_return_if_fail(string != NULL);
  _g_file_attribute_value_clear(attr);
  attr->type = G_FILE_ATTRIBUTE_TYPE_BYTE_STRING;
  attr->u.string = g_strdup(string);
}
void _g_file_attribute_value_set_stringv(GFileAttributeValue *attr, char **value) {
  g_return_if_fail(attr != NULL);
  g_return_if_fail(value != NULL);
  _g_file_attribute_value_clear(attr);
  attr->type = G_FILE_ATTRIBUTE_TYPE_STRINGV;
  attr->u.stringv = g_strdupv(value);
}
void _g_file_attribute_value_set_boolean(GFileAttributeValue *attr, gboolean value) {
  g_return_if_fail(attr != NULL);
  _g_file_attribute_value_clear(attr);
  attr->type = G_FILE_ATTRIBUTE_TYPE_BOOLEAN;
  attr->u.boolean = !!value;
}
void _g_file_attribute_value_set_uint32(GFileAttributeValue *attr, guint32 value) {
  g_return_if_fail(attr != NULL);
  _g_file_attribute_value_clear(attr);
  attr->type = G_FILE_ATTRIBUTE_TYPE_UINT32;
  attr->u.uint32 = value;
}
void _g_file_attribute_value_set_int32(GFileAttributeValue *attr, gint32 value) {
  g_return_if_fail(attr != NULL);
  _g_file_attribute_value_clear(attr);
  attr->type = G_FILE_ATTRIBUTE_TYPE_INT32;
  attr->u.int32 = value;
}
void _g_file_attribute_value_set_uint64(GFileAttributeValue *attr, guint64 value) {
  g_return_if_fail(attr != NULL);
  _g_file_attribute_value_clear(attr);
  attr->type = G_FILE_ATTRIBUTE_TYPE_UINT64;
  attr->u.uint64 = value;
}
void _g_file_attribute_value_set_int64(GFileAttributeValue *attr, gint64 value) {
  g_return_if_fail(attr != NULL);
  _g_file_attribute_value_clear(attr);
  attr->type = G_FILE_ATTRIBUTE_TYPE_INT64;
  attr->u.int64 = value;
}
void _g_file_attribute_value_set_object(GFileAttributeValue *attr, GObject *obj) {
  g_return_if_fail(attr != NULL);
  g_return_if_fail(obj != NULL);
  _g_file_attribute_value_clear(attr);
  attr->type = G_FILE_ATTRIBUTE_TYPE_OBJECT;
  attr->u.obj = g_object_ref(obj);
}
typedef struct {
  GFileAttributeInfoList public;
  GArray *array;
  int ref_count;
} GFileAttributeInfoListPriv;
static void list_update_public(GFileAttributeInfoListPriv *priv) {
  priv->public.infos = (GFileAttributeInfo*)priv->array->data;
  priv->public.n_infos = priv->array->len;
}
GFileAttributeInfoList *g_file_attribute_info_list_new(void) {
  GFileAttributeInfoListPriv *priv;
  priv = g_new0 (GFileAttributeInfoListPriv, 1);
  priv->ref_count = 1;
  priv->array = g_array_new (TRUE, FALSE, sizeof (GFileAttributeInfo));
  list_update_public (priv);
  return (GFileAttributeInfoList *)priv;
}
GFileAttributeInfoList *g_file_attribute_info_list_dup (GFileAttributeInfoList *list) {
  GFileAttributeInfoListPriv *new;
  int i;
  g_return_val_if_fail(list != NULL, NULL);
  new = g_new0(GFileAttributeInfoListPriv, 1);
  new->ref_count = 1;
  new->array = g_array_new(TRUE, FALSE, sizeof(GFileAttributeInfo));
  g_array_set_size(new->array, list->n_infos);
  list_update_public(new);
  for (i = 0; i < list->n_infos; i++) {
      new->public.infos[i].name = g_strdup(list->infos[i].name);
      new->public.infos[i].type = list->infos[i].type;
      new->public.infos[i].flags = list->infos[i].flags;
  }
  return (GFileAttributeInfoList*)new;
}
GFileAttributeInfoList *g_file_attribute_info_list_ref(GFileAttributeInfoList *list) {
  GFileAttributeInfoListPriv *priv = (GFileAttributeInfoListPriv*)list;
  g_return_val_if_fail(list != NULL, NULL);
  g_return_val_if_fail(priv->ref_count > 0, NULL);
  g_atomic_int_inc(&priv->ref_count);
  return list;
}
void g_file_attribute_info_list_unref (GFileAttributeInfoList *list) {
  GFileAttributeInfoListPriv *priv = (GFileAttributeInfoListPriv*)list;
  int i;
  g_return_if_fail(list != NULL);
  g_return_if_fail(priv->ref_count > 0);
  if (g_atomic_int_dec_and_test(&priv->ref_count)) {
      for (i = 0; i < list->n_infos; i++) g_free(list->infos[i].name);
      g_array_free(priv->array, TRUE);
  }
}
static int g_file_attribute_info_list_bsearch(GFileAttributeInfoList *list, const char *name) {
  int start, end, mid;
  start = 0;
  end = list->n_infos;
  while(start != end) {
      mid = start + (end - start) / 2;
      if (strcmp(name, list->infos[mid].name) < 0) end = mid;
      else if (strcmp(name, list->infos[mid].name) > 0) start = mid + 1;
      else return mid;
  }
  return start;
}
const GFileAttributeInfo *g_file_attribute_info_list_lookup(GFileAttributeInfoList *list, const char *name) {
  int i;
  g_return_val_if_fail(list != NULL, NULL);
  g_return_val_if_fail(name != NULL, NULL);
  i = g_file_attribute_info_list_bsearch(list, name);
  if (i < list->n_infos && strcmp(list->infos[i].name, name) == 0) return &list->infos[i];
  return NULL;
}
void g_file_attribute_info_list_add(GFileAttributeInfoList *list, const char *name, GFileAttributeType type, GFileAttributeInfoFlags flags) {
  GFileAttributeInfoListPriv *priv = (GFileAttributeInfoListPriv*)list;
  GFileAttributeInfo info;
  int i;
  g_return_if_fail(list != NULL);
  g_return_if_fail(name != NULL);
  i = g_file_attribute_info_list_bsearch(list, name);
  if (i < list->n_infos && strcmp(list->infos[i].name, name) == 0) {
      list->infos[i].type = type;
      return;
  }
  info.name = g_strdup(name);
  info.type = type;
  info.flags = flags;
  g_array_insert_vals(priv->array, i, &info, 1);
  list_update_public(priv);
}