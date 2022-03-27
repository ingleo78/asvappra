#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_TIME_ZONE_H__
#define __G_TIME_ZONE_H__

#include "gmacros.h"
#include "gtypes.h"
#include "glib-basic-types.h"

G_BEGIN_DECLS
typedef struct _GTimeZone GTimeZone;
typedef enum {
  G_TIME_TYPE_STANDARD,
  G_TIME_TYPE_DAYLIGHT,
  G_TIME_TYPE_UNIVERSAL
} GTimeType;
GTimeZone* g_time_zone_new(const gchar *identifier);
GTimeZone* g_time_zone_new_utc(void);
GTimeZone* g_time_zone_new_local(void);
GTimeZone* g_time_zone_ref(GTimeZone *tz);
void g_time_zone_unref(GTimeZone *tz);
gint g_time_zone_find_interval(GTimeZone *tz, GTimeType type, gint64 time);
gint g_time_zone_adjust_time(GTimeZone *tz, GTimeType type, gint64 *time);
const gchar* g_time_zone_get_abbreviation(GTimeZone *tz, gint interval);
gint32 g_time_zone_get_offset(GTimeZone *tz, gint interval);
gboolean g_time_zone_is_dst(GTimeZone *tz, gint interval);
G_END_DECLS

#endif