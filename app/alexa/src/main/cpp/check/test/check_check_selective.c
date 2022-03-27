#include <stdio.h>
#include <stdlib.h>
#include "../libcompat.h"
#include "../check.h"
#include "check_check.h"

static SRunner *sr;
static int test_tc11_executed;
static int test_tc12_executed;
static int test_tc21_executed;
static void reset_executed(void) {
  test_tc11_executed = 0;
  test_tc12_executed = 0;
  test_tc21_executed = 0;
}
START_TEST(test_tc11) {
  test_tc11_executed = 1;
}
END_TEST
START_TEST(test_tc12) {
  test_tc12_executed = 1;
}
END_TEST
START_TEST(test_tc21) {
  test_tc21_executed = 1;
}
END_TEST
static void selective_setup(void) {
  Suite *s1, *s2;
  TCase *tc11, *tc12, *tc21;
  s1 = suite_create("suite1");
  tc11 = tcase_create("tcase11");
  tcase_add_test(tc11, test_tc11);
  tc12 = tcase_create("tcase12");
  tcase_add_test(tc12, test_tc12);
  suite_add_tcase(s1, tc11);
  suite_add_tcase(s1, tc12);
  suite_add_tcase(s1, tc12);
  s2 = suite_create("suite2");
  tc21 = tcase_create("tcase21");
  tcase_add_test(tc21, test_tc21);
  suite_add_tcase(s2, tc21);
  sr = srunner_create(s1);
  srunner_add_suite(sr, s2);
  srunner_set_fork_status(sr, CK_NOFORK);
}
static void selective_teardown(void) {
  srunner_free(sr);
}
START_TEST(test_srunner_run_run_all) {
  srunner_run(sr,NULL,NULL,CK_VERBOSE);
  ck_assert_msg(srunner_ntests_run(sr) == 3, "Not all tests were executed.");
  reset_executed();
}
END_TEST
START_TEST(test_srunner_run_suite) {
  srunner_run(sr,"suite1",NULL,CK_VERBOSE);
  ck_assert_msg(test_tc11_executed && test_tc12_executed && !test_tc21_executed, "Expected tests were not executed.");
  reset_executed();
}
END_TEST
START_TEST(test_srunner_run_no_suite) {
  srunner_run(sr,"non-existing-suite",NULL,CK_VERBOSE);
  ck_assert_msg(!(test_tc11_executed || test_tc12_executed || test_tc21_executed), "An unexpected test was executed.");
  reset_executed();
}
END_TEST
START_TEST(test_srunner_run_tcase) {
  srunner_run(sr,NULL,"tcase12",CK_VERBOSE);
  ck_assert_msg(!test_tc11_executed && test_tc12_executed && !test_tc21_executed, "Expected tests were not executed.");
  reset_executed();
}
END_TEST
START_TEST(test_srunner_no_tcase) {
  srunner_run(sr,NULL,"non-existant-test-case",CK_VERBOSE);
  ck_assert_msg(!(test_tc11_executed || test_tc12_executed || test_tc21_executed), "An unexpected test was executed.");
  reset_executed();
}
END_TEST
START_TEST(test_srunner_suite_tcase) {
  srunner_run(sr,"suite2","tcase21",CK_VERBOSE);
  ck_assert_msg(!test_tc11_executed && !test_tc12_executed && test_tc21_executed, "Expected tests were not executed.");
  reset_executed();
}
END_TEST
START_TEST(test_srunner_suite_no_tcase) {
  srunner_run(sr,"suite1","non-existant-test-case",CK_VERBOSE);
  ck_assert_msg(!(test_tc11_executed || test_tc12_executed || test_tc21_executed), "An unexpected test was executed.");
  reset_executed();
}
END_TEST
#if HAVE_DECL_SETENV
START_TEST(test_srunner_run_suite_env) {
  setenv("CK_RUN_SUITE", "suite1", 1);
  srunner_run_all(sr, CK_VERBOSE);
  ck_assert_msg(test_tc11_executed && test_tc12_executed && !test_tc21_executed, "Expected tests were not executed.");
  reset_executed();
  unsetenv("CK_RUN_SUITE");
}
END_TEST
START_TEST(test_srunner_run_no_suite_env) {
  setenv("CK_RUN_SUITE", "non-existing-suite", 1);
  srunner_run_all(sr, CK_VERBOSE);
  ck_assert_msg(!(test_tc11_executed || test_tc12_executed || test_tc21_executed), "An unexpected test was executed.");
  reset_executed();
  unsetenv("CK_RUN_SUITE");
}
END_TEST
START_TEST(test_srunner_run_tcase_env) {
  setenv("CK_RUN_CASE", "tcase12", 1);
  srunner_run_all(sr, CK_VERBOSE);
  ck_assert_msg(!test_tc11_executed && test_tc12_executed && !test_tc21_executed, "Expected tests were not executed.");
  reset_executed();
  unsetenv("CK_RUN_CASE");
}
END_TEST
START_TEST(test_srunner_no_tcase_env) {
  setenv("CK_RUN_CASE", "non-existant-test-case", 1);
  srunner_run_all(sr, CK_VERBOSE);
  ck_assert_msg(!(test_tc11_executed || test_tc12_executed || test_tc21_executed), "An unexpected test was executed.");
  reset_executed();
  unsetenv("CK_RUN_CASE");
}
END_TEST
START_TEST(test_srunner_suite_tcase_env) {
  setenv("CK_RUN_SUITE", "suite2", 1);
  setenv("CK_RUN_CASE", "tcase21", 1);
  srunner_run_all(sr, CK_VERBOSE);
  ck_assert_msg(!test_tc11_executed && !test_tc12_executed && test_tc21_executed, "Expected tests were not executed.");
  reset_executed();
  unsetenv("CK_RUN_SUITE");
  unsetenv("CK_RUN_CASE");
}
END_TEST
START_TEST(test_srunner_suite_no_tcase_env) {
  setenv("CK_RUN_SUITE", "suite1", 1);
  setenv("CK_RUN_CASE", "non-existant-test-case", 1);
  srunner_run_all(sr, CK_VERBOSE);
  ck_assert_msg(!(test_tc11_executed || test_tc12_executed || test_tc21_executed), "An unexpected test was executed.");
  reset_executed();
  unsetenv("CK_RUN_SUITE");
  unsetenv("CK_RUN_CASE");
}
END_TEST
#endif
Suite *make_selective_suite(void) {
  Suite *s = suite_create("SelectiveTesting");
  TCase *tc = tcase_create("Core");
  suite_add_tcase(s, tc);
  tcase_add_test(tc, test_srunner_run_run_all);
  tcase_add_test(tc, test_srunner_run_suite);
  tcase_add_test(tc, test_srunner_run_no_suite);
  tcase_add_test(tc, test_srunner_run_tcase);
  tcase_add_test(tc, test_srunner_no_tcase);
  tcase_add_test(tc, test_srunner_suite_tcase);
  tcase_add_test(tc, test_srunner_suite_no_tcase);
#if HAVE_DECL_SETENV
  tcase_add_test(tc, test_srunner_run_suite_env);
  tcase_add_test(tc, test_srunner_run_no_suite_env);
  tcase_add_test(tc, test_srunner_run_tcase_env);
  tcase_add_test(tc, test_srunner_no_tcase_env);
  tcase_add_test(tc, test_srunner_suite_tcase_env);
  tcase_add_test(tc, test_srunner_suite_no_tcase_env);
#endif
  tcase_add_unchecked_fixture(tc, selective_setup, selective_teardown);
  return s;
}