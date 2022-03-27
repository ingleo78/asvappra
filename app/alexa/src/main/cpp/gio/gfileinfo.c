#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "../gobject/gboxed.h"
#include "config.h"
#include "gfileinfo.h"
#include "gfileinfo-priv.h"
#include "gfileattribute-priv.h"
#include "gicon.h"

#define NO_ATTRIBUTE_MASK ((GFileAttributeMatcher *)1)
typedef struct  {
  guint32 attribute;
  GFileAttributeValue value;
} GFileAttribute;
struct _GFileInfo {
  GObject parent_instance;
  GArray *attributes;
  GFileAttributeMatcher *mask;
};
struct _GFileInfoClass {
  GObjectClass parent_class;
};
G_DEFINE_TYPE(GFileInfo, g_file_info, G_TYPE_OBJECT);
typedef struct {
  guint32 id;
  guint32 attribute_id_counter;
} NSInfo;
G_LOCK_DEFINE_STATIC(attribute_hash);
static int namespace_id_counter = 0;
static GHashTable *ns_hash = NULL;
static GHashTable *attribute_hash = NULL;
static char ***attributes = NULL;
#define NS_POS 20
#define NS_MASK ((guint32)((1<<12) - 1))
#define ID_POS 0
#define ID_MASK ((guint32)((1<<20) - 1))
#define GET_NS(_attr_id)  (((guint32) (_attr_id) >> NS_POS) & NS_MASK)
#define GET_ID(_attr_id)  (((guint32)(_attr_id) >> ID_POS) & ID_MASK)
#define MAKE_ATTR_ID(_ns, _id)	 (((((guint32) _ns) & NS_MASK) << NS_POS) | ((((guint32) _id) & ID_MASK) << ID_POS))
static NSInfo *_lookup_namespace(const char *namespace) {
  NSInfo *ns_info;
  ns_info = g_hash_table_lookup(ns_hash, namespace);
  if (ns_info == NULL) {
      ns_info = g_new0(NSInfo, 1);
      ns_info->id = ++namespace_id_counter;
      g_hash_table_insert(ns_hash, g_strdup(namespace), ns_info);
      attributes = g_realloc(attributes,(ns_info->id + 1) * sizeof (char **));
      attributes[ns_info->id] = NULL;
  }
  return ns_info;
}
static guint32 _lookup_attribute(const char *attribute) {
  guint32 attr_id, id;
  char *ns;
  const char *colon;
  NSInfo *ns_info;
  attr_id = GPOINTER_TO_UINT(g_hash_table_lookup(attribute_hash, attribute));
  if (attr_id != 0) return attr_id;
  colon = strstr (attribute, "::");
  if (colon) ns = g_strndup(attribute, colon - attribute);
  else ns = g_strdup("");
  ns_info = _lookup_namespace(ns);
  g_free(ns);
  id = ++ns_info->attribute_id_counter;
  attributes[ns_info->id] = g_realloc(attributes[ns_info->id], (id + 1) * sizeof(char*));
  attributes[ns_info->id][id] = g_strdup(attribute);
  attr_id = MAKE_ATTR_ID(ns_info->id, id);
  g_hash_table_insert(attribute_hash, attributes[ns_info->id][id], GUINT_TO_POINTER(attr_id));
  return attr_id;
}
static void ensure_attribute_hash(void) {
  if (attribute_hash != NULL) return;
  ns_hash = g_hash_table_new(g_str_hash, g_str_equal);
  attribute_hash = g_hash_table_new(g_str_hash, g_str_equal);
  #define REGISTER_ATTRIBUTE(name) G_STMT_START{\
      guint _u = _lookup_attribute(G_FILE_ATTRIBUTE_ ## name); \
      g_assert (_u == G_FILE_ATTRIBUTE_ID_ ## name); \
  } G_STMT_END
  REGISTER_ATTRIBUTE(STANDARD_TYPE);
  REGISTER_ATTRIBUTE(STANDARD_IS_HIDDEN);
  REGISTER_ATTRIBUTE(STANDARD_IS_BACKUP);
  REGISTER_ATTRIBUTE(STANDARD_IS_SYMLINK);
  REGISTER_ATTRIBUTE(STANDARD_IS_VIRTUAL);
  REGISTER_ATTRIBUTE(STANDARD_NAME);
  REGISTER_ATTRIBUTE(STANDARD_DISPLAY_NAME);
  REGISTER_ATTRIBUTE(STANDARD_EDIT_NAME);
  REGISTER_ATTRIBUTE(STANDARD_COPY_NAME);
  REGISTER_ATTRIBUTE(STANDARD_DESCRIPTION);
  REGISTER_ATTRIBUTE(STANDARD_ICON);
  REGISTER_ATTRIBUTE(STANDARD_CONTENT_TYPE);
  REGISTER_ATTRIBUTE(STANDARD_FAST_CONTENT_TYPE);
  REGISTER_ATTRIBUTE(STANDARD_SIZE);
  REGISTER_ATTRIBUTE(STANDARD_ALLOCATED_SIZE);
  REGISTER_ATTRIBUTE(STANDARD_SYMLINK_TARGET);
  REGISTER_ATTRIBUTE(STANDARD_TARGET_URI);
  REGISTER_ATTRIBUTE(STANDARD_SORT_ORDER);
  REGISTER_ATTRIBUTE(ETAG_VALUE);
  REGISTER_ATTRIBUTE(ID_FILE);
  REGISTER_ATTRIBUTE(ID_FILESYSTEM);
  REGISTER_ATTRIBUTE(ACCESS_CAN_READ);
  REGISTER_ATTRIBUTE(ACCESS_CAN_WRITE);
  REGISTER_ATTRIBUTE(ACCESS_CAN_EXECUTE);
  REGISTER_ATTRIBUTE(ACCESS_CAN_DELETE);
  REGISTER_ATTRIBUTE(ACCESS_CAN_TRASH);
  REGISTER_ATTRIBUTE(ACCESS_CAN_RENAME);
  REGISTER_ATTRIBUTE(MOUNTABLE_CAN_MOUNT);
  REGISTER_ATTRIBUTE(MOUNTABLE_CAN_UNMOUNT);
  REGISTER_ATTRIBUTE(MOUNTABLE_CAN_EJECT);
  REGISTER_ATTRIBUTE(MOUNTABLE_UNIX_DEVICE);
  REGISTER_ATTRIBUTE(MOUNTABLE_UNIX_DEVICE_FILE);
  REGISTER_ATTRIBUTE(MOUNTABLE_HAL_UDI);
  REGISTER_ATTRIBUTE(MOUNTABLE_CAN_START);
  REGISTER_ATTRIBUTE(MOUNTABLE_CAN_START_DEGRADED);
  REGISTER_ATTRIBUTE(MOUNTABLE_CAN_STOP);
  REGISTER_ATTRIBUTE(MOUNTABLE_START_STOP_TYPE);
  REGISTER_ATTRIBUTE(MOUNTABLE_CAN_POLL);
  REGISTER_ATTRIBUTE(MOUNTABLE_IS_MEDIA_CHECK_AUTOMATIC);
  REGISTER_ATTRIBUTE(TIME_MODIFIED);
  REGISTER_ATTRIBUTE(TIME_MODIFIED_USEC);
  REGISTER_ATTRIBUTE(TIME_ACCESS);
  REGISTER_ATTRIBUTE(TIME_ACCESS_USEC);
  REGISTER_ATTRIBUTE(TIME_CHANGED);
  REGISTER_ATTRIBUTE(TIME_CHANGED_USEC);
  REGISTER_ATTRIBUTE(TIME_CREATED);
  REGISTER_ATTRIBUTE(TIME_CREATED_USEC);
  REGISTER_ATTRIBUTE(UNIX_DEVICE);
  REGISTER_ATTRIBUTE(UNIX_INODE);
  REGISTER_ATTRIBUTE(UNIX_MODE);
  REGISTER_ATTRIBUTE(UNIX_NLINK);
  REGISTER_ATTRIBUTE(UNIX_UID);
  REGISTER_ATTRIBUTE(UNIX_GID);
  REGISTER_ATTRIBUTE(UNIX_RDEV);
  REGISTER_ATTRIBUTE(UNIX_BLOCK_SIZE);
  REGISTER_ATTRIBUTE(UNIX_BLOCKS);
  REGISTER_ATTRIBUTE(UNIX_IS_MOUNTPOINT);
  REGISTER_ATTRIBUTE(DOS_IS_ARCHIVE);
  REGISTER_ATTRIBUTE(DOS_IS_SYSTEM);
  REGISTER_ATTRIBUTE(OWNER_USER);
  REGISTER_ATTRIBUTE(OWNER_USER_REAL);
  REGISTER_ATTRIBUTE(OWNER_GROUP);
  REGISTER_ATTRIBUTE(THUMBNAIL_PATH);
  REGISTER_ATTRIBUTE(THUMBNAILING_FAILED);
  REGISTER_ATTRIBUTE(PREVIEW_ICON);
  REGISTER_ATTRIBUTE(FILESYSTEM_SIZE);
  REGISTER_ATTRIBUTE(FILESYSTEM_FREE);
  REGISTER_ATTRIBUTE(FILESYSTEM_TYPE);
  REGISTER_ATTRIBUTE(FILESYSTEM_READONLY);
  REGISTER_ATTRIBUTE(FILESYSTEM_USE_PREVIEW);
  REGISTER_ATTRIBUTE(GVFS_BACKEND);
  REGISTER_ATTRIBUTE(SELINUX_CONTEXT);
  REGISTER_ATTRIBUTE(TRASH_ITEM_COUNT);
  REGISTER_ATTRIBUTE(TRASH_ORIG_PATH);
  REGISTER_ATTRIBUTE(TRASH_DELETION_DATE);
  #undef REGISTER_ATTRIBUTE
}
static guint32 lookup_namespace(const char *namespace) {
  NSInfo *ns_info;
  guint32 id;
  G_LOCK(attribute_hash);
  ensure_attribute_hash();
  ns_info = _lookup_namespace(namespace);
  id = 0;
  if (ns_info) id = ns_info->id;
  G_UNLOCK(attribute_hash);
  return id;
}
static char *get_attribute_for_id(int attribute) {
  char *s;
  G_LOCK(attribute_hash);
  s = attributes[GET_NS(attribute)][GET_ID(attribute)];
  G_UNLOCK(attribute_hash);
  return s;
}
static guint32 lookup_attribute(const char *attribute) {
  guint32 attr_id;
  G_LOCK(attribute_hash);
  ensure_attribute_hash();
  attr_id = _lookup_attribute(attribute);
  G_UNLOCK(attribute_hash);
  return attr_id;
}
static void g_file_info_finalize(GObject *object) {
  GFileInfo *info;
  int i;
  GFileAttribute *attrs;
  info = G_FILE_INFO(object);
  attrs = (GFileAttribute*)info->attributes->data;
  for (i = 0; i < info->attributes->len; i++) _g_file_attribute_value_clear(&attrs[i].value);
  g_array_free(info->attributes, TRUE);
  if (info->mask != NO_ATTRIBUTE_MASK) g_file_attribute_matcher_unref(info->mask);
  G_OBJECT_CLASS(g_file_info_parent_class)->finalize(object);
}
static void g_file_info_class_init(GFileInfoClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_file_info_finalize;
}
static void g_file_info_init(GFileInfo *info) {
  info->mask = NO_ATTRIBUTE_MASK;
  info->attributes = g_array_new(FALSE, FALSE, sizeof(GFileAttribute));
}
GFileInfo *g_file_info_new(void) {
  return g_object_new(G_TYPE_FILE_INFO, NULL);
}
void g_file_info_copy_into(GFileInfo *src_info, GFileInfo *dest_info) {
  GFileAttribute *source, *dest;
  int i;
  g_return_if_fail(G_IS_FILE_INFO(src_info));
  g_return_if_fail(G_IS_FILE_INFO(dest_info));
  dest = (GFileAttribute*)dest_info->attributes->data;
  for (i = 0; i < dest_info->attributes->len; i++) _g_file_attribute_value_clear(&dest[i].value);
  g_array_set_size(dest_info->attributes, src_info->attributes->len);
  source = (GFileAttribute*)src_info->attributes->data;
  dest = (GFileAttribute*)dest_info->attributes->data;
  for (i = 0; i < src_info->attributes->len; i++) {
      dest[i].attribute = source[i].attribute;
      dest[i].value.type = G_FILE_ATTRIBUTE_TYPE_INVALID;
      _g_file_attribute_value_set(&dest[i].value, &source[i].value);
  }
  if (dest_info->mask != NO_ATTRIBUTE_MASK) g_file_attribute_matcher_unref(dest_info->mask);
  if (src_info->mask == NO_ATTRIBUTE_MASK) dest_info->mask = NO_ATTRIBUTE_MASK;
  else dest_info->mask = g_file_attribute_matcher_ref(src_info->mask);
}
GFileInfo *g_file_info_dup(GFileInfo *other) {
  GFileInfo *new;
  g_return_val_if_fail(G_IS_FILE_INFO(other), NULL);
  new = g_file_info_new();
  g_file_info_copy_into(other, new);
  return new;
}
void g_file_info_set_attribute_mask(GFileInfo *info, GFileAttributeMatcher *mask) {
  GFileAttribute *attr;
  int i;
  g_return_if_fail(G_IS_FILE_INFO(info));
  if (mask != info->mask) {
      if (info->mask != NO_ATTRIBUTE_MASK) g_file_attribute_matcher_unref(info->mask);
      info->mask = g_file_attribute_matcher_ref(mask);
      for (i = 0; i < info->attributes->len; i++) {
          attr = &g_array_index(info->attributes, GFileAttribute, i);
          if (!_g_file_attribute_matcher_matches_id(mask, attr->attribute)) {
              _g_file_attribute_value_clear(&attr->value);
              g_array_remove_index(info->attributes, i);
              i--;
          }
	  }
  }
}
void g_file_info_unset_attribute_mask(GFileInfo *info) {
  g_return_if_fail (G_IS_FILE_INFO (info));
  if (info->mask != NO_ATTRIBUTE_MASK) g_file_attribute_matcher_unref(info->mask);
  info->mask = NO_ATTRIBUTE_MASK;
}
void g_file_info_clear_status(GFileInfo *info) {
  GFileAttribute *attrs;
  int i;
  g_return_if_fail(G_IS_FILE_INFO(info));
  attrs = (GFileAttribute*)info->attributes->data;
  for (i = 0; i < info->attributes->len; i++) attrs[i].value.status = G_FILE_ATTRIBUTE_STATUS_UNSET;
}
static int g_file_info_find_place(GFileInfo *info, guint32 attribute) {
  int min, max, med;
  GFileAttribute *attrs;
  min = 0;
  max = info->attributes->len;
  attrs = (GFileAttribute*)info->attributes->data;
  while(min < max) {
      med = min + (max - min) / 2;
      if (attrs[med].attribute == attribute) {
          min = med;
          break;
	  } else if (attrs[med].attribute < attribute) min = med + 1;
      else max = med;
  }
  return min;
}
static GFileAttributeValue *g_file_info_find_value(GFileInfo *info, guint32 attr_id) {
  GFileAttribute *attrs;
  int i;
  i = g_file_info_find_place(info, attr_id);
  attrs = (GFileAttribute*)info->attributes->data;
  if (i < info->attributes->len && attrs[i].attribute == attr_id) return &attrs[i].value;
  return NULL;
}
static GFileAttributeValue *g_file_info_find_value_by_name(GFileInfo *info, const char *attribute) {
  guint32 attr_id;
  attr_id = lookup_attribute(attribute);
  return g_file_info_find_value(info, attr_id);
}
gboolean g_file_info_has_attribute(GFileInfo *info, const char *attribute) {
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), FALSE);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', FALSE);
  value = g_file_info_find_value_by_name(info, attribute);
  return value != NULL;
}
gboolean g_file_info_has_namespace(GFileInfo *info, const char *name_space) {
  GFileAttribute *attrs;
  guint32 ns_id;
  int i;
  g_return_val_if_fail(G_IS_FILE_INFO(info), FALSE);
  g_return_val_if_fail(name_space != NULL, FALSE);
  ns_id = lookup_namespace(name_space);
  attrs = (GFileAttribute*)info->attributes->data;
  for (i = 0; i < info->attributes->len; i++) {
      if (GET_NS(attrs[i].attribute) == ns_id) return TRUE;
  }
  return FALSE;
}
char **g_file_info_list_attributes(GFileInfo *info, const char *name_space) {
  GPtrArray *names;
  GFileAttribute *attrs;
  guint32 attribute;
  guint32 ns_id = (name_space) ? lookup_namespace(name_space) : 0;
  int i;
  g_return_val_if_fail(G_IS_FILE_INFO(info), NULL);
  names = g_ptr_array_new();
  attrs = (GFileAttribute*)info->attributes->data;
  for (i = 0; i < info->attributes->len; i++) {
      attribute = attrs[i].attribute;
      if (ns_id == 0 || GET_NS(attribute) == ns_id) g_ptr_array_add(names, g_strdup(get_attribute_for_id(attribute)));
  }
  g_ptr_array_add(names, NULL);
  return (char**)g_ptr_array_free(names, FALSE);
}
GFileAttributeType g_file_info_get_attribute_type(GFileInfo *info, const char *attribute) {
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), G_FILE_ATTRIBUTE_TYPE_INVALID);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', G_FILE_ATTRIBUTE_TYPE_INVALID);
  value = g_file_info_find_value_by_name(info, attribute);
  if (value) return value->type;
  else return G_FILE_ATTRIBUTE_TYPE_INVALID;
}
void g_file_info_remove_attribute(GFileInfo *info, const char *attribute) {
  guint32 attr_id;
  GFileAttribute *attrs;
  int i = 0;
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(attribute != NULL && *attribute != '\0');
  attr_id = lookup_attribute (attribute);
  i = g_file_info_find_place(info, attr_id);
  attrs = (GFileAttribute*)info->attributes->data;
  if (i < info->attributes->len && attrs[i].attribute == attr_id) {
      _g_file_attribute_value_clear(&attrs[i].value);
      g_array_remove_index(info->attributes, i);
  }
}
gboolean g_file_info_get_attribute_data(GFileInfo *info, const char *attribute, GFileAttributeType *type, gpointer *value_pp, GFileAttributeStatus *status) {
  GFileAttributeValue *value;
  value = g_file_info_find_value_by_name(info, attribute);
  if (value == NULL) return FALSE;
  if (status) *status = value->status;
  if (type) *type = value->type;
  if (value_pp) *value_pp = _g_file_attribute_value_peek_as_pointer(value);
  return TRUE;
}
GFileAttributeStatus g_file_info_get_attribute_status(GFileInfo *info, const char *attribute) {
  GFileAttributeValue *val;
  g_return_val_if_fail(G_IS_FILE_INFO(info), 0);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', 0);
  val = g_file_info_find_value_by_name(info, attribute);
  if (val) return val->status;
  return G_FILE_ATTRIBUTE_STATUS_UNSET;
}
gboolean g_file_info_set_attribute_status(GFileInfo *info, const char *attribute, GFileAttributeStatus status) {
  GFileAttributeValue *val;
  g_return_val_if_fail(G_IS_FILE_INFO(info), FALSE);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', FALSE);
  val = g_file_info_find_value_by_name(info, attribute);
  if (val) {
      val->status = status;
      return TRUE;
  }
  return FALSE;
}
GFileAttributeValue *_g_file_info_get_attribute_value(GFileInfo *info, const char *attribute) {
  g_return_val_if_fail(G_IS_FILE_INFO(info), NULL);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', NULL);
  return g_file_info_find_value_by_name(info, attribute);
}
char *g_file_info_get_attribute_as_string(GFileInfo *info, const char *attribute) {
  GFileAttributeValue *val;
  val = _g_file_info_get_attribute_value(info, attribute);
  if (val) return _g_file_attribute_value_as_string(val);
  return NULL;
}
GObject *g_file_info_get_attribute_object(GFileInfo *info, const char *attribute) {
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), NULL);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', NULL);
  value = g_file_info_find_value_by_name(info, attribute);
  return _g_file_attribute_value_get_object(value);
}
const char *g_file_info_get_attribute_string(GFileInfo *info, const char *attribute) {
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), NULL);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', NULL);
  value = g_file_info_find_value_by_name(info, attribute);
  return _g_file_attribute_value_get_string(value);
}
const char *g_file_info_get_attribute_byte_string(GFileInfo *info, const char *attribute) {
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), NULL);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', NULL);
  value = g_file_info_find_value_by_name(info, attribute);
  return _g_file_attribute_value_get_byte_string(value);
}
char **g_file_info_get_attribute_stringv(GFileInfo *info, const char *attribute) {
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), NULL);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', NULL);
  value = g_file_info_find_value_by_name(info, attribute);
  return _g_file_attribute_value_get_stringv(value);
}
gboolean g_file_info_get_attribute_boolean(GFileInfo *info, const char *attribute) {
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), FALSE);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', FALSE);
  value = g_file_info_find_value_by_name(info, attribute);
  return _g_file_attribute_value_get_boolean(value);
}
guint32 g_file_info_get_attribute_uint32(GFileInfo *info, const char *attribute) {
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), 0);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', 0);
  value = g_file_info_find_value_by_name(info, attribute);
  return _g_file_attribute_value_get_uint32(value);
}
gint32 g_file_info_get_attribute_int32(GFileInfo *info, const char *attribute) {
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), 0);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', 0);
  value = g_file_info_find_value_by_name(info, attribute);
  return _g_file_attribute_value_get_int32(value);
}
guint64 g_file_info_get_attribute_uint64(GFileInfo *info, const char *attribute) {
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), 0);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', 0);
  value = g_file_info_find_value_by_name(info, attribute);
  return _g_file_attribute_value_get_uint64(value);
}
gint64 g_file_info_get_attribute_int64(GFileInfo *info, const char *attribute) {
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), 0);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', 0);
  value = g_file_info_find_value_by_name(info, attribute);
  return _g_file_attribute_value_get_int64(value);
}
static GFileAttributeValue *g_file_info_create_value(GFileInfo *info, guint32 attr_id) {
  GFileAttribute *attrs;
  int i = 0;
  if (info->mask != NO_ATTRIBUTE_MASK && !_g_file_attribute_matcher_matches_id(info->mask, attr_id)) return NULL;
  i = g_file_info_find_place(info, attr_id);
  attrs = (GFileAttribute*)info->attributes->data;
  if (i < info->attributes->len && attrs[i].attribute == attr_id) return &attrs[i].value;
  else {
      GFileAttribute attr = { 0 };
      attr.attribute = attr_id;
      g_array_insert_val(info->attributes, i, attr);
      attrs = (GFileAttribute*)info->attributes->data;
      return &attrs[i].value;
  }
}
void _g_file_info_set_attribute_by_id(GFileInfo *info, guint32 attribute, GFileAttributeType type, gpointer value_p) {
  GFileAttributeValue *value;
  value = g_file_info_create_value(info, attribute);
  if (value) _g_file_attribute_value_set_from_pointer(value, type, value_p, TRUE);
}
void g_file_info_set_attribute(GFileInfo *info, const char *attribute, GFileAttributeType type, gpointer value_p) {
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(attribute != NULL && *attribute != '\0');
  _g_file_info_set_attribute_by_id(info, lookup_attribute(attribute), type, value_p);
}

void _g_file_info_set_attribute_object_by_id(GFileInfo *info, guint32 attribute, GObject *attr_value) {
  GFileAttributeValue *value;
  value = g_file_info_create_value(info, attribute);
  if (value) _g_file_attribute_value_set_object(value, attr_value);
}
void g_file_info_set_attribute_object(GFileInfo *info, const char *attribute, GObject *attr_value) {
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(attribute != NULL && *attribute != '\0');
  g_return_if_fail(G_IS_OBJECT(attr_value));
  _g_file_info_set_attribute_object_by_id(info, lookup_attribute(attribute), attr_value);
}
void _g_file_info_set_attribute_stringv_by_id(GFileInfo *info, guint32 attribute, char **attr_value) {
  GFileAttributeValue *value;
  value = g_file_info_create_value(info, attribute);
  if (value) _g_file_attribute_value_set_stringv(value, attr_value);
}
void g_file_info_set_attribute_stringv(GFileInfo *info, const char *attribute, char **attr_value) {
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(attribute != NULL && *attribute != '\0');
  g_return_if_fail(attr_value != NULL);
  _g_file_info_set_attribute_stringv_by_id(info, lookup_attribute (attribute), attr_value);
}
void _g_file_info_set_attribute_string_by_id(GFileInfo *info, guint32 attribute, const char *attr_value) {
  GFileAttributeValue *value;
  value = g_file_info_create_value(info, attribute);
  if (value) _g_file_attribute_value_set_string(value, attr_value);
}
void g_file_info_set_attribute_string(GFileInfo *info, const char *attribute, const char *attr_value) {
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(attribute != NULL && *attribute != '\0');
  g_return_if_fail(attr_value != NULL);
  _g_file_info_set_attribute_string_by_id(info, lookup_attribute(attribute), attr_value);
}
void _g_file_info_set_attribute_byte_string_by_id(GFileInfo *info, guint32 attribute, const char *attr_value) {
  GFileAttributeValue *value;
  value = g_file_info_create_value(info, attribute);
  if (value) _g_file_attribute_value_set_byte_string(value, attr_value);
}
void g_file_info_set_attribute_byte_string(GFileInfo *info, const char *attribute, const char *attr_value) {
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(attribute != NULL && *attribute != '\0');
  g_return_if_fail(attr_value != NULL);
  _g_file_info_set_attribute_byte_string_by_id(info, lookup_attribute (attribute), attr_value);
}
void _g_file_info_set_attribute_boolean_by_id(GFileInfo *info, guint32 attribute, gboolean attr_value) {
  GFileAttributeValue *value;
  value = g_file_info_create_value(info, attribute);
  if (value) _g_file_attribute_value_set_boolean(value, attr_value);
}
void g_file_info_set_attribute_boolean(GFileInfo *info, const char *attribute, gboolean attr_value) {
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(attribute != NULL && *attribute != '\0');
  _g_file_info_set_attribute_boolean_by_id(info, lookup_attribute(attribute), attr_value);
}
void _g_file_info_set_attribute_uint32_by_id(GFileInfo *info, guint32 attribute, guint32 attr_value) {
  GFileAttributeValue *value;
  value = g_file_info_create_value(info, attribute);
  if (value) _g_file_attribute_value_set_uint32(value, attr_value);
}
void g_file_info_set_attribute_uint32(GFileInfo *info, const char *attribute, guint32 attr_value) {
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(attribute != NULL && *attribute != '\0');
  _g_file_info_set_attribute_uint32_by_id(info, lookup_attribute(attribute), attr_value);
}
void _g_file_info_set_attribute_int32_by_id(GFileInfo *info, guint32 attribute, gint32 attr_value) {
  GFileAttributeValue *value;
  value = g_file_info_create_value(info, attribute);
  if (value) _g_file_attribute_value_set_int32(value, attr_value);
}
void g_file_info_set_attribute_int32(GFileInfo *info, const char *attribute, gint32 attr_value) {
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(attribute != NULL && *attribute != '\0');
  _g_file_info_set_attribute_int32_by_id(info, lookup_attribute(attribute), attr_value);
}
void _g_file_info_set_attribute_uint64_by_id(GFileInfo *info, guint32 attribute, guint64 attr_value) {
  GFileAttributeValue *value;
  value = g_file_info_create_value(info, attribute);
  if (value) _g_file_attribute_value_set_uint64(value, attr_value);
}
void g_file_info_set_attribute_uint64(GFileInfo *info, const char *attribute, guint64 attr_value) {
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(attribute != NULL && *attribute != '\0');
  _g_file_info_set_attribute_uint64_by_id(info, lookup_attribute(attribute), attr_value);
}
void _g_file_info_set_attribute_int64_by_id(GFileInfo *info, guint32 attribute, gint64 attr_value) {
  GFileAttributeValue *value;
  value = g_file_info_create_value(info, attribute);
  if (value) _g_file_attribute_value_set_int64(value, attr_value);
}
void g_file_info_set_attribute_int64(GFileInfo  *info, const char *attribute, gint64 attr_value) {
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(attribute != NULL && *attribute != '\0');
  _g_file_info_set_attribute_int64_by_id(info, lookup_attribute(attribute), attr_value);
}
GFileType g_file_info_get_file_type(GFileInfo *info) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), G_FILE_TYPE_UNKNOWN);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_TYPE);
  value = g_file_info_find_value(info, attr);
  return (GFileType)_g_file_attribute_value_get_uint32(value);
}
gboolean g_file_info_get_is_hidden(GFileInfo *info) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), FALSE);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN);
  value = g_file_info_find_value(info, attr);
  return (GFileType)_g_file_attribute_value_get_boolean(value);
}
gboolean g_file_info_get_is_backup(GFileInfo *info) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), FALSE);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_IS_BACKUP);
  value = g_file_info_find_value(info, attr);
  return (GFileType)_g_file_attribute_value_get_boolean(value);
}
gboolean g_file_info_get_is_symlink(GFileInfo *info) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), FALSE);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK);
  value = g_file_info_find_value(info, attr);
  return (GFileType)_g_file_attribute_value_get_boolean(value);
}
const char *g_file_info_get_name(GFileInfo *info) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), NULL);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_NAME);
  value = g_file_info_find_value(info, attr);
  return _g_file_attribute_value_get_byte_string(value);
}
const char *g_file_info_get_display_name(GFileInfo *info) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), NULL);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
  value = g_file_info_find_value(info, attr);
  return _g_file_attribute_value_get_string(value);
}
const char *g_file_info_get_edit_name(GFileInfo *info) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), NULL);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_EDIT_NAME);
  value = g_file_info_find_value(info, attr);
  return _g_file_attribute_value_get_string(value);
}
GIcon *g_file_info_get_icon(GFileInfo *info) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  GObject *obj;
  g_return_val_if_fail(G_IS_FILE_INFO(info), NULL);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_ICON);
  value = g_file_info_find_value(info, attr);
  obj = _g_file_attribute_value_get_object(value);
  if (G_IS_ICON(obj)) return G_ICON(obj);
  return NULL;
}
const char *g_file_info_get_content_type(GFileInfo *info) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), NULL);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
  value = g_file_info_find_value(info, attr);
  return _g_file_attribute_value_get_string(value);
}
goffset g_file_info_get_size(GFileInfo *info) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), (goffset)0);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_SIZE);
  value = g_file_info_find_value(info, attr);
  return (goffset)_g_file_attribute_value_get_uint64(value);
}
void g_file_info_get_modification_time(GFileInfo *info, GTimeVal *result) {
  static guint32 attr_mtime = 0, attr_mtime_usec;
  GFileAttributeValue *value;
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(result != NULL);
  if (attr_mtime == 0) {
      attr_mtime = lookup_attribute(G_FILE_ATTRIBUTE_TIME_MODIFIED);
      attr_mtime_usec = lookup_attribute(G_FILE_ATTRIBUTE_TIME_MODIFIED_USEC);
  }
  value = g_file_info_find_value(info, attr_mtime);
  result->tv_sec = _g_file_attribute_value_get_uint64(value);
  value = g_file_info_find_value(info, attr_mtime_usec);
  result->tv_usec = _g_file_attribute_value_get_uint32(value);
}
const char *g_file_info_get_symlink_target(GFileInfo *info) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), NULL);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET);
  value = g_file_info_find_value(info, attr);
  return _g_file_attribute_value_get_byte_string(value);
}
const char *g_file_info_get_etag(GFileInfo *info) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), NULL);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_ETAG_VALUE);
  value = g_file_info_find_value(info, attr);
  return _g_file_attribute_value_get_string(value);
}
gint32 g_file_info_get_sort_order(GFileInfo *info) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_val_if_fail(G_IS_FILE_INFO(info), 0);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_SORT_ORDER);
  value = g_file_info_find_value(info, attr);
  return _g_file_attribute_value_get_int32(value);
}
void g_file_info_set_file_type(GFileInfo *info, GFileType type) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_if_fail(G_IS_FILE_INFO(info));
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_TYPE);
  value = g_file_info_create_value(info, attr);
  if (value) _g_file_attribute_value_set_uint32(value, type);
}
void g_file_info_set_is_hidden(GFileInfo *info, gboolean is_hidden) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_if_fail(G_IS_FILE_INFO(info));
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN);
  value = g_file_info_create_value(info, attr);
  if (value) _g_file_attribute_value_set_boolean(value, is_hidden);
}
void g_file_info_set_is_symlink(GFileInfo *info, gboolean is_symlink) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_if_fail(G_IS_FILE_INFO(info));
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK);
  value = g_file_info_create_value(info, attr);
  if (value) _g_file_attribute_value_set_boolean(value, is_symlink);
}
void g_file_info_set_name(GFileInfo *info, const char *name) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(name != NULL);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_NAME);
  value = g_file_info_create_value(info, attr);
  if (value) _g_file_attribute_value_set_byte_string(value, name);
}
void g_file_info_set_display_name(GFileInfo *info, const char *display_name) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(display_name != NULL);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
  value = g_file_info_create_value(info, attr);
  if (value) _g_file_attribute_value_set_string(value, display_name);
}
void g_file_info_set_edit_name(GFileInfo *info, const char *edit_name) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(edit_name != NULL);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_EDIT_NAME);
  value = g_file_info_create_value(info, attr);
  if (value) _g_file_attribute_value_set_string(value, edit_name);
}
void g_file_info_set_icon(GFileInfo *info, GIcon *icon) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(G_IS_ICON(icon));
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_ICON);
  value = g_file_info_create_value(info, attr);
  if (value) _g_file_attribute_value_set_object(value, G_OBJECT(icon));
}
void g_file_info_set_content_type(GFileInfo *info, const char *content_type) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(content_type != NULL);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
  value = g_file_info_create_value(info, attr);
  if (value) _g_file_attribute_value_set_string(value, content_type);
}
void g_file_info_set_size(GFileInfo *info, goffset size) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_if_fail(G_IS_FILE_INFO(info));
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_SIZE);
  value = g_file_info_create_value(info, attr);
  if (value) _g_file_attribute_value_set_uint64(value, size);
}
void g_file_info_set_modification_time(GFileInfo *info, GTimeVal *mtime) {
  static guint32 attr_mtime = 0, attr_mtime_usec;
  GFileAttributeValue *value;
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(mtime != NULL);
  if (attr_mtime == 0) {
      attr_mtime = lookup_attribute(G_FILE_ATTRIBUTE_TIME_MODIFIED);
      attr_mtime_usec = lookup_attribute(G_FILE_ATTRIBUTE_TIME_MODIFIED_USEC);
  }
  value = g_file_info_create_value(info, attr_mtime);
  if (value) _g_file_attribute_value_set_uint64(value, mtime->tv_sec);
  value = g_file_info_create_value(info, attr_mtime_usec);
  if (value) _g_file_attribute_value_set_uint32(value, mtime->tv_usec);
}
void g_file_info_set_symlink_target(GFileInfo *info, const char *symlink_target) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_if_fail(G_IS_FILE_INFO(info));
  g_return_if_fail(symlink_target != NULL);
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET);
  value = g_file_info_create_value(info, attr);
  if (value) _g_file_attribute_value_set_byte_string(value, symlink_target);
}
void g_file_info_set_sort_order(GFileInfo *info, gint32 sort_order) {
  static guint32 attr = 0;
  GFileAttributeValue *value;
  g_return_if_fail(G_IS_FILE_INFO(info));
  if (attr == 0) attr = lookup_attribute(G_FILE_ATTRIBUTE_STANDARD_SORT_ORDER);
  value = g_file_info_create_value(info, attr);
  if (value) _g_file_attribute_value_set_int32(value, sort_order);
}
#define ON_STACK_MATCHERS 5
typedef struct {
  guint32 id;
  guint32 mask;
} SubMatcher;
struct _GFileAttributeMatcher {
  gboolean all;
  SubMatcher sub_matchers[ON_STACK_MATCHERS];
  GArray *more_sub_matchers;
  guint32 iterator_ns;
  int iterator_pos;
  int ref;
};
static void matcher_add(GFileAttributeMatcher *matcher, guint id, guint mask) {
  SubMatcher *sub_matchers;
  int i;
  SubMatcher s;
  for (i = 0; i < ON_STACK_MATCHERS; i++) {
      if (matcher->sub_matchers[i].id == 0) {
          matcher->sub_matchers[i].id = id;
          matcher->sub_matchers[i].mask = mask;
          return;
	  }
      if (matcher->sub_matchers[i].id == id && matcher->sub_matchers[i].mask == mask) return;
  }
  if (matcher->more_sub_matchers == NULL) matcher->more_sub_matchers = g_array_new(FALSE, FALSE, sizeof(SubMatcher));
  sub_matchers = (SubMatcher *)matcher->more_sub_matchers->data;
  for (i = 0; i < matcher->more_sub_matchers->len; i++) {
      if (sub_matchers[i].id == id && sub_matchers[i].mask == mask) return;
  }
  s.id = id;
  s.mask = mask;
  g_array_append_val(matcher->more_sub_matchers, s);
}
G_DEFINE_BOXED_TYPE(GFileAttributeMatcher, g_file_attribute_matcher, g_file_attribute_matcher_ref, g_file_attribute_matcher_unref)
GFileAttributeMatcher *g_file_attribute_matcher_new(const char *attributes) {
  char **split;
  char *colon;
  int i;
  GFileAttributeMatcher *matcher;
  if (attributes == NULL || *attributes == '\0') return NULL;
  matcher = g_malloc0(sizeof(GFileAttributeMatcher));
  matcher->ref = 1;
  split = g_strsplit(attributes, ",", -1);
  for (i = 0; split[i] != NULL; i++) {
      if (strcmp(split[i], "*") == 0) matcher->all = TRUE;
      else {
          guint32 id, mask;
          colon = strstr(split[i], "::");
          if (colon != NULL && !(colon[2] == 0 || (colon[2] == '*' && colon[3] == 0))) {
              id = lookup_attribute(split[i]);
              mask = 0xffffffff;
          } else {
              if (colon) *colon = 0;
              id = lookup_namespace(split[i]) << NS_POS;
              mask = NS_MASK << NS_POS;
          }
          matcher_add(matcher, id, mask);
	  }
  }
  g_strfreev(split);
  return matcher;
}
GFileAttributeMatcher *g_file_attribute_matcher_ref(GFileAttributeMatcher *matcher) {
  if (matcher) {
      g_return_val_if_fail(matcher->ref > 0, NULL);
      g_atomic_int_inc(&matcher->ref);
  }
  return matcher;
}
void g_file_attribute_matcher_unref(GFileAttributeMatcher *matcher) {
  if (matcher) {
      g_return_if_fail(matcher->ref > 0);
      if (g_atomic_int_dec_and_test(&matcher->ref)) {
          if (matcher->more_sub_matchers) g_array_free(matcher->more_sub_matchers, TRUE);
          g_free(matcher);
	  }
  }
}
gboolean g_file_attribute_matcher_matches_only(GFileAttributeMatcher *matcher, const char *attribute) {
  guint32 id;
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', FALSE);
  if (matcher == NULL || matcher->all) return FALSE;
  id = lookup_attribute(attribute);
  if (matcher->sub_matchers[0].id != 0 && matcher->sub_matchers[1].id == 0 && matcher->sub_matchers[0].mask == 0xffffffff && matcher->sub_matchers[0].id == id)
      return TRUE;
  return FALSE;
}
static gboolean matcher_matches_id(GFileAttributeMatcher *matcher, guint32 id) {
  SubMatcher *sub_matchers;
  int i;
  for (i = 0; i < ON_STACK_MATCHERS; i++) {
      if (matcher->sub_matchers[i].id == 0) return FALSE;
      if (matcher->sub_matchers[i].id == (id & matcher->sub_matchers[i].mask)) return TRUE;
  }
  if (matcher->more_sub_matchers) {
      sub_matchers = (SubMatcher*)matcher->more_sub_matchers->data;
      for (i = 0; i < matcher->more_sub_matchers->len; i++) {
          if (sub_matchers[i].id == (id & sub_matchers[i].mask)) return TRUE;
	  }
  }
  return FALSE;
}
gboolean _g_file_attribute_matcher_matches_id(GFileAttributeMatcher *matcher, guint32 id) {
  if (matcher == NULL) return FALSE;
  if (matcher->all) return TRUE;
  return matcher_matches_id(matcher, id);
}
gboolean g_file_attribute_matcher_matches(GFileAttributeMatcher *matcher, const char *attribute) {
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', FALSE);
  if (matcher == NULL) return FALSE;
  if (matcher->all) return TRUE;
  return matcher_matches_id(matcher, lookup_attribute(attribute));
}
gboolean g_file_attribute_matcher_enumerate_namespace(GFileAttributeMatcher *matcher, const char *ns) {
  SubMatcher *sub_matchers;
  int ns_id;
  int i;
  g_return_val_if_fail(ns != NULL && *ns != '\0', FALSE);
  if (matcher == NULL) return FALSE;
  if (matcher->all) return TRUE;
  ns_id = lookup_namespace (ns) << NS_POS;
  for (i = 0; i < ON_STACK_MATCHERS; i++) {
      if (matcher->sub_matchers[i].id == ns_id) return TRUE;
  }
  if (matcher->more_sub_matchers) {
      sub_matchers = (SubMatcher*)matcher->more_sub_matchers->data;
      for (i = 0; i < matcher->more_sub_matchers->len; i++) {
          if (sub_matchers[i].id == ns_id) return TRUE;
	  }
  }
  matcher->iterator_ns = ns_id;
  matcher->iterator_pos = 0;
  return FALSE;
}
const char *g_file_attribute_matcher_enumerate_next(GFileAttributeMatcher *matcher) {
  int i;
  SubMatcher *sub_matcher;
  if (matcher == NULL) return NULL;
  while(1) {
      i = matcher->iterator_pos++;
      if (i < ON_STACK_MATCHERS) {
          if (matcher->sub_matchers[i].id == 0) return NULL;
          sub_matcher = &matcher->sub_matchers[i];
	  } else {
          if (matcher->more_sub_matchers == NULL) return NULL;
          i -= ON_STACK_MATCHERS;
          if (i < matcher->more_sub_matchers->len) sub_matcher = &g_array_index(matcher->more_sub_matchers, SubMatcher, i);
          else return NULL;
	  }
      if (sub_matcher->mask == 0xffffffff && (sub_matcher->id & (NS_MASK << NS_POS)) == matcher->iterator_ns) return get_attribute_for_id(sub_matcher->id);
  }
}