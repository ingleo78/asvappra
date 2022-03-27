#include <string.h>
#include <stdlib.h>
#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "../gobject/gboxed.h"
#include "config.h"
#include "gsettingsbackendinternal.h"
#include "gsimplepermission.h"
#include "giomodule-priv.h"

G_DEFINE_ABSTRACT_TYPE (GSettingsBackend, g_settings_backend, G_TYPE_OBJECT);
typedef struct _GSettingsBackendClosure GSettingsBackendClosure;
typedef struct _GSettingsBackendWatch   GSettingsBackendWatch;
struct _GSettingsBackendPrivate {
  GSettingsBackendWatch *watches;
  GStaticMutex lock;
};
static gboolean is_key(const gchar *key) {
  gint length;
  gint i;
  g_return_val_if_fail(key != NULL, FALSE);
  g_return_val_if_fail(key[0] == '/', FALSE);
  for (i = 1; key[i]; i++) g_return_val_if_fail(key[i] != '/' || key[i + 1] != '/', FALSE);
  length = i;
  g_return_val_if_fail(key[length - 1] != '/', FALSE);
  return TRUE;
}
static gboolean is_path(const gchar *path) {
  gint length;
  gint i;
  g_return_val_if_fail(path != NULL, FALSE);
  g_return_val_if_fail(path[0] == '/', FALSE);
  for (i = 1; path[i]; i++) g_return_val_if_fail(path[i] != '/' || path[i + 1] != '/', FALSE);
  length = i;
  g_return_val_if_fail(path[length - 1] == '/', FALSE);
  return TRUE;
}
struct _GSettingsBackendWatch {
  GObject *target;
  const GSettingsListenerVTable *vtable;
  GMainContext *context;
  GSettingsBackendWatch *next;
};
struct _GSettingsBackendClosure {
  void (*function)(GObject *target, GSettingsBackend *backend, const gchar *name, gpointer data1, gpointer data2);
  GSettingsBackend *backend;
  GObject *target;
  gchar *name;
  gpointer data1;
  GBoxedFreeFunc data1_free;
  gpointer data2;
};
static void g_settings_backend_watch_weak_notify(gpointer data, GObject *where_the_object_was) {
  GSettingsBackend *backend = data;
  GSettingsBackendWatch **ptr;
  g_static_mutex_lock(&backend->priv->lock);
  for (ptr = &backend->priv->watches; *ptr; ptr = &(*ptr)->next)
      if ((*ptr)->target == where_the_object_was) {
          GSettingsBackendWatch *tmp = *ptr;
          *ptr = tmp->next;
          g_slice_free(GSettingsBackendWatch, tmp);
          g_static_mutex_unlock(&backend->priv->lock);
          return;
      }
  g_assert_not_reached();
}
void g_settings_backend_watch(GSettingsBackend *backend, const GSettingsListenerVTable *vtable, GObject *target, GMainContext *context) {
  GSettingsBackendWatch *watch;
  watch = g_slice_new(GSettingsBackendWatch);
  watch->context = context;
  watch->vtable = vtable;
  watch->target = target;
  g_object_weak_ref(target, g_settings_backend_watch_weak_notify, backend);
  g_static_mutex_lock(&backend->priv->lock);
  watch->next = backend->priv->watches;
  backend->priv->watches = watch;
  g_static_mutex_unlock(&backend->priv->lock);
}
void g_settings_backend_unwatch(GSettingsBackend *backend, GObject *target) {
  g_object_weak_unref(target, g_settings_backend_watch_weak_notify, backend);
  g_settings_backend_watch_weak_notify(backend, target);
}
static gboolean g_settings_backend_invoke_closure(gpointer user_data) {
  GSettingsBackendClosure *closure = user_data;
  closure->function(closure->target, closure->backend, closure->name, closure->data1, closure->data2);
  closure->data1_free(closure->data1);
  g_object_unref(closure->backend);
  g_object_unref(closure->target);
  g_free(closure->name);
  g_slice_free(GSettingsBackendClosure, closure);
  return FALSE;
}
static gpointer pointer_id(gpointer a) {
  return a;
}
static void pointer_ignore(gpointer a) {}
static void g_settings_backend_dispatch_signal(GSettingsBackend *backend, gsize function_offset, const gchar *name, gpointer data1, GBoxedCopyFunc data1_copy,
                                               GBoxedFreeFunc data1_free, gpointer data2) {
  GSettingsBackendWatch *suffix, *watch, *next;
  if (data1_copy == NULL) data1_copy = pointer_id;
  if (data1_free == NULL) data1_free = pointer_ignore;
  g_static_mutex_lock(&backend->priv->lock);
  suffix = backend->priv->watches;
  for (watch = suffix; watch; watch = watch->next) g_object_ref(watch->target);
  g_static_mutex_unlock(&backend->priv->lock);
  for (watch = suffix; watch; watch = next) {
      GSettingsBackendClosure *closure;
      closure = g_slice_new(GSettingsBackendClosure);
      closure->backend = g_object_ref(backend);
      closure->target = watch->target;
      closure->function = G_STRUCT_MEMBER(void *, watch->vtable, function_offset);
      closure->name = g_strdup(name);
      closure->data1 = data1_copy(data1);
      closure->data1_free = data1_free;
      closure->data2 = data2;
      next = watch->next;
      if (watch->context) g_main_context_invoke(watch->context, g_settings_backend_invoke_closure, closure);
      else g_settings_backend_invoke_closure(closure);
  }
}
void g_settings_backend_changed(GSettingsBackend *backend, const gchar *key, gpointer origin_tag) {
  g_return_if_fail(G_IS_SETTINGS_BACKEND(backend));
  g_return_if_fail(is_key(key));
  g_settings_backend_dispatch_signal(backend, G_STRUCT_OFFSET(GSettingsListenerVTable, changed), key, origin_tag,NULL,NULL,NULL);
}
void g_settings_backend_keys_changed(GSettingsBackend *backend, const gchar *path, gchar const * const *items, gpointer origin_tag) {
  g_return_if_fail(G_IS_SETTINGS_BACKEND(backend));
  g_return_if_fail(is_path (path));
  g_return_if_fail(items != NULL);
  g_settings_backend_dispatch_signal(backend, G_STRUCT_OFFSET(GSettingsListenerVTable, keys_changed), path, (gpointer)items, (GBoxedCopyFunc)g_strdupv,
                                     (GBoxedFreeFunc)g_strfreev, origin_tag);
}
void g_settings_backend_path_changed(GSettingsBackend *backend, const gchar *path, gpointer origin_tag) {
  g_return_if_fail(G_IS_SETTINGS_BACKEND(backend));
  g_return_if_fail(is_path(path));
  g_settings_backend_dispatch_signal(backend, G_STRUCT_OFFSET(GSettingsListenerVTable, path_changed), path, origin_tag,NULL,NULL,NULL);
}
void g_settings_backend_writable_changed(GSettingsBackend *backend, const gchar *key) {
  g_return_if_fail(G_IS_SETTINGS_BACKEND(backend));
  g_return_if_fail(is_key(key));
  g_settings_backend_dispatch_signal(backend, G_STRUCT_OFFSET(GSettingsListenerVTable, writable_changed), key,NULL,NULL,NULL,NULL);
}
void g_settings_backend_path_writable_changed(GSettingsBackend *backend, const gchar *path) {
  g_return_if_fail(G_IS_SETTINGS_BACKEND(backend));
  g_return_if_fail(is_path(path));
  g_settings_backend_dispatch_signal(backend, G_STRUCT_OFFSET(GSettingsListenerVTable, path_writable_changed), path,NULL,NULL,NULL,NULL);
}
typedef struct {
  const gchar **keys;
  GVariant **values;
  gint prefix_len;
  gchar *prefix;
} FlattenState;
static gboolean g_settings_backend_flatten_one(gpointer key, gpointer value, gpointer user_data) {
  FlattenState *state = user_data;
  const gchar *skey = key;
  gint i;
  g_return_val_if_fail(is_key(key), TRUE);
  if (state->prefix == NULL) {
      gchar *last_byte;
      state->prefix = g_strdup(skey);
      last_byte = strrchr(state->prefix, '/') + 1;
      state->prefix_len = last_byte - state->prefix;
      *last_byte = '\0';
  } else {
      for (i = 0; state->prefix[i] == skey[i]; i++);
      if (state->prefix[i] != '\0') {
          while(state->prefix[i - 1] != '/') i--;
          state->prefix[i] = '\0';
          state->prefix_len = i;
      }
  }
  *state->keys++ = key;
  if (state->values) *state->values++ = value;
  return FALSE;
}
void g_settings_backend_flatten_tree(GTree *tree, gchar **path, const gchar ***keys, GVariant ***values) {
  FlattenState state = { 0, };
  gsize nnodes;
  nnodes = g_tree_nnodes(tree);
  *keys = state.keys = g_new(const gchar *, nnodes + 1);
  state.keys[nnodes] = NULL;
  if (values != NULL) {
      *values = state.values = g_new(GVariant *, nnodes + 1);
      state.values[nnodes] = NULL;
  }
  g_tree_foreach(tree, g_settings_backend_flatten_one, &state);
  g_return_if_fail(*keys + nnodes == state.keys);
  *path = state.prefix;
  while(nnodes--) *--state.keys += state.prefix_len;
}
void g_settings_backend_changed_tree(GSettingsBackend *backend, GTree *tree, gpointer origin_tag) {
  GSettingsBackendWatch *watch;
  const gchar **keys;
  gchar *path;
  g_return_if_fail(G_IS_SETTINGS_BACKEND(backend));
  g_settings_backend_flatten_tree(tree, &path, &keys, NULL);
#ifdef DEBUG_CHANGES
  {
      gint i;
      g_print("----\n");
      g_print("changed_tree(): prefix %s\n", path);
      for (i = 0; keys[i]; i++) g_print("  %s\n", keys[i]);
      g_print("----\n");
  }
#endif
  for (watch = backend->priv->watches; watch; watch = watch->next) watch->vtable->keys_changed(watch->target, backend, path, keys, origin_tag);
  g_free(path);
  g_free(keys);
}
GVariant *g_settings_backend_read(GSettingsBackend *backend, const gchar *key, const GVariantType *expected_type, gboolean default_value) {
  GVariant *value;
  value = G_SETTINGS_BACKEND_GET_CLASS(backend)->read(backend, key, expected_type, default_value);
  if G_UNLIKELY(value && !g_variant_is_of_type(value, expected_type)) {
      g_variant_unref(value);
      value = NULL;
  }
  return value;
}
gboolean g_settings_backend_write(GSettingsBackend *backend, const gchar *key, GVariant *value, gpointer origin_tag) {
  return G_SETTINGS_BACKEND_GET_CLASS(backend)->write(backend, key, value, origin_tag);
}
gboolean g_settings_backend_write_tree(GSettingsBackend *backend, GTree *tree, gpointer origin_tag) {
  return G_SETTINGS_BACKEND_GET_CLASS(backend)->write_tree(backend, tree, origin_tag);
}
void g_settings_backend_reset(GSettingsBackend *backend, const gchar *key, gpointer origin_tag) {
  G_SETTINGS_BACKEND_GET_CLASS(backend)->reset(backend, key, origin_tag);
}
gboolean g_settings_backend_get_writable(GSettingsBackend *backend, const gchar *key) {
  return G_SETTINGS_BACKEND_GET_CLASS(backend)->get_writable(backend, key);
}
void g_settings_backend_unsubscribe(GSettingsBackend *backend, const char *name) {
  G_SETTINGS_BACKEND_GET_CLASS(backend)->unsubscribe(backend, name);
}
void g_settings_backend_subscribe(GSettingsBackend *backend, const gchar *name) {
  G_SETTINGS_BACKEND_GET_CLASS(backend) ->subscribe(backend, name);
}
static void g_settings_backend_finalize(GObject *object) {
  GSettingsBackend *backend = G_SETTINGS_BACKEND(object);
  g_static_mutex_unlock(&backend->priv->lock);
  G_OBJECT_CLASS(g_settings_backend_parent_class)->finalize(object);
}
static void ignore_subscription(GSettingsBackend *backend, const gchar *key) {}
static void g_settings_backend_init(GSettingsBackend *backend) {
  backend->priv = G_TYPE_INSTANCE_GET_PRIVATE(backend, G_TYPE_SETTINGS_BACKEND, GSettingsBackendPrivate);
  g_static_mutex_init(&backend->priv->lock);
}
static void g_settings_backend_class_init(GSettingsBackendClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  class->subscribe = ignore_subscription;
  class->unsubscribe = ignore_subscription;
  gobject_class->finalize = g_settings_backend_finalize;
  g_type_class_add_private(class, sizeof(GSettingsBackendPrivate));
}
GTree *g_settings_backend_create_tree(void) {
  return g_tree_new_full((GCompareDataFunc)strcmp, NULL, g_free, (GDestroyNotify)g_variant_unref);
}
GSettingsBackend *g_settings_backend_get_default(void) {
  static gsize backend;
  if (g_once_init_enter(&backend)) {
      GSettingsBackend *instance;
      GIOExtensionPoint *point;
      GIOExtension *extension;
      GType extension_type;
      GList *extensions;
      const gchar *env;
      _g_io_modules_ensure_loaded();
      point = g_io_extension_point_lookup(G_SETTINGS_BACKEND_EXTENSION_POINT_NAME);
      extension = NULL;
      if ((env = getenv("GSETTINGS_BACKEND"))) {
          extension = g_io_extension_point_get_extension_by_name(point, env);
          if (extension == NULL) g_warning("Can't find GSettings backend '%s' given in GSETTINGS_BACKEND environment variable", env);
      }
      if (extension == NULL) {
          extensions = g_io_extension_point_get_extensions(point);
          if (extensions == NULL) g_error("No GSettingsBackend implementations exist.");
          extension = extensions->data;
          if (strcmp(g_io_extension_get_name(extension), "memory") == 0)
              g_message("Using the 'memory' GSettings backend.  Your settings will not be saved or shared with other applications.");
      }
      extension_type = g_io_extension_get_type(extension);
      instance = g_object_new(extension_type, NULL);
      g_once_init_leave(&backend, (gsize)instance);
  }
  return g_object_ref((void*)backend);
}
GPermission *g_settings_backend_get_permission(GSettingsBackend *backend, const gchar *path) {
  GSettingsBackendClass *class = G_SETTINGS_BACKEND_GET_CLASS(backend);
  if (class->get_permission) return class->get_permission(backend, path);
  return g_simple_permission_new(TRUE);
}
void g_settings_backend_sync_default(void) {
  GSettingsBackendClass *class;
  GSettingsBackend *backend;
  backend = g_settings_backend_get_default();
  class = G_SETTINGS_BACKEND_GET_CLASS(backend);
  if (class->sync) class->sync(backend);
}