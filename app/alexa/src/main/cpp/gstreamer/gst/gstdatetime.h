#ifndef __GST_DATE_TIME_H__
#define __GST_DATE_TIME_H__

#include <time.h>
#include <glib/glib.h>
#include <glib/glib-object.h>
#include "gstconfig.h"

G_BEGIN_DECLS
typedef struct _GstDateTime GstDateTime;
GST_EXPORT GType _gst_date_time_type;
#define GST_TYPE_DATE_TIME (_gst_date_time_type)
GType           gst_date_time_get_type (void);
gboolean        gst_date_time_has_year                (const GstDateTime * datetime);
gboolean        gst_date_time_has_month               (const GstDateTime * datetime);
gboolean        gst_date_time_has_day                 (const GstDateTime * datetime);
gboolean        gst_date_time_has_time                (const GstDateTime * datetime);
gboolean        gst_date_time_has_second              (const GstDateTime * datetime);
gint            gst_date_time_get_year                (const GstDateTime * datetime);
gint            gst_date_time_get_month               (const GstDateTime * datetime);
gint            gst_date_time_get_day                 (const GstDateTime * datetime);
gint            gst_date_time_get_hour                (const GstDateTime * datetime);
gint            gst_date_time_get_minute              (const GstDateTime * datetime);
gint            gst_date_time_get_second              (const GstDateTime * datetime);
gint            gst_date_time_get_microsecond         (const GstDateTime * datetime);
gfloat          gst_date_time_get_time_zone_offset    (const GstDateTime * datetime);
GstDateTime *   gst_date_time_new_from_unix_epoch_local_time (gint64 secs) G_GNUC_MALLOC;
GstDateTime *   gst_date_time_new_from_unix_epoch_utc   (gint64 secs) G_GNUC_MALLOC;
GstDateTime *   gst_date_time_new_local_time(gint year, gint month, gint day, gint hour, gint minute, gdouble seconds) G_GNUC_MALLOC;
GstDateTime *   gst_date_time_new_y                     (gint year) G_GNUC_MALLOC;
GstDateTime *   gst_date_time_new_ym                    (gint year, gint month) G_GNUC_MALLOC;
GstDateTime *   gst_date_time_new_ymd                   (gint year, gint month, gint day) G_GNUC_MALLOC;
GstDateTime *   gst_date_time_new(gfloat tzoffset, gint year, gint month, gint day, gint hour, gint minute, gdouble seconds) G_GNUC_MALLOC;
GstDateTime *   gst_date_time_new_now_local_time (void) G_GNUC_MALLOC;
GstDateTime *   gst_date_time_new_now_utc        (void) G_GNUC_MALLOC;
gchar *         gst_date_time_to_iso8601_string  (GstDateTime * datetime) G_GNUC_MALLOC;
GstDateTime *   gst_date_time_new_from_iso8601_string  (const gchar * string) G_GNUC_MALLOC;
GDateTime *     gst_date_time_to_g_date_time       (GstDateTime * datetime);
GstDateTime *   gst_date_time_new_from_g_date_time (GDateTime * dt);
GstDateTime *   gst_date_time_ref                (GstDateTime * datetime);
void            gst_date_time_unref              (GstDateTime * datetime);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstDateTime, gst_date_time_unref)
#endif
G_END_DECLS

#endif