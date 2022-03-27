#include "gatomic.h"

gint g_atomic_int_exchange_and_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  return __sync_fetch_and_add(atomic, val);
}
void g_atomic_int_add (volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  __sync_fetch_and_add(atomic, val);
}
gboolean g_atomic_int_compare_and_exchange(volatile gint G_GNUC_MAY_ALIAS *atomic, gint oldval, gint newval) {
  return __sync_bool_compare_and_swap(atomic, oldval, newval);
}
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval) {
  return __sync_bool_compare_and_swap(atomic, oldval, newval);
}
void _g_atomic_thread_init(void) {}
gint (g_atomic_int_get)(volatile gint G_GNUC_MAY_ALIAS *atomic) {
  __sync_synchronize();
  return *atomic;
}
void (g_atomic_int_set)(volatile gint G_GNUC_MAY_ALIAS *atomic, gint newval) {
  *atomic = newval;
  __sync_synchronize();
}
gpointer (g_atomic_pointer_get)(volatile gpointer G_GNUC_MAY_ALIAS *atomic) {
  __sync_synchronize();
  return *atomic;
}
void (g_atomic_pointer_set)(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer newval) {
  *atomic = newval;
  __sync_synchronize();
}