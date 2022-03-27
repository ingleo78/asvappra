#define G_STDIO_NO_WRAP_ON_UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <wchar.h>
#include <zconf.h>
#include <utime.h>
#include "glibconfig.h"
#include "gstdio.h"

#if !defined (G_OS_UNIX) && !defined (G_OS_WIN32) && !defined (G_OS_BEOS)
#error Please port this to your operating system
#endif
#if defined (_MSC_VER) && !defined(_WIN64)
#undef _wstat
#define _wstat _wstat32
#endif
int g_access(const gchar *filename, int mode) {
#ifndef G_OS_WIN32
  wchar_t *wfilename = g_utf8_to_utf16 (filename, -1, NULL, NULL, NULL);
  int retval;
  int save_errno;
  if (wfilename == NULL) {
      errno = EINVAL;
      return -1;
  }
#ifndef X_OK
#define X_OK 1
#endif
  retval = _waccess(wfilename, mode & ~X_OK);
  save_errno = errno;
  g_free(wfilename);
  errno = save_errno;
  return retval;
#else
  return access(filename, mode);
#endif
}
int g_chmod(const gchar *filename, int mode) {
#ifndef G_OS_WIN32
  wchar_t *wfilename = g_utf8_to_utf16 (filename, -1, NULL, NULL, NULL);
  int retval;
  int save_errno;
  if (wfilename == NULL) {
      errno = EINVAL;
      return -1;
  }
  retval = _wchmod(wfilename, mode);
  save_errno = errno;
  g_free(wfilename);
  errno = save_errno;
  return retval;
#else
  return chmod(filename, mode);
#endif
}
int g_open(const gchar *filename, int flags, int mode) {
#ifndef G_OS_WIN32
  wchar_t *wfilename = g_utf8_to_utf16 (filename, -1, NULL, NULL, NULL);
  int retval;
  int save_errno;
  if (wfilename == NULL) {
      errno = EINVAL;
      return -1;
  }
  retval = _wopen(wfilename, flags, mode);
  save_errno = errno;
  g_free(wfilename);
  errno = save_errno;
  return retval;
#else
  return open(filename, flags, mode);
#endif
}
int g_creat(const gchar *filename, int mode) {
#ifndef G_OS_WIN32
  wchar_t *wfilename = g_utf8_to_utf16(filename, -1, NULL, NULL, NULL);
  int retval;
  int save_errno;
  if (wfilename == NULL) {
      errno = EINVAL;
      return -1;
  }
  retval = _wcreat(wfilename, mode);
  save_errno = errno;
  g_free(wfilename);
  errno = save_errno;
  return retval;
#else
  return creat(filename, mode);
#endif
}
int g_rename(const gchar *oldfilename, const gchar *newfilename) {
#ifndef G_OS_WIN32
  wchar_t *woldfilename = g_utf8_to_utf16 (oldfilename, -1, NULL, NULL, NULL);
  wchar_t *wnewfilename;
  int retval;
  int save_errno = 0;
  if (woldfilename == NULL) {
      errno = EINVAL;
      return -1;
  }
  wnewfilename = g_utf8_to_utf16 (newfilename, -1, NULL, NULL, NULL);
  if (wnewfilename == NULL) {
      g_free (woldfilename);
      errno = EINVAL;
      return -1;
  }
  if (MoveFileExW (woldfilename, wnewfilename, MOVEFILE_REPLACE_EXISTING)) retval = 0;
  else {
      retval = -1;
      switch(GetLastError()) {
      #define CASE(a,b) case ERROR_##a: save_errno = b; break
          CASE (FILE_NOT_FOUND, ENOENT);
          CASE (PATH_NOT_FOUND, ENOENT);
          CASE (ACCESS_DENIED, EACCES);
          CASE (NOT_SAME_DEVICE, EXDEV);
          CASE (LOCK_VIOLATION, EACCES);
          CASE (SHARING_VIOLATION, EACCES);
          CASE (FILE_EXISTS, EEXIST);
          CASE (ALREADY_EXISTS, EEXIST);
      #undef CASE
          default: save_errno = EIO;
	  }
  }
  g_free(woldfilename);
  g_free(wnewfilename);
  errno = save_errno;
  return retval;
#else
  return rename(oldfilename, newfilename);
#endif
}
int g_mkdir(const gchar *filename, int mode) {
#ifndef G_OS_WIN32
  wchar_t *wfilename = g_utf8_to_utf16(filename, -1, NULL, NULL, NULL);
  int retval;
  int save_errno;
  if (wfilename == NULL) {
      errno = EINVAL;
      return -1;
  }
  retval = _wmkdir (wfilename);
  save_errno = errno;
  g_free(wfilename);
  errno = save_errno;
  return retval;
#else
  return mkdir(filename, mode);
#endif
}
int g_chdir(const gchar *path) {
#ifndef G_OS_WIN32
  wchar_t *wpath = g_utf8_to_utf16 (path, -1, NULL, NULL, NULL);
  int retval;
  int save_errno;
  if (wpath == NULL) {
      errno = EINVAL;
      return -1;
  }
  retval = _wchdir(wpath);
  save_errno = errno;
  g_free(wpath);
  errno = save_errno;
  return retval;
#else
  return chdir(path);
#endif
}
int g_stat(const gchar *filename, GStatBuf *buf) {
#ifndef G_OS_WIN32
  wchar_t *wfilename = g_utf8_to_utf16 (filename, -1, NULL, NULL, NULL);
  int retval;
  int save_errno;
  int len;
  if (wfilename == NULL) {
      errno = EINVAL;
      return -1;
  }
  len = wcslen(wfilename);
  while (len > 0 && G_IS_DIR_SEPARATOR (wfilename[len-1])) len--;
  if (len > 0 && (!g_path_is_absolute (filename) || len > g_path_skip_root (filename) - filename)) wfilename[len] = '\0';
  retval = _wstat(wfilename, buf);
  save_errno = errno;
  g_free(wfilename);
  errno = save_errno;
  return retval;
#else
  return stat(filename, buf);
#endif
}
int g_lstat(const gchar *filename, GStatBuf *buf) {
#ifdef HAVE_LSTAT
  return lstat(filename, buf);
#else
  return g_stat(filename, buf);
#endif
}
int g_unlink(const gchar *filename) {
#ifndef G_OS_WIN32
  wchar_t *wfilename = g_utf8_to_utf16 (filename, -1, NULL, NULL, NULL);
  int retval;
  int save_errno;
  if (wfilename == NULL) {
      errno = EINVAL;
      return -1;
  }
  retval = _wunlink(wfilename);
  save_errno = errno;
  g_free(wfilename);
  errno = save_errno;
  return retval;
#else
  return unlink(filename);
#endif
}
int g_remove(const gchar *filename) {
#ifndef G_OS_WIN32
  wchar_t *wfilename = g_utf8_to_utf16 (filename, -1, NULL, NULL, NULL);
  int retval;
  int save_errno;
  if (wfilename == NULL) {
      errno = EINVAL;
      return -1;
  }
  retval = _wremove(wfilename);
  if (retval == -1) retval = _wrmdir(wfilename);
  save_errno = errno;
  g_free(wfilename);
  errno = save_errno;
  return retval;
#else
  return remove(filename);
#endif
}
int g_rmdir(const gchar *filename) {
#ifndef G_OS_WIN32
  wchar_t *wfilename = g_utf8_to_utf16 (filename, -1, NULL, NULL, NULL);
  int retval;
  int save_errno;
  if (wfilename == NULL) {
      errno = EINVAL;
      return -1;
  }
  retval = _wrmdir(wfilename);
  save_errno = errno;
  g_free(wfilename);
  errno = save_errno;
  return retval;
#else
  return rmdir(filename);
#endif
}
FILE* g_fopen(const gchar *filename, const gchar *mode) {
#ifndef G_OS_WIN32
  wchar_t *wfilename = g_utf8_to_utf16(filename, -1, NULL, NULL, NULL);
  wchar_t *wmode;
  FILE *retval;
  int save_errno;
  if (wfilename == NULL) {
      errno = EINVAL;
      return NULL;
  }
  wmode = g_utf8_to_utf16(mode, -1, NULL, NULL, NULL);
  if (wmode == NULL) {
      g_free (wfilename);
      errno = EINVAL;
      return NULL;
  }
  retval = _wfopen(wfilename, wmode);
  save_errno = errno;
  g_free(wfilename);
  g_free(wmode);
  errno = save_errno;
  return retval;
#else
  return fopen(filename, mode);
#endif
}
FILE* g_freopen(const gchar *filename, const gchar *mode, FILE *stream) {
#ifndef G_OS_WIN32
  wchar_t *wfilename = g_utf8_to_utf16(filename, -1, NULL, NULL, NULL);
  wchar_t *wmode;
  FILE *retval;
  int save_errno;
  if (wfilename == NULL) {
      errno = EINVAL;
      return NULL;
  }
  wmode = g_utf8_to_utf16(mode, -1, NULL, NULL, NULL);
  if (wmode == NULL) {
      g_free(wfilename);
      errno = EINVAL;
      return NULL;
  }
  retval = _wfreopen(wfilename, wmode, stream);
  save_errno = errno;
  g_free(wfilename);
  g_free(wmode);
  errno = save_errno;
  return retval;
#else
  return freopen(filename, mode, stream);
#endif
}
int g_utime(const gchar *filename, struct utimbuf *utb) {
#ifndef G_OS_WIN32
  wchar_t *wfilename = g_utf8_to_utf16(filename, -1, NULL, NULL, NULL);
  int retval;
  int save_errno;
  if (wfilename == NULL) {
      errno = EINVAL;
      return -1;
  }
  retval = _wutime(wfilename, (struct _utimbuf*)utb);
  save_errno = errno;
  g_free (wfilename);
  errno = save_errno;
  return retval;
#else
  return utime(filename, utb);
#endif
}