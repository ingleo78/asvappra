#ifndef __GSTDEBUGUTILS_H__
#define __GSTDEBUGUTILS_H__

#include <glib/glib.h>
#include <glib/glib-object.h>
#include "gstconfig.h"
#include "gstbin.h"

G_BEGIN_DECLS
typedef enum {
  GST_DEBUG_GRAPH_SHOW_MEDIA_TYPE         = (1<<0),
  GST_DEBUG_GRAPH_SHOW_CAPS_DETAILS       = (1<<1),
  GST_DEBUG_GRAPH_SHOW_NON_DEFAULT_PARAMS = (1<<2),
  GST_DEBUG_GRAPH_SHOW_STATES             = (1<<3),
  GST_DEBUG_GRAPH_SHOW_FULL_PARAMS        = (1<<4),
  GST_DEBUG_GRAPH_SHOW_ALL                = ((1<<4)-1),
  GST_DEBUG_GRAPH_SHOW_VERBOSE            = (-1)
} GstDebugGraphDetails;
gchar * gst_debug_bin_to_dot_data (GstBin *bin, GstDebugGraphDetails details);
void gst_debug_bin_to_dot_file (GstBin *bin, GstDebugGraphDetails details, const gchar *file_name);
void gst_debug_bin_to_dot_file_with_ts (GstBin *bin, GstDebugGraphDetails details, const gchar *file_name);
#ifndef GST_DISABLE_GST_DEBUG
#define GST_DEBUG_BIN_TO_DOT_FILE(bin, details, file_name) gst_debug_bin_to_dot_file (bin, details, file_name)
#define GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(bin, details, file_name) gst_debug_bin_to_dot_file_with_ts (bin, details, file_name)
#else
#define GST_DEBUG_BIN_TO_DOT_FILE(bin, details, file_name)
#define GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(bin, details, file_name)
#endif
G_END_DECLS

#endif