#include "../glib.h"

const guint32 first_numbers[] = {
  0x7a7a7a7a,
  0xfdcc2d54,
  0x3a279ceb,
  0xc4d39c33,
  0xf31895cd,
  0x46ca0afc,
  0x3f5484ff,
  0x54bc9557,
  0xed2c24b1,
  0x84062503,
  0x8f6404b3,
  0x599a94b3,
  0xe46d03d5,
  0x310beb78,
  0x7bee5d08,
  0x760d09be,
  0x59b6e163,
  0xbf6d16ec,
  0xcca5fb54,
  0x5de7259b,
  0x1696330c,
};
const guint32 seed_array[] = {
  0x6553375f,
  0xd6b8d43b,
  0xa1e7667f,
  0x2b10117c
};
const guint32 array_outputs[] = {
  0xc22b7dc3,
  0xfdecb8ae,
  0xb4af0738,
  0x516bc6e1,
  0x7e372e91,
  0x2d38ff80,
  0x6096494a,
  0xd162d5a8,
  0x3c0aaa0d,
  0x10e736ae
};
static void test_rand(void) {
  guint n;
  guint ones;
  double proportion;
  GRand *rand;
  GRand *copy;
  rand = g_rand_new_with_seed(first_numbers[0]);
  for (n = 1; n < G_N_ELEMENTS(first_numbers); n++) g_assert(first_numbers[n] == g_rand_int(rand));
  g_rand_set_seed(rand, 2);
  g_rand_set_seed_array(rand, seed_array, G_N_ELEMENTS(seed_array));
  for (n = 0; n < G_N_ELEMENTS(array_outputs); n++) g_assert(array_outputs[n] == g_rand_int(rand));
  copy = g_rand_copy(rand);
  for (n = 0; n < 100; n++) g_assert(g_rand_int(copy) == g_rand_int(rand));
  for (n = 1; n < 100000; n++) {
      gint32 i;
      gdouble d;
      gboolean b;
      i = g_rand_int_range(rand, 8,16);
      g_assert(i >= 8 && i < 16);
      i = g_random_int_range(8,16);
      g_assert(i >= 8 && i < 16);
      d = g_rand_double (rand);
      g_assert(d >= 0 && d < 1);
      d = g_random_double();
      g_assert(d >= 0 && d < 1);
      d = g_rand_double_range(rand, -8, 32);
      g_assert(d >= -8 && d < 32);
      d = g_random_double_range(-8, 32);
      g_assert(d >= -8 && d < 32);
      b = g_random_boolean ();
      g_assert(b == TRUE || b  == FALSE);
      b = g_rand_boolean(rand);
      g_assert(b == TRUE || b  == FALSE);
  }
  ones = 0;
  for (n = 1; n < 100000; n++) {
      if (g_random_int_range (0, 4) == 1) ones ++;
  }
  proportion = (double)ones / (double)100000;
  g_assert (ABS (proportion - 0.25) < 0.025);
  g_rand_free (rand);
  g_rand_free (copy);
}
int main(int argc, char *argv[]) {
  g_test_init(&argc, &argv, NULL);
  g_test_add_func("/rand/test-rand", test_rand);
  return g_test_run();
}