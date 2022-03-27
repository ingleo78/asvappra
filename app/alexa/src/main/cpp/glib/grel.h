#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_REL_H__
#define __G_REL_H__

#include "gmacros.h"
#include "gtypes.h"

G_BEGIN_DECLS
typedef struct _GRelation GRelation;
typedef struct _GTuples GTuples;
typedef guint (*GHashFunc)(gconstpointer  key);
typedef gint (*GEqualFunc)(gconstpointer  a, gconstpointer  b);
struct _GTuples {
  guint len;
};
#ifndef G_DISABLE_DEPRECATED
GRelation* g_relation_new(gint fields);
void g_relation_destroy(GRelation *relation);
void g_relation_index(GRelation *relation, gint field, GHashFunc hash_func, GEqualFunc key_equal_func);
void g_relation_insert(GRelation *relation, ...);
gint g_relation_delete(GRelation *relation, gconstpointer key, gint field);
GTuples* g_relation_select(GRelation *relation, gconstpointer key, gint field);
gint g_relation_count(GRelation *relation, gconstpointer key, gint field);
int g_relation_exists(GRelation *relation, ...);
void g_relation_print(GRelation *relation);
void g_tuples_destroy(GTuples *tuples);
gpointer g_tuples_index(GTuples *tuples, gint index_, gint field);
#endif
G_END_DECLS

#endif