#ifndef __GST_MESSAGE_H__
#define __GST_MESSAGE_H__

#include "gstminiobject.h"
#include "gstobject.h"
#include "gsttaglist.h"
#include "gstquery.h"
#include "gsttoc.h"
#include "gstdevice.h"
#include "gstcontext.h"

G_BEGIN_DECLS
typedef struct _GstMessage GstMessage;
typedef enum {
  GST_MESSAGE_UNKNOWN           = 0,
  GST_MESSAGE_EOS               = (1 << 0),
  GST_MESSAGE_ERROR             = (1 << 1),
  GST_MESSAGE_WARNING           = (1 << 2),
  GST_MESSAGE_INFO              = (1 << 3),
  GST_MESSAGE_TAG               = (1 << 4),
  GST_MESSAGE_BUFFERING         = (1 << 5),
  GST_MESSAGE_STATE_CHANGED     = (1 << 6),
  GST_MESSAGE_STATE_DIRTY       = (1 << 7),
  GST_MESSAGE_STEP_DONE         = (1 << 8),
  GST_MESSAGE_CLOCK_PROVIDE     = (1 << 9),
  GST_MESSAGE_CLOCK_LOST        = (1 << 10),
  GST_MESSAGE_NEW_CLOCK         = (1 << 11),
  GST_MESSAGE_STRUCTURE_CHANGE  = (1 << 12),
  GST_MESSAGE_STREAM_STATUS     = (1 << 13),
  GST_MESSAGE_APPLICATION       = (1 << 14),
  GST_MESSAGE_ELEMENT           = (1 << 15),
  GST_MESSAGE_SEGMENT_START     = (1 << 16),
  GST_MESSAGE_SEGMENT_DONE      = (1 << 17),
  GST_MESSAGE_DURATION_CHANGED  = (1 << 18),
  GST_MESSAGE_LATENCY           = (1 << 19),
  GST_MESSAGE_ASYNC_START       = (1 << 20),
  GST_MESSAGE_ASYNC_DONE        = (1 << 21),
  GST_MESSAGE_REQUEST_STATE     = (1 << 22),
  GST_MESSAGE_STEP_START        = (1 << 23),
  GST_MESSAGE_QOS               = (1 << 24),
  GST_MESSAGE_PROGRESS          = (1 << 25),
  GST_MESSAGE_TOC               = (1 << 26),
  GST_MESSAGE_RESET_TIME        = (1 << 27),
  GST_MESSAGE_STREAM_START      = (1 << 28),
  GST_MESSAGE_NEED_CONTEXT      = (1 << 29),
  GST_MESSAGE_HAVE_CONTEXT      = (1 << 30),
  GST_MESSAGE_EXTENDED          = (1 << 31),
  GST_MESSAGE_DEVICE_ADDED      = GST_MESSAGE_EXTENDED + 1,
  GST_MESSAGE_DEVICE_REMOVED    = GST_MESSAGE_EXTENDED + 2,
  GST_MESSAGE_ANY               = (gint) (0xffffffff)
} GstMessageType;
GST_EXPORT GType _gst_message_type;
#define GST_TYPE_MESSAGE                         (_gst_message_type)
#define GST_IS_MESSAGE(obj)                      (GST_IS_MINI_OBJECT_TYPE (obj, GST_TYPE_MESSAGE))
#define GST_MESSAGE_CAST(obj)                    ((GstMessage*)(obj))
#define GST_MESSAGE(obj)                         (GST_MESSAGE_CAST(obj))
#define GST_MESSAGE_GET_LOCK(message)   (&GST_MESSAGE_CAST(message)->lock)
#define GST_MESSAGE_LOCK(message)       g_mutex_lock(GST_MESSAGE_GET_LOCK(message))
#define GST_MESSAGE_UNLOCK(message)     g_mutex_unlock(GST_MESSAGE_GET_LOCK(message))
#define GST_MESSAGE_GET_COND(message)   (&GST_MESSAGE_CAST(message)->cond)
#define GST_MESSAGE_WAIT(message)       g_cond_wait(GST_MESSAGE_GET_COND(message),GST_MESSAGE_GET_LOCK(message))
#define GST_MESSAGE_SIGNAL(message)     g_cond_signal(GST_MESSAGE_GET_COND(message))
#define GST_MESSAGE_TYPE(message)       (GST_MESSAGE_CAST(message)->type)
#define GST_MESSAGE_TYPE_IS_EXTENDED(message)       (!!(GST_MESSAGE_CAST(message)->type & GST_MESSAGE_EXTENDED))
#define GST_MESSAGE_TYPE_NAME(message)  gst_message_type_get_name(GST_MESSAGE_TYPE(message))
#define GST_MESSAGE_TIMESTAMP(message)  (GST_MESSAGE_CAST(message)->timestamp)
#define GST_MESSAGE_SRC(message)        (GST_MESSAGE_CAST(message)->src)
#define GST_MESSAGE_SEQNUM(message)     (GST_MESSAGE_CAST(message)->seqnum)
#define GST_MESSAGE_SRC_NAME(message)   (GST_MESSAGE_SRC(message) ? GST_OBJECT_NAME (GST_MESSAGE_SRC(message)) : "(NULL)")
typedef enum {
  GST_STRUCTURE_CHANGE_TYPE_PAD_LINK   = 0,
  GST_STRUCTURE_CHANGE_TYPE_PAD_UNLINK = 1
} GstStructureChangeType;
typedef enum {
  GST_STREAM_STATUS_TYPE_CREATE   = 0,
  GST_STREAM_STATUS_TYPE_ENTER    = 1,
  GST_STREAM_STATUS_TYPE_LEAVE    = 2,
  GST_STREAM_STATUS_TYPE_DESTROY  = 3,
  GST_STREAM_STATUS_TYPE_START    = 8,
  GST_STREAM_STATUS_TYPE_PAUSE    = 9,
  GST_STREAM_STATUS_TYPE_STOP     = 10
} GstStreamStatusType;
typedef enum {
  GST_PROGRESS_TYPE_START    = 0,
  GST_PROGRESS_TYPE_CONTINUE = 1,
  GST_PROGRESS_TYPE_COMPLETE = 2,
  GST_PROGRESS_TYPE_CANCELED = 3,
  GST_PROGRESS_TYPE_ERROR    = 4
} GstProgressType;
struct _GstMessage {
  GstMiniObject   mini_object;
  GstMessageType  type;
  guint64         timestamp;
  GstObject      *src;
  guint32         seqnum;
  GMutex          lock;
  GCond           cond;
};
GType           gst_message_get_type            (void);
const gchar*    gst_message_type_get_name       (GstMessageType type);
GQuark          gst_message_type_to_quark       (GstMessageType type);
static inline GstMessage *gst_message_ref (GstMessage * msg) {
  return (GstMessage *) gst_mini_object_ref (GST_MINI_OBJECT_CAST (msg));
}
static inline void gst_message_unref(GstMessage * msg) {
  gst_mini_object_unref (GST_MINI_OBJECT_CAST (msg));
}
static inline GstMessage *gst_message_copy(const GstMessage * msg) {
  return GST_MESSAGE_CAST (gst_mini_object_copy (GST_MINI_OBJECT_CONST_CAST (msg)));
}
#define         gst_message_is_writable(msg)     gst_mini_object_is_writable (GST_MINI_OBJECT_CAST (msg))
#define         gst_message_make_writable(msg)  GST_MESSAGE_CAST (gst_mini_object_make_writable (GST_MINI_OBJECT_CAST (msg)))
static inline gboolean gst_message_replace(GstMessage **old_message, GstMessage *new_message) {
  return gst_mini_object_replace ((GstMiniObject **) old_message, (GstMiniObject *) new_message);
}
GstMessage *    gst_message_new_custom(GstMessageType type, GstObject *src, GstStructure *structure) G_GNUC_MALLOC;
const GstStructure *gst_message_get_structure       (GstMessage *message);
gboolean        gst_message_has_name            (GstMessage *message, const gchar *name);
guint32         gst_message_get_seqnum          (GstMessage *message);
void            gst_message_set_seqnum          (GstMessage *message, guint32 seqnum);
GstMessage *    gst_message_new_eos             (GstObject * src) G_GNUC_MALLOC;
GstMessage *    gst_message_new_error           (GstObject * src, GError * error, const gchar * debug) G_GNUC_MALLOC;
void            gst_message_parse_error         (GstMessage *message, GError **gerror, gchar **debug);
GstMessage *    gst_message_new_warning         (GstObject * src, GError * error, const gchar * debug) G_GNUC_MALLOC;
void            gst_message_parse_warning       (GstMessage *message, GError **gerror, gchar **debug);
GstMessage *    gst_message_new_info            (GstObject * src, GError * error, const gchar * debug) G_GNUC_MALLOC;
void            gst_message_parse_info          (GstMessage *message, GError **gerror, gchar **debug);
GstMessage *    gst_message_new_tag             (GstObject * src, GstTagList * tag_list) G_GNUC_MALLOC;
void            gst_message_parse_tag           (GstMessage *message, GstTagList **tag_list);
GstMessage *    gst_message_new_buffering         (GstObject * src, gint percent) G_GNUC_MALLOC;
void            gst_message_parse_buffering       (GstMessage *message, gint *percent);
void            gst_message_set_buffering_stats   (GstMessage *message, GstBufferingMode mode, gint avg_in, gint avg_out,
                                                   gint64 buffering_left);
void            gst_message_parse_buffering_stats (GstMessage *message, GstBufferingMode *mode, gint *avg_in, gint *avg_out,
                                                   gint64 *buffering_left);
GstMessage *    gst_message_new_state_changed   (GstObject * src, GstState oldstate,
                                                 GstState newstate, GstState pending) G_GNUC_MALLOC;
void            gst_message_parse_state_changed (GstMessage *message, GstState *oldstate,
                                                 GstState *newstate, GstState *pending);
GstMessage *    gst_message_new_state_dirty     (GstObject * src) G_GNUC_MALLOC;
GstMessage *    gst_message_new_step_done       (GstObject * src, GstFormat format, guint64 amount,
                                                 gdouble rate, gboolean flush, gboolean intermediate,
                                                 guint64 duration, gboolean eos) G_GNUC_MALLOC;
void            gst_message_parse_step_done     (GstMessage * message, GstFormat *format, guint64 *amount,
                                                 gdouble *rate, gboolean *flush, gboolean *intermediate,
                                                 guint64 *duration, gboolean *eos);
GstMessage *    gst_message_new_clock_provide   (GstObject * src, GstClock *clock, gboolean ready) G_GNUC_MALLOC;
void            gst_message_parse_clock_provide (GstMessage *message, GstClock **clock,
                                                 gboolean *ready);
GstMessage *    gst_message_new_clock_lost      (GstObject * src, GstClock *clock) G_GNUC_MALLOC;
void            gst_message_parse_clock_lost    (GstMessage *message, GstClock **clock);
GstMessage *    gst_message_new_new_clock       (GstObject * src, GstClock *clock) G_GNUC_MALLOC;
void            gst_message_parse_new_clock     (GstMessage *message, GstClock **clock);
GstMessage *    gst_message_new_application     (GstObject * src, GstStructure * structure) G_GNUC_MALLOC;
GstMessage *    gst_message_new_element         (GstObject * src, GstStructure * structure) G_GNUC_MALLOC;
GstMessage *    gst_message_new_segment_start   (GstObject * src, GstFormat format, gint64 position) G_GNUC_MALLOC;
void            gst_message_parse_segment_start (GstMessage *message, GstFormat *format,
                                                 gint64 *position);
GstMessage *    gst_message_new_segment_done    (GstObject * src, GstFormat format, gint64 position) G_GNUC_MALLOC;
void            gst_message_parse_segment_done  (GstMessage *message, GstFormat *format,
                                                 gint64 *position);
GstMessage *    gst_message_new_duration_changed (GstObject * src) G_GNUC_MALLOC;
GstMessage *    gst_message_new_latency         (GstObject * src) G_GNUC_MALLOC;
GstMessage *    gst_message_new_async_start     (GstObject * src) G_GNUC_MALLOC;
GstMessage *    gst_message_new_async_done      (GstObject * src, GstClockTime running_time) G_GNUC_MALLOC;
void            gst_message_parse_async_done    (GstMessage *message, GstClockTime *running_time);
GstMessage *    gst_message_new_structure_change   (GstObject * src, GstStructureChangeType type,
                                                    GstElement *owner, gboolean busy) G_GNUC_MALLOC;
void            gst_message_parse_structure_change (GstMessage *message, GstStructureChangeType *type,
                                                    GstElement **owner, gboolean *busy);
GstMessage *    gst_message_new_stream_status        (GstObject * src, GstStreamStatusType type,
                                                      GstElement *owner) G_GNUC_MALLOC;
void            gst_message_parse_stream_status      (GstMessage *message, GstStreamStatusType *type,
                                                      GstElement **owner);
void            gst_message_set_stream_status_object (GstMessage *message, const GValue *object);
const GValue *  gst_message_get_stream_status_object (GstMessage *message);
GstMessage *    gst_message_new_request_state   (GstObject * src, GstState state) G_GNUC_MALLOC;
void            gst_message_parse_request_state (GstMessage * message, GstState *state);
GstMessage *    gst_message_new_step_start      (GstObject * src, gboolean active, GstFormat format,
                                                 guint64 amount, gdouble rate, gboolean flush,
                                                 gboolean intermediate) G_GNUC_MALLOC;
void            gst_message_parse_step_start    (GstMessage * message, gboolean *active, GstFormat *format,
                                                 guint64 *amount, gdouble *rate, gboolean *flush,
                                                 gboolean *intermediate);
GstMessage *    gst_message_new_qos             (GstObject * src, gboolean live, guint64 running_time,
                                                 guint64 stream_time, guint64 timestamp, guint64 duration) G_GNUC_MALLOC;
void            gst_message_set_qos_values      (GstMessage * message, gint64 jitter, gdouble proportion,
                                                 gint quality);
void            gst_message_set_qos_stats       (GstMessage * message, GstFormat format, guint64 processed,
                                                 guint64 dropped);
void            gst_message_parse_qos           (GstMessage * message, gboolean * live, guint64 * running_time,
                                                 guint64 * stream_time, guint64 * timestamp, guint64 * duration);
void            gst_message_parse_qos_values    (GstMessage * message, gint64 * jitter, gdouble * proportion,
                                                 gint * quality);
void            gst_message_parse_qos_stats     (GstMessage * message, GstFormat * format, guint64 * processed,
                                                 guint64 * dropped);
GstMessage *    gst_message_new_progress           (GstObject * src, GstProgressType type, const gchar *code,
                                                    const gchar *text) G_GNUC_MALLOC;
void            gst_message_parse_progress         (GstMessage * message, GstProgressType * type, gchar ** code,
                                                    gchar ** text);
GstMessage *    gst_message_new_toc             (GstObject *src, GstToc *toc, gboolean updated);
void            gst_message_parse_toc           (GstMessage *message, GstToc **toc, gboolean *updated);
GstMessage *    gst_message_new_reset_time      (GstObject * src, GstClockTime running_time) G_GNUC_MALLOC;
void            gst_message_parse_reset_time    (GstMessage *message, GstClockTime *running_time);
GstMessage *    gst_message_new_stream_start    (GstObject * src) G_GNUC_MALLOC;
void            gst_message_set_group_id        (GstMessage *message, guint group_id);
gboolean        gst_message_parse_group_id      (GstMessage *message, guint *group_id);
GstMessage *    gst_message_new_need_context    (GstObject * src, const gchar * context_type) G_GNUC_MALLOC;
gboolean        gst_message_parse_context_type  (GstMessage * message, const gchar ** context_type);
GstMessage *    gst_message_new_have_context    (GstObject * src, GstContext *context) G_GNUC_MALLOC;
void            gst_message_parse_have_context  (GstMessage *message, GstContext **context);
GstMessage *    gst_message_new_device_added    (GstObject * src, GstDevice * device) G_GNUC_MALLOC;
void            gst_message_parse_device_added  (GstMessage * message, GstDevice ** device);
GstMessage *    gst_message_new_device_removed    (GstObject * src, GstDevice * device) G_GNUC_MALLOC;
void            gst_message_parse_device_removed  (GstMessage * message, GstDevice ** device);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstMessage, gst_message_unref)
#endif
G_END_DECLS

#endif