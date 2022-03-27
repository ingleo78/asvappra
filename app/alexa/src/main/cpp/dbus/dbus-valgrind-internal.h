#ifndef DBUS_VALGRIND_INTERNAL_H
#define DBUS_VALGRIND_INTERNAL_H

#include "config.h"
#include "dbus-internals.h"

#ifdef WITH_VALGRIND
#include <memcheck.h>
#include <valgrind.h>
#else
#define VALGRIND_CREATE_MEMPOOL(_1, _2, _3)  do { }while(0);
#define VALGRIND_DESTROY_MEMPOOL(_1)  do { }while(0);
#define VALGRIND_MEMPOOL_ALLOC(_1, _2, _3)  do { }while(0);
#define VALGRIND_MEMPOOL_FREE(_1, _2)  do { }while(0);
static inline int VALGRIND_MAKE_MEM_UNDEFINED(void *addr, size_t len) {
  return 0;
}
static inline int VALGRIND_PRINTF(const char *format, ...) {
  return 0;
}
static inline int VALGRIND_PRINTF_BACKTRACE(const char *format, ...) {
  return 0;
}
#define RUNNING_ON_VALGRIND 0
#endif

#endif