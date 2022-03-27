#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_VARIANT_TYPE_H__
#define __G_VARIANT_TYPE_H__

#include "gmessages.h"
#include "gtypes.h"

G_BEGIN_DECLS
typedef struct _GVariantType GVariantType;
#define G_VARIANT_TYPE_BOOLEAN  ((const GVariantType*)"b")
#define G_VARIANT_TYPE_BYTE  ((const GVariantType*)"y")
#define G_VARIANT_TYPE_INT16  ((const GVariantType*)"n")
#define G_VARIANT_TYPE_UINT16  ((const GVariantType*)"q")
#define G_VARIANT_TYPE_INT32  ((const GVariantType*)"i")
#define G_VARIANT_TYPE_UINT32  ((const GVariantType*)"u")
#define G_VARIANT_TYPE_INT64  ((const GVariantType*)"x")
#define G_VARIANT_TYPE_UINT64  ((const GVariantType*)"t")
#define G_VARIANT_TYPE_DOUBLE  ((const GVariantType*)"d")
#define G_VARIANT_TYPE_STRING  ((const GVariantType*)"s")
#define G_VARIANT_TYPE_OBJECT_PATH  ((const GVariantType*)"o")
#define G_VARIANT_TYPE_SIGNATURE  ((const GVariantType*)"g")
#define G_VARIANT_TYPE_VARIANT  ((const GVariantType*)"v")
#define G_VARIANT_TYPE_HANDLE  ((const GVariantType*)"h")
#define G_VARIANT_TYPE_UNIT  ((const GVariantType*)"()")
#define G_VARIANT_TYPE_ANY  ((const GVariantType*)"*")
#define G_VARIANT_TYPE_BASIC  ((const GVariantType*)"?")
#define G_VARIANT_TYPE_MAYBE  ((const GVariantType*)"m*")
#define G_VARIANT_TYPE_ARRAY  ((const GVariantType*)"a*")
#define G_VARIANT_TYPE_TUPLE  ((const GVariantType*)"r")
#define G_VARIANT_TYPE_DICT_ENTRY  ((const GVariantType*)"{?*}")
#define G_VARIANT_TYPE_DICTIONARY  ((const GVariantType*)"a{?*}")
#define G_VARIANT_TYPE_STRING_ARRAY  ((const GVariantType*)"as")
#define G_VARIANT_TYPE_BYTESTRING  ((const GVariantType*)"ay")
#define G_VARIANT_TYPE_BYTESTRING_ARRAY  ((const GVariantType*)"aay")
#ifndef G_DISABLE_CHECKS
#define G_VARIANT_TYPE(type_string) (g_variant_type_checked_((type_string)))
#else
#define G_VARIANT_TYPE(type_string) ((const GVariantType*)(type_string))
#endif
gboolean g_variant_type_string_is_valid(const gchar *type_string);
gboolean g_variant_type_string_scan(const gchar *string, const gchar *limit, const gchar **endptr);
void g_variant_type_free(GVariantType *type);
GVariantType *g_variant_type_copy(const GVariantType *type);
GVariantType *g_variant_type_new(const gchar *type_string);
gsize g_variant_type_get_string_length(const GVariantType *type);
const gchar *g_variant_type_peek_string(const GVariantType *type);
gchar *g_variant_type_dup_string(const GVariantType *type);
gboolean g_variant_type_is_definite(const GVariantType *type);
gboolean g_variant_type_is_container(const GVariantType *type);
gboolean g_variant_type_is_basic(const GVariantType *type);
gboolean g_variant_type_is_maybe(const GVariantType *type);
gboolean g_variant_type_is_array(const GVariantType *type);
gboolean g_variant_type_is_tuple(const GVariantType *type);
gboolean g_variant_type_is_dict_entry(const GVariantType *type);
gboolean g_variant_type_is_variant(const GVariantType *type);
guint g_variant_type_hash(gconstpointer type);
gboolean g_variant_type_equal(gconstpointer type1, gconstpointer type2);
gboolean g_variant_type_is_subtype_of(const GVariantType *type, const GVariantType *supertype);
const GVariantType *g_variant_type_element(const GVariantType *type);
const GVariantType *g_variant_type_first(const GVariantType *type);
const GVariantType *g_variant_type_next(const GVariantType *type);
gsize g_variant_type_n_items(const GVariantType *type);
const GVariantType *g_variant_type_key(const GVariantType *type);
const GVariantType *g_variant_type_value(const GVariantType *type);
GVariantType *g_variant_type_new_array(const GVariantType *element);
GVariantType *g_variant_type_new_maybe(const GVariantType *element);
GVariantType *g_variant_type_new_tuple(const GVariantType * const *items,gint length);
GVariantType *g_variant_type_new_dict_entry(const GVariantType *key, const GVariantType *value);
const GVariantType *g_variant_type_checked_(const gchar *);
G_END_DECLS

#endif