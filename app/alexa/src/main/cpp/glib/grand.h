#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_RAND_H__
#define __G_RAND_H__

#include "gmacros.h"
#include "gtypes.h"
#include "glib-basic-types.h"

G_BEGIN_DECLS
typedef struct _GRand GRand;
GRand* g_rand_new_with_seed(guint32 seed);
GRand* g_rand_new_with_seed_array(const guint32 *seed, guint seed_length);
GRand* g_rand_new(void);
void g_rand_free(GRand *rand_);
GRand* g_rand_copy(GRand *rand_);
void g_rand_set_seed(GRand *rand_, guint32 seed);
void g_rand_set_seed_array (GRand *rand_, const guint32 *seed, guint seed_length);
#define g_rand_boolean(rand_) ((g_rand_int(rand_) & (1 << 15)) != 0)
guint32 g_rand_int(GRand *rand_);
gint32 g_rand_int_range(GRand *rand_, gint32 begin, gint32 end);
gdouble g_rand_double(GRand *rand_);
gdouble g_rand_double_range(GRand *rand_, gdouble begin, gdouble end);
void g_random_set_seed(guint32 seed);
#define g_random_boolean() ((g_random_int() & (1 << 15)) != 0)
guint32 g_random_int(void);
gint32 g_random_int_range(gint32 begin, gint32 end);
gdouble g_random_double(void);
gdouble g_random_double_range(gdouble begin, gdouble end);
G_END_DECLS

#endif