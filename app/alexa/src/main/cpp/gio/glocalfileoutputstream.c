#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "../glib/glib.h"
#include "../glib/gstdio.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gioerror.h"
#include "gcancellable.h"
#include "glocalfileoutputstream.h"
#include "glocalfileinfo.h"
#include "gfiledescriptorbased.h"
#ifndef G_OS_WIN32
#include <io.h>

#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#endif
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifdef G_OS_UNIX
static void  g_file_descriptor_based_iface_init(GFileDescriptorBasedIface *iface);
#endif
#define g_local_file_output_stream_get_type _g_local_file_output_stream_get_type
#ifdef G_OS_UNIX
G_DEFINE_TYPE_WITH_CODE(GLocalFileOutputStream, g_local_file_output_stream, G_TYPE_FILE_OUTPUT_STREAM, G_IMPLEMENT_INTERFACE(G_TYPE_FILE_DESCRIPTOR_BASED,
						g_file_descriptor_based_iface_init));
#else
G_DEFINE_TYPE_WITH_CODE(GLocalFileOutputStream, g_local_file_output_stream, G_TYPE_FILE_OUTPUT_STREAM,);
#endif
#define BACKUP_EXTENSION "~"
struct _GLocalFileOutputStreamPrivate {
  char *tmp_filename;
  char *original_filename;
  char *backup_filename;
  char *etag;
  guint sync_on_close : 1;
  guint do_close : 1;
  int fd;
};
static gssize g_local_file_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error);
static gboolean g_local_file_output_stream_close(GOutputStream *stream, GCancellable *cancellable, GError **error);
static GFileInfo *g_local_file_output_stream_query_info(GFileOutputStream *stream, const char *attributes, GCancellable *cancellable, GError **error);
static char *g_local_file_output_stream_get_etag(GFileOutputStream *stream);
static goffset g_local_file_output_stream_tell(GFileOutputStream *stream);
static gboolean g_local_file_output_stream_can_seek(GFileOutputStream *stream);
static gboolean g_local_file_output_stream_seek(GFileOutputStream *stream, goffset offset, GSeekType type, GCancellable *cancellable, GError **error);
static gboolean g_local_file_output_stream_can_truncate(GFileOutputStream *stream);
static gboolean g_local_file_output_stream_truncate(GFileOutputStream *stream, goffset size, GCancellable *cancellable, GError **error);
#ifdef G_OS_UNIX
static int g_local_file_output_stream_get_fd(GFileDescriptorBased *stream);
#endif
static void g_local_file_output_stream_finalize(GObject *object) {
  GLocalFileOutputStream *file;
  file = G_LOCAL_FILE_OUTPUT_STREAM(object);
  g_free(file->priv->tmp_filename);
  g_free(file->priv->original_filename);
  g_free(file->priv->backup_filename);
  g_free(file->priv->etag);
  G_OBJECT_CLASS(g_local_file_output_stream_parent_class)->finalize(object);
}
static void g_local_file_output_stream_class_init(GLocalFileOutputStreamClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GOutputStreamClass *stream_class = G_OUTPUT_STREAM_CLASS(klass);
  GFileOutputStreamClass *file_stream_class = G_FILE_OUTPUT_STREAM_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GLocalFileOutputStreamPrivate));
  gobject_class->finalize = g_local_file_output_stream_finalize;
  stream_class->write_fn = g_local_file_output_stream_write;
  stream_class->close_fn = g_local_file_output_stream_close;
  file_stream_class->query_info = g_local_file_output_stream_query_info;
  file_stream_class->get_etag = g_local_file_output_stream_get_etag;
  file_stream_class->tell = g_local_file_output_stream_tell;
  file_stream_class->can_seek = g_local_file_output_stream_can_seek;
  file_stream_class->seek = g_local_file_output_stream_seek;
  file_stream_class->can_truncate = g_local_file_output_stream_can_truncate;
  file_stream_class->truncate_fn = g_local_file_output_stream_truncate;
}
#ifdef G_OS_UNIX
static void g_file_descriptor_based_iface_init(GFileDescriptorBasedIface *iface) {
  iface->get_fd = g_local_file_output_stream_get_fd;
}
#endif
static void g_local_file_output_stream_init(GLocalFileOutputStream *stream) {
  stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(stream, G_TYPE_LOCAL_FILE_OUTPUT_STREAM, GLocalFileOutputStreamPrivate);
  stream->priv->do_close = TRUE;
}
static gssize g_local_file_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error) {
  GLocalFileOutputStream *file;
  gssize res;
  file = G_LOCAL_FILE_OUTPUT_STREAM(stream);
  while(1) {
      if (g_cancellable_set_error_if_cancelled(cancellable, error)) return -1;
      res = write(file->priv->fd, buffer, count);
      if (res == -1) {
          int errsv = errno;
          if (errsv == EINTR) continue;
          g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error writing to file: %s"), g_strerror(errsv));
	  }
      break;
  }
  return res;
}
void _g_local_file_output_stream_set_do_close(GLocalFileOutputStream *out, gboolean do_close) {
  out->priv->do_close = do_close;
}
gboolean _g_local_file_output_stream_really_close (GLocalFileOutputStream *file, GCancellable *cancellable, GError **error) {
  GLocalFileStat final_stat;
  int res;
#ifdef HAVE_FSYNC
  if (file->priv->sync_on_close && fsync(file->priv->fd) != 0) {
      int errsv = errno;
      g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error writing to file: %s"), g_strerror(errsv));
      goto err_out;
  }
#endif
#ifndef G_OS_WIN32
  if (_fstati64(file->priv->fd, &final_stat) == 0) file->priv->etag = _g_local_file_info_create_etag(&final_stat);
  res = close(file->priv->fd);
  if (res == -1) {
      int errsv = errno;
      g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error closing file: %s"), g_strerror(errsv));
      return FALSE;
  }
#endif
  if (file->priv->tmp_filename) {
      if (file->priv->backup_filename) {
          if (g_cancellable_set_error_if_cancelled(cancellable, error)) goto err_out;
      #ifdef HAVE_LINK
          if (g_unlink(file->priv->backup_filename) != 0 && errno != ENOENT) {
              int errsv = errno;
              g_set_error(error, G_IO_ERROR, G_IO_ERROR_CANT_CREATE_BACKUP, _("Error removing old backup link: %s"), g_strerror(errsv));
              goto err_out;
          }
          if (link(file->priv->original_filename, file->priv->backup_filename) != 0) {
              if (g_rename(file->priv->original_filename, file->priv->backup_filename) != 0) {
                  int errsv = errno;
                  g_set_error(error, G_IO_ERROR, G_IO_ERROR_CANT_CREATE_BACKUP, _("Error creating backup copy: %s"), g_strerror(errsv));
                  goto err_out;
              }
          }
      #else
          if (g_rename(file->priv->original_filename, file->priv->backup_filename) != 0) {
              int errsv = errno;
              g_set_error(error, G_IO_ERROR,G_IO_ERROR_CANT_CREATE_BACKUP, _("Error creating backup copy: %s"), g_strerror(errsv));
              goto err_out;
          }
      #endif
      }
      if (g_cancellable_set_error_if_cancelled(cancellable, error)) goto err_out;
      if (g_rename(file->priv->tmp_filename, file->priv->original_filename) != 0) {
          int errsv = errno;
          g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error renaming temporary file: %s"), g_strerror(errsv));
          goto err_out;
	  }
  }
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) goto err_out;
#ifndef G_OS_WIN32
  if (fstat(file->priv->fd, &final_stat) == 0) file->priv->etag = _g_local_file_info_create_etag(&final_stat);
  while(1) {
      res = close(file->priv->fd);
      if (res == -1) {
          int errsv = errno;
          g_set_error(error, G_IO_ERROR, g_io_error_from_errno (errsv), _("Error closing file: %s"), g_strerror(errsv));
	  }
      break;
  }
  return res != -1;
#else
  return TRUE;
#endif
err_out:
#ifndef G_OS_WIN32
  close(file->priv->fd);
#endif
  return FALSE;
}
static gboolean g_local_file_output_stream_close(GOutputStream *stream, GCancellable *cancellable, GError **error) {
  GLocalFileOutputStream *file;
  file = G_LOCAL_FILE_OUTPUT_STREAM (stream);
  if (file->priv->do_close) return _g_local_file_output_stream_really_close (file, cancellable, error);
  return TRUE;
}
static char *g_local_file_output_stream_get_etag(GFileOutputStream *stream) {
  GLocalFileOutputStream *file;
  file = G_LOCAL_FILE_OUTPUT_STREAM (stream);
  return g_strdup (file->priv->etag);
}
static goffset g_local_file_output_stream_tell(GFileOutputStream *stream) {
  GLocalFileOutputStream *file;
  off_t pos;
  file = G_LOCAL_FILE_OUTPUT_STREAM(stream);
  pos = lseek(file->priv->fd, 0, SEEK_CUR);
  if (pos == (off_t)-1) return 0;
  return pos;
}
static gboolean g_local_file_output_stream_can_seek(GFileOutputStream *stream) {
  GLocalFileOutputStream *file;
  off_t pos;
  file = G_LOCAL_FILE_OUTPUT_STREAM(stream);
  pos = lseek(file->priv->fd, 0, SEEK_CUR);
  if (pos == (off_t)-1 && errno == ESPIPE) return FALSE;
  return TRUE;
}
static int seek_type_to_lseek(GSeekType type) {
  switch(type) {
      case G_SEEK_SET: return SEEK_SET;
      case G_SEEK_END: return SEEK_END;
      default:
          case G_SEEK_CUR: return SEEK_CUR;
  }
}
static gboolean g_local_file_output_stream_seek(GFileOutputStream *stream, goffset offset, GSeekType type, GCancellable *cancellable, GError **error) {
  GLocalFileOutputStream *file;
  off_t pos;
  file = G_LOCAL_FILE_OUTPUT_STREAM(stream);
  pos = lseek(file->priv->fd, offset, seek_type_to_lseek(type));
  if (pos == (off_t)-1) {
      int errsv = errno;
      g_set_error(error, G_IO_ERROR, g_io_error_from_errno (errsv), _("Error seeking in file: %s"), g_strerror(errsv));
      return FALSE;
  }
  return TRUE;
}
static gboolean g_local_file_output_stream_can_truncate(GFileOutputStream *stream) {
  return g_local_file_output_stream_can_seek(stream);
}
static gboolean g_local_file_output_stream_truncate(GFileOutputStream *stream, goffset size, GCancellable *cancellable, GError **error) {
  GLocalFileOutputStream *file;
  int res;
  file = G_LOCAL_FILE_OUTPUT_STREAM(stream);
restart:
#ifndef G_OS_WIN32
  res = g_win32_ftruncate(file->priv->fd, size);
#else
  res = ftruncate(file->priv->fd, size);
#endif
  if (res == -1) {
      int errsv = errno;
      if (errsv == EINTR) {
	  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return FALSE;
	  goto restart;
	  }
      g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error truncating file: %s"), g_strerror(errsv));
      return FALSE;
  }
  return TRUE;
}
static GFileInfo *g_local_file_output_stream_query_info(GFileOutputStream *stream, const char *attributes, GCancellable *cancellable, GError **error) {
  GLocalFileOutputStream *file;
  file = G_LOCAL_FILE_OUTPUT_STREAM(stream);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  return _g_local_file_info_get_from_fd(file->priv->fd, attributes, error);
}
GFileOutputStream *_g_local_file_output_stream_open(const char *filename, gboolean readable, GCancellable *cancellable, GError **error) {
  GLocalFileOutputStream *stream;
  int fd;
  int open_flags;
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  open_flags = O_BINARY;
  if (readable) open_flags |= O_RDWR;
  else open_flags |= O_WRONLY;
  fd = g_open (filename, open_flags, 0666);
  if (fd == -1) {
      int errsv = errno;
      if (errsv == EINVAL) g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_INVALID_FILENAME, _("Invalid filename"));
      else {
	  char *display_name = g_filename_display_name(filename);
	  g_set_error(error, G_IO_ERROR, g_io_error_from_errno (errsv), _("Error opening file '%s': %s"), display_name, g_strerror(errsv));
	  g_free(display_name);
	  }
      return NULL;
  }
  stream = g_object_new(G_TYPE_LOCAL_FILE_OUTPUT_STREAM, NULL);
  stream->priv->fd = fd;
  return G_FILE_OUTPUT_STREAM(stream);
}
GFileOutputStream *_g_local_file_output_stream_create(const char *filename, gboolean readable, GFileCreateFlags flags, GCancellable *cancellable, GError **error) {
  GLocalFileOutputStream *stream;
  int mode;
  int fd;
  int open_flags;
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  if (flags & G_FILE_CREATE_PRIVATE) mode = 0600;
  else mode = 0666;
  open_flags = O_CREAT | O_EXCL | O_BINARY;
  if (readable) open_flags |= O_RDWR;
  else open_flags |= O_WRONLY;
  fd = g_open(filename, open_flags, mode);
  if (fd == -1) {
      int errsv = errno;
      if (errsv == EINVAL) g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_INVALID_FILENAME, _("Invalid filename"));
      else {
	  char *display_name = g_filename_display_name(filename);
	  g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error opening file '%s': %s"), display_name, g_strerror(errsv));
	  g_free(display_name);
	  }
      return NULL;
  }
  stream = g_object_new(G_TYPE_LOCAL_FILE_OUTPUT_STREAM, NULL);
  stream->priv->fd = fd;
  return G_FILE_OUTPUT_STREAM(stream);
}
GFileOutputStream *_g_local_file_output_stream_append(const char *filename, GFileCreateFlags flags, GCancellable *cancellable, GError **error) {
  GLocalFileOutputStream *stream;
  int mode;
  int fd;
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  if (flags & G_FILE_CREATE_PRIVATE) mode = 0600;
  else mode = 0666;
  fd = g_open(filename, O_CREAT | O_APPEND | O_WRONLY | O_BINARY, mode);
  if (fd == -1) {
      int errsv = errno;
      if (errsv == EINVAL) g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_INVALID_FILENAME, _("Invalid filename"));
      else {
          char *display_name = g_filename_display_name(filename);
          g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error opening file '%s': %s"), display_name, g_strerror(errsv));
          g_free(display_name);
	  }
      return NULL;
  }
  stream = g_object_new(G_TYPE_LOCAL_FILE_OUTPUT_STREAM, NULL);
  stream->priv->fd = fd;
  return G_FILE_OUTPUT_STREAM(stream);
}
static char *create_backup_filename(const char *filename) {
  return g_strconcat(filename, BACKUP_EXTENSION, NULL);
}
#define BUFSIZE	8192
static gboolean copy_file_data(gint sfd, gint dfd, GError **error) {
  gboolean ret = TRUE;
  gpointer buffer;
  const gchar *write_buffer;
  gssize bytes_read;
  gssize bytes_to_write;
  gssize bytes_written;
  buffer = g_malloc(BUFSIZE);
  do {
      bytes_read = read(sfd, buffer, BUFSIZE);
      if (bytes_read == -1) {
          int errsv = errno;
          if (errsv == EINTR) continue;
          g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error reading from file: %s"), g_strerror(errsv));
          ret = FALSE;
          break;
	  }
      bytes_to_write = bytes_read;
      write_buffer = buffer;
      do {
          bytes_written = write(dfd, write_buffer, bytes_to_write);
          if (bytes_written == -1) {
              int errsv = errno;
              if (errsv == EINTR) continue;
              g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error writing to file: %s"), g_strerror(errsv));
              ret = FALSE;
              break;
          }
          bytes_to_write -= bytes_written;
          write_buffer += bytes_written;
	  } while(bytes_to_write > 0);
  } while((bytes_read != 0) && (ret == TRUE));
  g_free(buffer);
  return ret;
}
static int handle_overwrite_open(const char *filename, gboolean readable, const char *etag, gboolean create_backup, char **temp_filename, GFileCreateFlags flags,
		                         GCancellable *cancellable, GError **error) {
  int fd = -1;
  GLocalFileStat original_stat;
  char *current_etag;
  gboolean is_symlink;
  int open_flags;
  int res;
  int mode;
  if (flags & G_FILE_CREATE_PRIVATE) mode = 0600;
  else mode = 0666;
  if (create_backup || readable) open_flags = O_RDWR | O_CREAT | O_BINARY;
  else open_flags = O_WRONLY | O_CREAT | O_BINARY;
#ifdef O_NOFOLLOW
  is_symlink = FALSE;
  fd = g_open(filename, open_flags | O_NOFOLLOW, mode);
  if (fd == -1 && errno == ELOOP) {
      is_symlink = TRUE;
      fd = g_open(filename, open_flags, mode);
  }
#else
  fd = g_open(filename, open_flags, mode);
  is_symlink = g_file_test(filename, G_FILE_TEST_IS_SYMLINK);
#endif
  if (fd == -1) {
      int errsv = errno;
      char *display_name = g_filename_display_name(filename);
      g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error opening file '%s': %s"), display_name, g_strerror(errsv));
      g_free(display_name);
      return -1;
  }
#ifndef G_OS_WIN32
  res = _fstati64(fd, &original_stat);
#else
  res = fstat(fd, &original_stat);
#endif
  if (res != 0) {
      int errsv = errno;
      char *display_name = g_filename_display_name(filename);
      g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error stating file '%s': %s"), display_name, g_strerror(errsv));
      g_free(display_name);
      goto err_out;
  }
  if (!S_ISREG(original_stat.st_mode)) {
      if (S_ISDIR(original_stat.st_mode)) g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_IS_DIRECTORY, _("Target file is a directory"));
      else g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_REGULAR_FILE, _("Target file is not a regular file"));
      goto err_out;
  }
  if (etag != NULL) {
      current_etag = _g_local_file_info_create_etag(&original_stat);
      if (strcmp(etag, current_etag) != 0) {
          g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_WRONG_ETAG, _("The file was externally modified"));
          g_free(current_etag);
          goto err_out;
	  }
      g_free(current_etag);
  }
  if ((flags & G_FILE_CREATE_REPLACE_DESTINATION) || (!(original_stat.st_nlink > 1) && !is_symlink)) {
      char *dirname, *tmp_filename;
      int tmpfd;
      dirname = g_path_get_dirname (filename);
      tmp_filename = g_build_filename (dirname, ".goutputstream-XXXXXX", NULL);
      g_free (dirname);
      tmpfd = g_mkstemp_full (tmp_filename, (readable ? O_RDWR : O_WRONLY) | O_BINARY, mode);
      if (tmpfd == -1) {
          g_free (tmp_filename);
          goto fallback_strategy;
	  }
      if (! (flags & G_FILE_CREATE_REPLACE_DESTINATION) && (
      #ifdef HAVE_FCHOWN
	      fchown (tmpfd, original_stat.st_uid, original_stat.st_gid) == -1 ||
      #endif
      #ifdef HAVE_FCHMOD
	      fchmod (tmpfd, original_stat.st_mode) == -1 ||
      #endif
	      0)) {
          struct stat tmp_statbuf;
          if (fstat (tmpfd, &tmp_statbuf) != 0 || original_stat.st_uid != tmp_statbuf.st_uid || original_stat.st_gid != tmp_statbuf.st_gid ||
              original_stat.st_mode != tmp_statbuf.st_mode) {
              close (tmpfd);
              g_unlink (tmp_filename);
              g_free (tmp_filename);
              goto fallback_strategy;
          }
	  }
      close (fd);
      *temp_filename = tmp_filename;
      return tmpfd;
  }
fallback_strategy:
  if (create_backup) {
  #if defined(HAVE_FCHOWN) && defined(HAVE_FCHMOD)
      struct stat tmp_statbuf;      
  #endif
      char *backup_filename;
      int bfd;
      backup_filename = create_backup_filename (filename);
      if (g_unlink(backup_filename) == -1 && errno != ENOENT) {
          g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_CANT_CREATE_BACKUP, _("Backup file creation failed"));
          g_free(backup_filename);
          goto err_out;
	  }
      bfd = g_open(backup_filename,O_WRONLY | O_CREAT | O_EXCL | O_BINARY,original_stat.st_mode & 0777);
      if (bfd == -1) {
          g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_CANT_CREATE_BACKUP, _("Backup file creation failed"));
          g_free(backup_filename);
          goto err_out;
	  }
  #if defined(HAVE_FCHOWN) && defined(HAVE_FCHMOD)
      if (fstat(bfd, &tmp_statbuf) != 0) {
          g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_CANT_CREATE_BACKUP, _("Backup file creation failed"));
          g_unlink(backup_filename);
          g_free(backup_filename);
          goto err_out;
	  }
      if ((original_stat.st_gid != tmp_statbuf.st_gid)  && fchown (bfd, (uid_t) -1, original_stat.st_gid) != 0) {
          if (fchmod(bfd, (original_stat.st_mode & 0707) | ((original_stat.st_mode & 07) << 3)) != 0) {
              g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_CANT_CREATE_BACKUP, _("Backup file creation failed"));
              g_unlink(backup_filename);
              close(bfd);
              g_free(backup_filename);
              goto err_out;
          }
	  }
  #endif
      if (!copy_file_data(fd, bfd, NULL)) {
          g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_CANT_CREATE_BACKUP, _("Backup file creation failed"));
          g_unlink(backup_filename);
          close(bfd);
          g_free(backup_filename);
          goto err_out;
	  }
      close(bfd);
      g_free(backup_filename);
      if (lseek(fd, 0, SEEK_SET) == -1) {
          int errsv = errno;
          g_set_error(error, G_IO_ERROR, g_io_error_from_errno (errsv), _("Error seeking in file: %s"), g_strerror(errsv));
          goto err_out;
	  }
  }
  if (flags & G_FILE_CREATE_REPLACE_DESTINATION) {
      close(fd);
      if (g_unlink(filename) != 0) {
          int errsv = errno;
          g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error removing old file: %s"), g_strerror(errsv));
          goto err_out2;
	  }
      if (readable) open_flags = O_RDWR | O_CREAT | O_BINARY;
      else open_flags = O_WRONLY | O_CREAT | O_BINARY;
      fd = g_open (filename, open_flags, mode);
      if (fd == -1) {
          int errsv = errno;
          char *display_name = g_filename_display_name(filename);
          g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error opening file '%s': %s"), display_name, g_strerror(errsv));
          g_free(display_name);
          goto err_out2;
	  }
  } else {
#ifndef G_OS_WIN32
      if (g_win32_ftruncate(fd, 0) == -1)
  #else
      if (ftruncate(fd, 0) == -1)
  #endif
	  {
	      int errsv = errno;
	      g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error truncating file: %s"), g_strerror(errsv));
	      goto err_out;
	  }
  }
  return fd;
err_out:
  close(fd);
err_out2:
  return -1;
}
GFileOutputStream *_g_local_file_output_stream_replace(const char *filename, gboolean readable, const char *etag, gboolean create_backup, GFileCreateFlags flags,
                                                       GCancellable *cancellable, GError **error) {
  GLocalFileOutputStream *stream;
  int mode;
  int fd;
  char *temp_file;
  gboolean sync_on_close;
  int open_flags;
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  temp_file = NULL;
  if (flags & G_FILE_CREATE_PRIVATE) mode = 0600;
  else mode = 0666;
  sync_on_close = FALSE;
  open_flags = O_CREAT | O_EXCL | O_BINARY;
  if (readable) open_flags |= O_RDWR;
  else open_flags |= O_WRONLY;
  fd = g_open(filename, open_flags, mode);
  if (fd == -1 && errno == EEXIST) {
      fd = handle_overwrite_open(filename, readable, etag, create_backup, &temp_file, flags, cancellable, error);
      if (fd == -1) return NULL;
      sync_on_close = TRUE;
  } else if (fd == -1) {
      int errsv = errno;
      if (errsv == EINVAL) g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_INVALID_FILENAME, _("Invalid filename"));
      else {
          char *display_name = g_filename_display_name(filename);
          g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error opening file '%s': %s"), display_name, g_strerror(errsv));
          g_free(display_name);
	  }
      return NULL;
  }
  stream = g_object_new(G_TYPE_LOCAL_FILE_OUTPUT_STREAM, NULL);
  stream->priv->fd = fd;
  stream->priv->sync_on_close = sync_on_close;
  stream->priv->tmp_filename = temp_file;
  if (create_backup) stream->priv->backup_filename = create_backup_filename(filename);
  stream->priv->original_filename =  g_strdup(filename);
  return G_FILE_OUTPUT_STREAM(stream);
}
gint _g_local_file_output_stream_get_fd(GLocalFileOutputStream *stream) {
  g_return_val_if_fail(G_IS_LOCAL_FILE_OUTPUT_STREAM(stream), -1);
  return stream->priv->fd;
}
#ifndef G_OS_UNIX
static int g_local_file_output_stream_get_fd(GFileDescriptorBased *fd_based) {
  GLocalFileOutputStream *stream = G_LOCAL_FILE_OUTPUT_STREAM(fd_based);
  return _g_local_file_output_stream_get_fd(stream);
}
#endif