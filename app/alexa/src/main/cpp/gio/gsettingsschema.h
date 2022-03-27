#ifndef __G_SETTINGS_SCHEMA_H__
#define __G_SETTINGS_SCHEMA_H__

#include "../glib/glib-object.h"
#include "../glib/gvariant.h"
#include "../gobject/gobject.h"

G_BEGIN_DECLS
#define G_TYPE_SETTINGS_SCHEMA  (g_settings_schema_get_type ())
#define G_SETTINGS_SCHEMA(inst)  (G_TYPE_CHECK_INSTANCE_CAST ((inst),  G_TYPE_SETTINGS_SCHEMA, GSettingsSchema))
#define G_SETTINGS_SCHEMA_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST ((class), G_TYPE_SETTINGS_SCHEMA, GSettingsSchemaClass))
#define G_IS_SETTINGS_SCHEMA(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), G_TYPE_SETTINGS_SCHEMA))
#define G_IS_SETTINGS_SCHEMA_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), G_TYPE_SETTINGS_SCHEMA))
#define G_SETTINGS_SCHEMA_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), G_TYPE_SETTINGS_SCHEMA, GSettingsSchemaClass))
typedef struct _GSettingsSchemaPrivate GSettingsSchemaPrivate;
typedef struct _GSettingsSchemaClass GSettingsSchemaClass;
typedef struct _GSettingsSchema GSettingsSchema;
struct _GSettingsSchemaClass {
  GObjectClass parent_class;
};
struct _GSettingsSchema {
  GObject parent_instance;
  GSettingsSchemaPrivate *priv;
};
G_GNUC_INTERNAL GType g_settings_schema_get_type(void);
G_GNUC_INTERNAL GSettingsSchema *g_settings_schema_new(const gchar *name);
G_GNUC_INTERNAL const gchar *g_settings_schema_get_path(GSettingsSchema *schema);
G_GNUC_INTERNAL
const gchar *g_settings_schema_get_gettext_domain(GSettingsSchema *schema);
G_GNUC_INTERNAL GVariantIter *g_settings_schema_get_value(GSettingsSchema *schema, const gchar *key);
G_GNUC_INTERNAL gboolean g_settings_schema_has_key(GSettingsSchema *schema, const gchar *key);
G_GNUC_INTERNAL const GQuark *g_settings_schema_list(GSettingsSchema *schema, gint *n_items);
G_GNUC_INTERNAL const gchar *g_settings_schema_get_string(GSettingsSchema *schema, const gchar *key);
G_END_DECLS

#endif