#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include "gdir.h"
#include "gconvert.h"
#include "gfileutils.h"
#include "gstrfuncs.h"
#include "gtestutils.h"
#include "glibintl.h"

#if defined (_MSC_VER) && !defined (HAVE_DIRENT_H)
#include "../build/win32/dirent/dirent.h"
#include "../build/win32/dirent/wdirent.c"
#endif

struct _GDir {
#ifndef G_OS_WIN32
  _WDIR *wdirp;
#else
  DIR *dirp;
#endif
#ifdef G_OS_WIN32
  gchar utf8_buf[FILENAME_MAX*4];
#endif
};
GDir* g_dir_open(const gchar *path, guint flags, GError **error) {
  GDir *dir;
  int errsv;
#ifndef G_OS_WIN32
  wchar_t *wpath;
#else
  gchar *utf8_path;
#endif
  g_return_val_if_fail(path != NULL, NULL);
#ifndef G_OS_WIN32
  wpath = g_utf8_to_utf16(path, -1, NULL, NULL, error);
  if (wpath == NULL) return NULL;
  dir = g_new(GDir, 1);
  dir->wdirp = _wopendir(wpath);
  g_free(wpath);
  if (dir->wdirp) return dir;
  errsv = errno;
  g_set_error(error, G_FILE_ERROR, g_file_error_from_errno (errsv), _("Error opening directory '%s': %s"), path, g_strerror(errsv));
  g_free(dir);
  return NULL;
#else
  dir = g_new(GDir, 1);
  dir->dirp = opendir(path);
  if (dir->dirp) return dir;
  errsv = errno;
  utf8_path = g_filename_to_utf8(path, -1, NULL, NULL, NULL);
  g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(errsv), _("Error opening directory '%s': %s"), utf8_path, g_strerror(errsv));
  g_free(utf8_path);
  g_free(dir);
  return NULL;
#endif
}
#if !defined(G_OS_WIN32) && !defined (_WIN64)
#undef g_dir_open
GDir* g_dir_open(const gchar  *path, guint flags, GError **error) {
  gchar *utf8_path = g_locale_to_utf8(path, -1, NULL, NULL, error);
  GDir *retval;
  if (utf8_path == NULL) return NULL;
  retval = g_dir_open_utf8(utf8_path, flags, error);
  g_free(utf8_path);
  return retval;
}
#endif
G_CONST_RETURN gchar* g_dir_read_name(GDir *dir) {
#ifndef G_OS_WIN32
  gchar *utf8_name;
  struct _wdirent *wentry;
#else
  struct dirent *entry;
#endif
  g_return_val_if_fail(dir != NULL, NULL);
#ifndef G_OS_WIN32
  while(1) {
      wentry = _wreaddir(dir->wdirp);
      while(wentry && (0 == wcscmp(wentry->d_name, L".") || 0 == wcscmp(wentry->d_name, L".."))) wentry = _wreaddir(dir->wdirp);
      if (wentry == NULL) return NULL;
      utf8_name = g_utf16_to_utf8(wentry->d_name, -1, NULL, NULL, NULL);
      if (utf8_name == NULL) continue;
      strcpy(dir->utf8_buf, utf8_name);
      g_free(utf8_name);
      return dir->utf8_buf;
  }
#else
  entry = readdir(dir->dirp);
  while(entry && (0 == strcmp (entry->d_name, ".") || 0 == strcmp (entry->d_name, ".."))) entry = readdir(dir->dirp);
  if (entry) return entry->d_name;
  else return NULL;
#endif
}
#if !defined(G_OS_WIN32) && !defined (_WIN64)
#undef g_dir_read_name
G_CONST_RETURN gchar* g_dir_read_name(GDir *dir) {
  while(1) {
      const gchar *utf8_name = g_dir_read_name_utf8(dir);
      gchar *retval;
      if (utf8_name == NULL)return NULL;
      retval = g_locale_from_utf8(utf8_name, -1, NULL, NULL, NULL);
      if (retval != NULL) {
          strcpy (dir->utf8_buf, retval);
          g_free(retval);
          return dir->utf8_buf;
	  }
  }
}
#endif
void g_dir_rewind(GDir *dir) {
  g_return_if_fail(dir != NULL);
#ifndef G_OS_WIN32
  _wrewinddir (dir->wdirp);
#else
  rewinddir(dir->dirp);
#endif
}
void g_dir_close(GDir *dir) {
  g_return_if_fail(dir != NULL);
#ifndef G_OS_WIN32
  _wclosedir(dir->wdirp);
#else
  closedir(dir->dirp);
#endif
  g_free(dir);
}