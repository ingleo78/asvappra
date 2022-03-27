#ifndef __GST_CLOCK_H__
#define __GST_CLOCK_H__

#include <glib/glib.h>
#include "gstconfig.h"
#include "gstobject.h"

G_BEGIN_DECLS
#define GST_TYPE_CLOCK                  (gst_clock_get_type ())
#define GST_CLOCK(clock)                (G_TYPE_CHECK_INSTANCE_CAST ((clock), GST_TYPE_CLOCK, GstClock))
#define GST_IS_CLOCK(clock)             (G_TYPE_CHECK_INSTANCE_TYPE ((clock), GST_TYPE_CLOCK))
#define GST_CLOCK_CLASS(cclass)         (G_TYPE_CHECK_CLASS_CAST ((cclass), GST_TYPE_CLOCK, GstClockClass))
#define GST_IS_CLOCK_CLASS(cclass)      (G_TYPE_CHECK_CLASS_TYPE ((cclass), GST_TYPE_CLOCK))
#define GST_CLOCK_GET_CLASS(clock)      (G_TYPE_INSTANCE_GET_CLASS ((clock), GST_TYPE_CLOCK, GstClockClass))
#define GST_CLOCK_CAST(clock)           ((GstClock*)(clock))
typedef guint64 GstClockTime;
#define GST_TYPE_CLOCK_TIME G_TYPE_UINT64
typedef gint64 GstClockTimeDiff;
typedef gpointer GstClockID;
#define GST_CLOCK_TIME_NONE             ((GstClockTime) -1)
#define GST_CLOCK_TIME_IS_VALID(time)   (((GstClockTime)(time)) != GST_CLOCK_TIME_NONE)
#define GST_CLOCK_STIME_NONE             G_MININT64
#define GST_CLOCK_STIME_IS_VALID(time)   (((GstClockTimeDiff)(time)) != GST_CLOCK_STIME_NONE)
#define GST_SECOND  (G_USEC_PER_SEC * G_GINT64_CONSTANT (1000))
#define GST_MSECOND (GST_SECOND / G_GINT64_CONSTANT (1000))
#define GST_USECOND (GST_SECOND / G_GINT64_CONSTANT (1000000))
#define GST_NSECOND (GST_SECOND / G_GINT64_CONSTANT (1000000000))
#define GST_TIME_AS_SECONDS(time)  ((time) / GST_SECOND)
#define GST_TIME_AS_MSECONDS(time) ((time) / G_GINT64_CONSTANT (1000000))
#define GST_TIME_AS_USECONDS(time) ((time) / G_GINT64_CONSTANT (1000))
#define GST_TIME_AS_NSECONDS(time) (time)
#define GST_CLOCK_DIFF(s, e)            (GstClockTimeDiff)((e) - (s))
#define GST_TIMEVAL_TO_TIME(tv)         (GstClockTime)((tv).tv_sec * GST_SECOND + (tv).tv_usec * GST_USECOND)
#define GST_TIME_TO_TIMEVAL(t,tv)                               \
G_STMT_START {                                                  \
  g_assert ("Value of time " #t " is out of timeval's range" && \
      ((t) / GST_SECOND) < G_MAXLONG);                          \
  (tv).tv_sec  = (glong) (((GstClockTime) (t)) / GST_SECOND);   \
  (tv).tv_usec = (glong) ((((GstClockTime) (t)) -               \
                  ((GstClockTime) (tv).tv_sec) * GST_SECOND)    \
                 / GST_USECOND);                                \
} G_STMT_END
#define GST_TIMESPEC_TO_TIME(ts)        (GstClockTime)((ts).tv_sec * GST_SECOND + (ts).tv_nsec * GST_NSECOND)
#define GST_TIME_TO_TIMESPEC(t,ts)                                \
G_STMT_START {                                                    \
  g_assert ("Value of time " #t " is out of timespec's range" &&  \
      ((t) / GST_SECOND) < G_MAXLONG);                            \
  (ts).tv_sec  =  (glong) ((t) / GST_SECOND);                     \
  (ts).tv_nsec = (glong) (((t) - (ts).tv_sec * GST_SECOND) / GST_NSECOND);        \
} G_STMT_END
#define GST_TIME_FORMAT "u:%02u:%02u.%09u"
#define GST_TIME_ARGS(t) \
        GST_CLOCK_TIME_IS_VALID (t) ? \
        (guint) (((GstClockTime)(t)) / (GST_SECOND * 60 * 60)) : 99, \
        GST_CLOCK_TIME_IS_VALID (t) ? \
        (guint) ((((GstClockTime)(t)) / (GST_SECOND * 60)) % 60) : 99, \
        GST_CLOCK_TIME_IS_VALID (t) ? \
        (guint) ((((GstClockTime)(t)) / GST_SECOND) % 60) : 99, \
        GST_CLOCK_TIME_IS_VALID (t) ? \
        (guint) (((GstClockTime)(t)) % GST_SECOND) : 999999999
#define GST_STIME_FORMAT "c%" GST_TIME_FORMAT
#define GST_STIME_ARGS(t)						\
  ((t) == GST_CLOCK_STIME_NONE || (t) >= 0) ? '+' : '-',		\
    GST_CLOCK_STIME_IS_VALID (t) ?					\
    (guint) (((GstClockTime)(ABS(t))) / (GST_SECOND * 60 * 60)) : 99,	\
    GST_CLOCK_STIME_IS_VALID (t) ?					\
    (guint) ((((GstClockTime)(ABS(t))) / (GST_SECOND * 60)) % 60) : 99,	\
    GST_CLOCK_STIME_IS_VALID (t) ?					\
    (guint) ((((GstClockTime)(ABS(t))) / GST_SECOND) % 60) : 99,	\
    GST_CLOCK_STIME_IS_VALID (t) ?					\
    (guint) (((GstClockTime)(ABS(t))) % GST_SECOND) : 999999999
typedef struct _GstClockEntry   GstClockEntry;
typedef struct _GstClock        GstClock;
typedef struct _GstClockClass   GstClockClass;
typedef struct _GstClockPrivate GstClockPrivate;
typedef gboolean        (*GstClockCallback)     (GstClock *clock, GstClockTime time, GstClockID id, gpointer user_data);
typedef enum {
  GST_CLOCK_OK          =  0,
  GST_CLOCK_EARLY       =  1,
  GST_CLOCK_UNSCHEDULED =  2,
  GST_CLOCK_BUSY        =  3,
  GST_CLOCK_BADTIME     =  4,
  GST_CLOCK_ERROR       =  5,
  GST_CLOCK_UNSUPPORTED =  6,
  GST_CLOCK_DONE        =  7
} GstClockReturn;
typedef enum {
  GST_CLOCK_ENTRY_SINGLE,
  GST_CLOCK_ENTRY_PERIODIC
} GstClockEntryType;
#define GST_CLOCK_ENTRY(entry)          ((GstClockEntry *)(entry))
#define GST_CLOCK_ENTRY_CLOCK(entry)    ((entry)->clock)
#define GST_CLOCK_ENTRY_TYPE(entry)     ((entry)->type)
#define GST_CLOCK_ENTRY_TIME(entry)     ((entry)->time)
#define GST_CLOCK_ENTRY_INTERVAL(entry) ((entry)->interval)
#define GST_CLOCK_ENTRY_STATUS(entry)   ((entry)->status)
struct _GstClockEntry {
  gint                  refcount;
  GstClock              *clock;
  GstClockEntryType      type;
  GstClockTime           time;
  GstClockTime           interval;
  GstClockReturn         status;
  GstClockCallback       func;
  gpointer               user_data;
  GDestroyNotify         destroy_data;
  gboolean               unscheduled;
  gboolean               woken_up;
  gpointer _gst_reserved[GST_PADDING];
};
typedef enum {
  GST_CLOCK_FLAG_CAN_DO_SINGLE_SYNC     = (GST_OBJECT_FLAG_LAST << 0),
  GST_CLOCK_FLAG_CAN_DO_SINGLE_ASYNC    = (GST_OBJECT_FLAG_LAST << 1),
  GST_CLOCK_FLAG_CAN_DO_PERIODIC_SYNC   = (GST_OBJECT_FLAG_LAST << 2),
  GST_CLOCK_FLAG_CAN_DO_PERIODIC_ASYNC  = (GST_OBJECT_FLAG_LAST << 3),
  GST_CLOCK_FLAG_CAN_SET_RESOLUTION     = (GST_OBJECT_FLAG_LAST << 4),
  GST_CLOCK_FLAG_CAN_SET_MASTER         = (GST_OBJECT_FLAG_LAST << 5),
  GST_CLOCK_FLAG_NEEDS_STARTUP_SYNC     = (GST_OBJECT_FLAG_LAST << 6),
  GST_CLOCK_FLAG_LAST                   = (GST_OBJECT_FLAG_LAST << 8)
} GstClockFlags;
#define GST_CLOCK_FLAGS(clock)  GST_OBJECT_FLAGS(clock)
struct _GstClock {
  GstObject      object;
  GstClockPrivate *priv;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstClockClass {
  GstObjectClass        parent_class;
  GstClockTime          (*change_resolution)    (GstClock *clock,
                                                 GstClockTime old_resolution,
                                                 GstClockTime new_resolution);
  GstClockTime          (*get_resolution)       (GstClock *clock);
  GstClockTime          (*get_internal_time)    (GstClock *clock);
  GstClockReturn        (*wait)                 (GstClock *clock, GstClockEntry *entry,
                                                 GstClockTimeDiff *jitter);
  GstClockReturn        (*wait_async)           (GstClock *clock, GstClockEntry *entry);
  void                  (*unschedule)           (GstClock *clock, GstClockEntry *entry);
  gpointer _gst_reserved[GST_PADDING];
};
GType                   gst_clock_get_type              (void);
GstClockTime            gst_clock_set_resolution        (GstClock *clock, GstClockTime resolution);
GstClockTime            gst_clock_get_resolution        (GstClock *clock);
GstClockTime            gst_clock_get_time              (GstClock *clock);
void                    gst_clock_set_calibration       (GstClock *clock, GstClockTime internal, GstClockTime external, GstClockTime rate_num,
                                                         GstClockTime rate_denom);
void                    gst_clock_get_calibration       (GstClock *clock, GstClockTime *internal, GstClockTime *external, GstClockTime *rate_num,
                                                         GstClockTime *rate_denom);
gboolean                gst_clock_set_master            (GstClock *clock, GstClock *master);
GstClock*               gst_clock_get_master            (GstClock *clock);
void                    gst_clock_set_timeout           (GstClock *clock,
                                                         GstClockTime timeout);
GstClockTime            gst_clock_get_timeout           (GstClock *clock);
gboolean                gst_clock_add_observation       (GstClock *clock, GstClockTime slave,
                                                         GstClockTime master, gdouble *r_squared);
gboolean                gst_clock_add_observation_unapplied (GstClock *clock, GstClockTime slave,
                                                         GstClockTime master, gdouble *r_squared,
                                                         GstClockTime *internal,
                                                         GstClockTime *external,
                                                         GstClockTime *rate_num,
                                                         GstClockTime *rate_denom);
GstClockTime            gst_clock_get_internal_time     (GstClock *clock);
GstClockTime            gst_clock_adjust_unlocked       (GstClock *clock, GstClockTime internal);
GstClockTime            gst_clock_adjust_with_calibration (GstClock *clock,
                                                         GstClockTime internal_target,
                                                         GstClockTime cinternal,
                                                         GstClockTime cexternal,
                                                         GstClockTime cnum,
                                                         GstClockTime cdenom);
GstClockTime            gst_clock_unadjust_with_calibration (GstClock *clock,
                                                         GstClockTime external_target,
                                                         GstClockTime cinternal,
                                                         GstClockTime cexternal,
                                                         GstClockTime cnum,
                                                         GstClockTime cdenom);
GstClockTime            gst_clock_unadjust_unlocked     (GstClock * clock, GstClockTime external);
gboolean                gst_clock_wait_for_sync         (GstClock * clock, GstClockTime timeout);
gboolean                gst_clock_is_synced             (GstClock * clock);
void                    gst_clock_set_synced            (GstClock * clock, gboolean synced);
GstClockID              gst_clock_new_single_shot_id    (GstClock *clock, GstClockTime time);
GstClockID              gst_clock_new_periodic_id       (GstClock *clock, GstClockTime start_time, GstClockTime interval);
GstClockID              gst_clock_id_ref                (GstClockID id);
void                    gst_clock_id_unref              (GstClockID id);
gint                    gst_clock_id_compare_func       (gconstpointer id1, gconstpointer id2);
GstClockTime            gst_clock_id_get_time           (GstClockID id);
GstClockReturn          gst_clock_id_wait               (GstClockID id, GstClockTimeDiff *jitter);
GstClockReturn          gst_clock_id_wait_async         (GstClockID id, GstClockCallback func, gpointer user_data, GDestroyNotify destroy_data);
void                    gst_clock_id_unschedule         (GstClockID id);
gboolean                gst_clock_single_shot_id_reinit (GstClock * clock, GstClockID id, GstClockTime time);
gboolean                gst_clock_periodic_id_reinit    (GstClock * clock, GstClockID id, GstClockTime start_time, GstClockTime interval);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstClock, gst_object_unref)
#endif
G_END_DECLS

#endif