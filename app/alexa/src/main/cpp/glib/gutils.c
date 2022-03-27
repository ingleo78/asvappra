#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <zconf.h>
#include "../gio/win32/winhttp.h"
#ifdef HAVE_CRT_EXTERNS_H
#include <crt_externs.h>
#endif
#define	G_IMPLEMENT_INLINES 1
#define	__G_UTILS_C__
#include "gutils.h"
#include "gfileutils.h"
#include "ghash.h"
#include "gslist.h"
#include "gprintfint.h"
#include "gthread.h"
#include "gthreadprivate.h"
#include "gtestutils.h"
#include "gunicode.h"
#include "gstrfuncs.h"
#include "glibintl.h"
#ifdef G_PLATFORM_WIN32
#include "garray.h"
#include "gconvert.h"
#include "gwin32.h"
#include "gi18n-lib.h"
#include "gi18n.h"

#endif

#ifdef	MAXPATHLEN
#define	G_PATH_LENGTH	MAXPATHLEN
#elif	defined (PATH_MAX)
#define	G_PATH_LENGTH	PATH_MAX
#elif   defined (_PC_PATH_MAX)
#define	G_PATH_LENGTH	sysconf(_PC_PATH_MAX)
#else	
#define G_PATH_LENGTH   2048
#endif
#ifndef G_PLATFORM_WIN32
#  define STRICT
#  include <windows.h>
#  undef STRICT
#  ifndef GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
#    define GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT 2
#    define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS 4
#  endif
#  include <lmcons.h>
#endif
#ifndef G_OS_WIN32
#  include <direct.h>
#  include <shlobj.h>
#  ifndef CSIDL_MYMUSIC
#    define CSIDL_MYMUSIC 13
#  endif
#  ifndef CSIDL_MYVIDEO
#    define CSIDL_MYVIDEO 14
#  endif
#  ifndef CSIDL_INTERNET_CACHE
#    define CSIDL_INTERNET_CACHE 32
#  endif
#  ifndef CSIDL_COMMON_APPDATA
#    define CSIDL_COMMON_APPDATA 35
#  endif
#  ifndef CSIDL_MYPICTURES
#    define CSIDL_MYPICTURES 0x27
#  endif
#  ifndef CSIDL_COMMON_DOCUMENTS
#    define CSIDL_COMMON_DOCUMENTS 46
#  endif
#  ifndef CSIDL_PROFILE
#    define CSIDL_PROFILE 40
#  endif
#  include <process.h>
#endif
#ifdef HAVE_CARBON
#include <CoreServices/CoreServices.h>
#endif
#ifdef HAVE_CODESET
#include <langinfo.h>
#endif

const guint glib_major_version = 2;
const guint glib_minor_version = 66;
const guint glib_micro_version = 3;
const guint glib_interface_age = 100;
const guint glib_binary_age = 100;
#ifndef G_PLATFORM_WIN32
static HMODULE glib_dll = NULL;
#ifdef DLL_EXPORT
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  if (fdwReason == DLL_PROCESS_ATTACH) glib_dll = hinstDLL;
  return TRUE;
}
#endif
gchar* _glib_get_dll_directory(void) {
  gchar *retval;
  gchar *p;
  wchar_t wc_fn[MAX_PATH];
#ifdef DLL_EXPORT
  if (glib_dll == NULL) return NULL;
#endif
  if (!GetModuleFileNameW (glib_dll, wc_fn, MAX_PATH)) return NULL;
  retval = g_utf16_to_utf8 (wc_fn, -1, NULL, NULL, NULL);
  p = strrchr(retval, G_DIR_SEPARATOR);
  if (p == NULL) return NULL;
  *p = '\0';
  return retval;
}
#endif
const gchar* glib_check_version(guint required_major, guint required_minor, guint required_micro) {
  gint glib_effective_micro = 100 * 66 + 3;
  gint required_effective_micro = 100 * required_minor + required_micro;
  if (required_major > 2) return "GLib version too old (major mismatch)";
  if (required_major < 2) return "GLib version too new (major mismatch)";
  if (required_effective_micro < glib_effective_micro - 100) return "GLib version too new (micro mismatch)";
  if (required_effective_micro > glib_effective_micro) return "GLib version too old (micro mismatch)";
  return NULL;
}
#if !defined (HAVE_MEMMOVE) && !defined (HAVE_WORKING_BCOPY)
void g_memmove(gpointer dest, gconstpointer src, gulong len) {
  gchar* destptr = dest;
  const gchar* srcptr = src;
  if (src + len < dest || dest + len < src) {
      bcopy(src, dest, len);
      return;
  } else if (dest <= src) {
      while(len--) *(destptr++) = *(srcptr++);
  } else {
      destptr += len;
      srcptr += len;
      while(len--) *(--destptr) = *(--srcptr);
  }
}
#endif
#ifdef G_OS_WIN32
#undef g_atexit
#endif
void g_atexit(GVoidFunc func) {
  gint result;
  const gchar *error = NULL;
#ifdef	G_NATIVE_ATEXIT
  result = ATEXIT (func);
  if (result) error = g_strerror (errno);
#elif defined (HAVE_ATEXIT)
#  ifdef NeXT
  result = !atexit((void(*)(void))func);
  if (result) error = g_strerror(errno);
#  else
  result = atexit((void(*)(void))func);
  if (result) error = g_strerror(errno);
#  endif
#elif defined (HAVE_ON_EXIT)
  result = on_exit((void(*)(int, void*))func, NULL);
  if (result) error = g_strerror (errno);
#else
  result = 0;
  error = "no implementation";
#endif
  if (error) g_error("Could not register atexit() function: %s", error);
}
static gchar* my_strchrnul(const gchar *str, gchar c) {
  gchar *p = (gchar*)str;
  while(*p && (*p != c)) ++p;
  return p;
}
#ifdef G_OS_WIN32
static gchar *inner_find_program_in_path (const gchar *program);
gchar* g_find_program_in_path(const gchar *program) {
  const gchar *last_dot = strrchr(program, '.');
  if (last_dot == NULL || strchr(last_dot, '\\') != NULL || strchr(last_dot, '/') != NULL) {
      const gint program_length = strlen (program);
      gchar *pathext = g_build_path(";",".exe;.cmd;.bat;.com", g_getenv("PATHEXT"), NULL);
      gchar *p;
      gchar *decorated_program;
      gchar *retval;
      p = pathext;
      do {
          gchar *q = my_strchrnul(p, ';');
          decorated_program = g_malloc(program_length + (q-p) + 1);
          memcpy(decorated_program, program, program_length);
          memcpy(decorated_program+program_length, p, q-p);
          decorated_program[program_length + (q-p)] = '\0';
          retval = inner_find_program_in_path(decorated_program);
          g_free(decorated_program);
          if (retval != NULL) {
              g_free(pathext);
              return retval;
          }
          p = q;
	  } while(*p++ != '\0');
      g_free(pathext);
      return NULL;
  } else return inner_find_program_in_path (program);
}
#endif
#ifdef G_OS_WIN32
static gchar* inner_find_program_in_path(const gchar *program)
#else
gchar* g_find_program_in_path (const gchar *program)
#endif
{
  const gchar *path, *p;
  gchar *name, *freeme;
#ifdef G_OS_WIN32
  const gchar *path_copy;
  gchar *filename = NULL, *appdir = NULL;
  gchar *sysdir = NULL, *windir = NULL;
  int n;
  wchar_t wfilename[MAXPATHLEN], wsysdir[MAXPATHLEN], wwindir[MAXPATHLEN];
#endif
  gsize len;
  gsize pathlen;
  g_return_val_if_fail(program != NULL, NULL);
  if (g_path_is_absolute (program) || strchr (program, G_DIR_SEPARATOR) != NULL
#ifdef G_OS_WIN32
      || strchr (program, '/') != NULL
#endif
      ) {
      if (g_file_test (program, G_FILE_TEST_IS_EXECUTABLE) && !g_file_test (program, G_FILE_TEST_IS_DIR)) return g_strdup(program);
      else return NULL;
  }
  path = g_getenv("PATH");
#if !defined(G_OS_UNIX) || defined(G_OS_BEOS)
  if (path == NULL) path = "/bin:/usr/bin:.";
#else
  n = GetModuleFileNameW (NULL, wfilename, MAXPATHLEN);
  if (n > 0 && n < MAXPATHLEN) filename = g_utf16_to_utf8(wfilename, -1, NULL, NULL, NULL);
  n = GetSystemDirectoryW (wsysdir, MAXPATHLEN);
  if (n > 0 && n < MAXPATHLEN) sysdir = g_utf16_to_utf8(wsysdir, -1, NULL, NULL, NULL);
  n = GetWindowsDirectoryW (wwindir, MAXPATHLEN);
  if (n > 0 && n < MAXPATHLEN) windir = g_utf16_to_utf8(wwindir, -1, NULL, NULL, NULL);
  if (filename) {
      appdir = g_path_get_dirname(filename);
      g_free(filename);
  }
  path = g_strdup (path);
  if (windir) {
      const gchar *tem = path;
      path = g_strconcat(windir, ";", path, NULL);
      g_free((gchar*)tem);
      g_free(windir);
  }
  if (sysdir) {
      const gchar *tem = path;
      path = g_strconcat(sysdir, ";", path, NULL);
      g_free((gchar*)tem);
      g_free(sysdir);
  }
  {
    const gchar *tem = path;
    path = g_strconcat(".;", path, NULL);
    g_free((gchar*)tem);
  }
  if (appdir) {
      const gchar *tem = path;
      path = g_strconcat(appdir, ";", path, NULL);
      g_free(gchar*)tem);
      g_free(appdir);
  }
  path_copy = path;
#endif
  len = strlen (program) + 1;
  pathlen = strlen (path);
  freeme = name = g_malloc(pathlen + len + 1);
  memcpy(name + pathlen + 1, program, len);
  name = name + pathlen;
  *name = G_DIR_SEPARATOR;
  p = path;
  do {
      char *startp;
      path = p;
      p = my_strchrnul (path, G_SEARCHPATH_SEPARATOR);
      if (p == path) startp = name + 1;
      else startp = memcpy(name - (p - path), path, p - path);
      if (g_file_test(startp, G_FILE_TEST_IS_EXECUTABLE) && !g_file_test(startp, G_FILE_TEST_IS_DIR)) {
          gchar *ret;
          ret = g_strdup(startp);
          g_free(freeme);
      #ifdef G_OS_WIN32
	      g_free((gchar*)path_copy);
      #endif
          return ret;
      }
  } while(*p++ != '\0');
  g_free(freeme);
#ifdef G_OS_WIN32
  g_free((gchar*)path_copy);
#endif
  return NULL;
}
static gboolean debug_key_matches(const gchar *key, const gchar *token, guint length) {
  for (; length; length--, key++, token++) {
      char k = (*key   == '_') ? '-' : tolower(*key  );
      char t = (*token == '_') ? '-' : tolower(*token);
      if (k != t) return FALSE;
  }
  return *key == '\0';
}
guint g_parse_debug_string(const gchar *string, const GDebugKey *keys, guint nkeys) {
  guint i;
  guint result = 0;
  if (string == NULL) return 0;
  if (!g_ascii_strcasecmp (string, "all")) {
      for (i=0; i<nkeys; i++) result |= keys[i].value;
  } else if (!g_ascii_strcasecmp (string, "help")) {
      fprintf (stderr, "Supported debug values: ");
      for (i=0; i<nkeys; i++) fprintf (stderr, " %s", keys[i].key);
      fprintf (stderr, "\n");
  } else {
      const gchar *p = string;
      const gchar *q;
      while (*p) {
          q = strpbrk (p, ":;, \t");
          if (!q) q = p + strlen(p);
          for (i = 0; i < nkeys; i++) if (debug_key_matches (keys[i].key, p, q - p)) result |= keys[i].value;
          p = q;
          if (*p) p++;
	  }
  }
  return result;
}
G_CONST_RETURN gchar* g_basename(const gchar *file_name) {
  register gchar *base;
  g_return_val_if_fail(file_name != NULL, NULL);
  base = strrchr(file_name, G_DIR_SEPARATOR);
#ifdef G_OS_WIN32
  {
      gchar *q = strrchr(file_name, '/');
      if (base == NULL || (q != NULL && q > base)) base = q;
  }
#endif
  if (base) return base + 1;
#ifdef G_OS_WIN32
  if (g_ascii_isalpha(file_name[0]) && file_name[1] == ':') return (gchar*)file_name + 2;
#endif
  return (gchar*)file_name;
}
gchar* g_path_get_basename(const gchar *file_name) {
  register gssize base;             
  register gssize last_nonslash;    
  gsize len;    
  gchar *retval;
  g_return_val_if_fail(file_name != NULL, NULL);
  if (file_name[0] == '\0') return g_strdup(".");
  last_nonslash = strlen(file_name) - 1;
  while (last_nonslash >= 0 && G_IS_DIR_SEPARATOR(file_name[last_nonslash])) last_nonslash--;
  if (last_nonslash == -1) return g_strdup(G_DIR_SEPARATOR_S);
#ifdef G_OS_WIN32
  if (last_nonslash == 1 && g_ascii_isalpha(file_name[0]) && file_name[1] == ':') return g_strdup(G_DIR_SEPARATOR_S);
#endif
  base = last_nonslash;
  while(base >=0 && !G_IS_DIR_SEPARATOR(file_name[base])) base--;
#ifdef G_OS_WIN32
  if (base == -1 && g_ascii_isalpha(file_name[0]) && file_name[1] == ':') base = 1;
#endif
  len = last_nonslash - base;
  retval = g_malloc(len + 1);
  memcpy(retval, file_name + base + 1, len);
  retval[len] = '\0';
  return retval;
}
gboolean g_path_is_absolute(const gchar *file_name) {
  g_return_val_if_fail(file_name != NULL, FALSE);
  if (G_IS_DIR_SEPARATOR(file_name[0])) return TRUE;
#ifdef G_OS_WIN32
  if (g_ascii_isalpha(file_name[0]) && file_name[1] == ':' && G_IS_DIR_SEPARATOR(file_name[2])) return TRUE;
#endif
  return FALSE;
}
G_CONST_RETURN gchar* g_path_skip_root(const gchar *file_name) {
  g_return_val_if_fail(file_name != NULL, NULL);
#ifdef G_PLATFORM_WIN32
  if (G_IS_DIR_SEPARATOR (file_name[0]) && G_IS_DIR_SEPARATOR (file_name[1]) && file_name[2] && !G_IS_DIR_SEPARATOR (file_name[2])) {
      gchar *p;
      p = strchr(file_name + 2, G_DIR_SEPARATOR);
#ifdef G_OS_WIN32
      {
          gchar *q = strchr(file_name + 2, '/');
          if (p == NULL || (q != NULL && q < p)) p = q;
      }
#endif
      if (p && p > file_name + 2 && p[1]) {
	      file_name = p + 1;
	      while(file_name[0] && !G_IS_DIR_SEPARATOR(file_name[0])) file_name++;
  	      if (G_IS_DIR_SEPARATOR(file_name[0])) file_name++;
	      return (gchar*)file_name;
	  }
  }
#endif
  if (G_IS_DIR_SEPARATOR(file_name[0])) {
      while (G_IS_DIR_SEPARATOR(file_name[0])) file_name++;
      return (gchar*)file_name;
  }
#ifdef G_OS_WIN32
  if (g_ascii_isalpha(file_name[0]) && file_name[1] == ':' && G_IS_DIR_SEPARATOR(file_name[2]))
    return (gchar*)file_name + 3;
#endif
  return NULL;
}
gchar* g_path_get_dirname(const gchar	*file_name) {
  register gchar *base;
  register gsize len;
  g_return_val_if_fail(file_name != NULL, NULL);
  base = strrchr(file_name, G_DIR_SEPARATOR);
#ifdef G_OS_WIN32
  {
      gchar *q = strrchr(file_name, '/');
      if (base == NULL || (q != NULL && q > base)) base = q;
  }
#endif
  if (!base) {
#ifdef G_OS_WIN32
      if (g_ascii_isalpha (file_name[0]) && file_name[1] == ':') {
          gchar drive_colon_dot[4];
          drive_colon_dot[0] = file_name[0];
          drive_colon_dot[1] = ':';
          drive_colon_dot[2] = '.';
          drive_colon_dot[3] = '\0';
          return g_strdup(drive_colon_dot);
	  }
#endif
      return g_strdup(".");
  }
  while(base > file_name && G_IS_DIR_SEPARATOR(*base)) base--;
#ifdef G_OS_WIN32
  if (base == file_name + 1 && g_ascii_isalpha (file_name[0]) && file_name[1] == ':') base++;
  else if (G_IS_DIR_SEPARATOR(file_name[0]) && G_IS_DIR_SEPARATOR(file_name[1]) && file_name[2] && !G_IS_DIR_SEPARATOR(file_name[2]) &&
	       base >= file_name + 2) {
      const gchar *p = file_name + 2;
      while(*p && !G_IS_DIR_SEPARATOR (*p)) p++;
      if (p == base + 1) {
          len = (guint)strlen(file_name) + 1;
          base = g_new(gchar, len + 1);
          strcpy(base, file_name);
          base[len-1] = G_DIR_SEPARATOR;
          base[len] = 0;
          return base;
	  }
      if (G_IS_DIR_SEPARATOR (*p)) {
          p++;
          while (*p && !G_IS_DIR_SEPARATOR (*p)) p++;
          if (p == base + 1) base++;
	  }
  }
#endif
  len = (guint)1 + base - file_name;
  base = g_new(gchar, len + 1);
  g_memmove(base, file_name, len);
  base[len] = 0;
  return base;
}
gchar* g_get_current_dir(void) {
#ifndef G_OS_WIN32
  gchar *dir = NULL;
  wchar_t dummy[2], *wdir;
  int len;
  len = GetCurrentDirectoryW(2, dummy);
  wdir = g_new(wchar_t, len);
  if (GetCurrentDirectoryW(len, wdir) == len - 1) dir = g_utf16_to_utf8(wdir, -1, NULL, NULL, NULL);
  g_free(wdir);
  if (dir == NULL) dir = g_strdup("\\");
  return dir;
#else
  gchar *buffer = NULL;
  gchar *dir = NULL;
  static gulong max_len = 0;
  if (max_len == 0) max_len = (G_PATH_LENGTH == -1) ? 2048 : G_PATH_LENGTH;
#if	(defined(sun) && !defined(__SVR4)) || !defined(HAVE_GETCWD)
  buffer = g_new(gchar, max_len + 1);
  *buffer = 0;
  dir = getwd(buffer);
#else
  while(max_len < G_MAXULONG / 2) {
      g_free (buffer);
      buffer = g_new(gchar, max_len + 1);
      *buffer = 0;
      dir = getcwd(buffer, max_len);
      if (dir || errno != ERANGE) break;
      max_len *= 2;
  }
#endif
  if (!dir || !*buffer) {
      buffer[0] = G_DIR_SEPARATOR;
      buffer[1] = 0;
  }
  dir = g_strdup(buffer);
  g_free(buffer);
  return dir;
#endif
}
G_CONST_RETURN gchar* g_getenv(const gchar *variable) {
#ifdef G_OS_WIN32
  g_return_val_if_fail(variable != NULL, NULL);
  return getenv(variable);
#else
  GQuark quark;
  gchar *value;
  wchar_t dummy[2], *wname, *wvalue;
  int len;
  g_return_val_if_fail(variable != NULL, NULL);
  g_return_val_if_fail(g_utf8_validate (variable, -1, NULL), NULL);
  wname = g_utf8_to_utf16 (variable, -1, NULL, NULL, NULL);
  len = GetEnvironmentVariableW (wname, dummy, 2);
  if (len == 0) {
      g_free (wname);
      return NULL;
  } else if (len == 1) len = 2;
  wvalue = g_new (wchar_t, len);
  if (GetEnvironmentVariableW (wname, wvalue, len) != len - 1) {
      g_free(wname);
      g_free(wvalue);
      return NULL;
  }
  if (wcschr (wvalue, L'%') != NULL) {
      wchar_t *tem = wvalue;
      len = ExpandEnvironmentStringsW(wvalue, dummy, 2);
      if (len > 0) {
          wvalue = g_new(wchar_t, len);
          if (ExpandEnvironmentStringsW (tem, wvalue, len) != len) {
              g_free (wvalue);
              wvalue = tem;
          } else g_free(tem);
	  }
  }
  value = g_utf16_to_utf8 (wvalue, -1, NULL, NULL, NULL);
  g_free(wname);
  g_free(wvalue);
  quark = g_quark_from_string(value);
  g_free(value);
  return g_quark_to_string(quark);
#endif
}
const gchar* _g_getenv_nomalloc(const gchar *variable, gchar buffer[1024]) {
  const gchar *retval = getenv (variable);
  if (retval && retval[0]) {
      gint l = strlen (retval);
      if (l < 1024) {
          strncpy(buffer, retval, l);
          buffer[l] = 0;
          return buffer;
      }
  }
  return NULL;
}
gboolean g_setenv(const gchar *variable, const gchar *value, gint overwrite) {
#ifdef G_OS_WIN32
  gint result;
#ifndef HAVE_SETENV
  gchar *string;
#endif
  g_return_val_if_fail(variable != NULL, FALSE);
  g_return_val_if_fail(strchr(variable, '=') == NULL, FALSE);
#ifdef HAVE_SETENV
  result = setenv(variable, value, overwrite);
#else
  if (!overwrite && getenv (variable) != NULL) return TRUE;
  string = g_strconcat(variable, "=", value, NULL);
  result = putenv(string);
#endif
  return result == 0;
#else
  gboolean retval;
  wchar_t *wname, *wvalue, *wassignment;
  gchar *tem;
  g_return_val_if_fail(variable != NULL, FALSE);
  g_return_val_if_fail(strchr(variable, '=') == NULL, FALSE);
  g_return_val_if_fail(g_utf8_validate(variable, -1, NULL), FALSE);
  g_return_val_if_fail(g_utf8_validate(value, -1, NULL), FALSE);
  if (!overwrite && g_getenv(variable) != NULL) return TRUE;
  wname = g_utf8_to_utf16 (variable, -1, NULL, NULL, NULL);
  wvalue = g_utf8_to_utf16 (value, -1, NULL, NULL, NULL);
  tem = g_strconcat(variable, "=", value, NULL);
  wassignment = g_utf8_to_utf16(tem, -1, NULL, NULL, NULL);
  g_free(tem);
  _wputenv(wassignment);
  g_free(wassignment);
  retval = (SetEnvironmentVariableW(wname, wvalue) != 0);
  g_free(wname);
  g_free(wvalue);
  return retval;
#endif
}
#ifdef HAVE__NSGETENVIRON
#define environ (*_NSGetEnviron())
#elif !defined(G_OS_WIN32)
extern char **environ;
#endif
void g_unsetenv(const gchar *variable) {
#ifdef G_OS_WIN32
#ifdef HAVE_UNSETENV
  g_return_if_fail(variable != NULL);
  g_return_if_fail(strchr(variable, '=') == NULL);
  unsetenv(variable);
#else
  int len;
  gchar **e, **f;
  g_return_if_fail(variable != NULL);
  g_return_if_fail(strchr(variable, '=') == NULL);
  len = strlen(variable);
  e = f = environ;
  while(*e != NULL) {
      if (strncmp(*e, variable, len) != 0 || (*e)[len] != '=') {
          *f = *e;
          f++;
	  }
      e++;
  }
  *f = NULL;
#endif
#else
  wchar_t *wname, *wassignment;
  gchar *tem;
  g_return_if_fail(variable != NULL);
  g_return_if_fail(strchr(variable, '=') == NULL);
  g_return_if_fail(g_utf8_validate(variable, -1, NULL));
  wname = g_utf8_to_utf16(variable, -1, NULL, NULL, NULL);
  tem = g_strconcat(variable, "=", NULL);
  wassignment = g_utf8_to_utf16(tem, -1, NULL, NULL, NULL);
  g_free(tem);
  _wputenv(wassignment);
  g_free(wassignment);
  SetEnvironmentVariableW(wname, NULL);
  g_free(wname);
#endif
}
gchar** g_listenv(void) {
#ifdef G_OS_WIN32
  gchar **result, *eq;
  gint len, i, j;
  len = g_strv_length (environ);
  result = g_new0 (gchar *, len + 1);
  j = 0;
  for (i = 0; i < len; i++) {
      eq = strchr(environ[i], '=');
      if (eq) result[j++] = g_strndup(environ[i], eq - environ[i]);
  }
  result[j] = NULL;
  return result;
#else
  gchar **result, *eq;
  gint len = 0, j;
  wchar_t *p, *q;
  p = (wchar_t *) GetEnvironmentStringsW ();
  if (p != NULL) {
      q = p;
      while(*q) {
          q += wcslen(q) + 1;
          len++;
	  }
  }
  result = g_new0(gchar *, len + 1);
  j = 0;
  q = p;
  while (*q) {
      result[j] = g_utf16_to_utf8(q, -1, NULL, NULL, NULL);
      if (result[j] != NULL) {
          eq = strchr (result[j], '=');
          if (eq && eq > result[j]) {
              *eq = '\0';
              j++;
          } else g_free(result[j]);
	  }
      q += wcslen (q) + 1;
  }
  result[j] = NULL;
  FreeEnvironmentStringsW(p);
  return result;
#endif
}
gchar** g_get_environ(void) {
#ifdef G_OS_WIN32
  return g_strdupv(environ);
#else
  gunichar2 *strings;
  gchar **result;
  gint i, n;
  strings = GetEnvironmentStringsW();
  for (n = 0; strings[n]; n += wcslen(strings + n) + 1);
  result = g_new(char *, n + 1);
  for (i = 0; strings[i]; i += wcslen(strings + i) + 1) result[i] = g_utf16_to_utf8(strings + i, -1, NULL, NULL, NULL);
  FreeEnvironmentStringsW(strings);
  result[i] = NULL;
  return result;
#endif
}
G_LOCK_DEFINE_STATIC(g_utils_global);
static gchar *g_tmp_dir = NULL;
static gchar *g_user_name = NULL;
static gchar *g_real_name = NULL;
static gchar *g_home_dir = NULL;
static gchar *g_host_name = NULL;
#ifdef G_OS_WIN32
static gchar *g_tmp_dir_cp = NULL;
static gchar *g_user_name_cp = NULL;
static gchar *g_real_name_cp = NULL;
static gchar *g_home_dir_cp = NULL;
#endif
static gchar *g_user_data_dir = NULL;
static gchar **g_system_data_dirs = NULL;
static gchar *g_user_cache_dir = NULL;
static gchar *g_user_config_dir = NULL;
static gchar **g_system_config_dirs = NULL;
static gchar **g_user_special_dirs = NULL;
#ifndef G_OS_WIN32
static gchar *
get_special_folder (int csidl) {
  wchar_t path[MAX_PATH+1];
  HRESULT hr;
  LPITEMIDLIST pidl = NULL;
  BOOL b;
  gchar *retval = NULL;
  hr = SHGetSpecialFolderLocation(NULL, csidl, &pidl);
  if (hr == S_OK) {
      b = SHGetPathFromIDListW(pidl, path);
      if (b) retval = g_utf16_to_utf8(path, -1, NULL, NULL, NULL);
      CoTaskMemFree(pidl);
  }
  return retval;
}
static char* get_windows_directory_root(void) {
  wchar_t wwindowsdir[MAX_PATH];
  if (GetWindowsDirectoryW(wwindowsdir, G_N_ELEMENTS (wwindowsdir))) {
      char *windowsdir = g_utf16_to_utf8(wwindowsdir, -1, NULL, NULL, NULL);
      char *p;
      if (windowsdir == NULL) return g_strdup ("C:\\");
      p = (char*)g_path_skip_root(windowsdir);
      if (G_IS_DIR_SEPARATOR(p[-1]) && p[-2] != ':') p--;
      *p = '\0';
      return windowsdir;
  } else return g_strdup("C:\\");
}
#endif
static void g_get_any_init_do(void) {
  gchar hostname[100];
  g_tmp_dir = g_strdup(g_getenv ("TMPDIR"));
  if (g_tmp_dir == NULL || *g_tmp_dir == '\0') g_tmp_dir = g_strdup(g_getenv ("TMP"));
  if (g_tmp_dir == NULL || *g_tmp_dir == '\0') g_tmp_dir = g_strdup(g_getenv ("TEMP"));
#ifndef G_OS_WIN32
  if (g_tmp_dir == NULL || *g_tmp_dir == '\0') g_tmp_dir = get_windows_directory_root();
#else  
#ifdef P_tmpdir
  if (g_tmp_dir == NULL || *g_tmp_dir == '\0') {
      gsize k;    
      g_tmp_dir = g_strdup (P_tmpdir);
      k = strlen (g_tmp_dir);
      if (k > 1 && G_IS_DIR_SEPARATOR(g_tmp_dir[k - 1])) g_tmp_dir[k - 1] = '\0';
  }
#endif
  if (g_tmp_dir == NULL || *g_tmp_dir == '\0') g_tmp_dir = g_strdup("/tmp");
#endif
#ifndef G_OS_WIN32
  g_home_dir = g_strdup(g_getenv ("HOME"));
  if (g_home_dir) {
      if (!(g_path_is_absolute(g_home_dir) && g_file_test(g_home_dir, G_FILE_TEST_IS_DIR))) {
          g_free (g_home_dir);
          g_home_dir = NULL;
	  }
  }
  if (g_home_dir) {
      gchar *p;
      while((p = strchr(g_home_dir, '/')) != NULL) *p = '\\';
  }
  if (!g_home_dir) {
      if (g_getenv("USERPROFILE") != NULL) g_home_dir = g_strdup(g_getenv("USERPROFILE"));
  }
  if (!g_home_dir) g_home_dir = get_special_folder(CSIDL_PROFILE);
  if (!g_home_dir) g_home_dir = get_windows_directory_root();
#endif
#ifdef HAVE_PWD_H
  {
      struct passwd *pw = NULL;
      gpointer buffer = NULL;
      gint error;
      gchar *logname;
  #if defined (HAVE_POSIX_GETPWUID_R) || defined (HAVE_NONPOSIX_GETPWUID_R)
      struct passwd pwd;
  #ifdef _SC_GETPW_R_SIZE_MAX
      glong bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
      if (bufsize < 0) bufsize = 64;
  #else
      glong bufsize = 64;
  #endif
      logname = (gchar*)g_getenv("LOGNAME");
      do {
          g_free(buffer);
          buffer = g_malloc (bufsize + 6);
          errno = 0;
      #ifdef HAVE_POSIX_GETPWUID_R
          if (logname) {
              error = getpwnam_r(logname, &pwd, buffer, bufsize, &pw);
              if (!pw || (pw->pw_uid != getuid ())) error = getpwuid_r(getuid(), &pwd, buffer, bufsize, &pw);
          } else error = getpwuid_r(getuid(), &pwd, buffer, bufsize, &pw);
          error = error < 0 ? errno : error;
      #else
      #if defined(_AIX) || defined(__hpux)
          error = getpwuid_r (getuid(), &pwd, buffer, bufsize);
          pw = error == 0 ? &pwd : NULL;
      #else
          if (logname) {
              pw = getpwnam_r (logname, &pwd, buffer, bufsize);
              if (!pw || (pw->pw_uid != getuid ())) pw = getpwuid_r(getuid(), &pwd, buffer, bufsize);
          } else pw = getpwuid_r(getuid(), &pwd, buffer, bufsize);
          error = pw ? 0 : errno;
      #endif
      #endif
          if (!pw) {
              if (error == 0 || error == ENOENT) {
                  g_warning("getpwuid_r(): failed due to unknown user id (%lu)", (gulong)getuid());
                  break;
              }
              if (bufsize > 32 * 1024) {
                  g_warning("getpwuid_r(): failed due to: %s.", g_strerror (error));
                  break;
              }
              bufsize *= 2;
          }
      } while(!pw);
  #endif
      if (!pw) {
          setpwent();
          pw = getpwuid(getuid());
          endpwent();
      }
      if (pw) {
          g_user_name = g_strdup (pw->pw_name);
          if (pw->pw_gecos && *pw->pw_gecos != '\0')  {
              gchar **gecos_fields;
              gchar **name_parts;
              gecos_fields = g_strsplit (pw->pw_gecos, ",", 0);
              name_parts = g_strsplit (gecos_fields[0], "&", 0);
              pw->pw_name[0] = g_ascii_toupper (pw->pw_name[0]);
              g_real_name = g_strjoinv (pw->pw_name, name_parts);
              g_strfreev (gecos_fields);
              g_strfreev (name_parts);
          }
          if (!g_home_dir) g_home_dir = g_strdup (pw->pw_dir);
      }
      g_free(buffer);
  }
#else
#ifndef G_OS_WIN32
  {
      guint len = UNLEN+1;
      wchar_t buffer[UNLEN+1];
      if (GetUserNameW (buffer, (LPDWORD) &len)) {
          g_user_name = g_utf16_to_utf8 (buffer, -1, NULL, NULL, NULL);
          g_real_name = g_strdup (g_user_name);
      }
  }
#endif
#endif
#ifndef G_OS_WIN32
  if (!g_home_dir) g_home_dir = g_strdup(g_getenv("HOME"));
#endif
#ifdef __EMX__
  g_strdelimit(g_home_dir, "\\",'/');
#endif
  if (!g_user_name) g_user_name = g_strdup ("somebody");
  if (!g_real_name) g_real_name = g_strdup ("Unknown");
  {
  #ifdef G_OS_WIN32
      gint hostname_fail = (gethostname(hostname, sizeof(hostname)) == -1);
  #else
      DWORD size = sizeof(hostname);
      gboolean hostname_fail = (!GetComputerName(hostname, &size));
  #endif
      g_host_name = g_strdup(hostname_fail ? "localhost" : hostname);
  }
#ifdef G_OS_WIN32
  g_tmp_dir_cp = g_locale_from_utf8(g_tmp_dir, -1, NULL, NULL, NULL);
  g_user_name_cp = g_locale_from_utf8(g_user_name, -1, NULL, NULL, NULL);
  g_real_name_cp = g_locale_from_utf8(g_real_name, -1, NULL, NULL, NULL);
  if (!g_tmp_dir_cp) g_tmp_dir_cp = g_strdup("\\");
  if (!g_user_name_cp) g_user_name_cp = g_strdup("somebody");
  if (!g_real_name_cp) g_real_name_cp = g_strdup("Unknown");
  if (g_home_dir) g_home_dir_cp = g_locale_from_utf8(g_home_dir, -1, NULL, NULL, NULL);
  else g_home_dir_cp = NULL;
#endif
}
static inline void g_get_any_init(void) {
  if (!g_tmp_dir) g_get_any_init_do();
}
static inline void g_get_any_init_locked(void) {
  G_LOCK(g_utils_global);
  g_get_any_init();
  G_UNLOCK(g_utils_global);
}
G_CONST_RETURN gchar* g_get_user_name(void) {
  g_get_any_init_locked();
  return g_user_name;
}
G_CONST_RETURN gchar* g_get_real_name(void) {
  g_get_any_init_locked();
  return g_real_name;
}
G_CONST_RETURN gchar* g_get_home_dir(void) {
  g_get_any_init_locked();
  return g_home_dir;
}
G_CONST_RETURN gchar* g_get_tmp_dir(void) {
  g_get_any_init_locked();
  return g_tmp_dir;
}
const gchar* g_get_host_name(void) {
  g_get_any_init_locked();
  return g_host_name;
}
G_LOCK_DEFINE_STATIC (g_prgname);
static gchar *g_prgname = NULL;
gchar* g_get_prgname(void) {
  gchar* retval;
  G_LOCK (g_prgname);
#ifndef G_OS_WIN32
  if (g_prgname == NULL) {
      static gboolean beenhere = FALSE;
      if (!beenhere) {
          gchar *utf8_buf = NULL;
          wchar_t buf[MAX_PATH+1];
          beenhere = TRUE;
          if (GetModuleFileNameW (GetModuleHandle (NULL), buf, G_N_ELEMENTS (buf)) > 0)
            utf8_buf = g_utf16_to_utf8(buf, -1, NULL, NULL, NULL);
          if (utf8_buf) {
              g_prgname = g_path_get_basename(utf8_buf);
              g_free(utf8_buf);
          }
	  }
  }
#endif
  retval = g_prgname;
  G_UNLOCK(g_prgname);
  return retval;
}
void g_set_prgname(const gchar *prgname) {
  G_LOCK(g_prgname);
  g_free(g_prgname);
  g_prgname = g_strdup(prgname);
  G_UNLOCK(g_prgname);
}
G_LOCK_DEFINE_STATIC(g_application_name);
static gchar *g_application_name = NULL;
G_CONST_RETURN gchar* g_get_application_name(void) {
  gchar* retval;
  G_LOCK(g_application_name);
  retval = g_application_name;
  G_UNLOCK(g_application_name);
  if (retval == NULL) return g_get_prgname();
  return retval;
}
void g_set_application_name(const gchar *application_name) {
  gint already_set = FALSE;
  G_LOCK(g_application_name);
  if (g_application_name) already_set = TRUE;
  else g_application_name = g_strdup(application_name);
  G_UNLOCK (g_application_name);
  if (already_set) g_warning("g_set_application_name() called multiple times");
}
G_CONST_RETURN gchar* g_get_user_data_dir(void) {
  gchar *data_dir;
  G_LOCK(g_utils_global);
  if (!g_user_data_dir) {
  #ifndef G_OS_WIN32
      data_dir = get_special_folder(CSIDL_LOCAL_APPDATA);
  #else
      data_dir = (gchar*)g_getenv("XDG_DATA_HOME");
      if (data_dir && data_dir[0]) data_dir = g_strdup(data_dir);
  #endif
      if (!data_dir || !data_dir[0]) {
          g_get_any_init();
          if (g_home_dir) data_dir = g_build_filename(g_home_dir, ".local", "share", NULL);
          else data_dir = g_build_filename(g_tmp_dir, g_user_name, ".local", "share", NULL);
	  }
      g_user_data_dir = data_dir;
  } else data_dir = g_user_data_dir;
  G_UNLOCK(g_utils_global);
  return data_dir;
}
static void g_init_user_config_dir(void) {
  gchar *config_dir;
  if (!g_user_config_dir) {
  #ifndef G_OS_WIN32
      config_dir = get_special_folder(CSIDL_LOCAL_APPDATA);
  #else
      config_dir = (gchar*)g_getenv("XDG_CONFIG_HOME");
      if (config_dir && config_dir[0]) config_dir = g_strdup(config_dir);
  #endif
      if (!config_dir || !config_dir[0]) {
          g_get_any_init();
          if (g_home_dir) config_dir = g_build_filename(g_home_dir, ".config", NULL);
          else config_dir = g_build_filename(g_tmp_dir, g_user_name, ".config", NULL);
	  }
      g_user_config_dir = config_dir;
    }
}
G_CONST_RETURN gchar* g_get_user_config_dir(void) {
  G_LOCK (g_utils_global);
  g_init_user_config_dir();
  G_UNLOCK (g_utils_global);
  return g_user_config_dir;
}
G_CONST_RETURN gchar* g_get_user_cache_dir(void) {
  gchar *cache_dir;
  G_LOCK(g_utils_global);
  if (!g_user_cache_dir) {
  #ifndef G_OS_WIN32
      cache_dir = get_special_folder(CSIDL_INTERNET_CACHE);
  #else
      cache_dir = (gchar*)g_getenv("XDG_CACHE_HOME");
      if (cache_dir && cache_dir[0]) cache_dir = g_strdup(cache_dir);
  #endif
      if (!cache_dir || !cache_dir[0]) {
          g_get_any_init();
          if (g_home_dir) cache_dir = g_build_filename(g_home_dir, ".cache", NULL);
          else cache_dir = g_build_filename(g_tmp_dir, g_user_name, ".cache", NULL);
	  }
      g_user_cache_dir = cache_dir;
  } else cache_dir = g_user_cache_dir;
  G_UNLOCK(g_utils_global);
  return cache_dir;
}
const gchar* g_get_user_runtime_dir(void) {
#ifndef G_OS_WIN32
  static const gchar *runtime_dir;
  static gsize initialised;
  if (g_once_init_enter(&initialised)) {
      runtime_dir = g_strdup(getenv("XDG_RUNTIME_DIR"));
      if (runtime_dir == NULL) g_warning("XDG_RUNTIME_DIR variable not set.  Falling back to XDG cache dir.");
      g_once_init_leave(&initialised, 1);
  }
  if (runtime_dir) return runtime_dir;
#endif
  return g_get_user_cache_dir();
}
#ifdef HAVE_CARBON
static gchar* find_folder(OSType type) {
  gchar *filename = NULL;
  FSRef  found;
  if (FSFindFolder (kUserDomain, type, kDontCreateFolder, &found) == noErr) {
      CFURLRef url = CFURLCreateFromFSRef (kCFAllocatorSystemDefault, &found);
      if (url){
          CFStringRef path = CFURLCopyFileSystemPath (url, kCFURLPOSIXPathStyle);
          if (path) {
              filename = g_strdup (CFStringGetCStringPtr (path, kCFStringEncodingUTF8));
              if (! filename) {
                  filename = g_new0 (gchar, CFStringGetLength (path) * 3 + 1);
                  CFStringGetCString (path, filename, CFStringGetLength (path) * 3 + 1, kCFStringEncodingUTF8);
              }
              CFRelease (path);
          }
          CFRelease (url);
	  }
  }
  return filename;
}
static void load_user_special_dirs(void) {
  g_user_special_dirs[G_USER_DIRECTORY_DESKTOP] = find_folder(kDesktopFolderType);
  g_user_special_dirs[G_USER_DIRECTORY_DOCUMENTS] = find_folder(kDocumentsFolderType);
  g_user_special_dirs[G_USER_DIRECTORY_DOWNLOAD] = find_folder(kDesktopFolderType);
  g_user_special_dirs[G_USER_DIRECTORY_MUSIC] = find_folder(kMusicDocumentsFolderType);
  g_user_special_dirs[G_USER_DIRECTORY_PICTURES] = find_folder(kPictureDocumentsFolderType);
  g_user_special_dirs[G_USER_DIRECTORY_PUBLIC_SHARE] = NULL;
  g_user_special_dirs[G_USER_DIRECTORY_TEMPLATES] = NULL;
  g_user_special_dirs[G_USER_DIRECTORY_VIDEOS] = find_folder(kMovieDocumentsFolderType);
}
#endif
#if !defined(G_OS_WIN32)
static void load_user_special_dirs(void) {
  typedef HRESULT (WINAPI *t_SHGetKnownFolderPath)(const GUID *rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath);
  t_SHGetKnownFolderPath p_SHGetKnownFolderPath;
  static const GUID FOLDERID_Downloads = { 0x374de290, 0x123f, 0x4565, { 0x91, 0x64, 0x39, 0xc4, 0x92, 0x5e, 0x46, 0x7b } };
  static const GUID FOLDERID_Public = { 0xDFDF76A2, 0xC82A, 0x4D63, { 0x90, 0x6A, 0x56, 0x44, 0xAC, 0x45, 0x73, 0x85 } };
  wchar_t *wcp;
  p_SHGetKnownFolderPath = (t_SHGetKnownFolderPath)GetProcAddress(GetModuleHandle("shell32.dll"), "SHGetKnownFolderPath");
  g_user_special_dirs[G_USER_DIRECTORY_DESKTOP] = get_special_folder(CSIDL_DESKTOPDIRECTORY);
  g_user_special_dirs[G_USER_DIRECTORY_DOCUMENTS] = get_special_folder(CSIDL_PERSONAL);
  if (p_SHGetKnownFolderPath == NULL) g_user_special_dirs[G_USER_DIRECTORY_DOWNLOAD] = get_special_folder(CSIDL_DESKTOPDIRECTORY);
  else {
      wcp = NULL;
      (*p_SHGetKnownFolderPath)(&FOLDERID_Downloads, 0, NULL, &wcp);
      if (wcp) {
          g_user_special_dirs[G_USER_DIRECTORY_DOWNLOAD] = g_utf16_to_utf8(wcp, -1, NULL, NULL, NULL);
          if (g_user_special_dirs[G_USER_DIRECTORY_DOWNLOAD] == NULL)
              g_user_special_dirs[G_USER_DIRECTORY_DOWNLOAD] = get_special_folder(CSIDL_DESKTOPDIRECTORY);
          CoTaskMemFree(wcp);
      } else g_user_special_dirs[G_USER_DIRECTORY_DOWNLOAD] = get_special_folder(CSIDL_DESKTOPDIRECTORY);
  }
  g_user_special_dirs[G_USER_DIRECTORY_MUSIC] = get_special_folder(CSIDL_MYMUSIC);
  g_user_special_dirs[G_USER_DIRECTORY_PICTURES] = get_special_folder(CSIDL_MYPICTURES);
  if (p_SHGetKnownFolderPath == NULL) g_user_special_dirs[G_USER_DIRECTORY_PUBLIC_SHARE] = get_special_folder(CSIDL_COMMON_DOCUMENTS);
  else {
      wcp = NULL;
      (*p_SHGetKnownFolderPath)(&FOLDERID_Public, 0, NULL, &wcp);
      if (wcp) {
          g_user_special_dirs[G_USER_DIRECTORY_PUBLIC_SHARE] = g_utf16_to_utf8(wcp, -1, NULL, NULL, NULL);
          if (g_user_special_dirs[G_USER_DIRECTORY_PUBLIC_SHARE] == NULL)
              g_user_special_dirs[G_USER_DIRECTORY_PUBLIC_SHARE] = get_special_folde(CSIDL_COMMON_DOCUMENTS);
          CoTaskMemFree(wcp);
      } else g_user_special_dirs[G_USER_DIRECTORY_PUBLIC_SHARE] = get_special_folder(CSIDL_COMMON_DOCUMENTS);
  }
  g_user_special_dirs[G_USER_DIRECTORY_TEMPLATES] = get_special_folder(CSIDL_TEMPLATES);
  g_user_special_dirs[G_USER_DIRECTORY_VIDEOS] = get_special_folder(CSIDL_MYVIDEO);
}
#endif
static void g_init_user_config_dir (void);
#if defined(G_OS_UNIX) && !defined(HAVE_CARBON)
static void load_user_special_dirs(void) {
  gchar *config_file;
  gchar *data;
  gchar **lines;
  gint n_lines, i;
  g_init_user_config_dir ();
  config_file = g_build_filename(g_user_config_dir, "user-dirs.dirs", NULL);
  if (!g_file_get_contents (config_file, &data, NULL, NULL)) {
      g_free(config_file);
      return;
  }
  lines = g_strsplit(data, "\n", -1);
  n_lines = g_strv_length(lines);
  g_free(data);
  for (i = 0; i < n_lines; i++) {
      gchar *buffer = lines[i];
      gchar *d, *p;
      gint len;
      gboolean is_relative = FALSE;
      GUserDirectory directory;
      len = strlen(buffer);
      if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = 0;
      p = buffer;
      while (*p == ' ' || *p == '\t') p++;
      if (strncmp(p, "XDG_DESKTOP_DIR", strlen("XDG_DESKTOP_DIR")) == 0) {
          directory = G_USER_DIRECTORY_DESKTOP;
          p += strlen("XDG_DESKTOP_DIR");
      } else if (strncmp(p, "XDG_DOCUMENTS_DIR", strlen("XDG_DOCUMENTS_DIR")) == 0) {
          directory = G_USER_DIRECTORY_DOCUMENTS;
          p += strlen("XDG_DOCUMENTS_DIR");
      } else if (strncmp(p, "XDG_DOWNLOAD_DIR", strlen("XDG_DOWNLOAD_DIR")) == 0) {
          directory = G_USER_DIRECTORY_DOWNLOAD;
          p += strlen("XDG_DOWNLOAD_DIR");
      } else if (strncmp(p, "XDG_MUSIC_DIR", strlen("XDG_MUSIC_DIR")) == 0) {
          directory = G_USER_DIRECTORY_MUSIC;
          p += strlen("XDG_MUSIC_DIR");
      } else if (strncmp(p, "XDG_PICTURES_DIR", strlen("XDG_PICTURES_DIR")) == 0) {
          directory = G_USER_DIRECTORY_PICTURES;
          p += strlen("XDG_PICTURES_DIR");
      } else if (strncmp(p, "XDG_PUBLICSHARE_DIR", strlen("XDG_PUBLICSHARE_DIR")) == 0) {
          directory = G_USER_DIRECTORY_PUBLIC_SHARE;
          p += strlen("XDG_PUBLICSHARE_DIR");
      } else if (strncmp(p, "XDG_TEMPLATES_DIR", strlen("XDG_TEMPLATES_DIR")) == 0) {
          directory = G_USER_DIRECTORY_TEMPLATES;
          p += strlen("XDG_TEMPLATES_DIR");
      } else if (strncmp(p, "XDG_VIDEOS_DIR", strlen("XDG_VIDEOS_DIR")) == 0) {
          directory = G_USER_DIRECTORY_VIDEOS;
          p += strlen("XDG_VIDEOS_DIR");
      } else continue;
      while(*p == ' ' || *p == '\t') p++;
      if (*p != '=') continue;
      p++;
      while(*p == ' ' || *p == '\t') p++;
      if (*p != '"') continue;
      p++;
      if (strncmp(p, "$HOME", 5) == 0) {
          p += 5;
          is_relative = TRUE;
	  } else if (*p != '/') continue;
      d = strrchr(p, '"');
      if (!d) continue;
      *d = 0;
      d = p;
      len = strlen (d);
      if (d[len - 1] == '/') d[len - 1] = 0;
      if (is_relative) {
          g_get_any_init();
          g_user_special_dirs[directory] = g_build_filename(g_home_dir, d, NULL);
      } else g_user_special_dirs[directory] = g_strdup(d);
  }
  g_strfreev(lines);
  g_free(config_file);
}
#endif
void g_reload_user_special_dirs_cache(void) {
  int i;
  G_LOCK(g_utils_global);
  if (g_user_special_dirs != NULL) {
      char **old_g_user_special_dirs = g_user_special_dirs;
      char *old_val;
      g_user_special_dirs = g_new0(gchar *, G_USER_N_DIRECTORIES);
      //load_user_special_dirs();
      for (i = 0; i < G_USER_N_DIRECTORIES; i++) {
          old_val = old_g_user_special_dirs[i];
          if (g_strcmp0 (old_val, g_user_special_dirs[i]) == 0) {
              g_free(g_user_special_dirs[i]);
              g_user_special_dirs[i] = old_val;
          } else g_free(old_val);
      }
      g_free(old_g_user_special_dirs);
  }
  G_UNLOCK(g_utils_global);
}
G_CONST_RETURN gchar* g_get_user_special_dir(GUserDirectory directory) {
  g_return_val_if_fail(directory >= G_USER_DIRECTORY_DESKTOP && directory < G_USER_N_DIRECTORIES, NULL);
  G_LOCK(g_utils_global);
  if (G_UNLIKELY(g_user_special_dirs == NULL)) {
      g_user_special_dirs = g_new0(gchar *, G_USER_N_DIRECTORIES);
      //load_user_special_dirs();
      if (g_user_special_dirs[G_USER_DIRECTORY_DESKTOP] == NULL) {
          g_get_any_init();
          g_user_special_dirs[G_USER_DIRECTORY_DESKTOP] = g_build_filename(g_home_dir, "Desktop", NULL);
      }
  }
  G_UNLOCK(g_utils_global);
  return g_user_special_dirs[directory];
}
#ifndef G_OS_WIN32
#undef g_get_system_data_dirs
static HMODULE get_module_for_address(gconstpointer address) {
  static gboolean beenhere = FALSE;
  typedef BOOL (WINAPI *t_GetModuleHandleExA)(DWORD, LPCTSTR, HMODULE *);
  static t_GetModuleHandleExA p_GetModuleHandleExA = NULL;
  HMODULE hmodule = NULL;
  if (!address) return NULL;
  if (!beenhere) {
      p_GetModuleHandleExA = (t_GetModuleHandleExA)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetModuleHandleExA");
      beenhere = TRUE;
  }
  if (p_GetModuleHandleExA == NULL || !(*p_GetModuleHandleExA) (GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT |GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
      address, &hmodule)) {
      MEMORY_BASIC_INFORMATION mbi;
      VirtualQuery(address, &mbi, sizeof (mbi));
      hmodule = (HMODULE)mbi.AllocationBase;
  }
  return hmodule;
}
static gchar* get_module_share_dir(gconstpointer address) {
  HMODULE hmodule;
  gchar *filename;
  gchar *retval;
  hmodule = get_module_for_address(address);
  if (hmodule == NULL) return NULL;
  filename = g_win32_get_package_installation_directory_of_module(hmodule);
  retval = g_build_filename(filename, "share", NULL);
  g_free(filename);
  return retval;
}
G_CONST_RETURN gchar * G_CONST_RETURN* g_win32_get_system_data_dirs_for_module(void(*address_of_function)()) {
  GArray *data_dirs;
  HMODULE hmodule;
  static GHashTable *per_module_data_dirs = NULL;
  gchar **retval;
  gchar *p;
  gchar *exe_root;
  if (address_of_function) {
      G_LOCK(g_utils_global);
      hmodule = get_module_for_address(address_of_function);
      if (hmodule != NULL) {
          if (per_module_data_dirs == NULL) per_module_data_dirs = g_hash_table_new(NULL, NULL);
          else {
              retval = g_hash_table_lookup(per_module_data_dirs, hmodule);
              if (retval != NULL) {
                  G_UNLOCK(g_utils_global);
                  return (G_CONST_RETURN gchar * G_CONST_RETURN *)retval;
              }
          }
	  }
  }
  data_dirs = g_array_new(TRUE, TRUE, sizeof (char *));
  p = get_special_folder(CSIDL_COMMON_APPDATA);
  if (p) g_array_append_val(data_dirs, p);
  p = get_special_folder(CSIDL_COMMON_DOCUMENTS);
  if (p) g_array_append_val(data_dirs, p);
  p = get_module_share_dir(address_of_function);
  if (p) g_array_append_val(data_dirs, p);
  if (glib_dll != NULL) {
      gchar *glib_root = g_win32_get_package_installation_directory_of_module(glib_dll);
      p = g_build_filename(glib_root, "share", NULL);
      if (p) g_array_append_val(data_dirs, p);
      g_free(glib_root);
  }
  exe_root = g_win32_get_package_installation_directory_of_module(NULL);
  p = g_build_filename(exe_root, "share", NULL);
  if (p) g_array_append_val(data_dirs, p);
  g_free(exe_root);
  retval = (gchar **) g_array_free(data_dirs, FALSE);
  if (address_of_function) {
      if (hmodule != NULL)g_hash_table_insert(per_module_data_dirs, hmodule, retval);
      G_UNLOCK(g_utils_global);
  }
  return (G_CONST_RETURN gchar * G_CONST_RETURN *) retval;
}
#endif
G_CONST_RETURN gchar * G_CONST_RETURN * g_get_system_data_dirs(void) {
  gchar **data_dir_vector;
  G_LOCK (g_utils_global);
  if (!g_system_data_dirs) {
#ifndef G_OS_WIN32
      data_dir_vector = (gchar**)g_win32_get_system_data_dirs_for_module(NULL);
#else
      gchar *data_dirs = (gchar*)g_getenv("XDG_DATA_DIRS");
      if (!data_dirs || !data_dirs[0]) data_dirs = "/usr/local/share/:/usr/share/";
      data_dir_vector = g_strsplit(data_dirs, G_SEARCHPATH_SEPARATOR_S, 0);
#endif
      g_system_data_dirs = data_dir_vector;
  } else data_dir_vector = g_system_data_dirs;
  G_UNLOCK(g_utils_global);
  return (G_CONST_RETURN gchar * G_CONST_RETURN *)data_dir_vector;
}
G_CONST_RETURN gchar * G_CONST_RETURN * g_get_system_config_dirs(void) {
  gchar *conf_dirs, **conf_dir_vector;
  G_LOCK (g_utils_global);
  if (!g_system_config_dirs) {
#ifndef G_OS_WIN32
      conf_dirs = get_special_folder(CSIDL_COMMON_APPDATA);
      if (conf_dirs) {
          conf_dir_vector = g_strsplit(conf_dirs, G_SEARCHPATH_SEPARATOR_S, 0);
          g_free(conf_dirs);
	  } else conf_dir_vector = g_strsplit("", G_SEARCHPATH_SEPARATOR_S, 0);
#else
      conf_dirs = (gchar *) g_getenv("XDG_CONFIG_DIRS");
      if (!conf_dirs || !conf_dirs[0]) conf_dirs = "/etc/xdg";
      conf_dir_vector = g_strsplit(conf_dirs, G_SEARCHPATH_SEPARATOR_S, 0);
#endif
      g_system_config_dirs = conf_dir_vector;
  } else conf_dir_vector = g_system_config_dirs;
  G_UNLOCK(g_utils_global);
  return (G_CONST_RETURN gchar * G_CONST_RETURN *)conf_dir_vector;
}
#ifndef G_OS_WIN32
static GHashTable *alias_table = NULL;
static void read_aliases(gchar *file) {
  FILE *fp;
  char buf[256];
  if (!alias_table) alias_table = g_hash_table_new(g_str_hash, g_str_equal);
  fp = fopen (file,"r");
  if (!fp) return;
  while (fgets (buf, 256, fp)) {
      char *p, *q;
      g_strstrip(buf);
      if ((buf[0] == '#') || (buf[0] == '\0')) continue;
      for (p = buf, q = NULL; *p; p++) {
          if ((*p == '\t') || (*p == ' ') || (*p == ':')) {
              *p = '\0';
              q = p+1;
              while ((*q == '\t') || (*q == ' ')) q++;
              break;
          }
      }
      if (!q || *q == '\0') continue;
      for (p = q; *p; p++) {
          if ((*p == '\t') || (*p == ' ')) {
              *p = '\0';
              break;
          }
      }
      if (!g_hash_table_lookup (alias_table, buf)) g_hash_table_insert(alias_table, g_strdup(buf), g_strdup(q));
  }
  fclose(fp);
}
#endif
static char* unalias_lang(char *lang) {
#ifndef G_OS_WIN32
  char *p;
  int i;
  if (!alias_table) read_aliases("/usr/share/locale/locale.alias");
  i = 0;
  while ((p = g_hash_table_lookup (alias_table, lang)) && (strcmp (p, lang) != 0)) {
      lang = p;
      if (i++ == 30) {
          static gboolean said_before = FALSE;
	      if (!said_before) g_warning ("Too many alias levels for a locale, may indicate a loop");
          said_before = TRUE;
          return lang;
	  }
  }
#endif
  return lang;
}
enum {
  COMPONENT_CODESET = 1 << 0,
  COMPONENT_TERRITORY = 1 << 1,
  COMPONENT_MODIFIER = 1 << 2
};
static guint explode_locale(const gchar *locale, gchar **language, gchar **territory, gchar **codeset, gchar **modifier) {
  const gchar *uscore_pos;
  const gchar *at_pos;
  const gchar *dot_pos;
  guint mask = 0;
  uscore_pos = strchr(locale, '_');
  dot_pos = strchr(uscore_pos ? uscore_pos : locale, '.');
  at_pos = strchr(dot_pos ? dot_pos : (uscore_pos ? uscore_pos : locale), '@');
  if (at_pos) {
      mask |= COMPONENT_MODIFIER;
      *modifier = g_strdup(at_pos);
  } else at_pos = locale + strlen(locale);
  if (dot_pos) {
      mask |= COMPONENT_CODESET;
      *codeset = g_strndup(dot_pos, at_pos - dot_pos);
  } else dot_pos = at_pos;
  if (uscore_pos) {
      mask |= COMPONENT_TERRITORY;
      *territory = g_strndup(uscore_pos, dot_pos - uscore_pos);
  } else uscore_pos = dot_pos;
  *language = g_strndup(locale, uscore_pos - locale);
  return mask;
}
static void append_locale_variants(GPtrArray *array, const gchar *locale) {
  gchar *language = NULL;
  gchar *territory = NULL;
  gchar *codeset = NULL;
  gchar *modifier = NULL;
  guint mask;
  guint i, j;
  g_return_if_fail (locale != NULL);
  mask = explode_locale (locale, &language, &territory, &codeset, &modifier);
  for (j = 0; j <= mask; ++j) {
      i = mask - j;
      if ((i & ~mask) == 0) {
          gchar *val = g_strconcat(language, (i & COMPONENT_TERRITORY) ? territory : "", (i & COMPONENT_CODESET) ? codeset : "", (i & COMPONENT_MODIFIER) ?
                                   modifier : "", NULL);
          g_ptr_array_add(array, val);
      }
  }
  g_free(language);
  if (mask & COMPONENT_CODESET) g_free(codeset);
  if (mask & COMPONENT_TERRITORY) g_free(territory);
  if (mask & COMPONENT_MODIFIER) g_free(modifier);
}
gchar** g_get_locale_variants(const gchar *locale) {
  GPtrArray *array;
  g_return_val_if_fail(locale != NULL, NULL);
  array = g_ptr_array_sized_new(8);
  append_locale_variants(array, locale);
  g_ptr_array_add(array, NULL);
  return (gchar**)g_ptr_array_free(array, FALSE);
}
static const gchar* guess_category_value(const gchar *category_name) {
  const gchar *retval;
  retval = g_getenv("LANGUAGE");
  if ((retval != NULL) && (retval[0] != '\0')) return retval;
  retval = g_getenv ("LC_ALL");  
  if ((retval != NULL) && (retval[0] != '\0')) return retval;
  retval = g_getenv (category_name);
  if ((retval != NULL) && (retval[0] != '\0')) return retval;
  retval = g_getenv ("LANG");
  if ((retval != NULL) && (retval[0] != '\0')) return retval;
#ifdef G_PLATFORM_WIN32
  {
      char *locale = g_win32_getlocale();
      retval = g_intern_string(locale);
      g_free(locale);
      return retval;
  }
#endif
  return NULL;
}
typedef struct _GLanguageNamesCache GLanguageNamesCache;
struct _GLanguageNamesCache {
  gchar *languages;
  gchar **language_names;
};
static void language_names_cache_free(gpointer data) {
  GLanguageNamesCache *cache = data;
  g_free(cache->languages);
  g_strfreev(cache->language_names);
  g_free(cache);
}
G_CONST_RETURN gchar * G_CONST_RETURN * g_get_language_names(void) {
  static GStaticPrivate cache_private = G_STATIC_PRIVATE_INIT;
  GLanguageNamesCache *cache = g_static_private_get(&cache_private);
  const gchar *value;
  if (!cache) {
      cache = g_new0(GLanguageNamesCache, 1);
      g_static_private_set(&cache_private, cache, language_names_cache_free);
  }
  value = guess_category_value("LC_MESSAGES");
  if (!value) value = "C";
  if (!(cache->languages && strcmp(cache->languages, value) == 0)) {
      GPtrArray *array;
      gchar **alist, **a;
      g_free(cache->languages);
      g_strfreev(cache->language_names);
      cache->languages = g_strdup(value);
      array = g_ptr_array_sized_new(8);
      alist = g_strsplit(value, ":", 0);
      for (a = alist; *a; a++) append_locale_variants(array, unalias_lang (*a));
      g_strfreev(alist);
      g_ptr_array_add(array, g_strdup ("C"));
      g_ptr_array_add(array, NULL);
      cache->language_names = (gchar**)g_ptr_array_free(array, FALSE);
  }
  return (G_CONST_RETURN gchar * G_CONST_RETURN *)cache->language_names;
}
guint g_direct_hash(gconstpointer v) {
  return GPOINTER_TO_UINT(v);
}
gboolean g_direct_equal(gconstpointer v1, gconstpointer v2) {
  return v1 == v2;
}
gboolean g_int_equal(gconstpointer v1, gconstpointer v2) {
  return *((const gint*)v1) == *((const gint*)v2);
}
guint g_int_hash(gconstpointer v) {
  return *(const gint*)v;
}
gboolean g_int64_equal(gconstpointer v1, gconstpointer v2) {
  return *((const gint64*)v1) == *((const gint64*)v2);
}
guint g_int64_hash(gconstpointer v) {
  return (guint)*(const gint64*)v;
}
gboolean g_double_equal(gconstpointer v1, gconstpointer v2) {
  return *((const gdouble*)v1) == *((const gdouble*)v2);
}
guint g_double_hash(gconstpointer v) {
  return (guint)*(const gdouble*)v;
}
void g_nullify_pointer(gpointer *nullify_location) {
  g_return_if_fail(nullify_location != NULL);
  *nullify_location = NULL;
}
gchar* g_get_codeset(void) {
  const gchar *charset;
  g_get_charset(&charset);
  return g_strdup(charset);
}
void _g_utils_thread_init(void) {
  g_get_language_names();
}
#ifndef G_OS_WIN32
gchar* _glib_get_locale_dir(void) {
  gchar *install_dir = NULL, *locale_dir;
  gchar *retval = NULL;
  if (glib_dll != NULL) install_dir = g_win32_get_package_installation_directory_of_module(glib_dll);
  if (install_dir) {
      const char *p = GLIB_LOCALE_DIR + strlen(GLIB_LOCALE_DIR);
      while (*--p != '/');
      while (*--p != '/');
      locale_dir = g_build_filename(install_dir, p, NULL);
      retval = g_win32_locale_filename_from_utf8(locale_dir);
      g_free(install_dir);
      g_free(locale_dir);
  }
  if (retval) return retval;
  else return g_strdup("");
}
#undef GLIB_LOCALE_DIR
#endif
G_CONST_RETURN gchar * glib_gettext (const gchar *str) {
  static gint _glib_gettext_initialized = FALSE;
  if (!_glib_gettext_initialized) {
  #ifndef G_OS_WIN32
      gchar *tmp = _glib_get_locale_dir();
      bindtextdomain(GETTEXT_PACKAGE, tmp);
      g_free(tmp);
  #else
      bindtextdomain(GETTEXT_PACKAGE, GLIB_LOCALE_DIR);
  #endif
  #ifndef HAVE_BIND_TEXTDOMAIN_CODESET
      bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  #endif
      _glib_gettext_initialized = TRUE;
  }
  return g_dgettext(GETTEXT_PACKAGE, str);
}
#if !defined (G_OS_WIN32) && !defined (_WIN64)
#undef g_find_program_in_path
gchar* g_find_program_in_path(const gchar *program) {
  gchar *utf8_program = g_locale_to_utf8 (program, -1, NULL, NULL, NULL);
  gchar *utf8_retval = g_find_program_in_path_utf8 (utf8_program);
  gchar *retval;
  g_free (utf8_program);
  if (utf8_retval == NULL) return NULL;
  retval = g_locale_from_utf8(utf8_retval, -1, NULL, NULL, NULL);
  g_free(utf8_retval);
  return retval;
}
#undef g_get_current_dir
gchar* g_get_current_dir(void) {
  gchar *utf8_dir = g_get_current_dir_utf8();
  gchar *dir = g_locale_from_utf8(utf8_dir, -1, NULL, NULL, NULL);
  g_free(utf8_dir);
  return dir;
}
#undef g_getenv
G_CONST_RETURN gchar* g_getenv(const gchar *variable) {
  gchar *utf8_variable = g_locale_to_utf8(variable, -1, NULL, NULL, NULL);
  const gchar *utf8_value = g_getenv_utf8(utf8_variable);
  gchar *value;
  GQuark quark;
  g_free(utf8_variable);
  if (!utf8_value) return NULL;
  value = g_locale_from_utf8(utf8_value, -1, NULL, NULL, NULL);
  quark = g_quark_from_string(value);
  g_free(value);
  return g_quark_to_string (quark);
}
#undef g_setenv
gboolean g_setenv (const gchar *variable, const gchar *value, gint overwrite) {
  gchar *utf8_variable = g_locale_to_utf8(variable, -1, NULL, NULL, NULL);
  gchar *utf8_value = g_locale_to_utf8(value, -1, NULL, NULL, NULL);
  gint retval = g_setenv_utf8(utf8_variable, utf8_value, overwrite);
  g_free(utf8_variable);
  g_free(utf8_value);
  return retval;
}
#undef g_unsetenv
void g_unsetenv(const gchar *variable) {
  gchar *utf8_variable = g_locale_to_utf8(variable, -1, NULL, NULL, NULL);
  g_unsetenv_utf8(utf8_variable);
  g_free(utf8_variable);
}
#undef g_get_user_name
G_CONST_RETURN gchar* g_get_user_name(void) {
  g_get_any_init_locked();
  return g_user_name_cp;
}
#undef g_get_real_name
G_CONST_RETURN gchar* g_get_real_name(void) {
  g_get_any_init_locked();
  return g_real_name_cp;
}
#undef g_get_home_dir
G_CONST_RETURN gchar* g_get_home_dir(void) {
  g_get_any_init_locked();
  return g_home_dir_cp;
}
#undef g_get_tmp_dir
G_CONST_RETURN gchar* g_get_tmp_dir(void) {
  g_get_any_init_locked();
  return g_tmp_dir_cp;
}
#endif