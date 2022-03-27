#ifndef __GST_GHOST_PAD_H__
#define __GST_GHOST_PAD_H__

#include "gstpad.h"

G_BEGIN_DECLS
#define GST_TYPE_PROXY_PAD              (gst_proxy_pad_get_type ())
#define GST_IS_PROXY_PAD(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_PROXY_PAD))
#define GST_IS_PROXY_PAD_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_PROXY_PAD))
#define GST_PROXY_PAD(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_PROXY_PAD, GstProxyPad))
#define GST_PROXY_PAD_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_PROXY_PAD, GstProxyPadClass))
typedef struct _GstProxyPad GstProxyPad;
typedef struct _GstProxyPadPrivate GstProxyPadPrivate;
typedef struct _GstProxyPadClass GstProxyPadClass;
struct _GstProxyPad {
  GstPad pad;
  GstProxyPadPrivate *priv;
};
struct _GstProxyPadClass {
  GstPadClass parent_class;
  gpointer _gst_reserved[1];
};
GType gst_proxy_pad_get_type (void);
GstProxyPad*     gst_proxy_pad_get_internal     (GstProxyPad *pad);
GstIterator*        gst_proxy_pad_iterate_internal_links_default (GstPad *pad, GstObject *parent) G_GNUC_MALLOC;
GstFlowReturn       gst_proxy_pad_chain_default                  (GstPad *pad, GstObject *parent, GstBuffer *buffer);
GstFlowReturn       gst_proxy_pad_chain_list_default             (GstPad *pad, GstObject *parent, GstBufferList *list);
GstFlowReturn       gst_proxy_pad_getrange_default(GstPad *pad, GstObject *parent, guint64 offset, guint size, GstBuffer **buffer);
#define GST_TYPE_GHOST_PAD              (gst_ghost_pad_get_type ())
#define GST_IS_GHOST_PAD(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_GHOST_PAD))
#define GST_IS_GHOST_PAD_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_GHOST_PAD))
#define GST_GHOST_PAD(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_GHOST_PAD, GstGhostPad))
#define GST_GHOST_PAD_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_GHOST_PAD, GstGhostPadClass))
#define GST_GHOST_PAD_CAST(obj)         ((GstGhostPad*)(obj))
typedef struct _GstGhostPad GstGhostPad;
typedef struct _GstGhostPadPrivate GstGhostPadPrivate;
typedef struct _GstGhostPadClass GstGhostPadClass;
struct _GstGhostPad {
  GstProxyPad pad;
  GstGhostPadPrivate *priv;
};
struct _GstGhostPadClass {
  GstProxyPadClass parent_class;
  gpointer _gst_reserved[GST_PADDING];
};
GType            gst_ghost_pad_get_type          (void);
GstPad*          gst_ghost_pad_new               (const gchar *name, GstPad *target) G_GNUC_MALLOC;
GstPad*          gst_ghost_pad_new_no_target     (const gchar *name, GstPadDirection dir) G_GNUC_MALLOC;
GstPad*          gst_ghost_pad_new_from_template (const gchar *name, GstPad * target, GstPadTemplate * templ) G_GNUC_MALLOC;
GstPad*          gst_ghost_pad_new_no_target_from_template (const gchar *name, GstPadTemplate * templ) G_GNUC_MALLOC;
GstPad*          gst_ghost_pad_get_target        (GstGhostPad *gpad);
gboolean         gst_ghost_pad_set_target        (GstGhostPad *gpad, GstPad *newtarget);
gboolean         gst_ghost_pad_construct         (GstGhostPad *gpad);
gboolean         gst_ghost_pad_activate_mode_default  (GstPad * pad, GstObject * parent, GstPadMode mode, gboolean active);
gboolean         gst_ghost_pad_internal_activate_mode_default   (GstPad * pad, GstObject * parent, GstPadMode mode, gboolean active);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstGhostPad, gst_object_unref)
#endif
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstProxyPad, gst_object_unref)
#endif
G_END_DECLS

#endif