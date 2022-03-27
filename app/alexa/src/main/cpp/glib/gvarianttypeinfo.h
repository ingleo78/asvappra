#ifndef __G_VARIANT_TYPE_INFO_H__
#define __G_VARIANT_TYPE_INFO_H__

#include "gvarianttype.h"

typedef struct _GVariantTypeInfo GVariantTypeInfo;
typedef struct {
  GVariantTypeInfo *type_info;
  gsize i, a;
  gint8 b, c;
  guint8 ending_type;
} GVariantMemberInfo;
#define G_VARIANT_MEMBER_ENDING_FIXED   0
#define G_VARIANT_MEMBER_ENDING_LAST    1
#define G_VARIANT_MEMBER_ENDING_OFFSET  2
const gchar *g_variant_type_info_get_type_string(GVariantTypeInfo *typeinfo);
void g_variant_type_info_query(GVariantTypeInfo *typeinfo, guint *alignment, gsize *size);
GVariantTypeInfo *g_variant_type_info_element(GVariantTypeInfo *typeinfo);
void g_variant_type_info_query_element(GVariantTypeInfo *typeinfo, guint *alignment, gsize *size);
gsize g_variant_type_info_n_members(GVariantTypeInfo *typeinfo);
const GVariantMemberInfo *g_variant_type_info_member_info(GVariantTypeInfo *typeinfo, gsize index);
GVariantTypeInfo *g_variant_type_info_get(const GVariantType *type);
GVariantTypeInfo *g_variant_type_info_ref(GVariantTypeInfo *typeinfo);
void g_variant_type_info_unref(GVariantTypeInfo *typeinfo);
void g_variant_type_info_assert_no_infos(void);
#define G_VARIANT_TYPE_INFO_CHAR_MAYBE  'm'
#define G_VARIANT_TYPE_INFO_CHAR_ARRAY  'a'
#define G_VARIANT_TYPE_INFO_CHAR_TUPLE  '('
#define G_VARIANT_TYPE_INFO_CHAR_DICT_ENTRY  '{'
#define G_VARIANT_TYPE_INFO_CHAR_VARIANT  'v'
#define g_variant_type_info_get_type_char(info)  (g_variant_type_info_get_type_string(info)[0])
#endif