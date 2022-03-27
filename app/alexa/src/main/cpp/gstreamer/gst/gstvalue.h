#ifndef __GST_VALUE_H__
#define __GST_VALUE_H__

#include "gstconfig.h"
#include "gstcaps.h"
#include "gststructure.h"
#include "gstcapsfeatures.h"

G_BEGIN_DECLS
#define GST_MAKE_FOURCC(a,b,c,d)        ((guint32)((a)|(b)<<8|(c)<<16|(d)<<24))
#define GST_STR_FOURCC(f)               ((guint32)(((f)[0])|((f)[1]<<8)|((f)[2]<<16)|((f)[3]<<24)))
#define GST_FOURCC_FORMAT "c%c%c%c"
#define __GST_PRINT_CHAR(c)  g_ascii_isprint(c) ? (c) : '.'
#define GST_FOURCC_ARGS(fourcc)               \
  __GST_PRINT_CHAR((fourcc) & 0xff),          \
  __GST_PRINT_CHAR(((fourcc) >> 8) & 0xff),   \
  __GST_PRINT_CHAR(((fourcc) >> 16) & 0xff),  \
  __GST_PRINT_CHAR(((fourcc) >> 24) & 0xff)
#define GST_VALUE_HOLDS_INT_RANGE(x)      ((x) != NULL && G_VALUE_TYPE(x) == _gst_int_range_type)
#define GST_VALUE_HOLDS_INT64_RANGE(x)    ((x) != NULL && G_VALUE_TYPE(x) == _gst_int64_range_type)
#define GST_VALUE_HOLDS_DOUBLE_RANGE(x)   ((x) != NULL && G_VALUE_TYPE(x) == _gst_double_range_type)
#define GST_VALUE_HOLDS_FRACTION_RANGE(x) ((x) != NULL && G_VALUE_TYPE(x) == _gst_fraction_range_type)
#define GST_VALUE_HOLDS_LIST(x)         ((x) != NULL && G_VALUE_TYPE(x) == _gst_value_list_type)
#define GST_VALUE_HOLDS_ARRAY(x)        ((x) != NULL && G_VALUE_TYPE(x) == _gst_value_array_type)
#define GST_VALUE_HOLDS_CAPS(x)         ((x) != NULL && G_VALUE_TYPE(x) == _gst_caps_type)
#define GST_VALUE_HOLDS_STRUCTURE(x)            (G_VALUE_HOLDS((x), _gst_structure_type))
#define GST_VALUE_HOLDS_CAPS_FEATURES(x)        (G_VALUE_HOLDS((x), _gst_caps_features_type))
#define GST_VALUE_HOLDS_BUFFER(x)       ((x) != NULL && G_VALUE_TYPE(x) == _gst_buffer_type)
#define GST_VALUE_HOLDS_SAMPLE(x)       ((x) != NULL && G_VALUE_TYPE(x) == _gst_sample_type)
#define GST_VALUE_HOLDS_FRACTION(x)     ((x) != NULL && G_VALUE_TYPE(x) == _gst_fraction_type)
#define GST_VALUE_HOLDS_DATE_TIME(x)    ((x) != NULL && G_VALUE_TYPE(x) == _gst_date_time_type)
#define GST_VALUE_HOLDS_BITMASK(x)      ((x) != NULL && G_VALUE_TYPE(x) == _gst_bitmask_type)
#define GST_VALUE_HOLDS_FLAG_SET(x)     (G_TYPE_CHECK_VALUE_TYPE ((x), GST_TYPE_FLAG_SET))
#define GST_FLAG_SET_MASK_EXACT ((guint)(-1))
GST_EXPORT GType _gst_int_range_type;
#define GST_TYPE_INT_RANGE               (_gst_int_range_type)
GST_EXPORT GType _gst_int64_range_type;
#define GST_TYPE_INT64_RANGE             (_gst_int64_range_type)
GST_EXPORT GType _gst_double_range_type;
#define GST_TYPE_DOUBLE_RANGE            (_gst_double_range_type)
GST_EXPORT GType _gst_fraction_range_type;
#define GST_TYPE_FRACTION_RANGE           (_gst_fraction_range_type)
GST_EXPORT GType _gst_value_list_type;
#define GST_TYPE_LIST                    (_gst_value_list_type)
GST_EXPORT GType _gst_value_array_type;
#define GST_TYPE_ARRAY                   (_gst_value_array_type)
GST_EXPORT GType _gst_fraction_type;
#define GST_TYPE_FRACTION                (_gst_fraction_type)
GST_EXPORT GType _gst_bitmask_type;
#define GST_TYPE_BITMASK                 (_gst_bitmask_type)
GST_EXPORT GType _gst_flagset_type;
#define GST_TYPE_FLAG_SET                   (_gst_flagset_type)
#define GST_TYPE_G_THREAD                gst_g_thread_get_type ()
#define GST_VALUE_LESS_THAN              (-1)
#define GST_VALUE_EQUAL                   0
#define GST_VALUE_GREATER_THAN            1
#define GST_VALUE_UNORDERED               2
typedef gint     (* GstValueCompareFunc)     (const GValue *value1, const GValue *value2);
typedef gchar *  (* GstValueSerializeFunc)   (const GValue *value1);
typedef gboolean (* GstValueDeserializeFunc) (GValue       *dest, const gchar  *s);
typedef struct _GstValueTable GstValueTable;
struct _GstValueTable {
  GType type;
  GstValueCompareFunc compare;
  GstValueSerializeFunc serialize;
  GstValueDeserializeFunc deserialize;
  gpointer _gst_reserved [GST_PADDING];
};
GType gst_int_range_get_type (void);
GType gst_int64_range_get_type (void);
GType gst_double_range_get_type (void);
GType gst_fraction_range_get_type (void);
GType gst_fraction_get_type (void);
GType gst_value_list_get_type (void);
GType gst_value_array_get_type (void);
GType gst_bitmask_get_type (void);
GType gst_flagset_get_type (void);
#ifndef __GI_SCANNER__
GType gst_g_thread_get_type (void);
#endif
void            gst_value_register              (const GstValueTable   *table);
void            gst_value_init_and_copy         (GValue                *dest, const GValue          *src);
gchar *         gst_value_serialize             (const GValue          *value) G_GNUC_MALLOC;
gboolean        gst_value_deserialize           (GValue                *dest, const gchar           *src);
void            gst_value_list_append_value     (GValue         *value, const GValue   *append_value);
void            gst_value_list_append_and_take_value (GValue         *value, GValue   *append_value);
void            gst_value_list_prepend_value    (GValue         *value, const GValue   *prepend_value);
void            gst_value_list_concat           (GValue *dest, const GValue   *value1, const GValue   *value2);
void            gst_value_list_merge            (GValue *dest, const GValue *value1, const GValue *value2);
guint           gst_value_list_get_size         (const GValue   *value);
const GValue *  gst_value_list_get_value        (const GValue   *value, guint          index);
void            gst_value_array_append_value    (GValue         *value, const GValue   *append_value);
void            gst_value_array_append_and_take_value    (GValue         *value, GValue   *append_value);
void            gst_value_array_prepend_value   (GValue         *value, const GValue   *prepend_value);
guint           gst_value_array_get_size        (const GValue   *value);
const GValue *  gst_value_array_get_value       (const GValue   *value, guint          index);
void            gst_value_set_int_range         (GValue         *value, gint           start, gint           end);
void            gst_value_set_int_range_step    (GValue *value, gint start, gint end, gint step);
gint            gst_value_get_int_range_min     (const GValue   *value);
gint            gst_value_get_int_range_max     (const GValue   *value);
gint            gst_value_get_int_range_step    (const GValue   *value);
void            gst_value_set_int64_range       (GValue         *value, gint64         start, gint64         end);
void            gst_value_set_int64_range_step  (GValue *value, gint64 start, gint64 end, gint64 step);
gint64          gst_value_get_int64_range_min   (const GValue   *value);
gint64          gst_value_get_int64_range_max   (const GValue   *value);
gint64          gst_value_get_int64_range_step  (const GValue   *value);
void            gst_value_set_double_range      (GValue *value, gdouble start, gdouble end);
gdouble         gst_value_get_double_range_min  (const GValue   *value);
gdouble         gst_value_get_double_range_max  (const GValue   *value);
const GstCaps * gst_value_get_caps              (const GValue   *value);
void            gst_value_set_caps              (GValue *value, const GstCaps  *caps);
const GstStructure *
                gst_value_get_structure         (const GValue   *value);
void            gst_value_set_structure         (GValue         *value, const GstStructure  *structure);
const GstCapsFeatures *
                gst_value_get_caps_features     (const GValue   *value);
void            gst_value_set_caps_features     (GValue         *value, const GstCapsFeatures  *features);
void            gst_value_set_fraction          (GValue *value, gint numerator, gint denominator);
gint            gst_value_get_fraction_numerator   (const GValue  *value);
gint            gst_value_get_fraction_denominator (const GValue *value);
gboolean        gst_value_fraction_multiply        (GValue   *product, const GValue   *factor1, const GValue   *factor2);
gboolean        gst_value_fraction_subtract     (GValue * dest, const GValue * minuend, const GValue * subtrahend);
void            gst_value_set_fraction_range    (GValue         *value, const GValue   *start, const GValue   *end);
void gst_value_set_fraction_range_full(GValue *value, gint numerator_start, gint denominator_start, gint numerator_end, gint denominator_end);
const GValue    *gst_value_get_fraction_range_min (const GValue *value);
const GValue    *gst_value_get_fraction_range_max (const GValue *value);
guint64         gst_value_get_bitmask           (const GValue   *value);
void            gst_value_set_bitmask           (GValue         *value, guint64         bitmask);
void            gst_value_set_flagset (GValue * value, guint flags, guint mask);
guint           gst_value_get_flagset_flags (const GValue * value);
guint           gst_value_get_flagset_mask (const GValue * value);
gint            gst_value_compare               (const GValue   *value1, const GValue   *value2);
gboolean        gst_value_can_compare           (const GValue   *value1, const GValue   *value2);
gboolean        gst_value_is_subset             (const GValue   *value1, const GValue   *value2);
gboolean        gst_value_union                 (GValue         *dest, const GValue   *value1, const GValue   *value2);
gboolean        gst_value_can_union             (const GValue   *value1, const GValue   *value2);
gboolean        gst_value_intersect             (GValue         *dest, const GValue   *value1, const GValue   *value2);
gboolean        gst_value_can_intersect         (const GValue   *value1, const GValue   *value2);
gboolean        gst_value_subtract              (GValue         *dest, const GValue   *minuend, const GValue   *subtrahend);
gboolean        gst_value_can_subtract          (const GValue   *minuend, const GValue   *subtrahend);
gboolean        gst_value_is_fixed              (const GValue   *value);
gboolean        gst_value_fixate                (GValue         *dest, const GValue   *src);
GType		gst_flagset_register (GType flags_type);
G_END_DECLS

#endif