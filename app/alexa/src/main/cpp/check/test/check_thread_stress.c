#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../libcompat.h"
#include "../check.h"

Suite *s;
TCase *tc;
SRunner *sr;
#if defined (HAVE_PTHREAD) || defined (HAVE_FORK)
static void *sendinfo(void *userdata CK_ATTRIBUTE_UNUSED) {
  unsigned int i;
  for (i = 0; i < 999; i++) {
      ck_assert_msg (1, "Shouldn't see this message");
  }
  return NULL;
}
#endif
#ifdef HAVE_PTHREAD
START_TEST (test_stress_threads) {
  pthread_t a, b;
  pthread_create (&a, NULL, sendinfo, (void *) 0xa);
  pthread_create (&b, NULL, sendinfo, (void *) 0xb);
  pthread_join (a, NULL);
  pthread_join (b, NULL);
}
END_TEST
#endif
#if defined(HAVE_FORK) && HAVE_FORK==1
START_TEST(test_stress_forks) {
  pid_t cpid = fork();
  if (cpid == 0) {
      sendinfo((void*)0x1);
      exit (EXIT_SUCCESS);
  } else sendinfo((void*)0x2);
}
END_TEST
#endif
int main(void) {
  int number_failed;
  s = suite_create("ForkThreadStress");
  tc = tcase_create("ForkThreadStress");
  sr = srunner_create(s);
  suite_add_tcase(s, tc);
#ifdef HAVE_PTHREAD
  tcase_add_loop_test(tc, test_stress_threads, 0, 100);
#endif
#if defined(HAVE_FORK) && HAVE_FORK==1
  tcase_add_loop_test(tc, test_stress_forks, 0, 100);
#endif
  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
#ifndef HAVE_FORK
  number_failed++;
#endif
  return number_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}