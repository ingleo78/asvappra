#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <assert.h>
#include <string.h>
#include "internal.h"
#include "minicheck.h"

Suite * suite_create(const char *name) {
  Suite *suite = (Suite *)calloc(1, sizeof(Suite));
  if (suite != NULL) suite->name = name;
  return suite;
}
TCase *tcase_create(const char *name) {
  TCase *tc = (TCase *)calloc(1, sizeof(TCase));
  if (tc != NULL) tc->name = name;
  return tc;
}
void suite_add_tcase(Suite *suite, TCase *tc) {
  assert(suite != NULL);
  assert(tc != NULL);
  assert(tc->next_tcase == NULL);
  tc->next_tcase = suite->tests;
  suite->tests = tc;
}
void tcase_add_checked_fixture(TCase *tc, tcase_setup_function setup, tcase_teardown_function teardown) {
  assert(tc != NULL);
  tc->setup = setup;
  tc->teardown = teardown;
}
void tcase_add_test(TCase *tc, tcase_test_function test) {
  assert(tc != NULL);
  if (tc->allocated == tc->ntests) {
      int nalloc = tc->allocated + 100;
      size_t new_size = sizeof(tcase_test_function) * nalloc;
      tcase_test_function *new_tests = realloc(tc->tests, new_size);
      assert(new_tests != NULL);
      tc->tests = new_tests;
      tc->allocated = nalloc;
  }
  tc->tests[tc->ntests] = test;
  tc->ntests++;
}
static void tcase_free(TCase *tc) {
  if (! tc) return;
  free(tc->tests);
  free(tc);
}
static void
suite_free(Suite *suite) {
  if (! suite) return;
  while (suite->tests != NULL) {
      TCase *next = suite->tests->next_tcase;
      tcase_free(suite->tests);
      suite->tests = next;
  }
  free(suite);
}
SRunner *srunner_create(Suite *suite) {
  SRunner *runner = calloc(1, sizeof(SRunner));
  if (runner != NULL) runner->suite = suite;
  return runner;
}
static jmp_buf env;
static char const *_check_current_function = NULL;
static int _check_current_lineno = -1;
static char const *_check_current_filename = NULL;
void _check_set_test_info(char const *function, char const *filename, int lineno) {
  _check_current_function = function;
  _check_current_lineno = lineno;
  _check_current_filename = filename;
}
static void handle_success(int verbosity) {
  if (verbosity >= CK_VERBOSE) printf("PASS: %s\n", _check_current_function);
}
static void handle_failure(SRunner *runner, int verbosity, const char *phase_info) {
  runner->nfailures++;
  if (verbosity != CK_SILENT) printf("FAIL: %s (%s at %s:%d)\n", _check_current_function, phase_info, _check_current_filename, _check_current_lineno);
}
void srunner_run_all(SRunner *runner, int verbosity) {
  Suite *suite;
  TCase *volatile tc;
  assert(runner != NULL);
  suite = runner->suite;
  tc = suite->tests;
  while(tc != NULL) {
      volatile int i;
      for (i = 0; i < tc->ntests; ++i) {
          runner->nchecks++;
          if (tc->setup != NULL) {
              if (setjmp(env)) {
                  handle_failure(runner, verbosity, "during setup");
                  continue;
              }
              tc->setup();
          }
          if (setjmp(env)) {
              handle_failure(runner, verbosity, "during actual test");
              continue;
          }
          (tc->tests[i])();
          if (tc->teardown != NULL) {
              if (setjmp(env)) {
                  handle_failure(runner, verbosity, "during teardown");
                  continue;
              }
              tc->teardown();
          }
          handle_success(verbosity);
      }
      tc = tc->next_tcase;
  }
  if (verbosity != CK_SILENT) {
      int passed = runner->nchecks - runner->nfailures;
      double percentage = ((double)passed) / runner->nchecks;
      int display = (int)(percentage * 100);
      printf("%d%%: Checks: %d, Failed: %d\n", display, runner->nchecks, runner->nfailures);
  }
}
void _fail_unless(int condition, const char *file, int line, const char *msg) {
  UNUSED_P(condition);
  _check_current_filename = file;
  _check_current_lineno = line;
  if (msg != NULL) {
      const int has_newline = (msg[strlen(msg) - 1] == '\n');
      fprintf(stderr, "ERROR: %s%s", msg, has_newline ? "" : "\n");
  }
  longjmp(env, 1);
}
int srunner_ntests_failed(SRunner *runner) {
  assert(runner != NULL);
  return runner->nfailures;
}
void srunner_free(SRunner *runner) {
  if (!runner) return;
  suite_free(runner->suite);
  free(runner);
}