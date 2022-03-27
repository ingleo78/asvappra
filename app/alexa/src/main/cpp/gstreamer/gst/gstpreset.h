#ifndef __GST_PRESET_H__
#define __GST_PRESET_H__

#include <glib/glib-object.h>
#include "gstconfig.h"

G_BEGIN_DECLS
#define GST_TYPE_PRESET               (gst_preset_get_type())
#define GST_PRESET(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_PRESET, GstPreset))
#define GST_IS_PRESET(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_PRESET))
#define GST_PRESET_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), GST_TYPE_PRESET, GstPresetInterface))
typedef struct _GstPreset GstPreset;
typedef struct _GstPresetInterface GstPresetInterface;
struct _GstPresetInterface {
  GTypeInterface parent;
  gchar**      (*get_preset_names)    (GstPreset *preset);
  gchar**      (*get_property_names)  (GstPreset *preset);
  gboolean     (*load_preset)         (GstPreset *preset, const gchar *name);
  gboolean     (*save_preset)         (GstPreset *preset, const gchar *name);
  gboolean     (*rename_preset)       (GstPreset *preset, const gchar *old_name, const gchar *new_name);
  gboolean     (*delete_preset)       (GstPreset *preset, const gchar *name);
  gboolean     (*set_meta)            (GstPreset *preset, const gchar *name, const gchar *tag, const gchar *value);
  gboolean     (*get_meta)            (GstPreset *preset, const gchar *name, const gchar *tag, gchar **value);
  gpointer _gst_reserved[GST_PADDING];
};
GType gst_preset_get_type(void);
gchar**      gst_preset_get_preset_names   (GstPreset *preset) G_GNUC_MALLOC;
gchar**      gst_preset_get_property_names (GstPreset *preset) G_GNUC_MALLOC;
gboolean     gst_preset_load_preset        (GstPreset *preset, const gchar *name);
gboolean     gst_preset_save_preset        (GstPreset *preset, const gchar *name);
gboolean     gst_preset_rename_preset      (GstPreset *preset, const gchar *old_name, const gchar *new_name);
gboolean     gst_preset_delete_preset      (GstPreset *preset, const gchar *name);
gboolean     gst_preset_set_meta           (GstPreset *preset, const gchar *name, const gchar *tag, const gchar *value);
gboolean     gst_preset_get_meta           (GstPreset *preset, const gchar *name, const gchar *tag, gchar **value);
gboolean     gst_preset_set_app_dir        (const gchar *app_dir);
const gchar *gst_preset_get_app_dir        (void);
gboolean     gst_preset_is_editable        (GstPreset *preset);
G_END_DECLS

#endif