#include <string.h>
#include <signal.h>
#include "../gio/config.h"
#include "../glib/glib.h"
#include "gvaluecollector.h"
#include "gsignal.h"
#include "gparamspecs.h"
#include "gvaluetypes.h"
#include "gobject_trace.h"
#include "gobjectnotifyqueue.c"
#include "gobject.h"

#define PARAM_SPEC_PARAM_ID(pspec)	((pspec)->param_id)
#define	PARAM_SPEC_SET_PARAM_ID(pspec, id)	((pspec)->param_id = (id))
#define OBJECT_HAS_TOGGLE_REF_FLAG 0x1
#define OBJECT_HAS_TOGGLE_REF(object)  ((g_datalist_get_flags(&(object)->qdata) & OBJECT_HAS_TOGGLE_REF_FLAG) != 0)
#define OBJECT_FLOATING_FLAG 0x2
#define CLASS_HAS_PROPS_FLAG 0x1
#define CLASS_HAS_PROPS(class)  ((class)->flags & CLASS_HAS_PROPS_FLAG)
#define CLASS_HAS_CUSTOM_CONSTRUCTOR(class)  ((class)->constructor != g_object_constructor)
#define CLASS_HAS_CUSTOM_CONSTRUCTED(class)  ((class)->constructed != g_object_constructed)
#define CLASS_HAS_DERIVED_CLASS_FLAG 0x2
#define CLASS_HAS_DERIVED_CLASS(class)  ((class)->flags & CLASS_HAS_DERIVED_CLASS_FLAG)
enum {
    NOTIFY,
    LAST_SIGNAL
};
enum {
    PROP_NONE
};
static void	g_object_base_class_init(GObjectClass	*class);
static void	g_object_base_class_finalize(GObjectClass	*class);
static void	g_object_do_class_init(GObjectClass	*class);
static void	g_object_init(GObject *object, GObjectClass	*class);
static GObject*	g_object_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_params);
static void g_object_constructed(GObject *object);
static void	g_object_real_dispose(GObject *object);
static void	g_object_finalize(GObject *object);
static void	g_object_do_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void	g_object_do_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void	g_value_object_init(GValue *value);
static void	g_value_object_free_value(GValue *value);
static void	g_value_object_copy_value(const GValue *src_value, GValue *dest_value);
static void	g_value_object_transform_value(const GValue	*src_value, GValue *dest_value);
static gpointer g_value_object_peek_pointer(const GValue *value);
static gchar* g_value_object_collect_value(GValue *value, guint n_collect_values, GTypeCValue *collect_values, guint collect_flags);
static gchar* g_value_object_lcopy_value(const GValue *value, guint n_collect_values, GTypeCValue *collect_values, guint collect_flags);
static void	g_object_dispatch_properties_changed(GObject *object, guint	n_pspecs, GParamSpec **pspecs);
static inline void object_get_property(GObject *object, GParamSpec *pspec, GValue *value);
static inline void object_set_property(GObject *object, GParamSpec *pspec, const GValue *value, GObjectNotifyQueue *nqueue);
static guint object_floating_flag_handler(GObject *object, gint job);
static void object_interface_check_properties(gpointer func_data, gpointer g_iface);
G_LOCK_DEFINE_STATIC (closure_array_mutex);
G_LOCK_DEFINE_STATIC (weak_refs_mutex);
G_LOCK_DEFINE_STATIC (toggle_refs_mutex);
static GQuark quark_closure_array = 0;
static GQuark quark_weak_refs = 0;
static GQuark quark_toggle_refs = 0;
static GParamSpecPool *pspec_pool = NULL;
static GObjectNotifyContext property_notify_context = { 0, };
static gulong gobject_signals[LAST_SIGNAL] = { 0, };
static guint (*floating_flag_handler) (GObject*, gint) = object_floating_flag_handler;
G_LOCK_DEFINE_STATIC (construction_mutex);
static GSList *construction_objects = NULL;
#ifdef	G_ENABLE_DEBUG
#define	IF_DEBUG(debug_type)  if (_g_type_debug_flags & G_TYPE_DEBUG_##debug_type)
G_LOCK_DEFINE_STATIC(debug_objects);
static volatile GObject *g_trap_object_ref = NULL;
static guint debug_objects_count = 0;
static GHashTable *debug_objects_ht = NULL;
static void debug_objects_foreach(gpointer key, gpointer value, gpointer user_data) {
    GObject *object = value;
    g_message ("[%p] stale %s\tref_count=%u", object, G_OBJECT_TYPE_NAME(object), object->ref_count);
}
static void debug_objects_atexit(void) {
    IF_DEBUG(OBJECTS) {
        G_LOCK(debug_objects);
        g_message("stale GObjects: %u", debug_objects_count);
        g_hash_table_foreach(debug_objects_ht, debug_objects_foreach, NULL);
        G_UNLOCK(debug_objects);
    }
}
#endif
void g_object_type_init(void) {
    static gboolean initialized = FALSE;
    static const GTypeFundamentalInfo finfo = {
        G_TYPE_FLAG_CLASSED | G_TYPE_FLAG_INSTANTIATABLE | G_TYPE_FLAG_DERIVABLE | G_TYPE_FLAG_DEEP_DERIVABLE,
    };
    static GTypeInfo info = {
        sizeof(GObjectClass),
        (GBaseInitFunc)g_object_base_class_init,
        (GBaseFinalizeFunc)g_object_base_class_finalize,
        (GClassInitFunc)g_object_do_class_init,
        NULL,
        NULL,
        sizeof(GObject),
        0,
        (GInstanceInitFunc)g_object_init,
        NULL
    };
    static const GTypeValueTable value_table = {
        g_value_object_init,
        g_value_object_free_value,
        g_value_object_copy_value,
        g_value_object_peek_pointer,
        "p",
        g_value_object_collect_value,
        "p",
        g_value_object_lcopy_value
    };
    GType type;
    g_return_if_fail(initialized == FALSE);
    initialized = TRUE;
    info.value_table = &value_table;
    type = g_type_register_fundamental(G_TYPE_OBJECT, g_intern_static_string("GObject"), &info, &finfo, 0);
    g_assert(type == G_TYPE_OBJECT);
    g_value_register_transform_func (G_TYPE_OBJECT, G_TYPE_OBJECT, g_value_object_transform_value);
#ifdef	G_ENABLE_DEBUG
    IF_DEBUG(OBJECTS) {
        debug_objects_ht = g_hash_table_new(g_direct_hash, NULL);
        g_atexit (debug_objects_atexit);
    }
#endif
}
static void g_object_base_class_init(GObjectClass *class) {
    GObjectClass *pclass = g_type_class_peek_parent(class);
    class->flags &= ~CLASS_HAS_DERIVED_CLASS_FLAG;
    if (pclass) pclass->flags |= CLASS_HAS_DERIVED_CLASS_FLAG;
    class->construct_properties = pclass ? g_slist_copy(pclass->construct_properties) : NULL;
    class->get_property = NULL;
    class->set_property = NULL;
}
static void g_object_base_class_finalize (GObjectClass *class) {
    GList *list, *node;
    _g_signals_destroy(G_OBJECT_CLASS_TYPE(class));
    g_slist_free(class->construct_properties);
    class->construct_properties = NULL;
    list = g_param_spec_pool_list_owned(pspec_pool, G_OBJECT_CLASS_TYPE(class));
    for (node = list; node; node = node->next) {
        GParamSpec *pspec = node->data;
        g_param_spec_pool_remove(pspec_pool, pspec);
        PARAM_SPEC_SET_PARAM_ID(pspec, 0);
        g_param_spec_unref(pspec);
    }
    g_list_free(list);
}
static void g_object_notify_dispatcher(GObject *object, guint n_pspecs, GParamSpec **pspecs) {
    G_OBJECT_GET_CLASS(object)->dispatch_properties_changed(object, n_pspecs, pspecs);
}
static void g_object_do_class_init(GObjectClass *class) {
    quark_closure_array = g_quark_from_static_string("GObject-closure-array");
    quark_weak_refs = g_quark_from_static_string("GObject-weak-references");
    quark_toggle_refs = g_quark_from_static_string("GObject-toggle-references");
    pspec_pool = g_param_spec_pool_new(TRUE);
    property_notify_context.quark_notify_queue = g_quark_from_static_string("GObject-notify-queue");
    property_notify_context.dispatcher = g_object_notify_dispatcher;
    class->constructor = g_object_constructor;
    class->constructed = g_object_constructed;
    class->set_property = g_object_do_set_property;
    class->get_property = g_object_do_get_property;
    class->dispose = g_object_real_dispose;
    class->finalize = g_object_finalize;
    class->dispatch_properties_changed = g_object_dispatch_properties_changed;
    class->notify = NULL;
    gobject_signals[NOTIFY] = g_signal_new(g_intern_static_string("notify"), G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE |
                                           G_SIGNAL_DETAILED | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION, G_STRUCT_OFFSET(GObjectClass, notify), NULL, NULL,
		                                   g_cclosure_marshal_VOID__PARAM, G_TYPE_NONE, 1, G_TYPE_PARAM);
    g_type_add_interface_check(NULL, object_interface_check_properties);
}
static inline void install_property_internal(GType g_type, guint property_id, GParamSpec *pspec) {
    if (g_param_spec_pool_lookup(pspec_pool, pspec->name, g_type, FALSE)) {
        g_warning("When installing property: type `%s' already has a property named `%s'", g_type_name(g_type), pspec->name);
        return;
    }
    g_param_spec_ref(pspec);
    g_param_spec_sink(pspec);
    PARAM_SPEC_SET_PARAM_ID(pspec, property_id);
    g_param_spec_pool_insert(pspec_pool, pspec, g_type);
}
void g_object_class_install_property(GObjectClass *class, guint property_id, GParamSpec *pspec) {
    g_return_if_fail(G_IS_OBJECT_CLASS(class));
    g_return_if_fail(G_IS_PARAM_SPEC(pspec));
    if (CLASS_HAS_DERIVED_CLASS(class)) g_error("Attempt to add property %s::%s to class after it was derived", G_OBJECT_CLASS_NAME(class), pspec->name);
    class->flags |= CLASS_HAS_PROPS_FLAG;
    if (pspec->flags & G_PARAM_WRITABLE) g_return_if_fail(class->set_property != NULL);
    if (pspec->flags & G_PARAM_READABLE) g_return_if_fail(class->get_property != NULL);
    g_return_if_fail(property_id > 0);
    g_return_if_fail(PARAM_SPEC_PARAM_ID (pspec) == 0);
    if (pspec->flags & G_PARAM_CONSTRUCT) g_return_if_fail((pspec->flags & G_PARAM_CONSTRUCT_ONLY) == 0);
    if (pspec->flags & (G_PARAM_CONSTRUCT | G_PARAM_CONSTRUCT_ONLY)) g_return_if_fail(pspec->flags & G_PARAM_WRITABLE);
    install_property_internal(G_OBJECT_CLASS_TYPE(class), property_id, pspec);
    if (pspec->flags & (G_PARAM_CONSTRUCT | G_PARAM_CONSTRUCT_ONLY)) class->construct_properties = g_slist_prepend(class->construct_properties, pspec);
    pspec = g_param_spec_pool_lookup(pspec_pool, pspec->name, g_type_parent(G_OBJECT_CLASS_TYPE(class)), TRUE);
    if (pspec && pspec->flags & (G_PARAM_CONSTRUCT | G_PARAM_CONSTRUCT_ONLY)) class->construct_properties = g_slist_remove(class->construct_properties, pspec);
}
void g_object_class_install_properties(GObjectClass *oclass, guint n_pspecs, GParamSpec **pspecs) {
    GType oclass_type, parent_type;
    gint i;
    g_return_if_fail(G_IS_OBJECT_CLASS(oclass));
    g_return_if_fail(n_pspecs > 1);
    g_return_if_fail(pspecs[0] == NULL);
    if (CLASS_HAS_DERIVED_CLASS(oclass)) g_error("Attempt to add properties to %s after it was derived", G_OBJECT_CLASS_NAME(oclass));
    oclass_type = G_OBJECT_CLASS_TYPE(oclass);
    parent_type = g_type_parent(oclass_type);
    for (i = 1; i < n_pspecs; i++) {
        GParamSpec *pspec = pspecs[i];
        g_return_if_fail(pspec != NULL);
        if (pspec->flags & G_PARAM_WRITABLE) g_return_if_fail(oclass->set_property != NULL);
        if (pspec->flags & G_PARAM_READABLE) g_return_if_fail(oclass->get_property != NULL);
        g_return_if_fail(PARAM_SPEC_PARAM_ID (pspec) == 0);
        if (pspec->flags & G_PARAM_CONSTRUCT) g_return_if_fail((pspec->flags & G_PARAM_CONSTRUCT_ONLY) == 0);
        if (pspec->flags & (G_PARAM_CONSTRUCT | G_PARAM_CONSTRUCT_ONLY)) g_return_if_fail(pspec->flags & G_PARAM_WRITABLE);
        oclass->flags |= CLASS_HAS_PROPS_FLAG;
        install_property_internal(oclass_type, i, pspec);
        if (pspec->flags & (G_PARAM_CONSTRUCT | G_PARAM_CONSTRUCT_ONLY)) oclass->construct_properties = g_slist_prepend(oclass->construct_properties, pspec);
        pspec = g_param_spec_pool_lookup (pspec_pool, pspec->name, parent_type, TRUE);
        if (pspec && pspec->flags & (G_PARAM_CONSTRUCT | G_PARAM_CONSTRUCT_ONLY)) oclass->construct_properties = g_slist_remove(oclass->construct_properties, pspec);
    }
}
void g_object_interface_install_property(gpointer g_iface, GParamSpec *pspec) {
  GTypeInterface *iface_class = g_iface;
  g_return_if_fail(G_TYPE_IS_INTERFACE(iface_class->g_type));
  g_return_if_fail(G_IS_PARAM_SPEC(pspec));
  g_return_if_fail(!G_IS_PARAM_SPEC_OVERRIDE(pspec));
  g_return_if_fail(PARAM_SPEC_PARAM_ID(pspec) == 0);
  install_property_internal(iface_class->g_type, 0, pspec);
}
GParamSpec* g_object_class_find_property(GObjectClass *class, const gchar *property_name) {
  GParamSpec *pspec;
  GParamSpec *redirect;
  g_return_val_if_fail(G_IS_OBJECT_CLASS (class), NULL);
  g_return_val_if_fail(property_name != NULL, NULL);
  pspec = g_param_spec_pool_lookup(pspec_pool, property_name, G_OBJECT_CLASS_TYPE(class), TRUE);
  if (pspec) {
      redirect = g_param_spec_get_redirect_target(pspec);
      if (redirect) return redirect;
      else return pspec;
    } else return NULL;
}
GParamSpec* g_object_interface_find_property(gpointer g_iface, const gchar *property_name) {
    GTypeInterface *iface_class = g_iface;
    g_return_val_if_fail(G_TYPE_IS_INTERFACE(iface_class->g_type), NULL);
    g_return_val_if_fail(property_name != NULL, NULL);
    return g_param_spec_pool_lookup(pspec_pool, property_name, iface_class->g_type, FALSE);
}
void g_object_class_override_property(GObjectClass *oclass, guint property_id, const gchar  *name) {
    GParamSpec *overridden = NULL;
    GParamSpec *new;
    GType parent_type;
    g_return_if_fail(G_IS_OBJECT_CLASS (oclass));
    g_return_if_fail(property_id > 0);
    g_return_if_fail(name != NULL);
    parent_type = g_type_parent(G_OBJECT_CLASS_TYPE (oclass));
    if (parent_type != G_TYPE_NONE) overridden = g_param_spec_pool_lookup(pspec_pool, name, parent_type, TRUE);
    if (!overridden) {
        GType *ifaces;
        guint n_ifaces;
        ifaces = g_type_interfaces(G_OBJECT_CLASS_TYPE(oclass), &n_ifaces);
        while(n_ifaces-- && !overridden) {
  	        overridden = g_param_spec_pool_lookup(pspec_pool, name, ifaces[n_ifaces], FALSE);
	    }
        g_free(ifaces);
    }
    if (!overridden) {
        g_warning("%s: Can't find property to override for '%s::%s'", G_STRFUNC, G_OBJECT_CLASS_NAME(oclass), name);
        return;
    }
    new = g_param_spec_override(name, overridden);
    g_object_class_install_property(oclass, property_id, new);
}
GParamSpec** g_object_class_list_properties(GObjectClass *class, guint *n_properties_p) {
    GParamSpec **pspecs;
    guint n;
    g_return_val_if_fail(G_IS_OBJECT_CLASS(class), NULL);
    pspecs = g_param_spec_pool_list(pspec_pool, G_OBJECT_CLASS_TYPE(class), &n);
    if (n_properties_p) *n_properties_p = n;
    return pspecs;
}
GParamSpec** g_object_interface_list_properties(gpointer g_iface, guint *n_properties_p) {
    GTypeInterface *iface_class = g_iface;
    GParamSpec **pspecs;
    guint n;
    g_return_val_if_fail(G_TYPE_IS_INTERFACE(iface_class->g_type), NULL);
    pspecs = g_param_spec_pool_list(pspec_pool, iface_class->g_type, &n);
    if (n_properties_p) *n_properties_p = n;
    return pspecs;
}
static void g_object_init(GObject *object, GObjectClass *class) {
    object->ref_count = 1;
    g_datalist_init (&object->qdata);
    if (CLASS_HAS_PROPS(class)) g_object_notify_queue_freeze(object, &property_notify_context);
    if (CLASS_HAS_CUSTOM_CONSTRUCTOR(class)) {
        G_LOCK (construction_mutex);
        construction_objects = g_slist_prepend(construction_objects, object);
        G_UNLOCK(construction_mutex);
    }
#ifdef	G_ENABLE_DEBUG
    IF_DEBUG(OBJECTS) {
        G_LOCK(debug_objects);
        debug_objects_count++;
        g_hash_table_insert(debug_objects_ht, object, object);
        G_UNLOCK(debug_objects);
    }
#endif
}
static void g_object_do_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
    switch(property_id) {
        default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec); break;
    }
}
static void g_object_do_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
    switch(property_id) {
        default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec); break;
    }
}
static void g_object_real_dispose(GObject *object) {
    g_signal_handlers_destroy(object);
    g_datalist_id_set_data(&object->qdata, quark_closure_array, NULL);
    g_datalist_id_set_data(&object->qdata, quark_weak_refs, NULL);
}
static void g_object_finalize(GObject *object) {
    g_datalist_clear(&object->qdata);
#ifdef	G_ENABLE_DEBUG
    IF_DEBUG(OBJECTS) {
        G_LOCK(debug_objects);
        g_assert(g_hash_table_lookup(debug_objects_ht, object) == object);
        g_hash_table_remove(debug_objects_ht, object);
        debug_objects_count--;
        G_UNLOCK(debug_objects);
    }
#endif
}
static void g_object_dispatch_properties_changed(GObject *object, guint n_pspecs, GParamSpec **pspecs) {
    guint i;
    for (i = 0; i < n_pspecs; i++) g_signal_emit(object, gobject_signals[NOTIFY], g_quark_from_string (pspecs[i]->name), pspecs[i]);
}
void g_object_run_dispose(GObject *object) {
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(object->ref_count > 0);
    g_object_ref(object);
    TRACE(GOBJECT_OBJECT_DISPOSE(object,G_TYPE_FROM_INSTANCE(object), 0));
    G_OBJECT_GET_CLASS(object)->dispose(object);
    TRACE(GOBJECT_OBJECT_DISPOSE_END(object,G_TYPE_FROM_INSTANCE(object), 0));
    g_object_unref(object);
}
void g_object_freeze_notify(GObject *object) {
    g_return_if_fail(G_IS_OBJECT(object));
    if (g_atomic_int_get(&object->ref_count) == 0) return;
    g_object_ref(object);
    g_object_notify_queue_freeze(object, &property_notify_context);
    g_object_unref(object);
}
static inline void g_object_notify_by_spec_internal(GObject    *object, GParamSpec *pspec) {
    GObjectNotifyQueue *nqueue;
    nqueue = g_object_notify_queue_freeze(object, &property_notify_context);
    g_object_notify_queue_add(object, nqueue, pspec);
    g_object_notify_queue_thaw(object, nqueue);
}
void g_object_notify(GObject *object, const gchar *property_name) {
    GParamSpec *pspec;
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(property_name != NULL);
    if (g_atomic_int_get(&object->ref_count) == 0) return;
    g_object_ref(object);
    pspec = g_param_spec_pool_lookup(pspec_pool, property_name, G_OBJECT_TYPE(object), TRUE);
    if (!pspec) g_warning("%s: object class `%s' has no property named `%s'", G_STRFUNC, G_OBJECT_TYPE_NAME(object), property_name);
    else g_object_notify_by_spec_internal(object, pspec);
    g_object_unref(object);
}
void g_object_notify_by_pspec(GObject *object, GParamSpec *pspec) {
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(G_IS_PARAM_SPEC(pspec));
    g_object_ref(object);
    g_object_notify_by_spec_internal(object, pspec);
    g_object_unref(object);
}
void g_object_thaw_notify(GObject *object) {
    GObjectNotifyQueue *nqueue;
    g_return_if_fail(G_IS_OBJECT(object));
    if (g_atomic_int_get(&object->ref_count) == 0) return;
    g_object_ref(object);
    nqueue = g_object_notify_queue_freeze(object,  &property_notify_context);
    g_object_notify_queue_thaw(object, nqueue);
    g_object_notify_queue_thaw(object, nqueue);
    g_object_unref(object);
}
static inline void object_get_property(GObject *object, GParamSpec *pspec, GValue *value) {
    GObjectClass *class = g_type_class_peek(pspec->owner_type);
    guint param_id = PARAM_SPEC_PARAM_ID(pspec);
    GParamSpec *redirect;
    redirect = g_param_spec_get_redirect_target(pspec);
    if (redirect) pspec = redirect;
    class->get_property(object, param_id, value, pspec);
}
static inline void object_set_property(GObject *object, GParamSpec *pspec, const GValue *value, GObjectNotifyQueue *nqueue) {
    GValue tmp_value = { 0, };
    GObjectClass *class = g_type_class_peek(pspec->owner_type);
    guint param_id = PARAM_SPEC_PARAM_ID(pspec);
    GParamSpec *redirect;
    static gchar* enable_diagnostic = NULL;
    redirect = g_param_spec_get_redirect_target(pspec);
    if (redirect) pspec = redirect;
    if (G_UNLIKELY(!enable_diagnostic)) {
        enable_diagnostic = g_getenv("G_ENABLE_DIAGNOSTIC");
        if (!enable_diagnostic) enable_diagnostic = "0";
    }
    if (enable_diagnostic[0] == '1') {
        if (pspec->flags & G_PARAM_DEPRECATED)
        g_warning("The property %s::%s is deprecated and shouldn't be used anymore. It will be removed in a future version.",
                  G_OBJECT_TYPE_NAME(object), pspec->name);
    }
    g_value_init(&tmp_value, pspec->value_type);
    if (!g_value_transform(value, &tmp_value)) {
        g_warning("unable to set property `%s' of type `%s' from value of type `%s'", pspec->name, g_type_name(pspec->value_type), G_VALUE_TYPE_NAME(value));
    } else if (g_param_value_validate (pspec, &tmp_value) && !(pspec->flags & G_PARAM_LAX_VALIDATION)) {
        gchar *contents = g_strdup_value_contents(value);
        g_warning("value \"%s\" of type `%s' is invalid or out of range for property `%s' of type `%s'", contents, G_VALUE_TYPE_NAME(value),
		          pspec->name, g_type_name(pspec->value_type));
        g_free(contents);
    } else {
        class->set_property(object, param_id, &tmp_value, pspec);
        g_object_notify_queue_add(object, nqueue, pspec);
    }
    g_value_unset(&tmp_value);
}
static void object_interface_check_properties(gpointer func_data, gpointer g_iface) {
    GTypeInterface *iface_class = g_iface;
    GObjectClass *class;
    GType iface_type = iface_class->g_type;
    GParamSpec **pspecs;
    guint n;
    class = g_type_class_ref(iface_class->g_instance_type);
    if (!G_IS_OBJECT_CLASS(class)) return;
    pspecs = g_param_spec_pool_list(pspec_pool, iface_type, &n);
    while (n--) {
        GParamSpec *class_pspec = g_param_spec_pool_lookup(pspec_pool, pspecs[n]->name, G_OBJECT_CLASS_TYPE(class), TRUE);
        if (!class_pspec) {
            g_critical("Object class %s doesn't implement property '%s' from interface '%s'", g_type_name(G_OBJECT_CLASS_TYPE(class)),
                       pspecs[n]->name, g_type_name(iface_type));
            continue;
        }
        if (class_pspec && !g_type_is_a(pspecs[n]->value_type, class_pspec->value_type)) {
            g_critical("Property '%s' on class '%s' has type '%s' which is different from the type '%s', of the property on interface '%s'\n", pspecs[n]->name,
                       g_type_name(G_OBJECT_CLASS_TYPE(class)), g_type_name(G_PARAM_SPEC_VALUE_TYPE(class_pspec)), g_type_name(G_PARAM_SPEC_VALUE_TYPE(pspecs[n])),
                       g_type_name(iface_type));
        }
        #define SUBSET(a,b,mask) (((a) & ~(b) & (mask)) == 0)
        if (class_pspec && (!SUBSET (class_pspec->flags,pspecs[n]->flags,G_PARAM_CONSTRUCT | G_PARAM_CONSTRUCT_ONLY) ||
           !SUBSET(pspecs[n]->flags, class_pspec->flags,G_PARAM_READABLE | G_PARAM_WRITABLE))) {
            g_critical("Flags for property '%s' on class '%s' are not compatible with the property oninterface '%s'\n", pspecs[n]->name,
                       g_type_name(G_OBJECT_CLASS_TYPE(class)), g_type_name(iface_type));
        }
        #undef SUBSET
    }
    g_free(pspecs);
    g_type_class_unref(class);
}
GType g_object_get_type(void) {
    return G_TYPE_OBJECT;
}
gpointer g_object_new(GType object_type, const gchar *first_property_name, ...) {
    GObject *object;
    va_list var_args;
    g_return_val_if_fail(G_TYPE_IS_OBJECT(object_type), NULL);
    if (!first_property_name) return g_object_newv(object_type, 0, NULL);
    va_start(var_args, first_property_name);
    object = g_object_new_valist(object_type, first_property_name, var_args);
    va_end(var_args);
    return object;
}
static gboolean slist_maybe_remove(GSList **slist, gconstpointer data) {
    GSList *last = NULL, *node = *slist;
    while(node) {
        if (node->data == data) {
            if (last) last->next = node->next;
            else *slist = node->next;
            g_slist_free_1(node);
            return TRUE;
        }
        last = node;
        node = last->next;
    }
    return FALSE;
}
static inline gboolean object_in_construction_list(GObject *object) {
    gboolean in_construction;
    G_LOCK(construction_mutex);
    in_construction = g_slist_find(construction_objects, object) != NULL;
    G_UNLOCK(construction_mutex);
    return in_construction;
}
gpointer g_object_newv(GType object_type, guint n_parameters, GParameter *parameters) {
    GObjectConstructParam *cparams = NULL, *oparams;
    GObjectNotifyQueue *nqueue = NULL;
    GObject *object;
    GObjectClass *class, *unref_class = NULL;
    GSList *slist;
    guint n_total_cparams = 0, n_cparams = 0, n_oparams = 0, n_cvalues;
    GValue *cvalues;
    GList *clist = NULL;
    gboolean newly_constructed;
    guint i;
    g_return_val_if_fail(G_TYPE_IS_OBJECT(object_type), NULL);
    class = g_type_class_peek_static(object_type);
    if (!class) class = unref_class = g_type_class_ref(object_type);
    for (slist = class->construct_properties; slist; slist = slist->next) {
        clist = g_list_prepend(clist, slist->data);
        n_total_cparams += 1;
    }
    if (n_parameters == 0 && n_total_cparams == 0) {
        oparams = NULL;
        object = class->constructor(object_type, 0, NULL);
        goto id_construction;
    }
    oparams = g_new(GObjectConstructParam, n_parameters);
    cparams = g_new(GObjectConstructParam, n_total_cparams);
    for (i = 0; i < n_parameters; i++) {
        GValue *value = &parameters[i].value;
        GParamSpec *pspec = g_param_spec_pool_lookup(pspec_pool, parameters[i].name, object_type, TRUE);
        if (!pspec) {
            g_warning("%s: object class `%s' has no property named `%s'", G_STRFUNC, g_type_name(object_type), parameters[i].name);
            continue;
	    }
        if (!(pspec->flags & G_PARAM_WRITABLE)) {
            g_warning("%s: property `%s' of object class `%s' is not writable", G_STRFUNC, pspec->name, g_type_name (object_type));
            continue;
	    }
        if (pspec->flags & (G_PARAM_CONSTRUCT | G_PARAM_CONSTRUCT_ONLY)) {
            GList *list = g_list_find (clist, pspec);
            if (!list) {
                g_warning ("%s: construct property \"%s\" for object `%s' can't be set twice", G_STRFUNC, pspec->name, g_type_name (object_type));
                continue;
            }
            cparams[n_cparams].pspec = pspec;
            cparams[n_cparams].value = value;
            n_cparams++;
            if (!list->prev) clist = list->next;
            else list->prev->next = list->next;
            if (list->next) list->next->prev = list->prev;
            g_list_free_1 (list);
	    } else {
            oparams[n_oparams].pspec = pspec;
            oparams[n_oparams].value = value;
            n_oparams++;
	    }
    }
    n_cvalues = n_total_cparams - n_cparams;
    cvalues = g_new(GValue, n_cvalues);
    while(clist) {
        GList *tmp = clist->next;
        GParamSpec *pspec = clist->data;
        GValue *value = cvalues + n_total_cparams - n_cparams - 1;
        value->g_type = 0;
        g_value_init(value, pspec->value_type);
        g_param_value_set_default (pspec, value);
        cparams[n_cparams].pspec = pspec;
        cparams[n_cparams].value = value;
        n_cparams++;
        g_list_free_1(clist);
        clist = tmp;
    }
    object = class->constructor(object_type, n_total_cparams, cparams);
    g_free(cparams);
    while(n_cvalues--) g_value_unset(cvalues + n_cvalues);
    g_free(cvalues);
id_construction:
    if (CLASS_HAS_CUSTOM_CONSTRUCTOR(class)) {
        G_LOCK(construction_mutex);
        newly_constructed = slist_maybe_remove(&construction_objects, object);
        G_UNLOCK(construction_mutex);
    } else newly_constructed = TRUE;
    if (CLASS_HAS_PROPS(class)) {
        if (newly_constructed || n_oparams) nqueue = g_object_notify_queue_freeze(object, &property_notify_context);
        if (newly_constructed) g_object_notify_queue_thaw(object, nqueue);
    }
    if (newly_constructed && CLASS_HAS_CUSTOM_CONSTRUCTED(class)) class->constructed(object);
    for (i = 0; i < n_oparams; i++) object_set_property(object, oparams[i].pspec, oparams[i].value, nqueue);
    g_free(oparams);
    if (CLASS_HAS_PROPS (class)) {
        if (newly_constructed || n_oparams) g_object_notify_queue_thaw(object, nqueue);
    }
    if (unref_class) g_type_class_unref(unref_class);
    return object;
}
GObject* g_object_new_valist(GType object_type, const gchar *first_property_name, va_list var_args) {
    GObjectClass *class;
    GParameter *params;
    const gchar *name;
    GObject *object;
    guint n_params = 0, n_alloced_params = 16;
    g_return_val_if_fail(G_TYPE_IS_OBJECT(object_type), NULL);
    if (!first_property_name) return g_object_newv(object_type, 0, NULL);
    class = g_type_class_ref(object_type);
    params = g_new0(GParameter, n_alloced_params);
    name = first_property_name;
    while(name) {
        gchar *error = NULL;
        GParamSpec *pspec = g_param_spec_pool_lookup(pspec_pool, name, object_type, TRUE);
        if (!pspec) {
            g_warning("%s: object class `%s' has no property named `%s'", G_STRFUNC, g_type_name(object_type), name);
            break;
        }
        if (n_params >= n_alloced_params) {
            n_alloced_params += 16;
            params = g_renew(GParameter, params, n_alloced_params);
            memset(params + n_params, 0, 16 * (sizeof *params));
        }
        params[n_params].name = name;
        G_VALUE_COLLECT_INIT(&params[n_params].value, pspec->value_type, var_args, 0, &error);
        if (error) {
            g_warning("%s: %s", G_STRFUNC, error);
            g_free(error);
            g_value_unset(&params[n_params].value);
            break;
        }
        n_params++;
        name = va_arg(var_args, gchar*);
    }
    object = g_object_newv(object_type, n_params, params);
    while(n_params--) g_value_unset(&params[n_params].value);
    g_free(params);
    g_type_class_unref(class);
    return object;
}
static GObject* g_object_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_params) {
  GObject *object;
  object = (GObject*)g_type_create_instance(type);
  if (n_construct_properties) {
      GObjectNotifyQueue *nqueue = g_object_notify_queue_freeze(object, &property_notify_context);
      while(n_construct_properties--) {
          GValue *value = construct_params->value;
          GParamSpec *pspec = construct_params->pspec;
          construct_params++;
          object_set_property(object, pspec, value, nqueue);
	  }
      g_object_notify_queue_thaw(object, nqueue);
  }
  return object;
}
static void g_object_constructed(GObject *object) {}
void g_object_set_valist(GObject *object, const gchar *first_property_name, va_list var_args) {
    GObjectNotifyQueue *nqueue;
    const gchar *name;
    g_return_if_fail(G_IS_OBJECT(object));
    g_object_ref(object);
    nqueue = g_object_notify_queue_freeze (object, &property_notify_context);
    name = first_property_name;
    while(name) {
        GValue value = { 0, };
        GParamSpec *pspec;
        gchar *error = NULL;
        pspec = g_param_spec_pool_lookup(pspec_pool, name, G_OBJECT_TYPE(object), TRUE);
        if (!pspec) {
            g_warning("%s: object class `%s' has no property named `%s'", G_STRFUNC, G_OBJECT_TYPE_NAME(object), name);
            break;
	    }
        if (!(pspec->flags & G_PARAM_WRITABLE)) {
	        g_warning("%s: property `%s' of object class `%s' is not writable", G_STRFUNC, pspec->name, G_OBJECT_TYPE_NAME(object));
	        break;
	    }
        if ((pspec->flags & G_PARAM_CONSTRUCT_ONLY) && !object_in_construction_list(object)) {
            g_warning("%s: construct property \"%s\" for object `%s' can't be set after construction", G_STRFUNC, pspec->name, G_OBJECT_TYPE_NAME(object));
            break;
        }
        G_VALUE_COLLECT_INIT(&value, pspec->value_type, var_args,0, &error);
        if (error) {
            g_warning("%s: %s", G_STRFUNC, error);
            g_free(error);
            g_value_unset(&value);
	        break;
	    }
        object_set_property(object, pspec, &value, nqueue);
        g_value_unset(&value);
        name = va_arg(var_args,gchar*);
    }
    g_object_notify_queue_thaw(object, nqueue);
    g_object_unref(object);
}
void g_object_get_valist(GObject *object, const gchar *first_property_name, va_list var_args) {
    const gchar *name;
    g_return_if_fail(G_IS_OBJECT(object));
    g_object_ref(object);
    name = first_property_name;
    while(name) {
        GValue value = { 0, };
        GParamSpec *pspec;
        gchar *error;
        pspec = g_param_spec_pool_lookup(pspec_pool, name, G_OBJECT_TYPE(object), TRUE);
        if (!pspec) {
            g_warning("%s: object class `%s' has no property named `%s'", G_STRFUNC, G_OBJECT_TYPE_NAME(object), name);
            break;
  	    }
        if (!(pspec->flags & G_PARAM_READABLE)) {
            g_warning("%s: property `%s' of object class `%s' is not readable", G_STRFUNC, pspec->name, G_OBJECT_TYPE_NAME(object));
            break;
	    }
        g_value_init(&value, pspec->value_type);
        object_get_property(object, pspec, &value);
        G_VALUE_LCOPY(&value, var_args, 0, &error);
        if (error) {
            g_warning("%s: %s", G_STRFUNC, error);
            g_free(error);
            g_value_unset(&value);
            break;
	    }
        g_value_unset(&value);
        name = va_arg(var_args, gchar*);
    }
    g_object_unref(object);
}
void g_object_set(gpointer _object, const gchar *first_property_name, ...) {
    GObject *object = _object;
    va_list var_args;
    g_return_if_fail(G_IS_OBJECT(object));
    va_start(var_args, first_property_name);
    g_object_set_valist(object, first_property_name, var_args);
    va_end(var_args);
}
void g_object_get(gpointer _object, const gchar *first_property_name, ...) {
    GObject *object = _object;
    va_list var_args;
    g_return_if_fail(G_IS_OBJECT(object));
    va_start(var_args, first_property_name);
    g_object_get_valist(object, first_property_name, var_args);
    va_end(var_args);
}
void g_object_set_property(GObject	*object, const gchar *property_name, const GValue *value) {
    GObjectNotifyQueue *nqueue;
    GParamSpec *pspec;
    g_return_if_fail(G_IS_OBJECT (object));
    g_return_if_fail(property_name != NULL);
    g_return_if_fail(G_IS_VALUE (value));
    g_object_ref(object);
    nqueue = g_object_notify_queue_freeze(object, &property_notify_context);
    pspec = g_param_spec_pool_lookup(pspec_pool, property_name, G_OBJECT_TYPE(object), TRUE);
    if (!pspec) g_warning("%s: object class `%s' has no property named `%s'", G_STRFUNC, G_OBJECT_TYPE_NAME(object), property_name);
    else if (!(pspec->flags & G_PARAM_WRITABLE)) {
        g_warning("%s: property `%s' of object class `%s' is not writable", G_STRFUNC, pspec->name, G_OBJECT_TYPE_NAME(object));
    } else if ((pspec->flags & G_PARAM_CONSTRUCT_ONLY) && !object_in_construction_list(object)) {
        g_warning("%s: construct property \"%s\" for object `%s' can't be set after construction", G_STRFUNC, pspec->name, G_OBJECT_TYPE_NAME(object));
    } else object_set_property(object, pspec, value, nqueue);
    g_object_notify_queue_thaw(object, nqueue);
    g_object_unref(object);
}
void g_object_get_property(GObject *object, const gchar *property_name, GValue *value) {
    GParamSpec *pspec;
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(property_name != NULL);
    g_return_if_fail(G_IS_VALUE(value));
    g_object_ref(object);
    pspec = g_param_spec_pool_lookup(pspec_pool, property_name, G_OBJECT_TYPE(object), TRUE);
    if (!pspec) g_warning("%s: object class `%s' has no property named `%s'",G_STRFUNC, G_OBJECT_TYPE_NAME(object), property_name);
    else if (!(pspec->flags & G_PARAM_READABLE)) {
        g_warning("%s: property `%s' of object class `%s' is not readable", G_STRFUNC, pspec->name, G_OBJECT_TYPE_NAME(object));
    } else {
        GValue *prop_value, tmp_value = { 0, };
        if (G_VALUE_TYPE(value) == pspec->value_type) {
            g_value_reset(value);
            prop_value = value;
	    } else if (!g_value_type_transformable (pspec->value_type, G_VALUE_TYPE(value))) {
            g_warning("%s: can't retrieve property `%s' of type `%s' as value of type `%s'", G_STRFUNC, pspec->name, g_type_name(pspec->value_type),
                      G_VALUE_TYPE_NAME(value));
            g_object_unref(object);
            return;
	    } else {
            g_value_init(&tmp_value, pspec->value_type);
            prop_value = &tmp_value;
	    }
        object_get_property(object, pspec, prop_value);
        if (prop_value != value) {
            g_value_transform(prop_value, value);
            g_value_unset(&tmp_value);
	    }
    }
    g_object_unref(object);
}
gpointer g_object_connect(gpointer _object, const gchar *signal_spec, ...) {
    GObject *object = _object;
    va_list var_args;
    g_return_val_if_fail(G_IS_OBJECT(object), NULL);
    g_return_val_if_fail(object->ref_count > 0, object);
    va_start(var_args, signal_spec);
    while(signal_spec) {
        GCallback callback = va_arg(var_args, GCallback);
        gpointer data = va_arg(var_args, gpointer);
        gulong sid;
        if (strncmp(signal_spec, "signal::", 8) == 0) {
            sid = g_signal_connect_data(object, signal_spec + 8, callback, data, NULL,0);
        } else if (strncmp(signal_spec, "object_signal::", 15) == 0 || strncmp(signal_spec, "object-signal::", 15) == 0) {
            sid = g_signal_connect_object(object, signal_spec + 15, callback, data,0);
        } else if (strncmp(signal_spec, "swapped_signal::", 16) == 0 || strncmp(signal_spec, "swapped-signal::", 16) == 0) {
            sid = g_signal_connect_data(object, signal_spec + 16, callback, data, NULL,G_CONNECT_SWAPPED);
        } else if (strncmp(signal_spec, "swapped_object_signal::", 23) == 0 || strncmp(signal_spec, "swapped-object-signal::", 23) == 0) {
            sid = g_signal_connect_object(object, signal_spec + 23, callback, data,G_CONNECT_SWAPPED);
        } else if (strncmp(signal_spec, "signal_after::", 14) == 0 || strncmp(signal_spec, "signal-after::", 14) == 0) {
            sid = g_signal_connect_data(object, signal_spec + 14, callback, data, NULL, G_CONNECT_AFTER);
        } else if (strncmp(signal_spec, "object_signal_after::", 21) == 0 || strncmp(signal_spec, "object-signal-after::", 21) == 0) {
            sid = g_signal_connect_object(object, signal_spec + 21, callback, data,G_CONNECT_AFTER);
        } else if (strncmp(signal_spec, "swapped_signal_after::", 22) == 0 || strncmp(signal_spec, "swapped-signal-after::", 22) == 0) {
            sid = g_signal_connect_data(object, signal_spec + 22, callback, data, NULL,G_CONNECT_SWAPPED | G_CONNECT_AFTER);
        } else if (strncmp(signal_spec, "swapped_object_signal_after::", 29) == 0 || strncmp(signal_spec, "swapped-object-signal-after::", 29) == 0) {
            sid = g_signal_connect_object(object, signal_spec + 29, callback, data,G_CONNECT_SWAPPED | G_CONNECT_AFTER);
        } else {
            g_warning("%s: invalid signal spec \"%s\"", G_STRFUNC, signal_spec);
            break;
	    }
        signal_spec = va_arg(var_args, gchar*);
    }
    va_end(var_args);
    return object;
}
void g_object_disconnect(gpointer _object, const gchar *signal_spec, ...) {
    GObject *object = _object;
    va_list var_args;
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(object->ref_count > 0);
    va_start(var_args, signal_spec);
    while(signal_spec) {
        GCallback callback = va_arg(var_args, GCallback);
        gpointer data = va_arg(var_args, gpointer);
        guint sid = 0, detail = 0, mask = 0;
        if (strncmp(signal_spec, "any_signal::", 12) == 0 || strncmp(signal_spec, "any-signal::", 12) == 0) {
	    signal_spec += 12;
	    mask = G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA;
        } else if (strcmp(signal_spec, "any_signal") == 0 || strcmp(signal_spec, "any-signal") == 0) {
            signal_spec += 10;
            mask = G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA;
        } else {
            g_warning("%s: invalid signal spec \"%s\"", G_STRFUNC, signal_spec);
            break;
        }
        if ((mask & G_SIGNAL_MATCH_ID) && !g_signal_parse_name(signal_spec, G_OBJECT_TYPE(object), &sid, &detail, FALSE)) {
            g_warning("%s: invalid signal name \"%s\"", G_STRFUNC, signal_spec);
        } else if (!g_signal_handlers_disconnect_matched(object, mask | (detail ? G_SIGNAL_MATCH_DETAIL : 0), sid, detail,NULL,
                   (gpointer)callback, data)) {
            g_warning("%s: signal handler %p(%p) is not connected", G_STRFUNC, callback, data);
        }
        signal_spec = va_arg(var_args, gchar*);
    }
    va_end(var_args);
}
typedef struct {
    GObject *object;
    guint n_weak_refs;
    struct {
        GWeakNotify notify;
        gpointer data;
    } weak_refs[1];
} WeakRefStack;
static void weak_refs_notify(gpointer data) {
    WeakRefStack *wstack = data;
    guint i;
    for (i = 0; i < wstack->n_weak_refs; i++) wstack->weak_refs[i].notify (wstack->weak_refs[i].data, wstack->object);
    g_free(wstack);
}
void g_object_weak_ref(GObject *object, GWeakNotify notify, gpointer data) {
    WeakRefStack *wstack;
    guint i;
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(notify != NULL);
    g_return_if_fail(object->ref_count >= 1);
    G_LOCK(weak_refs_mutex);
    wstack = g_datalist_id_remove_no_notify(&object->qdata, quark_weak_refs);
    if (wstack) {
        i = wstack->n_weak_refs++;
        wstack = g_realloc(wstack, sizeof(*wstack) + sizeof(wstack->weak_refs[0]) * i);
    } else {
        wstack = g_renew(WeakRefStack, NULL, 1);
        wstack->object = object;
        wstack->n_weak_refs = 1;
        i = 0;
    }
    wstack->weak_refs[i].notify = notify;
    wstack->weak_refs[i].data = data;
    g_datalist_id_set_data_full(&object->qdata, quark_weak_refs, wstack, weak_refs_notify);
    G_UNLOCK(weak_refs_mutex);
}
void g_object_weak_unref(GObject *object, GWeakNotify notify, gpointer data) {
    WeakRefStack *wstack;
    gboolean found_one = FALSE;
    g_return_if_fail(G_IS_OBJECT (object));
    g_return_if_fail(notify != NULL);
    G_LOCK(weak_refs_mutex);
    wstack = g_datalist_id_get_data(&object->qdata, quark_weak_refs);
    if (wstack) {
        guint i;
        for (i = 0; i < wstack->n_weak_refs; i++)
	    if (wstack->weak_refs[i].notify == notify && wstack->weak_refs[i].data == data) {
	        found_one = TRUE;
	        wstack->n_weak_refs -= 1;
	        if (i != wstack->n_weak_refs) wstack->weak_refs[i] = wstack->weak_refs[wstack->n_weak_refs];
	        break;
	    }
    }
    G_UNLOCK(weak_refs_mutex);
    if (!found_one) g_warning("%s: couldn't find weak ref %p(%p)", G_STRFUNC, notify, data);
}
void g_object_add_weak_pointer(GObject  *object, gpointer *weak_pointer_location) {
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(weak_pointer_location != NULL);
    g_object_weak_ref(object, (GWeakNotify)g_nullify_pointer, weak_pointer_location);
}
void g_object_remove_weak_pointer(GObject  *object, gpointer *weak_pointer_location) {
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(weak_pointer_location != NULL);
    g_object_weak_unref (object, (GWeakNotify)g_nullify_pointer, weak_pointer_location);
}
static guint object_floating_flag_handler(GObject *object, gint job) {
    gpointer oldvalue;
    switch(job) {
        case +1:
            do {
                oldvalue = g_atomic_pointer_get(&object->qdata);
            } while (!g_atomic_pointer_compare_and_exchange((void**) &object->qdata, oldvalue, (gpointer)((gsize)oldvalue | OBJECT_FLOATING_FLAG)));
            return (gsize)oldvalue & OBJECT_FLOATING_FLAG;
        case -1:
            do {
                oldvalue = g_atomic_pointer_get(&object->qdata);
            } while (!g_atomic_pointer_compare_and_exchange((void**) &object->qdata, oldvalue,(gpointer)((gsize) oldvalue & ~(gsize)OBJECT_FLOATING_FLAG)));
            return (gsize) oldvalue & OBJECT_FLOATING_FLAG;
        default: return 0 != ((gsize)g_atomic_pointer_get(&object->qdata) & OBJECT_FLOATING_FLAG);
    }
}
gboolean g_object_is_floating(gpointer _object) {
    GObject *object = _object;
    g_return_val_if_fail(G_IS_OBJECT(object), FALSE);
    return floating_flag_handler(object, 0);
}
gpointer g_object_ref_sink(gpointer _object) {
    GObject *object = _object;
    gboolean was_floating;
    g_return_val_if_fail(G_IS_OBJECT(object), object);
    g_return_val_if_fail(object->ref_count >= 1, object);
    g_object_ref(object);
    was_floating = floating_flag_handler(object, -1);
    if (was_floating) g_object_unref(object);
    return object;
}
void g_object_force_floating(GObject *object) {
    gboolean was_floating;
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(object->ref_count >= 1);
    was_floating = floating_flag_handler(object, +1);
}
typedef struct {
    GObject *object;
    guint n_toggle_refs;
    struct {
        GToggleNotify notify;
        gpointer data;
    } toggle_refs[1];
} ToggleRefStack;
static void toggle_refs_notify(GObject *object, gboolean is_last_ref) {
    ToggleRefStack tstack, *tstackptr;
    G_LOCK (toggle_refs_mutex);
    tstackptr = g_datalist_id_get_data(&object->qdata, quark_toggle_refs);
    tstack = *tstackptr;
    G_UNLOCK (toggle_refs_mutex);
    g_assert(tstack.n_toggle_refs == 1);
    tstack.toggle_refs[0].notify(tstack.toggle_refs[0].data, tstack.object, is_last_ref);
}
void g_object_add_toggle_ref(GObject *object, GToggleNotify notify, gpointer data) {
  ToggleRefStack *tstack;
  guint i;
  g_return_if_fail(G_IS_OBJECT (object));
  g_return_if_fail(notify != NULL);
  g_return_if_fail(object->ref_count >= 1);
  g_object_ref(object);
  G_LOCK(toggle_refs_mutex);
  tstack = g_datalist_id_remove_no_notify(&object->qdata, quark_toggle_refs);
  if (tstack) {
      i = tstack->n_toggle_refs++;
      tstack = g_realloc(tstack, sizeof(*tstack) + sizeof(tstack->toggle_refs[0]) * i);
    } else {
      tstack = g_renew(ToggleRefStack, NULL, 1);
      tstack->object = object;
      tstack->n_toggle_refs = 1;
      i = 0;
    }
  if (tstack->n_toggle_refs == 1) g_datalist_set_flags(&object->qdata, OBJECT_HAS_TOGGLE_REF_FLAG);
  tstack->toggle_refs[i].notify = notify;
  tstack->toggle_refs[i].data = data;
  g_datalist_id_set_data_full(&object->qdata, quark_toggle_refs, tstack,(GDestroyNotify)g_free);
  G_UNLOCK(toggle_refs_mutex);
}
void g_object_remove_toggle_ref(GObject *object, GToggleNotify notify, gpointer data) {
    ToggleRefStack *tstack;
    gboolean found_one = FALSE;
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(notify != NULL);
    G_LOCK(toggle_refs_mutex);
    tstack = g_datalist_id_get_data(&object->qdata, quark_toggle_refs);
    if (tstack) {
        guint i;
        for (i = 0; i < tstack->n_toggle_refs; i++)
        if (tstack->toggle_refs[i].notify == notify && tstack->toggle_refs[i].data == data) {
            found_one = TRUE;
            tstack->n_toggle_refs -= 1;
            if (i != tstack->n_toggle_refs) tstack->toggle_refs[i] = tstack->toggle_refs[tstack->n_toggle_refs];
            if (tstack->n_toggle_refs == 0) g_datalist_unset_flags(&object->qdata, OBJECT_HAS_TOGGLE_REF_FLAG);
            break;
        }
    }
    G_UNLOCK(toggle_refs_mutex);
    if (found_one) g_object_unref(object);
    else g_warning("%s: couldn't find toggle ref %p(%p)", G_STRFUNC, notify, data);
}
gpointer g_object_ref(gpointer _object) {
    GObject *object = _object;
    gint old_val;
    g_return_val_if_fail(G_IS_OBJECT(object), NULL);
    g_return_val_if_fail(object->ref_count > 0, NULL);
#ifdef  G_ENABLE_DEBUG
    if (g_trap_object_ref == object) G_BREAKPOINT();
#endif
    old_val = g_atomic_int_exchange_and_add((int*)&object->ref_count, 1);
    if (old_val == 1 && OBJECT_HAS_TOGGLE_REF(object)) toggle_refs_notify(object, FALSE);
    TRACE(GOBJECT_OBJECT_REF(object,G_TYPE_FROM_INSTANCE(object),old_val));
    return object;
}
void g_object_unref(gpointer _object) {
    GObject *object = _object;
    gint old_ref;
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(object->ref_count > 0);
#ifdef  G_ENABLE_DEBUG
    if (g_trap_object_ref == object) G_BREAKPOINT();
#endif
retry_atomic_decrement1:
    old_ref = g_atomic_int_get(&object->ref_count);
    if (old_ref > 1) {
        gboolean has_toggle_ref = OBJECT_HAS_TOGGLE_REF(object);
        if (!g_atomic_int_compare_and_exchange((int*)&object->ref_count, old_ref, old_ref - 1)) goto retry_atomic_decrement1;
        TRACE(GOBJECT_OBJECT_UNREF(object, G_TYPE_FROM_INSTANCE(object), old_ref));
        if (old_ref == 2 && has_toggle_ref) toggle_refs_notify (object, TRUE);
    } else {
        TRACE(GOBJECT_OBJECT_DISPOSE(object, G_TYPE_FROM_INSTANCE(object), 1));
        G_OBJECT_GET_CLASS (object)->dispose(object);
        TRACE(GOBJECT_OBJECT_DISPOSE_END(object,G_TYPE_FROM_INSTANCE(object), 1));
    retry_atomic_decrement2:
        old_ref = g_atomic_int_get((int*)&object->ref_count);
        if (old_ref > 1) {
            gboolean has_toggle_ref = OBJECT_HAS_TOGGLE_REF(object);
            if (!g_atomic_int_compare_and_exchange((int*)&object->ref_count, old_ref, old_ref - 1)) goto retry_atomic_decrement2;
	        TRACE(GOBJECT_OBJECT_UNREF(object,G_TYPE_FROM_INSTANCE(object), old_ref));
            if (old_ref == 2 && has_toggle_ref) toggle_refs_notify(object, TRUE);
	        return;
	    }
        g_datalist_id_set_data(&object->qdata, quark_closure_array, NULL);
        g_signal_handlers_destroy(object);
        g_datalist_id_set_data(&object->qdata, quark_weak_refs, NULL);
        old_ref = g_atomic_int_exchange_and_add ((int *)&object->ref_count, -1);
        TRACE (GOBJECT_OBJECT_UNREF(object,G_TYPE_FROM_INSTANCE(object),old_ref));
        if (G_LIKELY (old_ref == 1)) {
            TRACE (GOBJECT_OBJECT_FINALIZE(object,G_TYPE_FROM_INSTANCE(object)));
            G_OBJECT_GET_CLASS (object)->finalize (object);
            TRACE (GOBJECT_OBJECT_FINALIZE_END(object,G_TYPE_FROM_INSTANCE(object)));
        #ifdef	G_ENABLE_DEBUG
            IF_DEBUG (OBJECTS) {
                G_LOCK (debug_objects);
                g_assert (g_hash_table_lookup (debug_objects_ht, object) == NULL);
                G_UNLOCK (debug_objects);
            }
        #endif
            g_type_free_instance ((GTypeInstance*) object);
	    }
    }
}
#undef g_clear_object
void g_clear_object(volatile GObject **object_ptr) {
    gpointer *ptr = (gpointer)object_ptr;
    gpointer old;
    do {
        old = g_atomic_pointer_get(ptr);
    } while G_UNLIKELY(!g_atomic_pointer_compare_and_exchange(ptr, old, NULL));
    if (old) g_object_unref(old);
}
gpointer g_object_get_qdata(GObject *object, GQuark quark) {
    g_return_val_if_fail(G_IS_OBJECT(object), NULL);
    return quark ? g_datalist_id_get_data(&object->qdata, quark) : NULL;
}
void g_object_set_qdata(GObject *object, GQuark quark, gpointer data) {
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(quark > 0);
    g_datalist_id_set_data(&object->qdata, quark, data);
}
void g_object_set_qdata_full(GObject *object, GQuark quark, gpointer data, GDestroyNotify destroy) {
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(quark > 0);
    g_datalist_id_set_data_full(&object->qdata, quark, data,data ? destroy : (GDestroyNotify)NULL);
}
gpointer g_object_steal_qdata(GObject *object, GQuark quark) {
    g_return_val_if_fail(G_IS_OBJECT(object), NULL);
    g_return_val_if_fail(quark > 0, NULL);
    return g_datalist_id_remove_no_notify(&object->qdata, quark);
}
gpointer g_object_get_data(GObject *object, const gchar *key) {
    GQuark quark;
    g_return_val_if_fail(G_IS_OBJECT(object), NULL);
    g_return_val_if_fail(key != NULL, NULL);
    quark = g_quark_try_string(key);
    return quark ? g_datalist_id_get_data(&object->qdata, quark) : NULL;
}
void g_object_set_data(GObject *object, const gchar *key, gpointer data) {
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(key != NULL);
    g_datalist_id_set_data(&object->qdata, g_quark_from_string (key), data);
}
void g_object_set_data_full(GObject *object, const gchar *key, gpointer data, GDestroyNotify destroy) {
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(key != NULL);
    g_datalist_id_set_data_full(&object->qdata, g_quark_from_string(key), data,data ? destroy : (GDestroyNotify)NULL);
}
gpointer g_object_steal_data(GObject *object, const gchar *key) {
    GQuark quark;
    g_return_val_if_fail(G_IS_OBJECT(object), NULL);
    g_return_val_if_fail(key != NULL, NULL);
    quark = g_quark_try_string(key);
    return quark ? g_datalist_id_remove_no_notify(&object->qdata, quark) : NULL;
}
static void g_value_object_init(GValue *value) {
    value->data[0].v_pointer = NULL;
}
static void g_value_object_free_value(GValue *value) {
    if (value->data[0].v_pointer) g_object_unref(value->data[0].v_pointer);
}
static void g_value_object_copy_value(const GValue *src_value, GValue	*dest_value) {
    if (src_value->data[0].v_pointer) dest_value->data[0].v_pointer = g_object_ref(src_value->data[0].v_pointer);
    else dest_value->data[0].v_pointer = NULL;
}
static void g_value_object_transform_value(const GValue *src_value, GValue *dest_value) {
    if (src_value->data[0].v_pointer && g_type_is_a(G_OBJECT_TYPE(src_value->data[0].v_pointer), G_VALUE_TYPE(dest_value))) {
        dest_value->data[0].v_pointer = g_object_ref(src_value->data[0].v_pointer);
    } else dest_value->data[0].v_pointer = NULL;
}
static gpointer g_value_object_peek_pointer(const GValue *value) {
    return value->data[0].v_pointer;
}
static gchar* g_value_object_collect_value(GValue *value, guint n_collect_values, GTypeCValue *collect_values, guint collect_flags) {
    if (collect_values[0].v_pointer) {
        GObject *object = collect_values[0].v_pointer;
        if (object->g_type_instance.g_class == NULL) return g_strconcat("invalid unclassed object pointer for value type `", G_VALUE_TYPE_NAME(value), "'", NULL);
        else if (!g_value_type_compatible(G_OBJECT_TYPE(object), G_VALUE_TYPE(value))) {
            return g_strconcat("invalid object type `", G_OBJECT_TYPE_NAME(object), "' for value type `", G_VALUE_TYPE_NAME(value), "'", NULL);
        }
        value->data[0].v_pointer = g_object_ref(object);
      } else value->data[0].v_pointer = NULL;
    return NULL;
}
static gchar* g_value_object_lcopy_value(const GValue *value, guint n_collect_values, GTypeCValue *collect_values, guint collect_flags) {
    GObject **object_p = collect_values[0].v_pointer;
    if (!object_p) return g_strdup_printf("value location for `%s' passed as NULL", G_VALUE_TYPE_NAME(value));
    if (!value->data[0].v_pointer) *object_p = NULL;
    else if (collect_flags & G_VALUE_NOCOPY_CONTENTS) *object_p = value->data[0].v_pointer;
    else *object_p = g_object_ref(value->data[0].v_pointer);
    return NULL;
}
void g_value_set_object(GValue *value, gpointer v_object) {
    GObject *old;
    g_return_if_fail(G_VALUE_HOLDS_OBJECT(value));
    old = value->data[0].v_pointer;
    if (v_object) {
        g_return_if_fail(G_IS_OBJECT(v_object));
        g_return_if_fail(g_value_type_compatible(G_OBJECT_TYPE(v_object), G_VALUE_TYPE(value)));
        value->data[0].v_pointer = v_object;
        g_object_ref(value->data[0].v_pointer);
      } else value->data[0].v_pointer = NULL;
    if (old) g_object_unref(old);
}
void g_value_set_object_take_ownership(GValue *value, gpointer v_object) {
    g_value_take_object(value, v_object);
}
void g_value_take_object(GValue *value, gpointer v_object) {
    g_return_if_fail(G_VALUE_HOLDS_OBJECT(value));
    if (value->data[0].v_pointer) {
        g_object_unref(value->data[0].v_pointer);
        value->data[0].v_pointer = NULL;
    }
    if (v_object) {
        g_return_if_fail(G_IS_OBJECT(v_object));
        g_return_if_fail(g_value_type_compatible(G_OBJECT_TYPE(v_object), G_VALUE_TYPE(value)));
        value->data[0].v_pointer = v_object;
    }
}
gpointer g_value_get_object(const GValue *value) {
    g_return_val_if_fail(G_VALUE_HOLDS_OBJECT(value), NULL);
    return value->data[0].v_pointer;
}
gpointer g_value_dup_object(const GValue *value) {
    g_return_val_if_fail(G_VALUE_HOLDS_OBJECT(value), NULL);
    return value->data[0].v_pointer ? g_object_ref(value->data[0].v_pointer) : NULL;
}
gulong g_signal_connect_object(gpointer instance, const gchar *detailed_signal, GCallback c_handler, gpointer gobject, GConnectFlags connect_flags) {
    g_return_val_if_fail(G_TYPE_CHECK_INSTANCE(instance), 0);
    g_return_val_if_fail(detailed_signal != NULL, 0);
    g_return_val_if_fail(c_handler != NULL, 0);
    if (gobject) {
        GClosure *closure;
        g_return_val_if_fail(G_IS_OBJECT(gobject), 0);
        closure = ((connect_flags & G_CONNECT_SWAPPED) ? g_cclosure_new_object_swap : g_cclosure_new_object)(c_handler, gobject);
        return g_signal_connect_closure(instance, detailed_signal, closure, connect_flags & G_CONNECT_AFTER);
    } else return g_signal_connect_data(instance, detailed_signal, c_handler, NULL, NULL, connect_flags);
}
typedef struct {
    GObject *object;
    guint n_closures;
    GClosure *closures[1];
} CArray;
static void object_remove_closure(gpointer data, GClosure *closure) {
    GObject *object = data;
    CArray *carray;
    guint i;
    G_LOCK(closure_array_mutex);
    carray = g_object_get_qdata(object, quark_closure_array);
    for (i = 0; i < carray->n_closures; i++)
        if (carray->closures[i] == closure) {
            carray->n_closures--;
            if (i < carray->n_closures) carray->closures[i] = carray->closures[carray->n_closures];
            G_UNLOCK(closure_array_mutex);
            return;
        }
    G_UNLOCK(closure_array_mutex);
    g_assert_not_reached();
}
static void destroy_closure_array(gpointer data) {
    CArray *carray = data;
    GObject *object = carray->object;
    guint i, n = carray->n_closures;
    for (i = 0; i < n; i++) {
        GClosure *closure = carray->closures[i];
        g_closure_remove_invalidate_notifier(closure, object, object_remove_closure);
        g_closure_invalidate(closure);
    }
    g_free(carray);
}
void g_object_watch_closure(GObject  *object, GClosure *closure) {
    CArray *carray;
    guint i;
    g_return_if_fail(G_IS_OBJECT(object));
    g_return_if_fail(closure != NULL);
    g_return_if_fail(closure->is_invalid == FALSE);
    g_return_if_fail(closure->in_marshal == FALSE);
    g_return_if_fail(object->ref_count > 0);
    g_closure_add_invalidate_notifier(closure, object, object_remove_closure);
    g_closure_add_marshal_guards(closure, object, (GClosureNotify)g_object_ref, object, (GClosureNotify)g_object_unref);
    G_LOCK(closure_array_mutex);
    carray = g_datalist_id_remove_no_notify(&object->qdata, quark_closure_array);
    if (!carray) {
        carray = g_renew(CArray, NULL, 1);
        carray->object = object;
        carray->n_closures = 1;
        i = 0;
    } else {
        i = carray->n_closures++;
        carray = g_realloc(carray, sizeof(*carray) + sizeof(carray->closures[0]) * i);
    }
    carray->closures[i] = closure;
    g_datalist_id_set_data_full(&object->qdata, quark_closure_array, carray, destroy_closure_array);
    G_UNLOCK(closure_array_mutex);
}
GClosure* g_closure_new_object(guint sizeof_closure, GObject *object) {
    GClosure *closure;
    g_return_val_if_fail(G_IS_OBJECT(object), NULL);
    g_return_val_if_fail(object->ref_count > 0, NULL);
    closure = g_closure_new_simple(sizeof_closure, object);
    g_object_watch_closure(object, closure);
    return closure;
}
GClosure* g_cclosure_new_object(GCallback callback_func, GObject  *object) {
    GClosure *closure;
    g_return_val_if_fail(G_IS_OBJECT (object), NULL);
    g_return_val_if_fail(object->ref_count > 0, NULL);
    g_return_val_if_fail(callback_func != NULL, NULL);
    closure = g_cclosure_new(callback_func, object, NULL);
    g_object_watch_closure(object, closure);
    return closure;
}
GClosure* g_cclosure_new_object_swap(GCallback callback_func, GObject  *object) {
    GClosure *closure;
    g_return_val_if_fail(G_IS_OBJECT(object), NULL);
    g_return_val_if_fail(object->ref_count > 0, NULL);
    g_return_val_if_fail(callback_func != NULL, NULL);
    closure = g_cclosure_new_swap(callback_func, object, NULL);
    g_object_watch_closure(object, closure);
    return closure;
}
gsize g_object_compat_control(gsize what, gpointer data) {
    gpointer *pp;
    switch(what) {
        case 1: return G_TYPE_INITIALLY_UNOWNED;
        case 2:
            floating_flag_handler = (guint(*)(GObject*,gint)) data;
            return 1;
        case 3:
            pp = data;
            *pp = floating_flag_handler;
            return 1;
        default: return 0;
    }
}
G_DEFINE_TYPE(GInitiallyUnowned, g_initially_unowned, G_TYPE_OBJECT);
static void g_initially_unowned_init(GInitiallyUnowned *object) {
  g_object_force_floating (object);
}
static void g_initially_unowned_class_init (GInitiallyUnownedClass *klass){}