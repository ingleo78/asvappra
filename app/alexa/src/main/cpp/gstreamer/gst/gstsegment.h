#ifndef __GST_SEGMENT_H__
#define __GST_SEGMENT_H__

#include "gstformat.h"

G_BEGIN_DECLS
#define GST_TYPE_SEGMENT             (gst_segment_get_type())
typedef struct _GstSegment GstSegment;
typedef enum {
  GST_SEEK_TYPE_NONE            = 0,
  GST_SEEK_TYPE_SET             = 1,
  GST_SEEK_TYPE_END             = 2
} GstSeekType;
typedef enum {
  GST_SEEK_FLAG_NONE            = 0,
  GST_SEEK_FLAG_FLUSH           = (1 << 0),
  GST_SEEK_FLAG_ACCURATE        = (1 << 1),
  GST_SEEK_FLAG_KEY_UNIT        = (1 << 2),
  GST_SEEK_FLAG_SEGMENT         = (1 << 3),
  GST_SEEK_FLAG_TRICKMODE       = (1 << 4),
  GST_SEEK_FLAG_SKIP            = (1 << 4),
  GST_SEEK_FLAG_SNAP_BEFORE     = (1 << 5),
  GST_SEEK_FLAG_SNAP_AFTER      = (1 << 6),
  GST_SEEK_FLAG_SNAP_NEAREST    = GST_SEEK_FLAG_SNAP_BEFORE | GST_SEEK_FLAG_SNAP_AFTER,
  GST_SEEK_FLAG_TRICKMODE_KEY_UNITS = (1 << 7),
  GST_SEEK_FLAG_TRICKMODE_NO_AUDIO  = (1 << 8),
} GstSeekFlags;
typedef enum {
  GST_SEGMENT_FLAG_NONE            = GST_SEEK_FLAG_NONE,
  GST_SEGMENT_FLAG_RESET           = GST_SEEK_FLAG_FLUSH,
  GST_SEGMENT_FLAG_TRICKMODE       = GST_SEEK_FLAG_TRICKMODE,
  GST_SEGMENT_FLAG_SKIP            = GST_SEEK_FLAG_TRICKMODE,
  GST_SEGMENT_FLAG_SEGMENT         = GST_SEEK_FLAG_SEGMENT,
  GST_SEGMENT_FLAG_TRICKMODE_KEY_UNITS = GST_SEEK_FLAG_TRICKMODE_KEY_UNITS,
  GST_SEGMENT_FLAG_TRICKMODE_NO_AUDIO      = GST_SEEK_FLAG_TRICKMODE_NO_AUDIO
} GstSegmentFlags;
struct _GstSegment {
  GstSegmentFlags flags;
  gdouble         rate;
  gdouble         applied_rate;
  GstFormat       format;
  guint64         base;
  guint64         offset;
  guint64         start;
  guint64         stop;
  guint64         time;
  guint64         position;
  guint64         duration;
  gpointer        _gst_reserved[GST_PADDING];
};
GType        gst_segment_get_type            (void);
GstSegment * gst_segment_new                 (void) G_GNUC_MALLOC;
GstSegment * gst_segment_copy                (const GstSegment *segment) G_GNUC_MALLOC;
void         gst_segment_copy_into           (const GstSegment *src, GstSegment *dest);
void         gst_segment_free                (GstSegment *segment);

void         gst_segment_init                (GstSegment *segment, GstFormat format);

gint         gst_segment_to_stream_time_full (const GstSegment *segment, GstFormat format, guint64 position, guint64 * stream_time);
guint64      gst_segment_to_stream_time      (const GstSegment *segment, GstFormat format, guint64 position);
gint         gst_segment_position_from_stream_time_full (const GstSegment * segment, GstFormat format, guint64 stream_time, guint64 * position);
guint64      gst_segment_position_from_stream_time (const GstSegment * segment, GstFormat format, guint64 stream_time);
guint64      gst_segment_to_running_time     (const GstSegment *segment, GstFormat format, guint64 position);

gint         gst_segment_to_running_time_full (const GstSegment *segment, GstFormat format, guint64 position,
                                               guint64 * running_time);
#ifndef GST_DISABLE_DEPRECATED
guint64      gst_segment_to_position         (const GstSegment *segment, GstFormat format, guint64 running_time);
#endif
gint         gst_segment_position_from_running_time_full (const GstSegment *segment, GstFormat format, guint64 running_time, guint64 * position);
guint64      gst_segment_position_from_running_time (const GstSegment *segment, GstFormat format, guint64 running_time);

gboolean     gst_segment_set_running_time    (GstSegment *segment, GstFormat format, guint64 running_time);

gboolean     gst_segment_offset_running_time (GstSegment *segment, GstFormat format, gint64 offset);

gboolean     gst_segment_clip                (const GstSegment *segment, GstFormat format, guint64 start,
                                              guint64 stop, guint64 *clip_start, guint64 *clip_stop);


gboolean     gst_segment_do_seek             (GstSegment * segment, gdouble rate,
                                              GstFormat format, GstSeekFlags flags,
                                              GstSeekType start_type, guint64 start,
                                              GstSeekType stop_type, guint64 stop, gboolean * update);
gboolean     gst_segment_is_equal            (const GstSegment * s0, const GstSegment * s1);

#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstSegment, gst_segment_free)
#endif

G_END_DECLS

#endif /* __GST_SEGMENT_H__ */
