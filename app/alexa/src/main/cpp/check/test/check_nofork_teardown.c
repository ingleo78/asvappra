#include <stdio.h>
#include <stdlib.h>
#include "../libcompat.h"
#include "../check.h"

START_TEST( will_fail ) {
    ck_assert(0);
}
END_TEST
static void empty_checked_teardown( void ) {}
int main( void ) {
    SRunner *sr = srunner_create(NULL);
    Suite *s = suite_create("bug-99");
    TCase *tc = tcase_create("tc");
    int result;
    srunner_add_suite(sr, s);
    srunner_set_fork_status(sr, CK_NOFORK);
    suite_add_tcase(s, tc);
    tcase_add_checked_fixture(tc, NULL, empty_checked_teardown);
    tcase_add_test(tc, will_fail);
    srunner_run_all(sr, CK_ENV);
    result = srunner_ntests_failed(sr) ? EXIT_FAILURE : EXIT_SUCCESS;
    srunner_free(sr);
    return result;
}