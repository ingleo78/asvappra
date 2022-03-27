#ifndef __GST_QUERY_H__
#define __GST_QUERY_H__

#include <glib/glib.h>
#include "gstminiobject.h"
#include "gststructure.h"
#include "gstformat.h"
#include "gstallocator.h"
#include "gsttoc.h"
#include "gstcontext.h"

G_BEGIN_DECLS
typedef struct _GstQuery GstQuery;
typedef enum {
  GST_QUERY_TYPE_UPSTREAM       = 1 << 0,
  GST_QUERY_TYPE_DOWNSTREAM     = 1 << 1,
  GST_QUERY_TYPE_SERIALIZED     = 1 << 2
} GstQueryTypeFlags;
typedef enum {
    GST_PAD_MODE_NONE,
    GST_PAD_MODE_PUSH,
    GST_PAD_MODE_PULL
} GstPadMode;
#define GST_QUERY_TYPE_BOTH  (GST_QUERY_TYPE_UPSTREAM | GST_QUERY_TYPE_DOWNSTREAM)
#define GST_QUERY_NUM_SHIFT     (8)
#define GST_QUERY_MAKE_TYPE(num,flags)  (((num) << GST_QUERY_NUM_SHIFT) | (flags))
#define FLAG(name) GST_QUERY_TYPE_##name
typedef enum {
  GST_QUERY_UNKNOWN      = GST_QUERY_MAKE_TYPE (0, 0),
  GST_QUERY_POSITION     = GST_QUERY_MAKE_TYPE (10, FLAG(BOTH)),
  GST_QUERY_DURATION     = GST_QUERY_MAKE_TYPE (20, FLAG(BOTH)),
  GST_QUERY_LATENCY      = GST_QUERY_MAKE_TYPE (30, FLAG(BOTH)),
  GST_QUERY_JITTER       = GST_QUERY_MAKE_TYPE (40, FLAG(BOTH)),
  GST_QUERY_RATE         = GST_QUERY_MAKE_TYPE (50, FLAG(BOTH)),
  GST_QUERY_SEEKING      = GST_QUERY_MAKE_TYPE (60, FLAG(BOTH)),
  GST_QUERY_SEGMENT      = GST_QUERY_MAKE_TYPE (70, FLAG(BOTH)),
  GST_QUERY_CONVERT      = GST_QUERY_MAKE_TYPE (80, FLAG(BOTH)),
  GST_QUERY_FORMATS      = GST_QUERY_MAKE_TYPE (90, FLAG(BOTH)),
  GST_QUERY_BUFFERING    = GST_QUERY_MAKE_TYPE (110, FLAG(BOTH)),
  GST_QUERY_CUSTOM       = GST_QUERY_MAKE_TYPE (120, FLAG(BOTH)),
  GST_QUERY_URI          = GST_QUERY_MAKE_TYPE (130, FLAG(BOTH)),
  GST_QUERY_ALLOCATION   = GST_QUERY_MAKE_TYPE (140, FLAG(DOWNSTREAM) | FLAG(SERIALIZED)),
  GST_QUERY_SCHEDULING   = GST_QUERY_MAKE_TYPE (150, FLAG(UPSTREAM)),
  GST_QUERY_ACCEPT_CAPS  = GST_QUERY_MAKE_TYPE (160, FLAG(BOTH)),
  GST_QUERY_CAPS         = GST_QUERY_MAKE_TYPE (170, FLAG(BOTH)),
  GST_QUERY_DRAIN        = GST_QUERY_MAKE_TYPE (180, FLAG(DOWNSTREAM) | FLAG(SERIALIZED)),
  GST_QUERY_CONTEXT      = GST_QUERY_MAKE_TYPE (190, FLAG(BOTH))
} GstQueryType;
#undef FLAG
GST_EXPORT GType _gst_query_type;
#define GST_TYPE_QUERY                         (_gst_query_type)
#define GST_IS_QUERY(obj)                      (GST_IS_MINI_OBJECT_TYPE (obj, GST_TYPE_QUERY))
#define GST_QUERY_CAST(obj)                    ((GstQuery*)(obj))
#define GST_QUERY(obj)                         (GST_QUERY_CAST(obj))
#define GST_QUERY_TYPE(query)  (((GstQuery*)(query))->type)
#define GST_QUERY_TYPE_NAME(query) (gst_query_type_get_name(GST_QUERY_TYPE(query)))
#define GST_QUERY_IS_UPSTREAM(ev)       !!(GST_QUERY_TYPE (ev) & GST_QUERY_TYPE_UPSTREAM)
#define GST_QUERY_IS_DOWNSTREAM(ev)     !!(GST_QUERY_TYPE (ev) & GST_QUERY_TYPE_DOWNSTREAM)
#define GST_QUERY_IS_SERIALIZED(ev)     !!(GST_QUERY_TYPE (ev) & GST_QUERY_TYPE_SERIALIZED)
struct _GstQuery {
  GstMiniObject mini_object;
  GstQueryType type;
};
const gchar*    gst_query_type_get_name        (GstQueryType type);
GQuark          gst_query_type_to_quark        (GstQueryType type);
GstQueryTypeFlags gst_query_type_get_flags       (GstQueryType type);
GType           gst_query_get_type             (void);
static inline GstQuery *gst_query_ref (GstQuery * q) {
  return GST_QUERY_CAST (gst_mini_object_ref (GST_MINI_OBJECT_CAST (q)));
}
static inline void gst_query_unref (GstQuery * q) {
  gst_mini_object_unref (GST_MINI_OBJECT_CAST (q));
}
static inline GstQuery *gst_query_copy (const GstQuery * q) {
  return GST_QUERY_CAST (gst_mini_object_copy (GST_MINI_OBJECT_CONST_CAST (q)));
}
#define         gst_query_is_writable(q)     gst_mini_object_is_writable (GST_MINI_OBJECT_CAST (q))
#define         gst_query_make_writable(q)      GST_QUERY_CAST (gst_mini_object_make_writable (GST_MINI_OBJECT_CAST (q)))
static inline gboolean gst_query_replace (GstQuery **old_query, GstQuery *new_query) {
  return gst_mini_object_replace ((GstMiniObject **) old_query, (GstMiniObject *) new_query);
}
GstQuery *      gst_query_new_custom            (GstQueryType type, GstStructure *structure) G_GNUC_MALLOC;
const GstStructure *gst_query_get_structure         (GstQuery *query);
GstStructure *  gst_query_writable_structure    (GstQuery *query);
GstQuery*       gst_query_new_position          (GstFormat format) G_GNUC_MALLOC;
void            gst_query_set_position          (GstQuery *query, GstFormat format, gint64 cur);
void            gst_query_parse_position        (GstQuery *query, GstFormat *format, gint64 *cur);
GstQuery*       gst_query_new_duration          (GstFormat format) G_GNUC_MALLOC;
void            gst_query_set_duration          (GstQuery *query, GstFormat format, gint64 duration);
void            gst_query_parse_duration        (GstQuery *query, GstFormat *format, gint64 *duration);
GstQuery*       gst_query_new_latency           (void) G_GNUC_MALLOC;
void gst_query_set_latency(GstQuery *query, gboolean live, GstClockTime min_latency, GstClockTime max_latency);
void            gst_query_parse_latency(GstQuery *query, gboolean *live, GstClockTime *min_latency, GstClockTime *max_latency);
GstQuery*       gst_query_new_convert           (GstFormat src_format, gint64 value, GstFormat dest_format) G_GNUC_MALLOC;
void            gst_query_set_convert(GstQuery *query, GstFormat src_format, gint64 src_value, GstFormat dest_format, gint64 dest_value);
void            gst_query_parse_convert(GstQuery *query, GstFormat *src_format, gint64 *src_value, GstFormat *dest_format, gint64 *dest_value);
GstQuery*       gst_query_new_segment           (GstFormat format) G_GNUC_MALLOC;
void            gst_query_set_segment           (GstQuery *query, gdouble rate, GstFormat format, gint64 start_value, gint64 stop_value);
void            gst_query_parse_segment         (GstQuery *query, gdouble *rate, GstFormat *format, gint64 *start_value, gint64 *stop_value);
GstQuery*       gst_query_new_seeking           (GstFormat format) G_GNUC_MALLOC;
void gst_query_set_seeking(GstQuery *query, GstFormat format, gboolean seekable, gint64 segment_start, gint64 segment_end);
void gst_query_parse_seeking(GstQuery *query, GstFormat *format, gboolean *seekable, gint64 *segment_start, gint64 *segment_end);
GstQuery*       gst_query_new_formats           (void) G_GNUC_MALLOC;
void            gst_query_set_formats           (GstQuery *query, gint n_formats, ...);
void            gst_query_set_formatsv          (GstQuery *query, gint n_formats, const GstFormat *formats);
void            gst_query_parse_n_formats       (GstQuery *query, guint *n_formats);
void            gst_query_parse_nth_format      (GstQuery *query, guint nth, GstFormat *format);
typedef enum {
  GST_BUFFERING_STREAM,
  GST_BUFFERING_DOWNLOAD,
  GST_BUFFERING_TIMESHIFT,
  GST_BUFFERING_LIVE
} GstBufferingMode;
GstQuery*       gst_query_new_buffering           (GstFormat format) G_GNUC_MALLOC;
void            gst_query_set_buffering_percent   (GstQuery *query, gboolean busy, gint percent);
void            gst_query_parse_buffering_percent (GstQuery *query, gboolean *busy, gint *percent);
void            gst_query_set_buffering_stats(GstQuery *query, GstBufferingMode mode, gint avg_in, gint avg_out, gint64 buffering_left);
void            gst_query_parse_buffering_stats(GstQuery *query, GstBufferingMode *mode, gint *avg_in, gint *avg_out, gint64 *buffering_left);
void            gst_query_set_buffering_range     (GstQuery *query, GstFormat format, gint64 start, gint64 stop, gint64 estimated_total);
void            gst_query_parse_buffering_range   (GstQuery *query, GstFormat *format, gint64 *start, gint64 *stop, gint64 *estimated_total);
gboolean        gst_query_add_buffering_range       (GstQuery *query, gint64 start, gint64 stop);
guint           gst_query_get_n_buffering_ranges    (GstQuery *query);
gboolean        gst_query_parse_nth_buffering_range (GstQuery *query, guint index, gint64 *start, gint64 *stop);
GstQuery *      gst_query_new_uri                    (void) G_GNUC_MALLOC;
void            gst_query_parse_uri                  (GstQuery *query, gchar **uri);
void            gst_query_set_uri                    (GstQuery *query, const gchar *uri);
void            gst_query_parse_uri_redirection      (GstQuery *query, gchar **uri);
void            gst_query_set_uri_redirection        (GstQuery *query, const gchar *uri);
void            gst_query_parse_uri_redirection_permanent (GstQuery *query, gboolean * permanent);
void            gst_query_set_uri_redirection_permanent (GstQuery *query, gboolean permanent);
GstQuery *      gst_query_new_allocation             (GstCaps *caps, gboolean need_pool) G_GNUC_MALLOC;
void            gst_query_parse_allocation           (GstQuery *query, GstCaps **caps, gboolean *need_pool);
void            gst_query_add_allocation_pool        (GstQuery *query, GstBufferPool *pool, guint size, guint min_buffers, guint max_buffers);
guint           gst_query_get_n_allocation_pools     (GstQuery *query);
void gst_query_parse_nth_allocation_pool(GstQuery *query, guint index, GstBufferPool **pool, guint *size, guint *min_buffers,
                                         guint *max_buffers);
void gst_query_set_nth_allocation_pool(GstQuery *query, guint index, GstBufferPool *pool, guint size, guint min_buffers, guint max_buffers);
void            gst_query_remove_nth_allocation_pool (GstQuery *query, guint index);
void            gst_query_add_allocation_param       (GstQuery *query, GstAllocator *allocator,
                                                      const GstAllocationParams *params);
guint           gst_query_get_n_allocation_params    (GstQuery *query);
void gst_query_parse_nth_allocation_param(GstQuery *query, guint index, GstAllocator **allocator, GstAllocationParams *params);
void            gst_query_set_nth_allocation_param(GstQuery *query, guint index, GstAllocator *allocator, const GstAllocationParams *params);
void            gst_query_remove_nth_allocation_param (GstQuery *query, guint index);
void            gst_query_add_allocation_meta        (GstQuery *query, GType api, const GstStructure *params);
guint           gst_query_get_n_allocation_metas     (GstQuery *query);
GType           gst_query_parse_nth_allocation_meta  (GstQuery *query, guint index, const GstStructure **params);
void            gst_query_remove_nth_allocation_meta (GstQuery *query, guint index);
gboolean        gst_query_find_allocation_meta       (GstQuery *query, GType api, guint *index);
typedef enum {
  GST_SCHEDULING_FLAG_SEEKABLE          = (1 << 0),
  GST_SCHEDULING_FLAG_SEQUENTIAL        = (1 << 1),
  GST_SCHEDULING_FLAG_BANDWIDTH_LIMITED = (1 << 2)
} GstSchedulingFlags;
GstQuery *      gst_query_new_scheduling          (void) G_GNUC_MALLOC;
void            gst_query_set_scheduling          (GstQuery *query, GstSchedulingFlags flags, gint minsize, gint maxsize, gint align);
void            gst_query_parse_scheduling        (GstQuery *query, GstSchedulingFlags *flags, gint *minsize, gint *maxsize, gint *align);
void            gst_query_add_scheduling_mode       (GstQuery *query, GstPadMode mode);
guint           gst_query_get_n_scheduling_modes    (GstQuery *query);
GstPadMode      gst_query_parse_nth_scheduling_mode (GstQuery *query, guint index);
gboolean        gst_query_has_scheduling_mode       (GstQuery *query, GstPadMode mode);
gboolean        gst_query_has_scheduling_mode_with_flags (GstQuery * query, GstPadMode mode, GstSchedulingFlags flags);
GstQuery *      gst_query_new_accept_caps          (GstCaps *caps) G_GNUC_MALLOC;
void            gst_query_parse_accept_caps        (GstQuery *query, GstCaps **caps);
void            gst_query_set_accept_caps_result   (GstQuery *query, gboolean result);
void            gst_query_parse_accept_caps_result (GstQuery *query, gboolean *result);
GstQuery *      gst_query_new_caps                 (GstCaps *filter) G_GNUC_MALLOC;
void            gst_query_parse_caps               (GstQuery *query, GstCaps **filter);
void            gst_query_set_caps_result          (GstQuery *query, GstCaps *caps);
void            gst_query_parse_caps_result        (GstQuery *query, GstCaps **caps);
GstQuery *      gst_query_new_drain                (void) G_GNUC_MALLOC;
GstQuery *      gst_query_new_context              (const gchar * context_type) G_GNUC_MALLOC;
gboolean        gst_query_parse_context_type       (GstQuery * query, const gchar ** context_type);
void            gst_query_set_context              (GstQuery *query, GstContext *context);
void            gst_query_parse_context            (GstQuery *query, GstContext **context);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstQuery, gst_query_unref)
#endif
G_END_DECLS

#endif
