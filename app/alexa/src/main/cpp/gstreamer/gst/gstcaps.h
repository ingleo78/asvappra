#ifndef __GST_CAPS_H__
#define __GST_CAPS_H__

#include "gstconfig.h"
#include "gstminiobject.h"
#include "gststructure.h"
#include "gstcapsfeatures.h"
#include "glib-compat.h"

G_BEGIN_DECLS
GST_EXPORT GType _gst_caps_type;
#define GST_TYPE_CAPS             (_gst_caps_type)
#define GST_IS_CAPS(obj)          (GST_IS_MINI_OBJECT_TYPE((obj), GST_TYPE_CAPS))
#define GST_CAPS_CAST(obj)        ((GstCaps*)(obj))
#define GST_CAPS(obj)             (GST_CAPS_CAST(obj))
#define GST_TYPE_STATIC_CAPS      (gst_static_caps_get_type())
typedef enum {
  GST_CAPS_FLAG_ANY	= (GST_MINI_OBJECT_FLAG_LAST << 0)
} GstCapsFlags;
typedef enum {
  GST_CAPS_INTERSECT_ZIG_ZAG            =  0,
  GST_CAPS_INTERSECT_FIRST              =  1
} GstCapsIntersectMode;
#define GST_CAPS_ANY              _gst_caps_any
#define GST_CAPS_NONE             _gst_caps_none
#define GST_STATIC_CAPS_ANY       GST_STATIC_CAPS("ANY")
#define GST_STATIC_CAPS_NONE      GST_STATIC_CAPS("NONE")
#define GST_CAPS_IS_SIMPLE(caps) (gst_caps_get_size(caps) == 1)
#define GST_STATIC_CAPS(string) \
{ /* caps */ NULL, \
  /* string */ string, \
  GST_PADDING_INIT \
}
typedef struct _GstCaps GstCaps;
typedef struct _GstStaticCaps GstStaticCaps;
GST_EXPORT GstCaps * _gst_caps_any;
GST_EXPORT GstCaps * _gst_caps_none;
#define GST_CAPS_FLAGS(caps)                    GST_MINI_OBJECT_FLAGS(caps)
#define GST_CAPS_REFCOUNT(caps)                 GST_MINI_OBJECT_REFCOUNT(caps)
#define GST_CAPS_REFCOUNT_VALUE(caps)           GST_MINI_OBJECT_REFCOUNT_VALUE(caps)
#define GST_CAPS_FLAG_IS_SET(caps,flag)        GST_MINI_OBJECT_FLAG_IS_SET (caps, flag)
#define GST_CAPS_FLAG_SET(caps,flag)           GST_MINI_OBJECT_FLAG_SET (caps, flag)
#define GST_CAPS_FLAG_UNSET(caps,flag)         GST_MINI_OBJECT_FLAG_UNSET (caps, flag)
static inline GstCaps *gst_caps_ref (GstCaps * caps) {
  return (GstCaps *) gst_mini_object_ref (GST_MINI_OBJECT_CAST (caps));
}
static inline void gst_caps_unref (GstCaps * caps) {
  gst_mini_object_unref (GST_MINI_OBJECT_CAST (caps));
}
static inline GstCaps *gst_caps_copy (const GstCaps * caps) {
  return GST_CAPS (gst_mini_object_copy (GST_MINI_OBJECT_CAST (caps)));
}
#define         gst_caps_is_writable(caps)     gst_mini_object_is_writable (GST_MINI_OBJECT_CAST (caps))
#define         gst_caps_make_writable(caps)   GST_CAPS_CAST (gst_mini_object_make_writable (GST_MINI_OBJECT_CAST (caps)))
static inline gboolean gst_caps_replace (GstCaps **old_caps, GstCaps *new_caps) {
    return gst_mini_object_replace ((GstMiniObject **) old_caps, (GstMiniObject *) new_caps);
}
static inline gboolean gst_caps_take (GstCaps **old_caps, GstCaps *new_caps) {
    return gst_mini_object_take ((GstMiniObject **) old_caps, (GstMiniObject *) new_caps);
}
struct _GstCaps {
  GstMiniObject mini_object;
};
struct _GstStaticCaps {
  GstCaps *caps;
  const char *string;
  gpointer _gst_reserved[GST_PADDING];
};
typedef gboolean (*GstCapsForeachFunc)(GstCapsFeatures *features, GstStructure    *structure, gpointer user_data);

typedef gboolean (*GstCapsMapFunc)(GstCapsFeatures *features, GstStructure    *structure, gpointer user_data);
typedef gboolean (*GstCapsFilterMapFunc)(GstCapsFeatures *features, GstStructure    *structure, gpointer user_data);
GType             gst_caps_get_type                (void);
GstCaps *         gst_caps_new_empty               (void);
GstCaps *         gst_caps_new_any                 (void);
GstCaps *         gst_caps_new_empty_simple        (const char    *media_type) G_GNUC_WARN_UNUSED_RESULT;
GstCaps *gst_caps_new_simple(const char *media_type, const char *fieldname, ...) G_GNUC_NULL_TERMINATED G_GNUC_WARN_UNUSED_RESULT;
GstCaps *         gst_caps_new_full                (GstStructure  *struct1, ...) G_GNUC_NULL_TERMINATED G_GNUC_WARN_UNUSED_RESULT;
GstCaps *         gst_caps_new_full_valist         (GstStructure  *structure, va_list var_args) G_GNUC_WARN_UNUSED_RESULT;
GType             gst_static_caps_get_type         (void);
GstCaps *         gst_static_caps_get              (GstStaticCaps *static_caps);
void              gst_static_caps_cleanup          (GstStaticCaps *static_caps);
void              gst_caps_append                  (GstCaps       *caps1, GstCaps       *caps2);
void              gst_caps_append_structure        (GstCaps       *caps, GstStructure  *structure);
void              gst_caps_append_structure_full   (GstCaps       *caps, GstStructure  *structure, GstCapsFeatures *features);
void              gst_caps_remove_structure        (GstCaps       *caps, guint idx);
GstCaps *         gst_caps_merge                   (GstCaps       *caps1, GstCaps       *caps2) G_GNUC_WARN_UNUSED_RESULT;
GstCaps *         gst_caps_merge_structure         (GstCaps       *caps, GstStructure  *structure) G_GNUC_WARN_UNUSED_RESULT;
GstCaps *gst_caps_merge_structure_full(GstCaps *caps, GstStructure  *structure, GstCapsFeatures *features) G_GNUC_WARN_UNUSED_RESULT;
guint             gst_caps_get_size                (const GstCaps *caps);
GstStructure *    gst_caps_get_structure           (const GstCaps *caps, guint          index);
GstStructure *    gst_caps_steal_structure         (GstCaps       *caps, guint          index) G_GNUC_WARN_UNUSED_RESULT;
void              gst_caps_set_features            (GstCaps *caps, guint index, GstCapsFeatures * features);
GstCapsFeatures * gst_caps_get_features            (const GstCaps *caps, guint index);
GstCaps *         gst_caps_copy_nth                (const GstCaps *caps, guint nth) G_GNUC_WARN_UNUSED_RESULT;
GstCaps *         gst_caps_truncate                (GstCaps       *caps) G_GNUC_WARN_UNUSED_RESULT;
void              gst_caps_set_value               (GstCaps       *caps, const char    *field, const GValue  *value);
void              gst_caps_set_simple              (GstCaps       *caps, const char    *field, ...) G_GNUC_NULL_TERMINATED;
void              gst_caps_set_simple_valist       (GstCaps *caps, const char *field, va_list varargs);
gboolean          gst_caps_foreach                 (const GstCaps *caps, GstCapsForeachFunc func, gpointer user_data);
gboolean          gst_caps_map_in_place            (GstCaps *caps, GstCapsMapFunc  func, gpointer user_data);
void              gst_caps_filter_and_map_in_place (GstCaps *caps, GstCapsFilterMapFunc  func, gpointer user_data);
gboolean          gst_caps_is_any                  (const GstCaps *caps);
gboolean          gst_caps_is_empty                (const GstCaps *caps);
gboolean          gst_caps_is_fixed                (const GstCaps *caps);
gboolean          gst_caps_is_always_compatible    (const GstCaps *caps1, const GstCaps *caps2);
gboolean          gst_caps_is_subset		   (const GstCaps *subset, const GstCaps *superset);
gboolean          gst_caps_is_subset_structure     (const GstCaps *caps, const GstStructure *structure);
gboolean          gst_caps_is_subset_structure_full (const GstCaps *caps, const GstStructure *structure, const GstCapsFeatures *features);
gboolean          gst_caps_is_equal		   (const GstCaps *caps1, const GstCaps *caps2);
gboolean          gst_caps_is_equal_fixed          (const GstCaps *caps1, const GstCaps *caps2);
gboolean          gst_caps_can_intersect           (const GstCaps * caps1, const GstCaps * caps2);
gboolean          gst_caps_is_strictly_equal	   (const GstCaps *caps1, const GstCaps *caps2);
GstCaps *         gst_caps_intersect               (GstCaps *caps1, GstCaps *caps2) G_GNUC_WARN_UNUSED_RESULT;
GstCaps *         gst_caps_intersect_full          (GstCaps *caps1, GstCaps *caps2, GstCapsIntersectMode mode) G_GNUC_WARN_UNUSED_RESULT;
GstCaps *         gst_caps_subtract		   (GstCaps *minuend, GstCaps *subtrahend) G_GNUC_WARN_UNUSED_RESULT;
GstCaps *         gst_caps_normalize               (GstCaps *caps) G_GNUC_WARN_UNUSED_RESULT;
GstCaps *         gst_caps_simplify                (GstCaps *caps) G_GNUC_WARN_UNUSED_RESULT;
GstCaps *         gst_caps_fixate                  (GstCaps *caps) G_GNUC_WARN_UNUSED_RESULT;
gchar *           gst_caps_to_string               (const GstCaps *caps) G_GNUC_MALLOC;
GstCaps *         gst_caps_from_string             (const gchar   *string) G_GNUC_WARN_UNUSED_RESULT;
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstCaps, gst_caps_unref)
#endif
G_END_DECLS

#endif