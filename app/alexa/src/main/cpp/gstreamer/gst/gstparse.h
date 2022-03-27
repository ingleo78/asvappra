#ifndef __GST_PARSE_H__
#define __GST_PARSE_H__

#include "gstelement.h"

G_BEGIN_DECLS
GQuark gst_parse_error_quark (void);
#define GST_PARSE_ERROR gst_parse_error_quark ()
typedef enum {
  GST_PARSE_ERROR_SYNTAX,
  GST_PARSE_ERROR_NO_SUCH_ELEMENT,
  GST_PARSE_ERROR_NO_SUCH_PROPERTY,
  GST_PARSE_ERROR_LINK,
  GST_PARSE_ERROR_COULD_NOT_SET_PROPERTY,
  GST_PARSE_ERROR_EMPTY_BIN,
  GST_PARSE_ERROR_EMPTY,
  GST_PARSE_ERROR_DELAYED_LINK
} GstParseError;
typedef enum {
  GST_PARSE_FLAG_NONE = 0,
  GST_PARSE_FLAG_FATAL_ERRORS = (1 << 0),
  GST_PARSE_FLAG_NO_SINGLE_ELEMENT_BINS = (1 << 1)
} GstParseFlags;
#define GST_TYPE_PARSE_CONTEXT (gst_parse_context_get_type())
typedef struct _GstParseContext GstParseContext;
GType             gst_parse_context_get_type (void);
GstParseContext * gst_parse_context_new (void) G_GNUC_MALLOC;
gchar          ** gst_parse_context_get_missing_elements (GstParseContext * context) G_GNUC_MALLOC;
void              gst_parse_context_free (GstParseContext * context);
GstElement      * gst_parse_launch       (const gchar *pipeline_description, GError **error) G_GNUC_MALLOC;
GstElement      * gst_parse_launchv      (const gchar **argv, GError **error) G_GNUC_MALLOC;
GstElement *gst_parse_launch_full(const gchar *pipeline_description, GstParseContext *context, GstParseFlags flags,
                                  GError **error) G_GNUC_MALLOC;
GstElement      * gst_parse_launchv_full(const gchar **argv, GstParseContext *context, GstParseFlags flags, GError **error) G_GNUC_MALLOC;
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstParseContext, gst_parse_context_free)
#endif
G_END_DECLS

#endif