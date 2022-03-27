#ifndef __GST_TYPE_FIND_H__
#define __GST_TYPE_FIND_H__

#include "gstcaps.h"
#include "gstplugin.h"
#include "gstpluginfeature.h"

G_BEGIN_DECLS
#define GST_TYPE_TYPE_FIND  (gst_type_find_get_type())
typedef struct _GstTypeFind GstTypeFind;
typedef void (* GstTypeFindFunction) (GstTypeFind *find, gpointer user_data);
typedef enum {
  GST_TYPE_FIND_NONE = 0,
  GST_TYPE_FIND_MINIMUM = 1,
  GST_TYPE_FIND_POSSIBLE = 50,
  GST_TYPE_FIND_LIKELY = 80,
  GST_TYPE_FIND_NEARLY_CERTAIN = 99,
  GST_TYPE_FIND_MAXIMUM = 100
} GstTypeFindProbability;
struct _GstTypeFind {
  const guint8 *  (* peek)       (gpointer         data, gint64           offset, guint            size);
  void            (* suggest)    (gpointer         data, guint            probability, GstCaps         *caps);
  gpointer         data;
  guint64         (* get_length) (gpointer data);
  gpointer _gst_reserved[GST_PADDING];
};
GType     gst_type_find_get_type   (void);
const guint8 *  gst_type_find_peek       (GstTypeFind   * find, gint64          offset, guint           size);
void            gst_type_find_suggest    (GstTypeFind   * find, guint           probability, GstCaps       * caps);
void gst_type_find_suggest_simple(GstTypeFind *find, guint probability, const char  *media_type, const char  *fieldname, ...);
guint64   gst_type_find_get_length (GstTypeFind   * find);
gboolean  gst_type_find_register   (GstPlugin *plugin, const gchar *name, guint rank, GstTypeFindFunction func, const gchar *extensions,
                                    GstCaps *possible_caps, gpointer data, GDestroyNotify data_notify);
G_END_DECLS

#endif