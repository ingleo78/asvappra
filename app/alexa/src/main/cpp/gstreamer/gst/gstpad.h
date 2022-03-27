#ifndef __GST_PAD_H__
#define __GST_PAD_H__

#include <glib/glib.h>
#include "gstconfig.h"
#include "gstobject.h"
#include "gstbufferlist.h"
#include "gstcaps.h"
#include "gstpadtemplate.h"
#include "gstevent.h"
#include "gstquery.h"
#include "gsttask.h"
#include "gstiterator.h"

typedef struct _GstPadClass GstPadClass;
typedef struct _GstPadProbeInfo GstPadProbeInfo;
const gchar   * gst_pad_mode_get_name (GstPadMode mode);
G_BEGIN_DECLS
#define GST_TYPE_PAD			(gst_pad_get_type ())
#define GST_IS_PAD(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_PAD))
#define GST_IS_PAD_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_PAD))
#define GST_PAD(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_PAD, GstPad))
#define GST_PAD_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_PAD, GstPadClass))
#define GST_PAD_CAST(obj)		((GstPad*)(obj))
#define GST_PAD_LINK_FAILED(ret) ((ret) < GST_PAD_LINK_OK)
#define GST_PAD_LINK_SUCCESSFUL(ret) ((ret) >= GST_PAD_LINK_OK)
const gchar*	        gst_flow_get_name (GstFlowReturn ret);
GQuark			          gst_flow_to_quark (GstFlowReturn ret);
const gchar*          gst_pad_link_get_name (GstPadLinkReturn ret);
typedef enum {
  GST_PAD_PROBE_TYPE_INVALID          = 0,
  GST_PAD_PROBE_TYPE_IDLE             = (1 << 0),
  GST_PAD_PROBE_TYPE_BLOCK            = (1 << 1),
  GST_PAD_PROBE_TYPE_BUFFER           = (1 << 4),
  GST_PAD_PROBE_TYPE_BUFFER_LIST      = (1 << 5),
  GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM = (1 << 6),
  GST_PAD_PROBE_TYPE_EVENT_UPSTREAM   = (1 << 7),
  GST_PAD_PROBE_TYPE_EVENT_FLUSH      = (1 << 8),
  GST_PAD_PROBE_TYPE_QUERY_DOWNSTREAM = (1 << 9),
  GST_PAD_PROBE_TYPE_QUERY_UPSTREAM   = (1 << 10),
  GST_PAD_PROBE_TYPE_PUSH             = (1 << 12),
  GST_PAD_PROBE_TYPE_PULL             = (1 << 13),
  GST_PAD_PROBE_TYPE_BLOCKING         = GST_PAD_PROBE_TYPE_IDLE | GST_PAD_PROBE_TYPE_BLOCK,
  GST_PAD_PROBE_TYPE_DATA_DOWNSTREAM  = GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST | GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
  GST_PAD_PROBE_TYPE_DATA_UPSTREAM    = GST_PAD_PROBE_TYPE_EVENT_UPSTREAM,
  GST_PAD_PROBE_TYPE_DATA_BOTH        = GST_PAD_PROBE_TYPE_DATA_DOWNSTREAM | GST_PAD_PROBE_TYPE_DATA_UPSTREAM,
  GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM = GST_PAD_PROBE_TYPE_BLOCK | GST_PAD_PROBE_TYPE_DATA_DOWNSTREAM,
  GST_PAD_PROBE_TYPE_BLOCK_UPSTREAM   = GST_PAD_PROBE_TYPE_BLOCK | GST_PAD_PROBE_TYPE_DATA_UPSTREAM,
  GST_PAD_PROBE_TYPE_EVENT_BOTH       = GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM | GST_PAD_PROBE_TYPE_EVENT_UPSTREAM,
  GST_PAD_PROBE_TYPE_QUERY_BOTH       = GST_PAD_PROBE_TYPE_QUERY_DOWNSTREAM | GST_PAD_PROBE_TYPE_QUERY_UPSTREAM,
  GST_PAD_PROBE_TYPE_ALL_BOTH         = GST_PAD_PROBE_TYPE_DATA_BOTH | GST_PAD_PROBE_TYPE_QUERY_BOTH,
  GST_PAD_PROBE_TYPE_SCHEDULING       = GST_PAD_PROBE_TYPE_PUSH | GST_PAD_PROBE_TYPE_PULL
} GstPadProbeType;
typedef enum {
  GST_PAD_PROBE_DROP,
  GST_PAD_PROBE_OK,
  GST_PAD_PROBE_REMOVE,
  GST_PAD_PROBE_PASS,
  GST_PAD_PROBE_HANDLED
} GstPadProbeReturn;
struct _GstPadProbeInfo {
  GstPadProbeType type;
  gulong id;
  gpointer data;
  guint64 offset;
  guint size;
  union {
    gpointer _gst_reserved[GST_PADDING];
    struct {
      GstFlowReturn flow_ret;
    } abi;
  } ABI;
};
#define GST_PAD_PROBE_INFO_TYPE(d)         ((d)->type)
#define GST_PAD_PROBE_INFO_ID(d)           ((d)->id)
#define GST_PAD_PROBE_INFO_DATA(d)         ((d)->data)
#define GST_PAD_PROBE_INFO_FLOW_RETURN(d)  ((d)->ABI.abi.flow_ret)
#define GST_PAD_PROBE_INFO_BUFFER(d)       GST_BUFFER_CAST(GST_PAD_PROBE_INFO_DATA(d))
#define GST_PAD_PROBE_INFO_BUFFER_LIST(d)  GST_BUFFER_LIST_CAST(GST_PAD_PROBE_INFO_DATA(d))
#define GST_PAD_PROBE_INFO_EVENT(d)        GST_EVENT_CAST(GST_PAD_PROBE_INFO_DATA(d))
#define GST_PAD_PROBE_INFO_QUERY(d)        GST_QUERY_CAST(GST_PAD_PROBE_INFO_DATA(d))
#define GST_PAD_PROBE_INFO_OFFSET(d)       ((d)->offset)
#define GST_PAD_PROBE_INFO_SIZE(d)         ((d)->size)
GstEvent*      gst_pad_probe_info_get_event       (GstPadProbeInfo * info);
GstQuery*      gst_pad_probe_info_get_query       (GstPadProbeInfo * info);
GstBuffer*     gst_pad_probe_info_get_buffer      (GstPadProbeInfo * info);
GstBufferList* gst_pad_probe_info_get_buffer_list (GstPadProbeInfo * info);
typedef GstPadProbeReturn   (*GstPadProbeCallback)   (GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
typedef gboolean  (*GstPadStickyEventsForeachFunction) (GstPad *pad, GstEvent **event, gpointer user_data);
typedef enum {
  GST_PAD_FLAG_BLOCKED          = (GST_OBJECT_FLAG_LAST << 0),
  GST_PAD_FLAG_FLUSHING         = (GST_OBJECT_FLAG_LAST << 1),
  GST_PAD_FLAG_EOS              = (GST_OBJECT_FLAG_LAST << 2),
  GST_PAD_FLAG_BLOCKING         = (GST_OBJECT_FLAG_LAST << 3),
  GST_PAD_FLAG_NEED_PARENT      = (GST_OBJECT_FLAG_LAST << 4),
  GST_PAD_FLAG_NEED_RECONFIGURE = (GST_OBJECT_FLAG_LAST << 5),
  GST_PAD_FLAG_PENDING_EVENTS   = (GST_OBJECT_FLAG_LAST << 6),
  GST_PAD_FLAG_FIXED_CAPS       = (GST_OBJECT_FLAG_LAST << 7),
  GST_PAD_FLAG_PROXY_CAPS       = (GST_OBJECT_FLAG_LAST << 8),
  GST_PAD_FLAG_PROXY_ALLOCATION = (GST_OBJECT_FLAG_LAST << 9),
  GST_PAD_FLAG_PROXY_SCHEDULING = (GST_OBJECT_FLAG_LAST << 10),
  GST_PAD_FLAG_ACCEPT_INTERSECT = (GST_OBJECT_FLAG_LAST << 11),
  GST_PAD_FLAG_ACCEPT_TEMPLATE  = (GST_OBJECT_FLAG_LAST << 12),
  GST_PAD_FLAG_LAST        = (GST_OBJECT_FLAG_LAST << 16)
} GstPadFlags;
struct _GstPadClass {
  GstObjectClass	parent_class;
  void		(*linked)		(GstPad *pad, GstPad *peer);
  void		(*unlinked)		(GstPad *pad, GstPad *peer);
  gpointer _gst_reserved[GST_PADDING];
};
#define GST_PAD_NAME(pad)		(GST_OBJECT_NAME(pad))
#define GST_PAD_PARENT(pad)		(GST_ELEMENT_CAST(GST_OBJECT_PARENT(pad)))
#define GST_PAD_ELEMENT_PRIVATE(pad)    (GST_PAD_CAST(pad)->element_private)
#define GST_PAD_PAD_TEMPLATE(pad)	(GST_PAD_CAST(pad)->padtemplate)
#define GST_PAD_DIRECTION(pad)		(GST_PAD_CAST(pad)->direction)
#define GST_PAD_TASK(pad)		(GST_PAD_CAST(pad)->task)
#define GST_PAD_MODE(pad)	        (GST_PAD_CAST(pad)->mode)
#define GST_PAD_ACTIVATEFUNC(pad)	(GST_PAD_CAST(pad)->activatefunc)
#define GST_PAD_ACTIVATEMODEFUNC(pad)	(GST_PAD_CAST(pad)->activatemodefunc)
#define GST_PAD_CHAINFUNC(pad)		(GST_PAD_CAST(pad)->chainfunc)
#define GST_PAD_CHAINLISTFUNC(pad)      (GST_PAD_CAST(pad)->chainlistfunc)
#define GST_PAD_GETRANGEFUNC(pad)	(GST_PAD_CAST(pad)->getrangefunc)
#define GST_PAD_EVENTFUNC(pad)		(GST_PAD_CAST(pad)->eventfunc)
#define GST_PAD_EVENTFULLFUNC(pad)	(GST_PAD_CAST(pad)->ABI.abi.eventfullfunc)
#define GST_PAD_QUERYFUNC(pad)		(GST_PAD_CAST(pad)->queryfunc)
#define GST_PAD_ITERINTLINKFUNC(pad)    (GST_PAD_CAST(pad)->iterintlinkfunc)
#define GST_PAD_PEER(pad)		(GST_PAD_CAST(pad)->peer)
#define GST_PAD_LINKFUNC(pad)		(GST_PAD_CAST(pad)->linkfunc)
#define GST_PAD_UNLINKFUNC(pad)		(GST_PAD_CAST(pad)->unlinkfunc)
#define GST_PAD_IS_SRC(pad)		(GST_PAD_DIRECTION(pad) == GST_PAD_SRC)
#define GST_PAD_IS_SINK(pad)		(GST_PAD_DIRECTION(pad) == GST_PAD_SINK)
#define GST_PAD_IS_LINKED(pad)		(GST_PAD_PEER(pad) != NULL)
#define GST_PAD_IS_ACTIVE(pad)          (GST_PAD_MODE(pad) != GST_PAD_MODE_NONE)
#define GST_PAD_IS_BLOCKED(pad)		(GST_OBJECT_FLAG_IS_SET (pad, GST_PAD_FLAG_BLOCKED))
#define GST_PAD_IS_BLOCKING(pad)	(GST_OBJECT_FLAG_IS_SET (pad, GST_PAD_FLAG_BLOCKING))
#define GST_PAD_IS_FLUSHING(pad)	(GST_OBJECT_FLAG_IS_SET (pad, GST_PAD_FLAG_FLUSHING))
#define GST_PAD_SET_FLUSHING(pad)	(GST_OBJECT_FLAG_SET (pad, GST_PAD_FLAG_FLUSHING))
#define GST_PAD_UNSET_FLUSHING(pad)	(GST_OBJECT_FLAG_UNSET (pad, GST_PAD_FLAG_FLUSHING))
#define GST_PAD_IS_EOS(pad)	        (GST_OBJECT_FLAG_IS_SET (pad, GST_PAD_FLAG_EOS))
#define GST_PAD_NEEDS_RECONFIGURE(pad)  (GST_OBJECT_FLAG_IS_SET (pad, GST_PAD_FLAG_NEED_RECONFIGURE))
#define GST_PAD_HAS_PENDING_EVENTS(pad) (GST_OBJECT_FLAG_IS_SET (pad, GST_PAD_FLAG_PENDING_EVENTS))
#define GST_PAD_IS_FIXED_CAPS(pad)	(GST_OBJECT_FLAG_IS_SET (pad, GST_PAD_FLAG_FIXED_CAPS))
#define GST_PAD_NEEDS_PARENT(pad)       (GST_OBJECT_FLAG_IS_SET (pad, GST_PAD_FLAG_NEED_PARENT))
#define GST_PAD_IS_PROXY_CAPS(pad)      (GST_OBJECT_FLAG_IS_SET (pad, GST_PAD_FLAG_PROXY_CAPS))
#define GST_PAD_SET_PROXY_CAPS(pad)     (GST_OBJECT_FLAG_SET (pad, GST_PAD_FLAG_PROXY_CAPS))
#define GST_PAD_UNSET_PROXY_CAPS(pad)   (GST_OBJECT_FLAG_UNSET (pad, GST_PAD_FLAG_PROXY_CAPS))
#define GST_PAD_IS_PROXY_ALLOCATION(pad)    (GST_OBJECT_FLAG_IS_SET (pad, GST_PAD_FLAG_PROXY_ALLOCATION))
#define GST_PAD_SET_PROXY_ALLOCATION(pad)   (GST_OBJECT_FLAG_SET (pad, GST_PAD_FLAG_PROXY_ALLOCATION))
#define GST_PAD_UNSET_PROXY_ALLOCATION(pad) (GST_OBJECT_FLAG_UNSET (pad, GST_PAD_FLAG_PROXY_ALLOCATION))
#define GST_PAD_IS_PROXY_SCHEDULING(pad)    (GST_OBJECT_FLAG_IS_SET (pad, GST_PAD_FLAG_PROXY_SCHEDULING))
#define GST_PAD_SET_PROXY_SCHEDULING(pad)   (GST_OBJECT_FLAG_SET (pad, GST_PAD_FLAG_PROXY_SCHEDULING))
#define GST_PAD_UNSET_PROXY_SCHEDULING(pad) (GST_OBJECT_FLAG_UNSET (pad, GST_PAD_FLAG_PROXY_SCHEDULING))
#define GST_PAD_IS_ACCEPT_INTERSECT(pad)    (GST_OBJECT_FLAG_IS_SET (pad, GST_PAD_FLAG_ACCEPT_INTERSECT))
#define GST_PAD_SET_ACCEPT_INTERSECT(pad)   (GST_OBJECT_FLAG_SET (pad, GST_PAD_FLAG_ACCEPT_INTERSECT))
#define GST_PAD_UNSET_ACCEPT_INTERSECT(pad) (GST_OBJECT_FLAG_UNSET (pad, GST_PAD_FLAG_ACCEPT_INTERSECT))
#define GST_PAD_IS_ACCEPT_TEMPLATE(pad)    (GST_OBJECT_FLAG_IS_SET (pad, GST_PAD_FLAG_ACCEPT_TEMPLATE))
#define GST_PAD_SET_ACCEPT_TEMPLATE(pad)   (GST_OBJECT_FLAG_SET (pad, GST_PAD_FLAG_ACCEPT_TEMPLATE))
#define GST_PAD_UNSET_ACCEPT_TEMPLATE(pad) (GST_OBJECT_FLAG_UNSET (pad, GST_PAD_FLAG_ACCEPT_TEMPLATE))
#define GST_PAD_GET_STREAM_LOCK(pad)    (&(GST_PAD_CAST(pad)->stream_rec_lock))
#define GST_PAD_STREAM_LOCK(pad)        g_rec_mutex_lock(GST_PAD_GET_STREAM_LOCK(pad))
#define GST_PAD_STREAM_TRYLOCK(pad)     g_rec_mutex_trylock(GST_PAD_GET_STREAM_LOCK(pad))
#define GST_PAD_STREAM_UNLOCK(pad)      g_rec_mutex_unlock(GST_PAD_GET_STREAM_LOCK(pad))
#define GST_PAD_LAST_FLOW_RETURN(pad)   (GST_PAD_CAST(pad)->ABI.abi.last_flowret)
#define GST_PAD_BLOCK_GET_COND(pad)     (&GST_PAD_CAST(pad)->block_cond)
#define GST_PAD_BLOCK_WAIT(pad)         (g_cond_wait(GST_PAD_BLOCK_GET_COND (pad), GST_OBJECT_GET_LOCK (pad)))
#define GST_PAD_BLOCK_SIGNAL(pad)       (g_cond_signal(GST_PAD_BLOCK_GET_COND (pad)))
#define GST_PAD_BLOCK_BROADCAST(pad)    (g_cond_broadcast(GST_PAD_BLOCK_GET_COND (pad)))
GType			gst_pad_get_type			(void);
GstPad*			gst_pad_new				(const gchar *name, GstPadDirection direction);
GstPad*			gst_pad_new_from_template		(GstPadTemplate *templ, const gchar *name);
GstPad*			gst_pad_new_from_static_template	(GstStaticPadTemplate *templ, const gchar *name);
#define gst_pad_get_name(pad) gst_object_get_name (GST_OBJECT_CAST (pad))
#define gst_pad_get_parent(pad) gst_object_get_parent (GST_OBJECT_CAST (pad))
GstPadDirection		gst_pad_get_direction			(GstPad *pad);
gboolean		gst_pad_set_active			(GstPad *pad, gboolean active);
gboolean		gst_pad_is_active			(GstPad *pad);
gboolean		gst_pad_activate_mode			(GstPad *pad, GstPadMode mode, gboolean active);
gulong gst_pad_add_probe(GstPad *pad, GstPadProbeType mask, GstPadProbeCallback callback, gpointer user_data, GDestroyNotify destroy_data);
void                    gst_pad_remove_probe                    (GstPad *pad, gulong id);
gboolean		gst_pad_is_blocked			(GstPad *pad);
gboolean		gst_pad_is_blocking			(GstPad *pad);
void                    gst_pad_mark_reconfigure                (GstPad *pad);
gboolean		gst_pad_needs_reconfigure               (GstPad *pad);
gboolean		gst_pad_check_reconfigure               (GstPad *pad);
void			gst_pad_set_element_private		(GstPad *pad, gpointer priv);
gpointer		gst_pad_get_element_private		(GstPad *pad);
GstPadTemplate*		gst_pad_get_pad_template		(GstPad *pad);
GstFlowReturn           gst_pad_store_sticky_event              (GstPad *pad, GstEvent *event);
GstEvent*               gst_pad_get_sticky_event                (GstPad *pad, GstEventType event_type, guint idx);
void                    gst_pad_sticky_events_foreach(GstPad *pad, GstPadStickyEventsForeachFunction foreach_func, gpointer user_data);
void			gst_pad_set_activate_function_full	(GstPad *pad, GstPadActivateFunction activate, gpointer user_data, GDestroyNotify notify);
void gst_pad_set_activatemode_function_full(GstPad *pad, GstPadActivateModeFunction activatemode, gpointer user_data, GDestroyNotify notify);
void			gst_pad_set_chain_function_full		(GstPad *pad, GstPadChainFunction chain, gpointer user_data, GDestroyNotify notify);
void			gst_pad_set_chain_list_function_full(GstPad *pad, GstPadChainListFunction chainlist, gpointer user_data, GDestroyNotify notify);
void			gst_pad_set_getrange_function_full	(GstPad *pad, GstPadGetRangeFunction get, gpointer user_data, GDestroyNotify notify);
void			gst_pad_set_event_function_full		(GstPad *pad, GstPadEventFunction event, gpointer user_data, GDestroyNotify notify);
void			gst_pad_set_event_full_function_full	(GstPad *pad, GstPadEventFullFunction event, gpointer user_data, GDestroyNotify notify);
#define gst_pad_set_activate_function(p,f)      gst_pad_set_activate_function_full((p),(f),NULL,NULL)
#define gst_pad_set_activatemode_function(p,f)  gst_pad_set_activatemode_function_full((p),(f),NULL,NULL)
#define gst_pad_set_chain_function(p,f)         gst_pad_set_chain_function_full((p),(f),NULL,NULL)
#define gst_pad_set_chain_list_function(p,f)    gst_pad_set_chain_list_function_full((p),(f),NULL,NULL)
#define gst_pad_set_getrange_function(p,f)      gst_pad_set_getrange_function_full((p),(f),NULL,NULL)
#define gst_pad_set_event_function(p,f)         gst_pad_set_event_function_full((p),(f),NULL,NULL)
#define gst_pad_set_event_full_function(p,f)    gst_pad_set_event_full_function_full((p),(f),NULL,NULL)
void			gst_pad_set_link_function_full		(GstPad *pad, GstPadLinkFunction link, gpointer user_data, GDestroyNotify notify);
void			gst_pad_set_unlink_function_full        (GstPad *pad, GstPadUnlinkFunction unlink, gpointer user_data, GDestroyNotify notify);
#define gst_pad_set_link_function(p,f)          gst_pad_set_link_function_full((p),(f),NULL,NULL)
#define gst_pad_set_unlink_function(p,f)        gst_pad_set_unlink_function_full((p),(f),NULL,NULL)
gboolean                gst_pad_can_link                        (GstPad *srcpad, GstPad *sinkpad);
GstPadLinkReturn        gst_pad_link				(GstPad *srcpad, GstPad *sinkpad);
GstPadLinkReturn        gst_pad_link_full			(GstPad *srcpad, GstPad *sinkpad, GstPadLinkCheck flags);
gboolean		gst_pad_unlink				(GstPad *srcpad, GstPad *sinkpad);
gboolean		gst_pad_is_linked			(GstPad *pad);
GstPad*			gst_pad_get_peer			(GstPad *pad);
GstCaps*                gst_pad_get_pad_template_caps		(GstPad *pad);
GstCaps *		gst_pad_get_current_caps                (GstPad * pad);
gboolean		gst_pad_has_current_caps                (GstPad * pad);
GstCaps *		gst_pad_get_allowed_caps		(GstPad * pad);
gint64                  gst_pad_get_offset                      (GstPad *pad);
void                    gst_pad_set_offset                      (GstPad *pad, gint64 offset);
GstFlowReturn		gst_pad_push				(GstPad *pad, GstBuffer *buffer);
GstFlowReturn		gst_pad_push_list			(GstPad *pad, GstBufferList *list);
GstFlowReturn		gst_pad_pull_range			(GstPad *pad, guint64 offset, guint size, GstBuffer **buffer);
gboolean		gst_pad_push_event			(GstPad *pad, GstEvent *event);
gboolean		gst_pad_event_default			(GstPad *pad, GstObject *parent, GstEvent *event);
GstFlowReturn           gst_pad_get_last_flow_return            (GstPad *pad);
GstFlowReturn		gst_pad_chain				(GstPad *pad, GstBuffer *buffer);
GstFlowReturn		gst_pad_chain_list                      (GstPad *pad, GstBufferList *list);
GstFlowReturn		gst_pad_get_range			(GstPad *pad, guint64 offset, guint size, GstBuffer **buffer);
gboolean		gst_pad_send_event			(GstPad *pad, GstEvent *event);
gboolean		gst_pad_start_task			(GstPad *pad, GstTaskFunction func, gpointer user_data, GDestroyNotify notify);
gboolean		gst_pad_pause_task			(GstPad *pad);
gboolean		gst_pad_stop_task			(GstPad *pad);
void gst_pad_set_iterate_internal_links_function_full(GstPad *pad, GstPadIterIntLinkFunction iterintlink, gpointer user_data,
                                                      GDestroyNotify notify);
GstIterator *           gst_pad_iterate_internal_links          (GstPad * pad);
GstIterator *           gst_pad_iterate_internal_links_default  (GstPad * pad, GstObject *parent);
#define gst_pad_set_iterate_internal_links_function(p,f) gst_pad_set_iterate_internal_links_function_full((p),(f),NULL,NULL)
gboolean		gst_pad_query				(GstPad *pad, GstQuery *query);
gboolean		gst_pad_peer_query			(GstPad *pad, GstQuery *query);
void			gst_pad_set_query_function_full		(GstPad *pad, GstPadQueryFunction query, gpointer user_data, GDestroyNotify notify);
gboolean		gst_pad_query_default			(GstPad *pad, GstObject *parent, GstQuery *query);
#define gst_pad_set_query_function(p,f)   gst_pad_set_query_function_full((p),(f),NULL,NULL)
gboolean		gst_pad_forward                         (GstPad *pad, GstPadForwardFunction forward, gpointer user_data);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstPad, gst_object_unref)
#endif
G_END_DECLS

#endif