#ifndef __G_VARIANT_SERIALISER_H__
#define __G_VARIANT_SERIALISER_H__

#include "gmacros.h"
#include "gtypes.h"
#include "gvarianttypeinfo.h"

typedef struct {
  GVariantTypeInfo *type_info;
  guchar *data;
  gsize size;
} GVariantSerialised;
gsize g_variant_serialised_n_children(GVariantSerialised container);
GVariantSerialised g_variant_serialised_get_child(GVariantSerialised container, gsize index);
typedef void (*GVariantSerialisedFiller)(GVariantSerialised *serialised, gpointer data);
gsize g_variant_serialiser_needed_size(GVariantTypeInfo *info, GVariantSerialisedFiller gsv_filler, gconstpointer *children, gsize n_children);
void g_variant_serialiser_serialise(GVariantSerialised container, GVariantSerialisedFiller gsv_filler, gconstpointer *children, gsize n_children);
gboolean g_variant_serialised_is_normal(GVariantSerialised value);
void g_variant_serialised_byteswap(GVariantSerialised value);
gboolean g_variant_serialiser_is_string(gconstpointer data, gsize size);
gboolean g_variant_serialiser_is_object_path(gconstpointer data, gsize size);
gboolean g_variant_serialiser_is_signature(gconstpointer data, gsize size);

#endif
