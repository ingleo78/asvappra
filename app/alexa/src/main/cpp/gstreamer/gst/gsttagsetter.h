#ifndef __GST_TAG_SETTER_H__
#define __GST_TAG_SETTER_H__

#include <glib/glib.h>

G_BEGIN_DECLS

#define GST_TYPE_TAG_SETTER             (gst_tag_setter_get_type ())
#define GST_TAG_SETTER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_TAG_SETTER, GstTagSetter))
#define GST_IS_TAG_SETTER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_TAG_SETTER))
#define GST_TAG_SETTER_GET_INTERFACE(obj)       (G_TYPE_INSTANCE_GET_INTERFACE ((obj), GST_TYPE_TAG_SETTER, GstTagSetterInterface))
typedef struct _GstTagSetter                    GstTagSetter;
typedef struct _GstTagSetterInterface           GstTagSetterInterface;
struct _GstTagSetterInterface {
  GTypeInterface g_iface;
};
GType           gst_tag_setter_get_type             (void);
void            gst_tag_setter_reset_tags             (GstTagSetter * setter);
void            gst_tag_setter_merge_tags           (GstTagSetter *     setter, const GstTagList * list, GstTagMergeMode    mode);
void            gst_tag_setter_add_tags             (GstTagSetter *setter, GstTagMergeMode mode, const gchar *tag, ...) G_GNUC_NULL_TERMINATED;
void            gst_tag_setter_add_tag_values       (GstTagSetter *setter, GstTagMergeMode mode, const gchar *tag, ...) G_GNUC_NULL_TERMINATED;
void            gst_tag_setter_add_tag_valist       (GstTagSetter *setter, GstTagMergeMode mode, const gchar *tag, va_list var_args);
void            gst_tag_setter_add_tag_valist_values(GstTagSetter *setter, GstTagMergeMode mode, const gchar *tag, va_list var_args);
void            gst_tag_setter_add_tag_value        (GstTagSetter *setter, GstTagMergeMode mode, const gchar *tag, const GValue *value);
const GstTagList *gst_tag_setter_get_tag_list          (GstTagSetter *    setter);
void            gst_tag_setter_set_tag_merge_mode    (GstTagSetter *    setter, GstTagMergeMode   mode);
GstTagMergeMode gst_tag_setter_get_tag_merge_mode    (GstTagSetter *    setter);
G_END_DECLS

#endif