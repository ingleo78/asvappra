#ifndef __G_VARIANT_INTERNAL_H__
#define __G_VARIANT_INTERNAL_H__

#ifndef GLIB_COMPILATION
#define GLIB_COMPILATION
#endif

#include "gvarianttype.h"
#include "gmacros.h"
#include "gtypes.h"
#include "gvariant-serialiser.h"
#include "gvarianttypeinfo.h"

gboolean g_variant_format_string_scan(const gchar *string, const gchar *limit, const gchar **endptr);
GVariantType *g_variant_format_string_scan_type(const gchar *string, const gchar *limit, const gchar **endptr);
#endif