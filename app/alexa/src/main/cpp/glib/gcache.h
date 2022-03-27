#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_CACHE_H__
#define __G_CACHE_H__

#include "glist.h"
#include "gmacros.h"
#include "gtypes.h"

G_BEGIN_DECLS
typedef struct _GCache GCache;
typedef gpointer (*GCacheNewFunc)(gpointer key);
typedef gpointer (*GCacheDupFunc)(gpointer value);
typedef void (*GCacheDestroyFunc)(gpointer value);
GCache* g_cache_new(GCacheNewFunc value_new_func, GCacheDestroyFunc value_destroy_func, GCacheDupFunc key_dup_func, GCacheDestroyFunc key_destroy_func,
                    GHashFunc hash_key_func, GHashFunc hash_value_func, GEqualFunc key_equal_func);
void g_cache_destroy(GCache *cache);
gpointer g_cache_insert(GCache *cache, gpointer key);
void g_cache_remove(GCache *cache, gconstpointer value);
void g_cache_key_foreach(GCache *cache, GHFunc func, gpointer user_data);
#ifndef G_DISABLE_DEPRECATED
void g_cache_value_foreach(GCache *cache, GHFunc func, gpointer user_data);
#endif
G_END_DECLS
#endif