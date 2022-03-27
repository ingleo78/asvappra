#ifndef __GST_SYSTEM_CLOCK_H__
#define __GST_SYSTEM_CLOCK_H__

#include "gstclock.h"

G_BEGIN_DECLS
#define GST_TYPE_SYSTEM_CLOCK                   (gst_system_clock_get_type ())
#define GST_SYSTEM_CLOCK(obj)                   (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_SYSTEM_CLOCK, GstSystemClock))
#define GST_SYSTEM_CLOCK_CAST(obj)              ((GstSystemClock *)(obj))
#define GST_IS_SYSTEM_CLOCK(obj)                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_SYSTEM_CLOCK))
#define GST_SYSTEM_CLOCK_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_SYSTEM_CLOCK, GstSystemClockClass))
#define GST_IS_SYSTEM_CLOCK_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_SYSTEM_CLOCK))
#define GST_SYSTEM_CLOCK_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_SYSTEM_CLOCK, GstSystemClockClass))
typedef struct _GstSystemClock GstSystemClock;
typedef struct _GstSystemClockClass GstSystemClockClass;
typedef struct _GstSystemClockPrivate GstSystemClockPrivate;
typedef enum {
  GST_CLOCK_TYPE_REALTIME       = 0,
  GST_CLOCK_TYPE_MONOTONIC      = 1,
  GST_CLOCK_TYPE_OTHER          = 2
} GstClockType;
struct _GstSystemClock {
  GstClock       clock;
  GstSystemClockPrivate *priv;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstSystemClockClass {
  GstClockClass  parent_class;
  gpointer _gst_reserved[GST_PADDING];
};
GType                   gst_system_clock_get_type       (void);
GstClock*               gst_system_clock_obtain         (void);
void                    gst_system_clock_set_default    (GstClock *new_clock);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstSystemClock, gst_object_unref)
#endif
G_END_DECLS

#endif