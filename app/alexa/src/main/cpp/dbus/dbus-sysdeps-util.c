#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include "dbus-sysdeps.h"
#include "dbus-internals.h"
#include "dbus-string.h"
#include "dbus-test.h"
#include "dbus-test-tap.h"

extern char **environ;
char **_dbus_get_environment(void) {
  int i, length;
  char **environment;
  _dbus_assert(environ != NULL);
  for (length = 0; environ[length] != NULL; length++);
  length++;
  environment = dbus_new0(char *, length);
  if (environment == NULL) return NULL;
  for (i = 0; environ[i] != NULL; i++) {
      environment[i] = _dbus_strdup(environ[i]);
      if (environment[i] == NULL) break;
  }
  if (environ[i] != NULL) {
      dbus_free_string_array(environment);
      environment = NULL;
  }
  return environment;
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
static void check_dirname(const char *filename, const char *dirname) {
  DBusString f, d;
  _dbus_string_init_const(&f, filename);
  if (!_dbus_string_init(&d)) _dbus_test_fatal("no memory");
  if (!_dbus_string_get_dirname (&f, &d)) _dbus_test_fatal("no memory");
  if (!_dbus_string_equal_c_str(&d, dirname)) {
      _dbus_warn("For filename \"%s\" got dirname \"%s\" and expected \"%s\"", filename, _dbus_string_get_const_data(&d), dirname);
      exit(1);
  }
  _dbus_string_free(&d);
}
static void check_path_absolute(const char *path, dbus_bool_t expected) {
  DBusString p;
  _dbus_string_init_const(&p, path);
  if (_dbus_path_is_absolute(&p) != expected) {
      _dbus_warn("For path \"%s\" expected absolute = %d got %d", path, expected, _dbus_path_is_absolute(&p));
      exit(1);
  }
}
dbus_bool_t _dbus_sysdeps_test(void) {
#ifdef DBUS_WIN
  check_dirname("foo\\bar", "foo");
  check_dirname("foo\\\\bar", "foo");
  check_dirname("foo/\\/bar", "foo");
  check_dirname("foo\\bar/", "foo");
  check_dirname("foo//bar\\", "foo");
  check_dirname("foo\\bar/", "foo");
  check_dirname("foo/bar\\\\", "foo");
  check_dirname("\\foo", "\\");
  check_dirname("\\\\foo", "\\");
  check_dirname("\\", "\\");
  check_dirname("\\\\", "\\");
  check_dirname("\\/", "\\");
  check_dirname("/\\/", "/");
  check_dirname("c:\\foo\\bar", "c:\\foo");
  check_dirname("c:\\foo", "c:\\");
  check_dirname("c:/foo", "c:/");
  check_dirname("c:\\", "c:\\");
  check_dirname("c:/", "c:/");
  check_dirname("", ".");
#else  
  check_dirname("foo", ".");
  check_dirname("foo/bar", "foo");
  check_dirname("foo//bar", "foo");
  check_dirname("foo///bar", "foo");
  check_dirname("foo/bar/", "foo");
  check_dirname("foo//bar/", "foo");
  check_dirname("foo///bar/", "foo");
  check_dirname("foo/bar//", "foo");
  check_dirname("foo//bar////", "foo");
  check_dirname("foo///bar///////", "foo");
  check_dirname("/foo", "/");
  check_dirname("////foo", "/");
  check_dirname("/foo/bar", "/foo");
  check_dirname("/foo//bar", "/foo");
  check_dirname("/foo///bar", "/foo");
  check_dirname("/", "/");
  check_dirname("///", "/");
  check_dirname("", ".");
#endif
#ifdef DBUS_WIN
  check_path_absolute("c:/", TRUE);
  check_path_absolute("c:/foo", TRUE);
  check_path_absolute("", FALSE);
  check_path_absolute("foo", FALSE);
  check_path_absolute("foo/bar", FALSE);
  check_path_absolute("", FALSE);
  check_path_absolute("foo\\bar", FALSE);
  check_path_absolute("c:\\", TRUE);
  check_path_absolute("c:\\foo", TRUE);
  check_path_absolute("c:", TRUE);
  check_path_absolute("c:\\foo\\bar", TRUE);
  check_path_absolute("\\", TRUE);
  check_path_absolute("/", TRUE);
#else  
  check_path_absolute("/", TRUE);
  check_path_absolute("/foo", TRUE);
  check_path_absolute("", FALSE);
  check_path_absolute("foo", FALSE);
  check_path_absolute("foo/bar", FALSE);
#endif
  return TRUE;
}
#endif