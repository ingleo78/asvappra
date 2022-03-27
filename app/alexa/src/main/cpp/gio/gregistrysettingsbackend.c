#include <string.h>
#include <stdint.h>
#include "../glib/glib.h"
#include "config.h"
#include "gregistrysettingsbackend.h"
#include "gsimplepermission.h"
#include "gsettingsbackend.h"
#include "giomodule.h"

#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN
#define MAX_KEY_NAME_LENGTH   32
#define MAX_WATCHES  64
typedef struct {
  HANDLE event;
  HKEY hpath;
  char *prefix;
  GNode *cache_node;
} RegistryWatch;
typedef enum {
  WATCH_THREAD_NONE,
  WATCH_THREAD_ADD_WATCH,
  WATCH_THREAD_REMOVE_WATCH,
  WATCH_THREAD_STOP
} WatchThreadMessageType;
typedef struct {
  WatchThreadMessageType type;
  RegistryWatch watch;
} WatchThreadMessage;
typedef struct {
  GSettingsBackend *owner;
  HANDLE *thread;
  int watches_remaining;
  GPtrArray *events, *handles, *prefixes, *cache_nodes;
  WatchThreadMessage message;
  CRITICAL_SECTION *message_lock;
  HANDLE message_sent_event, message_received_event;
} WatchThreadState;
#define G_TYPE_REGISTRY_BACKEND  (g_registry_backend_get_type())
#define G_REGISTRY_BACKEND(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_REGISTRY_BACKEND, GRegistryBackend))
#define G_IS_REGISTRY_BACKEND(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_REGISTRY_BACKEND))
typedef GSettingsBackendClass GRegistryBackendClass;
typedef struct {
  GSettingsBackend  parent_instance;
  char *base_path;
  CRITICAL_SECTION *cache_lock;
  GNode *cache_root;
  WatchThreadState *watch;
} GRegistryBackend;
G_DEFINE_TYPE_WITH_CODE(GRegistryBackend, g_registry_backend, G_TYPE_SETTINGS_BACKEND,g_io_extension_point_implement(G_SETTINGS_BACKEND_EXTENSION_POINT_NAME,
                        g_define_type_id, "registry", 90));
#include <stdio.h>
#include "../dbus/dbus-sysdeps-wince-glue.h"

static void trace(const char *format, ...) {
  #ifdef TRACE
  va_list va; va_start(va, format);
  vprintf(format, va); fflush(stdout);
  va_end(va);
  #endif
};
static void g_message_win32_error(DWORD result_code, const gchar *format, ...) {
  /*va_list va;
  gint pos;
  gchar win32_message[1024];
  if (result_code == 0) result_code = GetLastError();
  va_start (va, format);
  pos = g_vsnprintf(win32_message, 512, format, va);
  win32_message[pos++] = ':'; win32_message[pos++] = ' ';
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, result_code, 0, (LPTSTR)(win32_message+pos), 1023 - pos, NULL);
  if (result_code == ERROR_KEY_DELETED) trace("(%s)", win32_message);
  else g_message(win32_message);*/
};
static gchar *parse_key(const gchar *key_name, const gchar *registry_prefix, gchar **value_name) {
  gchar *path_name, *c;
  if (key_name[0] == '/') key_name ++;
  if (registry_prefix == NULL) path_name = g_strdup(key_name);
  else path_name = g_strjoin ("/", registry_prefix, key_name, NULL);
  for (c = path_name + (registry_prefix ? strlen(registry_prefix):0); *c!=0; c++)
      if (*c == '/') {
          *c = '\\';
          (*value_name) = c;
      }
  **value_name = 0; (*value_name)++;
  return path_name;
};
static DWORD g_variant_get_as_dword(GVariant *variant) {
  switch(g_variant_get_type_string (variant)[0]) {
      case 'b': return g_variant_get_boolean(variant);
      case 'y': return g_variant_get_byte(variant);
      case 'n': return g_variant_get_int16(variant);
      case 'q': return g_variant_get_uint16(variant);
      case 'i': return g_variant_get_int32(variant);
      case 'u': return g_variant_get_uint32(variant);
      default:  g_warn_if_reached();
  }
  return 0;
}
static DWORDLONG g_variant_get_as_qword(GVariant *variant) {
  switch (g_variant_get_type_string(variant)[0]) {
      case 'x': return g_variant_get_int64(variant);
      case 't': return g_variant_get_uint64(variant);
      default:  g_warn_if_reached();
  }
  return 0;
}
static void handle_read_error(LONG result, const gchar *path_name, const gchar *value_name) {
  //if (result != ERROR_FILE_NOT_FOUND) g_message_win32_error(result, "Unable to query value %s/%s: %s.\n", path_name, value_name);
}
typedef struct {
  DWORD type;
  union {
      gint  dword;
      void *ptr;
  };
} RegistryValue;

static char *registry_value_dump(RegistryValue value) {
  if (value.type == REG_DWORD) return g_strdup_printf("%i", value.dword);
  else if (value.type == REG_QWORD) return g_strdup_printf("%I64i", value.ptr==NULL? 0: *(DWORDLONG *)value.ptr);
  else if (value.type == REG_SZ) return g_strdup_printf("%s", (char *)value.ptr);
  else if (value.type == REG_NONE) return g_strdup_printf("<empty>");
  else return g_strdup_printf("<invalid>");
}
static void registry_value_free(RegistryValue value) {
  if (value.type == REG_SZ || value.type == REG_QWORD) g_free(value.ptr);
  value.type = REG_NONE;
  value.ptr = NULL;
}
typedef struct {
  gchar *name;
  gint32 block_count : 8;
  gint32 subscription_count : 14;
  gint32 ref_count : 9;
  gint32 touched : 1;
  RegistryValue value;
} RegistryCacheItem;
static GNode *registry_cache_add_item(GNode *parent, gchar *name, RegistryValue value, gint ref_count) {
  RegistryCacheItem *item = g_slice_new(RegistryCacheItem);
  GNode *cache_node;
  g_return_val_if_fail(name != NULL, NULL);
  g_return_val_if_fail(parent != NULL, NULL);
  item->ref_count = ref_count;
  item->name = g_strdup(name);
  item->value = value;
  item->subscription_count = 0;
  item->block_count = 0;
  item->touched = FALSE;
  trace("\treg cache: adding %s to %s\n", name, ((RegistryCacheItem*)parent->data)->name);
  cache_node = g_node_new(item);
  g_node_append(parent, cache_node);
  return cache_node;
}
static void _ref_down(GNode *node) {
  RegistryCacheItem *item = node->data;
  g_node_children_foreach(node, G_TRAVERSE_ALL, (GNodeForeachFunc)_ref_down, NULL);
  item->ref_count ++;
}
static void registry_cache_ref_tree(GNode *tree) {
  RegistryCacheItem *item = tree->data;
  GNode *node = tree->parent;
  g_return_if_fail(tree != NULL);
  item->ref_count ++;
  g_node_children_foreach(tree, G_TRAVERSE_ALL, (GNodeForeachFunc)_ref_down, NULL);
  for (node=tree->parent; node; node=node->parent) {
      item = node->data;
      item->ref_count ++;
  }
}
static void _free_cache_item(RegistryCacheItem *item) {
  trace("\t -- Free node %s\n", item->name);
  g_free(item->name);
  registry_value_free(item->value);
  g_slice_free(RegistryCacheItem, item);
}
static void _unref_node(GNode *node) {
  RegistryCacheItem *item = node->data;
  item->ref_count --;
  g_warn_if_fail(item->ref_count >= 0);
  if (item->ref_count == 0) {
      _free_cache_item(item);
      g_node_destroy(node);
  }
}
static void _unref_down(GNode *node) {
  g_node_children_foreach(node, G_TRAVERSE_ALL, (GNodeForeachFunc)_unref_down, NULL);
  _unref_node(node);
}
static void registry_cache_unref_tree(GNode *tree) {
  GNode *parent = tree->parent, *next_parent;
  _unref_down(tree);
  while(parent) {
      next_parent = parent->parent;
      _unref_node(parent);
      parent = next_parent;
  }
}
static void registry_cache_dump(GNode *cache_node, gpointer data) {
  RegistryCacheItem *item = cache_node->data;
  int depth = GPOINTER_TO_INT(data), new_depth = depth+1, i;
  g_return_if_fail(cache_node != NULL);
  for (i=0; i<depth; i++) g_print("  ");
  if (item == NULL) g_print("*root*\n");
  else g_print("'%s'  [%i] @ %x = %s\n", item->name, item->ref_count, (guint)cache_node, registry_value_dump(item->value));
  g_node_children_foreach(cache_node, G_TRAVERSE_ALL, registry_cache_dump, GINT_TO_POINTER(new_depth));
}
typedef struct {
  gchar *name;
  GNode *result;
} RegistryCacheSearch;
static gboolean registry_cache_find_compare(GNode  *node, gpointer data) {
  RegistryCacheSearch *search = data;
  RegistryCacheItem *item = node->data;
  if (item == NULL) return FALSE;
  g_return_val_if_fail(search->name != NULL, FALSE);
  g_return_val_if_fail(item->name != NULL, FALSE);
  if (strcmp (search->name, item->name) == 0) {
      search->result = node;
      return TRUE;
  }
  return FALSE;
}
static GNode *registry_cache_find_immediate_child(GNode *node, gchar *name) {
  RegistryCacheSearch search;
  search.result = NULL;
  search.name = name;
  g_node_traverse(node, G_POST_ORDER, G_TRAVERSE_ALL, 2, registry_cache_find_compare, &search);
  return search.result;  
}
static GNode *registry_cache_get_node_for_key_recursive(GNode *node, gchar *key_name, gboolean create_if_not_found, gint n_parent_watches) {
  RegistryCacheItem *item;
  gchar *component = key_name, *c = strchr(component, '/');
  GNode *child;
  if (c != NULL) *c = 0;
  item = node->data;
  if (item->subscription_count > 0) n_parent_watches ++;
  child = registry_cache_find_immediate_child(node, component);
  if (child == NULL && create_if_not_found) {
      item = g_slice_new(RegistryCacheItem);
      item->name = g_strdup(component);
      item->value.type = REG_NONE;
      item->value.ptr = NULL;
      item->ref_count = n_parent_watches;
      child = g_node_new(item);
      g_node_append(node, child);
      trace("\tget node for key recursive: new %x = %s.\n", node, item->name);
  }
  if (child==NULL || c == NULL || *(c+1)==0) return child;
  else {
      trace("get node for key recursive: next: %s.\n", c+1);
      return registry_cache_get_node_for_key_recursive(child, c+1, create_if_not_found, n_parent_watches);
  }
}
static GNode *registry_cache_get_node_for_key(GNode *root, const gchar *key_name, gboolean create_if_not_found) {
  GNode *child = NULL, *result = NULL;
  gchar *component, *c;
  g_return_val_if_fail(key_name != NULL, NULL);
  if (key_name[0] == '/') key_name ++;
  component = g_strdup(key_name);
  c = strchr(component, '/');
  if (c != NULL) *c = 0;
  child = registry_cache_find_immediate_child(root, component);
  if (child == NULL && create_if_not_found) {
      RegistryCacheItem *item = g_slice_new(RegistryCacheItem);
      item->value.type = REG_NONE;
      item->value.ptr = NULL;
      item->name = g_strdup(component);
      item->ref_count = 0;
      trace("get_node_for_key: New node for component '%s'\n", item->name);
      child = g_node_new(item);
      g_node_append(root, child);
  }
  if (c == NULL) result = root;
  else if (*(c+1)==0) result = child;
  else if (child != NULL) result = registry_cache_get_node_for_key_recursive(child, c+1, create_if_not_found, 0);
  g_free(component);
  return result;
}
static gboolean registry_cache_update_node(GNode *cache_node, RegistryValue registry_value) {
  RegistryCacheItem *cache_item = cache_node->data;
  g_return_val_if_fail(cache_node != NULL, FALSE);
  g_return_val_if_fail(cache_item != NULL, FALSE);
  if (registry_value.type != cache_item->value.type) {
      cache_item->value = registry_value;
      return TRUE;
  }
  switch(registry_value.type) {
      case REG_DWORD: {
              if (cache_item->value.dword == registry_value.dword) return FALSE;
              else {
                  cache_item->value.dword = registry_value.dword;
                  return TRUE;
              }
          }
      case REG_QWORD: {
              g_return_val_if_fail(registry_value.ptr != NULL && cache_item->value.ptr != NULL, FALSE);
              if (memcmp(registry_value.ptr, cache_item->value.ptr, 8)==0) {
                  g_free(registry_value.ptr);
                  return FALSE;
              } else {
                  g_free(cache_item->value.ptr);
                  cache_item->value.ptr = registry_value.ptr;
                  return TRUE;
              }
          }
      case REG_SZ: {
              g_return_val_if_fail(cache_item->value.ptr != NULL, FALSE);
              g_return_val_if_fail(registry_value.ptr != NULL, FALSE);
              if (strcmp(registry_value.ptr, cache_item->value.ptr) == 0) {
                  g_free(registry_value.ptr);
                  return FALSE;
              } else {
                  g_free(cache_item->value.ptr);
                  cache_item->value.ptr = registry_value.ptr;
                  return TRUE;
              }
          }
      default:
          g_warning("gregistrybackend: registry_cache_update_node: Unhandled value type :(");
          return FALSE;
  }
}
static void registry_cache_block_notification(GNode *node) {
  RegistryCacheItem *item = node->data;
  g_return_if_fail(node != NULL);
  if (item->subscription_count > 0) item->block_count ++;
  if (node->parent != NULL) registry_cache_block_notification(node->parent);
}
static void registry_cache_destroy_tree(GNode *node, WatchThreadState *self);
static gboolean registry_read(HKEY hpath, const gchar *path_name, const gchar *value_name, RegistryValue *p_value) {
  LONG result;
  DWORD value_data_size;
  gpointer *buffer;
  g_return_val_if_fail(p_value != NULL, FALSE);
  p_value->type = REG_NONE;
  p_value->ptr = NULL;
  result = RegQueryValueExA(hpath, value_name, 0, &p_value->type, NULL, &value_data_size);
  if (result != ERROR_SUCCESS) {
      handle_read_error(result, path_name, value_name);
      return FALSE;
  }
  if (p_value->type == REG_SZ && value_data_size == 0) {
      p_value->ptr = g_strdup("");
      return TRUE;
  }
  if (p_value->type == REG_DWORD) buffer = (void*)&p_value->dword;
  else buffer = p_value->ptr = g_malloc (value_data_size);
  result = RegQueryValueExA(hpath, value_name, 0, NULL, (LPBYTE)buffer, &value_data_size);
  if (result != ERROR_SUCCESS) {
      handle_read_error(result, path_name, value_name);
      return FALSE;
  }
}
static GVariant *g_registry_backend_read (GSettingsBackend *backend, const gchar *key_name, const GVariantType *expected_type, gboolean default_value) {
  GRegistryBackend *self = G_REGISTRY_BACKEND(backend);
  GNode *cache_node;
  RegistryValue registry_value;
  GVariant *gsettings_value = NULL;
  gchar *gsettings_type;
  g_return_val_if_fail(expected_type != NULL, NULL);
  if (default_value) return NULL;
  //EnterCriticalSection(self->cache_lock);
  cache_node = registry_cache_get_node_for_key(self->cache_root, key_name, FALSE);
  //LeaveCriticalSection(self->cache_lock);
  trace("Reading key %s, cache node %x\n", key_name, cache_node);
  if (cache_node == NULL) return NULL;
  trace("\t- cached value %s\n", registry_value_dump(((RegistryCacheItem *)cache_node->data)->value));
  registry_value = ((RegistryCacheItem*)cache_node->data)->value;
  gsettings_type = g_variant_type_dup_string(expected_type);
  switch(gsettings_type[0]) {
      case 'b': case 'y': case 'n': case 'q': case 'i': case 'u':
          if (registry_value.type == REG_DWORD) gsettings_value = g_variant_new(gsettings_type, registry_value.dword);
          break;
      case 't': case 'x':
          if (registry_value.type == REG_QWORD) {
              DWORDLONG qword_value = *(DWORDLONG*)registry_value.ptr;
              gsettings_value = g_variant_new (gsettings_type, qword_value);
          }
          break;
      default:
          if (registry_value.type == REG_SZ) {
              if (gsettings_type[0]=='s') gsettings_value = g_variant_new_string ((char *)registry_value.ptr);
              else {
                  GError *error = NULL;
                  gsettings_value = g_variant_parse (expected_type, registry_value.ptr, NULL, NULL, &error);
                  if (error != NULL)
                  g_message ("gregistrysettingsbackend: error parsing key %s: %s\n", key_name, error->message);
              }
          }
          break;
  }
  g_free(gsettings_type);
  return gsettings_value;
}
typedef struct {
  GRegistryBackend *self;
  HKEY hroot;
} RegistryWrite;
static gboolean g_registry_backend_write_one(const char *key_name, GVariant *variant, gpointer user_data) {
  /*GRegistryBackend *self;
  RegistryWrite *action;
  RegistryValue value;
  HKEY hroot, hpath;
  gchar *path_name, *value_name = NULL;
  DWORD value_data_size;
  LPVOID value_data;
  LONG result;
  GNode *node;
  gboolean changed;
  const gchar *type_string = g_variant_get_type_string(variant);
  action = user_data;
  self = G_REGISTRY_BACKEND(action->self);
  hroot = action->hroot;
  value.type = REG_NONE;
  value.ptr = NULL;
  switch(type_string[0]) {
      case 'b': case 'y': case 'n': case 'q': case 'i': case 'u':
          value.type = REG_DWORD;
          value.dword = g_variant_get_as_dword(variant);
          value_data_size = 4;
          value_data = &value.dword;
          break;
      case 'x': case 't':
          value.type = REG_QWORD;
          value.ptr = g_malloc (8);
          *(DWORDLONG*)value.ptr = g_variant_get_as_qword(variant);
          value_data_size = 8;
          value_data = value.ptr;
          break;
      default:
          value.type = REG_SZ;
          if (type_string[0]=='s') {
              gsize length;
              value.ptr = g_strdup(g_variant_get_string(variant, &length));
              value_data_size = length + 1;
              value_data = value.ptr;
          } else {
              GString *value_string;
              value_string = g_variant_print_string(variant, NULL, FALSE);
              value_data_size = value_string->len+1;
              value.ptr = value_data = g_string_free(value_string, FALSE);
          }
          break;
  }
  EnterCriticalSection(self->cache_lock);
  node = registry_cache_get_node_for_key(self->cache_root, key_name, TRUE);
  changed = registry_cache_update_node(node, value);
  LeaveCriticalSection(self->cache_lock);
  if (!changed) return FALSE;
  registry_cache_block_notification(node);
  path_name = parse_key(key_name, NULL, &value_name);
  trace("Set key: %s / %s\n", path_name, value_name);
  result = RegCreateKeyExA(hroot, path_name, 0, NULL, 0, KEY_WRITE, NULL, &hpath, NULL);
  if (result != ERROR_SUCCESS) {
      g_message_win32_error(result, "gregistrybackend: opening key %s failed", path_name+1);
      registry_value_free(value);
      g_free(path_name);
      return FALSE;
  }
  result = RegSetValueExA(hpath, value_name, 0, value.type, value_data, value_data_size);
  if (result != ERROR_SUCCESS) g_message_win32_error(result, "gregistrybackend: setting value %s\\%s\\%s failed.\n", self->base_path, path_name, value_name);
  RegCloseKey(hpath);
  g_free(path_name);*/
  return FALSE;
};
static gboolean g_registry_backend_write(GSettingsBackend *backend, const gchar *key_name, GVariant *value, gpointer origin_tag) {
  /*GRegistryBackend *self = G_REGISTRY_BACKEND(backend);
  LONG result;
  HKEY hroot;
  RegistryWrite action;
  result = RegCreateKeyExA(HKEY_CURRENT_USER, self->base_path, 0, NULL, 0, KEY_WRITE, NULL, &hroot, NULL);
  if (result != ERROR_SUCCESS) {
    trace("Error opening/creating key %s.\n", self->base_path);
    return FALSE;
  }
  action.self = self;
  action.hroot = hroot;
  g_registry_backend_write_one(key_name, value, &action);
  g_settings_backend_changed(backend, key_name, origin_tag);
  RegCloseKey(hroot);*/
  return TRUE;
}
static gboolean g_registry_backend_write_tree(GSettingsBackend *backend, GTree *values, gpointer origin_tag) {
  /*GRegistryBackend *self = G_REGISTRY_BACKEND(backend);
  LONG result;
  HKEY hroot;
  RegistryWrite action;
  result = RegCreateKeyExA(HKEY_CURRENT_USER, self->base_path, 0, NULL, 0, KEY_WRITE, NULL, &hroot, NULL);
  if (result != ERROR_SUCCESS) {
    trace("Error opening/creating key %s.\n", self->base_path);
    return FALSE;
  }
  action.self =  self;
  action.hroot = hroot;
  g_tree_foreach(values, (GTraverseFunc)g_registry_backend_write_one, &action);
  g_settings_backend_changed_tree(backend, values, origin_tag);
  RegCloseKey(hroot);*/
  return TRUE;
}
static void g_registry_backend_reset(GSettingsBackend *backend, const gchar *key_name, gpointer origin_tag) {
  /*GRegistryBackend *self = G_REGISTRY_BACKEND (backend);
  gchar *path_name, *value_name = NULL;
  GNode *cache_node;
  LONG result;
  HKEY hpath;
  EnterCriticalSection(self->cache_lock);
  cache_node = registry_cache_get_node_for_key(self->cache_root, key_name, FALSE);
  if (cache_node) registry_cache_destroy_tree(cache_node, self->watch);
  LeaveCriticalSection(self->cache_lock);
  path_name = parse_key(key_name, self->base_path, &value_name);
  result = RegOpenKeyExA(HKEY_CURRENT_USER, path_name, 0, KEY_SET_VALUE, &hpath);
  if (result != ERROR_SUCCESS) {
      g_message_win32_error(result, "Registry: resetting key '%s'", path_name);
      g_free(path_name);
      return;
  }
  result = RegDeleteValueA(hpath, value_name);
  RegCloseKey(hpath);
  if (result != ERROR_SUCCESS) {
      g_message_win32_error(result, "Registry: resetting key '%s'", path_name);
      g_free(path_name);
      return;
  }
  g_free(path_name);
  g_settings_backend_changed(backend, key_name, origin_tag);*/
}
static gboolean g_registry_backend_get_writable(GSettingsBackend *backend, const gchar *key_name) {
  return TRUE;
}
static GPermission *g_registry_backend_get_permission(GSettingsBackend *backend, const gchar *key_name) {
  return g_simple_permission_new(TRUE);
}
static void _free_watch(WatchThreadState *self, gint index, GNode *cache_node);
static void registry_cache_item_reset_touched(GNode *node, gpointer data) {
  RegistryCacheItem *item = node->data;
  item->touched = FALSE;
}
static void registry_cache_destroy_tree(GNode *node, WatchThreadState *self) {
  RegistryCacheItem *item = node->data;
  g_node_children_foreach(node, G_TRAVERSE_ALL, (GNodeForeachFunc)registry_cache_destroy_tree, self);
  if (item->subscription_count > 0) {
	  gint i;
      g_warn_if_fail(self->cache_nodes->len > 1);
      for (i=1; i<self->cache_nodes->len; i++)
        if (g_ptr_array_index(self->cache_nodes, i) == node) break;
      if (i >= self->cache_nodes->len) {
          g_warning("watch thread: a watch point was deleted, but unable to find '%s' in the list of %i watch nodes\n", item->name, self->cache_nodes->len-1);
      } else {
          _free_watch(self, i, node);
          g_atomic_int_inc(&self->watches_remaining);
      }
  }
  _free_cache_item(node->data);
  g_node_destroy(node);
}
static void registry_cache_remove_deleted(GNode *node, gpointer data) {
  RegistryCacheItem *item = node->data;
  if (!item->touched) registry_cache_destroy_tree(node, data);
}
static void registry_cache_update(GRegistryBackend *self, HKEY hpath, const gchar *prefix, const gchar *partial_key_name, GNode *cache_node, int n_watches,
                                  GPtrArray *changes) {
  /*gchar buffer[MAX_KEY_NAME_LENGTH + 1];
  gchar *key_name;
  gint i;
  LONG result;
  RegistryCacheItem *item = cache_node->data;
  if (item->subscription_count > 0) n_watches ++;
  key_name = g_build_path("/", prefix, partial_key_name, NULL);
  trace("registry cache update: %s. Node %x has %i children\n", key_name, cache_node, g_node_n_children(cache_node));
  g_node_children_foreach(cache_node, G_TRAVERSE_ALL, registry_cache_item_reset_touched, NULL);
  i = 0;
  while(1) {
      DWORD buffer_size = MAX_KEY_NAME_LENGTH;
      HKEY hsubpath;
      result = RegEnumKeyEx(hpath, i++, buffer, &buffer_size, NULL, NULL, NULL, NULL);
      if (result != ERROR_SUCCESS) break;
      result = RegOpenKeyEx(hpath, buffer, 0, KEY_READ, &hsubpath);
      if (result == ERROR_SUCCESS) {
          GNode *subkey_node;
          RegistryCacheItem *child_item;bueno lo copio
          subkey_node = registry_cache_find_immediate_child(cache_node, buffer);
          if (subkey_node == NULL) {
              RegistryValue null_value = {REG_NONE, {0}};
              subkey_node = registry_cache_add_item(cache_node, buffer, null_value, n_watches);
          }
          registry_cache_update(self, hsubpath, prefix, buffer, subkey_node, n_watches, changes);
          child_item = subkey_node->data;
          child_item->touched = TRUE;
      }
      RegCloseKey(hsubpath);
  }
  if (result != ERROR_NO_MORE_ITEMS) g_message_win32_error(result, "gregistrybackend: error enumerating subkeys for cache.");
  i = 0;
  while(1) {
      DWORD buffer_size = MAX_KEY_NAME_LENGTH;
      GNode *cache_child_node;
      RegistryCacheItem *child_item;
      RegistryValue value;
      gboolean changed = FALSE;
      result = RegEnumValue(hpath, i++, buffer, &buffer_size, NULL, NULL, NULL, NULL);
      if (result != ERROR_SUCCESS) break;
      if (buffer[0]==0) continue;
      cache_child_node = registry_cache_find_immediate_child(cache_node, buffer);
      if (!registry_read(hpath, key_name, buffer, &value)) continue;
      trace("\tgot value %s for %s, node %x\n", registry_value_dump(value), buffer, cache_child_node);
      if (cache_child_node == NULL) {
          cache_child_node = registry_cache_add_item(cache_node, buffer, value, n_watches);
          changed = TRUE;
      } else changed = registry_cache_update_node(cache_child_node, value);
      child_item = cache_child_node->data;
      child_item->touched = TRUE;
      if (changed == TRUE && changes != NULL) {
          gchar *item;
          if (partial_key_name == NULL) item = g_strdup(buffer);
          else item = g_build_path("/", partial_key_name, buffer, NULL);
          g_ptr_array_add(changes, item);
      }
  }
  if (result != ERROR_NO_MORE_ITEMS) g_message_win32_error(result, "gregistrybackend: error enumerating values for cache");
  g_node_children_foreach(cache_node, G_TRAVERSE_ALL, registry_cache_remove_deleted, self->watch);
  trace("registry cache update complete.\n");
  g_free(key_name);*/
};
static DWORD registry_watch_key(HKEY hpath, HANDLE event) {
  //return RegNotifyChangeKeyValue(hpath, TRUE, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET, event, TRUE);
  return 0;
}
typedef struct {
  GRegistryBackend *self;
  gchar *prefix;
  GPtrArray *items;
} RegistryEvent;
static gboolean watch_handler(RegistryEvent *event) {
  gint i;
  trace("Watch handler: got event in %s, items %i.\n", event->prefix, event->items->len);
  g_ptr_array_add(event->items, NULL);
  g_settings_backend_keys_changed(G_SETTINGS_BACKEND(event->self), event->prefix, (gchar const**)event->items->pdata, NULL);
  for (i=0; i<event->items->len; i++) g_free(g_ptr_array_index(event->items, i));
  g_ptr_array_free(event->items, TRUE);
  g_free(event->prefix);
  g_object_unref(event->self);
  g_slice_free(RegistryEvent, event);
  return FALSE;
};
static void _free_watch(WatchThreadState *self, gint index, GNode *cache_node) {
  /*HKEY hpath;
  HANDLE cond;
  gchar *prefix;
  g_return_if_fail(index > 0 && index < self->events->len);
  cond = g_ptr_array_index(self->events, index);
  hpath = g_ptr_array_index(self->handles, index);
  prefix = g_ptr_array_index(self->prefixes, index);
  trace("Freeing watch %i [%s]\n", index, prefix);
  if (hpath != NULL) RegCloseKey(hpath);
  if (cache_node != NULL) registry_cache_unref_tree(cache_node);
  CloseHandle(cond);
  g_free(prefix);
  g_ptr_array_remove_index_fast(self->handles, index);
  g_ptr_array_remove_index_fast(self->events, index);
  g_ptr_array_remove_index_fast(self->prefixes, index);
  g_ptr_array_remove_index_fast(self->cache_nodes, index);*/
}
static void watch_thread_handle_message(WatchThreadState *self) {
  switch(self->message.type) {
      case WATCH_THREAD_NONE: trace("watch thread: you woke me up for nothin', man!"); break;
      case WATCH_THREAD_ADD_WATCH: {
              RegistryWatch *watch = &self->message.watch;
              LONG result;
              result = registry_watch_key(watch->hpath, watch->event);
              /*if (result == ERROR_SUCCESS) {
                  g_ptr_array_add(self->events, watch->event);
                  g_ptr_array_add(self->handles, watch->hpath);
                  g_ptr_array_add(self->prefixes, watch->prefix);
                  g_ptr_array_add(self->cache_nodes, watch->cache_node);
                  trace("watch thread: new watch on %s, %i total\n", watch->prefix, self->events->len);
              } else {
                  g_message_win32_error(result, "watch thread: could not watch %s", watch->prefix);
                  CloseHandle(watch->event);
                  RegCloseKey(watch->hpath);
                  g_free(watch->prefix);
                  registry_cache_unref_tree(watch->cache_node);
              }*/
              break;
          }
      case WATCH_THREAD_REMOVE_WATCH: {
              GNode *cache_node;
              RegistryCacheItem *cache_item;
              gint i;
              for (i=1; i<self->prefixes->len; i++)
                  if (strcmp (g_ptr_array_index(self->prefixes, i), self->message.watch.prefix) == 0) break;
              if (i >= self->prefixes->len) {
                  trace("unsubscribe: prefix %s is not being watched [%i things are]!\n", self->message.watch.prefix, self->prefixes->len);
                  g_free(self->message.watch.prefix);
                  break;
              }
              cache_node = g_ptr_array_index(self->cache_nodes, i);
              trace("watch thread: unsubscribe: freeing node %x, prefix %s, index %i\n", (guint)cache_node, self->message.watch.prefix, i);
              if (cache_node != NULL) {
                  cache_item = cache_node->data;
                  cache_item->subscription_count --;
                  if (cache_item->subscription_count > 0) break;
              }
              _free_watch(self, i, cache_node);
              g_free(self->message.watch.prefix);
              g_atomic_int_inc(&self->watches_remaining);
              break;
          }
      case WATCH_THREAD_STOP: {
              gint i;
              for (i=1; i<self->events->len; i++) _free_watch(self, i, g_ptr_array_index(self->cache_nodes, i));
              //SetEvent(self->message_received_event);
              //ExitThread(0);
          }
  }
  self->message.type = WATCH_THREAD_NONE;
  //SetEvent(self->message_received_event);
}
static DWORD watch_thread_function(LPVOID parameter) {
  /*WatchThreadState *self = (WatchThreadState*)parameter;
  DWORD result;
  self->events = g_ptr_array_new();
  self->handles = g_ptr_array_new();
  self->prefixes = g_ptr_array_new();
  self->cache_nodes = g_ptr_array_new();
  g_ptr_array_add(self->events, self->message_sent_event);
  g_ptr_array_add(self->handles, NULL);
  g_ptr_array_add(self->prefixes, NULL);
  g_ptr_array_add(self->cache_nodes, NULL);
  while(1) {
      trace("watch thread: going to sleep; %i events watched.\n", self->events->len);
      result = WaitForMultipleObjects(self->events->len, self->events->pdata, FALSE, INFINITE);
      if (result == WAIT_OBJECT_0) {
          watch_thread_handle_message(self);
      } else if (result > WAIT_OBJECT_0 && result <= WAIT_OBJECT_0 + self->events->len) {
          HKEY hpath;
          HANDLE cond;
          gchar *prefix;
          GNode *cache_node;
          RegistryCacheItem *cache_item;
          RegistryEvent *event;
          gint notify_index = result - WAIT_OBJECT_0;
          hpath = g_ptr_array_index(self->handles, notify_index);
          cond = g_ptr_array_index(self->events, notify_index);
          prefix = g_ptr_array_index(self->prefixes, notify_index);
          cache_node = g_ptr_array_index(self->cache_nodes, notify_index);
          trace("Watch thread: notify received on prefix %i: %s.\n", notify_index, prefix);
          if (cache_node == NULL) {
              trace("Notify received on a path that was deleted :(\n");
              continue;
          }
          result = registry_watch_key(hpath, cond);
          if (result != ERROR_SUCCESS) {
              if (result != ERROR_KEY_DELETED) g_message_win32_error(result, "watch thread: failed to watch %s", prefix);
              _free_watch(self, notify_index, cache_node);
              g_atomic_int_inc(&self->watches_remaining);
              continue;
          }
          cache_item = cache_node->data;
          if (cache_item->block_count) {
              cache_item->block_count --;
              trace("Watch thread: notify blocked at %s\n", prefix);
              continue;
          }
          event = g_slice_new(RegistryEvent);
          event->self = G_REGISTRY_BACKEND(self->owner);
          g_object_ref(self->owner);
          event->items = g_ptr_array_new();
          EnterCriticalSection(G_REGISTRY_BACKEND(self->owner)->cache_lock);
          registry_cache_update(G_REGISTRY_BACKEND(self->owner), hpath, prefix, NULL, cache_node, 0, event->items);
          LeaveCriticalSection(G_REGISTRY_BACKEND(self->owner)->cache_lock);
          if (event->items->len > 0) {
              event->prefix = g_strdup(prefix);
              g_idle_add((GSourceFunc)watch_handler, event);
          } else {
              g_ptr_array_free(event->items, TRUE);
              g_slice_free(RegistryEvent, event);
          }
      } else g_message_win32_error(GetLastError(), "watch thread: WaitForMultipleObjects error");
  }*/
  return -1;
}
static gboolean watch_start(GRegistryBackend *self) {
  /*WatchThreadState *watch;
  g_return_val_if_fail(self->watch == NULL, FALSE);
  self->cache_lock = g_slice_new(CRITICAL_SECTION);
  InitializeCriticalSection(self->cache_lock);
  watch = g_slice_new(WatchThreadState);
  watch->owner = G_SETTINGS_BACKEND(self);
  watch->watches_remaining = MAX_WATCHES;
  watch->message_lock = g_slice_new(CRITICAL_SECTION);
  InitializeCriticalSection(watch->message_lock);
  watch->message_sent_event = CreateEvent(NULL, FALSE, FALSE, NULL);
  watch->message_received_event = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (watch->message_sent_event == NULL || watch->message_received_event == NULL) {
      g_message_win32_error(0, "gregistrybackend: Failed to create sync objects.");
      goto fail_1;
  }
  watch->thread = CreateThread(NULL, 1024, watch_thread_function, watch, 0, NULL);
  if (watch->thread == NULL) {
      g_message_win32_error(0, "gregistrybackend: Failed to create notify watch thread.");
      goto fail_2;
  }
  self->watch = watch;
  return TRUE;
fail_2:
  DeleteCriticalSection(self->cache_lock);
  g_slice_free(CRITICAL_SECTION, self->cache_lock);
  DeleteCriticalSection(watch->message_lock);
  g_slice_free(CRITICAL_SECTION, watch->message_lock);
  CloseHandle(watch->message_sent_event);
  CloseHandle(watch->message_received_event);
fail_1:
  g_slice_free(WatchThreadState, watch);*/
  return FALSE;
}
static void watch_stop_unlocked(GRegistryBackend *self) {
  /*WatchThreadState *watch = self->watch;
  DWORD result;
  g_return_if_fail(watch != NULL);
  watch->message.type = WATCH_THREAD_STOP;
  SetEvent(watch->message_sent_event);
  result = WaitForSingleObject(watch->message_received_event, INFINITE);
  if (result != WAIT_OBJECT_0) {
      g_warning("gregistrybackend: unable to stop watch thread.");
      return;
  }
  LeaveCriticalSection(watch->message_lock);
  DeleteCriticalSection(watch->message_lock);
  DeleteCriticalSection(self->cache_lock);
  g_slice_free(CRITICAL_SECTION, watch->message_lock);
  g_slice_free(CRITICAL_SECTION, self->cache_lock);
  CloseHandle(watch->message_sent_event);
  CloseHandle(watch->message_received_event);
  CloseHandle(watch->thread);
  g_slice_free(WatchThreadState, watch);
  trace("\nwatch thread: %x: all data freed.\n", self);
  self->watch = NULL;*/
};
static gboolean watch_add_notify(GRegistryBackend *self, HANDLE event, HKEY hpath, gchar *gsettings_prefix) {
  /*WatchThreadState *watch = self->watch;
  GNode *cache_node;
  RegistryCacheItem *cache_item;
  DWORD result;
  g_return_val_if_fail(watch != NULL, FALSE);
  trace("watch_add_notify: prefix %s.\n", gsettings_prefix);
  EnterCriticalSection(self->cache_lock);
  cache_node = registry_cache_get_node_for_key(self->cache_root, gsettings_prefix, TRUE);
  g_return_val_if_fail(cache_node != NULL, FALSE);
  g_return_val_if_fail(cache_node->data != NULL, FALSE);
  cache_item = cache_node->data;
  cache_item->subscription_count ++;
  if (cache_item->subscription_count > 1) {
      trace("watch_add_notify: prefix %s already watched, %i subscribers.\n", gsettings_prefix, cache_item->subscription_count);
      return FALSE;
  }
  registry_cache_ref_tree(cache_node);
  registry_cache_update(self, hpath, gsettings_prefix, NULL, cache_node, 0, NULL);
  //registry_cache_dump(self->cache_root, NULL);
  LeaveCriticalSection(self->cache_lock);
  EnterCriticalSection(watch->message_lock);
  watch->message.type = WATCH_THREAD_ADD_WATCH;
  watch->message.watch.event = event;
  watch->message.watch.hpath = hpath;
  watch->message.watch.prefix = gsettings_prefix;
  watch->message.watch.cache_node = cache_node;
  SetEvent(watch->message_sent_event);
  result = WaitForSingleObject(watch->message_received_event, 200);
#ifdef TRACE
  if (result != WAIT_OBJECT_0) trace("watch thread is slow to respond - notification may not be added.");
#endif
  LeaveCriticalSection(watch->message_lock);*/
  return TRUE;
};
static void watch_remove_notify(GRegistryBackend *self, const gchar *key_name) {
  /*WatchThreadState *watch = self->watch;
  LONG result;
  if (self->watch == NULL) return;
  EnterCriticalSection(watch->message_lock);
  watch->message.type = WATCH_THREAD_REMOVE_WATCH;
  watch->message.watch.prefix = g_strdup(key_name);
  SetEvent(watch->message_sent_event);
  result = WaitForSingleObject(watch->message_received_event, INFINITE);
  if (result != ERROR_SUCCESS) g_warning("unsubscribe from %s: message not acknowledged\n", key_name);
  if (g_atomic_int_get(&watch->watches_remaining) >= MAX_WATCHES) watch_stop_unlocked(self);
  else LeaveCriticalSection(watch->message_lock);*/
}
static void g_registry_backend_subscribe(GSettingsBackend *backend, const char *key_name) {
  /*GRegistryBackend *self = G_REGISTRY_BACKEND(backend);
  gchar *path_name, *value_name = NULL;
  HKEY hpath;
  HANDLE event;
  LONG result;
  if (self->watch == NULL)
      if (!watch_start(self)) return;
  if (g_atomic_int_dec_and_test(&self->watch->watches_remaining)) {
      g_atomic_int_inc(&self->watch->watches_remaining);
      g_warning("subscribe() failed: only %i different paths may be watched.\n", MAX_WATCHES);
      return;
  }
  path_name = parse_key(key_name, self->base_path, &value_name);
  if (value_name != NULL && *value_name != 0) g_warning("subscribe() failed: path must end in a /, got %s\n", key_name);
  trace("Subscribing to %s [registry %s / %s] - watch %x\n", key_name, path_name, value_name, self->watch);
  result = RegCreateKeyExA(HKEY_CURRENT_USER, path_name, 0, NULL, 0, KEY_READ, NULL, &hpath, NULL);
  g_free(path_name);
  if (result != ERROR_SUCCESS) {
      g_message_win32_error(result, "gregistrybackend: Unable to subscribe to key %s.", key_name);
      g_atomic_int_inc(&self->watch->watches_remaining);
      return;
  }
  event = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (event == NULL) {
      g_message_win32_error(result, "gregistrybackend: CreateEvent failed.\n");
      g_atomic_int_inc(&self->watch->watches_remaining);
      RegCloseKey(hpath);
      return;
  }
  if (!watch_add_notify(self, event, hpath, g_strdup (key_name))) g_atomic_int_inc(&self->watch->watches_remaining);*/
}
static void g_registry_backend_unsubscribe(GSettingsBackend *backend, const char *key_name) {
  trace("unsubscribe: %s.\n", key_name);
  watch_remove_notify(G_REGISTRY_BACKEND (backend), key_name);
}
GSettingsBackend *g_registry_backend_new(void) {
  return g_object_new(G_TYPE_REGISTRY_BACKEND, NULL);
}
static void g_registry_backend_finalize(GObject *object) {
  /*GRegistryBackend *self = G_REGISTRY_BACKEND(object);
  RegistryCacheItem *item;
  item = self->cache_root->data;
  g_warn_if_fail(item->ref_count == 1);
  _free_cache_item(item);
  g_node_destroy(self->cache_root);
  if (self->watch != NULL) {
      EnterCriticalSection(self->watch->message_lock);
      watch_stop_unlocked(self);
  }
  g_free(self->base_path);*/
}
static void g_registry_backend_class_init(GRegistryBackendClass *class) {
  GSettingsBackendClass *backend_class = G_SETTINGS_BACKEND_CLASS(class);
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  object_class->finalize = g_registry_backend_finalize;
  backend_class->read = g_registry_backend_read;
  backend_class->write = g_registry_backend_write;
  backend_class->write_tree = g_registry_backend_write_tree;
  backend_class->reset = g_registry_backend_reset;
  backend_class->get_writable = g_registry_backend_get_writable;
  backend_class->get_permission = g_registry_backend_get_permission;
  backend_class->subscribe = g_registry_backend_subscribe;
  backend_class->unsubscribe = g_registry_backend_unsubscribe;
}
static void g_registry_backend_init(GRegistryBackend *self) {
  RegistryCacheItem *item;
  self->base_path = g_strdup_printf("Software\\GSettings");
  item = g_slice_new(RegistryCacheItem);
  item->value.type = REG_NONE;
  item->value.ptr = NULL;
  item->name = g_strdup("<root>");
  item->ref_count = 1;
  self->cache_root = g_node_new(item);
  self->watch = NULL;
}