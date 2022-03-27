#include <stdarg.h>
#include "../glib.h"

static gboolean strv_check(const gchar * const *strv, ...) {
  va_list args;
  gchar *s;
  gint i;
  va_start(args, strv);
  for (i = 0; strv[i]; i++) {
      s = va_arg(args, gchar*);
      if (g_strcmp0(strv[i], s) != 0) {
          va_end(args);
          return FALSE;
      }
  }
  va_end(args);
  return TRUE;
}
static void test_language_names(void) {
  const gchar * const *names;
  g_setenv("LANGUAGE", "de:en_US", TRUE);
  names = g_get_language_names();
  g_assert(strv_check(names, "de", "en_US", "en", "C", NULL));
  g_setenv("LANGUAGE", "tt_RU.UTF-8@iqtelif", TRUE);
  names = g_get_language_names();
  g_assert(strv_check(names, "tt_RU.UTF-8@iqtelif", "tt_RU@iqtelif", "tt.UTF-8@iqtelif", "tt@iqtelif", "tt_RU.UTF-8", "tt_RU", "tt.UTF-8", "tt", "C", NULL));
}
static void test_locale_variants(void) {
  char **v;
  v = g_get_locale_variants("fr_BE");
  g_assert(strv_check((const gchar * const*)v, "fr_BE", "fr", NULL));
  g_strfreev(v);
  v = g_get_locale_variants("sr_SR@latin");
  g_assert(strv_check((const gchar * const*) v, "sr_SR@latin", "sr@latin", "sr_SR", "sr", NULL));
  g_strfreev(v);
}
static void test_version(void) {
  g_print("(header %d.%d.%d library %d.%d.%d) ", GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION, glib_major_version, glib_minor_version,
          glib_micro_version);
  g_assert(glib_check_version(GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION) == NULL);
  g_assert(glib_check_version(GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, 0) == NULL);
  g_assert(glib_check_version(GLIB_MAJOR_VERSION - 1, 0, 0) != NULL);
  g_assert(glib_check_version(GLIB_MAJOR_VERSION + 1, 0, 0) != NULL);
  g_assert(glib_check_version(GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION + 1, 0) != NULL);
  g_assert(glib_check_version(GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION + 3) != NULL);
}
static const gchar *argv0;
static void test_appname(void) {
  const gchar *prgname;
  const gchar *appname;
  prgname = g_get_prgname();
  appname = g_get_application_name();
  g_assert_cmpstr(prgname, ==, argv0);
  g_assert_cmpstr(appname, ==, prgname);
  g_set_prgname("prgname");
  prgname = g_get_prgname();
  appname = g_get_application_name();
  g_assert_cmpstr(prgname, ==, "prgname");
  g_assert_cmpstr(appname, ==, "prgname");
  g_set_application_name("appname");
  prgname = g_get_prgname();
  appname = g_get_application_name();
  g_assert_cmpstr(prgname, ==, "prgname");
  g_assert_cmpstr(appname, ==, "appname");
}
static void test_tmpdir(void) {
  g_test_bug("627969");
  g_assert_cmpstr(g_get_tmp_dir(), !=, "");
}
static void test_bits(void) {
  gulong mask;
  gint max_bit;
  gint i, pos;
  pos = g_bit_nth_lsf(0, -1);
  g_assert_cmpint(pos, ==, -1);
  max_bit = sizeof(gulong) * 8;
  for (i = 0; i < max_bit; i++) {
      mask = 1UL << i;
      pos = g_bit_nth_lsf(mask, -1);
      g_assert_cmpint(pos, ==, i);
      pos = g_bit_nth_lsf(mask, i - 3);
      g_assert_cmpint(pos , ==, i);
      pos = g_bit_nth_lsf(mask, i);
      g_assert_cmpint(pos , ==, -1);
      pos = g_bit_nth_lsf(mask, i + 1);
      g_assert_cmpint(pos , ==, -1);
  }
  pos = g_bit_nth_msf(0, -1);
  g_assert_cmpint(pos, ==, -1);
  for (i = 0; i < max_bit; i++) {
      mask = 1UL << i;
      pos = g_bit_nth_msf(mask, -1);
      g_assert_cmpint(pos, ==, i);
      pos = g_bit_nth_msf(mask, i + 3);
      g_assert_cmpint(pos , ==, i);
      pos = g_bit_nth_msf(mask, i);
      g_assert_cmpint(pos , ==, -1);
      if (i > 0) {
          pos = g_bit_nth_msf(mask, i - 1);
          g_assert_cmpint(pos , ==, -1);
      }
  }
}
int main(int argc, char *argv[]) {
  argv0 = argv[0];
  g_setenv("TMPDIR", "", TRUE);
  g_unsetenv("TMP");
  g_unsetenv("TEMP");
  g_test_init(&argc, &argv, NULL);
  g_test_bug_base("http://bugzilla.gnome.org/");
  g_test_add_func("/utils/language-names", test_language_names);
  g_test_add_func("/utils/locale-variants", test_locale_variants);
  g_test_add_func("/utils/version", test_version);
  g_test_add_func("/utils/appname", test_appname);
  g_test_add_func("/utils/tmpdir", test_tmpdir);
  g_test_add_func("/utils/bits", test_bits);
  return g_test_run();
}