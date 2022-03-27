#ifndef __GST_STRUCTURE_H__
#define __GST_STRUCTURE_H__

#include <glib/glib-object.h>
#include "gstconfig.h"
#include "gstclock.h"
#include "gstdatetime.h"
#include "glib-compat.h"

G_BEGIN_DECLS
GST_EXPORT GType _gst_structure_type;
typedef struct _GstStructure GstStructure;
#define GST_TYPE_STRUCTURE             (_gst_structure_type)
#define GST_IS_STRUCTURE(object)       ((object) && (GST_STRUCTURE(object)->type == GST_TYPE_STRUCTURE))
#define GST_STRUCTURE_CAST(object)     ((GstStructure *)(object))
#define GST_STRUCTURE(object)          (GST_STRUCTURE_CAST(object))
typedef gboolean (*GstStructureForeachFunc) (GQuark   field_id, const GValue * value, gpointer user_data);
typedef gboolean (*GstStructureMapFunc)     (GQuark   field_id, GValue * value, gpointer user_data);
typedef gboolean (*GstStructureFilterMapFunc) (GQuark   field_id, GValue * value, gpointer user_data);
struct _GstStructure {
  GType type;
  GQuark name;
};
GType                 gst_structure_get_type             (void);
GstStructure *        gst_structure_new_empty            (const gchar * name) G_GNUC_MALLOC;
GstStructure *        gst_structure_new_id_empty         (GQuark quark) G_GNUC_MALLOC;
GstStructure *        gst_structure_new(const gchar * name, const gchar * firstfield, ...) G_GNUC_NULL_TERMINATED  G_GNUC_MALLOC;
GstStructure *        gst_structure_new_valist           (const gchar * name, const gchar * firstfield, va_list varargs) G_GNUC_MALLOC;
GstStructure *        gst_structure_new_id               (GQuark name_quark, GQuark field_quark, ...) G_GNUC_MALLOC;
GstStructure *        gst_structure_new_from_string      (const gchar * string);
GstStructure *        gst_structure_copy                 (const GstStructure  * structure) G_GNUC_MALLOC;
gboolean              gst_structure_set_parent_refcount  (GstStructure        *structure, gint *refcount);
void                  gst_structure_free                 (GstStructure        * structure);
const gchar *         gst_structure_get_name             (const GstStructure  * structure);
GQuark                gst_structure_get_name_id          (const GstStructure  * structure);
gboolean              gst_structure_has_name             (const GstStructure  * structure, const gchar * name);
void                  gst_structure_set_name             (GstStructure        * structure, const gchar * name);
void                  gst_structure_id_set_value         (GstStructure * structure, GQuark field, const GValue * value);
void                  gst_structure_set_value            (GstStructure * structure, const gchar * fieldname, const GValue * value);
void                  gst_structure_id_take_value        (GstStructure * structure, GQuark field, GValue * value);
void                  gst_structure_take_value           (GstStructure * structure, const gchar * fieldname, GValue * value);
void                  gst_structure_set                  (GstStructure * structure, const gchar * fieldname, ...) G_GNUC_NULL_TERMINATED;
void                  gst_structure_set_valist           (GstStructure * structure, const gchar * fieldname, va_list varargs);
void                  gst_structure_id_set               (GstStructure * structure, GQuark fieldname, ...) G_GNUC_NULL_TERMINATED;
void                  gst_structure_id_set_valist        (GstStructure * structure, GQuark fieldname, va_list varargs);
gboolean              gst_structure_get_valist           (const GstStructure  * structure, const char * first_fieldname, va_list args);
gboolean              gst_structure_get(const GstStructure  * structure, const char * first_fieldname, ...) G_GNUC_NULL_TERMINATED;
gboolean              gst_structure_id_get_valist        (const GstStructure  * structure, GQuark first_field_id, va_list args);
gboolean              gst_structure_id_get               (const GstStructure  * structure, GQuark first_field_id, ...) G_GNUC_NULL_TERMINATED;
const GValue *        gst_structure_id_get_value         (const GstStructure  * structure, GQuark field);
const GValue *        gst_structure_get_value            (const GstStructure  * structure, const gchar         * fieldname);
void                  gst_structure_remove_field         (GstStructure        * structure, const gchar         * fieldname);
void                  gst_structure_remove_fields        (GstStructure * structure, const gchar * fieldname, ...) G_GNUC_NULL_TERMINATED;
void                  gst_structure_remove_fields_valist (GstStructure * structure, const gchar * fieldname, va_list varargs);
void                  gst_structure_remove_all_fields    (GstStructure        * structure);
GType                 gst_structure_get_field_type       (const GstStructure  * structure, const gchar * fieldname);
gboolean              gst_structure_foreach              (const GstStructure  * structure, GstStructureForeachFunc func, gpointer user_data);
gboolean              gst_structure_map_in_place         (GstStructure * structure, GstStructureMapFunc func, gpointer user_data);
void                  gst_structure_filter_and_map_in_place (GstStructure * structure, GstStructureFilterMapFunc func, gpointer user_data);
gint                  gst_structure_n_fields             (const GstStructure  * structure);
const gchar *         gst_structure_nth_field_name       (const GstStructure  * structure, guint index);
gboolean              gst_structure_id_has_field         (const GstStructure  * structure, GQuark field);
gboolean              gst_structure_id_has_field_typed   (const GstStructure  * structure, GQuark field, GType type);
gboolean              gst_structure_has_field            (const GstStructure  * structure, const gchar * fieldname);
gboolean              gst_structure_has_field_typed      (const GstStructure  * structure, const gchar * fieldname, GType type);
gboolean              gst_structure_get_boolean          (const GstStructure  * structure, const gchar * fieldname, gboolean * value);
gboolean              gst_structure_get_int              (const GstStructure  * structure, const gchar * fieldname, gint * value);
gboolean              gst_structure_get_uint             (const GstStructure  * structure, const gchar * fieldname, guint * value);
gboolean              gst_structure_get_int64            (const GstStructure  * structure, const gchar * fieldname, gint64 * value);
gboolean              gst_structure_get_uint64           (const GstStructure  * structure, const gchar * fieldname, guint64 * value);
gboolean              gst_structure_get_double           (const GstStructure  * structure, const gchar * fieldname, gdouble * value);
gboolean              gst_structure_get_date             (const GstStructure  * structure, const gchar * fieldname, GDate ** value);
gboolean              gst_structure_get_date_time        (const GstStructure  * structure, const gchar * fieldname, GstDateTime ** value);
gboolean              gst_structure_get_clock_time       (const GstStructure  * structure, const gchar * fieldname, GstClockTime * value);
const gchar *         gst_structure_get_string           (const GstStructure  * structure, const gchar         * fieldname);
gboolean              gst_structure_get_enum(const GstStructure  * structure, const gchar * fieldname, GType enumtype, gint * value);
gboolean gst_structure_get_fraction(const GstStructure  * structure, const gchar *fieldname, gint *value_numerator, gint *value_denominator);
gboolean gst_structure_get_flagset(const GstStructure *structure, const gchar *fieldname, guint *value_flags, guint *value_mask);
gchar *               gst_structure_to_string    (const GstStructure * structure) G_GNUC_MALLOC;
GstStructure *        gst_structure_from_string  (const gchar * string, gchar      ** end) G_GNUC_MALLOC;
gboolean              gst_structure_fixate_field_nearest_int      (GstStructure * structure, const char   * field_name, int target);
gboolean              gst_structure_fixate_field_nearest_double   (GstStructure * structure, const char   * field_name, double target);
gboolean              gst_structure_fixate_field_boolean          (GstStructure * structure, const char   * field_name, gboolean target);
gboolean              gst_structure_fixate_field_string           (GstStructure * structure, const char   * field_name, const gchar  * target);
gboolean gst_structure_fixate_field_nearest_fraction(GstStructure *structure, const char *field_name, const gint target_numerator,
                                                                   const gint target_denominator);

gboolean              gst_structure_fixate_field  (GstStructure * structure, const char   * field_name);
void                  gst_structure_fixate        (GstStructure * structure);
gboolean              gst_structure_is_equal      (const GstStructure * structure1, const GstStructure * structure2);
gboolean              gst_structure_is_subset     (const GstStructure * subset, const GstStructure * superset);
gboolean              gst_structure_can_intersect (const GstStructure * struct1, const GstStructure * struct2);
GstStructure *        gst_structure_intersect     (const GstStructure * struct1, const GstStructure * struct2) G_GNUC_MALLOC;
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstStructure, gst_structure_free)
#endif
G_END_DECLS

#endif