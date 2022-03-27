#ifndef __G_SETTINGS_MAPPING_H__
#define __G_SETTINGS_MAPPING_H__

#include "../gobject/gobject.h"
#include "../gobject/gvaluetypes.h"
#include "../glib/glib-object.h"

G_GNUC_INTERNAL GVariant *g_settings_set_mapping(const GValue *value, const GVariantType *expected_type, gpointer user_data);
G_GNUC_INTERNAL gboolean g_settings_get_mapping(GValue *value, GVariant *variant, gpointer user_data);
G_GNUC_INTERNAL gboolean g_settings_mapping_is_compatible(GType gvalue_type, const GVariantType *variant_type);

#endif