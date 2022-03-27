#include <string.h>
#include <stdlib.h>
#include "../glib.h"

typedef struct {
  GScanner *scanner;
} ScannerFixture;
static void scanner_fixture_setup(ScannerFixture *fix, gconstpointer test_data) {
  fix->scanner = g_scanner_new(NULL);
  g_assert(fix->scanner != NULL);
}
static void scanner_fixture_teardown(ScannerFixture *fix, gconstpointer test_data) {
  g_assert(fix->scanner != NULL);
  g_scanner_destroy(fix->scanner);
}
static void scanner_msg_func(GScanner *scanner, gchar *message, gboolean  error) {
  g_assert_cmpstr(message, ==, "test");
}
static void test_scanner_warn(ScannerFixture *fix, gconstpointer test_data) {
  fix->scanner->msg_handler = scanner_msg_func;
  g_scanner_warn(fix->scanner, "test");
}
static void test_scanner_error(ScannerFixture *fix, gconstpointer test_data) {
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR)) {
      int pe = fix->scanner->parse_errors;
      g_scanner_error(fix->scanner, "scanner-error-message-test");
      g_assert_cmpint(fix->scanner->parse_errors, ==, pe + 1);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stderr("*scanner-error-message-test*");
}
static void check_keys(gpointer key, gpointer value, gpointer user_data) {
  g_assert_cmpint(GPOINTER_TO_INT(value), ==, g_ascii_strtoull(key, NULL, 0));
}
static void test_scanner_symbols(ScannerFixture *fix, gconstpointer   test_data) {
  gint i;
  gchar buf[2];
  g_scanner_set_scope(fix->scanner, 1);
  for (i = 0; i < 10; i++) g_scanner_scope_add_symbol(fix->scanner,1,g_ascii_dtostr(buf, 2, (gdouble)i), GINT_TO_POINTER(i));
  g_scanner_scope_foreach_symbol(fix->scanner, 1, check_keys, NULL);
  g_assert_cmpint(GPOINTER_TO_INT(g_scanner_lookup_symbol(fix->scanner, "5")), ==, 5);
  g_scanner_scope_remove_symbol(fix->scanner, 1, "5");
  g_assert(g_scanner_lookup_symbol(fix->scanner, "5") == NULL);
  g_assert_cmpint(GPOINTER_TO_INT(g_scanner_scope_lookup_symbol(fix->scanner, 1, "4")), ==, 4);
  g_assert_cmpint(GPOINTER_TO_INT(g_scanner_scope_lookup_symbol(fix->scanner, 1, "5")), ==, 0);
}
static void test_scanner_tokens (ScannerFixture *fix, gconstpointer   test_data) {
  gchar buf[] = "(\t\n\r\\){}";
  const gint buflen = strlen(buf);
  gchar tokbuf[] = "(\\){}";
  const gint tokbuflen = strlen(tokbuf);
  guint i;
  g_scanner_input_text(fix->scanner, buf, buflen);
  g_assert_cmpint(g_scanner_cur_token(fix->scanner), ==, G_TOKEN_NONE);
  g_scanner_get_next_token(fix->scanner);
  g_assert_cmpint(g_scanner_cur_token(fix->scanner), ==, tokbuf[0]);
  g_assert_cmpint(g_scanner_cur_line(fix->scanner), ==, 1);
  for (i = 1; i < tokbuflen; i++) g_assert_cmpint(g_scanner_get_next_token(fix->scanner), ==, tokbuf[i]);
  g_assert_cmpint(g_scanner_get_next_token(fix->scanner), ==, G_TOKEN_EOF);
  return;
}
int main(int argc, char *argv[]) {
  g_test_init(&argc, &argv, NULL);
  g_test_add("/scanner/warn", ScannerFixture, 0, scanner_fixture_setup, test_scanner_warn, scanner_fixture_teardown);
  g_test_add("/scanner/error", ScannerFixture, 0, scanner_fixture_setup, test_scanner_error, scanner_fixture_teardown);
  g_test_add("/scanner/symbols", ScannerFixture, 0, scanner_fixture_setup, test_scanner_symbols, scanner_fixture_teardown);
  g_test_add("/scanner/tokens", ScannerFixture, 0, scanner_fixture_setup, test_scanner_tokens, scanner_fixture_teardown);
  return g_test_run();
}