#ifndef __GST_TOC_SETTER_H__
#define __GST_TOC_SETTER_H__

#include <glib/glib.h>

G_BEGIN_DECLS
#define GST_TYPE_TOC_SETTER              (gst_toc_setter_get_type ())
#define GST_TOC_SETTER(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_TOC_SETTER, GstTocSetter))
#define GST_IS_TOC_SETTER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_TOC_SETTER))
#define GST_TOC_SETTER_GET_IFACE(obj)    (G_TYPE_INSTANCE_GET_INTERFACE ((obj), GST_TYPE_TOC_SETTER, GstTocSetterInterface))
typedef struct _GstTocSetter GstTocSetter;
typedef struct _GstTocSetterInterface GstTocSetterInterface;
struct _GstTocSetterInterface {
  GTypeInterface g_iface;
};
GType         gst_toc_setter_get_type (void);
void          gst_toc_setter_reset   (GstTocSetter *setter);
GstToc *      gst_toc_setter_get_toc (GstTocSetter *setter);
void          gst_toc_setter_set_toc (GstTocSetter *setter, GstToc *toc);
G_END_DECLS

#endif