#ifndef __GST_PAD_TEMPLATE_H__
#define __GST_PAD_TEMPLATE_H__

#include "gstconfig.h"
#include "gstobject.h"
#include "gstbuffer.h"
#include "gstcaps.h"
#include "gstevent.h"
#include "gstquery.h"
#include "gsttask.h"
#include "gstpad.h"

typedef struct _GstPadTemplate GstPadTemplate;
typedef struct _GstPadTemplateClass GstPadTemplateClass;
typedef struct _GstStaticPadTemplate GstStaticPadTemplate;
G_BEGIN_DECLS
#define GST_TYPE_STATIC_PAD_TEMPLATE	(gst_static_pad_template_get_type ())
#define GST_TYPE_PAD_TEMPLATE		(gst_pad_template_get_type ())
#define GST_PAD_TEMPLATE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_PAD_TEMPLATE,GstPadTemplate))
#define GST_PAD_TEMPLATE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_PAD_TEMPLATE,GstPadTemplateClass))
#define GST_IS_PAD_TEMPLATE(obj)	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_PAD_TEMPLATE))
#define GST_IS_PAD_TEMPLATE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_PAD_TEMPLATE))
typedef enum {
  GST_PAD_ALWAYS,
  GST_PAD_SOMETIMES,
  GST_PAD_REQUEST
} GstPadPresence;
typedef enum {
    GST_PAD_UNKNOWN,
    GST_PAD_SRC,
    GST_PAD_SINK
} GstPadDirection;
typedef enum {
    GST_PAD_LINK_CHECK_NOTHING       = 0,
    GST_PAD_LINK_CHECK_HIERARCHY     = 1 << 0,
    GST_PAD_LINK_CHECK_TEMPLATE_CAPS = 1 << 1,
    GST_PAD_LINK_CHECK_CAPS          = 1 << 2,
    GST_PAD_LINK_CHECK_DEFAULT       = GST_PAD_LINK_CHECK_HIERARCHY | GST_PAD_LINK_CHECK_CAPS
} GstPadLinkCheck;
typedef enum {
    GST_FLOW_CUSTOM_SUCCESS_2 = 102,
    GST_FLOW_CUSTOM_SUCCESS_1 = 101,
    GST_FLOW_CUSTOM_SUCCESS = 100,
    GST_FLOW_OK		  =  0,
    GST_FLOW_NOT_LINKED     = -1,
    GST_FLOW_FLUSHING       = -2,
    GST_FLOW_EOS            = -3,
    GST_FLOW_NOT_NEGOTIATED = -4,
    GST_FLOW_ERROR	  = -5,
    GST_FLOW_NOT_SUPPORTED  = -6,
    GST_FLOW_CUSTOM_ERROR   = -100,
    GST_FLOW_CUSTOM_ERROR_1 = -101,
    GST_FLOW_CUSTOM_ERROR_2 = -102
} GstFlowReturn;
typedef enum {
    GST_PAD_LINK_OK               =  0,
    GST_PAD_LINK_WRONG_HIERARCHY  = -1,
    GST_PAD_LINK_WAS_LINKED       = -2,
    GST_PAD_LINK_WRONG_DIRECTION  = -3,
    GST_PAD_LINK_NOFORMAT         = -4,
    GST_PAD_LINK_NOSCHED          = -5,
    GST_PAD_LINK_REFUSED          = -6
} GstPadLinkReturn;
typedef struct _GstPad GstPad;
typedef struct _GstPadPrivate GstPadPrivate;
typedef gboolean		(*GstPadActivateFunction)	(GstPad *pad, GstObject *parent);
typedef gboolean		(*GstPadActivateModeFunction)	(GstPad *pad, GstObject *parent, GstPadMode mode, gboolean active);
typedef GstFlowReturn		(*GstPadChainFunction)		(GstPad *pad, GstObject *parent, GstBuffer *buffer);
typedef GstFlowReturn		(*GstPadChainListFunction)	(GstPad *pad, GstObject *parent, GstBufferList *list);
typedef GstFlowReturn		(*GstPadGetRangeFunction)	(GstPad *pad, GstObject *parent, guint64 offset, guint length, GstBuffer **buffer);
typedef gboolean		(*GstPadEventFunction)		(GstPad *pad, GstObject *parent, GstEvent *event);
typedef GstFlowReturn		(*GstPadEventFullFunction)	(GstPad *pad, GstObject *parent, GstEvent *event);
typedef GstIterator*           (*GstPadIterIntLinkFunction)    (GstPad *pad, GstObject *parent);
typedef gboolean		(*GstPadQueryFunction)		(GstPad *pad, GstObject *parent, GstQuery *query);
typedef GstPadLinkReturn	(*GstPadLinkFunction)		(GstPad *pad, GstObject *parent, GstPad *peer);
typedef void			(*GstPadUnlinkFunction)		(GstPad *pad, GstObject *parent);
typedef gboolean		(*GstPadForwardFunction)	(GstPad *pad, gpointer user_data);
struct _GstPad {
    GstObject                      object;
    gpointer                       element_private;
    GstPadTemplate                *padtemplate;
    GstPadDirection                direction;
    GRecMutex		         stream_rec_lock;
    GstTask			*task;
    GCond				 block_cond;
    GHookList                      probes;
    GstPadMode		         mode;
    GstPadActivateFunction	 activatefunc;
    gpointer                       activatedata;
    GDestroyNotify                 activatenotify;
    GstPadActivateModeFunction	 activatemodefunc;
    gpointer                       activatemodedata;
    GDestroyNotify                 activatemodenotify;
    GstPad			*peer;
    GstPadLinkFunction		 linkfunc;
    gpointer                       linkdata;
    GDestroyNotify                 linknotify;
    GstPadUnlinkFunction		 unlinkfunc;
    gpointer                       unlinkdata;
    GDestroyNotify                 unlinknotify;
    GstPadChainFunction		 chainfunc;
    gpointer                       chaindata;
    GDestroyNotify                 chainnotify;
    GstPadChainListFunction        chainlistfunc;
    gpointer                       chainlistdata;
    GDestroyNotify                 chainlistnotify;
    GstPadGetRangeFunction	 getrangefunc;
    gpointer                       getrangedata;
    GDestroyNotify                 getrangenotify;
    GstPadEventFunction		 eventfunc;
    gpointer                       eventdata;
    GDestroyNotify                 eventnotify;
    gint64                         offset;
    GstPadQueryFunction		 queryfunc;
    gpointer                       querydata;
    GDestroyNotify                 querynotify;
    GstPadIterIntLinkFunction      iterintlinkfunc;
    gpointer                       iterintlinkdata;
    GDestroyNotify                 iterintlinknotify;
    gint				 num_probes;
    gint				 num_blocked;
    GstPadPrivate                 *priv;
    union {
        gpointer _gst_reserved[GST_PADDING];
        struct {
            GstFlowReturn last_flowret;
            GstPadEventFullFunction eventfullfunc;
        } abi;
    } ABI;
};
#define GST_PAD_TEMPLATE_NAME_TEMPLATE(templ)	(((GstPadTemplate *)(templ))->name_template)
#define GST_PAD_TEMPLATE_DIRECTION(templ)	(((GstPadTemplate *)(templ))->direction)
#define GST_PAD_TEMPLATE_PRESENCE(templ)	(((GstPadTemplate *)(templ))->presence)
#define GST_PAD_TEMPLATE_CAPS(templ)		(((GstPadTemplate *)(templ))->caps)
typedef enum {
  GST_PAD_TEMPLATE_FLAG_LAST    = (GST_OBJECT_FLAG_LAST << 4)
} GstPadTemplateFlags;
#define GST_PAD_TEMPLATE_IS_FIXED(templ)	(GST_OBJECT_FLAG_IS_SET(templ, GST_PAD_TEMPLATE_FIXED))
struct _GstPadTemplate {
  GstObject	   object;
  gchar           *name_template;
  GstPadDirection  direction;
  GstPadPresence   presence;
  GstCaps	  *caps;
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstPadTemplateClass {
  GstObjectClass   parent_class;
  void (*pad_created)	(GstPadTemplate *templ, GstPad *pad);
  gpointer _gst_reserved[GST_PADDING];
};
struct _GstStaticPadTemplate {
  const gchar     *name_template;
  GstPadDirection  direction;
  GstPadPresence   presence;
  GstStaticCaps    static_caps;
};
#define GST_STATIC_PAD_TEMPLATE(padname, dir, pres, caps)  { padname, dir, pres, caps }
GType			gst_pad_template_get_type		(void);
GType			gst_static_pad_template_get_type	(void);
GstPadTemplate *gst_pad_template_new(const gchar *name_template, GstPadDirection direction, GstPadPresence presence, GstCaps *caps) G_GNUC_MALLOC;
GstPadTemplate *	gst_static_pad_template_get             (GstStaticPadTemplate *pad_template);
GstCaps*		gst_static_pad_template_get_caps	(GstStaticPadTemplate *templ);
GstCaps*		gst_pad_template_get_caps		(GstPadTemplate *templ);
void                    gst_pad_template_pad_created            (GstPadTemplate * templ, GstPad * pad);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstPadTemplate, gst_object_unref)
#endif
G_END_DECLS

#endif