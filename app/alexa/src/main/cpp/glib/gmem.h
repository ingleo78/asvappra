#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_MEM_H__
#define __G_MEM_H__

#include "gmacros.h"
#include "gtypes.h"
#include "gslice.h"
#include "glibconfig.h"

G_BEGIN_DECLS
typedef struct _GMemVTable GMemVTable;
#if GLIB_SIZEOF_VOID_P > GLIB_SIZEOF_LONG
#define G_MEM_ALIGN	GLIB_SIZEOF_VOID_P
#else
#define G_MEM_ALIGN	GLIB_SIZEOF_LONG
#endif
void g_free(gpointer mem);
gpointer g_malloc(gsize n_bytes) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(1);
gpointer g_malloc0(gsize n_bytes) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(1);
gpointer g_realloc(gpointer mem, gsize n_bytes) G_GNUC_WARN_UNUSED_RESULT;
gpointer g_try_malloc(gsize n_bytes) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(1);
gpointer g_try_malloc0(gsize n_bytes) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(1);
gpointer g_try_realloc(gpointer	mem, gsize n_bytes) G_GNUC_WARN_UNUSED_RESULT;
gpointer g_malloc_n(gsize n_blocks, gsize n_block_bytes) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE2(1,2);
gpointer g_malloc0_n(gsize n_blocks, gsize n_block_bytes) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE2(1,2);
gpointer g_realloc_n(gpointer mem, gsize n_blocks, gsize n_block_bytes) G_GNUC_WARN_UNUSED_RESULT;
gpointer g_try_malloc_n(gsize n_blocks, gsize n_block_bytes) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE2(1,2);
gpointer g_try_malloc0_n(gsize n_blocks, gsize n_block_bytes) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE2(1,2);
gpointer g_try_realloc_n(gpointer mem, gsize n_blocks, gsize n_block_bytes) G_GNUC_WARN_UNUSED_RESULT;
#if defined (__GNUC__) && (__GNUC__ >= 2) && defined(__OPTIMIZE__)
#  define _G_NEW(struct_type, n_structs, func) (struct_type*)(__extension__ ({			                       \
	  gsize __n = (gsize) (n_structs);			                                                               \
	  gsize __s = sizeof (struct_type);			                                                               \
	  gpointer __p;						                                                                       \
	  if (__s == 1) __p = g_##func (__n);				                                                       \
	  else if (__builtin_constant_p (__n) && (__s == 0 || __n <= G_MAXSIZE / __s)) __p = g_##func (__n * __s); \
	  else __p = g_##func##_n (__n, __s);			                                                           \
	  __p;							                                                                           \
	}))
#  define _G_RENEW(struct_type, mem, n_structs, func) (struct_type *) (__extension__ ({			                    \
	  gsize __n = (gsize) (n_structs);			                                                                    \
	  gsize __s = sizeof (struct_type);			                                                                    \
	  gpointer __p = (gpointer) (mem);			                                                                    \
	  if (__s == 1) __p = g_##func (__p, __n);				                                                        \
	  else if (__builtin_constant_p (__n) && (__s == 0 || __n <= G_MAXSIZE / __s)) __p = g_##func (__p, __n * __s);	\
	  else __p = g_##func##_n (__p, __n, __s);			                                                            \
	  __p;							                                                                                \
	}))
#else
#define _G_NEW(struct_type, n_structs, func) ((struct_type*)g_##func##_n((n_structs), sizeof(struct_type)))
#define _G_RENEW(struct_type, mem, n_structs, func) ((struct_type*)g_##func##_n(mem, (n_structs), sizeof(struct_type)))
#endif
#define g_new(struct_type, n_structs) _G_NEW (struct_type, n_structs, malloc)
#define g_new0(struct_type, n_structs) _G_NEW (struct_type, n_structs, malloc0)
#define g_renew(struct_type, mem, n_structs) _G_RENEW (struct_type, mem, n_structs, realloc)
#define g_try_new(struct_type, n_structs) _G_NEW (struct_type, n_structs, try_malloc)
#define g_try_new0(struct_type, n_structs) _G_NEW (struct_type, n_structs, try_malloc0)
#define g_try_renew(struct_type, mem, n_structs) _G_RENEW (struct_type, mem, n_structs, try_realloc)
struct _GMemVTable {
  gpointer (*malloc)(gsize n_bytes);
  gpointer (*realloc)(gpointer mem, gsize n_bytes);
  void (*free)(gpointer mem);
  gpointer (*calloc)(gsize n_blocks, gsize n_block_bytes);
  gpointer (*try_malloc)(gsize n_bytes);
  gpointer (*try_realloc)(gpointer mem, gsize n_bytes);
};
void g_mem_set_vtable(GMemVTable *vtable);
gboolean g_mem_is_system_malloc (void);
GLIB_VAR gboolean g_mem_gc_friendly;
GLIB_VAR GMemVTable	*glib_mem_profiler_table;
void g_mem_profile(void);
#if !defined (G_DISABLE_DEPRECATED) || defined (GTK_COMPILATION) || defined (GDK_COMPILATION)
typedef struct _GAllocator GAllocator;
typedef struct _GMemChunk  GMemChunk;
#define g_mem_chunk_create(type, pre_alloc, alloc_type)	(g_mem_chunk_new(#type " mem chunks (" #pre_alloc ")", sizeof(type), sizeof(type) * (pre_alloc), (alloc_type)))
#define g_chunk_new(type, chunk) ((type*)g_mem_chunk_alloc (chunk))
#define g_chunk_new0(type, chunk) ((type*)g_mem_chunk_alloc0 (chunk))
#define g_chunk_free(mem, mem_chunk)	G_STMT_START { \
  g_mem_chunk_free ((mem_chunk), (mem)); \
} G_STMT_END
#define G_ALLOC_ONLY	  1
#define G_ALLOC_AND_FREE  2
GMemChunk* g_mem_chunk_new(const gchar *name, gint atom_size, gsize area_size, gint type);
void g_mem_chunk_destroy(GMemChunk *mem_chunk);
gpointer g_mem_chunk_alloc(GMemChunk *mem_chunk);
gpointer g_mem_chunk_alloc0(GMemChunk *mem_chunk);
void g_mem_chunk_free(GMemChunk *mem_chunk, gpointer mem);
void g_mem_chunk_clean(GMemChunk *mem_chunk);
void g_mem_chunk_reset(GMemChunk *mem_chunk);
void g_mem_chunk_print(GMemChunk *mem_chunk);
void g_mem_chunk_info(void);
void g_blow_chunks(void);
GAllocator* g_allocator_new(const gchar *name, guint n_preallocs);
void g_allocator_free(GAllocator *allocator);
gpointer g_steal_pointer(gpointer pp);
#define	G_ALLOCATOR_LIST       (1)
#define	G_ALLOCATOR_SLIST      (2)
#define	G_ALLOCATOR_NODE       (3)
#define g_clear_pointer(pp, destroy) \
  G_STMT_START { \
      G_STATIC_ASSERT(sizeof*(pp) == sizeof(gpointer)); \
      gpointer *_pp = (gpointer*)(pp); \
      gpointer _p; \
      GDestroyNotify _destroy = (GDestroyNotify)(destroy); \
      (void) (0 ? (gpointer)*(pp) : 0); \
      do { \
          _p = g_atomic_pointer_get(_pp); \
      } while G_UNLIKELY(!g_atomic_pointer_compare_and_exchange(_pp, _p, NULL)); \
      if (_p) _destroy(_p); \
  } G_STMT_END
#endif
G_END_DECLS

#endif