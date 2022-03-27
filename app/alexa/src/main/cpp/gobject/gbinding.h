#if defined (__GLIB_GOBJECT_H_INSIDE__) && defined (GOBJECT_COMPILATION)
//#error "Only <glib-object.h> can be included directly."
#endif

#ifndef __G_BINDING_H__
#define __G_BINDING_H__

#include "gclosure.h"

G_BEGIN_DECLS
#define G_TYPE_BINDING_FLAGS  (g_binding_flags_get_type())
#define G_TYPE_BINDING  (g_binding_get_type())
#define G_BINDING(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_BINDING, GBinding))
#define G_IS_BINDING(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_BINDING))
typedef struct _GBinding GBinding;
typedef gboolean (* GBindingTransformFunc)(GBinding *binding, const GValue *source_value, GValue *target_value, gpointer user_data);
typedef enum {
  G_BINDING_DEFAULT        = 0,
  G_BINDING_BIDIRECTIONAL  = 1 << 0,
  G_BINDING_SYNC_CREATE    = 1 << 1,
  G_BINDING_INVERT_BOOLEAN = 1 << 2
} GBindingFlags;
GType g_binding_flags_get_type(void) G_GNUC_CONST;
GType g_binding_get_type(void) G_GNUC_CONST;
GBindingFlags g_binding_get_flags(GBinding *binding);
GObject *g_binding_get_source(GBinding *binding);
GObject *g_binding_get_target(GBinding *binding);
G_CONST_RETURN gchar *g_binding_get_source_property(GBinding *binding);
G_CONST_RETURN gchar *g_binding_get_target_property(GBinding *binding);
GBinding *g_object_bind_property(gpointer source, const gchar *source_property, gpointer target, const gchar *target_property, GBindingFlags flags);
GBinding *g_object_bind_property_full(gpointer source, const gchar *source_property, gpointer target, const gchar *target_property, GBindingFlags flags,
                                      GBindingTransformFunc transform_to, GBindingTransformFunc transform_from, gpointer user_data, GDestroyNotify notify);
GBinding *g_object_bind_property_with_closures(gpointer source, const gchar *source_property, gpointer target, const gchar *target_property, GBindingFlags flags,
                                               GClosure *transform_to, GClosure *transform_from);
G_END_DECLS
#endif