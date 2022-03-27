#define LOCKS      48
#define ITERATIONS 10000
#define THREADS    100

#if TEST_EMULATED_FUTEX
#define GLIB_COMPILATION
#define g_bit_lock            _emufutex_g_bit_lock
#define g_bit_trylock         _emufutex_g_bit_trylock
#define g_bit_unlock          _emufutex_g_bit_unlock
#define _g_futex_thread_init  _emufutex_g_futex_thread_init
#define G_BIT_LOCK_FORCE_FUTEX_EMULATION
#include <glib/gbitlock.c>
#endif

#include "../../glib/glib.h"

volatile GThread *owners[LOCKS];
volatile gint locks[LOCKS];
volatile gint bits[LOCKS];
static void acquire(int nr) {
  GThread *self;
  self = g_thread_self ();
  if (!g_bit_trylock (&locks[nr], bits[nr])) {
      if (g_test_verbose ()) g_print ("thread %p going to block on lock %d\n", self, nr);
      g_bit_lock (&locks[nr], bits[nr]);
  }
  g_assert (owners[nr] == NULL);
  owners[nr] = self;
  g_thread_yield ();
  g_thread_yield ();
  g_thread_yield ();
  g_assert (owners[nr] == self);
  owners[nr] = NULL;
  g_bit_unlock (&locks[nr], bits[nr]);
}
static gpointer thread_func(gpointer data) {
  gint i;
  for (i = 0; i < ITERATIONS; i++) acquire (g_random_int () % LOCKS);
  return NULL;
}
static void testcase (void) {
  GThread *threads[THREADS];
  int i;
  g_thread_init (NULL);
#ifdef TEST_EMULATED_FUTEX
  _g_futex_thread_init ();
  #define SUFFIX "-emufutex"
  g_assert (g_futex_mutex != NULL);
#else
  #define SUFFIX ""
#endif
  for (i = 0; i < LOCKS; i++) bits[i] = g_random_int () % 32;
  for (i = 0; i < THREADS; i++) threads[i] = g_thread_create (thread_func, NULL, TRUE, NULL);
  for (i = 0; i < THREADS; i++) g_thread_join (threads[i]);
  for (i = 0; i < LOCKS; i++) {
      g_assert (owners[i] == NULL);
      g_assert (locks[i] == 0);
  }
}
int main(int argc, char **argv) {
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/glib/1bit-mutex" SUFFIX, testcase);
  return g_test_run ();
}