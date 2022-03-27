#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_SETTINGS_H__
#define __G_SETTINGS_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_SETTINGS  (g_settings_get_type())
#define G_SETTINGS(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_SETTINGS, GSettings))
#define G_SETTINGS_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_SETTINGS, GSettingsClass))
#define G_IS_SETTINGS(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_SETTINGS))
#define G_IS_SETTINGS_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_SETTINGS))
#define G_SETTINGS_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_SETTINGS, GSettingsClass))
typedef struct _GSettingsPrivate GSettingsPrivate;
typedef struct _GSettingsClass GSettingsClass;
struct _GSettingsClass {
  GObjectClass parent_class;
  void (*writable_changed)(GSettings *settings, const gchar *key);
  void (*changed)(GSettings *settings, const gchar *key);
  gboolean (*writable_change_event)(GSettings *settings, GQuark key);
  gboolean (*change_event)(GSettings *settings, const GQuark *keys, gint n_keys);
  gpointer padding[20];
};
struct _GSettings {
  GObject parent_instance;
  GSettingsPrivate *priv;
};
GType g_settings_get_type(void);
const gchar * const *g_settings_list_schemas(void);
const gchar * const *g_settings_list_relocatable_schemas(void);
GSettings *g_settings_new(const gchar *schema);
GSettings *g_settings_new_with_path(const gchar *schema, const gchar *path);
GSettings *g_settings_new_with_backend(const gchar *schema, GSettingsBackend *backend);
GSettings *g_settings_new_with_backend_and_path(const gchar *schema, GSettingsBackend *backend, const gchar *path);
gchar **g_settings_list_children(GSettings *settings);
gchar **g_settings_list_keys(GSettings *settings);
GVariant *g_settings_get_range(GSettings *settings, const gchar *key);
gboolean g_settings_range_check(GSettings *settings, const gchar *key, GVariant *value);
gboolean g_settings_set_value(GSettings *settings, const gchar *key, GVariant *value);
GVariant *g_settings_get_value(GSettings *settings, const gchar *key);
gboolean g_settings_set(GSettings *settings, const gchar *key, const gchar *format, ...);
void g_settings_get(GSettings *settings, const gchar *key, const gchar *format, ...);
void g_settings_reset (GSettings *settings, const gchar *key);
gint g_settings_get_int(GSettings *settings, const gchar *key);
gboolean g_settings_set_int(GSettings *settings, const gchar *key, gint value);
gchar *g_settings_get_string(GSettings *settings, const gchar *key);
gboolean g_settings_set_string(GSettings *settings, const gchar *key, const gchar *value);
gboolean g_settings_get_boolean(GSettings *settings, const gchar *key);
gboolean g_settings_set_boolean(GSettings *settings, const gchar *key, gboolean value);
gdouble g_settings_get_double(GSettings *settings, const gchar *key);
gboolean g_settings_set_double(GSettings *settings, const gchar *key, gdouble value);
gchar **g_settings_get_strv(GSettings *settings, const gchar *key);
gboolean g_settings_set_strv(GSettings *settings, const gchar *key, const gchar *const *value);
gint g_settings_get_enum(GSettings *settings, const gchar *key);
gboolean g_settings_set_enum(GSettings *settings, const gchar *key, gint value);
guint g_settings_get_flags(GSettings *settings, const gchar *key);
gboolean g_settings_set_flags(GSettings *settings, const gchar *key, guint value);
GSettings *g_settings_get_child(GSettings *settings, const gchar *name);
gboolean g_settings_is_writable(GSettings *settings, const gchar *name);
void g_settings_delay(GSettings *settings);
void g_settings_apply(GSettings *settings);
void g_settings_revert(GSettings *settings);
gboolean g_settings_get_has_unapplied(GSettings *settings);
void g_settings_sync(void);
typedef GVariant *(*GSettingsBindSetMapping)(const GValue *value, const GVariantType *expected_type, gpointer user_data);
typedef gboolean (*GSettingsBindGetMapping)(GValue *value, GVariant *variant, gpointer user_data);
typedef gboolean (*GSettingsGetMapping)(GVariant *value, gpointer *result, gpointer user_data);
typedef enum {
  G_SETTINGS_BIND_DEFAULT,
  G_SETTINGS_BIND_GET = (1<<0),
  G_SETTINGS_BIND_SET = (1<<1),
  G_SETTINGS_BIND_NO_SENSITIVITY = (1<<2),
  G_SETTINGS_BIND_GET_NO_CHANGES = (1<<3),
  G_SETTINGS_BIND_INVERT_BOOLEAN = (1<<4)
} GSettingsBindFlags;
void g_settings_bind(GSettings *settings, const gchar *key, gpointer object, const gchar *property, GSettingsBindFlags flags);
void g_settings_bind_with_mapping(GSettings *settings, const gchar *key, gpointer object, const gchar *property, GSettingsBindFlags flags,
                                  GSettingsBindGetMapping get_mapping, GSettingsBindSetMapping set_mapping, gpointer user_data, GDestroyNotify destroy);
void g_settings_bind_writable(GSettings *settings, const gchar *key, gpointer object, const gchar *property, gboolean inverted);
void g_settings_unbind(gpointer object, const gchar *property);
gpointer g_settings_get_mapped(GSettings *settings, const gchar *key, GSettingsGetMapping mapping, gpointer user_data);
G_END_DECLS

#endif