#ifndef __G_DELAYED_SETTINGS_BACKEND_H__
#define __G_DELAYED_SETTINGS_BACKEND_H__

#include "../glib/glib-object.h"
#include "../glib/glib.h"
#include "../gobject/gtype.h"
#include "gsettingsbackend.h"
#include "giotypes.h"

#define G_TYPE_DELAYED_SETTINGS_BACKEND  (g_delayed_settings_backend_get_type ())
#define G_DELAYED_SETTINGS_BACKEND(inst)  (G_TYPE_CHECK_INSTANCE_CAST ((inst), G_TYPE_DELAYED_SETTINGS_BACKEND, GDelayedSettingsBackend))
#define G_DELAYED_SETTINGS_BACKEND_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST ((class), G_TYPE_DELAYED_SETTINGS_BACKEND, GDelayedSettingsBackendClass))
#define G_IS_DELAYED_SETTINGS_BACKEND(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), G_TYPE_DELAYED_SETTINGS_BACKEND))
#define G_IS_DELAYED_SETTINGS_BACKEND_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), G_TYPE_DELAYED_SETTINGS_BACKEND))
#define G_DELAYED_SETTINGS_BACKEND_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), G_TYPE_DELAYED_SETTINGS_BACKEND, GDelayedSettingsBackendClass))
typedef struct _GDelayedSettingsBackendPrivate GDelayedSettingsBackendPrivate;
typedef struct _GDelayedSettingsBackendClass GDelayedSettingsBackendClass;
typedef struct _GDelayedSettingsBackend GDelayedSettingsBackend;
struct _GDelayedSettingsBackendClass {
  GSettingsBackendClass parent_class;
};
struct _GDelayedSettingsBackend {
  GSettingsBackend parent_instance;
  GDelayedSettingsBackendPrivate *priv;
};
G_GNUC_INTERNAL GType g_delayed_settings_backend_get_type(void);
G_GNUC_INTERNAL GDelayedSettingsBackend *g_delayed_settings_backend_new(GSettingsBackend *backend, gpointer owner, GMainContext *owner_context);
G_GNUC_INTERNAL void g_delayed_settings_backend_revert(GDelayedSettingsBackend *delayed);
G_GNUC_INTERNAL void g_delayed_settings_backend_apply(GDelayedSettingsBackend *delayed);
G_GNUC_INTERNAL gboolean g_delayed_settings_backend_get_has_unapplied(GDelayedSettingsBackend *delayed);

#endif