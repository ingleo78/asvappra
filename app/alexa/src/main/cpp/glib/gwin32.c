#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>
#include "glibconfig.h"
#define STRICT
#undef STRICT
#ifdef G_WITH_CYGWIN
#include <direct.h>
#endif
#include <errno.h>
#include <ctype.h>
#include "../gio/win32/winhttp.h"
#if defined(_MSC_VER) || defined(__DMC__)
#include <io.h>
#endif
#include "glib.h"
#ifdef G_WITH_CYGWIN
#include <sys/cygwin.h>
#endif

#ifdef G_WITH_CYGWIN
gint g_win32_ftruncate(gint fd, guint size) {
  return _chsize(fd, size);
}
#endif
#ifndef SUBLANG_SERBIAN_LATIN_BA
#define SUBLANG_SERBIAN_LATIN_BA 0x06
#endif
gchar *g_win32_getlocale(void) {
  //LCID lcid;
  //LANGID langid;
  gchar *ev;
  gint primary, sub;
  char iso639[10];
  char iso3166[10];
  const gchar *script = NULL;
  if (((ev = getenv("LC_ALL")) != NULL && ev[0] != '\0') || ((ev = getenv("LC_MESSAGES")) != NULL && ev[0] != '\0') ||
      ((ev = getenv("LANG")) != NULL && ev[0] != '\0')) {
      return g_strdup (ev);
  }
  /*lcid = GetThreadLocale();
  if (!GetLocaleInfo(lcid, LOCALE_SISO639LANGNAME, iso639, sizeof (iso639)) || !GetLocaleInfo(lcid, LOCALE_SISO3166CTRYNAME, iso3166, sizeof (iso3166)))
      return g_strdup ("C");
  langid = LANGIDFROMLCID(lcid);
  primary = PRIMARYLANGID(langid);
  sub = SUBLANGID(langid);
  switch(primary) {
      case LANG_AZERI:
          switch(sub) {
              case SUBLANG_AZERI_LATIN: script = "@Latn"; break;
              case SUBLANG_AZERI_CYRILLIC: script = "@Cyrl"; break;
          }
          break;
      case LANG_SERBIAN:
          switch(sub) {
              case SUBLANG_SERBIAN_LATIN: case 0x06: script = "@Latn"; break;
          }
          break;
      case LANG_UZBEK:
          switch(sub) {
              case SUBLANG_UZBEK_LATIN: script = "@Latn"; break;
              case SUBLANG_UZBEK_CYRILLIC: script = "@Cyrl"; break;
          }
          break;
  }*/
  return g_strconcat (iso639, "_", iso3166, script, NULL);
}
gchar *g_win32_error_message(gint error) {
  gchar *retval;
  wchar_t *msg = NULL;
  int nchars;
  //FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, (LPWSTR)&msg, 0, NULL);
  if (msg != NULL) {
      nchars = wcslen (msg);
      if (nchars > 2 && msg[nchars-1] == '\n' && msg[nchars-2] == '\r') msg[nchars-2] = '\0';
      retval = g_utf16_to_utf8 (msg, -1, NULL, NULL, NULL);
      //LocalFree (msg);
  } else retval = g_strdup ("");
  return retval;
}
gchar *g_win32_get_package_installation_directory_of_module(gpointer hmodule) {
  gchar *retval;
  gchar *p;
  //wchar_t wc_fn[MAX_PATH];
  //if (!GetModuleFileNameW (hmodule, wc_fn, MAX_PATH)) return NULL;
  //retval = g_utf16_to_utf8(wc_fn, -1, NULL, NULL, NULL);
  if ((p = strrchr(retval, G_DIR_SEPARATOR)) != NULL) *p = '\0';
  p = strrchr(retval, G_DIR_SEPARATOR);
  if (p && (g_ascii_strcasecmp(p + 1, "bin") == 0 || g_ascii_strcasecmp(p + 1, "lib") == 0)) *p = '\0';
#ifdef G_WITH_CYGWIN
  {
      gchar tmp[MAX_PATH];
      cygwin_conv_to_posix_path(retval, tmp);
      g_free(retval);
      retval = g_strdup(tmp);
  }
#endif
  return retval;
}
static gchar * get_package_directory_from_module(const gchar *module_name) {
  static GHashTable *module_dirs = NULL;
  G_LOCK_DEFINE_STATIC(module_dirs);
  //HMODULE hmodule = NULL;
  gchar *fn;
  G_LOCK(module_dirs);
  if (module_dirs == NULL) module_dirs = g_hash_table_new(g_str_hash, g_str_equal);
  fn = g_hash_table_lookup(module_dirs, module_name ? module_name : "");
  if (fn) {
      G_UNLOCK(module_dirs);
      return g_strdup(fn);
  }
  if (module_name) {
      wchar_t *wc_module_name = g_utf8_to_utf16(module_name, -1, NULL, NULL, NULL);
      //hmodule = GetModuleHandleW(wc_module_name);
      g_free(wc_module_name);
      /*if (!hmodule) {
          G_UNLOCK(module_dirs);
          return NULL;
	  }*/
  }
  //fn = g_win32_get_package_installation_directory_of_module(hmodule);
  if (fn == NULL) {
      G_UNLOCK(module_dirs);
      return NULL;
  }
  g_hash_table_insert(module_dirs, module_name ? g_strdup(module_name) : "", fn);
  G_UNLOCK(module_dirs);
  return g_strdup(fn);
}
gchar *g_win32_get_package_installation_directory_utf8(const gchar *package, const gchar *dll_name) {
  gchar *result = NULL;
  if (package != NULL) g_warning("Passing a non-NULL package to g_win32_get_package_installation_directory() is deprecated and it is ignored.");
  if (dll_name != NULL) result = get_package_directory_from_module(dll_name);
  if (result == NULL) result = get_package_directory_from_module(NULL);
  return result;
}
#if !defined(_WIN64)
gchar *g_win32_get_package_installation_directory(const gchar *package, const gchar *dll_name) {
  gchar *utf8_package = NULL, *utf8_dll_name = NULL;
  gchar *utf8_retval, *retval;
  if (package != NULL) utf8_package = g_locale_to_utf8(package, -1, NULL, NULL, NULL);
  if (dll_name != NULL) utf8_dll_name = g_locale_to_utf8(dll_name, -1, NULL, NULL, NULL);
  utf8_retval = g_win32_get_package_installation_directory_utf8(utf8_package, utf8_dll_name);
  retval = g_locale_from_utf8(utf8_retval, -1, NULL, NULL, NULL);
  g_free(utf8_package);
  g_free(utf8_dll_name);
  g_free(utf8_retval);
  return retval;
}
#endif
gchar *g_win32_get_package_installation_subdirectory_utf8(const gchar *package, const gchar *dll_name, const gchar *subdir) {
  gchar *prefix;
  gchar *dirname;
  prefix = g_win32_get_package_installation_directory_utf8(package, dll_name);
  dirname = g_build_filename(prefix, subdir, NULL);
  g_free(prefix);
  return dirname;
}
#if !defined (_WIN64)
gchar *g_win32_get_package_installation_subdirectory(const gchar *package, const gchar *dll_name, const gchar *subdir) {
  gchar *prefix;
  gchar *dirname;
  prefix = g_win32_get_package_installation_directory(package, dll_name);
  dirname = g_build_filename(prefix, subdir, NULL);
  g_free(prefix);
  return dirname;
}
#endif
static guint windows_version;
static void g_win32_windows_version_init(void) {
  static gboolean beenhere = FALSE;
  if (!beenhere) {
      beenhere = TRUE;
      //windows_version = GetVersion();
      if (windows_version & 0x80000000) g_error ("This version of GLib requires NT-based Windows.");
  }
}
void _g_win32_thread_init(void) {
  g_win32_windows_version_init();
}
guint g_win32_get_windows_version(void) {
  g_win32_windows_version_init();
  return windows_version;
}
gchar *g_win32_locale_filename_from_utf8(const gchar *utf8filename) {
  gchar *retval = g_locale_from_utf8(utf8filename, -1, NULL, NULL, NULL);
  if (retval == NULL) {
      wchar_t *wname = g_utf8_to_utf16(utf8filename, -1, NULL, NULL, NULL);
      if (wname != NULL) {
          /*wchar_t wshortname[MAX_PATH + 1];
          if (GetShortPathNameW(wname, wshortname, G_N_ELEMENTS (wshortname))) {
              gchar *tem = g_utf16_to_utf8(wshortname, -1, NULL, NULL, NULL);
              retval = g_locale_from_utf8(tem, -1, NULL, NULL, NULL);
              g_free(tem);
          }*/
          g_free(wname);
	  }
  }
  return retval;
}