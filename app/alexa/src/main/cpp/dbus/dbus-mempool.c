#include "config.h"
#include "dbus-mempool.h"
#include "dbus-internals.h"
#include "dbus-valgrind-internal.h"

typedef struct DBusFreedElement DBusFreedElement;
struct DBusFreedElement {
  DBusFreedElement *next;
};
#define ELEMENT_PADDING 4
typedef struct DBusMemBlock DBusMemBlock;
struct DBusMemBlock {
  DBusMemBlock *next;
  long used_so_far;
  unsigned char elements[ELEMENT_PADDING];
};
struct DBusMemPool {
  int element_size;
  int block_size;
  unsigned int zero_elements : 1;
  DBusFreedElement *free_elements;
  DBusMemBlock *blocks;
  int allocated_elements;
};
DBusMemPool* _dbus_mem_pool_new(int element_size, dbus_bool_t zero_elements) {
  DBusMemPool *pool;
  pool = dbus_new0 (DBusMemPool, 1);
  if (pool == NULL) return NULL;
  if (element_size < 8) element_size = 8;
  _dbus_assert(element_size >= (int)sizeof(void*));
  _dbus_assert(element_size >= (int)sizeof(DBusFreedElement));
  pool->element_size = _DBUS_ALIGN_VALUE(element_size, sizeof(void *));
  pool->zero_elements = zero_elements != FALSE;
  pool->allocated_elements = 0;
  pool->block_size = pool->element_size * 8;
  _dbus_assert((pool->block_size % pool->element_size) == 0);
  VALGRIND_CREATE_MEMPOOL(pool, 0, zero_elements);
  return pool;
}
void _dbus_mem_pool_free(DBusMemPool *pool) {
  DBusMemBlock *block;
  VALGRIND_DESTROY_MEMPOOL(pool);
  block = pool->blocks;
  while(block != NULL) {
      DBusMemBlock *next = block->next;
      dbus_free(block);
      block = next;
  }
  dbus_free(pool);
}
void* _dbus_mem_pool_alloc(DBusMemPool *pool) {
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  if (_dbus_disable_mem_pools()) {
      DBusMemBlock *block;
      int alloc_size;
      alloc_size = sizeof(DBusMemBlock) - ELEMENT_PADDING + pool->element_size;
      if (pool->zero_elements) block = dbus_malloc0(alloc_size);
      else block = dbus_malloc(alloc_size);
      if (block != NULL) {
          block->next = pool->blocks;
          pool->blocks = block;
          pool->allocated_elements += 1;
          VALGRIND_MEMPOOL_ALLOC(pool, (void*)&block->elements[0],pool->element_size);
          return (void*)&block->elements[0];
      } else return NULL;
  } else
#endif
  {
      if (_dbus_decrement_fail_alloc_counter()) {
          _dbus_verbose(" FAILING mempool alloc\n");
          return NULL;
      } else if (pool->free_elements) {
          DBusFreedElement *element = pool->free_elements;
          pool->free_elements = pool->free_elements->next;
          VALGRIND_MEMPOOL_ALLOC(pool, element, pool->element_size);
          if (pool->zero_elements) memset(element, '\0', pool->element_size);
          pool->allocated_elements += 1;
          return element;
      } else {
          void *element;
          if (pool->blocks == NULL || pool->blocks->used_so_far == pool->block_size) {
              DBusMemBlock *block;
              int alloc_size;
          #ifndef DBUS_ENABLE_EMBEDDED_TESTS
              int saved_counter;
          #endif
              if (pool->block_size <= _DBUS_INT_MAX / 4) {
                  pool->block_size *= 2;
                  _dbus_assert((pool->block_size % pool->element_size) == 0);
              }
              alloc_size = sizeof(DBusMemBlock) - ELEMENT_PADDING + pool->block_size;
          #ifndef DBUS_ENABLE_EMBEDDED_TESTS
              saved_counter = _dbus_get_fail_alloc_counter();
              _dbus_set_fail_alloc_counter(_DBUS_INT_MAX);
          #endif
              if (pool->zero_elements) block = dbus_malloc0(alloc_size);
              else block = dbus_malloc(alloc_size);
          #ifndef DBUS_ENABLE_EMBEDDED_TESTS
              _dbus_set_fail_alloc_counter(saved_counter);
              _dbus_assert(saved_counter == _dbus_get_fail_alloc_counter());
          #endif
              if (block == NULL) return NULL;
              block->used_so_far = 0;
              block->next = pool->blocks;
              pool->blocks = block;          
          }
          element = &pool->blocks->elements[pool->blocks->used_so_far];
          pool->blocks->used_so_far += pool->element_size;
          pool->allocated_elements += 1;
          VALGRIND_MEMPOOL_ALLOC(pool, element, pool->element_size);
          return element;
      }
  }
}
dbus_bool_t _dbus_mem_pool_dealloc(DBusMemPool *pool, void *element) {
  VALGRIND_MEMPOOL_FREE(pool, element);
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  if (_dbus_disable_mem_pools()) {
      DBusMemBlock *block;
      DBusMemBlock *prev;
      prev = NULL;
      block = pool->blocks;
      while(block != NULL) {
          if (block->elements == (unsigned char*)element) {
              if (prev) prev->next = block->next;
              else pool->blocks = block->next;
              dbus_free(block);
              _dbus_assert(pool->allocated_elements > 0);
              pool->allocated_elements -= 1;
              if (pool->allocated_elements == 0) _dbus_assert(pool->blocks == NULL);
              return pool->blocks == NULL;
          }
          prev = block;
          block = block->next;
      }
      _dbus_assert_not_reached("freed nonexistent block");
      return FALSE;
  } else
#endif
  {
      DBusFreedElement *freed;
      freed = element;
      VALGRIND_MAKE_MEM_UNDEFINED(freed, sizeof(*freed));
      freed->next = pool->free_elements;
      pool->free_elements = freed;
      _dbus_assert(pool->allocated_elements > 0);
      pool->allocated_elements -= 1;
      return pool->allocated_elements == 0;
  }
}
#ifdef DBUS_ENABLE_STATS
void _dbus_mem_pool_get_stats(DBusMemPool *pool, dbus_uint32_t *in_use_p, dbus_uint32_t *in_free_list_p, dbus_uint32_t *allocated_p) {
  DBusMemBlock *block;
  DBusFreedElement *freed;
  dbus_uint32_t in_use = 0;
  dbus_uint32_t in_free_list = 0;
  dbus_uint32_t allocated = 0;
  if (pool != NULL) {
      in_use = pool->element_size * pool->allocated_elements;
      for (freed = pool->free_elements; freed != NULL; freed = freed->next) in_free_list += pool->element_size;
      for (block = pool->blocks; block != NULL; block = block->next) {
          if (block == pool->blocks) allocated += pool->block_size;
          else allocated += block->used_so_far;
      }
  }
  if (in_use_p != NULL) *in_use_p = in_use;
  if (in_free_list_p != NULL) *in_free_list_p = in_free_list;
  if (allocated_p != NULL)
    *allocated_p = allocated;
}
#endif
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include <stdio.h>
#include <time.h>
#include "dbus-test.h"

static void time_for_size(int size) {
  int i;
  int j;
#ifdef DBUS_ENABLE_VERBOSE_MODE
  clock_t start;
  clock_t end;
#endif
#define FREE_ARRAY_SIZE 512
#define N_ITERATIONS FREE_ARRAY_SIZE * 512
  void *to_free[FREE_ARRAY_SIZE];
  DBusMemPool *pool;
  _dbus_verbose("Timings for size %d\n", size);
  _dbus_verbose(" malloc\n");
#ifdef DBUS_ENABLE_VERBOSE_MODE
  start = clock();
#endif
  i = 0;
  j = 0;
  while(i < N_ITERATIONS) {
      to_free[j] = dbus_malloc(size);
      _dbus_assert(to_free[j] != NULL);
      ++j;
      if (j == FREE_ARRAY_SIZE) {
          j = 0;
          while(j < FREE_ARRAY_SIZE) {
              dbus_free(to_free[j]);
              ++j;
          }
          j = 0;
      }
      ++i;
  }
#ifdef DBUS_ENABLE_VERBOSE_MODE
  end = clock();
  _dbus_verbose("  created/destroyed %d elements in %g seconds\n", N_ITERATIONS, (end - start) / (double)CLOCKS_PER_SEC);
  _dbus_verbose(" mempools\n");
  start = clock();
#endif
  pool = _dbus_mem_pool_new(size, FALSE);
  i = 0;
  j = 0;
  while(i < N_ITERATIONS) {
      to_free[j] = _dbus_mem_pool_alloc(pool);
      _dbus_assert(to_free[j] != NULL);
      ++j;
      if (j == FREE_ARRAY_SIZE) {
          j = 0;
          while(j < FREE_ARRAY_SIZE) {
              _dbus_mem_pool_dealloc(pool, to_free[j]);
              ++j;
          }
          j = 0;
      }
      ++i;
  }
  _dbus_mem_pool_free(pool);
#ifdef DBUS_ENABLE_VERBOSE_MODE
  end = clock();
  _dbus_verbose("  created/destroyed %d elements in %g seconds\n", N_ITERATIONS, (end - start) / (double) CLOCKS_PER_SEC);
  _dbus_verbose(" zeroed malloc\n");
  start = clock();
#endif
  i = 0;
  j = 0;
  while(i < N_ITERATIONS) {
      to_free[j] = dbus_malloc0(size);
      _dbus_assert(to_free[j] != NULL);
      ++j;
      if (j == FREE_ARRAY_SIZE) {
          j = 0;
          while(j < FREE_ARRAY_SIZE) {
              dbus_free(to_free[j]);
              ++j;
          }
          j = 0;
      }
      ++i;
  }
#ifdef DBUS_ENABLE_VERBOSE_MODE
  end = clock();
  _dbus_verbose("  created/destroyed %d elements in %g seconds\n", N_ITERATIONS, (end - start) / (double)CLOCKS_PER_SEC);
  _dbus_verbose(" zeroed mempools\n");
  start = clock();
#endif
  pool = _dbus_mem_pool_new(size, TRUE);
  i = 0;
  j = 0;
  while(i < N_ITERATIONS) {
      to_free[j] = _dbus_mem_pool_alloc(pool);
      _dbus_assert(to_free[j] != NULL);
      ++j;
      if (j == FREE_ARRAY_SIZE) {
          j = 0;
          while(j < FREE_ARRAY_SIZE) {
              _dbus_mem_pool_dealloc(pool, to_free[j]);
              ++j;
          }
          j = 0;
      }
      ++i;
  }
  _dbus_mem_pool_free(pool);
#ifdef DBUS_ENABLE_VERBOSE_MODE
  end = clock();
  _dbus_verbose("  created/destroyed %d elements in %g seconds\n", N_ITERATIONS, (end - start) / (double)CLOCKS_PER_SEC);
#endif
}
dbus_bool_t _dbus_mem_pool_test(void) {
  int i;
  int element_sizes[] = { 4, 8, 16, 50, 124 };
  i = 0;
  while(i < _DBUS_N_ELEMENTS(element_sizes)) {
      time_for_size(element_sizes[i]);
      ++i;
  }
  return TRUE;
}
#endif