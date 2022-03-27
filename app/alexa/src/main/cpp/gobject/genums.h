#if defined (__GLIB_GOBJECT_H_INSIDE__) && !defined (GOBJECT_COMPILATION)
//#error "Only <glib-object.h> can be included directly."
#endif

#ifndef __G_ENUMS_H__
#define __G_ENUMS_H__

#include "gtype.h"

G_BEGIN_DECLS
#define G_TYPE_IS_ENUM(type)  (G_TYPE_FUNDAMENTAL(type) == G_TYPE_ENUM)
#define G_ENUM_CLASS(class)	 (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_ENUM, GEnumClass))
#define G_IS_ENUM_CLASS(class)	(G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_ENUM))
#define G_ENUM_CLASS_TYPE(class)  (G_TYPE_FROM_CLASS(class))
#define G_ENUM_CLASS_TYPE_NAME(class)  (g_type_name(G_ENUM_CLASS_TYPE (class)))
#define G_TYPE_IS_FLAGS(type)  (G_TYPE_FUNDAMENTAL(type) == G_TYPE_FLAGS)
#define G_FLAGS_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_FLAGS, GFlagsClass))
#define G_IS_FLAGS_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_FLAGS))
#define G_FLAGS_CLASS_TYPE(class)  (G_TYPE_FROM_CLASS(class))
#define G_FLAGS_CLASS_TYPE_NAME(class) (g_type_name(G_FLAGS_CLASS_TYPE(class)))
#define G_VALUE_HOLDS_ENUM(value)  (G_TYPE_CHECK_VALUE_TYPE((value), G_TYPE_ENUM))
#define G_VALUE_HOLDS_FLAGS(value)  (G_TYPE_CHECK_VALUE_TYPE((value), G_TYPE_FLAGS))
typedef struct _GEnumClass GEnumClass;
typedef struct _GFlagsClass GFlagsClass;
typedef struct _GEnumValue  GEnumValue;
typedef struct _GFlagsValue GFlagsValue;
struct	_GEnumClass {
    GTypeClass g_type_class;
    gint minimum;
    gint maximum;
    guint n_values;
    GEnumValue *values;
};
struct	_GFlagsClass {
    GTypeClass g_type_class;
    guint mask;
    guint n_values;
    GFlagsValue *values;
};
struct _GEnumValue {
    gint value;
    const gchar *value_name;
    const gchar *value_nick;
};
struct _GFlagsValue {
    guint value;
    const gchar *value_name;
    const gchar *value_nick;
};
GEnumValue*	g_enum_get_value(GEnumClass	*enum_class, gint value);
GEnumValue*	g_enum_get_value_by_name(GEnumClass	*enum_class, const gchar *name);
GEnumValue*	g_enum_get_value_by_nick(GEnumClass	*enum_class, const gchar *nick);
GFlagsValue* g_flags_get_first_value(GFlagsClass *flags_class, guint value);
GFlagsValue* g_flags_get_value_by_name(GFlagsClass *flags_class, const gchar *name);
GFlagsValue* g_flags_get_value_by_nick(GFlagsClass *flags_class, const gchar *nick);
void g_value_set_enum(GValue *value, gint v_enum);
gint g_value_get_enum(const GValue *value);
void g_value_set_flags(GValue *value, guint v_flags);
guint g_value_get_flags(const GValue   *value);
GType g_enum_register_static(const gchar *name, const GEnumValue *const_static_values);
GType g_flags_register_static(const gchar *name, const GFlagsValue *const_static_values);
void g_enum_complete_type_info(GType g_enum_type, GTypeInfo *info, const GEnumValue  *const_values);
void g_flags_complete_type_info(GType g_flags_type, GTypeInfo *info, const GFlagsValue *const_values);
G_END_DECLS
#endif