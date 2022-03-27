#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_PRIMES_H__
#define __G_PRIMES_H__

#include "gmacros.h"
#include "gtypes.h"

G_BEGIN_DECLS
guint g_spaced_primes_closest(guint num) G_GNUC_CONST;
G_END_DECLS

#endif