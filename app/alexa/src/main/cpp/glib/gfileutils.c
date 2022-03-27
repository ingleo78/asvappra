#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#ifdef __linux__
#include <linux/magic.h>
#include <sys/vfs.h>
#include <zconf.h>
#endif
#include "../gio/config.h"
#include "glibconfig.h"
#include "gfileutils.h"
#include "gstdio.h"
#include "glibintl.h"
#include "glib.h"

#ifndef S_ISLNK
#define S_ISLNK(x) 0
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif
int g_mkdir_with_parents(const gchar *pathname, int mode) {
  gchar *fn, *p;
  if (pathname == NULL || *pathname == '\0') {
      errno = EINVAL;
      return -1;
  }
  fn = g_strdup(pathname);
  if (g_path_is_absolute(fn)) p = (gchar*)g_path_skip_root(fn);
  else p = fn;
  do {
      while(*p && !G_IS_DIR_SEPARATOR(*p)) p++;
      if (!*p) p = NULL;
      else *p = '\0';
      if (!g_file_test(fn, G_FILE_TEST_EXISTS)) {
          if (g_mkdir(fn, mode) == -1) {
              int errno_save = errno;
              g_free(fn);
              errno = errno_save;
              return -1;
          }
	  } else if (!g_file_test(fn, G_FILE_TEST_IS_DIR)) {
          g_free(fn);
          errno = ENOTDIR;
          return -1;
	  }
      if (p) {
          *p++ = G_DIR_SEPARATOR;
          while(*p && G_IS_DIR_SEPARATOR(*p)) p++;
	  }
  } while(p);
  g_free(fn);
  return 0;
}
gboolean g_file_test(const gchar *filename, GFileTest test) {
#ifndef G_OS_WIN32
  #ifndef INVALID_FILE_ATTRIBUTES
  #define INVALID_FILE_ATTRIBUTES -1
  #endif
  #ifndef FILE_ATTRIBUTE_DEVICE
  #define FILE_ATTRIBUTE_DEVICE 64
  #endif
  int attributes;
  wchar_t *wfilename = g_utf8_to_utf16(filename, -1, NULL, NULL, NULL);
  if (wfilename == NULL) return FALSE;
  attributes = GetFileAttributesW(wfilename);
  g_free(wfilename);
  if (attributes == INVALID_FILE_ATTRIBUTES) return FALSE;
  if (test & G_FILE_TEST_EXISTS) return TRUE;
  if (test & G_FILE_TEST_IS_REGULAR) {
      if ((attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE)) == 0) return TRUE;
  }
  if (test & G_FILE_TEST_IS_DIR) {
      if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) return TRUE;
  }
  while (test & G_FILE_TEST_IS_EXECUTABLE) {
      const gchar *lastdot = strrchr (filename, '.');
      const gchar *pathext = NULL, *p;
      int extlen;
      if (lastdot == NULL)  break;
      if (_stricmp (lastdot, ".exe") == 0 || _stricmp (lastdot, ".cmd") == 0 || _stricmp (lastdot, ".bat") == 0 || _stricmp (lastdot, ".com") == 0) return TRUE;
      pathext = g_getenv ("PATHEXT");
      if (pathext == NULL) break;
      pathext = g_utf8_casefold(pathext, -1);
      lastdot = g_utf8_casefold(lastdot, -1);
      extlen = strlen(lastdot);
      p = pathext;
      while(TRUE) {
          const gchar *q = strchr (p, ';');
          if (q == NULL) q = p + strlen(p);
          if (extlen == q - p && memcmp (lastdot, p, extlen) == 0) {
              g_free((gchar*)pathext);
              g_free((gchar*)lastdot);
              return TRUE;
          }
          if (*q) p = q + 1;
          else break;
	  }
      g_free((gchar*)pathext);
      g_free((gchar*)lastdot);
      break;
  }
  return FALSE;
#else
  if ((test & G_FILE_TEST_EXISTS) && (access (filename, F_OK) == 0)) return TRUE;
  if ((test & G_FILE_TEST_IS_EXECUTABLE) && (access (filename, X_OK) == 0)) {
      if (getuid() != 0)return TRUE;
  } else test &= ~G_FILE_TEST_IS_EXECUTABLE;
  if (test & G_FILE_TEST_IS_SYMLINK) {
      struct stat s;
      if ((lstat (filename, &s) == 0) && S_ISLNK (s.st_mode)) return TRUE;
  }
  if (test & (G_FILE_TEST_IS_REGULAR | G_FILE_TEST_IS_DIR | G_FILE_TEST_IS_EXECUTABLE)) {
      struct stat s;
      if (stat (filename, &s) == 0) {
          if ((test & G_FILE_TEST_IS_REGULAR) && S_ISREG (s.st_mode)) return TRUE;
          if ((test & G_FILE_TEST_IS_DIR) && S_ISDIR (s.st_mode)) return TRUE;
          if ((test & G_FILE_TEST_IS_EXECUTABLE) && ((s.st_mode & S_IXOTH) || (s.st_mode & S_IXUSR) || (s.st_mode & S_IXGRP))) return TRUE;
	  }
  }
  return FALSE;
#endif
}
GQuark g_file_error_quark(void) {
  return g_quark_from_static_string("g-file-error-quark");
}
GFileError g_file_error_from_errno (gint err_no) {
  switch (err_no) {
  #ifdef EEXIST
      case EEXIST: return G_FILE_ERROR_EXIST;
  #endif
  #ifdef EISDIR
      case EISDIR: return G_FILE_ERROR_ISDIR;
  #endif
  #ifdef EACCES
      case EACCES: return G_FILE_ERROR_ACCES;
  #endif
  #ifdef ENAMETOOLONG
      case ENAMETOOLONG: return G_FILE_ERROR_NAMETOOLONG;
  #endif
  #ifdef ENOENT
      case ENOENT: return G_FILE_ERROR_NOENT;
  #endif
  #ifdef ENOTDIR
      case ENOTDIR: return G_FILE_ERROR_NOTDIR;
  #endif
  #ifdef ENXIO
      case ENXIO: return G_FILE_ERROR_NXIO;
  #endif
  #ifdef ENODEV
      case ENODEV: return G_FILE_ERROR_NODEV;
  #endif
  #ifdef EROFS
      case EROFS: return G_FILE_ERROR_ROFS;
  #endif
  #ifdef ETXTBSY
      case ETXTBSY: return G_FILE_ERROR_TXTBSY;
  #endif
  #ifdef EFAULT
      case EFAULT: return G_FILE_ERROR_FAULT;
  #endif
  #ifdef ELOOP
      case ELOOP: return G_FILE_ERROR_LOOP;
  #endif
  #ifdef ENOSPC
      case ENOSPC: return G_FILE_ERROR_NOSPC;
  #endif
  #ifdef ENOMEM
      case ENOMEM: return G_FILE_ERROR_NOMEM;
  #endif
  #ifdef EMFILE
      case EMFILE: return G_FILE_ERROR_MFILE;
  #endif
  #ifdef ENFILE
      case ENFILE: return G_FILE_ERROR_NFILE;
  #endif
  #ifdef EBADF
      case EBADF: return G_FILE_ERROR_BADF;
  #endif
  #ifdef EINVAL
      case EINVAL: return G_FILE_ERROR_INVAL;
  #endif
  #ifdef EPIPE
      case EPIPE: return G_FILE_ERROR_PIPE;
  #endif
  #ifdef EAGAIN
      case EAGAIN: return G_FILE_ERROR_AGAIN;
  #endif
  #ifdef EINTR
      case EINTR: return G_FILE_ERROR_INTR;
  #endif
  #ifdef EIO
      case EIO: return G_FILE_ERROR_IO;
  #endif
  #ifdef EPERM
      case EPERM: return G_FILE_ERROR_PERM;
  #endif
  #ifdef ENOSYS
      case ENOSYS: return G_FILE_ERROR_NOSYS;
  #endif
      default: return G_FILE_ERROR_FAILED;
  }
}
static gboolean get_contents_stdio(const gchar  *display_filename, FILE *f, gchar **contents, gsize *length, GError **error) {
  gchar buf[4096];
  gsize bytes;
  gchar *str = NULL;
  gsize total_bytes = 0;
  gsize total_allocated = 0;
  gchar *tmp;
  g_assert(f != NULL);
  while (!feof(f)) {
      gint save_errno;
      bytes = fread(buf, 1, sizeof(buf), f);
      save_errno = errno;
      while((total_bytes + bytes + 1) > total_allocated) {
          if (str) total_allocated *= 2;
          else total_allocated = MIN(bytes + 1, sizeof(buf));
          tmp = g_try_realloc(str, total_allocated);
          if (tmp == NULL) {
              g_set_error(error, G_FILE_ERROR,G_FILE_ERROR_NOMEM, _("Could not allocate %lu bytes to read file \"%s\""), (gulong) total_allocated,
			              display_filename);
              goto error;
          }
	      str = tmp;
      }
      if (ferror (f)) {
          g_set_error(error, G_FILE_ERROR, g_file_error_from_errno (save_errno), _("Error reading file '%s': %s"), display_filename, g_strerror(save_errno));
          goto error;
      }
      memcpy(str + total_bytes, buf, bytes);
      if (total_bytes + bytes < total_bytes) {
          g_set_error(error, G_FILE_ERROR,G_FILE_ERROR_FAILED, _("File \"%s\" is too large"), display_filename);
          goto error;
      }
      total_bytes += bytes;
  }
  fclose(f);
  if (total_allocated == 0) {
      str = g_new(gchar, 1);
      total_bytes = 0;
  }
  str[total_bytes] = '\0';
  if (length) *length = total_bytes;
  *contents = str;
  return TRUE;
  error:
  g_free(str);
  fclose(f);
  return FALSE;
}
#ifndef G_OS_WIN32
static gboolean get_contents_regfile(const gchar *display_filename, struct stat *stat_buf, gint fd, gchar **contents, gsize *length, GError **error) {
  gchar *buf;
  gsize bytes_read;
  gsize size;
  gsize alloc_size;
  size = stat_buf->st_size;
  alloc_size = size + 1;
  buf = g_try_malloc(alloc_size);
  if (buf == NULL) {
      g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOMEM, _("Could not allocate %lu bytes to read file \"%s\""), (gulong)alloc_size, display_filename);
      goto error;
  }
  bytes_read = 0;
  while (bytes_read < size) {
      gssize rc;
      rc = read(fd, buf + bytes_read, size - bytes_read);
      if (rc < 0) {
          if (errno != EINTR) {
	          int save_errno = errno;
              g_free(buf);
              g_set_error(error, G_FILE_ERROR, g_file_error_from_errno (save_errno), _("Failed to read from file '%s': %s"), display_filename,
			  g_strerror(save_errno));
	          goto error;
          }
      } else if (rc == 0) break;
      else bytes_read += rc;
  }
  buf[bytes_read] = '\0';
  if (length) *length = bytes_read;
  *contents = buf;
  close(fd);
  return TRUE;
  error:
  close(fd);
  return FALSE;
}
static gboolean get_contents_posix(const gchar *filename, gchar **contents, gsize *length, GError **error) {
  struct stat stat_buf;
  gint fd;
  gchar *display_filename = g_filename_display_name(filename);
  fd = open (filename, O_RDONLY|O_BINARY);
  if (fd < 0) {
      int save_errno = errno;
      g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(save_errno), _("Failed to open file '%s': %s"), display_filename, g_strerror(save_errno));
      g_free(display_filename);
      return FALSE;
  }
  if (fstat (fd, &stat_buf) < 0) {
      int save_errno = errno;
      close(fd);
      g_set_error(error, G_FILE_ERROR, g_file_error_from_errno (save_errno), _("Failed to get attributes of file '%s': fstat() failed: %s"), display_filename,
                  g_strerror (save_errno));
      g_free(display_filename);
      return FALSE;
  }
  if (stat_buf.st_size > 0 && S_ISREG(stat_buf.st_mode)) {
      gboolean retval = get_contents_regfile(display_filename, &stat_buf, fd, contents, length, error);
      g_free(display_filename);
      return retval;
  } else {
      FILE *f;
      gboolean retval;
      f = fdopen(fd, "r");
      if (f == NULL) {
	      int save_errno = errno;
          g_set_error(error, G_FILE_ERROR,  g_file_error_from_errno (save_errno), _("Failed to open file '%s': fdopen() failed: %s"), display_filename,
		              g_strerror (save_errno));
          g_free(display_filename);
          return FALSE;
      }
      retval = get_contents_stdio(display_filename, f, contents, length, error);
      g_free(display_filename);
      return retval;
  }
}
#else
static gboolean get_contents_win32(const gchar *filename, gchar **contents, gsize *length, GError **error) {
  FILE *f;
  gboolean retval;
  gchar *display_filename = g_filename_display_name (filename);
  int save_errno;
  f = g_fopen(filename, "rb");
  save_errno = errno;
  if (f == NULL) {
      g_set_error(error, G_FILE_ERROR, g_file_error_from_errno (save_errno), _("Failed to open file '%s': %s"), display_filename, g_strerror (save_errno));
      g_free(display_filename);
      return FALSE;
  }
  retval = get_contents_stdio(display_filename, f, contents, length, error);
  g_free(display_filename);
  return retval;
}
#endif
gboolean g_file_get_contents(const gchar *filename, gchar **contents, gsize *length, GError **error) {
  g_return_val_if_fail(filename != NULL, FALSE);
  g_return_val_if_fail(contents != NULL, FALSE);
  *contents = NULL;
  if (length) *length = 0;
#ifdef G_OS_WIN32
  return get_contents_win32(filename, contents, length, error);
#else
  return get_contents_posix (filename, contents, length, error);
#endif
}
static gboolean rename_file(const char *old_name, const char *new_name, GError **err) {
  errno = 0;
  if (g_rename (old_name, new_name) == -1) {
      int save_errno = errno;
      gchar *display_old_name = g_filename_display_name(old_name);
      gchar *display_new_name = g_filename_display_name(new_name);
      g_set_error(err, G_FILE_ERROR, g_file_error_from_errno (save_errno), _("Failed to rename file '%s' to '%s': g_rename() failed: %s"),
		          display_old_name, display_new_name, g_strerror(save_errno));
      g_free(display_old_name);
      g_free(display_new_name);
      return FALSE;
  }
  return TRUE;
}
static gchar* write_to_temp_file(const gchar *contents, gssize length, const gchar *dest_file, GError **err) {
  gchar *tmp_name;
  gchar *display_name;
  gchar *retval;
  FILE *file;
  gint fd;
  int save_errno;
  retval = NULL;
  tmp_name = g_strdup_printf("%s.XXXXXX", dest_file);
  errno = 0;
  fd = g_mkstemp_full(tmp_name, O_RDWR | O_BINARY, 0666);
  save_errno = errno;
  display_name = g_filename_display_name(tmp_name);
  if (fd == -1) {
      g_set_error(err, G_FILE_ERROR, g_file_error_from_errno(save_errno), _("Failed to create file '%s': %s"), display_name, g_strerror(save_errno));
      goto out;
  }
  errno = 0;
  file = fdopen (fd, "wb");
  if (!file) {
      save_errno = errno;
      g_set_error(err, G_FILE_ERROR, g_file_error_from_errno(save_errno), _("Failed to open file '%s' for writing: fdopen() failed: %s"), display_name,
		          g_strerror(save_errno));
      close(fd);
      g_unlink(tmp_name);
      goto out;
  }
  if (length > 0) {
      gsize n_written;
      errno = 0;
      n_written = fwrite(contents, 1, length, file);
      if (n_written < length){
          save_errno = errno;
          g_set_error(err, G_FILE_ERROR, g_file_error_from_errno (save_errno), _("Failed to write file '%s': fwrite() failed: %s"), display_name,
                      g_strerror (save_errno));
          fclose(file);
          g_unlink(tmp_name);
          goto out;
	  }
  }
  errno = 0;
  if (fflush (file) != 0) {
      save_errno = errno;
      g_set_error(err, G_FILE_ERROR, g_file_error_from_errno (save_errno),  _("Failed to write file '%s': fflush() failed: %s"), display_name,
		          g_strerror (save_errno));
      g_unlink(tmp_name);
      goto out;
  }
#ifdef BTRFS_SUPER_MAGIC
  {
      struct statfs buf;
      if (fstatfs(fd, &buf) == 0 && buf.f_type == BTRFS_SUPER_MAGIC) goto no_fsync;
  }
#endif
#ifdef HAVE_FSYNC
  {
      struct stat statbuf;
      errno = 0;
      if (g_lstat (dest_file, &statbuf) == 0 && statbuf.st_size > 0 && fsync(fileno (file)) != 0) {
          save_errno = errno;
          g_set_error(err, G_FILE_ERROR, g_file_error_from_errno (save_errno), _("Failed to write file '%s': fsync() failed: %s"), display_name,
                      g_strerror (save_errno));
          g_unlink(tmp_name);
          goto out;
      }
  }
#endif
  no_fsync:
  errno = 0;
  if (fclose (file) == EOF) {
      save_errno = errno;
      g_set_error(err, G_FILE_ERROR, g_file_error_from_errno (save_errno), _("Failed to close file '%s': fclose() failed: %s"), display_name,
		          g_strerror (save_errno));
      g_unlink(tmp_name);
      goto out;
  }
  retval = g_strdup(tmp_name);
  out:
  g_free(tmp_name);
  g_free(display_name);
  return retval;
}
gboolean g_file_set_contents(const gchar *filename, const gchar *contents, gssize	length, GError	**error) {
  gchar *tmp_filename;
  gboolean retval;
  GError *rename_error = NULL;
  g_return_val_if_fail(filename != NULL, FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail(contents != NULL || length == 0, FALSE);
  g_return_val_if_fail(length >= -1, FALSE);
  if (length == -1) length = strlen(contents);
  tmp_filename = write_to_temp_file(contents, length, filename, error);
  if (!tmp_filename) {
      retval = FALSE;
      goto out;
  }
  if (!rename_file(tmp_filename, filename, &rename_error)) {
  #ifndef G_OS_WIN32
      g_unlink(tmp_filename);
      g_propagate_error (error, rename_error);
      retval = FALSE;
      goto out;
  #else
      if (!g_file_test (filename, G_FILE_TEST_EXISTS)) {
          g_unlink (tmp_filename);
          g_propagate_error (error, rename_error);
          retval = FALSE;
          goto out;
	  }
      g_error_free(rename_error);
      if (g_unlink(filename) == -1) {
          gchar *display_filename = g_filename_display_name(filename);
          int save_errno = errno;
          g_set_error(error, G_FILE_ERROR, g_file_error_from_errno (save_errno), _("Existing file '%s' could not be removed: g_unlink() failed: %s"),
                      display_filename, g_strerror (save_errno));
          g_free(display_filename);
          g_unlink(tmp_filename);
          retval = FALSE;
          goto out;
	  }
      if (!rename_file (tmp_filename, filename, error)) {
          g_unlink(tmp_filename);
          retval = FALSE;
          goto out;
	  }
  #endif
  }
  retval = TRUE;
  out:
  g_free(tmp_filename);
  return retval;
}
gint g_mkstemp_full(gchar *tmpl, int flags, int mode) {
  char *XXXXXX;
  int count, fd;
  static const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  static const int NLETTERS = sizeof (letters) - 1;
  glong value;
  GTimeVal tv;
  static int counter = 0;
  g_return_val_if_fail (tmpl != NULL, -1);
  XXXXXX = g_strrstr (tmpl, "XXXXXX");
  if (!XXXXXX || strncmp (XXXXXX, "XXXXXX", 6)) {
      errno = EINVAL;
      return -1;
  }
  g_get_current_time(&tv);
  value = (tv.tv_usec ^ tv.tv_sec) + counter++;
  for (count = 0; count < 100; value += 7777, ++count) {
      glong v = value;
      XXXXXX[0] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[1] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[2] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[3] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[4] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[5] = letters[v % NLETTERS];
      fd = g_open(tmpl, flags | O_CREAT | O_EXCL, mode);
      if (fd >= 0) return fd;
      else if (errno != EEXIST) return -1;
  }
  errno = EEXIST;
  return -1;
}
gint g_mkstemp(gchar *tmpl) {
  return g_mkstemp_full(tmpl, O_RDWR | O_BINARY, 0600);
}
gint g_file_open_tmp(const gchar *tmpl, gchar **name_used, GError **error) {
  int retval;
  const char *tmpdir;
  const char *sep;
  char *fulltemplate;
  const char *slash;
  if (tmpl == NULL) tmpl = ".XXXXXX";
  if ((slash = strchr(tmpl, G_DIR_SEPARATOR)) != NULL
#ifdef G_OS_WIN32
      || (strchr(tmpl, '/') != NULL && (slash = "/"))
#endif
      ) {
      gchar *display_tmpl = g_filename_display_name(tmpl);
      char c[2];
      c[0] = *slash;
      c[1] = '\0';
      g_set_error(error, G_FILE_ERROR,G_FILE_ERROR_FAILED, _("Template '%s' invalid, should not contain a '%s'"), display_tmpl, c);
      g_free(display_tmpl);
      return -1;
  }
  if (strstr (tmpl, "XXXXXX") == NULL) {
      gchar *display_tmpl = g_filename_display_name(tmpl);
      g_set_error(error, G_FILE_ERROR,G_FILE_ERROR_FAILED, _("Template '%s' doesn't contain XXXXXX"), display_tmpl);
      g_free(display_tmpl);
      return -1;
  }
  tmpdir = g_get_tmp_dir();
  if (G_IS_DIR_SEPARATOR(tmpdir [strlen (tmpdir) - 1])) sep = "";
  else sep = G_DIR_SEPARATOR_S;
  fulltemplate = g_strconcat(tmpdir, sep, tmpl, NULL);
  retval = g_mkstemp(fulltemplate);
  if (retval == -1) {
      int save_errno = errno;
      gchar *display_fulltemplate = g_filename_display_name(fulltemplate);
      g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(save_errno), _("Failed to create file '%s': %s"), display_fulltemplate, g_strerror(save_errno));
      g_free(display_fulltemplate);
      g_free(fulltemplate);
      return -1;
  }
  if (name_used) *name_used = fulltemplate;
  else g_free(fulltemplate);
  return retval;
}
static gchar* g_build_path_va(const gchar *separator, const gchar *first_element, va_list *args, gchar **str_array) {
  GString *result;
  gint separator_len = strlen(separator);
  gboolean is_first = TRUE;
  gboolean have_leading = FALSE;
  const gchar *single_element = NULL;
  const gchar *next_element;
  const gchar *last_trailing = NULL;
  gint i = 0;
  result = g_string_new (NULL);
  if (str_array) next_element = str_array[i++];
  else next_element = first_element;
  while(TRUE) {
      const gchar *element;
      const gchar *start;
      const gchar *end;
      if (next_element) {
          element = next_element;
          if (str_array) next_element = str_array[i++];
          else next_element = va_arg(*args, gchar *);
	  } else break;
      if (!*element) continue;
      start = element;
      if (separator_len) {
	      while(strncmp (start, separator, separator_len) == 0) start += separator_len;
      }
      end = start + strlen (start);
      if (separator_len) {
          while (end >= start + separator_len && strncmp (end - separator_len, separator, separator_len) == 0) end -= separator_len;
          last_trailing = end;
          while (last_trailing >= element + separator_len && strncmp (last_trailing - separator_len, separator, separator_len) == 0) last_trailing -= separator_len;
          if (!have_leading) {
              if (last_trailing <= start) single_element = element;
              g_string_append_len(result, element, start - element);
              have_leading = TRUE;
          } else single_element = NULL;
	  }
      if (end == start) continue;
      if (!is_first) g_string_append(result, separator);
      g_string_append_len(result, start, end - start);
      is_first = FALSE;
  }
  if (single_element) {
      g_string_free(result, TRUE);
      return g_strdup(single_element);
  } else {
      if (last_trailing) g_string_append(result, last_trailing);
      return g_string_free(result, FALSE);
  }
}
gchar* g_build_pathv(const gchar *separator, gchar **args) {
  if (!args) return NULL;
  return g_build_path_va(separator, NULL, NULL, args);
}
gchar* g_build_path(const gchar *separator, const gchar *first_element, ...) {
  gchar *str;
  va_list args;
  g_return_val_if_fail(separator != NULL, NULL);
  va_start(args, first_element);
  str = g_build_path_va(separator, first_element, &args, NULL);
  va_end(args);
  return str;
}
#ifdef G_OS_WIN32
static gchar* g_build_pathname_va(const gchar *first_element, va_list *args, gchar **str_array) {
  GString *result;
  gboolean is_first = TRUE;
  gboolean have_leading = FALSE;
  const gchar *single_element = NULL;
  const gchar *next_element;
  const gchar *last_trailing = NULL;
  gchar current_separator = '\\';
  gint i = 0;
  result = g_string_new(NULL);
  if (str_array) next_element = str_array[i++];
  else next_element = first_element;
  while (TRUE) {
      const gchar *element;
      const gchar *start;
      const gchar *end;
      if (next_element) {
          element = next_element;
          if (str_array) next_element = str_array[i++];
          else next_element = va_arg(*args, gchar *);
	  } else break;
      if (!*element) continue;
      start = element;
      if (TRUE) {
          while (start && (*start == '\\' || *start == '/')) {
              current_separator = *start;
              start++;
          }
	  }
      end = start + strlen (start);
      if (TRUE) {
          while (end >= start + 1 && (end[-1] == '\\' || end[-1] == '/')) {
              current_separator = end[-1];
              end--;
          }
          last_trailing = end;
          while (last_trailing >= element + 1 && (last_trailing[-1] == '\\' || last_trailing[-1] == '/')) last_trailing--;
          if (!have_leading) {
              if (last_trailing <= start) single_element = element;
              g_string_append_len(result, element, start - element);
              have_leading = TRUE;
          } else single_element = NULL;
	  }
      if (end == start) continue;
      if (!is_first) g_string_append_len(result, &current_separator, 1);
      g_string_append_len(result, start, end - start);
      is_first = FALSE;
  }
  if (single_element) {
      g_string_free(result, TRUE);
      return g_strdup(single_element);
  } else {
      if (last_trailing) g_string_append(result, last_trailing);
      return g_string_free(result, FALSE);
  }
}
#endif
gchar* g_build_filenamev(gchar **args) {
  gchar *str;
#ifndef G_OS_WIN32
  str = g_build_path_va(G_DIR_SEPARATOR_S, NULL, NULL, args);
#else
  str = g_build_pathname_va(NULL, NULL, args);
#endif
  return str;
}
gchar* g_build_filename(const gchar *first_element, ...) {
  gchar *str;
  va_list args;
  va_start(args, first_element);
#ifndef G_OS_WIN32
  str = g_build_path_va(G_DIR_SEPARATOR_S, first_element, &args, NULL);
#else
  str = g_build_pathname_va(first_element, &args, NULL);
#endif
  va_end(args);
  return str;
}
#define KILOBYTE_FACTOR (G_GOFFSET_CONSTANT (1024))
#define MEGABYTE_FACTOR (KILOBYTE_FACTOR * KILOBYTE_FACTOR)
#define GIGABYTE_FACTOR (MEGABYTE_FACTOR * KILOBYTE_FACTOR)
#define TERABYTE_FACTOR (GIGABYTE_FACTOR * KILOBYTE_FACTOR)
#define PETABYTE_FACTOR (TERABYTE_FACTOR * KILOBYTE_FACTOR)
#define EXABYTE_FACTOR  (PETABYTE_FACTOR * KILOBYTE_FACTOR)
char* g_format_size_for_display(goffset size) {
  if (size < (goffset) KILOBYTE_FACTOR) return g_strdup_printf(g_dngettext(GETTEXT_PACKAGE, "%u byte", "%u bytes",(guint)size), (guint)size);
  else {
      gdouble displayed_size;
      if (size < (goffset) MEGABYTE_FACTOR) {
          displayed_size = (gdouble) size / (gdouble) KILOBYTE_FACTOR;
          return g_strdup_printf (_("%.1f KB"), displayed_size);
	  } else if (size < (goffset) GIGABYTE_FACTOR) {
          displayed_size = (gdouble) size / (gdouble) MEGABYTE_FACTOR;
          return g_strdup_printf (_("%.1f MB"), displayed_size);
	  } else if (size < (goffset) TERABYTE_FACTOR) {
          displayed_size = (gdouble) size / (gdouble) GIGABYTE_FACTOR;
          return g_strdup_printf (_("%.1f GB"), displayed_size);
	  } else if (size < (goffset) PETABYTE_FACTOR) {
          displayed_size = (gdouble) size / (gdouble) TERABYTE_FACTOR;
          return g_strdup_printf (_("%.1f TB"), displayed_size);
	  } else if (size < (goffset) EXABYTE_FACTOR) {
          displayed_size = (gdouble) size / (gdouble) PETABYTE_FACTOR;
          return g_strdup_printf (_("%.1f PB"), displayed_size);
	  } else {
          displayed_size = (gdouble) size / (gdouble) EXABYTE_FACTOR;
          return g_strdup_printf (_("%.1f EB"), displayed_size);
      }
  }
}
gchar* g_file_read_link(const gchar  *filename, GError **error) {
#ifdef HAVE_READLINK
  gchar *buffer;
  guint size;
  gint read_size;
  size = 256; 
  buffer = g_malloc (size);
  while (TRUE) {
      read_size = readlink (filename, buffer, size);
      if (read_size < 0) {
          int save_errno = errno;
          gchar *display_filename = g_filename_display_name (filename);
          g_free (buffer);
          g_set_error(error, G_FILE_ERROR, g_file_error_from_errno (save_errno), _("Failed to read the symbolic link '%s': %s"), display_filename,
                      g_strerror (save_errno));
          g_free (display_filename);
          return NULL;
      }
      if (read_size < size) {
          buffer[read_size] = 0;
          return buffer;
	  }
      size *= 2;
      buffer = g_realloc (buffer, size);
  }
#else
  g_set_error_literal(error, G_FILE_ERROR,G_FILE_ERROR_INVAL, _("Symbolic links not supported"));
  return NULL;
#endif
}
#if !defined(G_OS_WIN32) && !defined (_WIN64)
#undef g_file_test
gboolean g_file_test(const gchar *filename, GFileTest test) {
  gchar *utf8_filename = g_locale_to_utf8 (filename, -1, NULL, NULL, NULL);
  gboolean retval;
  if (utf8_filename == NULL) return FALSE;
  retval = g_file_test_utf8(utf8_filename, test);
  g_free(utf8_filename);
  return retval;
}
#undef g_file_get_contents
gboolean g_file_get_contents(const gchar *filename, gchar **contents, gsize *length, GError **error) {
  gchar *utf8_filename = g_locale_to_utf8(filename, -1, NULL, NULL, error);
  gboolean retval;
  if (utf8_filename == NULL) return FALSE;
  retval = g_file_get_contents_utf8(utf8_filename, contents, length, error);
  g_free(utf8_filename);
  return retval;
}
#undef g_mkstemp
gint g_mkstemp(gchar *tmpl) {
  char *XXXXXX;
  int count, fd;
  static const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  static const int NLETTERS = sizeof(letters) - 1;
  glong value;
  GTimeVal tv;
  static int counter = 0;
  XXXXXX = g_strrstr (tmpl, "XXXXXX");
  if (!XXXXXX) {
      errno = EINVAL;
      return -1;
  }
  g_get_current_time (&tv);
  value = (tv.tv_usec ^ tv.tv_sec) + counter++;
  for (count = 0; count < 100; value += 7777, ++count) {
      glong v = value;
      XXXXXX[0] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[1] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[2] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[3] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[4] = letters[v % NLETTERS];
      v /= NLETTERS;
      XXXXXX[5] = letters[v % NLETTERS];
      fd = open (tmpl, O_RDWR | O_CREAT | O_EXCL | O_BINARY, 0600);
      if (fd >= 0) return fd;
      else if (errno != EEXIST) return -1;
  }
  errno = EEXIST;
  return -1;
}
#undef g_file_open_tmp
gint g_file_open_tmp(const gchar *tmpl, gchar **name_used, GError **error) {
  gchar *utf8_tmpl = g_locale_to_utf8(tmpl, -1, NULL, NULL, error);
  gchar *utf8_name_used;
  gint retval;
  if (utf8_tmpl == NULL) return -1;
  retval = g_file_open_tmp_utf8(utf8_tmpl, &utf8_name_used, error);
  if (retval == -1) return -1;
  if (name_used) *name_used = g_locale_from_utf8(utf8_name_used, -1, NULL, NULL, NULL);
  g_free(utf8_name_used);
  return retval;
}
#endif