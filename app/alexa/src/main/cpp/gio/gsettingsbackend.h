#ifndef __G_SETTINGS_BACKEND_H__
#define __G_SETTINGS_BACKEND_H__

#if defined (G_SETTINGS_ENABLE_BACKEND) && !defined (GIO_COMPILATION)
#error "You must define G_SETTINGS_ENABLE_BACKEND before including <gio/gsettingsbackend.h>."
#endif

#define __GIO_GIO_H_INSIDE__
#include "../gobject/gobject.h"
#include "giotypes.h"
#undef __GIO_GIO_H_INSIDE__

G_BEGIN_DECLS
#define G_TYPE_SETTINGS_BACKEND  (g_settings_backend_get_type())
#define G_SETTINGS_BACKEND(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_SETTINGS_BACKEND, GSettingsBackend))
#define G_SETTINGS_BACKEND_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_SETTINGS_BACKEND, GSettingsBackendClass))
#define G_IS_SETTINGS_BACKEND(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_SETTINGS_BACKEND))
#define G_IS_SETTINGS_BACKEND_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_SETTINGS_BACKEND))
#define G_SETTINGS_BACKEND_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_SETTINGS_BACKEND, GSettingsBackendClass))
#define G_SETTINGS_BACKEND_EXTENSION_POINT_NAME "gsettings-backend"
typedef struct _GSettingsBackendPrivate GSettingsBackendPrivate;
typedef struct _GSettingsBackendClass GSettingsBackendClass;
struct _GSettingsBackendClass {
  GObjectClass parent_class;
  GVariant *(*read)(GSettingsBackend *backend, const gchar *key, const GVariantType *expected_type, gboolean default_value);
  gboolean (*get_writable)(GSettingsBackend *backend, const gchar *key);
  gboolean (*write)(GSettingsBackend *backend, const gchar *key, GVariant *value, gpointer origin_tag);
  gboolean (*write_tree)(GSettingsBackend *backend, GTree *tree, gpointer origin_tag);
  void (*reset)(GSettingsBackend *backend, const gchar *key, gpointer origin_tag);
  void (*subscribe)(GSettingsBackend *backend, const gchar *name);
  void (*unsubscribe)(GSettingsBackend *backend, const gchar *name);
  void (*sync)(GSettingsBackend *backend);
  GPermission *(*get_permission)(GSettingsBackend *backend, const gchar *path);
  gpointer padding[24];
};
struct _GSettingsBackend {
  GObject parent_instance;
  GSettingsBackendPrivate *priv;
};
GType g_settings_backend_get_type(void);
void g_settings_backend_changed(GSettingsBackend *backend, const gchar *key, gpointer origin_tag);
void g_settings_backend_path_changed(GSettingsBackend *backend, const gchar *path, gpointer origin_tag);
void g_settings_backend_flatten_tree(GTree *tree, gchar **path, const gchar ***keys, GVariant ***values);
void g_settings_backend_keys_changed(GSettingsBackend *backend, const gchar *path, gchar const * const *items, gpointer origin_tag);
void g_settings_backend_path_writable_changed(GSettingsBackend *backend, const gchar *path);
void g_settings_backend_writable_changed(GSettingsBackend *backend, const gchar *key);
void g_settings_backend_changed_tree(GSettingsBackend *backend, GTree *tree, gpointer origin_tag);
GSettingsBackend *g_settings_backend_get_default(void);
GSettingsBackend *g_keyfile_settings_backend_new(const gchar *filename, const gchar *root_path, const gchar *root_group);
GSettingsBackend *g_null_settings_backend_new(void);
GSettingsBackend *g_memory_settings_backend_new(void);
G_END_DECLS

#endif