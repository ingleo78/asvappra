#ifndef __GST_BIN_H__
#define __GST_BIN_H__

#include "gstelement.h"
#include "gstiterator.h"
#include "gstbus.h"

G_BEGIN_DECLS
#define GST_TYPE_BIN             (gst_bin_get_type ())
#define GST_IS_BIN(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_BIN))
#define GST_IS_BIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_BIN))
#define GST_BIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_BIN, GstBinClass))
#define GST_BIN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_BIN, GstBin))
#define GST_BIN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_BIN, GstBinClass))
#define GST_BIN_CAST(obj)        ((GstBin*)(obj))
typedef enum {
  GST_BIN_FLAG_NO_RESYNC	= (GST_ELEMENT_FLAG_LAST << 0),
  GST_BIN_FLAG_LAST		= (GST_ELEMENT_FLAG_LAST << 5)
} GstBinFlags;
#define GST_BIN_IS_NO_RESYNC(bin)        (GST_OBJECT_FLAG_IS_SET(bin,GST_BIN_FLAG_NO_RESYNC))
typedef struct _GstBin GstBin;
typedef struct _GstBinClass GstBinClass;
typedef struct _GstBinPrivate GstBinPrivate;
#define GST_BIN_NUMCHILDREN(bin)	(GST_BIN_CAST(bin)->numchildren)
#define GST_BIN_CHILDREN(bin)		(GST_BIN_CAST(bin)->children)
#define GST_BIN_CHILDREN_COOKIE(bin)	(GST_BIN_CAST(bin)->children_cookie)
struct _GstBin {
  GstElement	 element;
  gint		 numchildren;
  GList		*children;
  guint32	 children_cookie;
  GstBus        *child_bus;
  GList         *messages;
  gboolean	 polling;
  gboolean       state_dirty;
  gboolean       clock_dirty;
  GstClock	*provided_clock;
  GstElement    *clock_provider;
  GstBinPrivate *priv;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstBinClass {
  GstElementClass parent_class;
  GThreadPool  *pool;
  void		(*element_added)	(GstBin *bin, GstElement *child);
  void		(*element_removed)	(GstBin *bin, GstElement *child);
  gboolean	(*add_element)		(GstBin *bin, GstElement *element);
  gboolean	(*remove_element)	(GstBin *bin, GstElement *element);
  void		(*handle_message)	(GstBin *bin, GstMessage *message);
  gboolean	(*do_latency)           (GstBin *bin);
  gpointer _gst_reserved[GST_PADDING];
};
GType		gst_bin_get_type		(void);
GstElement*	gst_bin_new			(const gchar *name);
gboolean	gst_bin_add			(GstBin *bin, GstElement *element);
gboolean	gst_bin_remove			(GstBin *bin, GstElement *element);
GstElement*	gst_bin_get_by_name		 (GstBin *bin, const gchar *name);
GstElement*	gst_bin_get_by_name_recurse_up	 (GstBin *bin, const gchar *name);
GstElement*	gst_bin_get_by_interface	 (GstBin *bin, GType iface);
GstIterator*    gst_bin_iterate_elements	 (GstBin *bin);
GstIterator*    gst_bin_iterate_sorted		 (GstBin *bin);
GstIterator*    gst_bin_iterate_recurse		 (GstBin *bin);
GstIterator*	gst_bin_iterate_sinks		 (GstBin *bin);
GstIterator*	gst_bin_iterate_sources		 (GstBin *bin);
GstIterator*	gst_bin_iterate_all_by_interface (GstBin *bin, GType iface);
gboolean        gst_bin_recalculate_latency      (GstBin * bin);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstBin, gst_object_unref)
#endif
G_END_DECLS

#endif