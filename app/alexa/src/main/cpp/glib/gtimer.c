#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include "glib-basic-types.h"
#include "gtimer.h"
#include "glibconfig.h"
#include "gmem.h"
#include "gstrfuncs.h"
#include "gtestutils.h"
#include "gmain.h"

struct _GTimer {
  guint64 start;
  guint64 end;
  guint active : 1;
};
GTimer* g_timer_new(void) {
  GTimer *timer;
  timer = g_new(GTimer, 1);
  timer->active = TRUE;
  timer->start = g_get_monotonic_time();
  return timer;
}
void g_timer_destroy(GTimer *timer) {
  g_return_if_fail(timer != NULL);
  g_free(timer);
}
void g_timer_start(GTimer *timer) {
  g_return_if_fail(timer != NULL);
  timer->active = TRUE;
  timer->start = g_get_monotonic_time();
}
void g_timer_stop(GTimer *timer) {
  g_return_if_fail(timer != NULL);
  timer->active = FALSE;
  timer->end = g_get_monotonic_time();
}
void g_timer_reset(GTimer *timer) {
  g_return_if_fail(timer != NULL);
  timer->start = g_get_monotonic_time();
}
void g_timer_continue(GTimer *timer) {
  guint64 elapsed;
  g_return_if_fail(timer != NULL);
  g_return_if_fail(timer->active == FALSE);
  elapsed = timer->end - timer->start;
  timer->start = g_get_monotonic_time();
  timer->start -= elapsed;
  timer->active = TRUE;
}
gdouble g_timer_elapsed(GTimer *timer, gulong *microseconds) {
  gdouble total;
  gint64 elapsed;
  g_return_val_if_fail (timer != NULL, 0);
  if (timer->active) timer->end = g_get_monotonic_time ();
  elapsed = timer->end - timer->start;
  total = elapsed / 1e6;
  if (microseconds) *microseconds = elapsed % 1000000;
  return total;
}
void g_usleep(gulong microseconds) {
#ifndef G_OS_WIN32
  Sleep (microseconds / 1000);
#else
  struct timespec request, remaining;
  request.tv_sec = microseconds / G_USEC_PER_SEC;
  request.tv_nsec = 1000 * (microseconds % G_USEC_PER_SEC);
  while(nanosleep (&request, &remaining) == -1 && errno == EINTR) request = remaining;
#endif
}
void g_time_val_add(GTimeVal *time_, glong microseconds) {
  g_return_if_fail(time_->tv_usec >= 0 && time_->tv_usec < G_USEC_PER_SEC);
  if (microseconds >= 0) {
      time_->tv_usec += microseconds % G_USEC_PER_SEC;
      time_->tv_sec += microseconds / G_USEC_PER_SEC;
      if (time_->tv_usec >= G_USEC_PER_SEC) {
         time_->tv_usec -= G_USEC_PER_SEC;
         time_->tv_sec++;
      }
  } else {
      microseconds *= -1;
      time_->tv_usec -= microseconds % G_USEC_PER_SEC;
      time_->tv_sec -= microseconds / G_USEC_PER_SEC;
      if (time_->tv_usec < 0) {
         time_->tv_usec += G_USEC_PER_SEC;
         time_->tv_sec--;
      }
  }
}
static time_t mktime_utc(struct tm *tm) {
  time_t retval;
#ifndef HAVE_TIMEGM
  static const gint days_before[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
  };
#endif
#ifndef HAVE_TIMEGM
  if (tm->tm_mon < 0 || tm->tm_mon > 11) return (time_t) -1;
  retval = (tm->tm_year - 70) * 365;
  retval += (tm->tm_year - 68) / 4;
  retval += days_before[tm->tm_mon] + tm->tm_mday - 1;
  if (tm->tm_year % 4 == 0 && tm->tm_mon < 2) retval -= 1;
  retval = ((((retval * 24) + tm->tm_hour) * 60) + tm->tm_min) * 60 + tm->tm_sec;
#else
  retval = timegm (tm);
#endif
  return retval;
}
gboolean g_time_val_from_iso8601(const gchar *iso_date, GTimeVal *time_) {
  struct tm tm = {0};
  long val;
  g_return_val_if_fail(iso_date != NULL, FALSE);
  g_return_val_if_fail(time_ != NULL, FALSE);
  while(g_ascii_isspace(*iso_date)) iso_date++;
  if (*iso_date == '\0') return FALSE;
  if (!g_ascii_isdigit(*iso_date) && *iso_date != '-' && *iso_date != '+') return FALSE;
  val = strtoul (iso_date, (char **)&iso_date, 10);
  if (*iso_date == '-') {
      tm.tm_year = val - 1900;
      iso_date++;
      tm.tm_mon = strtoul(iso_date, (char**)&iso_date, 10) - 1;
      if (*iso_date++ != '-') return FALSE;
      tm.tm_mday = strtoul(iso_date, (char**)&iso_date, 10);
  } else {
      tm.tm_mday = val % 100;
      tm.tm_mon = (val % 10000) / 100 - 1;
      tm.tm_year = val / 10000 - 1900;
  }
  if (*iso_date != 'T') {
      if (*iso_date == '\0') return TRUE;
      return FALSE;
  }
  iso_date++;
  if (!g_ascii_isdigit(*iso_date)) return FALSE;
  val = strtoul (iso_date, (char**)&iso_date, 10);
  if (*iso_date == ':') {
      tm.tm_hour = val;
      iso_date++;
      tm.tm_min = strtoul(iso_date, (char**)&iso_date, 10);
      if (*iso_date++ != ':') return FALSE;
      tm.tm_sec = strtoul(iso_date, (char**)&iso_date, 10);
  } else {
      tm.tm_sec = val % 100;
      tm.tm_min = (val % 10000) / 100;
      tm.tm_hour = val / 10000;
  }
  time_->tv_usec = 0;
  if (*iso_date == ',' || *iso_date == '.') {
      glong mul = 100000;
      while(g_ascii_isdigit(*++iso_date)) {
          time_->tv_usec += (*iso_date - '0') * mul;
          mul /= 10;
      }
  }
  if (*iso_date == 'Z') {
      iso_date++;
      time_->tv_sec = mktime_utc (&tm);
  } else if (*iso_date == '+' || *iso_date == '-') {
      gint sign = (*iso_date == '+') ? -1 : 1;
      val = strtoul(iso_date + 1, (char**)&iso_date, 10);
      if (*iso_date == ':') val = 60 * val + strtoul(iso_date + 1, (char**)&iso_date, 10);
      else val = 60 * (val / 100) + (val % 100);
      time_->tv_sec = mktime_utc(&tm) + (time_t)(60 * val * sign);
  } else {
      tm.tm_isdst = -1;
      time_->tv_sec = mktime (&tm);
  }
  while(g_ascii_isspace(*iso_date)) iso_date++;
  return *iso_date == '\0';
}
gchar* g_time_val_to_iso8601(GTimeVal *time_) {
  gchar *retval;
  struct tm *tm;
#ifdef HAVE_GMTIME_R
  struct tm tm_;
#endif
  time_t secs;
  g_return_val_if_fail(time_->tv_usec >= 0 && time_->tv_usec < G_USEC_PER_SEC, NULL);
  secs = time_->tv_sec;
#ifdef _WIN32
  tm = gmtime(&secs);
#else
#ifdef HAVE_GMTIME_R
  tm = gmtime_r(&secs, &tm_);
#else
  tm = gmtime(&secs);
#endif
#endif
  if (time_->tv_usec != 0) {
      retval = g_strdup_printf("%4d-%02d-%02dT%02d:%02d:%02d.%06ldZ", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
                                time_->tv_usec);
  } else {
      retval = g_strdup_printf("%4d-%02d-%02dT%02d:%02d:%02dZ", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
  }
  return retval;
}