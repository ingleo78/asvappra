#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../glib.h"
#include "../gstdio.h"

static void test_retval_and_trunc(void) {
  gchar buf[128];
  gint res;
  res = g_snprintf(buf, 0, "abc");
  g_assert_cmpint(res, ==, 3);
  res = g_snprintf(NULL, 0, "abc");
  g_assert_cmpint(res, ==, 3);
  res = g_snprintf(buf, 5, "abc");
  g_assert_cmpint(res, ==, 3);
  res = g_snprintf(buf, 1, "abc");
  g_assert_cmpint(res, ==, 3);
  g_assert(buf[0] == '\0');
  g_assert_cmpstr(buf, ==, "");
  res = g_snprintf(buf, 2, "abc");
  g_assert_cmpint(res, ==, 3);
  g_assert(buf[1] == '\0');
  g_assert_cmpstr(buf, ==, "a");
  res = g_snprintf(buf, 3, "abc");
  g_assert_cmpint(res, ==, 3);
  g_assert(buf[2] == '\0');
  g_assert_cmpstr(buf, ==, "ab");
  res = g_snprintf(buf, 4, "abc");
  g_assert_cmpint(res, ==, 3);
  g_assert(buf[3] == '\0');
  g_assert_cmpstr(buf, ==, "abc");
  res = g_snprintf(buf, 5, "abc");
  g_assert_cmpint(res, ==, 3);
  g_assert(buf[3] == '\0');
  g_assert_cmpstr(buf, ==, "abc");
}
static void test_d(void) {
  gchar buf[128];
  gint res;
  const gchar *fmt;
  res = g_snprintf(buf, 128, "%d", 5);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "5");
  res = g_snprintf(buf, 128, "%d", 0);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "0");
  res = g_snprintf(buf, 128, "%.0d", 0);
  g_assert_cmpint(res, ==, 0);
  g_assert_cmpstr(buf, ==, "");
  res = g_snprintf(buf, 128, "%.0d", 1);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "1");
  res = g_snprintf(buf, 128, "%.d", 2);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "2");
  res = g_snprintf(buf, 128, "%d", -1);
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, "-1");
  res = g_snprintf(buf, 128, "%.3d", 5);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "005");
  res = g_snprintf(buf, 128, "%.3d", -5);
  g_assert_cmpint(res, ==, 4);
  g_assert_cmpstr(buf, ==, "-005");
  res = g_snprintf(buf, 128, "%5.3d", 5);
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "  005");
  res = g_snprintf(buf, 128, "%-5.3d", -5);
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "-005 ");
  res = g_snprintf(buf, 128, "%" G_GINT16_FORMAT, (gint16)-5);
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, "-5");
  res = g_snprintf(buf, 128, "%" G_GUINT16_FORMAT, (guint16)5);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "5");
  res = g_snprintf(buf, 128, "%" G_GINT32_FORMAT, (gint32)-5);
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, "-5");
  res = g_snprintf(buf, 128, "%" G_GUINT32_FORMAT, (guint32)5);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "5");
  res = g_snprintf(buf, 128, "%" G_GINT64_FORMAT, (gint64)-5);
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, "-5");
  res = g_snprintf(buf, 128, "%" G_GUINT64_FORMAT, (guint64)5);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "5");
  res = g_snprintf(buf, 128, "%" G_GSSIZE_FORMAT, (gssize)-5);
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, "-5");
  res = g_snprintf(buf, 128, "%" G_GSIZE_FORMAT, (gsize)5);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "5");
  res = g_snprintf(buf, 128, "%-d", 5);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "5");
  res = g_snprintf(buf, 128, "%-+d", 5);
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, "+5");
  res = g_snprintf(buf, 128, "%+-d", 5);
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, "+5");
  res = g_snprintf(buf, 128, "%+d", -5);
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, "-5");
  res = g_snprintf(buf, 128, "% d", 5);
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, " 5");
  res = g_snprintf(buf, 128, "% .0d", 0);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, " ");
  res = g_snprintf(buf, 128, "%03d", 5);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "005");
  res = g_snprintf(buf, 128, "%03d", -5);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "-05");
  fmt = "% +d";
  res = g_snprintf(buf, 128, fmt, 5);
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, "+5");
  fmt = "%-03d";
  res = g_snprintf(buf, 128, fmt, -5);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "-5 ");
}
static void test_o(void) {
  gchar buf[128];
  gint res;
  res = g_snprintf(buf, 128, "%o", 5);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "5");
  res = g_snprintf(buf, 128, "%o", 8);
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, "10");
  res = g_snprintf(buf, 128, "%o", 0);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "0");
  res = g_snprintf(buf, 128, "%.0o", 0);
  g_assert_cmpint(res, ==, 0);
  g_assert_cmpstr(buf, ==, "");
  res = g_snprintf(buf, 128, "%.0o", 1);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "1");
  res = g_snprintf(buf, 128, "%.3o", 5);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "005");
  res = g_snprintf(buf, 128, "%.3o", 8);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "010");
  res = g_snprintf(buf, 128, "%5.3o", 5);
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "  005");
}
static void test_u(void) {
  gchar buf[128];
  gint res;
  res = g_snprintf(buf, 128, "%u", 5);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "5");
  res = g_snprintf(buf, 128, "%u", 0);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "0");
  res = g_snprintf(buf, 128, "%.0u", 0);
  g_assert_cmpint(res, ==, 0);
  g_assert_cmpstr(buf, ==, "");
  res = g_snprintf(buf, 128, "%.0u", 1);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "1");
  res = g_snprintf(buf, 128, "%.3u", 5);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "005");
  res = g_snprintf(buf, 128, "%5.3u", 5);
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "  005");
}
static void test_x(void) {
  gchar buf[128];
  gint res;
  res = g_snprintf(buf, 128, "%x", 5);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "5");
  res = g_snprintf(buf, 128, "%x", 31);
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, "1f");
  res = g_snprintf(buf, 128, "%x", 0);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "0");
  res = g_snprintf(buf, 128, "%.0x", 0);
  g_assert_cmpint(res, ==, 0);
  g_assert_cmpstr(buf, ==, "");
  res = g_snprintf(buf, 128, "%.0x", 1);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "1");
  res = g_snprintf(buf, 128, "%.3x", 5);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "005");
  res = g_snprintf(buf, 128, "%.3x", 31);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "01f");
  res = g_snprintf(buf, 128, "%5.3x", 5);
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "  005");
  res = g_snprintf(buf, 128, "%-x", 5);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "5");
  res = g_snprintf(buf, 128, "%03x", 5);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "005");
  res = g_snprintf(buf, 128, "%#x", 31);
  g_assert_cmpint(res, ==, 4);
  g_assert_cmpstr(buf, ==, "0x1f");
  res = g_snprintf(buf, 128, "%#x", 0);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "0");
}
static void test_X(void) {
  gchar buf[128];
  gint res;
  res = g_snprintf(buf, 128, "%X", 5);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "5");
  res = g_snprintf(buf, 128, "%X", 31);
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, "1F");
  res = g_snprintf(buf, 128, "%X", 0);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "0");
  res = g_snprintf(buf, 128, "%.0X", 0);
  g_assert_cmpint(res, ==, 0);
  g_assert_cmpstr(buf, ==, "");
  res = g_snprintf(buf, 128, "%.0X", 1);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "1");
  res = g_snprintf(buf, 128, "%.3X", 5);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "005");
  res = g_snprintf(buf, 128, "%.3X", 31);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "01F");
  res = g_snprintf(buf, 128, "%5.3X", 5);
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "  005");
  res = g_snprintf(buf, 128, "%-X", 5);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "5");
  res = g_snprintf(buf, 128, "%03X", 5);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "005");
  res = g_snprintf(buf, 128, "%#X", 31);
  g_assert_cmpint(res, ==, 4);
  g_assert_cmpstr(buf, ==, "0X1F");
  res = g_snprintf(buf, 128, "%#X", 0);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "0");
}
static void test_f(void) {
  gchar buf[128];
  gint res;
  res = g_snprintf(buf, 128, "%f", G_PI);
  g_assert_cmpint(res, ==, 8);
  g_assert(0 == strncmp(buf, "3.14159", 7));
  res = g_snprintf(buf, 128, "%.8f", G_PI);
  g_assert_cmpint(res, ==, 10);
  g_assert(0 == strncmp(buf, "3.1415926", 9));
  res = g_snprintf(buf, 128, "%.0f", G_PI);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "3");
  res = g_snprintf(buf, 128, "%1.f", G_PI);
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "3");
  res = g_snprintf(buf, 128, "%3.f", G_PI);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "  3");
  res = g_snprintf(buf, 128, "%+f", G_PI);
  g_assert_cmpint(res, ==, 9);
  g_assert(0 == strncmp(buf, "+3.14159", 8));
  res = g_snprintf(buf, 128, "% f", G_PI);
  g_assert_cmpint(res, ==, 9);
  g_assert(0 == strncmp(buf, " 3.14159", 8));
  res = g_snprintf(buf, 128, "%#.0f", G_PI);
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, "3.");
  res = g_snprintf(buf, 128, "%05.2f", G_PI);
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "03.14");
}
static gboolean same_value(const gchar *actual, const gchar *expected) {
  gdouble actual_value, expected_value;
  actual_value = g_ascii_strtod(actual, NULL);
  expected_value = g_ascii_strtod(expected, NULL);
  return actual_value == expected_value;
}
static void test_e(void) {
  gchar buf[128];
  gint res;
  res = g_snprintf(buf, 128, "%e", G_PI);
  g_assert_cmpint(res, >=, 12);
  g_assert(same_value(buf, "3.141593e+00"));
  res = g_snprintf(buf, 128, "%.8e", G_PI);
  g_assert_cmpint(res, >=, 14);
  g_assert(same_value(buf, "3.14159265e+00"));
  res = g_snprintf(buf, 128, "%.0e", G_PI);
  g_assert_cmpint(res, >=, 5);
  g_assert(same_value(buf, "3e+00"));
  res = g_snprintf(buf, 128, "%.1e", 0.0);
  g_assert_cmpint(res, >=, 7);
  g_assert(same_value(buf, "0.0e+00"));
  res = g_snprintf(buf, 128, "%.1e", 0.00001);
  g_assert_cmpint(res, >=, 7);
  g_assert(same_value(buf, "1.0e-05"));
  res = g_snprintf (buf, 128, "%.1e", 10000.0);
  g_assert_cmpint(res, >=, 7);
  g_assert(same_value(buf, "1.0e+04"));
  res = g_snprintf (buf, 128, "%+e", G_PI);
  g_assert_cmpint (res, >=, 13);
  g_assert(same_value(buf, "+3.141593e+00"));
  res = g_snprintf(buf, 128, "% e", G_PI);
  g_assert_cmpint(res, >=, 13);
  g_assert(same_value (buf, " 3.141593e+00"));
  res = g_snprintf(buf, 128, "%#.0e", G_PI);
  g_assert_cmpint(res, >=, 6);
  g_assert(same_value (buf, "3.e+00"));
  res = g_snprintf(buf, 128, "%09.2e", G_PI);
  g_assert_cmpint(res, >=, 9);
  g_assert(same_value(buf, "03.14e+00"));
}
static void test_c(void) {
  gchar buf[128];
  gint res;
  res = g_snprintf(buf, 128, "%c", 'a');
  g_assert_cmpint(res, ==, 1);
  g_assert_cmpstr(buf, ==, "a");
}
static void test_s (void) {
  gchar buf[128];
  gint res;
  res = g_snprintf(buf, 128, "%.2s", "abc");
  g_assert_cmpint(res, ==, 2);
  g_assert_cmpstr(buf, ==, "ab");
  res = g_snprintf(buf, 128, "%.6s", "abc");
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "abc");
  res = g_snprintf(buf, 128, "%5s", "abc");
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "  abc");
  res = g_snprintf(buf, 128, "%-5s", "abc");
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "abc  ");
  res = g_snprintf(buf, 128, "%5.2s", "abc");
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "   ab");
  res = g_snprintf(buf, 128, "%*s", 5, "abc");
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "  abc");
#if 0
  res = g_snprintf(buf, 128, "%*s", -5, "abc");
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "abc  ");
#endif
  res = g_snprintf(buf, 128, "%*.*s", 5, 2, "abc");
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "   ab");
}
static void test_n(void) {
  gchar buf[128];
  gint res;
  gint i;
  glong l;
  res = g_snprintf(buf, 128, "abc%n", &i);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "abc");
  g_assert_cmpint(i, ==, 3);
  res = g_snprintf(buf, 128, "abc%ln", &l);
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "abc");
  g_assert_cmpint(l, ==, 3);
}
static void test_percent(void) {
  gchar buf[128];
  gint res;
  res = g_snprintf (buf, 128, "%%");
  g_assert_cmpint (res, ==, 1);
  g_assert_cmpstr (buf, ==, "%");
}
static void test_positional_params(void) {
  gchar buf[128];
  gint res;
  res = g_snprintf(buf, 128, "%2$c %1$c", 'b', 'a');
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "a b");
  res = g_snprintf(buf, 128, "%1$*2$.*3$s", "abc", 5, 2);
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "   ab");
  res = g_snprintf(buf, 128, "%1$s%1$s", "abc");
  g_assert_cmpint(res, ==, 6);
  g_assert_cmpstr(buf, ==, "abcabc");
}
static void test_positional_params2(void) {
  gint res;
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%2$c %1$c", 'b', 'a');
      g_assert_cmpint(res, ==, 3);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*a b*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%1$*2$.*3$s", "abc", 5, 2);
      g_assert_cmpint(res, ==, 5);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*   ab*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%1$s%1$s", "abc");
      g_assert_cmpint(res, ==, 6);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*abcabc*");
}
static void test_positional_params3(void) {
  gchar buf[128];
  gint res;
  res = g_sprintf(buf, "%2$c %1$c", 'b', 'a');
  g_assert_cmpint(res, ==, 3);
  g_assert_cmpstr(buf, ==, "a b");
  res = g_sprintf(buf, "%1$*2$.*3$s", "abc", 5, 2);
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "   ab");
  res = g_sprintf(buf, "%1$s%1$s", "abc");
  g_assert_cmpint(res, ==, 6);
  g_assert_cmpstr(buf, ==, "abcabc");
}
static void test_percent2(void) {
  gint res;
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%%");
      g_assert_cmpint(res, ==, 1);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*%*");
}
static void test_64bit(void) {
  gchar buf[128];
  gint res;
  res = g_snprintf(buf, 128, "%" G_GINT64_FORMAT, (gint64)123456);
  g_assert_cmpint(res, ==, 6);
  g_assert_cmpstr(buf, ==, "123456");
  res = g_snprintf(buf, 128, "%" G_GINT64_FORMAT, (gint64)-123456);
  g_assert_cmpint(res, ==, 7);
  g_assert_cmpstr(buf, ==, "-123456");
  res = g_snprintf(buf, 128, "%" G_GUINT64_FORMAT, (guint64)123456);
  g_assert_cmpint(res, ==, 6);
  g_assert_cmpstr(buf, ==, "123456");
  res = g_snprintf(buf, 128, "%" G_GINT64_MODIFIER "o", (gint64)123456);
  g_assert_cmpint(res, ==, 6);
  g_assert_cmpstr(buf, ==, "361100");
  res = g_snprintf(buf, 128, "%#" G_GINT64_MODIFIER "o", (gint64)123456);
  g_assert_cmpint(res, ==, 7);
  g_assert_cmpstr(buf, ==, "0361100");
  res = g_snprintf(buf, 128, "%" G_GINT64_MODIFIER "x", (gint64)123456);
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "1e240");
  res = g_snprintf(buf, 128, "%#" G_GINT64_MODIFIER "x", (gint64)123456);
  g_assert_cmpint(res, ==, 7);
  g_assert_cmpstr(buf, ==, "0x1e240");
  res = g_snprintf(buf, 128, "%" G_GINT64_MODIFIER "X", (gint64)123456);
  g_assert_cmpint(res, ==, 5);
  g_assert_cmpstr(buf, ==, "1E240");
#ifdef G_OS_WIN32
  res = g_snprintf (buf, 128, "%" "lli", (gint64)123456);
  g_assert_cmpint (res, ==, 6);
  g_assert_cmpstr (buf, ==, "123456");
  res = g_snprintf (buf, 128, "%" "lli", (gint64)-123456);
  g_assert_cmpint (res, ==, 7);
  g_assert_cmpstr (buf, ==, "-123456");
  res = g_snprintf (buf, 128, "%" "llu", (guint64)123456);
  g_assert_cmpint (res, ==, 6);
  g_assert_cmpstr (buf, ==, "123456");
  res = g_snprintf (buf, 128, "%" "ll" "o", (gint64)123456);
  g_assert_cmpint (res, ==, 6);
  g_assert_cmpstr (buf, ==, "361100");
  res = g_snprintf (buf, 128, "%#" "ll" "o", (gint64)123456);
  g_assert_cmpint (res, ==, 7);
  g_assert_cmpstr (buf, ==, "0361100");
  res = g_snprintf (buf, 128, "%" "ll" "x", (gint64)123456);
  g_assert_cmpint (res, ==, 5);
  g_assert_cmpstr (buf, ==, "1e240");
  res = g_snprintf (buf, 128, "%#" "ll" "x", (gint64)123456);
  g_assert_cmpint (res, ==, 7);
  g_assert_cmpstr (buf, ==, "0x1e240");
  res = g_snprintf (buf, 128, "%" "ll" "X", (gint64)123456);
  g_assert_cmpint (res, ==, 5);
  g_assert_cmpstr (buf, ==, "1E240");
#endif
}
static void test_64bit2(void) {
  gint res;
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%" G_GINT64_FORMAT, (gint64)123456);
      g_assert_cmpint(res, ==, 6);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*123456*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%" G_GINT64_FORMAT, (gint64)-123456);
      g_assert_cmpint(res, ==, 7);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*-123456*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%" G_GUINT64_FORMAT, (guint64)123456);
      g_assert_cmpint(res, ==, 6);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*123456*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%" G_GINT64_MODIFIER "o", (gint64)123456);
      g_assert_cmpint(res, ==, 6);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*361100*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%#" G_GINT64_MODIFIER "o", (gint64)123456);
      g_assert_cmpint(res, ==, 7);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*0361100*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%" G_GINT64_MODIFIER "x", (gint64)123456);
      g_assert_cmpint(res, ==, 5);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*1e240*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%#" G_GINT64_MODIFIER "x", (gint64)123456);
      g_assert_cmpint(res, ==, 7);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*0x1e240*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%" G_GINT64_MODIFIER "X", (gint64)123456);
      g_assert_cmpint(res, ==, 5);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*1E240*");
#ifdef G_OS_WIN32
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%" "lli", (gint64)123456);
      g_assert_cmpint(res, ==, 6);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*123456*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%" "lli", (gint64)-123456);
      g_assert_cmpint(res, ==, 7);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*-123456*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%" "llu", (guint64)123456);
      g_assert_cmpint(res, ==, 6);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*123456*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%" "ll" "o", (gint64)123456);
      g_assert_cmpint(res, ==, 6);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*361100*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%#" "ll" "o", (gint64)123456);
      g_assert_cmpint(res, ==, 7);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*0361100*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%" "ll" "x", (gint64)123456);
      g_assert_cmpint(res, ==, 5);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*1e240*");
  if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf("%#" "ll" "x", (gint64)123456);
      g_assert_cmpint(res, ==, 7);
      exit(0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*0x1e240*");
  if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT)) {
      res = g_printf ("%" "ll" "X", (gint64)123456);
      g_assert_cmpint (res, ==, 5);
      exit (0);
  }
  g_test_trap_assert_passed();
  g_test_trap_assert_stdout("*1E240*");
#endif
}
int main(int argc, char *argv[]) {
  g_test_init(&argc, &argv, NULL);
  g_test_add_func("/snprintf/retval-and-trunc", test_retval_and_trunc);
  g_test_add_func("/snprintf/%d", test_d);
  g_test_add_func("/snprintf/%o", test_o);
  g_test_add_func("/snprintf/%u", test_u);
  g_test_add_func("/snprintf/%x", test_x);
  g_test_add_func("/snprintf/%X", test_X);
  g_test_add_func("/snprintf/%f", test_f);
  g_test_add_func("/snprintf/%e", test_e);
  g_test_add_func("/snprintf/%c", test_c);
  g_test_add_func("/snprintf/%s", test_s);
  g_test_add_func("/snprintf/%n", test_n);
  g_test_add_func("/snprintf/test-percent", test_percent);
  g_test_add_func("/snprintf/test-positional-params", test_positional_params);
  g_test_add_func("/snprintf/test-64bit", test_64bit);
  g_test_add_func("/printf/test-percent", test_percent2);
  g_test_add_func("/printf/test-positional-params", test_positional_params2);
  g_test_add_func("/printf/test-64bit", test_64bit2);
  g_test_add_func("/sprintf/test-positional-params", test_positional_params3);
  return g_test_run();
}