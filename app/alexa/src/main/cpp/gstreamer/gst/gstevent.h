#ifndef __GST_EVENT_H__
#define __GST_EVENT_H__

#include "gstminiobject.h"
#include "gstformat.h"
#include "gstobject.h"
#include "gstclock.h"
#include "gststructure.h"
#include "gsttaglist.h"
#include "gstsegment.h"
#include "gstmessage.h"
#include "gstcontext.h"

typedef struct _GstEvent GstEvent;
typedef enum {
  GST_EVENT_TYPE_UPSTREAM       = 1 << 0,
  GST_EVENT_TYPE_DOWNSTREAM     = 1 << 1,
  GST_EVENT_TYPE_SERIALIZED     = 1 << 2,
  GST_EVENT_TYPE_STICKY         = 1 << 3,
  GST_EVENT_TYPE_STICKY_MULTI   = 1 << 4
} GstEventTypeFlags;
#define GST_EVENT_TYPE_BOTH  (GST_EVENT_TYPE_UPSTREAM | GST_EVENT_TYPE_DOWNSTREAM)
#define GST_EVENT_NUM_SHIFT     (8)
#define GST_EVENT_MAKE_TYPE(num,flags)  (((num) << GST_EVENT_NUM_SHIFT) | (flags))
#define FLAG(name) GST_EVENT_TYPE_##name
typedef enum {
  GST_EVENT_UNKNOWN               = GST_EVENT_MAKE_TYPE (0, 0),
  GST_EVENT_FLUSH_START           = GST_EVENT_MAKE_TYPE (10, FLAG(BOTH)),
  GST_EVENT_FLUSH_STOP            = GST_EVENT_MAKE_TYPE (20, FLAG(BOTH) | FLAG(SERIALIZED)),
  GST_EVENT_STREAM_START          = GST_EVENT_MAKE_TYPE (40, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY)),
  GST_EVENT_CAPS                  = GST_EVENT_MAKE_TYPE (50, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY)),
  GST_EVENT_SEGMENT               = GST_EVENT_MAKE_TYPE (70, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY)),
  GST_EVENT_TAG                   = GST_EVENT_MAKE_TYPE (80, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY) | FLAG(STICKY_MULTI)),
  GST_EVENT_BUFFERSIZE            = GST_EVENT_MAKE_TYPE (90, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY)),
  GST_EVENT_SINK_MESSAGE          = GST_EVENT_MAKE_TYPE (100, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY) | FLAG(STICKY_MULTI)),
  GST_EVENT_EOS                   = GST_EVENT_MAKE_TYPE (110, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY)),
  GST_EVENT_TOC                   = GST_EVENT_MAKE_TYPE (120, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY) | FLAG(STICKY_MULTI)),
  GST_EVENT_PROTECTION            = GST_EVENT_MAKE_TYPE (130, FLAG (DOWNSTREAM) | FLAG (SERIALIZED) | FLAG (STICKY) | FLAG (STICKY_MULTI)),
  GST_EVENT_SEGMENT_DONE          = GST_EVENT_MAKE_TYPE (150, FLAG(DOWNSTREAM) | FLAG(SERIALIZED)),
  GST_EVENT_GAP                   = GST_EVENT_MAKE_TYPE (160, FLAG(DOWNSTREAM) | FLAG(SERIALIZED)),
  GST_EVENT_QOS                   = GST_EVENT_MAKE_TYPE (190, FLAG(UPSTREAM)),
  GST_EVENT_SEEK                  = GST_EVENT_MAKE_TYPE (200, FLAG(UPSTREAM)),
  GST_EVENT_NAVIGATION            = GST_EVENT_MAKE_TYPE (210, FLAG(UPSTREAM)),
  GST_EVENT_LATENCY               = GST_EVENT_MAKE_TYPE (220, FLAG(UPSTREAM)),
  GST_EVENT_STEP                  = GST_EVENT_MAKE_TYPE (230, FLAG(UPSTREAM)),
  GST_EVENT_RECONFIGURE           = GST_EVENT_MAKE_TYPE (240, FLAG(UPSTREAM)),
  GST_EVENT_TOC_SELECT            = GST_EVENT_MAKE_TYPE (250, FLAG(UPSTREAM)),
  GST_EVENT_CUSTOM_UPSTREAM          = GST_EVENT_MAKE_TYPE (270, FLAG(UPSTREAM)),
  GST_EVENT_CUSTOM_DOWNSTREAM        = GST_EVENT_MAKE_TYPE (280, FLAG(DOWNSTREAM) | FLAG(SERIALIZED)),
  GST_EVENT_CUSTOM_DOWNSTREAM_OOB    = GST_EVENT_MAKE_TYPE (290, FLAG(DOWNSTREAM)),
  GST_EVENT_CUSTOM_DOWNSTREAM_STICKY = GST_EVENT_MAKE_TYPE (300, FLAG(DOWNSTREAM) | FLAG(SERIALIZED) | FLAG(STICKY) | FLAG(STICKY_MULTI)),
  GST_EVENT_CUSTOM_BOTH              = GST_EVENT_MAKE_TYPE (310, FLAG(BOTH) | FLAG(SERIALIZED)),
  GST_EVENT_CUSTOM_BOTH_OOB          = GST_EVENT_MAKE_TYPE (320, FLAG(BOTH))
} GstEventType;
#undef FLAG
G_BEGIN_DECLS
GST_EXPORT GType _gst_event_type;
#define GST_TYPE_EVENT                  (_gst_event_type)
#define GST_IS_EVENT(obj)               (GST_IS_MINI_OBJECT_TYPE (obj, GST_TYPE_EVENT))
#define GST_EVENT_CAST(obj)             ((GstEvent *)(obj))
#define GST_EVENT(obj)                  (GST_EVENT_CAST(obj))
#define GST_EVENT_TYPE(event)           (GST_EVENT_CAST(event)->type)
#define GST_EVENT_TYPE_NAME(event)      (gst_event_type_get_name(GST_EVENT_TYPE(event)))

#define GST_EVENT_TIMESTAMP(event)      (GST_EVENT_CAST(event)->timestamp)
#define GST_EVENT_SEQNUM(event)         (GST_EVENT_CAST(event)->seqnum)
#define GST_EVENT_IS_UPSTREAM(ev)       !!(GST_EVENT_TYPE (ev) & GST_EVENT_TYPE_UPSTREAM)
#define GST_EVENT_IS_DOWNSTREAM(ev)     !!(GST_EVENT_TYPE (ev) & GST_EVENT_TYPE_DOWNSTREAM)
#define GST_EVENT_IS_SERIALIZED(ev)     !!(GST_EVENT_TYPE (ev) & GST_EVENT_TYPE_SERIALIZED)
#define GST_EVENT_IS_STICKY(ev)     !!(GST_EVENT_TYPE (ev) & GST_EVENT_TYPE_STICKY)
#define         gst_event_is_writable(ev)     gst_mini_object_is_writable (GST_MINI_OBJECT_CAST (ev))
#define         gst_event_make_writable(ev)   GST_EVENT_CAST (gst_mini_object_make_writable (GST_MINI_OBJECT_CAST (ev)))
static inline gboolean gst_event_replace (GstEvent **old_event, GstEvent *new_event) {
  return gst_mini_object_replace ((GstMiniObject **) old_event, (GstMiniObject *) new_event);
}
static inline GstEvent *gst_event_steal (GstEvent **old_event) {
  return GST_EVENT_CAST (gst_mini_object_steal ((GstMiniObject **) old_event));
}
static inline gboolean gst_event_take (GstEvent **old_event, GstEvent *new_event) {
  return gst_mini_object_take ((GstMiniObject **) old_event, (GstMiniObject *) new_event);
}
typedef enum {
  GST_QOS_TYPE_OVERFLOW        = 0,
  GST_QOS_TYPE_UNDERFLOW       = 1,
  GST_QOS_TYPE_THROTTLE        = 2
} GstQOSType;
typedef enum {
  GST_STREAM_FLAG_NONE,
  GST_STREAM_FLAG_SPARSE       = (1 << 0),
  GST_STREAM_FLAG_SELECT       = (1 << 1),
  GST_STREAM_FLAG_UNSELECT     = (1 << 2)
} GstStreamFlags;
struct _GstEvent {
  GstMiniObject mini_object;
  GstEventType  type;
  guint64       timestamp;
  guint32       seqnum;
};
const gchar*    gst_event_type_get_name         (GstEventType type);
GQuark          gst_event_type_to_quark         (GstEventType type);
GstEventTypeFlags gst_event_type_get_flags        (GstEventType type);
static inline GstEvent *gst_event_ref (GstEvent * event) {
  return (GstEvent *) gst_mini_object_ref (GST_MINI_OBJECT_CAST (event));
}
static inline void gst_event_unref (GstEvent * event) {
  gst_mini_object_unref (GST_MINI_OBJECT_CAST (event));
}
static inline GstEvent *gst_event_copy (const GstEvent * event) {
  return GST_EVENT_CAST (gst_mini_object_copy (GST_MINI_OBJECT_CONST_CAST (event)));
}
GType           gst_event_get_type              (void);
GstEvent*       gst_event_new_custom            (GstEventType type, GstStructure *structure) G_GNUC_MALLOC;
const GstStructure *gst_event_get_structure         (GstEvent *event);
GstStructure *  gst_event_writable_structure    (GstEvent *event);
gboolean        gst_event_has_name              (GstEvent *event, const gchar *name);
guint32         gst_event_get_seqnum            (GstEvent *event);
void            gst_event_set_seqnum            (GstEvent *event, guint32 seqnum);
gint64          gst_event_get_running_time_offset (GstEvent *event);
void            gst_event_set_running_time_offset (GstEvent *event, gint64 offset);
GstEvent *      gst_event_new_stream_start      (const gchar *stream_id) G_GNUC_MALLOC;
void            gst_event_parse_stream_start    (GstEvent *event, const gchar **stream_id);
void            gst_event_set_stream_flags      (GstEvent *event, GstStreamFlags flags);
void            gst_event_parse_stream_flags    (GstEvent *event, GstStreamFlags *flags);
void            gst_event_set_group_id          (GstEvent *event, guint group_id);
gboolean        gst_event_parse_group_id        (GstEvent *event, guint *group_id);
GstEvent *      gst_event_new_flush_start       (void) G_GNUC_MALLOC;
GstEvent *      gst_event_new_flush_stop        (gboolean reset_time) G_GNUC_MALLOC;
void            gst_event_parse_flush_stop      (GstEvent *event, gboolean *reset_time);
GstEvent *      gst_event_new_eos               (void) G_GNUC_MALLOC;
GstEvent *      gst_event_new_gap               (GstClockTime   timestamp, GstClockTime   duration) G_GNUC_MALLOC;
void            gst_event_parse_gap             (GstEvent     * event, GstClockTime * timestamp, GstClockTime * duration);
GstEvent *      gst_event_new_caps              (GstCaps *caps) G_GNUC_MALLOC;
void            gst_event_parse_caps            (GstEvent *event, GstCaps **caps);
GstEvent*       gst_event_new_segment           (const GstSegment *segment) G_GNUC_MALLOC;
void            gst_event_parse_segment         (GstEvent *event, const GstSegment **segment);
void            gst_event_copy_segment          (GstEvent *event, GstSegment *segment);
GstEvent*       gst_event_new_tag               (GstTagList *taglist) G_GNUC_MALLOC;
void            gst_event_parse_tag             (GstEvent *event, GstTagList **taglist);
GstEvent*      gst_event_new_toc                (GstToc *toc, gboolean updated);
void           gst_event_parse_toc              (GstEvent *event, GstToc **toc, gboolean *updated);
GstEvent *     gst_event_new_protection         (const gchar * system_id, GstBuffer * data, const gchar * origin);
void           gst_event_parse_protection       (GstEvent * event, const gchar ** system_id, GstBuffer ** data, const gchar ** origin);
GstEvent *      gst_event_new_buffer_size       (GstFormat format, gint64 minsize, gint64 maxsize, gboolean async) G_GNUC_MALLOC;
void            gst_event_parse_buffer_size     (GstEvent *event, GstFormat *format, gint64 *minsize, gint64 *maxsize, gboolean *async);
GstEvent*       gst_event_new_sink_message      (const gchar *name, GstMessage *msg) G_GNUC_MALLOC;
void            gst_event_parse_sink_message    (GstEvent *event, GstMessage **msg);
GstEvent*       gst_event_new_qos(GstQOSType type, gdouble proportion, GstClockTimeDiff diff, GstClockTime timestamp) G_GNUC_MALLOC;
void            gst_event_parse_qos(GstEvent *event, GstQOSType *type, gdouble *proportion, GstClockTimeDiff *diff, GstClockTime *timestamp);
GstEvent *gst_event_new_seek(gdouble rate, GstFormat format, GstSeekFlags flags, GstSeekType start_type, gint64 start, GstSeekType stop_type,
                             gint64 stop) G_GNUC_MALLOC;
void gst_event_parse_seek(GstEvent *event, gdouble *rate, GstFormat *format, GstSeekFlags *flags, GstSeekType *start_type, gint64 *start,
                                                 GstSeekType *stop_type, gint64 *stop);
GstEvent*       gst_event_new_navigation        (GstStructure *structure) G_GNUC_MALLOC;
GstEvent*       gst_event_new_latency           (GstClockTime latency) G_GNUC_MALLOC;
void            gst_event_parse_latency         (GstEvent *event, GstClockTime *latency);
GstEvent *gst_event_new_step(GstFormat format, guint64 amount, gdouble rate, gboolean flush, gboolean intermediate) G_GNUC_MALLOC;
void gst_event_parse_step(GstEvent *event, GstFormat *format, guint64 *amount, gdouble *rate, gboolean *flush, gboolean *intermediate);
GstEvent*       gst_event_new_reconfigure       (void) G_GNUC_MALLOC;
GstEvent*       gst_event_new_toc_select        (const gchar *uid) G_GNUC_MALLOC;
void            gst_event_parse_toc_select      (GstEvent *event, gchar **uid);
GstEvent*       gst_event_new_segment_done      (GstFormat format, gint64 position) G_GNUC_MALLOC;
void            gst_event_parse_segment_done    (GstEvent *event, GstFormat *format, gint64 *position);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstEvent, gst_event_unref)
#endif
G_END_DECLS

#endif