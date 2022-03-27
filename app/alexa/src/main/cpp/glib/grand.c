#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "grand.h"
#include "gmain.h"
#include "gmem.h"
#include "gtestutils.h"
#include "gthread.h"
#include "gthreadprivate.h"
#ifndef G_OS_WIN32
#include <process.h>
#endif

G_LOCK_DEFINE_STATIC (global_random);
static GRand* global_random = NULL;
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df
#define UPPER_MASK 0x80000000
#define LOWER_MASK 0x7fffffff
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)
static guint get_random_version(void) {
  static int initialized = FALSE;
  static guint random_version;
  if (!initialized) {
      const gchar *version_string = g_getenv("G_RANDOM_VERSION");
      if (!version_string || version_string[0] == '\000' || strcmp(version_string, "2.2") == 0) random_version = 22;
      else if (strcmp(version_string, "2.0") == 0) random_version = 20;
      else {
          g_warning ("Unknown G_RANDOM_VERSION \"%s\". Using version 2.2.", version_string);
          random_version = 22;
	  }
      initialized = TRUE;
  }
  return random_version;
}
void _g_rand_thread_init(void) {
  (void)get_random_version();
}
struct _GRand {
  guint32 mt[N];
  guint mti; 
};
GRand* g_rand_new_with_seed(guint32 seed) {
  GRand *rand = g_new0(GRand, 1);
  g_rand_set_seed(rand, seed);
  return rand;
}
GRand* g_rand_new_with_seed_array(const guint32 *seed, guint seed_length) {
  GRand *rand = g_new0(GRand, 1);
  g_rand_set_seed_array(rand, seed, seed_length);
  return rand;
}
GRand* g_rand_new(void) {
  guint32 seed[4];
  GTimeVal now;
#ifdef G_OS_UNIX
  static int dev_urandom_exists = TRUE;
  if (dev_urandom_exists) {
      FILE* dev_urandom;
      do {
          errno = 0;
          dev_urandom = fopen("/dev/urandom", "rb");
	  } while G_UNLIKELY (errno == EINTR);
      if (dev_urandom) {
          int r;
          setvbuf (dev_urandom, NULL, _IONBF, 0);
          do {
              errno = 0;
              r = fread (seed, sizeof (seed), 1, dev_urandom);
          } while G_UNLIKELY (errno == EINTR);
          if (r != 1) dev_urandom_exists = FALSE;
          fclose (dev_urandom);
	  } else dev_urandom_exists = FALSE;
  }
#else
  static int dev_urandom_exists = FALSE;
#endif
  if (!dev_urandom_exists) {
      g_get_current_time(&now);
      seed[0] = now.tv_sec;
      seed[1] = now.tv_usec;
      seed[2] = getpid();
  #ifdef G_OS_UNIX
      seed[3] = getppid ();
  #else
      seed[3] = 0;
  #endif
  }
  return g_rand_new_with_seed_array (seed, 4);
}
void g_rand_free(GRand* rand) {
  g_return_if_fail(rand != NULL);
  g_free (rand);
}
GRand* g_rand_copy(GRand* rand) {
  GRand* new_rand;
  g_return_val_if_fail(rand != NULL, NULL);
  new_rand = g_new0(GRand, 1);
  memcpy(new_rand, rand, sizeof(GRand));
  return new_rand;
}
void g_rand_set_seed(GRand* rand, guint32 seed) {
  g_return_if_fail(rand != NULL);
  switch (get_random_version()) {
      case 20:
          if (seed == 0) seed = 0x6b842128;
          rand->mt[0]= seed;
          for (rand->mti=1; rand->mti<N; rand->mti++) rand->mt[rand->mti] = (69069 * rand->mt[rand->mti-1]);
          break;
      case 22:
          rand->mt[0]= seed;
          for (rand->mti=1; rand->mti<N; rand->mti++) rand->mt[rand->mti] = 1812433253UL *
          (rand->mt[rand->mti-1] ^ (rand->mt[rand->mti-1] >> 30)) + rand->mti;
          break;
      default: g_assert_not_reached();
  }
}
void g_rand_set_seed_array(GRand* rand, const guint32 *seed, guint seed_length) {
  int i, j, k;
  g_return_if_fail(rand != NULL);
  g_return_if_fail(seed_length >= 1);
  g_rand_set_seed (rand, 19650218UL);
  i=1; j=0;
  k = (N>seed_length ? N : seed_length);
  for (; k; k--) {
      rand->mt[i] = (rand->mt[i] ^ ((rand->mt[i-1] ^ (rand->mt[i-1] >> 30)) * 1664525UL)) + seed[j] + j;
      rand->mt[i] &= 0xffffffffUL;
      i++; j++;
      if (i>=N) {
          rand->mt[0] = rand->mt[N-1];
          i=1;
	  }
      if (j>=seed_length) j=0;
  }
  for (k=N-1; k; k--) {
      rand->mt[i] = (rand->mt[i] ^ ((rand->mt[i-1] ^ (rand->mt[i-1] >> 30)) * 1566083941UL)) - i;
      rand->mt[i] &= 0xffffffffUL;
      i++;
      if (i>=N) {
          rand->mt[0] = rand->mt[N-1];
          i=1;
	  }
  }
  rand->mt[0] = 0x80000000UL;
}
guint32 g_rand_int(GRand* rand) {
  guint32 y;
  static const guint32 mag01[2] = {0x0, MATRIX_A};
  g_return_val_if_fail(rand != NULL, 0);
  if (rand->mti >= N) {
      int kk;
      for (kk = 0; kk < N - M; kk++) {
          y = (rand->mt[kk] & UPPER_MASK) | (rand->mt[kk+1] & LOWER_MASK);
          rand->mt[kk] = rand->mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1];
      }
      for ( ; kk < N - 1; kk++) {
          y = (rand->mt[kk] & UPPER_MASK) | (rand->mt[kk + 1] & LOWER_MASK);
          rand->mt[kk] = rand->mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
      }
      y = (rand->mt[N-1] & UPPER_MASK) | (rand->mt[0] & LOWER_MASK);
      rand->mt[N-1] = rand->mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];
      rand->mti = 0;
  }
  y = rand->mt[rand->mti++];
  y ^= TEMPERING_SHIFT_U(y);
  y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
  y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
  y ^= TEMPERING_SHIFT_L(y);
  return y; 
}
#define G_RAND_DOUBLE_TRANSFORM 2.3283064365386962890625e-10
gint32 g_rand_int_range(GRand* rand, gint32 begin, gint32 end) {
  guint32 dist = end - begin;
  guint32 random;
  g_return_val_if_fail(rand != NULL, begin);
  g_return_val_if_fail(end > begin, begin);
  switch(get_random_version()) {
      case 20:
          if (dist <= 0x10000L) {
              gdouble double_rand = g_rand_int (rand) * (G_RAND_DOUBLE_TRANSFORM + G_RAND_DOUBLE_TRANSFORM * G_RAND_DOUBLE_TRANSFORM);
              random = (gint32) (double_rand * dist);
          } else random = (gint32) g_rand_double_range (rand, 0, dist);
          break;
      case 22:
          if (dist == 0) random = 0;
          else {
          guint32 maxvalue;
          if (dist <= 0x80000000u) {
              guint32 leftover = (0x80000000u % dist) * 2;
              if (leftover >= dist) leftover -= dist;
              maxvalue = 0xffffffffu - leftover;
          } else maxvalue = dist - 1;
          do {
              random = g_rand_int(rand);
          } while(random > maxvalue);
          random %= dist;
          }
          break;
      default:
          random = 0;
          g_assert_not_reached();
  }
  return begin + random;
}
gdouble g_rand_double(GRand* rand) {
  gdouble retval = g_rand_int(rand) * G_RAND_DOUBLE_TRANSFORM;
  retval = (retval + g_rand_int(rand)) * G_RAND_DOUBLE_TRANSFORM;
  if (retval >= 1.0) return g_rand_double(rand);
  return retval;
}
gdouble g_rand_double_range(GRand* rand, gdouble begin, gdouble end) {
  return g_rand_double(rand) * (end - begin) + begin;
}
guint32 g_random_int(void) {
  guint32 result;
  G_LOCK(global_random);
  if (!global_random) global_random = g_rand_new();
  result = g_rand_int(global_random);
  G_UNLOCK(global_random);
  return result;
}
gint32 g_random_int_range(gint32 begin, gint32 end) {
  gint32 result;
  G_LOCK(global_random);
  if (!global_random) global_random = g_rand_new();
  result = g_rand_int_range(global_random, begin, end);
  G_UNLOCK (global_random);
  return result;
}
gdouble g_random_double(void) {
  double result;
  G_LOCK(global_random);
  if (!global_random) global_random = g_rand_new();
  result = g_rand_double(global_random);
  G_UNLOCK(global_random);
  return result;
}
gdouble g_random_double_range(gdouble begin, gdouble end) {
  double result;
  G_LOCK(global_random);
  if (!global_random) global_random = g_rand_new();
  result = g_rand_double_range(global_random, begin, end);
  G_UNLOCK(global_random);
  return result;
}
void g_random_set_seed(guint32 seed) {
  G_LOCK(global_random);
  if (!global_random) global_random = g_rand_new_with_seed(seed);
  else g_rand_set_seed(global_random, seed);
  G_UNLOCK(global_random);
}