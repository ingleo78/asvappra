#include <string.h>
#include "../gio/config.h"
#include "../glib/glib.h"
#include "gboxed.h"
#include "gtype-private.h"
#include "gvalue.h"
#include "gvaluearray.h"
#include "gclosure.h"
#include "gvaluecollector.h"

static inline void value_meminit(GValue *value, GType   value_type) {
    value->g_type = value_type;
    memset(value->data, 0, sizeof(value->data));
}
static GValue * value_copy(GValue *src_value) {
    GValue *dest_value = g_new0(GValue, 1);
    if (G_VALUE_TYPE(src_value)) {
        g_value_init(dest_value, G_VALUE_TYPE(src_value));
        g_value_copy(src_value, dest_value);
    }
    return dest_value;
}
static void value_free(GValue *value) {
    if (G_VALUE_TYPE(value)) g_value_unset(value);
    g_free (value);
}
void g_boxed_type_init(void) {
    static const GTypeInfo info = {
      0,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      0,
      0,
      NULL,
      NULL
    };
    const GTypeFundamentalInfo finfo = { G_TYPE_FLAG_DERIVABLE, };
    GType type;
    type = g_type_register_fundamental(G_TYPE_BOXED,g_intern_static_string("GBoxed"), &info, &finfo,G_TYPE_FLAG_ABSTRACT |
                                       G_TYPE_FLAG_VALUE_ABSTRACT);
    g_assert(type == G_TYPE_BOXED);
}
static GDate *gdate_copy(GDate *date) {
    return g_date_new_julian(g_date_get_julian(date));
}
static GString *gstring_copy(GString *src_gstring) {
    return g_string_new_len(src_gstring->str, src_gstring->len);
}
static void gstring_free(GString *gstring) {
    g_string_free(gstring, TRUE);
}
G_DEFINE_BOXED_TYPE(GClosure, g_closure, g_closure_ref, g_closure_unref);
G_DEFINE_BOXED_TYPE(GValue, g_value, value_copy, value_free);
G_DEFINE_BOXED_TYPE(GValueArray, g_value_array, g_value_array_copy, g_value_array_free);
G_DEFINE_BOXED_TYPE(GDate, g_date, gdate_copy, g_date_free);
G_DEFINE_BOXED_TYPE(GString, g_gstring, gstring_copy, gstring_free);
G_DEFINE_BOXED_TYPE(GHashTable, g_hash_table, g_hash_table_ref, g_hash_table_unref);
G_DEFINE_BOXED_TYPE(GArray, g_array, g_array_ref, g_array_unref);
G_DEFINE_BOXED_TYPE(GPtrArray, g_ptr_array,g_ptr_array_ref, g_ptr_array_unref);
G_DEFINE_BOXED_TYPE(GByteArray, g_byte_array, g_byte_array_ref, g_byte_array_unref);
#ifdef ENABLE_REGEX
G_DEFINE_BOXED_TYPE(GRegex, g_regex, g_regex_ref, g_regex_unref);
#else
GType g_regex_get_type(void) { return G_TYPE_INVALID; }
#endif
#define g_variant_type_get_type g_variant_type_get_gtype
G_DEFINE_BOXED_TYPE(GVariantType, g_variant_type, g_variant_type_copy, g_variant_type_free);
#undef g_variant_type_get_type
G_DEFINE_BOXED_TYPE(GError, g_error, g_error_copy, g_error_free);
G_DEFINE_BOXED_TYPE(GDateTime, g_date_time, g_date_time_ref, g_date_time_unref);
GType g_strv_get_type(void) {
    static volatile gsize g_define_type_id__volatile = 0;
    if (g_once_init_enter(&g_define_type_id__volatile)) {
        GType g_define_type_id = g_boxed_type_register_static(g_intern_static_string("GStrv"), (GBoxedCopyFunc)g_strdupv, (GBoxedFreeFunc)g_strfreev);
        g_once_init_leave(&g_define_type_id__volatile, g_define_type_id);
    }
    return g_define_type_id__volatile;
}
GType g_variant_get_gtype(void) {
    return G_TYPE_VARIANT;
}
static void boxed_proxy_value_init(GValue *value) {
    value->data[0].v_pointer = NULL;
}
static void boxed_proxy_value_free(GValue *value) {
    if (value->data[0].v_pointer && !(value->data[1].v_uint & G_VALUE_NOCOPY_CONTENTS)) _g_type_boxed_free(G_VALUE_TYPE(value), value->data[0].v_pointer);
}
static void boxed_proxy_value_copy(const GValue *src_value, GValue *dest_value) {
    if (src_value->data[0].v_pointer) dest_value->data[0].v_pointer = _g_type_boxed_copy(G_VALUE_TYPE(src_value), src_value->data[0].v_pointer);
    else dest_value->data[0].v_pointer = src_value->data[0].v_pointer;
}
static gpointer boxed_proxy_value_peek_pointer(const GValue *value) {
    return value->data[0].v_pointer;
}
static gchar* boxed_proxy_collect_value(GValue *value, guint n_collect_values, GTypeCValue *collect_values, guint collect_flags) {
    if (!collect_values[0].v_pointer) value->data[0].v_pointer = NULL;
    else {
        if (collect_flags & G_VALUE_NOCOPY_CONTENTS) {
            value->data[0].v_pointer = collect_values[0].v_pointer;
            value->data[1].v_uint = G_VALUE_NOCOPY_CONTENTS;
	    } else value->data[0].v_pointer = _g_type_boxed_copy(G_VALUE_TYPE(value), collect_values[0].v_pointer);
    }
    return NULL;
}
static gchar* boxed_proxy_lcopy_value(const GValue *value, guint n_collect_values, GTypeCValue *collect_values, guint collect_flags) {
  gpointer *boxed_p = collect_values[0].v_pointer;
  if (!boxed_p) return g_strdup_printf("value location for `%s' passed as NULL", G_VALUE_TYPE_NAME(value));
  if (!value->data[0].v_pointer) *boxed_p = NULL;
  else if (collect_flags & G_VALUE_NOCOPY_CONTENTS) *boxed_p = value->data[0].v_pointer;
  else *boxed_p = _g_type_boxed_copy(G_VALUE_TYPE(value), value->data[0].v_pointer);
  return NULL;
}
GType g_boxed_type_register_static(const gchar *name, GBoxedCopyFunc boxed_copy, GBoxedFreeFunc boxed_free) {
    static const GTypeValueTable vtable = {
        boxed_proxy_value_init,
        boxed_proxy_value_free,
        boxed_proxy_value_copy,
        boxed_proxy_value_peek_pointer,
        "p",
        boxed_proxy_collect_value,
        "p",
        boxed_proxy_lcopy_value
    };
    GTypeInfo type_info = {
        0,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        0,
        0,
        NULL,
        &vtable
    };
    GType type;
    g_return_val_if_fail(name != NULL, 0);
    g_return_val_if_fail(boxed_copy != NULL, 0);
    g_return_val_if_fail(boxed_free != NULL, 0);
    g_return_val_if_fail(g_type_from_name(name) == 0, 0);
    type = g_type_register_static(G_TYPE_BOXED, name, &type_info, 0);
    if (type) _g_type_boxed_init(type, boxed_copy, boxed_free);
    return type;
}
gpointer g_boxed_copy(GType boxed_type, gconstpointer src_boxed) {
    GTypeValueTable *value_table;
    gpointer dest_boxed;
    g_return_val_if_fail(G_TYPE_IS_BOXED(boxed_type), NULL);
    g_return_val_if_fail(G_TYPE_IS_ABSTRACT(boxed_type) == FALSE, NULL);
    g_return_val_if_fail(src_boxed != NULL, NULL);
    value_table = g_type_value_table_peek(boxed_type);
    if (!value_table) g_return_val_if_fail(G_TYPE_IS_VALUE_TYPE (boxed_type), NULL);
    if (value_table->value_copy == boxed_proxy_value_copy) dest_boxed = _g_type_boxed_copy(boxed_type, (gpointer)src_boxed);
    else {
        GValue src_value, dest_value;
        value_meminit(&src_value, boxed_type);
        src_value.data[0].v_pointer = (gpointer)src_boxed;
        src_value.data[1].v_uint = G_VALUE_NOCOPY_CONTENTS;
        value_meminit(&dest_value, boxed_type);
        value_table->value_copy(&src_value, &dest_value);
        if (dest_value.data[1].v_ulong)
            g_warning("the copy_value() implementation of type `%s' seems to make use of reserved GValue fields", g_type_name(boxed_type));
        dest_boxed = dest_value.data[0].v_pointer;
      }
    return dest_boxed;
}
void g_boxed_free(GType boxed_type, gpointer boxed) {
    GTypeValueTable *value_table;
    g_return_if_fail(G_TYPE_IS_BOXED(boxed_type));
    g_return_if_fail(G_TYPE_IS_ABSTRACT(boxed_type) == FALSE);
    g_return_if_fail(boxed != NULL);
    value_table = g_type_value_table_peek(boxed_type);
    if (!value_table) g_return_if_fail(G_TYPE_IS_VALUE_TYPE(boxed_type));
    if (value_table->value_free == boxed_proxy_value_free) _g_type_boxed_free(boxed_type, boxed);
    else {
        GValue value;
        value_meminit(&value, boxed_type);
        value.data[0].v_pointer = boxed;
        value_table->value_free(&value);
    }
}
gpointer g_value_get_boxed(const GValue *value) {
    g_return_val_if_fail(G_VALUE_HOLDS_BOXED(value), NULL);
    g_return_val_if_fail(G_TYPE_IS_VALUE(G_VALUE_TYPE(value)), NULL);
    return value->data[0].v_pointer;
}
gpointer g_value_dup_boxed(const GValue *value) {
    g_return_val_if_fail(G_VALUE_HOLDS_BOXED(value), NULL);
    g_return_val_if_fail(G_TYPE_IS_VALUE(G_VALUE_TYPE(value)), NULL);
    return value->data[0].v_pointer ? g_boxed_copy(G_VALUE_TYPE(value), value->data[0].v_pointer) : NULL;
}
static inline void value_set_boxed_internal(GValue *value, gconstpointer boxed, gboolean need_copy, gboolean need_free) {
    if (!boxed) {
        g_value_reset(value);
        return;
    }
    if (value->data[0].v_pointer && !(value->data[1].v_uint & G_VALUE_NOCOPY_CONTENTS)) g_boxed_free (G_VALUE_TYPE(value), value->data[0].v_pointer);
    value->data[1].v_uint = need_free ? 0 : G_VALUE_NOCOPY_CONTENTS;
    value->data[0].v_pointer = need_copy ? g_boxed_copy(G_VALUE_TYPE(value), boxed) : (gpointer)boxed;
}
void g_value_set_boxed(GValue *value, gconstpointer boxed) {
    g_return_if_fail(G_VALUE_HOLDS_BOXED(value));
    g_return_if_fail(G_TYPE_IS_VALUE(G_VALUE_TYPE(value)));
    value_set_boxed_internal(value, boxed, TRUE, TRUE);
}
void g_value_set_static_boxed(GValue *value, gconstpointer boxed) {
    g_return_if_fail(G_VALUE_HOLDS_BOXED(value));
    g_return_if_fail(G_TYPE_IS_VALUE(G_VALUE_TYPE(value)));
    value_set_boxed_internal(value, boxed, FALSE, FALSE);
}
void g_value_set_boxed_take_ownership(GValue *value, gconstpointer boxed) {
    g_value_take_boxed(value, boxed);
}
void g_value_take_boxed(GValue *value, gconstpointer boxed) {
    g_return_if_fail(G_VALUE_HOLDS_BOXED(value));
    g_return_if_fail(G_TYPE_IS_VALUE(G_VALUE_TYPE(value)));
    value_set_boxed_internal(value, boxed, FALSE, TRUE);
}