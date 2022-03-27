#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "gdatetime.h"
#include "gatomic.h"
#include "gfileutils.h"
#include "ghash.h"
#include "gmain.h"
#include "gmappedfile.h"
#include "gstrfuncs.h"
#include "gtestutils.h"
#include "gthread.h"
#include "gtimezone.h"
#include "glibintl.h"
#ifndef G_OS_WIN32
#include <sys/time.h>
#include <time.h>
#endif

#define guint64 G_GNUC_EXTENSION unsigned long long
struct _GDateTime {
  gint32 days;
  guint64 usec;
  GTimeZone *tz;
  gint interval;
  volatile gint ref_count;
};
#define UNIX_EPOCH_START     719163
#define INSTANT_TO_UNIX(instant)  ((instant)/USEC_PER_SECOND - UNIX_EPOCH_START * SEC_PER_DAY)
#define UNIX_TO_INSTANT(unix)  (((unix) + UNIX_EPOCH_START * SEC_PER_DAY) * USEC_PER_SECOND)
#define DAYS_IN_4YEARS    1461
#define DAYS_IN_100YEARS  36524
#define DAYS_IN_400YEARS  146097
#define USEC_PER_SECOND  (G_GINT64_CONSTANT (1000000))
#define USEC_PER_MINUTE  (G_GINT64_CONSTANT (60000000))
#define USEC_PER_HOUR  (G_GINT64_CONSTANT (3600000000))
#define USEC_PER_MILLISECOND (G_GINT64_CONSTANT (1000))
#define USEC_PER_DAY  (G_GINT64_CONSTANT (86400000000))
#define SEC_PER_DAY  (G_GINT64_CONSTANT (86400))
#define GREGORIAN_LEAP(y)  ((((y) % 4) == 0) && (!((((y) % 100) == 0) && (((y) % 400) != 0))))
#define JULIAN_YEAR(d)  ((d)->julian / 365.25)
#define DAYS_PER_PERIOD  (G_GINT64_CONSTANT (2914695))
#define GET_AMPM(d,l) ((g_date_time_get_hour(d) < 12) ? (l ? C_("GDateTime", "am") : C_("GDateTime", "AM")) : (l ? C_("GDateTime", "pm") : C_("GDateTime", "PM")))
#define WEEKDAY_ABBR(d)  (get_weekday_name_abbr(g_date_time_get_day_of_week(datetime)))
#define WEEKDAY_FULL(d)  (get_weekday_name(g_date_time_get_day_of_week(datetime)))
#define MONTH_ABBR(d)  (get_month_name_abbr(g_date_time_get_month(datetime)))
#define MONTH_FULL(d)  (get_month_name(g_date_time_get_month(datetime)))
#define GET_PREFERRED_DATE(d) (g_date_time_format((d), C_("GDateTime", "%m/%d/%y")))
#define GET_PREFERRED_TIME(d) (g_date_time_format((d), C_("GDateTime", "%H:%M:%S")))
#define SECS_PER_MINUTE (60)
#define SECS_PER_HOUR  (60 * SECS_PER_MINUTE)
#define SECS_PER_DAY  (24 * SECS_PER_HOUR)
#define SECS_PER_YEAR   (365 * SECS_PER_DAY)
#define SECS_PER_JULIAN (DAYS_PER_PERIOD * SECS_PER_DAY)
static const guint16 days_in_months[2][13] = {
  { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};
static const guint16 days_in_year[2][13] = {
  {  0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
  {  0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};
static const gchar* get_month_name(gint month) {
  switch(month) {
      case 1: return C_("full month name", "January");
      case 2: return C_("full month name", "February");
      case 3: return C_("full month name", "March");
      case 4: return C_("full month name", "April");
      case 5: return C_("full month name", "May");
      case 6: return C_("full month name", "June");
      case 7: return C_("full month name", "July");
      case 8: return C_("full month name", "August");
      case 9: return C_("full month name", "September");
      case 10: return C_("full month name", "October");
      case 11: return C_("full month name", "November");
      case 12: return C_("full month name", "December");
      default: g_warning ("Invalid month number %d", month);
  }
  return NULL;
}
static const gchar* get_month_name_abbr(gint month) {
  switch(month) {
      case 1: return C_("abbreviated month name", "Jan");
      case 2: return C_("abbreviated month name", "Feb");
      case 3: return C_("abbreviated month name", "Mar");
      case 4: return C_("abbreviated month name", "Apr");
      case 5: return C_("abbreviated month name", "May");
      case 6: return C_("abbreviated month name", "Jun");
      case 7: return C_("abbreviated month name", "Jul");
      case 8: return C_("abbreviated month name", "Aug");
      case 9: return C_("abbreviated month name", "Sep");
      case 10: return C_("abbreviated month name", "Oct");
      case 11: return C_("abbreviated month name", "Nov");
      case 12: return C_("abbreviated month name", "Dec");
      default: g_warning ("Invalid month number %d", month);
  }
  return NULL;
}
static const gchar* get_weekday_name(gint day) {
  switch (day) {
      case 1: return C_("full weekday name", "Monday");
      case 2: return C_("full weekday name", "Tuesday");
      case 3: return C_("full weekday name", "Wednesday");
      case 4: return C_("full weekday name", "Thursday");
      case 5: return C_("full weekday name", "Friday");
      case 6: return C_("full weekday name", "Saturday");
      case 7: return C_("full weekday name", "Sunday");
      default: g_warning ("Invalid week day number %d", day);
  }
  return NULL;
}
static const gchar* get_weekday_name_abbr(gint day) {
  switch (day) {
      case 1: return C_("abbreviated weekday name", "Mon");
      case 2: return C_("abbreviated weekday name", "Tue");
      case 3: return C_("abbreviated weekday name", "Wed");
      case 4: return C_("abbreviated weekday name", "Thu");
      case 5: return C_("abbreviated weekday name", "Fri");
      case 6: return C_("abbreviated weekday name", "Sat");
      case 7: return C_("abbreviated weekday name", "Sun");
      default: g_warning ("Invalid week day number %d", day);
  }
  return NULL;
}
static inline gint ymd_to_days(gint year, gint month, gint day) {
  gint64 days;
  days = (year - 1) * 365 + ((year - 1) / 4) - ((year - 1) / 100) + ((year - 1) / 400);
  days += days_in_year[0][month - 1];
  if (GREGORIAN_LEAP(year) && month > 2) day++;
  days += day;
  return days;
}
static void g_date_time_get_week_number(GDateTime *datetime, gint *week_number, gint *day_of_week, gint *day_of_year) {
  gint a, b, c, d, e, f, g, n, s, month, day, year;
  g_date_time_get_ymd(datetime, &year, &month, &day);
  if (month <= 2) {
      a = g_date_time_get_year (datetime) - 1;
      b = (a / 4) - (a / 100) + (a / 400);
      c = ((a - 1) / 4) - ((a - 1) / 100) + ((a - 1) / 400);
      s = b - c;
      e = 0;
      f = day - 1 + (31 * (month - 1));
  } else {
      a = year;
      b = (a / 4) - (a / 100) + (a / 400);
      c = ((a - 1) / 4) - ((a - 1) / 100) + ((a - 1) / 400);
      s = b - c;
      e = s + 1;
      f = day + (((153 * (month - 3)) + 2) / 5) + 58 + s;
  }
  g = (a + b) % 7;
  d = (f + g - e) % 7;
  n = f + 3 - d;
  if (week_number) {
      if (n < 0) *week_number = 53 - ((g - s) / 5);
      else if (n > 364 + s) *week_number = 1;
      else *week_number = (n / 7) + 1;
  }
  if (day_of_week) *day_of_week = d + 1;
  if (day_of_year) *day_of_year = f + 1;
}
static GDateTime* g_date_time_alloc(GTimeZone *tz) {
  GDateTime *datetime;
  datetime = g_slice_new0(GDateTime);
  datetime->tz = g_time_zone_ref(tz);
  datetime->ref_count = 1;
  return datetime;
}
GDateTime* g_date_time_ref(GDateTime *datetime) {
  g_return_val_if_fail(datetime != NULL, NULL);
  g_return_val_if_fail(datetime->ref_count > 0, NULL);
  g_atomic_int_inc(&datetime->ref_count);
  return datetime;
}
void g_date_time_unref(GDateTime *datetime) {
  g_return_if_fail(datetime != NULL);
  g_return_if_fail(datetime->ref_count > 0);
  if (g_atomic_int_dec_and_test(&datetime->ref_count)) {
      g_time_zone_unref(datetime->tz);
      g_slice_free(GDateTime, datetime);
  }
}
static gint64 g_date_time_to_instant(GDateTime *datetime) {
  gint64 offset;
  offset = g_time_zone_get_offset(datetime->tz, datetime->interval);
  offset *= USEC_PER_SECOND;
  return datetime->days * USEC_PER_DAY + datetime->usec - offset;
}
static GDateTime* g_date_time_from_instant(GTimeZone *tz, gint64 instant) {
  GDateTime *datetime;
  gint64 offset;
  if (instant < 0 || instant > G_GINT64_CONSTANT(1000000000000000000)) return NULL;
  datetime = g_date_time_alloc (tz);
  datetime->interval = g_time_zone_find_interval(tz,G_TIME_TYPE_UNIVERSAL,INSTANT_TO_UNIX (instant));
  offset = g_time_zone_get_offset(datetime->tz, datetime->interval);
  offset *= USEC_PER_SECOND;
  instant += offset;
  datetime->days = instant / USEC_PER_DAY;
  datetime->usec = instant % USEC_PER_DAY;
  if (datetime->days < 1 || 3652059 < datetime->days) {
      g_date_time_unref(datetime);
      datetime = NULL;
  }
  return datetime;
}
static gboolean g_date_time_deal_with_date_change(GDateTime *datetime) {
  GTimeType was_dst;
  gint64 full_time;
  gint64 usec;
  if (datetime->days < 1 || datetime->days > 3652059) return FALSE;
  was_dst = g_time_zone_is_dst (datetime->tz, datetime->interval);
  full_time = datetime->days * USEC_PER_DAY + datetime->usec;
  usec = full_time % USEC_PER_SECOND;
  full_time /= USEC_PER_SECOND;
  full_time -= UNIX_EPOCH_START * SEC_PER_DAY;
  datetime->interval = g_time_zone_adjust_time(datetime->tz, was_dst, &full_time);
  full_time += UNIX_EPOCH_START * SEC_PER_DAY;
  full_time *= USEC_PER_SECOND;
  full_time += usec;
  datetime->days = full_time / USEC_PER_DAY;
  datetime->usec = full_time % USEC_PER_DAY;
  return TRUE;
}
static GDateTime* g_date_time_replace_days(GDateTime *datetime, gint days) {
  GDateTime *new;
  new = g_date_time_alloc(datetime->tz);
  new->interval = datetime->interval;
  new->usec = datetime->usec;
  new->days = days;
  if (!g_date_time_deal_with_date_change(new)) {
      g_date_time_unref(new);
      new = NULL;
  }
  return new;
}
static GDateTime* g_date_time_new_from_timeval(GTimeZone *tz, const GTimeVal *tv) {
  return g_date_time_from_instant(tz, tv->tv_usec + UNIX_TO_INSTANT(tv->tv_sec));
}
static GDateTime* g_date_time_new_from_unix(GTimeZone *tz, gint64 secs) {
  return g_date_time_from_instant (tz, UNIX_TO_INSTANT (secs));
}
GDateTime* g_date_time_new_now(GTimeZone *tz) {
  GTimeVal tv;
  g_get_current_time(&tv);
  return g_date_time_new_from_timeval(tz, &tv);
}
GDateTime* g_date_time_new_now_local(void) {
  GDateTime *datetime;
  GTimeZone *local;
  local = g_time_zone_new_local();
  datetime = g_date_time_new_now(local);
  g_time_zone_unref(local);
  return datetime;
}
GDateTime* g_date_time_new_now_utc(void) {
  GDateTime *datetime;
  GTimeZone *utc;
  utc = g_time_zone_new_utc();
  datetime = g_date_time_new_now(utc);
  g_time_zone_unref(utc);
  return datetime;
}
GDateTime* g_date_time_new_from_unix_local(gint64 t) {
  GDateTime *datetime;
  GTimeZone *local;
  local = g_time_zone_new_local();
  datetime = g_date_time_new_from_unix(local, t);
  g_time_zone_unref(local);
  return datetime;
}
GDateTime* g_date_time_new_from_unix_utc(gint64 t) {
  GDateTime *datetime;
  GTimeZone *utc;
  utc = g_time_zone_new_utc();
  datetime = g_date_time_new_from_unix(utc, t);
  g_time_zone_unref(utc);
  return datetime;
}
GDateTime* g_date_time_new_from_timeval_local(const GTimeVal *tv) {
  GDateTime *datetime;
  GTimeZone *local;
  local = g_time_zone_new_local();
  datetime = g_date_time_new_from_timeval(local, tv);
  g_time_zone_unref(local);
  return datetime;
}
GDateTime* g_date_time_new_from_timeval_utc(const GTimeVal *tv) {
  GDateTime *datetime;
  GTimeZone *utc;
  utc = g_time_zone_new_utc();
  datetime = g_date_time_new_from_timeval(utc, tv);
  g_time_zone_unref(utc);
  return datetime;
}
GDateTime* g_date_time_new(GTimeZone *tz, gint year, gint month, gint day, gint hour, gint minute, gdouble seconds) {
  GDateTime *datetime;
  gint64 full_time;
  datetime = g_date_time_alloc(tz);
  datetime->days = ymd_to_days(year, month, day);
  datetime->usec = (hour * USEC_PER_HOUR) + (minute * USEC_PER_MINUTE) + (gint64) (seconds * USEC_PER_SECOND);
  full_time = SEC_PER_DAY * (ymd_to_days (year, month, day) - UNIX_EPOCH_START) + SECS_PER_HOUR * hour + SECS_PER_MINUTE * minute + (int)seconds;
  datetime->interval = g_time_zone_adjust_time(datetime->tz,G_TIME_TYPE_STANDARD, &full_time);
  full_time += UNIX_EPOCH_START * SEC_PER_DAY;
  datetime->days = full_time / SEC_PER_DAY;
  datetime->usec = (full_time % SEC_PER_DAY) * USEC_PER_SECOND;
  datetime->usec += ((int)(seconds * USEC_PER_SECOND)) % USEC_PER_SECOND;
  return datetime;
}
GDateTime* g_date_time_new_local(gint year, gint month, gint day, gint hour, gint minute, gdouble seconds) {
  GDateTime *datetime;
  GTimeZone *local;
  local = g_time_zone_new_local();
  datetime = g_date_time_new(local, year, month, day, hour, minute, seconds);
  g_time_zone_unref(local);
  return datetime;
}
GDateTime* g_date_time_new_utc(gint year, gint month, gint day, gint hour, gint minute, gdouble seconds) {
  GDateTime *datetime;
  GTimeZone *utc;
  utc = g_time_zone_new_utc();
  datetime = g_date_time_new(utc, year, month, day, hour, minute, seconds);
  g_time_zone_unref(utc);
  return datetime;
}
GDateTime* g_date_time_add(GDateTime *datetime, GTimeSpan  timespan) {
  return g_date_time_from_instant(datetime->tz, timespan + g_date_time_to_instant(datetime));
}
GDateTime* g_date_time_add_years(GDateTime *datetime, gint years) {
  gint year, month, day;
  g_return_val_if_fail(datetime != NULL, NULL);
  if (years < -10000 || years > 10000) return NULL;
  g_date_time_get_ymd(datetime, &year, &month, &day);
  year += years;
  if (month == 2 && day == 29 && !GREGORIAN_LEAP(year)) day = 28;
  return g_date_time_replace_days(datetime, ymd_to_days(year, month, day));
}
GDateTime* g_date_time_add_months(GDateTime *datetime, gint months) {
  gint year, month, day;
  g_return_val_if_fail(datetime != NULL, NULL);
  g_date_time_get_ymd(datetime, &year, &month, &day);
  if (months < -120000 || months > 120000) return NULL;
  year += months / 12;
  month += months % 12;
  if (month < 1) {
      month += 12;
      year--;
  } else if (month > 12) {
      month -= 12;
      year++;
  }
  day = MIN(day, days_in_months[GREGORIAN_LEAP(year)][month]);
  return g_date_time_replace_days(datetime, ymd_to_days(year, month, day));
}
GDateTime* g_date_time_add_weeks(GDateTime *datetime, gint weeks) {
  g_return_val_if_fail(datetime != NULL, NULL);
  return g_date_time_add_days(datetime, weeks * 7);
}
GDateTime* g_date_time_add_days(GDateTime *datetime, gint days) {
  g_return_val_if_fail(datetime != NULL, NULL);
  if (days < -3660000 || days > 3660000) return NULL;
  return g_date_time_replace_days(datetime, datetime->days + days);
}
GDateTime* g_date_time_add_hours(GDateTime *datetime, gint hours) {
  return g_date_time_add(datetime, hours * USEC_PER_HOUR);
}
GDateTime* g_date_time_add_minutes(GDateTime *datetime, gint minutes) {
  return g_date_time_add(datetime, minutes * USEC_PER_MINUTE);
}
GDateTime* g_date_time_add_seconds(GDateTime *datetime, gdouble seconds) {
  return g_date_time_add(datetime, seconds * USEC_PER_SECOND);
}
GDateTime* g_date_time_add_full(GDateTime *datetime, gint years, gint months, gint days, gint hours, gint minutes, gdouble seconds) {
  gint year, month, day;
  gint64 full_time;
  GDateTime *new;
  gint interval;
  g_return_val_if_fail(datetime != NULL, NULL);
  g_date_time_get_ymd(datetime, &year, &month, &day);
  months += years * 12;
  if (months < -120000 || months > 120000) return NULL;
  if (days < -3660000 || days > 3660000) return NULL;
  year += months / 12;
  month += months % 12;
  if (month < 1) {
      month += 12;
      year--;
  } else if (month > 12) {
      month -= 12;
      year++;
  }
  day = MIN (day, days_in_months[GREGORIAN_LEAP(year)][month]);
  full_time = datetime->usec / USEC_PER_SECOND + SEC_PER_DAY *
    (ymd_to_days (year, month, day) + days - UNIX_EPOCH_START);
  interval = g_time_zone_adjust_time(datetime->tz, g_time_zone_is_dst(datetime->tz, datetime->interval), &full_time);
  full_time -= g_time_zone_get_offset(datetime->tz, interval);
  full_time += UNIX_EPOCH_START * SEC_PER_DAY;
  full_time = full_time * USEC_PER_SECOND + datetime->usec % USEC_PER_SECOND;
  full_time += (hours * USEC_PER_HOUR) + (minutes * USEC_PER_MINUTE) + (gint64) (seconds * USEC_PER_SECOND);
  interval = g_time_zone_find_interval(datetime->tz,G_TIME_TYPE_UNIVERSAL,INSTANT_TO_UNIX(full_time));
  full_time += USEC_PER_SECOND * g_time_zone_get_offset(datetime->tz, interval);
  new = g_date_time_alloc(datetime->tz);
  new->interval = interval;
  new->days = full_time / USEC_PER_DAY;
  new->usec = full_time % USEC_PER_DAY;
  return new;
}
gint g_date_time_compare(gconstpointer dt1, gconstpointer dt2) {
  gint64 difference;
  difference = g_date_time_difference((GDateTime*)dt1, (GDateTime*)dt2);
  if (difference < 0) return -1;
  else if (difference > 0) return 1;
  else return 0;
}
GTimeSpan g_date_time_difference(GDateTime *end, GDateTime *begin) {
  g_return_val_if_fail(begin != NULL, 0);
  g_return_val_if_fail(end != NULL, 0);
  return g_date_time_to_instant(end) - g_date_time_to_instant(begin);
}
guint g_date_time_hash(gconstpointer datetime) {
  return g_date_time_to_instant((GDateTime *) datetime);
}
gboolean g_date_time_equal(gconstpointer dt1,  gconstpointer dt2) {
  return g_date_time_difference((GDateTime*)dt1, (GDateTime*)dt2) == 0;
}
void g_date_time_get_ymd(GDateTime *datetime, gint *year, gint *month, gint *day) {
  gint the_year;
  gint the_month;
  gint the_day;
  gint remaining_days;
  gint y100_cycles;
  gint y4_cycles;
  gint y1_cycles;
  gint preceding;
  gboolean leap;
  g_return_if_fail(datetime != NULL);
  remaining_days = datetime->days;
  remaining_days--;
  the_year = (remaining_days / DAYS_IN_400YEARS) * 400 + 1;
  remaining_days = remaining_days % DAYS_IN_400YEARS;
  y100_cycles = remaining_days / DAYS_IN_100YEARS;
  remaining_days = remaining_days % DAYS_IN_100YEARS;
  the_year += y100_cycles * 100;
  y4_cycles = remaining_days / DAYS_IN_4YEARS;
  remaining_days = remaining_days % DAYS_IN_4YEARS;
  the_year += y4_cycles * 4;
  y1_cycles = remaining_days / 365;
  the_year += y1_cycles;
  remaining_days = remaining_days % 365;
  if (y1_cycles == 4 || y100_cycles == 4) {
    g_assert(remaining_days == 0);
    the_year--;
    the_month = 12;
    the_day = 31;
    goto end;
  }
  leap = y1_cycles == 3 && (y4_cycles != 24 || y100_cycles == 3);
  g_assert(leap == GREGORIAN_LEAP(the_year));
  the_month = (remaining_days + 50) >> 5;
  preceding = (days_in_year[0][the_month - 1] + (the_month > 2 && leap));
  if (preceding > remaining_days) {
      the_month -= 1;
      preceding -= leap ? days_in_months[1][the_month] : days_in_months[0][the_month];
  }
  remaining_days -= preceding;
  g_assert(0 <= remaining_days);
  the_day = remaining_days + 1;
end:
  if (year) *year = the_year;
  if (month) *month = the_month;
  if (day) *day = the_day;
}
gint g_date_time_get_year(GDateTime *datetime) {
  gint year;
  g_return_val_if_fail(datetime != NULL, 0);
  g_date_time_get_ymd(datetime, &year, NULL, NULL);
  return year;
}
gint g_date_time_get_month(GDateTime *datetime) {
  gint month;
  g_return_val_if_fail(datetime != NULL, 0);
  g_date_time_get_ymd(datetime, NULL, &month, NULL);
  return month;
}
gint g_date_time_get_day_of_month(GDateTime *datetime) {
  gint day_of_year, i;
  const guint16 *days;
  guint16 last = 0;
  g_return_val_if_fail(datetime != NULL, 0);
  days = days_in_year[GREGORIAN_LEAP(g_date_time_get_year (datetime)) ? 1 : 0];
  g_date_time_get_week_number(datetime, NULL, NULL, &day_of_year);
  for (i = 1; i <= 12; i++) {
      if (days [i] >= day_of_year) return day_of_year - last;
      last = days[i];
  }
  g_warn_if_reached();
  return 0;
}
gint g_date_time_get_week_numbering_year (GDateTime *datetime) {
  gint year, month, day, weekday;
  g_date_time_get_ymd(datetime, &year, &month, &day);
  weekday = g_date_time_get_day_of_week(datetime);
  if (month == 1 && (day - weekday) <= -4) return year - 1;
  else if (month == 12 && (day - weekday) >= 28) return year + 1;
  else return year;
}
gint g_date_time_get_week_of_year (GDateTime *datetime) {
  gint weeknum;
  g_return_val_if_fail(datetime != NULL, 0);
  g_date_time_get_week_number(datetime, &weeknum, NULL, NULL);
  return weeknum;
}
gint g_date_time_get_day_of_week(GDateTime *datetime) {
  g_return_val_if_fail(datetime != NULL, 0);
  return (datetime->days - 1) % 7 + 1;
}
gint g_date_time_get_day_of_year(GDateTime *datetime) {
  gint doy = 0;
  g_return_val_if_fail(datetime != NULL, 0);
  g_date_time_get_week_number(datetime, NULL, NULL, &doy);
  return doy;
}
gint g_date_time_get_hour(GDateTime *datetime) {
  g_return_val_if_fail(datetime != NULL, 0);
  return (datetime->usec / USEC_PER_HOUR);
}
gint g_date_time_get_minute(GDateTime *datetime) {
  g_return_val_if_fail(datetime != NULL, 0);
  return (datetime->usec % USEC_PER_HOUR) / USEC_PER_MINUTE;
}
gint g_date_time_get_second(GDateTime *datetime) {
  g_return_val_if_fail(datetime != NULL, 0);
  return (datetime->usec % USEC_PER_MINUTE) / USEC_PER_SECOND;
}
gint g_date_time_get_microsecond(GDateTime *datetime) {
  g_return_val_if_fail(datetime != NULL, 0);
  return (datetime->usec % USEC_PER_SECOND);
}
gdouble g_date_time_get_seconds(GDateTime *datetime) {
  g_return_val_if_fail(datetime != NULL, 0);
  return (datetime->usec % USEC_PER_MINUTE) / 1000000.0;
}
gint64 g_date_time_to_unix(GDateTime *datetime) {
  return INSTANT_TO_UNIX(g_date_time_to_instant(datetime));
}
gboolean g_date_time_to_timeval(GDateTime *datetime, GTimeVal *tv) {
  tv->tv_sec = INSTANT_TO_UNIX(g_date_time_to_instant(datetime));
  tv->tv_usec = datetime->usec % USEC_PER_SECOND;
  return TRUE;
}
GTimeSpan g_date_time_get_utc_offset(GDateTime *datetime) {
  gint offset;
  g_return_val_if_fail(datetime != NULL, 0);
  offset = g_time_zone_get_offset(datetime->tz, datetime->interval);
  return (gint64)offset * USEC_PER_SECOND;
}
const gchar* g_date_time_get_timezone_abbreviation(GDateTime *datetime) {
  g_return_val_if_fail(datetime != NULL, NULL);
  return g_time_zone_get_abbreviation(datetime->tz, datetime->interval);
}
gboolean g_date_time_is_daylight_savings(GDateTime *datetime) {
  g_return_val_if_fail(datetime != NULL, FALSE);
  return g_time_zone_is_dst(datetime->tz, datetime->interval);
}
GDateTime* g_date_time_to_timezone(GDateTime *datetime, GTimeZone *tz) {
  return g_date_time_from_instant(tz, g_date_time_to_instant (datetime));
}
GDateTime* g_date_time_to_local(GDateTime *datetime) {
  GDateTime *new;
  GTimeZone *local;
  local = g_time_zone_new_local();
  new = g_date_time_to_timezone(datetime, local);
  g_time_zone_unref(local);
  return new;
}
GDateTime* g_date_time_to_utc(GDateTime *datetime) {
  GDateTime *new;
  GTimeZone *utc;
  utc = g_time_zone_new_utc();
  new = g_date_time_to_timezone(datetime, utc);
  g_time_zone_unref(utc);
  return new;
}
gchar* g_date_time_format(GDateTime *datetime, const gchar *format){
  GString *outstr;
  gchar *tmp;
  gunichar c;
  gboolean in_mod;
  g_return_val_if_fail(datetime != NULL, NULL);
  g_return_val_if_fail(format != NULL, NULL);
  g_return_val_if_fail(g_utf8_validate(format, -1, NULL), NULL);
  outstr = g_string_sized_new(strlen (format) * 2);
  in_mod = FALSE;
  for (; *format; format = g_utf8_next_char(format)) {
      c = g_utf8_get_char(format);
      switch(c) {
          case '%':
              if (!in_mod) {
                  in_mod = TRUE;
                  break;
              }
          default:
          if (in_mod) {
              switch(c) {
                  case 'a': g_string_append(outstr, WEEKDAY_ABBR(datetime)); break;
                  case 'A': g_string_append(outstr, WEEKDAY_FULL(datetime)); break;
                  case 'b': g_string_append(outstr, MONTH_ABBR(datetime)); break;
                  case 'B': g_string_append(outstr, MONTH_FULL(datetime)); break;
                  case 'd': g_string_append_printf(outstr, "%02d", g_date_time_get_day_of_month(datetime)); break;
                  case 'e': g_string_append_printf(outstr, "%2d", g_date_time_get_day_of_month(datetime)); break;
                  case 'F':
                      g_string_append_printf(outstr, "%d-%02d-%02d", g_date_time_get_year(datetime), g_date_time_get_month(datetime),
                                             g_date_time_get_day_of_month(datetime));
                      break;
                  case 'h': g_string_append(outstr, MONTH_ABBR(datetime)); break;
                  case 'H': g_string_append_printf(outstr, "%02d", g_date_time_get_hour(datetime)); break;
                  case 'I':
                      if ((g_date_time_get_hour(datetime) % 12) == 0) g_string_append(outstr, "12");
                      else g_string_append_printf(outstr, "%02d", g_date_time_get_hour(datetime) % 12);
                      break;
                  case 'j': g_string_append_printf(outstr, "%03d", g_date_time_get_day_of_year(datetime)); break;
                  case 'k': g_string_append_printf(outstr, "%2d", g_date_time_get_hour(datetime)); break;
                  case 'l':
                      if ((g_date_time_get_hour(datetime) % 12) == 0) g_string_append(outstr, "12");
                      else g_string_append_printf(outstr, "%2d", g_date_time_get_hour(datetime) % 12);
                      break;
                  case 'm': g_string_append_printf(outstr, "%02d", g_date_time_get_month(datetime)); break;
                  case 'M': g_string_append_printf(outstr, "%02d", g_date_time_get_minute(datetime)); break;
                  case 'N': g_string_append_printf(outstr, "%"G_GUINT64_FORMAT, datetime->usec % USEC_PER_SECOND); break;
                  case 'p': g_string_append(outstr, GET_AMPM(datetime, FALSE)); break;
                  case 'P': g_string_append(outstr, GET_AMPM(datetime, TRUE)); break;
                  case 'r': {
                          gint hour = g_date_time_get_hour(datetime) % 12;
                          if (hour == 0) hour = 12;
                          g_string_append_printf(outstr, "%02d:%02d:%02d %s", hour, g_date_time_get_minute(datetime), g_date_time_get_second(datetime),
                                                 GET_AMPM(datetime, FALSE));
                      }
                      break;
                  case 'R': g_string_append_printf(outstr, "%02d:%02d", g_date_time_get_hour(datetime), g_date_time_get_minute(datetime)); break;
                  case 's': g_string_append_printf(outstr, "%" G_GINT64_FORMAT, g_date_time_to_unix(datetime)); break;
                  case 'S': g_string_append_printf(outstr, "%02d", g_date_time_get_second(datetime)); break;
                  case 't': g_string_append_c(outstr, '\t'); break;
                  case 'u': g_string_append_printf(outstr, "%d", g_date_time_get_day_of_week(datetime)); break;
                  case 'W': g_string_append_printf(outstr, "%d", g_date_time_get_day_of_year(datetime) / 7); break;
                  case 'x': {
                          tmp = GET_PREFERRED_DATE(datetime);
                          g_string_append(outstr, tmp);
                          g_free (tmp);
                      }
                      break;
                  case 'X': {
                          tmp = GET_PREFERRED_TIME(datetime);
                          g_string_append(outstr, tmp);
                          g_free(tmp);
                      }
                      break;
                  case 'y': g_string_append_printf(outstr, "%02d", g_date_time_get_year(datetime) % 100); break;
                  case 'Y': g_string_append_printf(outstr, "%d", g_date_time_get_year(datetime)); break;
                  case 'z':
                  if (datetime->tz != NULL){
                      gint64 offset = g_date_time_get_utc_offset(datetime) / USEC_PER_SECOND;
                      g_string_append_printf(outstr, "%c%02d%02d", offset >= 0 ? '+' : '-', (int)offset / 3600, (int)offset / 60 % 60);
                  } else g_string_append(outstr, "+0000");
                  break;
                  case 'Z': g_string_append(outstr, g_date_time_get_timezone_abbreviation(datetime)); break;
                  case '%': g_string_append_c(outstr, '%'); break;
                  case 'n': g_string_append_c(outstr, '\n'); break;
                  default: goto bad_format;
              }
              in_mod = FALSE;
          } else g_string_append_unichar(outstr, c);
      }
  }
  return g_string_free(outstr, FALSE);
bad_format:
  g_string_free(outstr, TRUE);
  return NULL;
}