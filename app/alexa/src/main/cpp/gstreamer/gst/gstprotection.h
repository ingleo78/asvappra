#ifndef __GST_PROTECTION_H__
#define __GST_PROTECTION_H__

#include <glib/glib.h>

G_BEGIN_DECLS
#define GST_PROTECTION_SYSTEM_ID_CAPS_FIELD "protection-system"
typedef struct _GstProtectionMeta GstProtectionMeta;
struct _GstProtectionMeta {
  GstMeta meta;
  GstStructure *info;
};
GType gst_protection_meta_api_get_type (void);
#define GST_PROTECTION_META_API_TYPE (gst_protection_meta_api_get_type())
#define gst_buffer_get_protection_meta(b)  ((GstProtectionMeta*)gst_buffer_get_meta ((b), GST_PROTECTION_META_API_TYPE))
#define GST_PROTECTION_META_INFO (gst_protection_meta_get_info())
const GstMetaInfo *gst_protection_meta_get_info (void);
GstProtectionMeta *gst_buffer_add_protection_meta (GstBuffer * buffer, GstStructure * info);
const gchar *gst_protection_select_system (const gchar ** system_identifiers);
G_END_DECLS
#endif