#include <stdarg.h>
#include <glib/glib.h>
#include <gobject/gvaluecollector.h>
#include "gst_private.h"
#include "gstelement.h"
#include "gstelementmetadata.h"
#include "gstenumtypes.h"
#include "gstbus.h"
#include "gsterror.h"
#include "gstevent.h"
#include "gstutils.h"
#include "gstinfo.h"
#include "gstquark.h"
#include "gsttracerutils.h"
#include "gstvalue.h"
#include "gst-i18n-lib.h"
#include "glib-compat-private.h"
#ifndef GST_DISABLE_GST_DEBUG
#include "printf/printf.h"
#endif

enum {
  PAD_ADDED,
  PAD_REMOVED,
  NO_MORE_PADS,
  LAST_SIGNAL
};
enum {
  ARG_0
};
static void gst_element_class_init (GstElementClass * klass);
static void gst_element_init (GstElement * element);
static void gst_element_base_class_init (gpointer g_class);
static void gst_element_base_class_finalize (gpointer g_class);
static void gst_element_constructed (GObject * object);
static void gst_element_dispose (GObject * object);
static void gst_element_finalize (GObject * object);
static GstStateChangeReturn gst_element_change_state_func (GstElement * element, GstStateChange transition);
static GstStateChangeReturn gst_element_get_state_func (GstElement * element, GstState * state, GstState * pending, GstClockTime timeout);
static GstStateChangeReturn gst_element_set_state_func (GstElement * element, GstState state);
static gboolean gst_element_set_clock_func (GstElement * element, GstClock * clock);
static void gst_element_set_bus_func (GstElement * element, GstBus * bus);
static gboolean gst_element_post_message_default (GstElement * element, GstMessage * message);
static void gst_element_set_context_default (GstElement * element, GstContext * context);
static gboolean gst_element_default_send_event (GstElement * element, GstEvent * event);
static gboolean gst_element_default_query (GstElement * element, GstQuery * query);
static GstPadTemplate *gst_element_class_get_request_pad_template (GstElementClass *element_class, const gchar * name);
static GstObjectClass *parent_class = NULL;
static guint gst_element_signals[LAST_SIGNAL] = { 0 };
GQuark __gst_elementclass_factory = 0;
GType gst_element_get_type (void) {
  static volatile gsize gst_element_type = 0;
  if (g_once_init_enter (&gst_element_type)) {
    GType _type;
    static const GTypeInfo element_info = {
      sizeof (GstElementClass),
      gst_element_base_class_init,
      gst_element_base_class_finalize,
      (GClassInitFunc) gst_element_class_init,
      NULL,
      NULL,
      sizeof (GstElement),
      0,
      (GInstanceInitFunc) gst_element_init,
      NULL
    };
    _type = g_type_register_static (GST_TYPE_OBJECT, "GstElement", &element_info, G_TYPE_FLAG_ABSTRACT);
    __gst_elementclass_factory = g_quark_from_static_string ("GST_ELEMENTCLASS_FACTORY");
    g_once_init_leave (&gst_element_type, _type);
  }
  return gst_element_type;
}
static void gst_element_class_init (GstElementClass * klass) {
  GObjectClass *gobject_class;
  gobject_class = (GObjectClass *) klass;
  parent_class = g_type_class_peek_parent (klass);
  gst_element_signals[PAD_ADDED] =
      g_signal_new ("pad-added", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstElementClass, pad_added), NULL, NULL,
      NULL, G_TYPE_NONE, 1, GST_TYPE_PAD);
  gst_element_signals[PAD_REMOVED] =
      g_signal_new ("pad-removed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstElementClass, pad_removed), NULL, NULL,
      NULL, G_TYPE_NONE, 1, GST_TYPE_PAD);
  gst_element_signals[NO_MORE_PADS] =
      g_signal_new ("no-more-pads", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstElementClass, no_more_pads), NULL,
      NULL, NULL, G_TYPE_NONE, 0);
  gobject_class->dispose = gst_element_dispose;
  gobject_class->finalize = gst_element_finalize;
  gobject_class->constructed = gst_element_constructed;
  klass->change_state = GST_DEBUG_FUNCPTR (gst_element_change_state_func);
  klass->set_state = GST_DEBUG_FUNCPTR (gst_element_set_state_func);
  klass->get_state = GST_DEBUG_FUNCPTR (gst_element_get_state_func);
  klass->set_clock = GST_DEBUG_FUNCPTR (gst_element_set_clock_func);
  klass->set_bus = GST_DEBUG_FUNCPTR (gst_element_set_bus_func);
  klass->query = GST_DEBUG_FUNCPTR (gst_element_default_query);
  klass->send_event = GST_DEBUG_FUNCPTR (gst_element_default_send_event);
  klass->numpadtemplates = 0;
  klass->post_message = GST_DEBUG_FUNCPTR (gst_element_post_message_default);
  klass->set_context = GST_DEBUG_FUNCPTR (gst_element_set_context_default);
  klass->elementfactory = NULL;
}
static void gst_element_base_class_init (gpointer g_class) {
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);
  GList *node, *padtemplates;
  element_class->metadata = element_class->metadata ? gst_structure_copy (element_class->metadata) : gst_structure_new_empty ("metadata");
  padtemplates = g_list_copy (element_class->padtemplates);
  for (node = padtemplates; node != NULL; node = node->next) {
    GstPadTemplate *tmpl = (GstPadTemplate *) node->data;
    gst_object_ref (tmpl);
  }
  element_class->padtemplates = padtemplates;
  element_class->elementfactory = g_type_get_qdata (G_TYPE_FROM_CLASS (element_class), __gst_elementclass_factory);
  GST_CAT_DEBUG (GST_CAT_ELEMENT_PADS, "type %s : factory %p", G_OBJECT_CLASS_NAME (element_class), element_class->elementfactory);
}
static void gst_element_base_class_finalize (gpointer g_class) {
  GstElementClass *klass = GST_ELEMENT_CLASS (g_class);
  g_list_foreach (klass->padtemplates, (GFunc) gst_object_unref, NULL);
  g_list_free (klass->padtemplates);
  gst_structure_free (klass->metadata);
}
static void gst_element_init (GstElement * element) {
  GST_STATE (element) = GST_STATE_NULL;
  GST_STATE_TARGET (element) = GST_STATE_NULL;
  GST_STATE_NEXT (element) = GST_STATE_VOID_PENDING;
  GST_STATE_PENDING (element) = GST_STATE_VOID_PENDING;
  GST_STATE_RETURN (element) = GST_STATE_CHANGE_SUCCESS;
  g_static_rec_mutex_init (&element->state_lock);
  g_cond_init (&element->state_cond);
}
static void gst_element_constructed (GObject * object) {
  GST_TRACER_ELEMENT_NEW (GST_ELEMENT_CAST (object));
  G_OBJECT_CLASS (parent_class)->constructed (object);
}
void gst_element_release_request_pad (GstElement * element, GstPad * pad) {
  GstElementClass *oclass;
  g_return_if_fail (GST_IS_ELEMENT (element));
  g_return_if_fail (GST_IS_PAD (pad));
  g_return_if_fail (GST_PAD_PAD_TEMPLATE (pad) == NULL ||
      GST_PAD_TEMPLATE_PRESENCE (GST_PAD_PAD_TEMPLATE (pad)) ==
      GST_PAD_REQUEST);
  oclass = GST_ELEMENT_GET_CLASS (element);
  if (oclass->release_pad) oclass->release_pad (element, pad);
  else gst_element_remove_pad (element, pad);
}
GstClock *gst_element_provide_clock (GstElement * element) {
  GstClock *result = NULL;
  GstElementClass *oclass;
  g_return_val_if_fail (GST_IS_ELEMENT (element), NULL);
  oclass = GST_ELEMENT_GET_CLASS (element);
  if (oclass->provide_clock) result = oclass->provide_clock (element);
  return result;
}
static gboolean gst_element_set_clock_func (GstElement * element, GstClock * clock) {
  GstClock **clock_p;
  GST_OBJECT_LOCK (element);
  clock_p = &element->clock;
  gst_object_replace ((GstObject **) clock_p, (GstObject *) clock);
  GST_OBJECT_UNLOCK (element);
  return TRUE;
}
gboolean gst_element_set_clock (GstElement * element, GstClock * clock) {
  GstElementClass *oclass;
  gboolean res = FALSE;
  g_return_val_if_fail (GST_IS_ELEMENT (element), FALSE);
  g_return_val_if_fail (clock == NULL || GST_IS_CLOCK (clock), FALSE);
  oclass = GST_ELEMENT_GET_CLASS (element);
  GST_CAT_DEBUG_OBJECT (GST_CAT_CLOCK, element, "setting clock %p", clock);
  if (oclass->set_clock) res = oclass->set_clock (element, clock);
  return res;
}
GstClock *gst_element_get_clock (GstElement * element) {
  GstClock *result;
  g_return_val_if_fail (GST_IS_ELEMENT (element), NULL);
  GST_OBJECT_LOCK (element);
  if ((result = element->clock)) gst_object_ref (result);
  GST_OBJECT_UNLOCK (element);
  return result;
}
void gst_element_set_base_time (GstElement * element, GstClockTime time) {
  GstClockTime old;
  g_return_if_fail (GST_IS_ELEMENT (element));
  GST_OBJECT_LOCK (element);
  old = element->base_time;
  element->base_time = time;
  GST_OBJECT_UNLOCK (element);
  GST_CAT_DEBUG_OBJECT (GST_CAT_CLOCK, element, "set base_time=%" GST_TIME_FORMAT ", old %" GST_TIME_FORMAT, GST_TIME_ARGS (time),
                        GST_TIME_ARGS (old));
}
GstClockTime gst_element_get_base_time (GstElement * element) {
  GstClockTime result;
  g_return_val_if_fail (GST_IS_ELEMENT (element), GST_CLOCK_TIME_NONE);
  GST_OBJECT_LOCK (element);
  result = element->base_time;
  GST_OBJECT_UNLOCK (element);
  return result;
}
void gst_element_set_start_time (GstElement * element, GstClockTime time) {
  GstClockTime old;
  g_return_if_fail (GST_IS_ELEMENT (element));
  GST_OBJECT_LOCK (element);
  old = GST_ELEMENT_START_TIME (element);
  GST_ELEMENT_START_TIME (element) = time;
  GST_OBJECT_UNLOCK (element);
  GST_CAT_DEBUG_OBJECT (GST_CAT_CLOCK, element, "set start_time=%" GST_TIME_FORMAT ", old %" GST_TIME_FORMAT, GST_TIME_ARGS (time),
                        GST_TIME_ARGS (old));
}
GstClockTime gst_element_get_start_time (GstElement * element) {
  GstClockTime result;
  g_return_val_if_fail (GST_IS_ELEMENT (element), GST_CLOCK_TIME_NONE);
  GST_OBJECT_LOCK (element);
  result = GST_ELEMENT_START_TIME (element);
  GST_OBJECT_UNLOCK (element);
  return result;
}
#if 0
void gst_element_set_index (GstElement * element, GstIndex * index) {
  GstElementClass *oclass;
  g_return_if_fail (GST_IS_ELEMENT (element));
  g_return_if_fail (index == NULL || GST_IS_INDEX (index));
  oclass = GST_ELEMENT_GET_CLASS (element);
  if (oclass->set_index) oclass->set_index (element, index);
}
GstIndex *gst_element_get_index (GstElement * element) {
  GstElementClass *oclass;
  GstIndex *result = NULL;
  g_return_val_if_fail (GST_IS_ELEMENT (element), NULL);
  oclass = GST_ELEMENT_GET_CLASS (element);
  if (oclass->get_index) result = oclass->get_index (element);
  return result;
}
#endif
gboolean gst_element_add_pad (GstElement * element, GstPad * pad) {
  gchar *pad_name;
  gboolean active;
  g_return_val_if_fail (GST_IS_ELEMENT (element), FALSE);
  g_return_val_if_fail (GST_IS_PAD (pad), FALSE);
  GST_OBJECT_LOCK (pad);
  pad_name = g_strdup (GST_PAD_NAME (pad));
  GST_CAT_INFO_OBJECT (GST_CAT_ELEMENT_PADS, element, "adding pad '%s'", GST_STR_NULL (pad_name));
  active = GST_PAD_IS_ACTIVE (pad);
  GST_OBJECT_FLAG_SET (pad, GST_PAD_FLAG_NEED_PARENT);
  GST_OBJECT_UNLOCK (pad);
  GST_OBJECT_LOCK (element);
  if (G_UNLIKELY (!gst_object_check_uniqueness (element->pads, pad_name))) goto name_exists;
  if (G_UNLIKELY (!gst_object_set_parent (GST_OBJECT_CAST (pad), GST_OBJECT_CAST (element)))) goto had_parent;
  if (!active && (GST_STATE (element) > GST_STATE_READY || GST_STATE_NEXT (element) == GST_STATE_PAUSED)) {
    g_warning ("adding inactive pad '%s' to running element '%s', you need to use gst_pad_set_active(pad,TRUE) before adding it.",
        GST_STR_NULL (pad_name), GST_ELEMENT_NAME (element));
    gst_pad_set_active (pad, TRUE);
  }
  g_free (pad_name);
  switch (gst_pad_get_direction (pad)) {
    case GST_PAD_SRC:
      element->srcpads = g_list_append (element->srcpads, pad);
      element->numsrcpads++;
      break;
    case GST_PAD_SINK:
      element->sinkpads = g_list_append (element->sinkpads, pad);
      element->numsinkpads++;
      break;
    default:
      goto no_direction;
  }
  element->pads = g_list_append (element->pads, pad);
  element->numpads++;
  element->pads_cookie++;
  GST_OBJECT_UNLOCK (element);
  g_signal_emit (element, gst_element_signals[PAD_ADDED], 0, pad);
  GST_TRACER_ELEMENT_ADD_PAD (element, pad);
  return TRUE;
name_exists:
  {
    g_critical ("Padname %s is not unique in element %s, not adding", pad_name, GST_ELEMENT_NAME (element));
    GST_OBJECT_UNLOCK (element);
    g_free (pad_name);
    return FALSE;
  }
had_parent:
  {
    g_critical("Pad %s already has parent when trying to add to element %s", pad_name, GST_ELEMENT_NAME (element));
    GST_OBJECT_UNLOCK (element);
    g_free (pad_name);
    return FALSE;
  }
no_direction:
  {
    GST_OBJECT_LOCK (pad);
    g_critical("Trying to add pad %s to element %s, but it has no direction", GST_OBJECT_NAME (pad), GST_ELEMENT_NAME (element));
    GST_OBJECT_UNLOCK (pad);
    GST_OBJECT_UNLOCK (element);
    return FALSE;
  }
}
gboolean gst_element_remove_pad (GstElement * element, GstPad * pad) {
  GstPad *peer;
  g_return_val_if_fail (GST_IS_ELEMENT (element), FALSE);
  g_return_val_if_fail (GST_IS_PAD (pad), FALSE);
  GST_OBJECT_LOCK (pad);
  GST_CAT_INFO_OBJECT (GST_CAT_ELEMENT_PADS, element, "removing pad '%s'", GST_STR_NULL (GST_PAD_NAME (pad)));
  if (G_UNLIKELY (GST_PAD_PARENT (pad) != element)) goto not_our_pad;
  GST_OBJECT_UNLOCK (pad);
  if ((peer = gst_pad_get_peer (pad))) {
    if (GST_PAD_IS_SRC (pad)) gst_pad_unlink (pad, peer);
    else gst_pad_unlink (peer, pad);
    gst_object_unref (peer);
  }
  GST_OBJECT_LOCK (element);
  switch (gst_pad_get_direction (pad)) {
    case GST_PAD_SRC:
      element->srcpads = g_list_remove (element->srcpads, pad);
      element->numsrcpads--;
      break;
    case GST_PAD_SINK:
      element->sinkpads = g_list_remove (element->sinkpads, pad);
      element->numsinkpads--;
      break;
    default:
      g_critical ("Removing pad without direction???");
      break;
  }
  element->pads = g_list_remove (element->pads, pad);
  element->numpads--;
  element->pads_cookie++;
  GST_OBJECT_UNLOCK (element);
  g_signal_emit (element, gst_element_signals[PAD_REMOVED], 0, pad);
  GST_TRACER_ELEMENT_REMOVE_PAD (element, pad);
  gst_object_unparent (GST_OBJECT_CAST (pad));
  return TRUE;
not_our_pad:
  {
    GST_OBJECT_UNLOCK (pad);
    GST_OBJECT_LOCK (element);
    GST_OBJECT_LOCK (pad);
    g_critical ("Padname %s:%s does not belong to element %s when removing", GST_DEBUG_PAD_NAME (pad), GST_ELEMENT_NAME (element));
    GST_OBJECT_UNLOCK (pad);
    GST_OBJECT_UNLOCK (element);
    return FALSE;
  }
}
void gst_element_no_more_pads (GstElement * element) {
  g_return_if_fail (GST_IS_ELEMENT (element));
  g_signal_emit (element, gst_element_signals[NO_MORE_PADS], 0);
}
static gint pad_compare_name (GstPad * pad1, const gchar * name) {
  gint result;
  GST_OBJECT_LOCK (pad1);
  result = strcmp (GST_PAD_NAME (pad1), name);
  GST_OBJECT_UNLOCK (pad1);
  return result;
}
GstPad *gst_element_get_static_pad (GstElement * element, const gchar * name) {
  GList *find;
  GstPad *result = NULL;
  g_return_val_if_fail (GST_IS_ELEMENT (element), NULL);
  g_return_val_if_fail (name != NULL, NULL);
  GST_OBJECT_LOCK (element);
  find = g_list_find_custom (element->pads, name, (GCompareFunc) pad_compare_name);
  if (find) {
    result = GST_PAD_CAST (find->data);
    gst_object_ref (result);
  }
  if (result == NULL) {
    GST_CAT_INFO (GST_CAT_ELEMENT_PADS, "no such pad '%s' in element \"%s\"", name, GST_ELEMENT_NAME (element));
  } else { GST_CAT_INFO (GST_CAT_ELEMENT_PADS, "found pad %s:%s", GST_ELEMENT_NAME (element), name); }
  GST_OBJECT_UNLOCK (element);
  return result;
}
static GstPad *_gst_element_request_pad (GstElement * element, GstPadTemplate * templ, const gchar * name, const GstCaps * caps) {
  GstPad *newpad = NULL;
  GstElementClass *oclass;
  oclass = GST_ELEMENT_GET_CLASS (element);
#ifndef G_DISABLE_CHECKS
  if (name) {
    GstPad *pad;
    if (strstr (name, "%") || !strchr (templ->name_template, '%')) {
      g_return_val_if_fail (strcmp (name, templ->name_template) == 0, NULL);
    } else {
      const gchar *str, *data;
      gchar *endptr;
      str = strchr (templ->name_template, '%');
      g_return_val_if_fail (str != NULL, NULL);
      g_return_val_if_fail (strncmp (templ->name_template, name, str - templ->name_template) == 0, NULL);
      g_return_val_if_fail (strlen (name) > str - templ->name_template, NULL);
      data = name + (str - templ->name_template);
      if (*(str + 1) == 'd') {
        gint64 tmp;
        tmp = g_ascii_strtoll (data, &endptr, 10);
        g_return_val_if_fail (tmp >= G_MININT && tmp <= G_MAXINT && *endptr == '\0', NULL);
      } else if (*(str + 1) == 'u') {
        guint64 tmp;
        tmp = g_ascii_strtoull (data, &endptr, 10);
        g_return_val_if_fail (tmp <= G_MAXUINT && *endptr == '\0', NULL);
      }
    }
    pad = gst_element_get_static_pad (element, name);
    if (pad) {
      gst_object_unref (pad);
      g_critical ("Element %s already has a pad named %s, the behaviour of  gst_element_get_request_pad() for existing pads is "
                  "undefined!", GST_ELEMENT_NAME (element), name);
    }
  }
#endif
  if (oclass->request_new_pad) newpad = (oclass->request_new_pad) (element, templ, name, caps);
  if (newpad) gst_object_ref (newpad);
  return newpad;
}
GstPad *gst_element_get_request_pad (GstElement * element, const gchar * name) {
  GstPadTemplate *templ = NULL;
  GstPad *pad;
  const gchar *req_name = NULL;
  gboolean templ_found = FALSE;
  GList *list;
  const gchar *data;
  gchar *str, *endptr = NULL;
  GstElementClass *class;
  g_return_val_if_fail (GST_IS_ELEMENT (element), NULL);
  g_return_val_if_fail (name != NULL, NULL);
  class = GST_ELEMENT_GET_CLASS (element);
  if (strstr (name, "%")) {
    templ = gst_element_class_get_request_pad_template (class, name);
    req_name = NULL;
    if (templ) templ_found = TRUE;
  } else {
    list = class->padtemplates;
    while (!templ_found && list) {
      templ = (GstPadTemplate *) list->data;
      if (templ->presence == GST_PAD_REQUEST) {
        GST_CAT_DEBUG (GST_CAT_PADS, "comparing %s to %s", name, templ->name_template);
        if (strcmp (name, templ->name_template) == 0) {
          templ_found = TRUE;
          req_name = name;
          break;
        }
        else if ((str = strchr (templ->name_template, '%')) && strncmp (templ->name_template, name,
                 str - templ->name_template) == 0 && strlen (name) > str - templ->name_template) {
          data = name + (str - templ->name_template);
          if (*(str + 1) == 'd') {
            glong tmp;
            tmp = strtol (data, &endptr, 10);
            if (tmp != G_MINLONG && tmp != G_MAXLONG && endptr && *endptr == '\0') {
              templ_found = TRUE;
              req_name = name;
              break;
            }
          } else if (*(str + 1) == 'u') {
            gulong tmp;
            tmp = strtoul (data, &endptr, 10);
            if (tmp != G_MAXULONG && endptr && *endptr == '\0') {
              templ_found = TRUE;
              req_name = name;
              break;
            }
          } else {
            templ_found = TRUE;
            req_name = name;
            break;
          }
        }
      }
      list = list->next;
    }
  }
  if (!templ_found) return NULL;
  pad = _gst_element_request_pad (element, templ, req_name, NULL);
  return pad;
}
GstPad *gst_element_request_pad (GstElement * element, GstPadTemplate * templ, const gchar * name, const GstCaps * caps) {
  g_return_val_if_fail (GST_IS_ELEMENT (element), NULL);
  g_return_val_if_fail (templ != NULL, NULL);
  g_return_val_if_fail (templ->presence == GST_PAD_REQUEST, NULL);
  return _gst_element_request_pad (element, templ, name, caps);
}
static GstIterator *gst_element_iterate_pad_list (GstElement * element, GList ** padlist) {
  GstIterator *result;
  GST_OBJECT_LOCK (element);
  result = gst_iterator_new_list (GST_TYPE_PAD, GST_OBJECT_GET_LOCK (element), &element->pads_cookie, padlist, (GObject *) element, NULL);
  GST_OBJECT_UNLOCK (element);
  return result;
}
GstIterator *gst_element_iterate_pads (GstElement * element) {
  g_return_val_if_fail (GST_IS_ELEMENT (element), NULL);
  return gst_element_iterate_pad_list (element, &element->pads);
}
GstIterator *gst_element_iterate_src_pads (GstElement * element) {
  g_return_val_if_fail (GST_IS_ELEMENT (element), NULL);
  return gst_element_iterate_pad_list (element, &element->srcpads);
}
GstIterator *gst_element_iterate_sink_pads (GstElement * element) {
  g_return_val_if_fail (GST_IS_ELEMENT (element), NULL);
  return gst_element_iterate_pad_list (element, &element->sinkpads);
}
void gst_element_class_add_pad_template (GstElementClass * klass, GstPadTemplate * templ) {
  GList *template_list = klass->padtemplates;
  g_return_if_fail (GST_IS_ELEMENT_CLASS (klass));
  g_return_if_fail (GST_IS_PAD_TEMPLATE (templ));
  while (template_list) {
    GstPadTemplate *padtempl = (GstPadTemplate *) template_list->data;
    if (strcmp (templ->name_template, padtempl->name_template) == 0) {
      gst_object_unref (padtempl);
      template_list->data = templ;
      return;
    }
    template_list = g_list_next (template_list);
  }
  gst_object_ref_sink (templ);
  klass->padtemplates = g_list_append (klass->padtemplates, templ);
  klass->numpadtemplates++;
}
void gst_element_class_add_static_pad_template (GstElementClass * klass, GstStaticPadTemplate * static_templ) {
  gst_element_class_add_pad_template (klass, gst_static_pad_template_get (static_templ));
}
void gst_element_class_add_metadata (GstElementClass * klass, const gchar * key, const gchar * value) {
  g_return_if_fail (GST_IS_ELEMENT_CLASS (klass));
  g_return_if_fail (key != NULL);
  g_return_if_fail (value != NULL);
  gst_structure_set ((GstStructure *) klass->metadata, key, G_TYPE_STRING, value, NULL);
}
void gst_element_class_add_static_metadata (GstElementClass * klass, const gchar * key, const gchar * value) {
  GValue val = G_VALUE_INIT;
  g_return_if_fail (GST_IS_ELEMENT_CLASS (klass));
  g_return_if_fail (key != NULL);
  g_return_if_fail (value != NULL);
  g_value_init (&val, G_TYPE_STRING);
  g_value_set_static_string (&val, value);
  gst_structure_take_value ((GstStructure *) klass->metadata, key, &val);
}
void gst_element_class_set_metadata (GstElementClass * klass, const gchar * longname, const gchar * classification, const gchar * description,
                                     const gchar * author) {
  g_return_if_fail (GST_IS_ELEMENT_CLASS (klass));
  g_return_if_fail (longname != NULL && *longname != '\0');
  g_return_if_fail (classification != NULL && *classification != '\0');
  g_return_if_fail (description != NULL && *description != '\0');
  g_return_if_fail (author != NULL && *author != '\0');
  gst_structure_id_set((GstStructure *) klass->metadata, GST_QUARK (ELEMENT_METADATA_LONGNAME), G_TYPE_STRING, longname,
                       GST_QUARK (ELEMENT_METADATA_KLASS), G_TYPE_STRING, classification, GST_QUARK (ELEMENT_METADATA_DESCRIPTION),
                       G_TYPE_STRING, description, GST_QUARK (ELEMENT_METADATA_AUTHOR), G_TYPE_STRING, author, NULL);
}
void gst_element_class_set_static_metadata(GstElementClass *klass, const gchar *longname, const gchar *classification, const gchar *description,
                                           const gchar * author) {
  GstStructure *s = (GstStructure *) klass->metadata;
  GValue val = G_VALUE_INIT;
  g_return_if_fail (GST_IS_ELEMENT_CLASS (klass));
  g_return_if_fail (longname != NULL && *longname != '\0');
  g_return_if_fail (classification != NULL && *classification != '\0');
  g_return_if_fail (description != NULL && *description != '\0');
  g_return_if_fail (author != NULL && *author != '\0');
  g_value_init (&val, G_TYPE_STRING);
  g_value_set_static_string (&val, longname);
  gst_structure_id_set_value (s, GST_QUARK (ELEMENT_METADATA_LONGNAME), &val);
  g_value_set_static_string (&val, classification);
  gst_structure_id_set_value (s, GST_QUARK (ELEMENT_METADATA_KLASS), &val);
  g_value_set_static_string (&val, description);
  gst_structure_id_set_value (s, GST_QUARK (ELEMENT_METADATA_DESCRIPTION), &val);
  g_value_set_static_string (&val, author);
  gst_structure_id_take_value (s, GST_QUARK (ELEMENT_METADATA_AUTHOR), &val);
}
const gchar *gst_element_class_get_metadata (GstElementClass * klass, const gchar * key) {
  g_return_val_if_fail (GST_IS_ELEMENT_CLASS (klass), NULL);
  g_return_val_if_fail (key != NULL, NULL);
  return gst_structure_get_string ((GstStructure *) klass->metadata, key);
}
GList *gst_element_class_get_pad_template_list (GstElementClass * element_class) {
  g_return_val_if_fail (GST_IS_ELEMENT_CLASS (element_class), NULL);
  return element_class->padtemplates;
}
GstPadTemplate *gst_element_class_get_pad_template (GstElementClass *element_class, const gchar * name) {
  GList *padlist;
  g_return_val_if_fail (GST_IS_ELEMENT_CLASS (element_class), NULL);
  g_return_val_if_fail (name != NULL, NULL);
  padlist = element_class->padtemplates;
  while (padlist) {
    GstPadTemplate *padtempl = (GstPadTemplate *) padlist->data;
    if (strcmp (padtempl->name_template, name) == 0) return padtempl;
    padlist = g_list_next (padlist);
  }
  return NULL;
}
static GstPadTemplate *gst_element_class_get_request_pad_template (GstElementClass *element_class, const gchar * name) {
  GstPadTemplate *tmpl;
  tmpl = gst_element_class_get_pad_template (element_class, name);
  if (tmpl != NULL && tmpl->presence == GST_PAD_REQUEST) return tmpl;
  return NULL;
}
static GstPad *gst_element_get_random_pad (GstElement * element, gboolean need_linked, GstPadDirection dir) {
  GstPad *result = NULL;
  GList *pads;
  GST_CAT_DEBUG (GST_CAT_ELEMENT_PADS, "getting a random pad");
  switch (dir) {
    case GST_PAD_SRC:
      GST_OBJECT_LOCK (element);
      pads = element->srcpads;
      break;
    case GST_PAD_SINK:
      GST_OBJECT_LOCK (element);
      pads = element->sinkpads;
      break;
    default:
      goto wrong_direction;
  }
  for (; pads; pads = g_list_next (pads)) {
    GstPad *pad = GST_PAD_CAST (pads->data);
    GST_OBJECT_LOCK (pad);
    GST_CAT_DEBUG (GST_CAT_ELEMENT_PADS, "checking pad %s:%s", GST_DEBUG_PAD_NAME (pad));
    if (need_linked && !GST_PAD_IS_LINKED (pad)) {
      GST_CAT_DEBUG (GST_CAT_ELEMENT_PADS, "pad %s:%s is not linked", GST_DEBUG_PAD_NAME (pad));
      GST_OBJECT_UNLOCK (pad);
      continue;
    } else {
      GST_CAT_DEBUG (GST_CAT_ELEMENT_PADS, "found pad %s:%s", GST_DEBUG_PAD_NAME (pad));
      GST_OBJECT_UNLOCK (pad);
      result = pad;
      break;
    }
  }
  if (result) gst_object_ref (result);
  GST_OBJECT_UNLOCK (element);
  return result;
wrong_direction:
  {
    g_warning ("unknown pad direction %d", dir);
    return NULL;
  }
}
static gboolean gst_element_default_send_event (GstElement * element, GstEvent * event) {
  gboolean result = FALSE;
  GstPad *pad;
  pad = GST_EVENT_IS_DOWNSTREAM (event) ? gst_element_get_random_pad (element, TRUE, GST_PAD_SINK) :
        gst_element_get_random_pad (element, TRUE, GST_PAD_SRC);
  if (pad) {
    GST_CAT_DEBUG (GST_CAT_ELEMENT_PADS, "pushing %s event to random %s pad %s:%s", GST_EVENT_TYPE_NAME (event),
                  (GST_PAD_DIRECTION (pad) == GST_PAD_SRC ? "src" : "sink"), GST_DEBUG_PAD_NAME (pad));
    result = gst_pad_send_event (pad, event);
    gst_object_unref (pad);
  } else {
    GST_CAT_INFO (GST_CAT_ELEMENT_PADS, "can't send %s event on element %s", GST_EVENT_TYPE_NAME (event), GST_ELEMENT_NAME (element));
    gst_event_unref (event);
  }
  return result;
}
gboolean gst_element_send_event (GstElement * element, GstEvent * event) {
  GstElementClass *oclass;
  gboolean result = FALSE;
  g_return_val_if_fail (GST_IS_ELEMENT (element), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
  oclass = GST_ELEMENT_GET_CLASS (element);
  g_static_rec_mutex_lock (element);
  if (oclass->send_event) {
    GST_CAT_DEBUG (GST_CAT_ELEMENT_PADS, "send %s event on element %s", GST_EVENT_TYPE_NAME (event), GST_ELEMENT_NAME (element));
    result = oclass->send_event (element, event);
  } else gst_event_unref (event);
  g_static_rec_mutex_unlock (element);
  return result;
}
gboolean gst_element_seek (GstElement * element, gdouble rate, GstFormat format, GstSeekFlags flags, GstSeekType start_type, gint64 start,
                          GstSeekType stop_type, gint64 stop) {
  GstEvent *event;
  gboolean result;
  g_return_val_if_fail (GST_IS_ELEMENT (element), FALSE);
  event = gst_event_new_seek (rate, format, flags, start_type, start, stop_type, stop);
  result = gst_element_send_event (element, event);
  return result;
}
static gboolean gst_element_default_query (GstElement * element, GstQuery * query) {
  gboolean result = FALSE;
  GstPad *pad;
  pad = gst_element_get_random_pad (element, FALSE, GST_PAD_SRC);
  if (pad) {
    result = gst_pad_query (pad, query);
    gst_object_unref (pad);
  } else {
    pad = gst_element_get_random_pad (element, TRUE, GST_PAD_SINK);
    if (pad) {
      GstPad *peer = gst_pad_get_peer (pad);
      if (peer) {
        result = gst_pad_query (peer, query);
        gst_object_unref (peer);
      }
      gst_object_unref (pad);
    }
  }
  return result;
}
gboolean gst_element_query (GstElement * element, GstQuery * query) {
  GstElementClass *klass;
  gboolean res = FALSE;
  g_return_val_if_fail (GST_IS_ELEMENT (element), FALSE);
  g_return_val_if_fail (query != NULL, FALSE);
  GST_TRACER_ELEMENT_QUERY_PRE (element, query);
  klass = GST_ELEMENT_GET_CLASS (element);
  if (klass->query) {
    GST_CAT_DEBUG (GST_CAT_ELEMENT_PADS, "send query on element %s", GST_ELEMENT_NAME (element));
    res = klass->query (element, query);
  }
  GST_TRACER_ELEMENT_QUERY_POST (element, query, res);
  return res;
}
static gboolean gst_element_post_message_default (GstElement * element, GstMessage * message) {
  GstBus *bus;
  gboolean result = FALSE;
  g_return_val_if_fail (GST_IS_ELEMENT (element), FALSE);
  g_return_val_if_fail (message != NULL, FALSE);
  GST_OBJECT_LOCK (element);
  bus = element->bus;
  if (G_UNLIKELY (bus == NULL)) goto no_bus;
  gst_object_ref (bus);
  GST_OBJECT_UNLOCK (element);
  result = gst_bus_post (bus, message);
  gst_object_unref (bus);
  return result;
no_bus:
  {
    GST_CAT_DEBUG_OBJECT (GST_CAT_MESSAGE, element, "not posting message %p: no bus", message);
    GST_OBJECT_UNLOCK (element);
    gst_message_unref (message);
    return FALSE;
  }
}
gboolean gst_element_post_message (GstElement * element, GstMessage * message) {
  GstElementClass *klass;
  gboolean res = FALSE;
  g_return_val_if_fail (GST_IS_ELEMENT (element), FALSE);
  g_return_val_if_fail (message != NULL, FALSE);
  GST_TRACER_ELEMENT_POST_MESSAGE_PRE (element, message);
  klass = GST_ELEMENT_GET_CLASS (element);
  if (klass->post_message) res = klass->post_message (element, message);
  else gst_message_unref (message);
  GST_TRACER_ELEMENT_POST_MESSAGE_POST (element, res);
  return res;
}
gchar *_gst_element_error_printf (const gchar * format, ...) {
  va_list args;
  gchar *buffer;
  int len;
  if (format == NULL) return NULL;
  if (format[0] == 0) return NULL;
  va_start (args, format);
  len = __gst_vasprintf (&buffer, format, args);
  va_end (args);
  if (len < 0) buffer = NULL;
  return buffer;
}
void gst_element_message_full(GstElement *element, GstMessageType type, GQuark domain, gint code, gchar *text, gchar *debug, const gchar *file,
                              const gchar * function, gint line) {
  GError *gerror = NULL;
  gchar *name;
  gchar *sent_text;
  gchar *sent_debug;
  gboolean has_debug = TRUE;
  GstMessage *message = NULL;
  GST_CAT_DEBUG_OBJECT (GST_CAT_MESSAGE, element, "start");
  g_return_if_fail (GST_IS_ELEMENT (element));
  g_return_if_fail ((type == GST_MESSAGE_ERROR) || (type == GST_MESSAGE_WARNING) || (type == GST_MESSAGE_INFO));
  if ((text == NULL) || (text[0] == 0)) {
    g_free (text);
    sent_text = gst_error_get_message (domain, code);
  } else sent_text = text;
  if ((debug == NULL) || (debug[0] == 0)) has_debug = FALSE;
  name = gst_object_get_path_string (GST_OBJECT_CAST (element));
  if (has_debug) sent_debug = g_strdup_printf ("%s(%d): %s (): %s:\n%s", file, line, function, name, debug);
  else sent_debug = g_strdup_printf ("%s(%d): %s (): %s", file, line, function, name);
  g_free (name);
  g_free (debug);
  GST_CAT_INFO_OBJECT (GST_CAT_ERROR_SYSTEM, element, "posting message: %s", sent_text);
  gerror = g_error_new_literal (domain, code, sent_text);
  switch (type) {
    case GST_MESSAGE_ERROR: message = gst_message_new_error (GST_OBJECT_CAST (element), gerror, sent_debug); break;
    case GST_MESSAGE_WARNING: message = gst_message_new_warning (GST_OBJECT_CAST (element), gerror, sent_debug); break;
    case GST_MESSAGE_INFO: message = gst_message_new_info (GST_OBJECT_CAST (element), gerror, sent_debug); break;
    default: g_assert_not_reached (); break;
  }
  gst_element_post_message (element, message);
  GST_CAT_INFO_OBJECT (GST_CAT_ERROR_SYSTEM, element, "posted %s message: %s", (type == GST_MESSAGE_ERROR ? "error" : "warning"), sent_text);
  g_error_free (gerror);
  g_free (sent_debug);
  g_free (sent_text);
}
gboolean gst_element_is_locked_state (GstElement * element) {
  gboolean result;
  g_return_val_if_fail (GST_IS_ELEMENT (element), FALSE);
  GST_OBJECT_LOCK (element);
  result = GST_ELEMENT_IS_LOCKED_STATE (element);
  GST_OBJECT_UNLOCK (element);
  return result;
}
gboolean gst_element_set_locked_state (GstElement * element, gboolean locked_state) {
  gboolean old;
  g_return_val_if_fail (GST_IS_ELEMENT (element), FALSE);
  GST_OBJECT_LOCK (element);
  old = GST_ELEMENT_IS_LOCKED_STATE (element);
  if (G_UNLIKELY (old == locked_state)) goto was_ok;
  if (locked_state) {
    GST_CAT_DEBUG (GST_CAT_STATES, "locking state of element %s", GST_ELEMENT_NAME (element));
    GST_OBJECT_FLAG_SET (element, GST_ELEMENT_FLAG_LOCKED_STATE);
  } else {
    GST_CAT_DEBUG (GST_CAT_STATES, "unlocking state of element %s", GST_ELEMENT_NAME (element));
    GST_OBJECT_FLAG_UNSET (element, GST_ELEMENT_FLAG_LOCKED_STATE);
  }
  GST_OBJECT_UNLOCK (element);
  return TRUE;
was_ok:
  {
    GST_CAT_DEBUG (GST_CAT_STATES, "elements %s was already in locked state %d", GST_ELEMENT_NAME (element), old);
    GST_OBJECT_UNLOCK (element);
    return FALSE;
  }
}
gboolean gst_element_sync_state_with_parent (GstElement * element) {
  GstElement *parent;
  GstState target;
  GstStateChangeReturn ret;
  g_return_val_if_fail (GST_IS_ELEMENT (element), FALSE);
  if ((parent = GST_ELEMENT_CAST (gst_element_get_parent (element)))) {
    GstState parent_current, parent_pending;
    GST_OBJECT_LOCK (parent);
    parent_current = GST_STATE (parent);
    parent_pending = GST_STATE_PENDING (parent);
    GST_OBJECT_UNLOCK (parent);
    if (parent_pending != GST_STATE_VOID_PENDING) target = parent_pending;
    else target = parent_current;
    GST_CAT_DEBUG_OBJECT(GST_CAT_STATES, element, "syncing state (%s) to parent %s %s (%s, %s)", gst_element_state_get_name(GST_STATE(element)),
                         GST_ELEMENT_NAME (parent), gst_element_state_get_name (target), gst_element_state_get_name (parent_current),
                         gst_element_state_get_name (parent_pending));
    ret = gst_element_set_state (element, target);
    if (ret == GST_STATE_CHANGE_FAILURE) goto failed;
    gst_object_unref (parent);
    return TRUE;
  } else { GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "element has no parent"); }
  return FALSE;
failed:
  {
    GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "syncing state failed (%s)", gst_element_state_change_return_get_name (ret));
    gst_object_unref (parent);
    return FALSE;
  }
}
static GstStateChangeReturn gst_element_get_state_func (GstElement * element, GstState * state, GstState * pending, GstClockTime timeout) {
  GstStateChangeReturn ret = GST_STATE_CHANGE_FAILURE;
  GstState old_pending;
  GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "getting state, timeout %" GST_TIME_FORMAT, GST_TIME_ARGS (timeout));
  GST_OBJECT_LOCK (element);
  ret = GST_STATE_RETURN (element);
  GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "RETURN is %s", gst_element_state_change_return_get_name (ret));
  if (ret == GST_STATE_CHANGE_FAILURE) goto done;
  if (ret == GST_STATE_CHANGE_NO_PREROLL) goto done;
  if (ret != GST_STATE_CHANGE_ASYNC) goto done;
  old_pending = GST_STATE_PENDING (element);
  if (old_pending != GST_STATE_VOID_PENDING) {
    gboolean signaled;
    guint32 cookie = TRUE;
    cookie = element->state_cookie;
    GST_CAT_INFO_OBJECT (GST_CAT_STATES, element, "waiting for element to commit state");
    if (timeout != GST_CLOCK_TIME_NONE) {
      gint64 end_time;
      end_time = g_get_monotonic_time () + (timeout / 1000);
      //signaled = GST_STATE_WAIT_UNTIL (element, end_time);
    } else {
      GST_STATE_WAIT (element);
      signaled = TRUE;
    }
    if (!signaled) {
      GST_CAT_INFO_OBJECT (GST_CAT_STATES, element, "timed out");
      ret = GST_STATE_CHANGE_ASYNC;
    } else {
      if (cookie != element->state_cookie) goto interrupted;
      if (old_pending == GST_STATE (element)) {
        GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "got success");
        ret = GST_STATE_CHANGE_SUCCESS;
      } else {
        GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "got failure");
        ret = GST_STATE_CHANGE_FAILURE;
      }
    }
    if (GST_STATE_PENDING (element) == GST_STATE_VOID_PENDING) {
      GST_CAT_LOG_OBJECT (GST_CAT_STATES, element, "nothing pending");
      ret = GST_STATE_CHANGE_SUCCESS;
    }
  }
done:
  if (state) *state = GST_STATE (element);
  if (pending) *pending = GST_STATE_PENDING (element);
  GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "state current: %s, pending: %s, result: %s", gst_element_state_get_name (GST_STATE (element)),
                        gst_element_state_get_name (GST_STATE_PENDING (element)), gst_element_state_change_return_get_name (ret));
  GST_OBJECT_UNLOCK (element);
  return ret;
interrupted:
  {
    if (state) *state = GST_STATE_VOID_PENDING;
    if (pending) *pending = GST_STATE_VOID_PENDING;
    GST_CAT_INFO_OBJECT (GST_CAT_STATES, element, "interruped");
    GST_OBJECT_UNLOCK (element);
    return GST_STATE_CHANGE_FAILURE;
  }
}
GstStateChangeReturn gst_element_get_state (GstElement * element, GstState * state, GstState * pending, GstClockTime timeout) {
  GstElementClass *oclass;
  GstStateChangeReturn result = GST_STATE_CHANGE_FAILURE;
  g_return_val_if_fail (GST_IS_ELEMENT (element), GST_STATE_CHANGE_FAILURE);
  oclass = GST_ELEMENT_GET_CLASS (element);
  if (oclass->get_state) result = (oclass->get_state) (element, state, pending, timeout);
  return result;
}
void gst_element_abort_state (GstElement * element) {
  GstState pending;
#ifndef GST_DISABLE_GST_DEBUG
  GstState old_state;
#endif
  g_return_if_fail (GST_IS_ELEMENT (element));
  GST_OBJECT_LOCK (element);
  pending = GST_STATE_PENDING (element);
  if (pending == GST_STATE_VOID_PENDING || GST_STATE_RETURN (element) == GST_STATE_CHANGE_FAILURE) goto nothing_aborted;
#ifndef GST_DISABLE_GST_DEBUG
  old_state = GST_STATE (element);
  GST_CAT_INFO_OBJECT (GST_CAT_STATES, element,
      "aborting state from %s to %s", gst_element_state_get_name (old_state),
      gst_element_state_get_name (pending));
#endif
  GST_STATE_RETURN (element) = GST_STATE_CHANGE_FAILURE;
  GST_STATE_BROADCAST (element);
  GST_OBJECT_UNLOCK (element);
  return;
nothing_aborted:
  {
    GST_OBJECT_UNLOCK (element);
    return;
  }
}
void _priv_gst_element_state_changed (GstElement * element, GstState oldstate, GstState newstate, GstState pending) {
  GstElementClass *klass = GST_ELEMENT_GET_CLASS (element);
  GstMessage *message;
  GST_CAT_INFO_OBJECT (GST_CAT_STATES, element, "notifying about state-changed %s to %s (%s pending)", gst_element_state_get_name (oldstate),
                      gst_element_state_get_name (newstate), gst_element_state_get_name (pending));
  if (klass->state_changed) klass->state_changed (element, oldstate, newstate, pending);
  message = gst_message_new_state_changed (GST_OBJECT_CAST (element), oldstate, newstate, pending);
  gst_element_post_message (element, message);
}
GstStateChangeReturn gst_element_continue_state (GstElement * element, GstStateChangeReturn ret) {
  GstStateChangeReturn old_ret;
  GstState old_state, old_next;
  GstState current, next, pending;
  GstStateChange transition;
  GST_OBJECT_LOCK (element);
  old_ret = GST_STATE_RETURN (element);
  GST_STATE_RETURN (element) = ret;
  pending = GST_STATE_PENDING (element);
  if (pending == GST_STATE_VOID_PENDING) goto nothing_pending;
  old_state = GST_STATE (element);
  old_next = GST_STATE_NEXT (element);
  current = GST_STATE (element) = old_next;
  if (pending == current) goto complete;
  next = GST_STATE_GET_NEXT (current, pending);
  transition = (GstStateChange) GST_STATE_TRANSITION (current, next);
  GST_STATE_NEXT (element) = next;
  GST_STATE_RETURN (element) = GST_STATE_CHANGE_ASYNC;
  GST_OBJECT_UNLOCK (element);
  GST_CAT_INFO_OBJECT (GST_CAT_STATES, element, "committing state from %s to %s, pending %s, next %s", gst_element_state_get_name (old_state),
                      gst_element_state_get_name (old_next), gst_element_state_get_name (pending), gst_element_state_get_name (next));
  _priv_gst_element_state_changed (element, old_state, old_next, pending);
  GST_CAT_INFO_OBJECT (GST_CAT_STATES, element, "continue state change %s to %s, final %s", gst_element_state_get_name (current),
                      gst_element_state_get_name (next), gst_element_state_get_name (pending));
  ret = gst_element_change_state (element, transition);
  return ret;
nothing_pending:
  {
    GST_CAT_INFO_OBJECT (GST_CAT_STATES, element, "nothing pending");
    GST_OBJECT_UNLOCK (element);
    return ret;
  }
complete:
  {
    GST_STATE_PENDING (element) = GST_STATE_VOID_PENDING;
    GST_STATE_NEXT (element) = GST_STATE_VOID_PENDING;
    GST_CAT_INFO_OBJECT (GST_CAT_STATES, element, "completed state change to %s", gst_element_state_get_name (pending));
    GST_OBJECT_UNLOCK (element);
    if (old_state != old_next || old_ret == GST_STATE_CHANGE_ASYNC)
      _priv_gst_element_state_changed (element, old_state, old_next, GST_STATE_VOID_PENDING);
    GST_STATE_BROADCAST (element);
    return ret;
  }
}
void gst_element_lost_state (GstElement * element) {
  GstState old_state, new_state;
  GstMessage *message;
  g_return_if_fail (GST_IS_ELEMENT (element));
  GST_OBJECT_LOCK (element);
  if (GST_STATE_RETURN (element) == GST_STATE_CHANGE_FAILURE) goto nothing_lost;
  if (GST_STATE_PENDING (element) != GST_STATE_VOID_PENDING) goto only_async_start;
  old_state = GST_STATE (element);
  if (old_state > GST_STATE_PAUSED) new_state = GST_STATE_PAUSED;
  else new_state = old_state;
  GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "lost state of %s to %s", gst_element_state_get_name (old_state),
                       gst_element_state_get_name (new_state));
  GST_STATE (element) = new_state;
  GST_STATE_NEXT (element) = new_state;
  GST_STATE_PENDING (element) = new_state;
  GST_STATE_RETURN (element) = GST_STATE_CHANGE_ASYNC;
  GST_OBJECT_UNLOCK (element);
  _priv_gst_element_state_changed (element, new_state, new_state, new_state);
  message = gst_message_new_async_start (GST_OBJECT_CAST (element));
  gst_element_post_message (element, message);
  return;
nothing_lost:
  {
    GST_OBJECT_UNLOCK (element);
    return;
  }
only_async_start:
  {
    GST_OBJECT_UNLOCK (element);
    message = gst_message_new_async_start (GST_OBJECT_CAST (element));
    gst_element_post_message (element, message);
    return;
  }
}
GstStateChangeReturn gst_element_set_state (GstElement * element, GstState state) {
  GstElementClass *oclass;
  GstStateChangeReturn result = GST_STATE_CHANGE_FAILURE;
  g_return_val_if_fail (GST_IS_ELEMENT (element), GST_STATE_CHANGE_FAILURE);
  oclass = GST_ELEMENT_GET_CLASS (element);
  if (oclass->set_state) result = (oclass->set_state) (element, state);
  return result;
}
static GstStateChangeReturn gst_element_set_state_func (GstElement * element, GstState state) {
  GstState current, next, old_pending;
  GstStateChangeReturn ret;
  GstStateChange transition;
  GstStateChangeReturn old_ret;
  g_return_val_if_fail (GST_IS_ELEMENT (element), GST_STATE_CHANGE_FAILURE);
  GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "set_state to %s", gst_element_state_get_name (state));
  g_static_rec_mutex_lock(element);
  GST_OBJECT_LOCK (element);
  old_ret = GST_STATE_RETURN (element);
  if (old_ret == GST_STATE_CHANGE_FAILURE) {
    GST_STATE_NEXT (element) = GST_STATE_VOID_PENDING;
    GST_STATE_PENDING (element) = GST_STATE_VOID_PENDING;
    GST_STATE_RETURN (element) = GST_STATE_CHANGE_SUCCESS;
  }
  current = GST_STATE (element);
  next = GST_STATE_NEXT (element);
  old_pending = GST_STATE_PENDING (element);
  if (state != GST_STATE_TARGET (element)) {
    GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "setting target state to %s", gst_element_state_get_name (state));
    GST_STATE_TARGET (element) = state;
    element->state_cookie++;
  }
  GST_STATE_PENDING (element) = state;
  GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "current %s, old_pending %s, next %s, old return %s", gst_element_state_get_name (current),
                       gst_element_state_get_name (old_pending), gst_element_state_get_name (next),
                       gst_element_state_change_return_get_name (old_ret));
  if (old_pending != GST_STATE_VOID_PENDING) {
    if (old_pending <= state) goto was_busy;
    else if (next == state) goto was_busy;
    else if (next > state && GST_STATE_RETURN (element) == GST_STATE_CHANGE_ASYNC) current = next;
  }
  next = GST_STATE_GET_NEXT (current, state);
  GST_STATE_NEXT (element) = next;
  if (current != next) GST_STATE_RETURN (element) = GST_STATE_CHANGE_ASYNC;
  transition = (GstStateChange) GST_STATE_TRANSITION (current, next);
  GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "%s: setting state from %s to %s", (next != state ? "intermediate" : "final"),
                       gst_element_state_get_name (current), gst_element_state_get_name (next));
  GST_STATE_BROADCAST (element);
  GST_OBJECT_UNLOCK (element);
  ret = gst_element_change_state (element, transition);
  g_static_rec_mutex_free(element);
  GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "returned %s", gst_element_state_change_return_get_name (ret));
  return ret;
was_busy:
  {
    GST_STATE_RETURN (element) = GST_STATE_CHANGE_ASYNC;
    GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "element was busy with async state change");
    GST_OBJECT_UNLOCK (element);
    g_static_rec_mutex_free(element);
    return GST_STATE_CHANGE_ASYNC;
  }
}
GstStateChangeReturn gst_element_change_state (GstElement * element, GstStateChange transition) {
  GstElementClass *oclass;
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  oclass = GST_ELEMENT_GET_CLASS (element);
  GST_TRACER_ELEMENT_CHANGE_STATE_PRE (element, transition);
  if (oclass->change_state) ret = (oclass->change_state) (element, transition);
  else ret = GST_STATE_CHANGE_FAILURE;
  GST_TRACER_ELEMENT_CHANGE_STATE_POST (element, transition, ret);
  switch (ret) {
    case GST_STATE_CHANGE_FAILURE:
      GST_CAT_INFO_OBJECT (GST_CAT_STATES, element, "have FAILURE change_state return");
      gst_element_abort_state (element);
      break;
    case GST_STATE_CHANGE_ASYNC:
    {
      GstState target;
      GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "element will change state ASYNC");
      target = GST_STATE_TARGET (element);
      if (target > GST_STATE_READY) goto async;
      GST_CAT_INFO_OBJECT (GST_CAT_STATES, element, "forcing commit state %s <= %s", gst_element_state_get_name (target),
                          gst_element_state_get_name (GST_STATE_READY));
      ret = gst_element_continue_state (element, GST_STATE_CHANGE_SUCCESS);
      break;
    }
    case GST_STATE_CHANGE_SUCCESS:
      GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "element changed state SUCCESS");
      ret = gst_element_continue_state (element, ret);
      break;
    case GST_STATE_CHANGE_NO_PREROLL:
      GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "element changed state NO_PREROLL");
      ret = gst_element_continue_state (element, ret);
      break;
    default: goto invalid_return;
  }
  GST_CAT_LOG_OBJECT (GST_CAT_STATES, element, "exit state change %d", ret);
  return ret;
async:
  GST_CAT_LOG_OBJECT (GST_CAT_STATES, element, "exit async state change %d", ret);
  return ret;
invalid_return:
  {
    GST_OBJECT_LOCK (element);
    g_critical ("%s: unknown return value %d from a state change function", GST_ELEMENT_NAME (element), ret);
    ret = GST_STATE_CHANGE_FAILURE;
    GST_STATE_RETURN (element) = ret;
    GST_OBJECT_UNLOCK (element);
    return ret;
  }
}
static gboolean activate_pads (const GValue * vpad, GValue * ret, gboolean * active) {
  GstPad *pad = g_value_get_object (vpad);
  gboolean cont = TRUE;
  if (!gst_pad_set_active (pad, *active)) {
    if (GST_PAD_PARENT (pad) != NULL) {
      cont = FALSE;
      g_value_set_boolean (ret, FALSE);
    }
  }
  return cont;
}
static gboolean iterator_activate_fold_with_resync (GstIterator * iter, GstIteratorFoldFunction func, gpointer user_data) {
  GstIteratorResult ires;
  GValue ret = { 0 };
  g_value_init (&ret, G_TYPE_BOOLEAN);
  g_value_set_boolean (&ret, TRUE);
  while (1) {
    ires = gst_iterator_fold (iter, func, &ret, user_data);
    switch (ires) {
      case GST_ITERATOR_RESYNC:
        g_value_set_boolean (&ret, TRUE);
        gst_iterator_resync (iter);
        break;
      case GST_ITERATOR_DONE: goto done;
      default:
        g_value_set_boolean (&ret, FALSE);
        goto done;
    }
  }
done:
  return g_value_get_boolean (&ret);
}
static gboolean gst_element_pads_activate (GstElement * element, gboolean active) {
  GstIterator *iter;
  gboolean res;
  GST_CAT_DEBUG_OBJECT (GST_CAT_ELEMENT_PADS, element, "%s pads", active ? "activate" : "deactivate");
  iter = gst_element_iterate_src_pads (element);
  res = iterator_activate_fold_with_resync (iter, (GstIteratorFoldFunction) activate_pads, &active);
  gst_iterator_free (iter);
  if (G_UNLIKELY (!res)) goto src_failed;
  iter = gst_element_iterate_sink_pads (element);
  res = iterator_activate_fold_with_resync (iter, (GstIteratorFoldFunction) activate_pads, &active);
  gst_iterator_free (iter);
  if (G_UNLIKELY (!res)) goto sink_failed;
  GST_CAT_DEBUG_OBJECT (GST_CAT_ELEMENT_PADS, element, "pad %sactivation successful", active ? "" : "de");
  return TRUE;
src_failed:
  {
    GST_CAT_DEBUG_OBJECT (GST_CAT_ELEMENT_PADS, element, "pad %sactivation failed", active ? "" : "de");
    return FALSE;
  }
sink_failed:
  {
    GST_CAT_DEBUG_OBJECT (GST_CAT_ELEMENT_PADS, element, "sink pads_activate failed");
    return FALSE;
  }
}
static GstStateChangeReturn gst_element_change_state_func (GstElement * element, GstStateChange transition) {
  GstState state, next;
  GstStateChangeReturn result = GST_STATE_CHANGE_SUCCESS;
  g_return_val_if_fail (GST_IS_ELEMENT (element), GST_STATE_CHANGE_FAILURE);
  state = (GstState) GST_STATE_TRANSITION_CURRENT (transition);
  next = GST_STATE_TRANSITION_NEXT (transition);
  if (next == GST_STATE_VOID_PENDING || state == next) goto was_ok;
  GST_CAT_LOG_OBJECT (GST_CAT_STATES, element, "default handler tries setting state from %s to %s (%04x)", gst_element_state_get_name (state),
                     gst_element_state_get_name (next), transition);
  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY: break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      if (!gst_element_pads_activate (element, TRUE)) result = GST_STATE_CHANGE_FAILURE;
      break;
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING: break;
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED: break;
    case GST_STATE_CHANGE_PAUSED_TO_READY: case GST_STATE_CHANGE_READY_TO_NULL:{
      GList *l;
      if (!gst_element_pads_activate (element, FALSE)) result = GST_STATE_CHANGE_FAILURE;
      GST_OBJECT_LOCK (element);
      for (l = element->contexts; l;) {
        GstContext *context = l->data;
        if (!gst_context_is_persistent (context)) {
          GList *next;
          gst_context_unref (context);
          next = l->next;
          element->contexts = g_list_delete_link (element->contexts, l);
          l = next;
        } else l = l->next;
      }
      GST_OBJECT_UNLOCK (element);
      break;
    }
    default:
      g_warning ("Unhandled state change from %s to %s", gst_element_state_get_name (state), gst_element_state_get_name (next));
      break;
  }
  return result;
was_ok:
  {
    GST_OBJECT_LOCK (element);
    result = GST_STATE_RETURN (element);
    GST_CAT_DEBUG_OBJECT (GST_CAT_STATES, element, "element is already in the %s state", gst_element_state_get_name (state));
    GST_OBJECT_UNLOCK (element);
    return result;
  }
}
GstElementFactory *gst_element_get_factory (GstElement * element) {
  g_return_val_if_fail (GST_IS_ELEMENT (element), NULL);
  return GST_ELEMENT_GET_CLASS (element)->elementfactory;
}
static void gst_element_dispose (GObject * object) {
  GstElement *element = GST_ELEMENT_CAST (object);
  GstClock **clock_p;
  GstBus **bus_p;
  GstElementClass *oclass;
  GList *walk;
  oclass = GST_ELEMENT_GET_CLASS (element);
  GST_CAT_INFO_OBJECT (GST_CAT_REFCOUNTING, element, "dispose");
  if (GST_STATE (element) != GST_STATE_NULL) goto not_null;
  walk = element->pads;
  while (walk) {
    GstPad *pad = GST_PAD_CAST (walk->data);
    walk = walk->next;
    if (oclass->release_pad && GST_PAD_PAD_TEMPLATE (pad) && GST_PAD_TEMPLATE_PRESENCE (GST_PAD_PAD_TEMPLATE (pad)) == GST_PAD_REQUEST) {
      GST_CAT_DEBUG_OBJECT (GST_CAT_ELEMENT_PADS, element, "removing request pad %s:%s", GST_DEBUG_PAD_NAME (pad));
      oclass->release_pad (element, pad);
      if (walk && g_list_position (element->pads, walk) == -1) walk = element->pads;
    }
  }
  while (element->pads) {
    GstPad *pad = GST_PAD_CAST (element->pads->data);
    GST_CAT_DEBUG_OBJECT (GST_CAT_ELEMENT_PADS, element, "removing pad %s:%s", GST_DEBUG_PAD_NAME (pad));
    if (!gst_element_remove_pad (element, pad)) {
      g_critical ("failed to remove pad %s:%s", GST_DEBUG_PAD_NAME (pad));
      break;
    }
  }
  GST_OBJECT_LOCK (element);
  clock_p = &element->clock;
  bus_p = &element->bus;
  gst_object_replace ((GstObject **) clock_p, NULL);
  gst_object_replace ((GstObject **) bus_p, NULL);
  g_list_free_full (element->contexts, (GDestroyNotify) gst_context_unref);
  GST_OBJECT_UNLOCK (element);
  GST_CAT_INFO_OBJECT (GST_CAT_REFCOUNTING, element, "parent class dispose");
  G_OBJECT_CLASS (parent_class)->dispose (object);
  return;
not_null:
  {
    gboolean is_locked;
    is_locked = GST_ELEMENT_IS_LOCKED_STATE (element);
    g_critical("\nTrying to dispose element %s, but it is in %s%s instead of the NULL state.\nYou need to explicitly set elements to the "
               "NULL state before\ndropping the final reference, to allow them to clean up.\nThis problem may also be caused by a "
               "refcounting bug in the\napplication or some element.\n", GST_OBJECT_NAME (element),
               gst_element_state_get_name (GST_STATE (element)), is_locked ? " (locked)" : "");
    return;
  }
}
static void gst_element_finalize (GObject * object) {
  GstElement *element = GST_ELEMENT_CAST (object);
  GST_CAT_INFO_OBJECT (GST_CAT_REFCOUNTING, element, "finalize");
  g_cond_clear (&element->state_cond);
  g_static_rec_mutex_free (&element->state_lock);
  GST_CAT_INFO_OBJECT (GST_CAT_REFCOUNTING, element, "finalize parent");
  G_OBJECT_CLASS (parent_class)->finalize (object);
}
static void gst_element_set_bus_func (GstElement * element, GstBus * bus) {
  GstBus **bus_p;
  g_return_if_fail (GST_IS_ELEMENT (element));
  GST_CAT_DEBUG_OBJECT (GST_CAT_PARENTAGE, element, "setting bus to %p", bus);
  GST_OBJECT_LOCK (element);
  bus_p = &GST_ELEMENT_BUS (element);
  gst_object_replace ((GstObject **) bus_p, GST_OBJECT_CAST (bus));
  GST_OBJECT_UNLOCK (element);
}
void gst_element_set_bus (GstElement * element, GstBus * bus) {
  GstElementClass *oclass;
  g_return_if_fail (GST_IS_ELEMENT (element));
  oclass = GST_ELEMENT_GET_CLASS (element);
  if (oclass->set_bus) oclass->set_bus (element, bus);
}
GstBus *gst_element_get_bus (GstElement * element) {
  GstBus *result = NULL;
  g_return_val_if_fail (GST_IS_ELEMENT (element), result);
  GST_OBJECT_LOCK (element);
  if ((result = GST_ELEMENT_BUS (element))) gst_object_ref (result);
  GST_OBJECT_UNLOCK (element);
  GST_CAT_DEBUG_OBJECT (GST_CAT_BUS, element, "got bus %" GST_PTR_FORMAT, result);
  return result;
}
static void gst_element_set_context_default (GstElement * element, GstContext * context) {
  const gchar *context_type;
  GList *l;
  GST_OBJECT_LOCK (element);
  context_type = gst_context_get_context_type (context);
  for (l = element->contexts; l; l = l->next) {
    GstContext *tmp = l->data;
    const gchar *tmp_type = gst_context_get_context_type (tmp);
    if (strcmp (context_type, tmp_type) == 0 && (gst_context_is_persistent (context) || !gst_context_is_persistent (tmp))) {
      gst_context_replace ((GstContext **) & l->data, context);
      break;
    }
  }
  if (l == NULL) element->contexts = g_list_prepend (element->contexts, gst_context_ref (context));
  GST_OBJECT_UNLOCK (element);
}
void gst_element_set_context (GstElement * element, GstContext * context) {
  GstElementClass *oclass;
  g_return_if_fail (GST_IS_ELEMENT (element));
  oclass = GST_ELEMENT_GET_CLASS (element);
  GST_CAT_DEBUG_OBJECT (GST_CAT_CONTEXT, element, "set context %p %" GST_PTR_FORMAT, context, gst_context_get_structure (context));
  if (oclass->set_context) oclass->set_context (element, context);
}
GList *gst_element_get_contexts (GstElement * element) {
  GList *ret;
  g_return_val_if_fail (GST_IS_ELEMENT (element), NULL);
  GST_OBJECT_LOCK (element);
  ret = g_list_copy(element->contexts);
  //ret = g_list_copy_deep (element->contexts, (GCopyFunc) gst_context_ref, NULL);
  GST_OBJECT_UNLOCK (element);
  return ret;
}
static gint _match_context_type (GstContext * c1, const gchar * context_type) {
  const gchar *c1_type;
  c1_type = gst_context_get_context_type (c1);
  return g_strcmp0 (c1_type, context_type);
}
GstContext *gst_element_get_context_unlocked (GstElement * element, const gchar * context_type) {
  GstContext *ret = NULL;
  GList *node;
  g_return_val_if_fail (GST_IS_ELEMENT (element), NULL);
  node = g_list_find_custom (element->contexts, context_type, (GCompareFunc) _match_context_type);
  if (node && node->data) ret = gst_context_ref (node->data);
  return ret;
}
GstContext *gst_element_get_context (GstElement * element, const gchar * context_type) {
  GstContext *ret = NULL;
  g_return_val_if_fail (GST_IS_ELEMENT (element), NULL);
  GST_OBJECT_LOCK (element);
  ret = gst_element_get_context_unlocked (element, context_type);
  GST_OBJECT_UNLOCK (element);
  return ret;
}