#ifndef __G_VARIANT_CORE_H__
#define __G_VARIANT_CORE_H__

#include "gvarianttypeinfo.h"
#include "gvariant.h"
#include "gbuffer.h"

G_GNUC_INTERNAL GVariant *g_variant_new_from_buffer(const GVariantType *type,GBuffer *buffer, gboolean trusted);
G_GNUC_INTERNAL GVariant *g_variant_new_from_children(const GVariantType *type, GVariant **children, gsize n_children, gboolean trusted);
G_GNUC_INTERNAL gboolean g_variant_is_trusted(GVariant *value);
G_GNUC_INTERNAL GVariantTypeInfo *g_variant_get_type_info(GVariant *value);

#endif