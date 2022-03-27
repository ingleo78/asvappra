#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_QSORT_H__
#define __G_QSORT_H__

#include "gmacros.h"
#include "gtypes.h"
#include "glib-basic-types.h"

G_BEGIN_DECLS
void g_qsort_with_data(gconstpointer pbase, gint total_elems, gsize size, GCompareDataFunc compare_func, gpointer user_data);
G_END_DECLS

#endif