#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_ATOMIC_H__
#define __G_ATOMIC_H__

#include "gmacros.h"
#include "gtypes.h"

G_BEGIN_DECLS
gint g_atomic_int_exchange_and_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val);
void g_atomic_int_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val);
gboolean g_atomic_int_compare_and_exchange(volatile gint G_GNUC_MAY_ALIAS *atomic, gint oldval, gint newval);
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval);
gint g_atomic_int_get(volatile gint G_GNUC_MAY_ALIAS *atomic);
void g_atomic_int_set(volatile gint G_GNUC_MAY_ALIAS *atomic, gint newval);
gpointer g_atomic_pointer_get(volatile gpointer G_GNUC_MAY_ALIAS *atomic);
void g_atomic_pointer_set(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer newval);
#ifndef G_ATOMIC_OP_MEMORY_BARRIER_NEEDED
# define g_atomic_int_get(atomic) ((gint)*(atomic))
# define g_atomic_int_set(atomic, newval) ((void)(*(atomic) = (newval)))
# define g_atomic_pointer_get(atomic) ((gpointer)*(atomic))
# define g_atomic_pointer_set(atomic, newval) ((void)(*(atomic) = (newval)))
#else
# define g_atomic_int_get(atomic) \
 ((void)sizeof(gchar[sizeof(*(atomic)) == sizeof(gint) ? 1 : -1]), (g_atomic_int_get)((volatile gint G_GNUC_MAY_ALIAS*)(volatile void*)(atomic)))
# define g_atomic_int_set(atomic, newval) \
 ((void)sizeof(gchar[sizeof(*(atomic)) == sizeof(gint) ? 1 : -1]), (g_atomic_int_set)((volatile gint G_GNUC_MAY_ALIAS*)(volatile void*)(atomic),(newval)))
# define g_atomic_pointer_get(atomic) \
 ((void)sizeof(gchar[sizeof(*(atomic)) == sizeof(gpointer) ? 1 : -1]), (g_atomic_pointer_get)((volatile gpointer G_GNUC_MAY_ALIAS*)(volatile void*)(atomic)))
# define g_atomic_pointer_set(atomic, newval) \
 ((void)sizeof(gchar[sizeof(*(atomic)) == sizeof(gpointer) ? 1 : -1]), (g_atomic_pointer_set)((volatile gpointer G_GNUC_MAY_ALIAS*)(volatile void*)(atomic), (newval)))
#endif
#define g_atomic_int_inc(atomic) (g_atomic_int_add((atomic), 1))
#define g_atomic_int_dec_and_test(atomic) (g_atomic_int_exchange_and_add((atomic), -1) == 1)
G_END_DECLS
#endif