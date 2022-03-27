#include "gatomic.h"
#include "gslist.h"
#include "gthread.h"
#include "gbitlock.h"
#include "gthreadprivate.h"

#ifdef G_BIT_LOCK_FORCE_FUTEX_EMULATION
#undef HAVE_FUTEX
#endif
#ifndef HAVE_FUTEX
static GSList *g_futex_address_list = NULL;
static GMutex *g_futex_mutex = NULL;
#endif
void
_g_futex_thread_init (void) {
#ifndef HAVE_FUTEX
  g_futex_mutex = g_mutex_new();
#endif
}
#ifdef HAVE_FUTEX
#include <linux/futex.h>
#include <syscall.h>
#include <unistd.h>

static void g_futex_wait(const volatile gint *address, gint value) {
  syscall(SYS_futex, address, (gsize)FUTEX_WAIT, (gsize)value, NULL);
}
static void g_futex_wake(const volatile gint *address) {
  syscall(SYS_futex, address, (gsize)FUTEX_WAKE, (gsize)1, NULL);
}
#else
typedef struct {
  const volatile gint *address;
  gint ref_count;
  GCond *wait_queue;
} WaitAddress;
static WaitAddress* g_futex_find_address(const volatile gint *address) {
  GSList *node;
  for (node = g_futex_address_list; node; node = node->next) {
      WaitAddress *waiter = node->data;
      if (waiter->address == address) return waiter;
  }
  return NULL;
}
static void g_futex_wait(const volatile gint *address, gint value) {
  g_mutex_lock(g_futex_mutex);
  if G_LIKELY(g_atomic_int_get(address) == value) {
      WaitAddress *waiter;
      if ((waiter = g_futex_find_address(address)) == NULL) {
          waiter = g_slice_new(WaitAddress);
          waiter->address = address;
          waiter->wait_queue = g_cond_new();
          waiter->ref_count = 0;
          g_futex_address_list = g_slist_prepend(g_futex_address_list, waiter);
      }
      waiter->ref_count++;
      g_cond_wait(waiter->wait_queue, g_futex_mutex);
      if (!--waiter->ref_count) {
          g_futex_address_list = g_slist_remove(g_futex_address_list, waiter);
          g_cond_free(waiter->wait_queue);
          g_slice_free(WaitAddress, waiter);
      }
  }
  g_mutex_unlock(g_futex_mutex);
}
static void g_futex_wake(const volatile gint *address) {
  WaitAddress *waiter;
  g_mutex_lock (g_futex_mutex);
  if ((waiter = g_futex_find_address (address))) g_cond_signal(waiter->wait_queue);
  g_mutex_unlock(g_futex_mutex);
}
#endif
#define CONTENTION_CLASSES 11
static volatile gint g_bit_lock_contended[CONTENTION_CLASSES];
void g_bit_lock(volatile gint *address, gint lock_bit) {
  guint v;
  retry:
  v = g_atomic_int_get(address);
  if (v & (1u << lock_bit)) {
      guint class = ((gsize)address) % G_N_ELEMENTS(g_bit_lock_contended);
      g_atomic_int_add(&g_bit_lock_contended[class], +1);
      g_futex_wait(address, v);
      g_atomic_int_add(&g_bit_lock_contended[class], -1);
      goto retry;
  }
  if (!g_atomic_int_compare_and_exchange (address, v, v | (1u << lock_bit))) goto retry;
}
gboolean g_bit_trylock(volatile gint *address, gint lock_bit) {
  guint v;
  retry:
  v = g_atomic_int_get (address);
  if (v & (1u << lock_bit)) return FALSE;
  if (!g_atomic_int_compare_and_exchange (address, v, v | (1u << lock_bit))) goto retry;
  return TRUE;
}
void g_bit_unlock(volatile gint *address, gint lock_bit) {
  guint class = ((gsize)address) % G_N_ELEMENTS(g_bit_lock_contended);
  guint v;
  retry:
  v = g_atomic_int_get(address);
  if (!g_atomic_int_compare_and_exchange(address, v, v & ~(1u << lock_bit))) goto retry;
  if (g_atomic_int_get(&g_bit_lock_contended[class])) g_futex_wake(address);
}