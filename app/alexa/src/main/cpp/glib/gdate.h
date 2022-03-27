#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_DATE_H__
#define __G_DATE_H__

#include <time.h>
#include <sys/types.h>
#include "gmacros.h"
#include "gquark.h"
#include "gtypes.h"
#include "glib-basic-types.h"
#include "gmain.h"

G_BEGIN_DECLS
typedef gint32  GTime;
typedef guint16 GDateYear;
typedef guint8  GDateDay;
typedef struct _GDate GDate;
typedef enum {
  G_DATE_DAY   = 0,
  G_DATE_MONTH = 1,
  G_DATE_YEAR  = 2
} GDateDMY;
typedef enum {
  G_DATE_BAD_WEEKDAY  = 0,
  G_DATE_MONDAY       = 1,
  G_DATE_TUESDAY      = 2,
  G_DATE_WEDNESDAY    = 3,
  G_DATE_THURSDAY     = 4,
  G_DATE_FRIDAY       = 5,
  G_DATE_SATURDAY     = 6,
  G_DATE_SUNDAY       = 7
} GDateWeekday;
typedef enum {
  G_DATE_BAD_MONTH = 0,
  G_DATE_JANUARY   = 1,
  G_DATE_FEBRUARY  = 2,
  G_DATE_MARCH     = 3,
  G_DATE_APRIL     = 4,
  G_DATE_MAY       = 5,
  G_DATE_JUNE      = 6,
  G_DATE_JULY      = 7,
  G_DATE_AUGUST    = 8,
  G_DATE_SEPTEMBER = 9,
  G_DATE_OCTOBER   = 10,
  G_DATE_NOVEMBER  = 11,
  G_DATE_DECEMBER  = 12
} GDateMonth;
#define G_DATE_BAD_JULIAN 0U
#define G_DATE_BAD_DAY    0U
#define G_DATE_BAD_YEAR   0U
struct _GDate {
  guint julian_days : 32;
  guint julian : 1;
  guint dmy    : 1;
  guint day    : 6;
  guint month  : 4;
  guint year   : 16;
};
GDate* g_date_new(void);
GDate* g_date_new_dmy(GDateDay day, GDateMonth month, GDateYear year);
GDate* g_date_new_julian(guint32 julian_day);
void g_date_free(GDate *date);
int g_date_valid(const GDate *date);
int g_date_valid_day(GDateDay day) G_GNUC_CONST;
int g_date_valid_month(GDateMonth month) G_GNUC_CONST;
int g_date_valid_year(GDateYear  year) G_GNUC_CONST;
int g_date_valid_weekday(GDateWeekday weekday) G_GNUC_CONST;
int g_date_valid_julian(guint32 julian_date) G_GNUC_CONST;
int g_date_valid_dmy(GDateDay day, GDateMonth month, GDateYear year) G_GNUC_CONST;
GDateWeekday g_date_get_weekday(const GDate *date);
GDateMonth g_date_get_month(const GDate *date);
GDateYear g_date_get_year(const GDate *date);
GDateDay g_date_get_day(const GDate *date);
guint32 g_date_get_julian(const GDate *date);
guint g_date_get_day_of_year(const GDate *date);
guint g_date_get_monday_week_of_year(const GDate *date);
guint g_date_get_sunday_week_of_year(const GDate *date);
guint g_date_get_iso8601_week_of_year(const GDate *date);
void g_date_clear(GDate *date, guint n_dates);
void g_date_set_parse(GDate *date, const gchar *str);
void g_date_set_time_t(GDate *date, time_t timet);
void g_date_set_time_val(GDate *date, GTimeVal *timeval);
#ifndef G_DISABLE_DEPRECATED
void g_date_set_time(GDate *date, GTime time_);
#endif
void g_date_set_month(GDate *date, GDateMonth month);
void g_date_set_day(GDate *date, GDateDay day);
void g_date_set_year(GDate *date, GDateYear year);
void g_date_set_dmy(GDate *date, GDateDay day, GDateMonth month, GDateYear y);
void g_date_set_julian(GDate *date, guint32 julian_date);
int g_date_is_first_of_month(const GDate *date);
int g_date_is_last_of_month(const GDate *date);
void g_date_add_days(GDate *date, guint n_days);
void g_date_subtract_days(GDate *date, guint n_days);
void g_date_add_months(GDate *date, guint n_months);
void g_date_subtract_months(GDate *date, guint n_months);
void g_date_add_years(GDate *date, guint n_years);
void g_date_subtract_years(GDate *date, guint n_years);
int g_date_is_leap_year(GDateYear year) G_GNUC_CONST;
guint8 g_date_get_days_in_month(GDateMonth month, GDateYear year) G_GNUC_CONST;
guint8 g_date_get_monday_weeks_in_year(GDateYear year) G_GNUC_CONST;
guint8 g_date_get_sunday_weeks_in_year(GDateYear year) G_GNUC_CONST;
gint g_date_days_between(const GDate *date1, const GDate *date2);
gint g_date_compare(const GDate *lhs, const GDate *rhs);
void g_date_to_struct_tm(const GDate *date, struct tm *tm);
void g_date_clamp(GDate *date, const GDate *min_date, const GDate *max_date);
void g_date_order(GDate *date1, GDate *date2);
gsize g_date_strftime(gchar *s, gsize slen, const gchar *format, const GDate *date);
#ifndef G_DISABLE_DEPRECATED
#define g_date_weekday 			g_date_get_weekday
#define g_date_month 			g_date_get_month
#define g_date_year 			g_date_get_year
#define g_date_day 			g_date_get_day
#define g_date_julian 			g_date_get_julian
#define g_date_day_of_year 		g_date_get_day_of_year
#define g_date_monday_week_of_year 	g_date_get_monday_week_of_year
#define g_date_sunday_week_of_year 	g_date_get_sunday_week_of_year
#define g_date_days_in_month 		g_date_get_days_in_month
#define g_date_monday_weeks_in_year 	g_date_get_monday_weeks_in_year
#define g_date_sunday_weeks_in_year	g_date_get_sunday_weeks_in_year
#endif
G_END_DECLS
#endif