#include <stdlib.h>
#include <stdio.h>
#include "../libcompat.h"
#include "../check.h"
#include "check_check.h"

int main(void) {
  int number_failed;
  SRunner *sr;
  fork_setup();
  setup();
  sr = srunner_create (make_master_suite());
  srunner_add_suite(sr, make_log_suite());
  srunner_add_suite(sr, make_fork_suite());
  printf ("Ran %d tests in subordinate suite\n", sub_ntests);
  srunner_run_all (sr, CK_VERBOSE);
  cleanup();
  fork_teardown();
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}