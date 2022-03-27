#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_TIMER_H__
#define __G_TIMER_H__

#include "gmacros.h"
#include "gtypes.h"
#include "glib-basic-types.h"

G_BEGIN_DECLS
typedef struct _GTimer GTimer;
typedef struct _GTimeVal GTimeVal;
#define G_USEC_PER_SEC 1000000
GTimer* g_timer_new(void);
void g_timer_destroy(GTimer *timer);
void g_timer_start(GTimer *timer);
void g_timer_stop(GTimer *timer);
void g_timer_reset(GTimer *timer);
void g_timer_continue(GTimer *timer);
gdouble g_timer_elapsed(GTimer *timer, gulong *microseconds);
void g_usleep(gulong microseconds);
void g_time_val_add(GTimeVal *time_, glong microseconds);
gboolean g_time_val_from_iso8601(const gchar *iso_date, GTimeVal *time_);
gchar* g_time_val_to_iso8601(GTimeVal *time_) G_GNUC_MALLOC;
G_END_DECLS

#endif