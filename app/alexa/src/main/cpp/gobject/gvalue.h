#if defined (__GLIB_GOBJECT_H_INSIDE__) && !defined (GOBJECT_COMPILATION)
#error "Only <glib-object.h> can be included directly."
#endif

#ifndef __G_VALUE_H__
#define __G_VALUE_H__

#include "gtype.h"

G_BEGIN_DECLS
#define	G_TYPE_IS_VALUE(type)  (g_type_check_is_value_type(type))
#define	G_IS_VALUE(value)  (G_TYPE_CHECK_VALUE(value))
#define	G_VALUE_TYPE(value)  (((GValue*)(value))->g_type)
#define	G_VALUE_TYPE_NAME(value)  (g_type_name(G_VALUE_TYPE(value)))
#define G_VALUE_HOLDS(value,type)  (G_TYPE_CHECK_VALUE_TYPE((value), (type)))
typedef void (*GValueTransform)(const GValue *src_value, GValue *dest_value);
struct _GValue {
  GType	g_type;
  union {
      gint v_int;
      guint v_uint;
      glong v_long;
      gulong v_ulong;
      gint64 v_int64;
      guint64 v_uint64;
      gfloat v_float;
      gdouble v_double;
      gpointer v_pointer;
  } data[2];
};
GValue* g_value_init(GValue *value, GType g_type);
void g_value_copy(const GValue *src_value, GValue *dest_value);
GValue* g_value_reset(GValue *value);
void g_value_unset(GValue *value);
void g_value_set_instance(GValue *value, gpointer instance);
gboolean g_value_fits_pointer(const GValue *value);
gpointer g_value_peek_pointer(const GValue *value);
gboolean g_value_type_compatible(GType src_type, GType dest_type);
gboolean g_value_type_transformable(GType src_type, GType dest_type);
gboolean g_value_transform(const GValue *src_value, GValue *dest_value);
void g_value_register_transform_func(GType src_type, GType dest_type, GValueTransform transform_func);
#define G_VALUE_NOCOPY_CONTENTS (1 << 27)
G_END_DECLS
#endif