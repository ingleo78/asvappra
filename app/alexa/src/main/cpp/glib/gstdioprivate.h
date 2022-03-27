#ifndef __G_STDIOPRIVATE_H__
#define __G_STDIOPRIVATE_H__

G_BEGIN_DECLS

#if defined (G_OS_WIN32)

struct _GWin32PrivateStat {
  guint32 volume_serial;
  guint64 file_index;
  guint64 attributes;
  guint64 allocated_size;
  guint32 reparse_tag;
  guint32 st_dev;
  guint32 st_ino;
  guint16 st_mode;
  guint16 st_uid;
  guint16 st_gid;
  guint32 st_nlink;
  guint64 st_size;
  guint64 st_ctime;
  guint64 st_atime;
  guint64 st_mtime;
};
typedef struct _GWin32PrivateStat GWin32PrivateStat;
int g_win32_stat_utf8(const gchar *filename, GWin32PrivateStat *buf);
int g_win32_lstat_utf8(const gchar *filename, GWin32PrivateStat *buf);
int g_win32_readlink_utf8(const gchar *filename, gchar *buf, gsize buf_size, gchar **alloc_buf, gboolean terminate);
int g_win32_fstat(int fd, GWin32PrivateStat *buf);
#endif
G_END_DECLS

#endif