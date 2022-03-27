#include <string.h>
#include "../gio/config.h"
#include "../glib/gi18n.h"
#include "../glib/glibintl.h"
#include "../glib/ghash.h"
#include "../glib/glib.h"
#include "genums.h"
#include "gobject.h"
#include "gsignal.h"
#include "gparamspecs.h"
#include "gvaluetypes.h"
#include "gbinding.h"

GType g_binding_flags_get_type(void) {
    static volatile gsize g_define_type_id__volatile = 0;
    if (g_once_init_enter(&g_define_type_id__volatile)) {
        static const GFlagsValue values[] = {
            { G_BINDING_DEFAULT, "G_BINDING_DEFAULT", "default" },
            { G_BINDING_BIDIRECTIONAL, "G_BINDING_BIDIRECTIONAL", "bidirectional" },
            { G_BINDING_SYNC_CREATE, "G_BINDING_SYNC_CREATE", "sync-create" },
            { G_BINDING_INVERT_BOOLEAN, "G_BINDING_INVERT_BOOLEAN", "invert-boolean" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_flags_register_static(g_intern_static_string("GBindingFlags"), values);
        g_once_init_leave(&g_define_type_id__volatile, g_define_type_id);
    }
    return g_define_type_id__volatile;
}
#define G_BINDING_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_BINDING, GBindingClass))
#define G_IS_BINDING_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_BINDING))
#define G_BINDING_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_BINDING, GBindingClass))
typedef struct _GBindingClass  GBindingClass;
struct _GBinding {
    GObject parent_instance;
    GObject *source;
    GObject *target;
    gchar *source_property;
    gchar *target_property;
    GParamSpec *source_pspec;
    GParamSpec *target_pspec;
    GBindingTransformFunc transform_s2t;
    GBindingTransformFunc transform_t2s;
    GBindingFlags flags;
    guint source_notify;
    guint target_notify;
    gpointer transform_data;
    GDestroyNotify notify;
    guint is_frozen : 1;
};
struct _GBindingClass {
    GObjectClass parent_class;
};
enum {
    PROP_0,
    PROP_SOURCE,
    PROP_TARGET,
    PROP_SOURCE_PROPERTY,
    PROP_TARGET_PROPERTY,
    PROP_FLAGS
};
static GQuark quark_gbinding = 0;
G_DEFINE_TYPE(GBinding, g_binding, G_TYPE_OBJECT);
static inline void add_binding_qdata(GObject  *gobject, GBinding *binding) {
    GHashTable *bindings;
    bindings = g_object_get_qdata(gobject, quark_gbinding);
    if (bindings == NULL) {
        bindings = g_hash_table_new(NULL, NULL);
        g_object_set_qdata_full(gobject, quark_gbinding, bindings, (GDestroyNotify)g_hash_table_destroy);
    }
    g_hash_table_insert(bindings, binding, GUINT_TO_POINTER(1));
}
static inline void remove_binding_qdata(GObject  *gobject, GBinding *binding) {
    GHashTable *bindings;
    bindings = g_object_get_qdata(gobject, quark_gbinding);
    g_hash_table_remove(bindings, binding);
}
static void weak_unbind(gpointer  user_data, GObject  *where_the_object_was) {
    GBinding *binding = user_data;
    if (binding->source == where_the_object_was) binding->source = NULL;
    else {
        if (binding->source_notify != 0) g_signal_handler_disconnect(binding->source, binding->source_notify);
        g_object_weak_unref(binding->source, weak_unbind, user_data);
        remove_binding_qdata(binding->source, binding);
        binding->source = NULL;
    }
    if (binding->target == where_the_object_was) binding->target = NULL;
    else {
        if (binding->target_notify != 0)
          g_signal_handler_disconnect(binding->target, binding->target_notify);

      g_object_weak_unref (binding->target, weak_unbind, user_data);
      remove_binding_qdata (binding->target, binding);
      binding->target = NULL;
    }
    g_object_unref (binding);
}
static inline gboolean default_transform(const GValue *value_a, GValue *value_b) {
    if (!g_type_is_a(G_VALUE_TYPE(value_a), G_VALUE_TYPE(value_b))) {
        if (g_value_type_compatible(G_VALUE_TYPE(value_a), G_VALUE_TYPE(value_b))) {
            g_value_copy(value_a, value_b);
            goto done;
        }
        if (g_value_type_transformable(G_VALUE_TYPE(value_a), G_VALUE_TYPE(value_b))) {
            if (g_value_transform(value_a, value_b)) goto done;
            g_warning("%s: Unable to convert a value of type %s to a value of type %s", G_STRLOC, g_type_name(G_VALUE_TYPE(value_a)),
                      g_type_name(G_VALUE_TYPE(value_b)));
            return FALSE;
        }
    } else g_value_copy(value_a, value_b);
done:
    return TRUE;
}
static inline gboolean default_invert_boolean_transform(const GValue *value_a, GValue *value_b) {
  gboolean value;
  g_assert(G_VALUE_HOLDS_BOOLEAN(value_a));
  g_assert(G_VALUE_HOLDS_BOOLEAN(value_b));
  value = g_value_get_boolean(value_a);
  value = !value;
  g_value_set_boolean(value_b, value);
  return TRUE;
}
static gboolean default_transform_to(GBinding *binding, const GValue *value_a, GValue *value_b, gpointer user_data G_GNUC_UNUSED) {
  if (binding->flags & G_BINDING_INVERT_BOOLEAN) return default_invert_boolean_transform(value_a, value_b);
  return default_transform(value_a, value_b);
}
static gboolean default_transform_from(GBinding *binding, const GValue *value_a, GValue *value_b, gpointer user_data G_GNUC_UNUSED) {
    if (binding->flags & G_BINDING_INVERT_BOOLEAN) return default_invert_boolean_transform(value_a, value_b);
    return default_transform(value_a, value_b);
}
static void on_source_notify(GObject    *gobject, GParamSpec *pspec, GBinding   *binding) {
    const gchar *p_name;
    GValue source_value = { 0, };
    GValue target_value = { 0, };
    gboolean res;
    if (binding->is_frozen) return;
    p_name = g_intern_string(pspec->name);
    if (p_name != binding->source_property) return;
    g_value_init(&source_value, G_PARAM_SPEC_VALUE_TYPE(binding->source_pspec));
    g_value_init(&target_value, G_PARAM_SPEC_VALUE_TYPE(binding->target_pspec));
    g_object_get_property(binding->source, binding->source_pspec->name, &source_value);
    res = binding->transform_s2t(binding, &source_value, &target_value, binding->transform_data);
    if (res) {
        binding->is_frozen = TRUE;
        g_param_value_validate(binding->target_pspec, &target_value);
        g_object_set_property(binding->target, binding->target_pspec->name, &target_value);
        binding->is_frozen = FALSE;
    }
    g_value_unset(&source_value);
    g_value_unset(&target_value);
}
static void on_target_notify(GObject    *gobject, GParamSpec *pspec, GBinding   *binding) {
    const gchar *p_name;
    GValue source_value = { 0, };
    GValue target_value = { 0, };
    gboolean res;
    if (binding->is_frozen) return;
    p_name = g_intern_string(pspec->name);
    if (p_name != binding->target_property) return;
    g_value_init(&source_value, G_PARAM_SPEC_VALUE_TYPE(binding->target_pspec));
    g_value_init(&target_value, G_PARAM_SPEC_VALUE_TYPE(binding->source_pspec));
    g_object_get_property(binding->target, binding->target_pspec->name, &source_value);
    res = binding->transform_t2s(binding, &source_value, &target_value, binding->transform_data);
    if (res) {
        binding->is_frozen = TRUE;
        g_param_value_validate(binding->source_pspec, &target_value);
        g_object_set_property(binding->source, binding->source_pspec->name, &target_value);
        binding->is_frozen = FALSE;
    }
    g_value_unset(&source_value);
    g_value_unset(&target_value);
}
static void g_binding_finalize(GObject *gobject) {
    GBinding *binding = G_BINDING(gobject);
    if (binding->notify != NULL) {
        binding->notify(binding->transform_data);
        binding->transform_data = NULL;
        binding->notify = NULL;
    }
    if (binding->source != NULL) {
        if (binding->source_notify != 0) g_signal_handler_disconnect(binding->source, binding->source_notify);
        g_object_weak_unref(binding->source, weak_unbind, binding);
        remove_binding_qdata(binding->source, binding);
    }
    if (binding->target != NULL) {
        if (binding->target_notify != 0) g_signal_handler_disconnect(binding->target, binding->target_notify);
        g_object_weak_unref(binding->target, weak_unbind, binding);
        remove_binding_qdata(binding->target, binding);
    }
    G_OBJECT_CLASS(g_binding_parent_class)->finalize(gobject);
}
static void g_binding_set_property(GObject *gobject, guint prop_id, const GValue *value, GParamSpec *pspec) {
    GBinding *binding = G_BINDING(gobject);
    switch(prop_id) {
        case PROP_SOURCE: binding->source = g_value_get_object(value); break;
        case PROP_SOURCE_PROPERTY: binding->source_property = g_intern_string(g_value_get_string(value)); break;
        case PROP_TARGET: binding->target = g_value_get_object(value); break;
        case PROP_TARGET_PROPERTY: binding->target_property = g_intern_string(g_value_get_string(value)); break;
        case PROP_FLAGS: binding->flags = g_value_get_flags(value); break;
        default: G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec); break;
    }
}
static void g_binding_get_property(GObject *gobject, guint prop_id, GValue *value, GParamSpec *pspec) {
    GBinding *binding = G_BINDING(gobject);
    switch (prop_id) {
        case PROP_SOURCE: g_value_set_object(value, binding->source); break;
        case PROP_SOURCE_PROPERTY: g_value_set_string(value, binding->source_property); break;
        case PROP_TARGET: g_value_set_object(value, binding->target); break;
        case PROP_TARGET_PROPERTY: g_value_set_string(value, binding->target_property); break;
        case PROP_FLAGS: g_value_set_flags(value, binding->flags); break;
        default: G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec); break;
    }
}
static void g_binding_constructed(GObject *gobject) {
    GBinding *binding = G_BINDING(gobject);
    g_assert(binding->source != NULL);
    g_assert(binding->target != NULL);
    g_assert(binding->source_property != NULL);
    g_assert(binding->target_property != NULL);
    binding->source_pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(binding->source), binding->source_property);
    binding->target_pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(binding->target), binding->target_property);
    g_assert(binding->source_pspec != NULL);
    g_assert(binding->target_pspec != NULL);
    binding->transform_s2t = default_transform_to;
    binding->transform_t2s = default_transform_from;
    binding->transform_data = NULL;
    binding->notify = NULL;
    binding->source_notify = g_signal_connect(binding->source, "notify", G_CALLBACK(on_source_notify), binding);
    g_object_weak_ref(binding->source, weak_unbind, binding);
    add_binding_qdata(binding->source, binding);
    if (binding->flags & G_BINDING_BIDIRECTIONAL)
        binding->target_notify = g_signal_connect(binding->target, "notify",G_CALLBACK(on_target_notify), binding);
    g_object_weak_ref(binding->target, weak_unbind, binding);
    add_binding_qdata(binding->target, binding);
}
static void g_binding_class_init(GBindingClass *klass) {
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    quark_gbinding = g_quark_from_static_string("g-binding");
    gobject_class->constructed = g_binding_constructed;
    gobject_class->set_property = g_binding_set_property;
    gobject_class->get_property = g_binding_get_property;
    gobject_class->finalize = g_binding_finalize;
    g_object_class_install_property(gobject_class, PROP_SOURCE, g_param_spec_object ("source", P_("Source"), P_("The source of the binding"),
                                    G_TYPE_OBJECT, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    g_object_class_install_property(gobject_class, PROP_TARGET, g_param_spec_object ("target", P_("Target"), P_("The target of the binding"),
                                    G_TYPE_OBJECT, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    g_object_class_install_property(gobject_class, PROP_SOURCE_PROPERTY, g_param_spec_string ("source-property", P_("Source Property"),
                                    P_("The property on the source to bind"),NULL,G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
                                    G_PARAM_STATIC_STRINGS));
    g_object_class_install_property(gobject_class, PROP_TARGET_PROPERTY,g_param_spec_string ("target-property", P_("Target Property"),
                                    P_("The property on the target to bind"),NULL,G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
                                    G_PARAM_STATIC_STRINGS));
    g_object_class_install_property(gobject_class, PROP_FLAGS, g_param_spec_flags("flags", P_("Flags"), P_("The binding flags"), G_TYPE_BINDING_FLAGS,
                                    G_BINDING_DEFAULT, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
static void g_binding_init(GBinding *binding) {}
GBindingFlags g_binding_get_flags(GBinding *binding) {
    g_return_val_if_fail(G_IS_BINDING(binding), G_BINDING_DEFAULT);
    return binding->flags;
}
GObject* g_binding_get_source(GBinding *binding) {
    g_return_val_if_fail(G_IS_BINDING(binding), NULL);
    return binding->source;
}
GObject *g_binding_get_target(GBinding *binding) {
    g_return_val_if_fail (G_IS_BINDING(binding), NULL);
    return binding->target;
}
G_CONST_RETURN gchar *g_binding_get_source_property(GBinding *binding) {
    g_return_val_if_fail(G_IS_BINDING(binding), NULL);
    return binding->source_property;
}
G_CONST_RETURN gchar *g_binding_get_target_property(GBinding *binding) {
    g_return_val_if_fail(G_IS_BINDING(binding), NULL);
    return binding->target_property;
}
GBinding *g_object_bind_property_full(gpointer source, const gchar *source_property, gpointer target, const gchar *target_property, GBindingFlags flags,
                                      GBindingTransformFunc transform_to, GBindingTransformFunc transform_from, gpointer user_data, GDestroyNotify notify) {
    GParamSpec *pspec;
    GBinding *binding;
    g_return_val_if_fail(G_IS_OBJECT(source), NULL);
    g_return_val_if_fail(source_property != NULL, NULL);
    g_return_val_if_fail(G_IS_OBJECT(target), NULL);
    g_return_val_if_fail(target_property != NULL, NULL);
    if (source == target && g_strcmp0(source_property, target_property) == 0) {
        g_warning("Unable to bind the same property on the same instance");
        return NULL;
    }
    if ((flags & G_BINDING_INVERT_BOOLEAN) && (transform_to != NULL || transform_from != NULL)) flags &= ~G_BINDING_INVERT_BOOLEAN;
    pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(source), source_property);
    if (pspec == NULL) {
        g_warning("%s: The source object of type %s has no property called '%s'", G_STRLOC, G_OBJECT_TYPE_NAME(source), source_property);
        return NULL;
    }
    if (!(pspec->flags & G_PARAM_READABLE)) {
        g_warning("%s: The source object of type %s has no readable property called '%s'", G_STRLOC, G_OBJECT_TYPE_NAME(source), source_property);
        return NULL;
    }
    if ((flags & G_BINDING_BIDIRECTIONAL) && ((pspec->flags & G_PARAM_CONSTRUCT_ONLY) || !(pspec->flags & G_PARAM_WRITABLE))) {
        g_warning("%s: The source object of type %s has no writable property called '%s'", G_STRLOC, G_OBJECT_TYPE_NAME(source), source_property);
        return NULL;
    }
    if ((flags & G_BINDING_INVERT_BOOLEAN) && !(G_PARAM_SPEC_VALUE_TYPE(pspec) == G_TYPE_BOOLEAN)) {
        g_warning ("%s: The G_BINDING_INVERT_BOOLEAN flag can only be used when binding boolean properties; the source property '%s' is of type '%s'",
                   G_STRLOC, source_property, g_type_name(G_PARAM_SPEC_VALUE_TYPE(pspec)));
        return NULL;
    }
    pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(target), target_property);
    if (pspec == NULL) {
        g_warning("%s: The target object of type %s has no property called '%s'", G_STRLOC, G_OBJECT_TYPE_NAME(target), target_property);
        return NULL;
    }
    if ((pspec->flags & G_PARAM_CONSTRUCT_ONLY) || !(pspec->flags & G_PARAM_WRITABLE)) {
        g_warning("%s: The target object of type %s has no writable property called '%s'", G_STRLOC, G_OBJECT_TYPE_NAME(target), target_property);
        return NULL;
    }
    if ((flags & G_BINDING_BIDIRECTIONAL) && !(pspec->flags & G_PARAM_READABLE)) {
        g_warning("%s: The starget object of type %s has no writable property called '%s'", G_STRLOC, G_OBJECT_TYPE_NAME(target), target_property);
        return NULL;
    }
    if ((flags & G_BINDING_INVERT_BOOLEAN) && !(G_PARAM_SPEC_VALUE_TYPE(pspec) == G_TYPE_BOOLEAN)) {
        g_warning("%s: The G_BINDING_INVERT_BOOLEAN flag can only be used when binding boolean properties; the target property '%s' is of type '%s'",
                  G_STRLOC, target_property, g_type_name(G_PARAM_SPEC_VALUE_TYPE(pspec)));
        return NULL;
    }
    binding = g_object_new(G_TYPE_BINDING, "source", source, "source-property", source_property, "target", target, "target-property", target_property,
                            "flags", flags, NULL);
    if (transform_to != NULL) binding->transform_s2t = transform_to;
    if (transform_from != NULL) binding->transform_t2s = transform_from;
    binding->transform_data = user_data;
    binding->notify = notify;
    if (flags & G_BINDING_SYNC_CREATE) on_source_notify(binding->source, binding->source_pspec, binding);
    return binding;
}
GBinding * gobject_bind_property(gpointer source, const gchar *source_property, gpointer target, const gchar *target_property, GBindingFlags flags) {
    return g_object_bind_property_full(source, source_property, target, target_property, flags,NULL,NULL,NULL, NULL);
}
typedef struct _TransformData {
    GClosure *transform_to_closure;
    GClosure *transform_from_closure;
} TransformData;
static gboolean bind_with_closures_transform_to(GBinding *binding, const GValue *source, GValue *target, gpointer data) {
    TransformData *t_data = data;
    GValue params[3] = { { 0, }, { 0, }, { 0, } };
    GValue retval = { 0, };
    gboolean res;
    g_value_init(&params[0], G_TYPE_BINDING);
    g_value_set_object(&params[0], binding);
    g_value_init (&params[1], G_TYPE_VALUE);
    g_value_set_boxed(&params[1], source);
    g_value_init(&params[2], G_TYPE_VALUE);
    g_value_set_boxed(&params[2], target);
    g_value_init(&retval, G_TYPE_BOOLEAN);
    g_value_set_boolean(&retval, FALSE);
    g_closure_invoke(t_data->transform_to_closure, &retval, 3, params, NULL);
    res = g_value_get_boolean(&retval);
    if (res) {
        const GValue *out_value = g_value_get_boxed (&params[2]);
        g_assert (out_value != NULL);
        g_value_copy (out_value, target);
    }
    g_value_unset (&params[0]);
    g_value_unset (&params[1]);
    g_value_unset (&params[2]);
    g_value_unset (&retval);
    return res;
}
static gboolean bind_with_closures_transform_from(GBinding *binding, const GValue *source, GValue *target, gpointer data) {
    TransformData *t_data = data;
    GValue params[3] = { { 0, }, { 0, }, { 0, } };
    GValue retval = { 0, };
    gboolean res;
    g_value_init(&params[0], G_TYPE_BINDING);
    g_value_set_object(&params[0], binding);
    g_value_init(&params[1], G_TYPE_VALUE);
    g_value_set_boxed(&params[1], source);
    g_value_init(&params[2], G_TYPE_VALUE);
    g_value_set_boxed(&params[2], target);
    g_value_init(&retval, G_TYPE_BOOLEAN);
    g_value_set_boolean(&retval, FALSE);
    g_closure_invoke(t_data->transform_from_closure, &retval, 3, params, NULL);
    res = g_value_get_boolean(&retval);
    if (res) {
        const GValue *out_value = g_value_get_boxed(&params[2]);
        g_assert(out_value != NULL);
        g_value_copy(out_value, target);
    }
    g_value_unset (&params[0]);
    g_value_unset (&params[1]);
    g_value_unset (&params[2]);
    g_value_unset (&retval);
    return res;
}
static void bind_with_closures_free_func (gpointer data) {
    TransformData *t_data = data;
    if (t_data->transform_to_closure != NULL) g_closure_unref (t_data->transform_to_closure);
    if (t_data->transform_from_closure != NULL) g_closure_unref (t_data->transform_from_closure);
    g_slice_free (TransformData, t_data);
}
GBinding *g_object_bind_property_with_closures(gpointer source, const gchar *source_property, gpointer target, const gchar *target_property, GBindingFlags flags,
                                               GClosure *transform_to, GClosure *transform_from) {
    TransformData *data;
    data = g_slice_new0 (TransformData);
    if (transform_to != NULL) {
        data->transform_to_closure = g_closure_ref (transform_to);
        g_closure_sink (data->transform_to_closure);
    }
    if (transform_from != NULL) {
        data->transform_from_closure = g_closure_ref (transform_from);
        g_closure_sink (data->transform_from_closure);
    }
    return g_object_bind_property_full(source, source_property,target, target_property, flags,transform_to != NULL ? bind_with_closures_transform_to :
                                       NULL,transform_from != NULL ? bind_with_closures_transform_from : NULL, data, bind_with_closures_free_func);
}