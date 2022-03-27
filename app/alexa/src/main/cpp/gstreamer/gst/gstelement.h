#ifndef __GST_ELEMENT_H__
#define __GST_ELEMENT_H__

#include <glib/glib.h>
#include "gstconfig.h"
#include "gstobject.h"
#include "gstbus.h"
#include "gstelementfactory.h"
#include "gstplugin.h"
#include "gstpluginfeature.h"
#include "gstiterator.h"
#include "gsttaglist.h"
#include "gstevent.h"
#include "gstcontext.h"
#include "gstpadtemplate.h"
#include "gstcaps.h"
#include "gstsegment.h"
#include "gstpad.h"

typedef struct _GstElementClass GstElementClass;
G_BEGIN_DECLS
#define G_VALUE_INIT  { 0, { { 0 } } }
#define GST_TYPE_ELEMENT                (gst_element_get_type ())
#define GST_IS_ELEMENT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_ELEMENT))
#define GST_IS_ELEMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_ELEMENT))
#define GST_ELEMENT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_ELEMENT, GstElementClass))
#define GST_ELEMENT(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_ELEMENT, GstElement))
#define GST_ELEMENT_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_ELEMENT, GstElementClass))
#define GST_ELEMENT_CAST(obj)           ((GstElement*)(obj))
#define GST_STATE(elem)                 (GST_ELEMENT_CAST(elem)->current_state)
#define GST_STATE_NEXT(elem)            (GST_ELEMENT_CAST(elem)->next_state)
#define GST_STATE_PENDING(elem)         (GST_ELEMENT_CAST(elem)->pending_state)
#define GST_STATE_TARGET(elem)          (GST_ELEMENT_CAST(elem)->target_state)
#define GST_STATE_RETURN(elem)          (GST_ELEMENT_CAST(elem)->last_return)
#define __GST_SIGN(val)                 ((val) < 0 ? -1 : ((val) > 0 ? 1 : 0))
#define GST_STATE_GET_NEXT(cur,pending)         ((GstState)((cur) + __GST_SIGN ((gint)(pending) - (gint)(cur))))
#define GST_STATE_TRANSITION(cur,next)          ((GstStateChange)(((cur)<<3)|(next)))
#define GST_STATE_TRANSITION_CURRENT(trans)     ((GstState)((trans)>>3))
#define GST_STATE_TRANSITION_NEXT(trans)        ((GstState)((trans)&0x7))
typedef enum {
  GST_STATE_CHANGE_NULL_TO_READY        = (GST_STATE_NULL<<3) | GST_STATE_READY,
  GST_STATE_CHANGE_READY_TO_PAUSED      = (GST_STATE_READY<<3) | GST_STATE_PAUSED,
  GST_STATE_CHANGE_PAUSED_TO_PLAYING    = (GST_STATE_PAUSED<<3) | GST_STATE_PLAYING,
  GST_STATE_CHANGE_PLAYING_TO_PAUSED    = (GST_STATE_PLAYING<<3) | GST_STATE_PAUSED,
  GST_STATE_CHANGE_PAUSED_TO_READY      = (GST_STATE_PAUSED<<3) | GST_STATE_READY,
  GST_STATE_CHANGE_READY_TO_NULL        = (GST_STATE_READY<<3) | GST_STATE_NULL
} GstStateChange;
typedef enum {
  GST_ELEMENT_FLAG_LOCKED_STATE   = (GST_OBJECT_FLAG_LAST << 0),
  GST_ELEMENT_FLAG_SINK           = (GST_OBJECT_FLAG_LAST << 1),
  GST_ELEMENT_FLAG_SOURCE         = (GST_OBJECT_FLAG_LAST << 2),
  GST_ELEMENT_FLAG_PROVIDE_CLOCK  = (GST_OBJECT_FLAG_LAST << 3),
  GST_ELEMENT_FLAG_REQUIRE_CLOCK  = (GST_OBJECT_FLAG_LAST << 4),
  GST_ELEMENT_FLAG_INDEXABLE      = (GST_OBJECT_FLAG_LAST << 5),
  GST_ELEMENT_FLAG_LAST           = (GST_OBJECT_FLAG_LAST << 10)
} GstElementFlags;
#define GST_ELEMENT_IS_LOCKED_STATE(elem)        (GST_OBJECT_FLAG_IS_SET(elem,GST_ELEMENT_FLAG_LOCKED_STATE))
#define GST_ELEMENT_NAME(elem)                  (GST_OBJECT_NAME(elem))
#define GST_ELEMENT_PARENT(elem)                (GST_ELEMENT_CAST(GST_OBJECT_PARENT(elem)))
#define GST_ELEMENT_BUS(elem)                   (GST_ELEMENT_CAST(elem)->bus)
#define GST_ELEMENT_CLOCK(elem)                 (GST_ELEMENT_CAST(elem)->clock)
#define GST_ELEMENT_PADS(elem)                  (GST_ELEMENT_CAST(elem)->pads)
#define GST_ELEMENT_START_TIME(elem)            (GST_ELEMENT_CAST(elem)->start_time)
#define GST_ELEMENT_ERROR(el, domain, code, text, debug)                \
G_STMT_START {                                                          \
  gchar *__txt = _gst_element_error_printf text;                        \
  gchar *__dbg = _gst_element_error_printf debug;                       \
  if (__txt)                                                            \
    GST_WARNING_OBJECT (el, "error: %s", __txt);                        \
  if (__dbg)                                                            \
    GST_WARNING_OBJECT (el, "error: %s", __dbg);                        \
  gst_element_message_full (GST_ELEMENT(el), GST_MESSAGE_ERROR,         \
    GST_ ## domain ## _ERROR, GST_ ## domain ## _ERROR_ ## code,        \
    __txt, __dbg, __FILE__, GST_FUNCTION, __LINE__);                    \
} G_STMT_END
#define GST_ELEMENT_WARNING(el, domain, code, text, debug)              \
G_STMT_START {                                                          \
  gchar *__txt = _gst_element_error_printf text;                        \
  gchar *__dbg = _gst_element_error_printf debug;                       \
  if (__txt)                                                            \
    GST_WARNING_OBJECT (el, "warning: %s", __txt);                      \
  if (__dbg)                                                            \
    GST_WARNING_OBJECT (el, "warning: %s", __dbg);                      \
  gst_element_message_full (GST_ELEMENT(el), GST_MESSAGE_WARNING,       \
    GST_ ## domain ## _ERROR, GST_ ## domain ## _ERROR_ ## code,        \
  __txt, __dbg, __FILE__, GST_FUNCTION, __LINE__);                      \
} G_STMT_END
#define GST_ELEMENT_INFO(el, domain, code, text, debug)                 \
G_STMT_START {                                                          \
  gchar *__txt = _gst_element_error_printf text;                        \
  gchar *__dbg = _gst_element_error_printf debug;                       \
  if (__txt)                                                            \
    GST_INFO_OBJECT (el, "info: %s", __txt);                            \
  if (__dbg)                                                            \
    GST_INFO_OBJECT (el, "info: %s", __dbg);                            \
  gst_element_message_full (GST_ELEMENT(el), GST_MESSAGE_INFO,          \
    GST_ ## domain ## _ERROR, GST_ ## domain ## _ERROR_ ## code,        \
  __txt, __dbg, __FILE__, GST_FUNCTION, __LINE__);                      \
} G_STMT_END
#define GST_STATE_GET_LOCK(elem)               (&(GST_ELEMENT_CAST(elem)->state_lock))
#define GST_STATE_GET_COND(elem)               (&GST_ELEMENT_CAST(elem)->state_cond)
#define GST_STATE_LOCK(elem)                   g_rec_mutex_lock(GST_STATE_GET_LOCK(elem))
#define GST_STATE_TRYLOCK(elem)                g_rec_mutex_trylock(GST_STATE_GET_LOCK(elem))
#define GST_STATE_UNLOCK(elem)                 g_rec_mutex_unlock(GST_STATE_GET_LOCK(elem))
#define GST_STATE_WAIT(elem)                   g_cond_wait (GST_STATE_GET_COND (elem), GST_OBJECT_GET_LOCK (elem))
#define GST_STATE_WAIT_UNTIL(elem, end_time)   g_cond_wait_until (GST_STATE_GET_COND (elem), GST_OBJECT_GET_LOCK (elem), end_time)
#define GST_STATE_SIGNAL(elem)                 g_cond_signal (GST_STATE_GET_COND (elem));
#define GST_STATE_BROADCAST(elem)              g_cond_broadcast (GST_STATE_GET_COND (elem));
struct _GstElementClass {
  GstObjectClass         parent_class;
  gpointer		 metadata;
  GstElementFactory     *elementfactory;
  GList                 *padtemplates;
  gint                   numpadtemplates;
  guint32                pad_templ_cookie;
  void (*pad_added)     (GstElement *element, GstPad *pad);
  void (*pad_removed)   (GstElement *element, GstPad *pad);
  void (*no_more_pads)  (GstElement *element);
  GstPad*               (*request_new_pad)      (GstElement *element, GstPadTemplate *templ,
                                                 const gchar* name, const GstCaps *caps);
  void                  (*release_pad)          (GstElement *element, GstPad *pad);
  GstStateChangeReturn (*get_state)             (GstElement * element, GstState * state,
                                                 GstState * pending, GstClockTime timeout);
  GstStateChangeReturn (*set_state)             (GstElement *element, GstState state);
  GstStateChangeReturn (*change_state)          (GstElement *element, GstStateChange transition);
  void                 (*state_changed)         (GstElement *element, GstState oldstate,
                                                 GstState newstate, GstState pending);
  void                  (*set_bus)              (GstElement * element, GstBus * bus);
  GstClock*             (*provide_clock)        (GstElement *element);
  gboolean              (*set_clock)            (GstElement *element, GstClock *clock);
  gboolean              (*send_event)           (GstElement *element, GstEvent *event);
  gboolean              (*query)                (GstElement *element, GstQuery *query);
  gboolean              (*post_message)         (GstElement *element, GstMessage *message);
  void                  (*set_context)          (GstElement *element, GstContext *context);
  gpointer _gst_reserved[GST_PADDING_LARGE-2];
};
void                    gst_element_class_add_pad_template      (GstElementClass *klass, GstPadTemplate *templ);
void                    gst_element_class_add_static_pad_template (GstElementClass *klass, GstStaticPadTemplate *static_templ);
GstPadTemplate*         gst_element_class_get_pad_template      (GstElementClass *element_class, const gchar *name);
GList*                  gst_element_class_get_pad_template_list (GstElementClass *element_class);
void                    gst_element_class_set_metadata          (GstElementClass *klass,
                                                                 const gchar     *longname,
                                                                 const gchar     *classification,
                                                                 const gchar     *description,
                                                                 const gchar     *author);
void                    gst_element_class_set_static_metadata   (GstElementClass *klass,
                                                                 const gchar     *longname,
                                                                 const gchar     *classification,
                                                                 const gchar     *description,
                                                                 const gchar     *author);
void                    gst_element_class_add_metadata          (GstElementClass * klass,
                                                                 const gchar * key, const gchar * value);
void                    gst_element_class_add_static_metadata   (GstElementClass * klass,
                                                                 const gchar * key, const gchar * value);
const gchar *           gst_element_class_get_metadata          (GstElementClass * klass,
                                                                 const gchar * key);
GType                   gst_element_get_type            (void);
#define                 gst_element_get_name(elem)      gst_object_get_name(GST_OBJECT_CAST(elem))
#define                 gst_element_set_name(elem,name) gst_object_set_name(GST_OBJECT_CAST(elem),name)
#define                 gst_element_get_parent(elem)    gst_object_get_parent(GST_OBJECT_CAST(elem))
#define                 gst_element_set_parent(elem,parent)     gst_object_set_parent(GST_OBJECT_CAST(elem),parent)
GstClock*               gst_element_provide_clock       (GstElement *element);
GstClock*               gst_element_get_clock           (GstElement *element);
gboolean                gst_element_set_clock           (GstElement *element, GstClock *clock);
void                    gst_element_set_base_time       (GstElement *element, GstClockTime time);
GstClockTime            gst_element_get_base_time       (GstElement *element);
void                    gst_element_set_start_time      (GstElement *element, GstClockTime time);
GstClockTime            gst_element_get_start_time      (GstElement *element);
void                    gst_element_set_bus             (GstElement * element, GstBus * bus);
GstBus *                gst_element_get_bus             (GstElement * element);
void                    gst_element_set_context         (GstElement * element, GstContext * context);
GList *                 gst_element_get_contexts        (GstElement * element);
GstContext *            gst_element_get_context         (GstElement * element, const gchar * context_type);
GstContext *            gst_element_get_context_unlocked (GstElement * element, const gchar * context_type);
gboolean                gst_element_add_pad             (GstElement *element, GstPad *pad);
gboolean                gst_element_remove_pad          (GstElement *element, GstPad *pad);
void                    gst_element_no_more_pads        (GstElement *element);
GstPad*                 gst_element_get_static_pad      (GstElement *element, const gchar *name);
GstPad*                 gst_element_get_request_pad     (GstElement *element, const gchar *name);
GstPad*                 gst_element_request_pad         (GstElement *element, GstPadTemplate *templ,
							 const gchar * name, const GstCaps *caps);
void                    gst_element_release_request_pad (GstElement *element, GstPad *pad);
GstIterator *           gst_element_iterate_pads        (GstElement * element);
GstIterator *           gst_element_iterate_src_pads    (GstElement * element);
GstIterator *           gst_element_iterate_sink_pads   (GstElement * element);
gboolean                gst_element_send_event          (GstElement *element, GstEvent *event);
gboolean                gst_element_seek                (GstElement *element, gdouble rate,
                                                         GstFormat format, GstSeekFlags flags,
                                                         GstSeekType start_type, gint64 start,
                                                         GstSeekType stop_type, gint64 stop);
gboolean                gst_element_query               (GstElement *element, GstQuery *query);
gboolean                gst_element_post_message        (GstElement * element, GstMessage * message);
#if (!defined(__GNUC__) || (__GNUC__ < 3) || (__GNUC__ == 3 && __GNUC_MINOR__ < 3))
gchar *                 _gst_element_error_printf       (const gchar *format, ...);
#else
gchar *                 _gst_element_error_printf       (const gchar *format, ...) G_GNUC_PRINTF (1, 2);
#endif
void                    gst_element_message_full        (GstElement * element, GstMessageType type,
                                                         GQuark domain, gint code, gchar * text,
                                                         gchar * debug, const gchar * file,
                                                         const gchar * function, gint line);
gboolean                gst_element_is_locked_state     (GstElement *element);
gboolean                gst_element_set_locked_state    (GstElement *element, gboolean locked_state);
gboolean                gst_element_sync_state_with_parent (GstElement *element);
GstStateChangeReturn    gst_element_get_state           (GstElement * element,
                                                         GstState * state,
                                                         GstState * pending,
                                                         GstClockTime timeout);
GstStateChangeReturn    gst_element_set_state           (GstElement *element, GstState state);
void                    gst_element_abort_state         (GstElement * element);
GstStateChangeReturn    gst_element_change_state        (GstElement * element,
                                                         GstStateChange transition);
GstStateChangeReturn    gst_element_continue_state      (GstElement * element,
                                                         GstStateChangeReturn ret);
void                    gst_element_lost_state          (GstElement * element);
GstElementFactory*      gst_element_get_factory         (GstElement *element);
#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstElement, gst_object_unref)
#endif
G_END_DECLS

#endif