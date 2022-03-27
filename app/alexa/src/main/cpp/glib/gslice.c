#include "glibconfig.h"

#if     defined HAVE_POSIX_MEMALIGN && defined POSIX_MEMALIGN_WITH_COMPLIANT_ALLOCS
#  define HAVE_COMPLIANT_POSIX_MEMALIGN 1
#endif

#ifdef HAVE_COMPLIANT_POSIX_MEMALIGN
#define _XOPEN_SOURCE 600
#endif
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#ifndef G_OS_WIN32
#include <windows.h>
#include <process.h>
#endif
#include <stdio.h>
#include "gslice.h"
#include "gmain.h"
#include "gmem.h"
#include "gstrfuncs.h"
#include "gutils.h"
#include "gtestutils.h"
#include "gthread.h"
#include "gthreadprivate.h"
#include "glib_trace.h"

#define LARGEALIGNMENT          (256)
#define P2ALIGNMENT             (2 * sizeof (gsize))
#define ALIGN(size, base)       ((base) * (gsize) (((size) + (base) - 1) / (base)))
#define NATIVE_MALLOC_PADDING   P2ALIGNMENT
#define SLAB_INFO_SIZE          P2ALIGN (sizeof (SlabInfo) + NATIVE_MALLOC_PADDING)
#define MAX_MAGAZINE_SIZE       (256)
#define MIN_MAGAZINE_SIZE       (4)
#define MAX_STAMP_COUNTER       (7)
#define MAX_SLAB_CHUNK_SIZE(al) (((al)->max_page_size - SLAB_INFO_SIZE) / 8)
#define MAX_SLAB_INDEX(al)      (SLAB_INDEX (al, MAX_SLAB_CHUNK_SIZE (al)) + 1)
#define SLAB_INDEX(al, asize)   ((asize) / P2ALIGNMENT - 1)
#define SLAB_CHUNK_SIZE(al, ix) (((ix) + 1) * P2ALIGNMENT)
#define SLAB_BPAGE_SIZE(al,csz) (8 * (csz) + SLAB_INFO_SIZE)
#if GLIB_SIZEOF_SIZE_T * 2 == 8
#define P2ALIGN(size)   (((size) + 0x7) & ~(gsize) 0x7)
#elif GLIB_SIZEOF_SIZE_T * 2 == 16
#define P2ALIGN(size)   (((size) + 0xf) & ~(gsize) 0xf)
#else
#define P2ALIGN(size)   ALIGN (size, P2ALIGNMENT)
#endif
static void mem_error (const char *format, ...) G_GNUC_PRINTF (1,2);
#define mem_assert(cond)    do { if (G_LIKELY (cond)) ; else mem_error ("assertion failed: %s", #cond); } while(0);
typedef struct _ChunkLink      ChunkLink;
typedef struct _SlabInfo       SlabInfo;
typedef struct _CachedMagazine CachedMagazine;
struct _ChunkLink {
  ChunkLink *next;
  ChunkLink *data;
};
struct _SlabInfo {
  ChunkLink *chunks;
  guint n_allocated;
  SlabInfo *next, *prev;
};
typedef struct {
  ChunkLink *chunks;
  gsize      count;
} Magazine;
typedef struct {
  Magazine   *magazine1;
  Magazine   *magazine2;
} ThreadMemory;
typedef struct {
  gboolean always_malloc;
  gboolean bypass_magazines;
  gboolean debug_blocks;
  gsize    working_set_msecs;
  guint    color_increment;
} SliceConfig;
typedef struct {
  gsize min_page_size, max_page_size;
  SliceConfig config;
  gsize max_slab_chunk_size_for_magazine_cache;
  GMutex *magazine_mutex;
  ChunkLink **magazines;
  guint *contention_counters;
  gint mutex_counter;
  guint stamp_counter;
  guint last_stamp;
  GMutex *slab_mutex;
  SlabInfo **slab_stack;
  guint color_accu;
} Allocator;
static gpointer slab_allocator_alloc_chunk(gsize chunk_size);
static void slab_allocator_free_chunk(gsize chunk_size, gpointer mem);
static void private_thread_memory_cleanup(gpointer data);
static gpointer allocator_memalign(gsize alignment, gsize memsize);
static void allocator_memfree(gsize memsize, gpointer mem);
static inline void magazine_cache_update_stamp(void);
static inline gsize allocator_get_magazine_threshold(Allocator *allocator, guint ix);
static void smc_notify_alloc(void *pointer, size_t size);
static int smc_notify_free(void *pointer, size_t size);
static GPrivate *private_thread_memory = NULL;
static gsize sys_page_size = 0;
static Allocator allocator[1] = { { 0, }, };
static SliceConfig slice_config = {
  FALSE,
  FALSE,
  FALSE,
  15 * 1000,
  1,
};
static GMutex *smc_tree_mutex = NULL;
void g_slice_set_config(GSliceConfig ckey, gint64 value) {
  g_return_if_fail(sys_page_size == 0);
  switch(ckey) {
      case G_SLICE_CONFIG_ALWAYS_MALLOC: slice_config.always_malloc = value != 0; break;
      case G_SLICE_CONFIG_BYPASS_MAGAZINES: slice_config.bypass_magazines = value != 0; break;
      case G_SLICE_CONFIG_WORKING_SET_MSECS: slice_config.working_set_msecs = value; break;
      case G_SLICE_CONFIG_COLOR_INCREMENT: slice_config.color_increment = value;
  }
}
gint64 g_slice_get_config(GSliceConfig ckey) {
  switch(ckey) {
      case G_SLICE_CONFIG_ALWAYS_MALLOC: return slice_config.always_malloc;
      case G_SLICE_CONFIG_BYPASS_MAGAZINES: return slice_config.bypass_magazines;
      case G_SLICE_CONFIG_WORKING_SET_MSECS: return slice_config.working_set_msecs;
      case G_SLICE_CONFIG_CHUNK_SIZES: return MAX_SLAB_INDEX (allocator);
      case G_SLICE_CONFIG_COLOR_INCREMENT: return slice_config.color_increment;
      default: return 0;
  }
}
gint64* g_slice_get_config_state(GSliceConfig ckey, gint64 address, guint *n_values) {
  guint i = 0;
  g_return_val_if_fail(n_values != NULL, NULL);
  *n_values = 0;
  gint64 array[64];
  switch(ckey) {
      case G_SLICE_CONFIG_CONTENTION_COUNTER:
          array[i++] = SLAB_CHUNK_SIZE(allocator, address);
          array[i++] = allocator->contention_counters[address];
          array[i++] = allocator_get_magazine_threshold(allocator, address);
          *n_values = i;
          return g_memdup(array, sizeof(array[0]) * *n_values);
      default: return NULL;
  }
}
static void slice_config_init(SliceConfig *config) {
  gchar buffer[1024];
  const gchar *val = _g_getenv_nomalloc ("G_SLICE", buffer);
  const GDebugKey keys[] = {
      { "always-malloc", 1 << 0 },
      { "debug-blocks",  1 << 1 },
  };
  gint flags = !val ? 0 : g_parse_debug_string (val, keys, G_N_ELEMENTS (keys));
  *config = slice_config;
  if (flags & (1 << 0)) config->always_malloc = TRUE;
  if (flags & (1 << 1)) config->debug_blocks = TRUE;
}
static void g_slice_init_nomessage(void) {
  mem_assert(sys_page_size == 0);
  mem_assert(MIN_MAGAZINE_SIZE >= 4);
#ifndef G_OS_WIN32
  {
      SYSTEM_INFO system_info;
      GetSystemInfo(&system_info);
      sys_page_size = system_info.dwPageSize;
  }
#else
  sys_page_size = sysconf(_SC_PAGESIZE);
#endif
  mem_assert(sys_page_size >= 2 * LARGEALIGNMENT);
  mem_assert((sys_page_size & (sys_page_size - 1)) == 0);
  slice_config_init(&allocator->config);
  allocator->min_page_size = sys_page_size;
#if HAVE_COMPLIANT_POSIX_MEMALIGN || HAVE_MEMALIGN
  allocator->min_page_size = MAX(allocator->min_page_size, 4096);
  allocator->max_page_size = MAX(allocator->min_page_size, 8192);
  allocator->min_page_size = MIN(allocator->min_page_size, 128);
#else
  allocator->max_page_size = sys_page_size;
#endif
  if (allocator->config.always_malloc) {
      allocator->contention_counters = NULL;
      allocator->magazines = NULL;
      allocator->slab_stack = NULL;
  } else {
      allocator->contention_counters = g_new0(guint, MAX_SLAB_INDEX (allocator));
      allocator->magazines = g_new0(ChunkLink*, MAX_SLAB_INDEX (allocator));
      allocator->slab_stack = g_new0(SlabInfo*, MAX_SLAB_INDEX (allocator));
  }
  allocator->magazine_mutex = NULL;
  allocator->mutex_counter = 0;
  allocator->stamp_counter = MAX_STAMP_COUNTER;
  allocator->last_stamp = 0;
  allocator->slab_mutex = NULL;
  allocator->color_accu = 0;
  magazine_cache_update_stamp();
  allocator->max_slab_chunk_size_for_magazine_cache = MAX_SLAB_CHUNK_SIZE(allocator);
  if (allocator->config.always_malloc || allocator->config.bypass_magazines) allocator->max_slab_chunk_size_for_magazine_cache = 0;
}
static inline guint allocator_categorize(gsize aligned_chunk_size) {
  if (G_LIKELY (aligned_chunk_size && aligned_chunk_size <= allocator->max_slab_chunk_size_for_magazine_cache)) return 1;
  if (!sys_page_size) g_slice_init_nomessage ();
  if (!allocator->config.always_malloc && aligned_chunk_size && aligned_chunk_size <= MAX_SLAB_CHUNK_SIZE (allocator)) {
      if (allocator->config.bypass_magazines) return 2;
      return 1;
  }
  return 0;
}
void _g_slice_thread_init_nomessage(void) {
  if (!sys_page_size) g_slice_init_nomessage();
  private_thread_memory = g_private_new(private_thread_memory_cleanup);
  allocator->magazine_mutex = g_mutex_new();
  allocator->slab_mutex = g_mutex_new();
  if (allocator->config.debug_blocks) smc_tree_mutex = g_mutex_new();
}
static inline void g_mutex_lock_a(GMutex *mutex, guint *contention_counter) {
  gboolean contention = TRUE;
  if (!g_mutex_trylock(mutex)) {
      g_mutex_lock(mutex);
      contention = TRUE;
  }
  if (contention) {
      allocator->mutex_counter++;
      if (allocator->mutex_counter >= 1) {
          allocator->mutex_counter = 0;
          *contention_counter = MIN(*contention_counter + 1, MAX_MAGAZINE_SIZE);
      }
  } else {
      allocator->mutex_counter--;
      if (allocator->mutex_counter < -11) {
          allocator->mutex_counter = 0;
          *contention_counter = MAX(*contention_counter, 1) - 1;
      }
  }
}
static inline ThreadMemory* thread_memory_from_self(void) {
  ThreadMemory *tmem = g_private_get(private_thread_memory);
  if (G_UNLIKELY(!tmem)) {
      static ThreadMemory *single_thread_memory = NULL;
      if (single_thread_memory && g_thread_supported()) {
          g_mutex_lock(allocator->slab_mutex);
          if (single_thread_memory) {
              tmem = single_thread_memory;
              single_thread_memory = NULL;
          }
          g_mutex_unlock(allocator->slab_mutex);
      }
      if (!tmem) {
          const guint n_magazines = MAX_SLAB_INDEX(allocator);
          tmem = g_malloc0(sizeof(ThreadMemory) + sizeof(Magazine) * 2 * n_magazines);
          tmem->magazine1 = (Magazine*)(tmem + 1);
          tmem->magazine2 = &tmem->magazine1[n_magazines];
	  }
      g_private_set(private_thread_memory, tmem);
      if (!single_thread_memory && !g_thread_supported()) single_thread_memory = tmem;
  }
  return tmem;
}
static inline ChunkLink* magazine_chain_pop_head(ChunkLink **magazine_chunks) {
  ChunkLink *chunk = (*magazine_chunks)->data;
  if (G_UNLIKELY (chunk)) (*magazine_chunks)->data = chunk->next;
  else {
      chunk = *magazine_chunks;
      *magazine_chunks = chunk->next;
  }
  return chunk;
}
#if 0
static guint magazine_count(ChunkLink *head) {
  guint count = 0;
  if (!head) return 0;
  while(head) {
      ChunkLink *child = head->data;
      count += 1;
      for (child = head->data; child; child = child->next) count += 1;
      head = head->next;
  }
  return count;
}
#endif
static inline gsize allocator_get_magazine_threshold(Allocator *allocator, guint ix) {
  gsize chunk_size = SLAB_CHUNK_SIZE(allocator, ix);
  guint threshold = MAX(MIN_MAGAZINE_SIZE, allocator->max_page_size / MAX(5 * chunk_size, 5 * 32));
  guint contention_counter = allocator->contention_counters[ix];
  if (G_UNLIKELY(contention_counter)) {
      contention_counter = contention_counter * 64 / chunk_size;
      threshold = MAX(threshold, contention_counter);
  }
  return threshold;
}
static inline void magazine_cache_update_stamp(void) {
  if (allocator->stamp_counter >= MAX_STAMP_COUNTER) {
      GTimeVal tv;
      g_get_current_time (&tv);
      allocator->last_stamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
      allocator->stamp_counter = 0;
  } else allocator->stamp_counter++;
}
static inline ChunkLink* magazine_chain_prepare_fields(ChunkLink *magazine_chunks) {
  ChunkLink *chunk1;
  ChunkLink *chunk2;
  ChunkLink *chunk3;
  ChunkLink *chunk4;
  chunk1 = magazine_chain_pop_head(&magazine_chunks);
  chunk2 = magazine_chain_pop_head(&magazine_chunks);
  chunk3 = magazine_chain_pop_head(&magazine_chunks);
  chunk4 = magazine_chain_pop_head(&magazine_chunks);
  chunk4->next = magazine_chunks;
  chunk3->next = chunk4;
  chunk2->next = chunk3;
  chunk1->next = chunk2;
  return chunk1;
}
#define magazine_chain_prev(mc)         ((mc)->data)
#define magazine_chain_stamp(mc)        ((mc)->next->data)
#define magazine_chain_uint_stamp(mc)   GPOINTER_TO_UINT ((mc)->next->data)
#define magazine_chain_next(mc)         ((mc)->next->next->data)
#define magazine_chain_count(mc)        ((mc)->next->next->next->data)
static void magazine_cache_trim(Allocator *allocator, guint ix, guint stamp) {
  ChunkLink *current = magazine_chain_prev(allocator->magazines[ix]);
  ChunkLink *trash = NULL;
  while(ABS(stamp - magazine_chain_uint_stamp(current)) >= allocator->config.working_set_msecs) {
      ChunkLink *prev = magazine_chain_prev(current);
      ChunkLink *next = magazine_chain_next(current);
      magazine_chain_next(prev) = next;
      magazine_chain_prev(next) = prev;
      magazine_chain_next(current) = NULL;
      magazine_chain_count(current) = NULL;
      magazine_chain_stamp(current) = NULL;
      magazine_chain_prev(current) = trash;
      trash = current;
      if (current == allocator->magazines[ix]) {
          allocator->magazines[ix] = NULL;
          break;
      }
      current = prev;
  }
  g_mutex_unlock(allocator->magazine_mutex);
  if (trash) {
      const gsize chunk_size = SLAB_CHUNK_SIZE (allocator, ix);
      g_mutex_lock(allocator->slab_mutex);
      while (trash) {
          current = trash;
          trash = magazine_chain_prev(current);
          magazine_chain_prev(current) = NULL;
          while(current) {
              ChunkLink *chunk = magazine_chain_pop_head(&current);
              slab_allocator_free_chunk(chunk_size, chunk);
          }
      }
  }
}
static void magazine_cache_push_magazine(guint ix, ChunkLink *magazine_chunks, gsize count) {
  ChunkLink *current = magazine_chain_prepare_fields(magazine_chunks);
  ChunkLink *next, *prev;
  g_mutex_lock(allocator->magazine_mutex);
  next = allocator->magazines[ix];
  if (next) prev = magazine_chain_prev(next);
  else next = prev = current;
  magazine_chain_next(prev) = current;
  magazine_chain_prev(next) = current;
  magazine_chain_prev(current) = prev;
  magazine_chain_next(current) = next;
  magazine_chain_count(current) = (gpointer)count;
  magazine_cache_update_stamp();
  magazine_chain_stamp(current) = GUINT_TO_POINTER(allocator->last_stamp);
  allocator->magazines[ix] = current;
  magazine_cache_trim(allocator, ix, allocator->last_stamp);
}
static ChunkLink* magazine_cache_pop_magazine(guint  ix, gsize *countp) {
  g_mutex_lock_a(allocator->magazine_mutex, &allocator->contention_counters[ix]);
  if (!allocator->magazines[ix]) {
      guint magazine_threshold = allocator_get_magazine_threshold (allocator, ix);
      gsize i, chunk_size = SLAB_CHUNK_SIZE (allocator, ix);
      ChunkLink *chunk, *head;
      g_mutex_unlock(allocator->magazine_mutex);
      g_mutex_lock(allocator->slab_mutex);
      head = slab_allocator_alloc_chunk(chunk_size);
      head->data = NULL;
      chunk = head;
      for (i = 1; i < magazine_threshold; i++) {
          chunk->next = slab_allocator_alloc_chunk(chunk_size);
          chunk = chunk->next;
          chunk->data = NULL;
      }
      chunk->next = NULL;
      g_mutex_unlock(allocator->slab_mutex);
      *countp = i;
      return head;
  } else {
      ChunkLink *current = allocator->magazines[ix];
      ChunkLink *prev = magazine_chain_prev(current);
      ChunkLink *next = magazine_chain_next(current);
      magazine_chain_next(prev) = next;
      magazine_chain_prev(next) = prev;
      allocator->magazines[ix] = next == current ? NULL : next;
      g_mutex_unlock(allocator->magazine_mutex);
      *countp = (gsize) magazine_chain_count(current);
      magazine_chain_prev(current) = NULL;
      magazine_chain_next(current) = NULL;
      magazine_chain_count(current) = NULL;
      magazine_chain_stamp(current) = NULL;
      return current;
  }
}
static void private_thread_memory_cleanup(gpointer data) {
  ThreadMemory *tmem = data;
  const guint n_magazines = MAX_SLAB_INDEX(allocator);
  guint ix;
  for (ix = 0; ix < n_magazines; ix++) {
      Magazine *mags[2];
      guint j;
      mags[0] = &tmem->magazine1[ix];
      mags[1] = &tmem->magazine2[ix];
      for (j = 0; j < 2; j++) {
          Magazine *mag = mags[j];
          if (mag->count >= MIN_MAGAZINE_SIZE) magazine_cache_push_magazine(ix, mag->chunks, mag->count);
          else {
              const gsize chunk_size = SLAB_CHUNK_SIZE (allocator, ix);
              g_mutex_lock(allocator->slab_mutex);
              while(mag->chunks) {
                  ChunkLink *chunk = magazine_chain_pop_head(&mag->chunks);
                  slab_allocator_free_chunk(chunk_size, chunk);
              }
              g_mutex_unlock(allocator->slab_mutex);
          }
      }
  }
  g_free(tmem);
}
static void thread_memory_magazine1_reload(ThreadMemory *tmem, guint ix) {
  Magazine *mag = &tmem->magazine1[ix];
  mem_assert(mag->chunks == NULL);
  mag->count = 0;
  mag->chunks = magazine_cache_pop_magazine(ix, &mag->count);
}
static void thread_memory_magazine2_unload(ThreadMemory *tmem, guint ix) {
  Magazine *mag = &tmem->magazine2[ix];
  magazine_cache_push_magazine(ix, mag->chunks, mag->count);
  mag->chunks = NULL;
  mag->count = 0;
}
static inline void thread_memory_swap_magazines(ThreadMemory *tmem, guint ix) {
  Magazine xmag = tmem->magazine1[ix];
  tmem->magazine1[ix] = tmem->magazine2[ix];
  tmem->magazine2[ix] = xmag;
}
static inline gboolean thread_memory_magazine1_is_empty(ThreadMemory *tmem, guint ix) {
  return tmem->magazine1[ix].chunks == NULL;
}
static inline gboolean thread_memory_magazine2_is_full(ThreadMemory *tmem, guint ix) {
  return tmem->magazine2[ix].count >= allocator_get_magazine_threshold(allocator, ix);
}
static inline gpointer thread_memory_magazine1_alloc(ThreadMemory *tmem, guint ix) {
  Magazine *mag = &tmem->magazine1[ix];
  ChunkLink *chunk = magazine_chain_pop_head(&mag->chunks);
  if (G_LIKELY(mag->count > 0)) mag->count--;
  return chunk;
}
static inline void thread_memory_magazine2_free(ThreadMemory *tmem, guint ix, gpointer mem) {
  Magazine *mag = &tmem->magazine2[ix];
  ChunkLink *chunk = mem;
  chunk->data = NULL;
  chunk->next = mag->chunks;
  mag->chunks = chunk;
  mag->count++;
}
gpointer g_slice_alloc(gsize mem_size) {
  gsize chunk_size;
  gpointer mem;
  guint acat;
  chunk_size = P2ALIGN(mem_size);
  acat = allocator_categorize(chunk_size);
  if (G_LIKELY(acat == 1)) {
      ThreadMemory *tmem = thread_memory_from_self();
      guint ix = SLAB_INDEX(allocator, chunk_size);
      if (G_UNLIKELY(thread_memory_magazine1_is_empty (tmem, ix))) {
          thread_memory_swap_magazines(tmem, ix);
          if (G_UNLIKELY(thread_memory_magazine1_is_empty(tmem, ix))) thread_memory_magazine1_reload(tmem, ix);
      }
      mem = thread_memory_magazine1_alloc(tmem, ix);
  } else if (acat == 2) {
      g_mutex_lock (allocator->slab_mutex);
      mem = slab_allocator_alloc_chunk(chunk_size);
      g_mutex_unlock (allocator->slab_mutex);
  } else mem = g_malloc(mem_size);
  if (G_UNLIKELY (allocator->config.debug_blocks)) smc_notify_alloc(mem, mem_size);
  TRACE(GLIB_SLICE_ALLOC((void*)mem, mem_size));
  return mem;
}
gpointer g_slice_alloc0(gsize mem_size) {
  gpointer mem = g_slice_alloc(mem_size);
  if (mem) memset(mem, 0, mem_size);
  return mem;
}
gpointer g_slice_copy(gsize mem_size, gconstpointer mem_block) {
  gpointer mem = g_slice_alloc(mem_size);
  if (mem) memcpy(mem, mem_block, mem_size);
  return mem;
}
void g_slice_free1(gsize mem_size, gpointer mem_block) {
  gsize chunk_size = P2ALIGN (mem_size);
  guint acat = allocator_categorize (chunk_size);
  if (G_UNLIKELY(!mem_block)) return;
  if (G_UNLIKELY(allocator->config.debug_blocks) && !smc_notify_free(mem_block, mem_size)) abort();
  if (G_LIKELY(acat == 1)) {
      ThreadMemory *tmem = thread_memory_from_self();
      guint ix = SLAB_INDEX(allocator, chunk_size);
      if (G_UNLIKELY(thread_memory_magazine2_is_full(tmem, ix))) {
          thread_memory_swap_magazines (tmem, ix);
          if (G_UNLIKELY(thread_memory_magazine2_is_full(tmem, ix))) thread_memory_magazine2_unload(tmem, ix);
      }
      if (G_UNLIKELY(g_mem_gc_friendly)) memset(mem_block, 0, chunk_size);
      thread_memory_magazine2_free(tmem, ix, mem_block);
  } else if (acat == 2) {
      if (G_UNLIKELY (g_mem_gc_friendly)) memset(mem_block, 0, chunk_size);
      g_mutex_lock(allocator->slab_mutex);
      slab_allocator_free_chunk(chunk_size, mem_block);
      g_mutex_unlock(allocator->slab_mutex);
  } else {
      if (G_UNLIKELY (g_mem_gc_friendly)) memset(mem_block, 0, mem_size);
      g_free(mem_block);
  }
  TRACE(GLIB_SLICE_FREE((void*)mem_block, mem_size));
}
void g_slice_free_chain_with_offset(gsize mem_size, gpointer mem_chain, gsize next_offset) {
  gpointer slice = mem_chain;
  gsize chunk_size = P2ALIGN(mem_size);
  guint acat = allocator_categorize(chunk_size);
  if (G_LIKELY(acat == 1)) {
      ThreadMemory *tmem = thread_memory_from_self();
      guint ix = SLAB_INDEX(allocator, chunk_size);
      while(slice) {
          guint8 *current = slice;
          slice = *(gpointer*)(current + next_offset);
          if (G_UNLIKELY(allocator->config.debug_blocks) && !smc_notify_free(current, mem_size)) abort();
          if (G_UNLIKELY(thread_memory_magazine2_is_full(tmem, ix))) {
              thread_memory_swap_magazines(tmem, ix);
              if (G_UNLIKELY(thread_memory_magazine2_is_full(tmem, ix))) thread_memory_magazine2_unload(tmem, ix);
          }
          if (G_UNLIKELY(g_mem_gc_friendly)) memset(current, 0, chunk_size);
          thread_memory_magazine2_free(tmem, ix, current);
      }
  } else if (acat == 2) {
      g_mutex_lock (allocator->slab_mutex);
      while(slice) {
          guint8 *current = slice;
          slice = *(gpointer*)(current + next_offset);
          if (G_UNLIKELY(allocator->config.debug_blocks) && !smc_notify_free(current, mem_size)) abort();
          if (G_UNLIKELY(g_mem_gc_friendly)) memset(current, 0, chunk_size);
          slab_allocator_free_chunk(chunk_size, current);
      }
      g_mutex_unlock(allocator->slab_mutex);
  } else {
      while (slice) {
          guint8 *current = slice;
          slice = *(gpointer*)(current + next_offset);
          if (G_UNLIKELY(allocator->config.debug_blocks) && !smc_notify_free(current, mem_size)) abort();
          if (G_UNLIKELY (g_mem_gc_friendly)) memset(current, 0, mem_size);
          g_free(current);
      }
  }
}
static void allocator_slab_stack_push(Allocator *allocator, guint ix, SlabInfo *sinfo) {
  if (!allocator->slab_stack[ix]) {
      sinfo->next = sinfo;
      sinfo->prev = sinfo;
  } else {
      SlabInfo *next = allocator->slab_stack[ix], *prev = next->prev;
      next->prev = sinfo;
      prev->next = sinfo;
      sinfo->next = next;
      sinfo->prev = prev;
    }
  allocator->slab_stack[ix] = sinfo;
}
static gsize allocator_aligned_page_size(Allocator *allocator, gsize n_bytes) {
  gsize val = 1 << g_bit_storage(n_bytes - 1);
  val = MAX(val, allocator->min_page_size);
  return val;
}
static void allocator_add_slab(Allocator *allocator, guint ix, gsize chunk_size) {
  ChunkLink *chunk;
  SlabInfo *sinfo;
  gsize addr, padding, n_chunks, color = 0;
  gsize page_size = allocator_aligned_page_size(allocator, SLAB_BPAGE_SIZE(allocator, chunk_size));
  gpointer aligned_memory = allocator_memalign(page_size, page_size - NATIVE_MALLOC_PADDING);
  guint8 *mem = aligned_memory;
  guint i;
  if (!mem) {
      const gchar *syserr = "unknown error";
#if HAVE_STRERROR
      syserr = strerror(errno);
#endif
      mem_error("failed to allocate %u bytes (alignment: %u): %s\n", (guint)(page_size - NATIVE_MALLOC_PADDING), (guint)page_size, syserr);
  }
  addr = ((gsize)mem / page_size) * page_size;
  mem_assert(aligned_memory == (gpointer)addr);
  sinfo = (SlabInfo*)(mem + page_size - SLAB_INFO_SIZE);
  sinfo->n_allocated = 0;
  sinfo->chunks = NULL;
  n_chunks = ((guint8*)sinfo - mem) / chunk_size;
  padding = ((guint8*)sinfo - mem) - n_chunks * chunk_size;
  if (padding) {
      color = (allocator->color_accu * P2ALIGNMENT) % padding;
      allocator->color_accu += allocator->config.color_increment;
  }
  chunk = (ChunkLink*)(mem + color);
  sinfo->chunks = chunk;
  for (i = 0; i < n_chunks - 1; i++) {
      chunk->next = (ChunkLink*)((guint8*)chunk + chunk_size);
      chunk = chunk->next;
  }
  chunk->next = NULL;
  allocator_slab_stack_push(allocator, ix, sinfo);
}
static gpointer slab_allocator_alloc_chunk(gsize chunk_size) {
  ChunkLink *chunk;
  guint ix = SLAB_INDEX (allocator, chunk_size);
  if (!allocator->slab_stack[ix] || !allocator->slab_stack[ix]->chunks) allocator_add_slab(allocator, ix, chunk_size);
  chunk = allocator->slab_stack[ix]->chunks;
  allocator->slab_stack[ix]->chunks = chunk->next;
  allocator->slab_stack[ix]->n_allocated++;
  if (!allocator->slab_stack[ix]->chunks) allocator->slab_stack[ix] = allocator->slab_stack[ix]->next;
  return chunk;
}
static void slab_allocator_free_chunk(gsize chunk_size, gpointer mem) {
  ChunkLink *chunk;
  gboolean was_empty;
  guint ix = SLAB_INDEX(allocator, chunk_size);
  gsize page_size = allocator_aligned_page_size(allocator, SLAB_BPAGE_SIZE(allocator, chunk_size));
  gsize addr = ((gsize)mem / page_size) * page_size;
  guint8 *page = (guint8*)addr;
  SlabInfo *sinfo = (SlabInfo*)(page + page_size - SLAB_INFO_SIZE);
  mem_assert(sinfo->n_allocated > 0);
  was_empty = sinfo->chunks == NULL;
  chunk = (ChunkLink*)mem;
  chunk->next = sinfo->chunks;
  sinfo->chunks = chunk;
  sinfo->n_allocated--;
  if (was_empty) {
      SlabInfo *next = sinfo->next, *prev = sinfo->prev;
      next->prev = prev;
      prev->next = next;
      if (allocator->slab_stack[ix] == sinfo) allocator->slab_stack[ix] = next == sinfo ? NULL : next;
      allocator_slab_stack_push(allocator, ix, sinfo);
  }
  if (!sinfo->n_allocated) {
      SlabInfo *next = sinfo->next, *prev = sinfo->prev;
      next->prev = prev;
      prev->next = next;
      if (allocator->slab_stack[ix] == sinfo) allocator->slab_stack[ix] = next == sinfo ? NULL : next;
      allocator_memfree (page_size, page);
  }
}
#include <malloc.h>
#if !(HAVE_COMPLIANT_POSIX_MEMALIGN || HAVE_MEMALIGN || HAVE_VALLOC)
static GTrashStack *compat_valloc_trash = NULL;
#endif
static gpointer allocator_memalign(gsize alignment, gsize memsize) {
  gpointer aligned_memory = NULL;
  gint err = ENOMEM;
#if HAVE_COMPLIANT_POSIX_MEMALIGN
  err = posix_memalign (&aligned_memory, alignment, memsize);
#elif HAVE_MEMALIGN
  errno = 0;
  aligned_memory = memalign (alignment, memsize);
  err = errno;
#elif HAVE_VALLOC
  errno = 0;
  aligned_memory = valloc (memsize);
  err = errno;
#else
  mem_assert(alignment == sys_page_size);
  mem_assert(memsize <= sys_page_size);
  if (!compat_valloc_trash) {
      const guint n_pages = 16;
      guint8 *mem = malloc(n_pages * sys_page_size);
      err = errno;
      if (mem) {
          gint i = n_pages;
          guint8 *amem = (guint8*)ALIGN((gsize)mem, sys_page_size);
          if (amem != mem) i--;
          while(--i >= 0) g_trash_stack_push(&compat_valloc_trash, amem + i * sys_page_size);
      }
  }
  aligned_memory = g_trash_stack_pop(&compat_valloc_trash);
#endif
  if (!aligned_memory) errno = err;
  return aligned_memory;
}
static void allocator_memfree(gsize memsize, gpointer mem) {
#if HAVE_COMPLIANT_POSIX_MEMALIGN || HAVE_MEMALIGN || HAVE_VALLOC
  free(mem);
#else
  mem_assert(memsize <= sys_page_size);
  g_trash_stack_push(&compat_valloc_trash, mem);
#endif
}
static void mem_error(const char *format, ...) {
  const char *pname;
  va_list args;
  fputs("\n***MEMORY-ERROR***: ", stderr);
  pname = g_get_prgname();
  fprintf(stderr, "%s[%ld]: GSlice: ", pname ? pname : "", (long)getpid());
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);
  abort();
  _exit(1);
}
typedef size_t SmcKType;
typedef size_t SmcVType;
typedef struct {
  SmcKType key;
  SmcVType value;
} SmcEntry;
static void smc_tree_insert(SmcKType  key, SmcVType  value);
static gboolean smc_tree_lookup(SmcKType  key, SmcVType *value_p);
static gboolean smc_tree_remove(SmcKType  key);
static void smc_notify_alloc(void *pointer, size_t size) {
  size_t adress = (size_t)pointer;
  if (pointer) smc_tree_insert(adress, size);
}
#if 0
static void smc_notify_ignore(void *pointer) {
  size_t adress = (size_t)pointer;
  if (pointer )smc_tree_remove(adress);
}
#endif
static int smc_notify_free(void *pointer, size_t size) {
  size_t adress = (size_t) pointer;
  SmcVType real_size;
  gboolean found_one;
  if (!pointer) return 1;
  found_one = smc_tree_lookup(adress, &real_size);
  if (!found_one) {
      fprintf(stderr, "GSlice: MemChecker: attempt to release non-allocated block: %p size=%" G_GSIZE_FORMAT "\n", pointer, size);
      return 0;
  }
  if (real_size != size && (real_size || size)) {
      fprintf(stderr, "GSlice: MemChecker: attempt to release block with invalid size: %p size=%" G_GSIZE_FORMAT " invalid-size=%" G_GSIZE_FORMAT "\n",
              pointer, real_size, size);
      return 0;
  }
  if (!smc_tree_remove(adress)) {
      fprintf(stderr, "GSlice: MemChecker: attempt to release non-allocated block: %p size=%" G_GSIZE_FORMAT "\n", pointer, size);
      return 0;
  }
  return 1;
}
#define SMC_TRUNK_COUNT     (4093)
#define SMC_BRANCH_COUNT    (511)
#define SMC_TRUNK_EXTENT    (SMC_BRANCH_COUNT * 2039)
#define SMC_TRUNK_HASH(k)   ((k / SMC_TRUNK_EXTENT) % SMC_TRUNK_COUNT)
#define SMC_BRANCH_HASH(k)  (k % SMC_BRANCH_COUNT)
typedef struct {
  SmcEntry *entries;
  unsigned int n_entries;
} SmcBranch;
static SmcBranch **smc_tree_root = NULL;
static void smc_tree_abort(int errval) {
  const char *syserr = "unknown error";
#if HAVE_STRERROR
  syserr = strerror(errval);
#endif
  mem_error("MemChecker: failure in debugging tree: %s", syserr);
}
static inline SmcEntry* smc_tree_branch_grow_L(SmcBranch *branch, unsigned int index) {
  unsigned int old_size = branch->n_entries * sizeof(branch->entries[0]);
  unsigned int new_size = old_size + sizeof(branch->entries[0]);
  SmcEntry *entry;
  mem_assert(index <= branch->n_entries);
  branch->entries = (SmcEntry*)realloc(branch->entries, new_size);
  if (!branch->entries) smc_tree_abort(errno);
  entry = branch->entries + index;
  g_memmove(entry + 1, entry, (branch->n_entries - index) * sizeof(entry[0]));
  branch->n_entries += 1;
  return entry;
}
static inline SmcEntry* smc_tree_branch_lookup_nearest_L(SmcBranch *branch, SmcKType key) {
  unsigned int n_nodes = branch->n_entries, offs = 0;
  SmcEntry *check = branch->entries;
  int cmp = 0;
  while(offs < n_nodes) {
      unsigned int i = (offs + n_nodes) >> 1;
      check = branch->entries + i;
      cmp = key < check->key ? -1 : key != check->key;
      if (cmp == 0) return check;
      else if (cmp < 0) n_nodes = i;
      else offs = i + 1;
  }
  return cmp > 0 ? check + 1 : check;
}
static void smc_tree_insert(SmcKType key, SmcVType value) {
  unsigned int ix0, ix1;
  SmcEntry *entry;
  g_mutex_lock(smc_tree_mutex);
  ix0 = SMC_TRUNK_HASH (key);
  ix1 = SMC_BRANCH_HASH (key);
  if (!smc_tree_root) {
      smc_tree_root = calloc(SMC_TRUNK_COUNT, sizeof(smc_tree_root[0]));
      if (!smc_tree_root) smc_tree_abort(errno);
  }
  if (!smc_tree_root[ix0]) {
      smc_tree_root[ix0] = calloc(SMC_BRANCH_COUNT, sizeof(smc_tree_root[0][0]));
      if (!smc_tree_root[ix0]) smc_tree_abort(errno);
  }
  entry = smc_tree_branch_lookup_nearest_L(&smc_tree_root[ix0][ix1], key);
  if (!entry || entry >= smc_tree_root[ix0][ix1].entries + smc_tree_root[ix0][ix1].n_entries || entry->key != key)
      entry = smc_tree_branch_grow_L(&smc_tree_root[ix0][ix1], entry - smc_tree_root[ix0][ix1].entries);
  entry->key = key;
  entry->value = value;
  g_mutex_unlock(smc_tree_mutex);
}
static gboolean smc_tree_lookup(SmcKType  key, SmcVType *value_p) {
  SmcEntry *entry = NULL;
  unsigned int ix0 = SMC_TRUNK_HASH (key), ix1 = SMC_BRANCH_HASH (key);
  gboolean found_one = FALSE;
  *value_p = 0;
  g_mutex_lock(smc_tree_mutex);
  if (smc_tree_root && smc_tree_root[ix0]) {
      entry = smc_tree_branch_lookup_nearest_L(&smc_tree_root[ix0][ix1], key);
      if (entry && entry < smc_tree_root[ix0][ix1].entries + smc_tree_root[ix0][ix1].n_entries && entry->key == key) {
          found_one = TRUE;
          *value_p = entry->value;
      }
  }
  g_mutex_unlock(smc_tree_mutex);
  return found_one;
}
static gboolean smc_tree_remove(SmcKType key) {
  unsigned int ix0 = SMC_TRUNK_HASH(key), ix1 = SMC_BRANCH_HASH(key);
  gboolean found_one = FALSE;
  g_mutex_lock(smc_tree_mutex);
  if (smc_tree_root && smc_tree_root[ix0]) {
      SmcEntry *entry = smc_tree_branch_lookup_nearest_L(&smc_tree_root[ix0][ix1], key);
      if (entry && entry < smc_tree_root[ix0][ix1].entries + smc_tree_root[ix0][ix1].n_entries && entry->key == key) {
          unsigned int i = entry - smc_tree_root[ix0][ix1].entries;
          smc_tree_root[ix0][ix1].n_entries -= 1;
          g_memmove (entry, entry + 1, (smc_tree_root[ix0][ix1].n_entries - i) * sizeof (entry[0]));
          if (!smc_tree_root[ix0][ix1].n_entries) {
              free (smc_tree_root[ix0][ix1].entries);
              smc_tree_root[ix0][ix1].entries = NULL;
          }
          found_one = TRUE;
      }
  }
  g_mutex_unlock (smc_tree_mutex);
  return found_one;
}
#ifdef G_ENABLE_DEBUG
void g_slice_debug_tree_statistics(void) {
  g_mutex_lock(smc_tree_mutex);
  if (smc_tree_root) {
      unsigned int i, j, t = 0, o = 0, b = 0, su = 0, ex = 0, en = 4294967295u;
      double tf, bf;
      for (i = 0; i < SMC_TRUNK_COUNT; i++)
          if (smc_tree_root[i]) {
              t++;
              for (j = 0; j < SMC_BRANCH_COUNT; j++)
                  if (smc_tree_root[i][j].n_entries) {
                      b++;
                      su += smc_tree_root[i][j].n_entries;
                      en = MIN(en, smc_tree_root[i][j].n_entries);
                      ex = MAX(ex, smc_tree_root[i][j].n_entries);
                  } else if (smc_tree_root[i][j].entries) o++;
          }
      en = b ? en : 0;
      tf = MAX (t, 1.0);
      bf = MAX (b, 1.0);
      fprintf(stderr, "GSlice: MemChecker: %u trunks, %u branches, %u old branches\n", t, b, o);
      fprintf(stderr, "GSlice: MemChecker: %f branches per trunk, %.2f%% utilization\n", b / tf, 100.0 - (SMC_BRANCH_COUNT - b / tf) / (0.01 * SMC_BRANCH_COUNT));
      fprintf(stderr, "GSlice: MemChecker: %f entries per branch, %u minimum, %u maximum\n", su / bf, en, ex);
  } else fprintf(stderr, "GSlice: MemChecker: root=NULL\n");
  g_mutex_unlock(smc_tree_mutex);
}
#endif