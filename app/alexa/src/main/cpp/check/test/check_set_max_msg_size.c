#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../check.h"
#include "../config.h"
#include "check_check.h"

START_TEST(test_set_max_msg_size) {
  ck_abort_msg("40 characters of an assertion message...");
}
END_TEST
static Suite *make_set_max_msg_size_suite(void) {
  Suite *s = suite_create("Check Set Max Msg Size");
  TCase *tc_set_max_msg_size = tcase_create("Test Set Max Msg Size");
  suite_add_tcase(s, tc_set_max_msg_size);
  tcase_add_test(tc_set_max_msg_size, test_set_max_msg_size);
  return s;
}
int main(int argc, char *argv[]) {
    SRunner *sr;
    if (argc != 2) {
        fprintf(stderr, "usage: %s max-msg-size\n", argv[0]);
        return EXIT_FAILURE;
    }
    check_set_max_msg_size(32);
    check_set_max_msg_size(strtoul(argv[1], NULL, 10));
    sr = srunner_create(make_set_max_msg_size_suite());
    srunner_run_all(sr, CK_NORMAL);
    srunner_free(sr);
    return EXIT_SUCCESS;
}