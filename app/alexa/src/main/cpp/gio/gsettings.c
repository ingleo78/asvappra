#define _GNU_SOURCE
#include <string.h>
#include <locale.h>
#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gsettings.h"
#include "gdelayedsettingsbackend.h"
#include "gsettingsbackendinternal.h"
#include "gsettings-mapping.h"
#include "gsettingsschema.h"
#include "strinfo.c"

struct _GSettingsPrivate {
  GMainContext *main_context;
  GSettingsBackend *backend;
  GSettingsSchema *schema;
  gchar *schema_name;
  gchar *path;
  GDelayedSettingsBackend *delayed;
};
enum {
  PROP_0,
  PROP_SCHEMA,
  PROP_BACKEND,
  PROP_PATH,
  PROP_HAS_UNAPPLIED,
  PROP_DELAY_APPLY
};
enum {
  SIGNAL_WRITABLE_CHANGE_EVENT,
  SIGNAL_WRITABLE_CHANGED,
  SIGNAL_CHANGE_EVENT,
  SIGNAL_CHANGED,
  N_SIGNALS
};
static guint g_settings_signals[N_SIGNALS];
G_DEFINE_TYPE(GSettings, g_settings, G_TYPE_OBJECT);
static gboolean g_settings_real_change_event(GSettings *settings, const GQuark *keys, gint n_keys) {
  gint i;
  if (keys == NULL) keys = g_settings_schema_list(settings->priv->schema, &n_keys);
  for (i = 0; i < n_keys; i++) g_signal_emit(settings, g_settings_signals[SIGNAL_CHANGED], keys[i], g_quark_to_string(keys[i]));
  return FALSE;
}
static gboolean g_settings_real_writable_change_event(GSettings *settings, GQuark key) {
  const GQuark *keys = &key;
  gint n_keys = 1;
  gint i;
  if (key == 0) keys = g_settings_schema_list(settings->priv->schema, &n_keys);
  for (i = 0; i < n_keys; i++) g_signal_emit(settings, g_settings_signals[SIGNAL_WRITABLE_CHANGED], keys[i], g_quark_to_string(keys[i]));
  return FALSE;
}
static void settings_backend_changed(GObject *target, GSettingsBackend *backend, const gchar *key, gpointer origin_tag) {
  GSettings *settings = G_SETTINGS(target);
  gboolean ignore_this;
  gint i;
  g_assert(settings->priv->backend == backend);
  for (i = 0; key[i] == settings->priv->path[i]; i++);
  if (settings->priv->path[i] == '\0' && g_settings_schema_has_key(settings->priv->schema, key + i)) {
      GQuark quark;
      quark = g_quark_from_string(key + i);
      g_signal_emit(settings, g_settings_signals[SIGNAL_CHANGE_EVENT],0, &quark, 1, &ignore_this);
  }
}
static void settings_backend_path_changed(GObject *target, GSettingsBackend *backend, const gchar *path, gpointer origin_tag) {
  GSettings *settings = G_SETTINGS(target);
  gboolean ignore_this;
  g_assert(settings->priv->backend == backend);
  if (g_str_has_prefix(settings->priv->path, path)) g_signal_emit(settings, g_settings_signals[SIGNAL_CHANGE_EVENT],0, NULL, 0, &ignore_this);
}
static void settings_backend_keys_changed(GObject *target, GSettingsBackend *backend, const gchar *path, const gchar * const *items, gpointer origin_tag) {
  GSettings *settings = G_SETTINGS(target);
  gboolean ignore_this;
  gint i;
  g_assert(settings->priv->backend == backend);
  for (i = 0; settings->priv->path[i] && settings->priv->path[i] == path[i]; i++);
  if (path[i] == '\0') {
      GQuark quarks[256];
      gint j, l = 0;
      for (j = 0; items[j]; j++) {
           const gchar *item = items[j];
           gint k;
           for (k = 0; item[k] == settings->priv->path[i + k]; k++);
           if (settings->priv->path[i + k] == '\0' && g_settings_schema_has_key(settings->priv->schema, item + k))
               quarks[l++] = g_quark_from_string(item + k);
           g_assert(l < 256);
      }
      if (l > 0) g_signal_emit(settings, g_settings_signals[SIGNAL_CHANGE_EVENT],0, quarks, l, &ignore_this);
  }
}
static void settings_backend_writable_changed(GObject *target, GSettingsBackend *backend, const gchar *key) {
  GSettings *settings = G_SETTINGS(target);
  gboolean ignore_this;
  gint i;
  g_assert(settings->priv->backend == backend);
  for (i = 0; key[i] == settings->priv->path[i]; i++);
  if (settings->priv->path[i] == '\0' && g_settings_schema_has_key(settings->priv->schema, key + i))
      g_signal_emit(settings, g_settings_signals[SIGNAL_WRITABLE_CHANGE_EVENT],0, g_quark_from_string(key + i), &ignore_this);
}
static void settings_backend_path_writable_changed(GObject *target, GSettingsBackend *backend, const gchar *path) {
  GSettings *settings = G_SETTINGS(target);
  gboolean ignore_this;
  g_assert(settings->priv->backend == backend);
  if (g_str_has_prefix (settings->priv->path, path)) g_signal_emit(settings, g_settings_signals[SIGNAL_WRITABLE_CHANGE_EVENT],0, (GQuark)0, &ignore_this);
}
static void g_settings_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GSettings *settings = G_SETTINGS(object);
  switch(prop_id) {
      case PROP_SCHEMA:
          g_assert(settings->priv->schema_name == NULL);
          settings->priv->schema_name = g_value_dup_string(value);
          break;
      case PROP_PATH: settings->priv->path = g_value_dup_string(value); break;
      case PROP_BACKEND: settings->priv->backend = g_value_dup_object(value); break;
      default: g_assert_not_reached();
  }
}
static void g_settings_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GSettings *settings = G_SETTINGS(object);
  switch(prop_id) {
     case PROP_SCHEMA: g_value_set_string(value, settings->priv->schema_name); break;
     case PROP_BACKEND: g_value_set_object(value, settings->priv->backend); break;
     case PROP_PATH: g_value_set_string(value, settings->priv->path); break;
     case PROP_HAS_UNAPPLIED: g_value_set_boolean(value, g_settings_get_has_unapplied(settings)); break;
     case PROP_DELAY_APPLY: g_value_set_boolean(value, settings->priv->delayed != NULL); break;
     default: g_assert_not_reached();
  }
}
static const GSettingsListenerVTable listener_vtable = {
  settings_backend_changed,
  settings_backend_path_changed,
  settings_backend_keys_changed,
  settings_backend_writable_changed,
  settings_backend_path_writable_changed
};
static void g_settings_constructed(GObject *object) {
  GSettings *settings = G_SETTINGS(object);
  const gchar *schema_path;
  settings->priv->schema = g_settings_schema_new(settings->priv->schema_name);
  schema_path = g_settings_schema_get_path(settings->priv->schema);
  if (settings->priv->path && schema_path && strcmp(settings->priv->path, schema_path) != 0) {
      g_error("settings object created with schema '%s' and path '%s', but path '%s' is specified by schema", settings->priv->schema_name,
              settings->priv->path, schema_path);
  }
  if (settings->priv->path == NULL) {
      if (schema_path == NULL) g_error("attempting to create schema '%s' without a path", settings->priv->schema_name);
      settings->priv->path = g_strdup(schema_path);
  }
  if (settings->priv->backend == NULL) settings->priv->backend = g_settings_backend_get_default();
  g_settings_backend_watch(settings->priv->backend, &listener_vtable, G_OBJECT(settings), settings->priv->main_context);
  g_settings_backend_subscribe(settings->priv->backend, settings->priv->path);
}
static void g_settings_finalize(GObject *object) {
  GSettings *settings = G_SETTINGS(object);
  g_settings_backend_unsubscribe(settings->priv->backend, settings->priv->path);
  g_main_context_unref(settings->priv->main_context);
  g_object_unref(settings->priv->backend);
  g_object_unref(settings->priv->schema);
  g_free(settings->priv->schema_name);
  g_free(settings->priv->path);
  G_OBJECT_CLASS(g_settings_parent_class)->finalize(object);
}
static void g_settings_init(GSettings *settings) {
  settings->priv = G_TYPE_INSTANCE_GET_PRIVATE(settings, G_TYPE_SETTINGS, GSettingsPrivate);
  settings->priv->main_context = g_main_context_get_thread_default();
  if (settings->priv->main_context == NULL) settings->priv->main_context = g_main_context_default();
  g_main_context_ref(settings->priv->main_context);
}
static void g_settings_class_init(GSettingsClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  class->writable_change_event = g_settings_real_writable_change_event;
  class->change_event = g_settings_real_change_event;
  object_class->set_property = g_settings_set_property;
  object_class->get_property = g_settings_get_property;
  object_class->constructed = g_settings_constructed;
  object_class->finalize = g_settings_finalize;
  g_type_class_add_private(object_class, sizeof(GSettingsPrivate));
  g_settings_signals[SIGNAL_CHANGED] = g_signal_new("changed", G_TYPE_SETTINGS, G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED, G_STRUCT_OFFSET(GSettingsClass, changed),
                                                    NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);
  g_settings_signals[SIGNAL_CHANGE_EVENT] = g_signal_new("change-event", G_TYPE_SETTINGS, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GSettingsClass, change_event),
                                                         g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 2,
                                                         G_TYPE_POINTER, G_TYPE_INT);
  g_settings_signals[SIGNAL_WRITABLE_CHANGED] = g_signal_new("writable-changed", G_TYPE_SETTINGS, G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                                                             G_STRUCT_OFFSET(GSettingsClass, writable_changed), NULL, NULL, g_cclosure_marshal_VOID__STRING,
                                                             G_TYPE_NONE, 1, G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);
  g_settings_signals[SIGNAL_WRITABLE_CHANGE_EVENT] = g_signal_new("writable-change-event", G_TYPE_SETTINGS, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GSettingsClass,
                                                                  writable_change_event), g_signal_accumulator_true_handled, NULL, NULL,
                                                                  G_TYPE_BOOLEAN, 1, G_TYPE_UINT);
  g_object_class_install_property (object_class, PROP_BACKEND,g_param_spec_object("backend", P_("GSettingsBackend"),
                                  P_("The GSettingsBackend for this settings object"), G_TYPE_SETTINGS_BACKEND, G_PARAM_CONSTRUCT_ONLY |
                                  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(object_class, PROP_SCHEMA,g_param_spec_string("schema", P_("Schema name"), P_("The name of"
                                  " the schema for this settings object"),NULL,G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_PATH,g_param_spec_string("path", P_("Base path"), P_("The path within "
                                   "the backend where the settings are"),NULL,G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_HAS_UNAPPLIED,g_param_spec_boolean("has-unapplied", P_("Has unapplied changes"),
                                   P_("TRUE if there are outstanding changes to apply()"),FALSE,G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_DELAY_APPLY,g_param_spec_boolean("delay-apply", P_("Delay-apply mode"),
                                   P_("Whether this settings object is in 'delay-apply' mode"),FALSE,G_PARAM_READABLE |
                                   G_PARAM_STATIC_STRINGS));
}
GSettings *g_settings_new(const gchar *schema) {
  g_return_val_if_fail(schema != NULL, NULL);
  return g_object_new(G_TYPE_SETTINGS, "schema", schema, NULL);
}
GSettings *g_settings_new_with_path(const gchar *schema, const gchar *path) {
  g_return_val_if_fail(schema != NULL, NULL);
  g_return_val_if_fail(path != NULL, NULL);
  return g_object_new(G_TYPE_SETTINGS, "schema", schema, "path", path, NULL);
}
GSettings *g_settings_new_with_backend(const gchar *schema, GSettingsBackend *backend) {
  g_return_val_if_fail(schema != NULL, NULL);
  g_return_val_if_fail(G_IS_SETTINGS_BACKEND(backend), NULL);
  return g_object_new(G_TYPE_SETTINGS, "schema", schema, "backend", backend, NULL);
}
GSettings *g_settings_new_with_backend_and_path(const gchar *schema, GSettingsBackend *backend, const gchar *path) {
  g_return_val_if_fail(schema != NULL, NULL);
  g_return_val_if_fail(G_IS_SETTINGS_BACKEND(backend), NULL);
  g_return_val_if_fail(path != NULL, NULL);
  return g_object_new(G_TYPE_SETTINGS, "schema", schema, "backend", backend, "path", path, NULL);
}
typedef struct {
  GSettings *settings;
  const gchar *key;
  GSettingsSchema *schema;
  guint is_flags : 1;
  guint is_enum  : 1;
  const guint32 *strinfo;
  gsize strinfo_length;
  const gchar *unparsed;
  gchar lc_char;
  const GVariantType *type;
  GVariant *minimum, *maximum;
  GVariant *default_value;
} GSettingsKeyInfo;
static inline void endian_fixup(GVariant **value) {
#if G_BYTE_ORDER == G_BIG_ENDIAN
  GVariant *tmp;
  tmp = g_variant_byteswap(*value);
  g_variant_unref(*value);
  *value = tmp;
#endif
}
static void g_settings_get_key_info(GSettingsKeyInfo *info, GSettings *settings, const gchar *key) {
  GVariantIter *iter;
  GVariant *data;
  guchar code;
  memset(info, 0, sizeof *info);
  iter = g_settings_schema_get_value(settings->priv->schema, key);
  info->default_value = g_variant_iter_next_value(iter);
  endian_fixup(&info->default_value);
  info->type = g_variant_get_type(info->default_value);
  info->settings = g_object_ref(settings);
  info->key = g_intern_string(key);
  while(g_variant_iter_next(iter, "(y*)", &code, &data)) {
      switch(code) {
          case 'l': g_variant_get(data, "(y&s)", &info->lc_char, &info->unparsed); break;
          case 'e': info->is_enum = TRUE; goto choice;
          case 'f': info->is_flags = TRUE; goto choice;
  choice: case 'c': info->strinfo = g_variant_get_fixed_array(data, &info->strinfo_length, sizeof(guint32)); break;
          case 'r':
              g_variant_get(data, "(**)", &info->minimum, &info->maximum);
              endian_fixup(&info->minimum);
              endian_fixup(&info->maximum);
              break;
          default: g_warning("unknown schema extension '%c'", code);
      }
      g_variant_unref(data);
  }
  g_variant_iter_free(iter);
}
static void g_settings_free_key_info(GSettingsKeyInfo *info) {
  if (info->minimum) g_variant_unref(info->minimum);
  if (info->maximum) g_variant_unref(info->maximum);
  g_variant_unref(info->default_value);
  g_object_unref(info->settings);
}
static gboolean g_settings_write_to_backend(GSettingsKeyInfo *info, GVariant *value) {
  gboolean success;
  gchar *path;
  path = g_strconcat(info->settings->priv->path, info->key, NULL);
  success = g_settings_backend_write(info->settings->priv->backend, path, value, NULL);
  g_free(path);
  return success;
}
static gboolean g_settings_type_check(GSettingsKeyInfo *info, GVariant *value) {
  g_return_val_if_fail(value != NULL, FALSE);
  return g_variant_is_of_type(value, info->type);
}
static gboolean g_settings_key_info_range_check(GSettingsKeyInfo *info, GVariant *value) {
  if (info->minimum == NULL && info->strinfo == NULL) return TRUE;
  if (g_variant_is_container(value)) {
      gboolean ok = TRUE;
      GVariantIter iter;
      GVariant *child;
      g_variant_iter_init(&iter, value);
      while(ok && (child = g_variant_iter_next_value(&iter))) {
          ok = g_settings_key_info_range_check(info, child);
          g_variant_unref(child);
      }
      return ok;
  }
  if (info->minimum) return g_variant_compare(info->minimum, value) <= 0 && g_variant_compare(value, info->maximum) <= 0;
  return strinfo_is_string_valid(info->strinfo, info->strinfo_length,g_variant_get_string(value, NULL));
}
static GVariant *g_settings_range_fixup(GSettingsKeyInfo *info, GVariant *value) {
  const gchar *target;
  if (g_settings_key_info_range_check(info, value)) return g_variant_ref(value);
  if (info->strinfo == NULL) return NULL;
  if (g_variant_is_container(value)) {
      GVariantBuilder builder;
      GVariantIter iter;
      GVariant *child;
      g_variant_iter_init(&iter, value);
      g_variant_builder_init(&builder, g_variant_get_type(value));
      while((child = g_variant_iter_next_value(&iter))) {
          GVariant *fixed;
          fixed = g_settings_range_fixup(info, child);
          g_variant_unref(child);
          if (fixed == NULL) {
              g_variant_builder_clear(&builder);
              return NULL;
          }
          g_variant_builder_add_value(&builder, fixed);
          g_variant_unref(fixed);
      }
      return g_variant_ref_sink(g_variant_builder_end(&builder));
  }
  target = strinfo_string_from_alias(info->strinfo, info->strinfo_length,g_variant_get_string(value, NULL));
  return target ? g_variant_ref_sink(g_variant_new_string(target)) : NULL;
}
static GVariant *g_settings_read_from_backend(GSettingsKeyInfo *info) {
  GVariant *value;
  GVariant *fixup;
  gchar *path;
  path = g_strconcat(info->settings->priv->path, info->key, NULL);
  value = g_settings_backend_read(info->settings->priv->backend, path, info->type, FALSE);
  g_free(path);
  if (value != NULL) {
      fixup = g_settings_range_fixup(info, value);
      g_variant_unref(value);
  } else fixup = NULL;
  return fixup;
}
static GVariant *g_settings_get_translated_default(GSettingsKeyInfo *info) {
  const gchar *translated;
  GError *error = NULL;
  const gchar *domain;
  GVariant *value;
  if (info->lc_char == '\0') return NULL;
  domain = g_settings_schema_get_gettext_domain(info->settings->priv->schema);
  if (info->lc_char == 't') translated = g_dcgettext(domain, info->unparsed, LC_TIME);
  else translated = g_dgettext(domain, info->unparsed);
  if (translated == info->unparsed) return NULL;
  value = g_variant_parse(info->type, translated, NULL, NULL, &error);
  if (value == NULL) {
      g_warning("Failed to parse translated string `%s' for key `%s' in schema `%s': %s", info->unparsed, info->key, info->settings->priv->schema_name,
                 error->message);
      g_warning("Using untranslated default instead.");
      g_error_free(error);
  } else if (!g_settings_key_info_range_check(info, value)) {
      g_warning("Translated default `%s' for key `%s' in schema `%s' is outside of valid range", info->unparsed, info->key, info->settings->priv->schema_name);
      g_variant_unref(value);
      value = NULL;
  }
  return value;
}
static gint g_settings_to_enum(GSettingsKeyInfo *info, GVariant *value) {
  gboolean it_worked;
  guint result;
  it_worked = strinfo_enum_from_string(info->strinfo, info->strinfo_length,g_variant_get_string(value, NULL), &result);
  g_assert(it_worked);
  return result;
}
static GVariant *g_settings_from_enum(GSettingsKeyInfo *info, gint value) {
  const gchar *string;
  string = strinfo_string_from_enum(info->strinfo, info->strinfo_length, value);
  if (string == NULL) return NULL;
  return g_variant_new_string(string);
}
static guint g_settings_to_flags(GSettingsKeyInfo *info, GVariant *value) {
  GVariantIter iter;
  const gchar *flag;
  guint result;
  result = 0;
  g_variant_iter_init(&iter, value);
  while(g_variant_iter_next(&iter,"&s", &flag)) {
      gboolean it_worked;
      guint flag_value;
      it_worked = strinfo_enum_from_string(info->strinfo, info->strinfo_length, flag, &flag_value);
      g_assert(it_worked);
      result |= flag_value;
  }
  return result;
}
static GVariant *g_settings_from_flags(GSettingsKeyInfo *info, guint value) {
  GVariantBuilder builder;
  gint i;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));
  for (i = 0; i < 32; i++)
      if (value & (1u << i)) {
          const gchar *string;
          string = strinfo_string_from_enum(info->strinfo, info->strinfo_length,1u << i);
          if (string == NULL) {
              g_variant_builder_clear(&builder);
              return NULL;
          }
          g_variant_builder_add(&builder,"s", string);
      }
  return g_variant_builder_end(&builder);
}
GVariant *g_settings_get_value(GSettings *settings, const gchar *key) {
  GSettingsKeyInfo info;
  GVariant *value;
  g_return_val_if_fail(G_IS_SETTINGS(settings), NULL);
  g_return_val_if_fail(key != NULL, NULL);
  g_settings_get_key_info(&info, settings, key);
  value = g_settings_read_from_backend(&info);
  if (value == NULL) value = g_settings_get_translated_default(&info);
  if (value == NULL) value = g_variant_ref(info.default_value);
  g_settings_free_key_info(&info);
  return value;
}
gint g_settings_get_enum(GSettings *settings, const gchar *key) {
  GSettingsKeyInfo info;
  GVariant *value;
  gint result;
  g_return_val_if_fail(G_IS_SETTINGS (settings), -1);
  g_return_val_if_fail(key != NULL, -1);
  g_settings_get_key_info (&info, settings, key);
  if (!info.is_enum) {
      g_critical("g_settings_get_enum() called on key `%s' which is not associated with an enumerated type", info.key);
      g_settings_free_key_info(&info);
      return -1;
  }
  value = g_settings_read_from_backend(&info);
  if (value == NULL) value = g_settings_get_translated_default(&info);
  if (value == NULL) value = g_variant_ref(info.default_value);
  result = g_settings_to_enum(&info, value);
  g_settings_free_key_info(&info);
  g_variant_unref(value);
  return result;
}
gboolean g_settings_set_enum(GSettings *settings, const gchar *key, gint value) {
  GSettingsKeyInfo info;
  GVariant *variant;
  gboolean success;
  g_return_val_if_fail(G_IS_SETTINGS(settings), FALSE);
  g_return_val_if_fail(key != NULL, FALSE);
  g_settings_get_key_info (&info, settings, key);
  if (!info.is_enum) {
      g_critical("g_settings_set_enum() called on key `%s' which is not associated with an enumerated type", info.key);
      return FALSE;
  }
  if (!(variant = g_settings_from_enum(&info, value))) {
      g_critical("g_settings_set_enum(): invalid enum value %d for key `%s' in schema `%s'.  Doing nothing.", value, info.key,
                 info.settings->priv->schema_name);
      g_settings_free_key_info(&info);
      return FALSE;
  }
  success = g_settings_write_to_backend(&info, variant);
  g_settings_free_key_info(&info);
  return success;
}
guint g_settings_get_flags(GSettings *settings, const gchar *key) {
  GSettingsKeyInfo info;
  GVariant *value;
  guint result;
  g_return_val_if_fail(G_IS_SETTINGS(settings), -1);
  g_return_val_if_fail(key != NULL, -1);
  g_settings_get_key_info(&info, settings, key);
  if (!info.is_flags) {
      g_critical("g_settings_get_flags() called on key `%s' which is not associated with a flags type", info.key);
      g_settings_free_key_info(&info);
      return -1;
  }
  value = g_settings_read_from_backend(&info);
  if (value == NULL) value = g_settings_get_translated_default(&info);
  if (value == NULL) value = g_variant_ref(info.default_value);
  result = g_settings_to_flags(&info, value);
  g_settings_free_key_info(&info);
  g_variant_unref(value);
  return result;
}
gboolean g_settings_set_flags(GSettings *settings, const gchar *key, guint value) {
  GSettingsKeyInfo info;
  GVariant *variant;
  gboolean success;
  g_return_val_if_fail(G_IS_SETTINGS(settings), FALSE);
  g_return_val_if_fail(key != NULL, FALSE);
  g_settings_get_key_info(&info, settings, key);
  if (!info.is_flags) {
      g_critical("g_settings_set_flags() called on key `%s' which is not associated with a flags type", info.key);
      return FALSE;
  }
  if (!(variant = g_settings_from_flags(&info, value))) {
      g_critical("g_settings_set_flags(): invalid flags value 0x%08x for key `%s' in schema `%s'.  Doing nothing.", value, info.key,
                 info.settings->priv->schema_name);
      g_settings_free_key_info(&info);
      return FALSE;
  }
  success = g_settings_write_to_backend(&info, variant);
  g_settings_free_key_info(&info);
  return success;
}
gboolean g_settings_set_value(GSettings *settings, const gchar *key, GVariant *value) {
  GSettingsKeyInfo info;
  g_return_val_if_fail(G_IS_SETTINGS(settings), FALSE);
  g_return_val_if_fail(key != NULL, FALSE);
  g_settings_get_key_info(&info, settings, key);
  if (!g_settings_type_check(&info, value)) {
      g_critical("g_settings_set_value: key '%s' in '%s' expects type '%s', but a GVariant of type '%s' was given", key, settings->priv->schema_name,
                 g_variant_type_peek_string(info.type), g_variant_get_type_string(value));
        return FALSE;
  }
  if (!g_settings_key_info_range_check(&info, value)) {
      g_warning("g_settings_set_value: value for key '%s' in schema '%s' is outside of valid range", key, settings->priv->schema_name);
      return FALSE;
  }
  g_settings_free_key_info(&info);
  return g_settings_write_to_backend(&info, value);
}
void g_settings_get(GSettings *settings, const gchar *key, const gchar *format, ...) {
  GVariant *value;
  va_list ap;
  value = g_settings_get_value(settings, key);
  va_start(ap, format);
  g_variant_get_va(value, format, NULL, &ap);
  va_end(ap);
  g_variant_unref(value);
}
gboolean g_settings_set(GSettings *settings, const gchar *key, const gchar *format, ...) {
  GVariant *value;
  va_list ap;
  va_start(ap, format);
  value = g_variant_new_va(format, NULL, &ap);
  va_end(ap);
  return g_settings_set_value(settings, key, value);
}
gpointer g_settings_get_mapped(GSettings *settings, const gchar *key, GSettingsGetMapping mapping, gpointer user_data) {
  gpointer result = NULL;
  GSettingsKeyInfo info;
  GVariant *value;
  gboolean okay;
  g_return_val_if_fail(G_IS_SETTINGS(settings), NULL);
  g_return_val_if_fail(key != NULL, NULL);
  g_return_val_if_fail(mapping != NULL, NULL);
  g_settings_get_key_info(&info, settings, key);
  if ((value = g_settings_read_from_backend(&info))) {
      okay = mapping(value, &result, user_data);
      g_variant_unref(value);
      if (okay) goto okay;
  }
  if ((value = g_settings_get_translated_default(&info))) {
      okay = mapping(value, &result, user_data);
      g_variant_unref(value);
      if (okay) goto okay;
  }
  if (mapping(info.default_value, &result, user_data)) goto okay;
  if (!mapping(NULL, &result, user_data)) {
      g_error("The mapping function given to g_settings_get_mapped() for key `%s' in schema `%s' returned FALSE when given a NULL value.", key,
              settings->priv->schema_name);
  }
okay:
  g_settings_free_key_info(&info);
  return result;
}
gchar *g_settings_get_string(GSettings *settings, const gchar *key) {
  GVariant *value;
  gchar *result;
  value = g_settings_get_value(settings, key);
  result = g_variant_dup_string(value, NULL);
  g_variant_unref(value);
  return result;
}
gboolean g_settings_set_string(GSettings *settings, const gchar *key, const gchar *value) {
  return g_settings_set_value(settings, key, g_variant_new_string(value));
}
gint g_settings_get_int(GSettings *settings, const gchar *key) {
  GVariant *value;
  gint result;
  value = g_settings_get_value(settings, key);
  result = g_variant_get_int32(value);
  g_variant_unref(value);
  return result;
}
gboolean g_settings_set_int(GSettings   *settings, const gchar *key, gint value) {
  return g_settings_set_value(settings, key, g_variant_new_int32(value));
}
gdouble g_settings_get_double(GSettings *settings, const gchar *key) {
  GVariant *value;
  gdouble result;
  value = g_settings_get_value(settings, key);
  result = g_variant_get_double(value);
  g_variant_unref(value);
  return result;
}
gboolean g_settings_set_double(GSettings *settings, const gchar *key, gdouble value) {
  return g_settings_set_value(settings, key, g_variant_new_double (value));
}
gboolean g_settings_get_boolean(GSettings *settings, const gchar *key) {
  GVariant *value;
  gboolean result;
  value = g_settings_get_value(settings, key);
  result = g_variant_get_boolean(value);
  g_variant_unref(value);
  return result;
}
gboolean g_settings_set_boolean(GSettings *settings, const gchar *key, gboolean value) {
  return g_settings_set_value(settings, key, g_variant_new_boolean (value));
}
gchar **g_settings_get_strv(GSettings *settings, const gchar *key) {
  GVariant *value;
  gchar **result;
  value = g_settings_get_value(settings, key);
  result = g_variant_dup_strv(value, NULL);
  g_variant_unref(value);
  return result;
}
gboolean g_settings_set_strv(GSettings *settings, const gchar *key, const gchar * const *value) {
  GVariant *array;
  if (value != NULL) array = g_variant_new_strv(value, -1);
  else array = g_variant_new_strv(NULL, 0);
  return g_settings_set_value(settings, key, array);
}
void g_settings_delay(GSettings *settings) {
  g_return_if_fail(G_IS_SETTINGS(settings));
  if (settings->priv->delayed) return;
  settings->priv->delayed = g_delayed_settings_backend_new(settings->priv->backend, settings, settings->priv->main_context);
  g_settings_backend_unwatch(settings->priv->backend, G_OBJECT(settings));
  g_object_unref(settings->priv->backend);
  settings->priv->backend = G_SETTINGS_BACKEND(settings->priv->delayed);
  g_settings_backend_watch(settings->priv->backend, &listener_vtable, G_OBJECT(settings), settings->priv->main_context);
  g_object_notify(G_OBJECT(settings), "delay-apply");
}
void g_settings_apply(GSettings *settings) {
  if (settings->priv->delayed) {
      GDelayedSettingsBackend *delayed;
      delayed = G_DELAYED_SETTINGS_BACKEND(settings->priv->backend);
      g_delayed_settings_backend_apply(delayed);
  }
}
void g_settings_revert(GSettings *settings) {
  if (settings->priv->delayed) {
      GDelayedSettingsBackend *delayed;
      delayed = G_DELAYED_SETTINGS_BACKEND(settings->priv->backend);
      g_delayed_settings_backend_revert(delayed);
  }
}
gboolean g_settings_get_has_unapplied(GSettings *settings) {
  g_return_val_if_fail(G_IS_SETTINGS(settings), FALSE);
  return settings->priv->delayed && g_delayed_settings_backend_get_has_unapplied(G_DELAYED_SETTINGS_BACKEND(settings->priv->backend));
}
void g_settings_reset(GSettings *settings, const gchar *key) {
  gchar *path;
  path = g_strconcat(settings->priv->path, key, NULL);
  g_settings_backend_reset(settings->priv->backend, path, NULL);
  g_free(path);
}
void g_settings_sync(void) {
  g_settings_backend_sync_default();
}
gboolean g_settings_is_writable(GSettings *settings, const gchar *name) {
  gboolean writable;
  gchar *path;
  g_return_val_if_fail(G_IS_SETTINGS(settings), FALSE);
  path = g_strconcat(settings->priv->path, name, NULL);
  writable = g_settings_backend_get_writable(settings->priv->backend, path);
  g_free(path);
  return writable;
}
GSettings *g_settings_get_child(GSettings *settings, const gchar *name) {
  const gchar *child_schema;
  gchar *child_path;
  gchar *child_name;
  GSettings *child;
  g_return_val_if_fail(G_IS_SETTINGS(settings), NULL);
  child_name = g_strconcat(name, "/", NULL);
  child_schema = g_settings_schema_get_string (settings->priv->schema, child_name);
  if (child_schema == NULL) g_error("Schema '%s' has no child '%s'", settings->priv->schema_name, name);
  child_path = g_strconcat(settings->priv->path, child_name, NULL);
  child = g_object_new(G_TYPE_SETTINGS, "schema", child_schema, "path", child_path, NULL);
  g_free(child_path);
  g_free(child_name);
  return child;
}
gchar **g_settings_list_keys(GSettings *settings) {
  const GQuark *keys;
  gchar **strv;
  gint n_keys;
  gint i, j;
  keys = g_settings_schema_list(settings->priv->schema, &n_keys);
  strv = g_new(gchar *, n_keys + 1);
  for (i = j = 0; i < n_keys; i++) {
      const gchar *key = g_quark_to_string(keys[i]);
      if (!g_str_has_suffix (key, "/")) strv[j++] = g_strdup(key);
  }
  strv[j] = NULL;
  return strv;
}
gchar **g_settings_list_children(GSettings *settings) {
  const GQuark *keys;
  gchar **strv;
  gint n_keys;
  gint i, j;
  keys = g_settings_schema_list(settings->priv->schema, &n_keys);
  strv = g_new(gchar*, n_keys + 1);
  for (i = j = 0; i < n_keys; i++) {
      const gchar *key = g_quark_to_string(keys[i]);
      if (g_str_has_suffix(key, "/")) {
          gint length = strlen(key);
          strv[j] = g_memdup(key, length);
          strv[j][length - 1] = '\0';
          j++;
      }
  }
  strv[j] = NULL;
  return strv;
}
GVariant *g_settings_get_range(GSettings *settings, const gchar *key) {
  GSettingsKeyInfo info;
  const gchar *type;
  GVariant *range;
  g_settings_get_key_info(&info, settings, key);
  if (info.minimum) {
      range = g_variant_new("(**)", info.minimum, info.maximum);
      type = "range";
  } else if (info.strinfo) {
      range = strinfo_enumerate(info.strinfo, info.strinfo_length);
      type = info.is_flags ? "flags" : "enum";
  } else {
      range = g_variant_new_array(info.type, NULL, 0);
      type = "type";
  }
  g_settings_free_key_info(&info);
  return g_variant_ref_sink(g_variant_new("(sv)", type, range));
}
gboolean g_settings_range_check(GSettings *settings, const gchar *key, GVariant *value) {
  GSettingsKeyInfo info;
  gboolean good;
  g_settings_get_key_info(&info, settings, key);
  good = g_settings_type_check(&info, value) && g_settings_key_info_range_check(&info, value);
  g_settings_free_key_info(&info);
  return good;
}
typedef struct {
  GSettingsKeyInfo info;
  GObject *object;
  GSettingsBindGetMapping get_mapping;
  GSettingsBindSetMapping set_mapping;
  gpointer user_data;
  GDestroyNotify destroy;
  guint writable_handler_id;
  guint property_handler_id;
  const GParamSpec *property;
  guint key_handler_id;
  gboolean running;
} GSettingsBinding;
static void g_settings_binding_free(gpointer data) {
  GSettingsBinding *binding = data;
  g_assert(!binding->running);
  if (binding->writable_handler_id) g_signal_handler_disconnect(binding->info.settings, binding->writable_handler_id);
  if (binding->key_handler_id) g_signal_handler_disconnect(binding->info.settings, binding->key_handler_id);
  if (g_signal_handler_is_connected(binding->object, binding->property_handler_id))
      g_signal_handler_disconnect(binding->object, binding->property_handler_id);
  g_settings_free_key_info(&binding->info);
  if (binding->destroy) binding->destroy(binding->user_data);
  g_slice_free(GSettingsBinding, binding);
}
static GQuark g_settings_binding_quark(const char *property) {
  GQuark quark;
  gchar *tmp;
  tmp = g_strdup_printf("gsettingsbinding-%s", property);
  quark = g_quark_from_string(tmp);
  g_free(tmp);
  return quark;
}
static void g_settings_binding_key_changed(GSettings *settings, const gchar *key, gpointer user_data) {
  GSettingsBinding *binding = user_data;
  GValue value = { 0, };
  GVariant *variant;
  g_assert(settings == binding->info.settings);
  g_assert(key == binding->info.key);
  if (binding->running) return;
  binding->running = TRUE;
  g_value_init(&value, binding->property->value_type);
  variant = g_settings_read_from_backend(&binding->info);
  if (variant && !binding->get_mapping(&value, variant, binding->user_data)) {
      g_variant_unref(variant);
      variant = NULL;
  }
  if (variant == NULL) {
      variant = g_settings_get_translated_default(&binding->info);
      if (variant && !binding->get_mapping(&value, variant, binding->user_data)) {
          g_warning("Translated default `%s' for key `%s' in schema `%s' was rejected by the binding mapping function", binding->info.unparsed,
                    binding->info.key, binding->info.settings->priv->schema_name);
          g_variant_unref(variant);
          variant = NULL;
      }
  }
  if (variant == NULL) {
      variant = g_variant_ref(binding->info.default_value);
      if (!binding->get_mapping (&value, variant, binding->user_data)) {
          g_error("The schema default value for key `%s' in schema `%s' s rejected by the binding mapping function.", binding->info.key,
                  binding->info.settings->priv->schema_name);
      }
  }
  g_object_set_property(binding->object, binding->property->name, &value);
  g_variant_unref(variant);
  g_value_unset(&value);
  binding->running = FALSE;
}
static void g_settings_binding_property_changed(GObject *object, const GParamSpec *pspec, gpointer user_data) {
  GSettingsBinding *binding = user_data;
  GValue value = { 0, };
  GVariant *variant;
  g_assert(object == binding->object);
  g_assert(pspec == binding->property);
  if (binding->running) return;
  binding->running = TRUE;
  g_value_init(&value, pspec->value_type);
  g_object_get_property(object, pspec->name, &value);
  if ((variant = binding->set_mapping(&value, binding->info.type, binding->user_data))) {
      if (g_variant_is_floating(variant)) g_variant_ref_sink(variant);
      if (!g_settings_type_check(&binding->info, variant)) {
          g_critical("binding mapping function for key `%s' returned GVariant of type `%s' when type `%s' was requested", binding->info.key,
                     g_variant_get_type_string(variant), g_variant_type_dup_string(binding->info.type));
          return;
      }
      if (!g_settings_key_info_range_check(&binding->info, variant)) {
          g_critical("GObject property `%s' on a `%s' object is out of schema-specified range for key `%s' of `%s': %s", binding->property->name,
                     g_type_name(binding->property->owner_type), binding->info.key, binding->info.settings->priv->schema_name, g_variant_print(variant, TRUE));
          return;
      }
      g_settings_write_to_backend(&binding->info, variant);
      g_variant_unref(variant);
  }
  g_value_unset(&value);
  binding->running = FALSE;
}
static gboolean g_settings_bind_invert_boolean_get_mapping(GValue *value, GVariant *variant, gpointer user_data) {
  g_value_set_boolean(value, !g_variant_get_boolean(variant));
  return TRUE;
}
static GVariant *g_settings_bind_invert_boolean_set_mapping(const GValue *value, const GVariantType *expected_type, gpointer user_data) {
  return g_variant_new_boolean(!g_value_get_boolean(value));
}
void g_settings_bind(GSettings *settings, const gchar *key, gpointer object, const gchar *property, GSettingsBindFlags flags) {
  GSettingsBindGetMapping get_mapping = NULL;
  GSettingsBindSetMapping set_mapping = NULL;
  if (flags & G_SETTINGS_BIND_INVERT_BOOLEAN) {
      get_mapping = g_settings_bind_invert_boolean_get_mapping;
      set_mapping = g_settings_bind_invert_boolean_set_mapping;
      flags &= ~G_SETTINGS_BIND_INVERT_BOOLEAN;
  }
  g_settings_bind_with_mapping(settings, key, object, property, flags, get_mapping, set_mapping, NULL, NULL);
}
void g_settings_bind_with_mapping(GSettings *settings, const gchar *key, gpointer object, const gchar *property, GSettingsBindFlags flags,
                                  GSettingsBindGetMapping get_mapping, GSettingsBindSetMapping set_mapping, gpointer user_data, GDestroyNotify destroy) {
  GSettingsBinding *binding;
  GObjectClass *objectclass;
  gchar *detailed_signal;
  GQuark binding_quark;
  g_return_if_fail(G_IS_SETTINGS(settings));
  g_return_if_fail(~flags & G_SETTINGS_BIND_INVERT_BOOLEAN);
  objectclass = G_OBJECT_GET_CLASS(object);
  binding = g_slice_new0(GSettingsBinding);
  g_settings_get_key_info(&binding->info, settings, key);
  binding->object = object;
  binding->property = g_object_class_find_property(objectclass, property);
  binding->user_data = user_data;
  binding->destroy = destroy;
  binding->get_mapping = get_mapping ? get_mapping : g_settings_get_mapping;
  binding->set_mapping = set_mapping ? set_mapping : g_settings_set_mapping;
  if (!(flags & (G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET))) flags |= G_SETTINGS_BIND_GET | G_SETTINGS_BIND_SET;
  if (binding->property == NULL) {
      g_critical("g_settings_bind: no property '%s' on class '%s'", property, G_OBJECT_TYPE_NAME(object));
      return;
  }
  if ((flags & G_SETTINGS_BIND_GET) && (binding->property->flags & G_PARAM_WRITABLE) == 0) {
      g_critical("g_settings_bind: property '%s' on class '%s' is not writable", property, G_OBJECT_TYPE_NAME(object));
      return;
  }
  if ((flags & G_SETTINGS_BIND_SET) && (binding->property->flags & G_PARAM_READABLE) == 0) {
      g_critical("g_settings_bind: property '%s' on class '%s' is not readable", property, G_OBJECT_TYPE_NAME(object));
      return;
  }
  if (get_mapping == g_settings_bind_invert_boolean_get_mapping) {
      if (binding->property->value_type != G_TYPE_BOOLEAN) {
          g_critical("g_settings_bind: G_SETTINGS_BIND_INVERT_BOOLEAN was specified, but property `%s' on type `%s' has type `%s'", property,
                     G_OBJECT_TYPE_NAME(object), g_type_name((binding->property->value_type)));
          return;
      }
      if (!g_variant_type_equal(binding->info.type, G_VARIANT_TYPE_BOOLEAN)) {
          g_critical("g_settings_bind: G_SETTINGS_BIND_INVERT_BOOLEAN was specified, but key `%s' on schema `%s' has type `%s'", key, settings->priv->schema_name,
                     g_variant_type_dup_string(binding->info.type));
          return;
      }
  } else if (((get_mapping == NULL && (flags & G_SETTINGS_BIND_GET)) || (set_mapping == NULL && (flags & G_SETTINGS_BIND_SET))) &&
             !g_settings_mapping_is_compatible(binding->property->value_type, binding->info.type)) {
      g_critical("g_settings_bind: property '%s' on class '%s' has type '%s' which is not compatible with type '%s' of key '%s' on schema '%s'",
                 property, G_OBJECT_TYPE_NAME(object), g_type_name(binding->property->value_type), g_variant_type_dup_string(binding->info.type), key,
                 settings->priv->schema_name);
      return;
  }
  if ((flags & G_SETTINGS_BIND_SET) && (~flags & G_SETTINGS_BIND_NO_SENSITIVITY)) {
      GParamSpec *sensitive;
      sensitive = g_object_class_find_property(objectclass, "sensitive");
      if (sensitive && sensitive->value_type == G_TYPE_BOOLEAN && (sensitive->flags & G_PARAM_WRITABLE))
          g_settings_bind_writable(settings, binding->info.key, object, "sensitive", FALSE);
  }
  if (flags & G_SETTINGS_BIND_SET) {
      detailed_signal = g_strdup_printf("notify::%s", property);
      binding->property_handler_id = g_signal_connect(object, detailed_signal,G_CALLBACK (g_settings_binding_property_changed), binding);
      g_free(detailed_signal);
      if (~flags & G_SETTINGS_BIND_GET) g_settings_binding_property_changed(object, binding->property, binding);
  }
  if (flags & G_SETTINGS_BIND_GET) {
      if (~flags & G_SETTINGS_BIND_GET_NO_CHANGES) {
          detailed_signal = g_strdup_printf("changed::%s", key);
          binding->key_handler_id = g_signal_connect(settings, detailed_signal, G_CALLBACK(g_settings_binding_key_changed), binding);
          g_free(detailed_signal);
      }
      g_settings_binding_key_changed(settings, binding->info.key, binding);
  }
  binding_quark = g_settings_binding_quark(property);
  g_object_set_qdata_full(object, binding_quark, binding, g_settings_binding_free);
}
typedef struct {
  GSettings *settings;
  gpointer object;
  const gchar *key;
  const gchar *property;
  gboolean inverted;
  gulong handler_id;
} GSettingsWritableBinding;
static void g_settings_writable_binding_free(gpointer data) {
  GSettingsWritableBinding *binding = data;
  g_signal_handler_disconnect(binding->settings, binding->handler_id);
  g_object_unref(binding->settings);
  g_slice_free(GSettingsWritableBinding, binding);
}
static void g_settings_binding_writable_changed(GSettings *settings, const gchar *key, gpointer user_data) {
  GSettingsWritableBinding *binding = user_data;
  gboolean writable;
  g_assert(settings == binding->settings);
  g_assert(key == binding->key);
  writable = g_settings_is_writable(settings, key);
  if (binding->inverted) writable = !writable;
  g_object_set(binding->object, binding->property, writable, NULL);
}
void g_settings_bind_writable(GSettings *settings, const gchar *key, gpointer object, const gchar *property, gboolean inverted) {
  GSettingsWritableBinding *binding;
  gchar *detailed_signal;
  GParamSpec *pspec;
  g_return_if_fail(G_IS_SETTINGS(settings));
  pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(object), property);
  if (pspec == NULL) {
      g_critical("g_settings_bind_writable: no property '%s' on class '%s'", property, G_OBJECT_TYPE_NAME(object));
      return;
  }
  if ((pspec->flags & G_PARAM_WRITABLE) == 0) {
      g_critical("g_settings_bind_writable: property '%s' on class '%s' is not writable", property, G_OBJECT_TYPE_NAME(object));
      return;
  }
  binding = g_slice_new(GSettingsWritableBinding);
  binding->settings = g_object_ref(settings);
  binding->object = object;
  binding->key = g_intern_string(key);
  binding->property = g_intern_string(property);
  binding->inverted = inverted;
  detailed_signal = g_strdup_printf("writable-changed::%s", key);
  binding->handler_id = g_signal_connect(settings, detailed_signal,G_CALLBACK(g_settings_binding_writable_changed), binding);
  g_free(detailed_signal);
  g_object_set_qdata_full(object, g_settings_binding_quark(property), binding, g_settings_writable_binding_free);
  g_settings_binding_writable_changed(settings, binding->key, binding);
}
void g_settings_unbind(gpointer object, const gchar *property) {
  GQuark binding_quark;
  binding_quark = g_settings_binding_quark(property);
  g_object_set_qdata(object, binding_quark, NULL);
}