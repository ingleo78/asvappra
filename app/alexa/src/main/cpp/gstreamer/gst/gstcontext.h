#ifndef __GST_CONTEXT_H__
#define __GST_CONTEXT_H__

#include <glib/glib.h>
#include "gstminiobject.h"
#include "gststructure.h"

G_BEGIN_DECLS
typedef struct _GstContext GstContext;
GST_EXPORT GType _gst_context_type;
#define GST_TYPE_CONTEXT                         (_gst_context_type)
#define GST_IS_CONTEXT(obj)                      (GST_IS_MINI_OBJECT_TYPE (obj, GST_TYPE_CONTEXT))
#define GST_CONTEXT_CAST(obj)                    ((GstContext*)(obj))
#define GST_CONTEXT(obj)                         (GST_CONTEXT_CAST(obj))
GType           gst_context_get_type            (void);
static inline GstContext *gst_context_ref (GstContext * context) {
  return (GstContext *) gst_mini_object_ref (GST_MINI_OBJECT_CAST (context));
}
static inline void gst_context_unref (GstContext * context) {
  gst_mini_object_unref (GST_MINI_OBJECT_CAST (context));
}
static inline GstContext *gst_context_copy (const GstContext * context) {
  return GST_CONTEXT_CAST (gst_mini_object_copy (GST_MINI_OBJECT_CONST_CAST (context)));
}
#define         gst_context_is_writable(context)     gst_mini_object_is_writable (GST_MINI_OBJECT_CAST (context))
#define         gst_context_make_writable(context)  GST_CONTEXT_CAST (gst_mini_object_make_writable (GST_MINI_OBJECT_CAST (context)))
static inline gboolean gst_context_replace (GstContext **old_context, GstContext *new_context) {
  return gst_mini_object_replace ((GstMiniObject **) old_context, (GstMiniObject *) new_context);
}
GstContext *          gst_context_new                      (const gchar * context_type, gboolean persistent) G_GNUC_MALLOC;
const gchar *         gst_context_get_context_type         (const GstContext * context);
gboolean              gst_context_has_context_type         (const GstContext * context, const gchar * context_type);
const GstStructure *  gst_context_get_structure            (const GstContext * context);
GstStructure *        gst_context_writable_structure       (GstContext * context);
gboolean              gst_context_is_persistent            (const GstContext * context);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstContext, gst_context_unref)
#endif
G_END_DECLS

#endif