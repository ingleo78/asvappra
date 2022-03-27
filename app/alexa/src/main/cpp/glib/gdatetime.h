#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_DATE_TIME_H__
#define __G_DATE_TIME_H__

#include "gmacros.h"
#include "gtypes.h"
#include "gtimezone.h"
#include "glib-basic-types.h"

G_BEGIN_DECLS
#define G_TIME_SPAN_DAY (G_GINT64_CONSTANT (86400000000))
#define G_TIME_SPAN_HOUR (G_GINT64_CONSTANT (3600000000))
#define G_TIME_SPAN_MINUTE (G_GINT64_CONSTANT (60000000))
#define G_TIME_SPAN_SECOND (G_GINT64_CONSTANT (1000000))
#define G_TIME_SPAN_MILLISECOND (G_GINT64_CONSTANT (1000))
typedef gint64 GTimeSpan;
typedef struct _GDateTime GDateTime;
void g_date_time_unref(GDateTime *datetime);
GDateTime* g_date_time_ref(GDateTime *datetime);
GDateTime* g_date_time_new_now(GTimeZone *tz);
GDateTime* g_date_time_new_now_local(void);
GDateTime* g_date_time_new_now_utc(void);
GDateTime* g_date_time_new_from_unix_local(gint64 t);
GDateTime* g_date_time_new_from_unix_utc(gint64 t);
GDateTime* g_date_time_new_from_timeval_local(const GTimeVal *tv);
GDateTime* g_date_time_new_from_timeval_utc(const GTimeVal *tv);
GDateTime* g_date_time_new(GTimeZone *tz, gint year, gint month, gint day, gint hour, gint minute, gdouble seconds);
GDateTime* g_date_time_new_local(gint year, gint month, gint day, gint hour, gint minute, gdouble seconds);
GDateTime* g_date_time_new_utc(gint year, gint month, gint day, gint hour, gint minute, gdouble seconds);
G_GNUC_WARN_UNUSED_RESULT GDateTime* g_date_time_add(GDateTime *datetime, GTimeSpan timespan);
G_GNUC_WARN_UNUSED_RESULT GDateTime* g_date_time_add_years(GDateTime *datetime, gint years);
G_GNUC_WARN_UNUSED_RESULT GDateTime* g_date_time_add_months(GDateTime *datetime, gint months);
G_GNUC_WARN_UNUSED_RESULT GDateTime* g_date_time_add_weeks(GDateTime *datetime, gint weeks);
G_GNUC_WARN_UNUSED_RESULT GDateTime* g_date_time_add_days(GDateTime *datetime, gint days);
G_GNUC_WARN_UNUSED_RESULT GDateTime* g_date_time_add_hours(GDateTime *datetime, gint hours);
G_GNUC_WARN_UNUSED_RESULT GDateTime* g_date_time_add_minutes(GDateTime *datetime, gint minutes);
G_GNUC_WARN_UNUSED_RESULT GDateTime* g_date_time_add_seconds(GDateTime *datetime, gdouble seconds);
G_GNUC_WARN_UNUSED_RESULT GDateTime* g_date_time_add_full(GDateTime *datetime, gint years, gint months, gint days, gint hours, gint minutes, gdouble seconds);
gint g_date_time_compare(gconstpointer dt1, gconstpointer dt2);
GTimeSpan g_date_time_difference(GDateTime *end, GDateTime *begin);
guint g_date_time_hash(gconstpointer datetime);
gboolean g_date_time_equal(gconstpointer dt1, gconstpointer dt2);
void g_date_time_get_ymd(GDateTime *datetime, gint *year, gint *month, gint *day);
gint g_date_time_get_year(GDateTime *datetime);
gint g_date_time_get_month(GDateTime *datetime);
gint g_date_time_get_day_of_month(GDateTime *datetime);
gint g_date_time_get_week_numbering_year(GDateTime *datetime);
gint g_date_time_get_week_of_year(GDateTime *datetime);
gint g_date_time_get_day_of_week(GDateTime *datetime);
gint g_date_time_get_day_of_year(GDateTime *datetime);
gint g_date_time_get_hour(GDateTime *datetime);
gint g_date_time_get_minute(GDateTime *datetime);
gint g_date_time_get_second(GDateTime *datetime);
gint g_date_time_get_microsecond(GDateTime *datetime);
gdouble g_date_time_get_seconds(GDateTime *datetime);
gint64 g_date_time_to_unix(GDateTime *datetime);
gboolean g_date_time_to_timeval(GDateTime *datetime, GTimeVal *tv);
GTimeSpan g_date_time_get_utc_offset(GDateTime *datetime);
const gchar* g_date_time_get_timezone_abbreviation(GDateTime *datetime);
gboolean g_date_time_is_daylight_savings(GDateTime *datetime);
GDateTime* g_date_time_to_timezone(GDateTime *datetime, GTimeZone *tz);
GDateTime* g_date_time_to_local(GDateTime *datetime);
GDateTime* g_date_time_to_utc(GDateTime *datetime);
gchar* g_date_time_format(GDateTime *datetime, const gchar *format) G_GNUC_MALLOC;
G_END_DECLS

#endif