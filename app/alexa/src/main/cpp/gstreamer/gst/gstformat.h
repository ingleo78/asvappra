#ifndef __GST_FORMAT_H__
#define __GST_FORMAT_H__

#include <glib/glib.h>
#include "gstiterator.h"

G_BEGIN_DECLS
typedef enum {
  GST_FORMAT_UNDEFINED  =  0,
  GST_FORMAT_DEFAULT    =  1,
  GST_FORMAT_BYTES      =  2,
  GST_FORMAT_TIME       =  3,
  GST_FORMAT_BUFFERS    =  4,
  GST_FORMAT_PERCENT    =  5
} GstFormat;
#define GST_FORMAT_PERCENT_MAX          G_GINT64_CONSTANT (1000000)
#define GST_FORMAT_PERCENT_SCALE        G_GINT64_CONSTANT (10000)
typedef struct _GstFormatDefinition GstFormatDefinition;
struct _GstFormatDefinition {
  GstFormat    value;
  const gchar *nick;
  const gchar *description;
  GQuark       quark;
};
const gchar*    gst_format_get_name             (GstFormat format);
GQuark          gst_format_to_quark             (GstFormat format);
GstFormat       gst_format_register             (const gchar *nick, const gchar *description);
GstFormat       gst_format_get_by_nick          (const gchar *nick);
gboolean        gst_formats_contains            (const GstFormat *formats, GstFormat format);
const GstFormatDefinition *gst_format_get_details          (GstFormat format);
GstIterator*    gst_format_iterate_definitions  (void);
G_END_DECLS

#endif