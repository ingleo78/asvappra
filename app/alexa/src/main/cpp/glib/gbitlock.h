#ifndef __G_BITLOCK_H__
#define __G_BITLOCK_H__

#include "gmacros.h"
#include "gtypes.h"

#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

G_BEGIN_DECLS
void g_bit_lock(volatile gint *address, gint lock_bit);
gboolean g_bit_trylock(volatile gint *address, gint lock_bit);
void g_bit_unlock(volatile gint *address, gint lock_bit);
G_END_DECLS

#endif