#include "gprimes.h"

static const guint g_primes[] = {
  11,
  19,
  37,
  73,
  109,
  163,
  251,
  367,
  557,
  823,
  1237,
  1861,
  2777,
  4177,
  6247,
  9371,
  14057,
  21089,
  31627,
  47431,
  71143,
  106721,
  160073,
  240101,
  360163,
  540217,
  810343,
  1215497,
  1823231,
  2734867,
  4102283,
  6153409,
  9230113,
  13845163,
};
static const guint g_nprimes = sizeof(g_primes) / sizeof(g_primes[0]);
guint g_spaced_primes_closest(guint num) {
  gint i;
  for (i = 0; i < g_nprimes; i++)
      if (g_primes[i] > num) return g_primes[i];
  return g_primes[g_nprimes - 1];
}