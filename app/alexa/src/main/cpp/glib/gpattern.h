#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_PATTERN_H__
#define __G_PATTERN_H__

#include "gmacros.h"
#include "gtypes.h"

G_BEGIN_DECLS
typedef struct _GPatternSpec GPatternSpec;
GPatternSpec* g_pattern_spec_new(const gchar  *pattern);
void g_pattern_spec_free(GPatternSpec *pspec);
gboolean g_pattern_spec_equal(GPatternSpec *pspec1, GPatternSpec *pspec2);
gboolean g_pattern_match(GPatternSpec *pspec, guint string_length, const gchar *string, const gchar *string_reversed);
gboolean g_pattern_match_string(GPatternSpec *pspec, const gchar *string);
gboolean g_pattern_match_simple(const gchar *pattern, const gchar *string);
G_END_DECLS

#endif