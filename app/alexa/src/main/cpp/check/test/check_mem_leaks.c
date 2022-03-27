#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check/check.h>
#include <check/config.h>
#include "check_check.h"

int main(void) {
    int number_failed;
    SRunner *sr;
    sr = srunner_create(make_sub_suite());
    srunner_set_log(sr, "test_mem_leak.log");
    srunner_set_xml(sr, "test_mem_leak.xml");
    srunner_set_tap(sr, "test_mem_leak.tap");
    srunner_run_all(sr, CK_NORMAL);
    srunner_free(sr);
    fork_setup();
    sr = srunner_create(make_log_suite());
    srunner_add_suite(sr, make_fork_suite());
#if defined(HAVE_FORK) && HAVE_FORK==1
    srunner_add_suite(sr, make_exit_suite());
#endif
    srunner_add_suite(sr, make_tag_suite());
    srunner_add_suite(sr, make_selective_suite());
    srunner_set_log(sr, "test_mem_leak.log");
    srunner_set_xml(sr, "test_mem_leak.xml");
    srunner_set_tap(sr, "test_mem_leak.tap");
    srunner_run_all(sr, CK_NORMAL);
    fork_teardown();
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}