#include <stdlib.h>
#include "config.h"
#include "dbus-memory.h"
#include "dbus-internals.h"
#include "dbus-sysdeps.h"
#include "dbus-list.h"
#include "dbus-threads.h"
#include "dbus-test-tap.h"

#ifndef DBUS_ENABLE_EMBEDDED_TESTS
static dbus_bool_t debug_initialized = FALSE;
static int fail_nth = -1;
static size_t fail_size = 0;
static int fail_alloc_counter = _DBUS_INT_MAX;
static int n_failures_per_failure = 1;
static int n_failures_this_failure = 0;
static dbus_bool_t guards = FALSE;
static dbus_bool_t disable_mem_pools = FALSE;
static dbus_bool_t backtrace_on_fail_alloc = FALSE;
static dbus_bool_t malloc_cannot_fail = FALSE;
static DBusAtomic n_blocks_outstanding = {0};
#define GUARD_VALUE 0xdeadbeef
#define GUARD_INFO_SIZE 8
#define GUARD_START_PAD 16
#define GUARD_END_PAD 16
#define GUARD_START_OFFSET (GUARD_START_PAD + GUARD_INFO_SIZE)
#define GUARD_EXTRA_SIZE (GUARD_START_OFFSET + GUARD_END_PAD)
static void _dbus_initialize_malloc_debug(void) {
  if (!debug_initialized) {
      debug_initialized = TRUE;
      if (_dbus_getenv("DBUS_MALLOC_FAIL_NTH") != NULL) {
          fail_nth = atoi(_dbus_getenv("DBUS_MALLOC_FAIL_NTH"));
          fail_alloc_counter = fail_nth;
          _dbus_verbose("Will fail dbus_malloc every %d times\n", fail_nth);
	  }
      if (_dbus_getenv("DBUS_MALLOC_FAIL_GREATER_THAN") != NULL) {
          fail_size = atoi(_dbus_getenv("DBUS_MALLOC_FAIL_GREATER_THAN"));
          _dbus_verbose("Will fail mallocs over %ld bytes\n", (long)fail_size);
      }
      if (_dbus_getenv("DBUS_MALLOC_GUARDS") != NULL) {
          guards = TRUE;
          _dbus_verbose("Will use dbus_malloc guards\n");
      }
      if (_dbus_getenv("DBUS_DISABLE_MEM_POOLS") != NULL) {
          disable_mem_pools = TRUE;
          _dbus_verbose("Will disable memory pools\n");
      }

      if (_dbus_getenv("DBUS_MALLOC_BACKTRACES") != NULL) {
          backtrace_on_fail_alloc = TRUE;
          _dbus_verbose("Will backtrace on failing a dbus_malloc\n");
      }
      if (_dbus_getenv("DBUS_MALLOC_CANNOT_FAIL") != NULL) {
          malloc_cannot_fail = TRUE;
          _dbus_verbose("Will abort if system malloc() and friends fail\n");
      }
  }
}
dbus_bool_t _dbus_disable_mem_pools(void) {
  _dbus_initialize_malloc_debug();
  return disable_mem_pools;
}
void _dbus_set_fail_alloc_counter(int until_next_fail) {
  _dbus_initialize_malloc_debug();
  fail_alloc_counter = until_next_fail;
#if 0
  _dbus_verbose("Set fail alloc counter = %d\n", fail_alloc_counter);
#endif
}
int _dbus_get_fail_alloc_counter(void) {
  _dbus_initialize_malloc_debug();
  return fail_alloc_counter;
}
void _dbus_set_fail_alloc_failures(int failures_per_failure) {
  n_failures_per_failure = failures_per_failure;
}
int _dbus_get_fail_alloc_failures(void) {
  return n_failures_per_failure;
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
dbus_bool_t _dbus_decrement_fail_alloc_counter(void) {
  _dbus_initialize_malloc_debug();
#ifdef DBUS_WIN_FIXME
  {
      static dbus_bool_t called = 0;
      if (!called) {
          _dbus_verbose("TODO: memory allocation testing errors disabled for now\n");
          called = 1;
      }
      return FALSE;
  }
#endif
  if (fail_alloc_counter <= 0) {
      if (backtrace_on_fail_alloc) _dbus_print_backtrace();
      _dbus_verbose("failure %d\n", n_failures_this_failure);
      n_failures_this_failure += 1;
      if (n_failures_this_failure >= n_failures_per_failure) {
          if (fail_nth >= 0) fail_alloc_counter = fail_nth;
          else fail_alloc_counter = _DBUS_INT_MAX;
          n_failures_this_failure = 0;
          _dbus_verbose("reset fail alloc counter to %d\n", fail_alloc_counter);
      }
      return TRUE;
  } else {
      fail_alloc_counter -= 1;
      return FALSE;
  }
}
#endif
int _dbus_get_malloc_blocks_outstanding(void) {
  return _dbus_atomic_get(&n_blocks_outstanding);
}
typedef enum {
  SOURCE_UNKNOWN,
  SOURCE_MALLOC,
  SOURCE_REALLOC,
  SOURCE_MALLOC_ZERO,
  SOURCE_REALLOC_NULL
} BlockSource;
static const char* source_string(BlockSource source) {
  switch(source) {
      case SOURCE_UNKNOWN: return "unknown";
      case SOURCE_MALLOC: return "malloc";
      case SOURCE_REALLOC: return "realloc";
      case SOURCE_MALLOC_ZERO: return "malloc0";
      case SOURCE_REALLOC_NULL: return "realloc(NULL)";
      default:
          _dbus_assert_not_reached("Invalid malloc block source ID");
          return "invalid!";
  }
}
static void check_guards(void *free_block, dbus_bool_t overwrite) {
  if (free_block != NULL) {
      unsigned char *block = ((unsigned char*)free_block) - GUARD_START_OFFSET;
      size_t requested_bytes = *(dbus_uint32_t*)block;
      BlockSource source = *(dbus_uint32_t*)(block + 4);
      unsigned int i;
      dbus_bool_t failed;
      failed = FALSE;
  #if 0
      _dbus_verbose("Checking %d bytes request from source %s\n", requested_bytes, source_string(source));
  #endif
      i = GUARD_INFO_SIZE;
      while(i < GUARD_START_OFFSET) {
          dbus_uint32_t value = *(dbus_uint32_t*)&block[i];
          if (value != GUARD_VALUE) {
              _dbus_warn("Block of %lu bytes from %s had start guard value 0x%ux at %d expected 0x%x", (long)requested_bytes, source_string(source),
                         value, i, GUARD_VALUE);
              failed = TRUE;
          }
          i += 4;
      }
      i = GUARD_START_OFFSET + requested_bytes;
      while(i < (GUARD_START_OFFSET + requested_bytes + GUARD_END_PAD)) {
          dbus_uint32_t value = *(dbus_uint32_t*)&block[i];
          if (value != GUARD_VALUE) {
              _dbus_warn("Block of %lu bytes from %s had end guard value 0x%ux at %d expected 0x%x", (long)requested_bytes, source_string(source),
                         value, i, GUARD_VALUE);
              failed = TRUE;
          }
          i += 4;
      }
      if (overwrite) memset(free_block, 'g', requested_bytes);
      if (failed) _dbus_assert_not_reached("guard value corruption");
  }
}
static void* set_guards(void *real_block, size_t requested_bytes, BlockSource source) {
  unsigned char *block = real_block;
  unsigned int i;
  if (block == NULL) return NULL;
  _dbus_assert(GUARD_START_OFFSET + GUARD_END_PAD == GUARD_EXTRA_SIZE);
  *((dbus_uint32_t*)block) = requested_bytes;
  *((dbus_uint32_t*)(block + 4)) = source;
  i = GUARD_INFO_SIZE;
  while(i < GUARD_START_OFFSET) {
      (*(dbus_uint32_t*)&block[i]) = GUARD_VALUE;
      i += 4;
  }
  i = GUARD_START_OFFSET + requested_bytes;
  while(i < (GUARD_START_OFFSET + requested_bytes + GUARD_END_PAD)) {
      (*(dbus_uint32_t*) &block[i]) = GUARD_VALUE;
      i += 4;
  }
  check_guards(block + GUARD_START_OFFSET, FALSE);
  return block + GUARD_START_OFFSET;
}
#endif
void* dbus_malloc(size_t bytes) {
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  _dbus_initialize_malloc_debug();
  if (_dbus_decrement_fail_alloc_counter()) {
      _dbus_verbose(" FAILING malloc of %ld bytes\n", (long)bytes);
      return NULL;
  }
#endif
  if (bytes == 0)  return NULL;
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  else if (fail_size != 0 && bytes > fail_size) return NULL;
  else if (guards) {
      void *block;
      block = malloc(bytes + GUARD_EXTRA_SIZE);
      if (block) _dbus_atomic_inc(&n_blocks_outstanding);
      else if (malloc_cannot_fail) {
          _dbus_warn("out of memory: malloc (%ld + %ld)", (long)bytes, (long)GUARD_EXTRA_SIZE);
          _dbus_abort();
      }
      return set_guards(block, bytes, SOURCE_MALLOC);
    }
#endif
  else {
      void *mem;
      mem = malloc(bytes);
  #ifndef DBUS_ENABLE_EMBEDDED_TESTS
      if (mem) _dbus_atomic_inc(&n_blocks_outstanding);
      else if (malloc_cannot_fail) {
          _dbus_warn("out of memory: malloc (%ld)", (long)bytes);
          _dbus_abort();
      }
  #endif
      return mem;
  }
}
void* dbus_malloc0(size_t bytes) {
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  _dbus_initialize_malloc_debug();
  if (_dbus_decrement_fail_alloc_counter()) {
      _dbus_verbose(" FAILING malloc0 of %ld bytes\n", (long)bytes);
      return NULL;
  }
#endif
  if (bytes == 0) return NULL;
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  else if (fail_size != 0 && bytes > fail_size) return NULL;
  else if (guards) {
      void *block;
      block = calloc(bytes + GUARD_EXTRA_SIZE, 1);
      if (block) _dbus_atomic_inc(&n_blocks_outstanding);
      else if (malloc_cannot_fail) {
          _dbus_warn("out of memory: calloc (%ld + %ld, 1)", (long)bytes, (long)GUARD_EXTRA_SIZE);
          _dbus_abort();
      }
      return set_guards(block, bytes, SOURCE_MALLOC_ZERO);
  }
#endif
  else {
      void *mem;
      mem = calloc(bytes, 1);
  #ifndef DBUS_ENABLE_EMBEDDED_TESTS
      if (mem) _dbus_atomic_inc(&n_blocks_outstanding);
      else if (malloc_cannot_fail) {
          _dbus_warn("out of memory: calloc (%ld)", (long)bytes);
          _dbus_abort();
      }
  #endif
      return mem;
    }
}
void* dbus_realloc(void *memory, size_t bytes) {
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  _dbus_initialize_malloc_debug();
  if (_dbus_decrement_fail_alloc_counter()) {
      _dbus_verbose(" FAILING realloc of %ld bytes\n", (long)bytes);
      return NULL;
  }
#endif
  if (bytes == 0)  {
      dbus_free(memory);
      return NULL;
  }
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  else if (fail_size != 0 && bytes > fail_size) return NULL;
  else if (guards) {
      if (memory) {
          size_t old_bytes;
          void *block;
          check_guards(memory, FALSE);
          block = realloc(((unsigned char*)memory) - GUARD_START_OFFSET,bytes + GUARD_EXTRA_SIZE);
          if (block == NULL) {
              if (malloc_cannot_fail) {
                  _dbus_warn("out of memory: realloc (%p, %ld + %ld)", memory, (long)bytes, (long)GUARD_EXTRA_SIZE);
                  _dbus_abort();
              }
              return NULL;
          }
          old_bytes = *(dbus_uint32_t*)block;
          if (bytes >= old_bytes) check_guards(((unsigned char*)block) + GUARD_START_OFFSET, FALSE);
          return set_guards(block, bytes, SOURCE_REALLOC);
      } else {
          void *block;
          block = malloc(bytes + GUARD_EXTRA_SIZE);
          if (block) _dbus_atomic_inc(&n_blocks_outstanding);
          else if (malloc_cannot_fail) {
              _dbus_warn("out of memory: malloc (%ld + %ld)", (long)bytes, (long)GUARD_EXTRA_SIZE);
              _dbus_abort();
          }
          return set_guards(block, bytes, SOURCE_REALLOC_NULL);
      }
  }
#endif
  else {
      void *mem;
      mem = realloc(memory, bytes);
  #ifndef DBUS_ENABLE_EMBEDDED_TESTS
      if (mem == NULL && malloc_cannot_fail) {
          _dbus_warn("out of memory: malloc (%ld)", (long)bytes);
          _dbus_abort();
      }
      if (memory == NULL && mem != NULL) _dbus_atomic_inc(&n_blocks_outstanding);
  #endif
      return mem;
    }
}
void dbus_free(void *memory) {
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  if (guards) {
      check_guards(memory, TRUE);
      if (memory) {
      #ifdef DBUS_DISABLE_ASSERT
          _dbus_atomic_dec (&n_blocks_outstanding);
      #else
          dbus_int32_t old_value;
          old_value = _dbus_atomic_dec (&n_blocks_outstanding);
          _dbus_assert (old_value >= 1);
      #endif
          free (((unsigned char*)memory) - GUARD_START_OFFSET);
      }
      return;
  }
#endif
  if (memory) {
  #ifndef DBUS_ENABLE_EMBEDDED_TESTS
  #ifdef DBUS_DISABLE_ASSERT
      _dbus_atomic_dec(&n_blocks_outstanding);
  #else
      dbus_int32_t old_value;
      old_value = _dbus_atomic_dec(&n_blocks_outstanding);
      _dbus_assert(old_value >= 1);
  #endif
  #endif
      free(memory);
  }
}
void dbus_free_string_array(char **str_array) {
  if (str_array) {
      int i;
      i = 0;
      while(str_array[i]) {
          dbus_free(str_array[i]);
          i++;
	  }
      dbus_free(str_array);
  }
}
int _dbus_current_generation = 1;
typedef struct ShutdownClosure ShutdownClosure;
struct ShutdownClosure {
  ShutdownClosure *next;
  DBusShutdownFunction func;
  void *data;
};
static ShutdownClosure *registered_globals = NULL;
dbus_bool_t _dbus_register_shutdown_func(DBusShutdownFunction func, void *data) {
  dbus_bool_t ok;
  if (!_DBUS_LOCK(shutdown_funcs)) return FALSE;
  ok = _dbus_register_shutdown_func_unlocked(func, data);
  _DBUS_UNLOCK(shutdown_funcs);
  return ok;
}
dbus_bool_t _dbus_register_shutdown_func_unlocked(DBusShutdownFunction func, void *data) {
  ShutdownClosure *c;
  c = dbus_new(ShutdownClosure, 1);
  if (c == NULL) return FALSE;
  c->func = func;
  c->data = data;
  c->next = registered_globals;
  registered_globals = c;
  return TRUE;
}
void dbus_shutdown(void) {
  while(registered_globals != NULL) {
      ShutdownClosure *c;
      c = registered_globals;
      registered_globals = c->next;
      (*c->func)(c->data);
      dbus_free(c);
  }
  _dbus_threads_lock_platform_specific();
  _dbus_current_generation += 1;
  _dbus_threads_unlock_platform_specific();
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include "dbus-test.h"

dbus_bool_t _dbus_memory_test(void) {
  dbus_bool_t old_guards;
  void *p;
  size_t size;
  old_guards = guards;
  guards = TRUE;
  p = dbus_malloc(4);
  if (p == NULL) _dbus_test_fatal("no memory");
  for (size = 4; size < 256; size += 4) {
      p = dbus_realloc(p, size);
      if (p == NULL) _dbus_test_fatal("no memory");
  }
  for (size = 256; size != 0; size -= 4) {
      p = dbus_realloc(p, size);
      if (p == NULL) _dbus_test_fatal("no memory");
  }
  dbus_free(p);
  guards = old_guards;
  return TRUE;
}
#endif