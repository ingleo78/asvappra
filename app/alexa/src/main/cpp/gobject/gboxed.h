#if !defined (__GLIB_GOBJECT_H_INSIDE__) && !defined (GOBJECT_COMPILATION)
//#error "Only <glib-object.h> can be included directly."
#endif

#ifndef __G_BOXED_H__
#define __G_BOXED_H__

#include "gtype.h"

G_BEGIN_DECLS
#define G_TYPE_IS_BOXED(type)  (G_TYPE_FUNDAMENTAL (type) == G_TYPE_BOXED)
#define G_VALUE_HOLDS_BOXED(value)  (G_TYPE_CHECK_VALUE_TYPE ((value), G_TYPE_BOXED))
typedef gpointer (*GBoxedCopyFunc)  (gpointer boxed);
typedef void (*GBoxedFreeFunc)(gpointer boxed);
gpointer g_boxed_copy(GType boxed_type, gconstpointer src_boxed);
void g_boxed_free(GType boxed_type, gpointer boxed);
void g_value_set_boxed(GValue *value, gconstpointer v_boxed);
void g_value_set_static_boxed(GValue *value, gconstpointer v_boxed);
void g_value_take_boxed(GValue *value, gconstpointer v_boxed);
#ifndef G_DISABLE_DEPRECATED
void g_value_set_boxed_take_ownership(GValue *value, gconstpointer v_boxed);
#endif
gpointer g_value_get_boxed(const GValue *value);
gpointer g_value_dup_boxed(const GValue *value);
GType g_boxed_type_register_static(const gchar *name, GBoxedCopyFunc boxed_copy, GBoxedFreeFunc boxed_free);
#define G_TYPE_CLOSURE  (g_closure_get_type())
#define G_TYPE_VALUE  (g_value_get_type())
#define G_TYPE_VALUE_ARRAY  (g_value_array_get_type())
#define G_TYPE_DATE  (g_date_get_type())
#define G_TYPE_STRV  (g_strv_get_type())
#define G_TYPE_GSTRING  (g_gstring_get_type())
#define G_TYPE_HASH_TABLE  (g_hash_table_get_type())
#define G_TYPE_REGEX  (g_regex_get_type())
#define G_TYPE_ARRAY  (g_array_get_type())
#define G_TYPE_BYTE_ARRAY  (g_byte_array_get_type())
#define G_TYPE_PTR_ARRAY  (g_ptr_array_get_type())
#define G_TYPE_VARIANT_TYPE  (g_variant_type_get_gtype())
#define G_TYPE_ERROR  (g_error_get_type())
#define G_TYPE_DATE_TIME  (g_date_time_get_type())
GType g_closure_get_type(void) G_GNUC_CONST;
GType g_value_get_type(void) G_GNUC_CONST;
GType g_value_array_get_type(void) G_GNUC_CONST;
GType g_date_get_type(void) G_GNUC_CONST;
GType g_strv_get_type(void) G_GNUC_CONST;
GType g_gstring_get_type(void) G_GNUC_CONST;
GType g_hash_table_get_type(void) G_GNUC_CONST;
GType g_array_get_type(void) G_GNUC_CONST;
GType g_byte_array_get_type(void) G_GNUC_CONST;
GType g_ptr_array_get_type(void) G_GNUC_CONST;
GType g_variant_type_get_gtype(void) G_GNUC_CONST;
GType g_regex_get_type(void) G_GNUC_CONST;
GType g_error_get_type(void) G_GNUC_CONST;
GType g_date_time_get_type(void) G_GNUC_CONST;
#ifndef G_DISABLE_DEPRECATED
GType g_variant_get_gtype(void) G_GNUC_CONST;
#endif
typedef gchar** GStrv;
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)
#define _G_DEFINE_BOXED_TYPE_BEGIN(TypeName, type_name, copy_func, free_func) \
GType type_name##_get_type(void) { \
    static volatile gsize g_define_type_id__volatile = 0; \
    if (g_once_init_enter(&g_define_type_id__volatile)) { \
        GType (*_g_register_boxed)(const gchar *, union { \
                TypeName * (*do_copy_type)(TypeName*); \
                TypeName * (*do_const_copy_type)(const TypeName*); \
                GBoxedCopyFunc do_copy_boxed; \
            } __attribute__((__transparent_union__)), union { \
                void (*do_free_type)(TypeName*); \
                GBoxedFreeFunc do_free_boxed; \
            } __attribute__((__transparent_union__))) = g_boxed_type_register_static; \
        GType g_define_type_id = _g_register_boxed(g_intern_static_string(#TypeName), copy_func, free_func); \
        {
#else
#define _G_DEFINE_BOXED_TYPE_BEGIN(TypeName, type_name, copy_func, free_func) \
GType type_name##_get_type(void) { \
    static volatile gsize g_define_type_id__volatile = 0; \
    if (g_once_init_enter(&g_define_type_id__volatile)) { \
        GType g_define_type_id = g_boxed_type_register_static(g_intern_static_string(#TypeName), (GBoxedCopyFunc)copy_func, (GBoxedFreeFunc)free_func); \
        {
#endif
#define G_DEFINE_BOXED_TYPE_WITH_CODE(TypeName, type_name, copy_func, free_func, _C_) \
_G_DEFINE_BOXED_TYPE_BEGIN(TypeName, type_name, copy_func, free_func) {_C_;} \
_G_DEFINE_TYPE_EXTENDED_END()
#define G_DEFINE_BOXED_TYPE(TypeName, type_name, copy_func, free_func) G_DEFINE_BOXED_TYPE_WITH_CODE(TypeName, type_name, copy_func, free_func, {})
G_END_DECLS
#endif