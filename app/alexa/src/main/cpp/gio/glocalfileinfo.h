#ifndef __G_LOCAL_FILE_INFO_H__
#define __G_LOCAL_FILE_INFO_H__

#include <sys/stat.h>
#include <sys/types.h>
#include "gfileinfo.h"
#include "gfile.h"

G_BEGIN_DECLS
typedef struct {
  gboolean writable;
  gboolean is_sticky;
  gboolean has_trash_dir;
  int owner;
  dev_t device;
  gpointer extra_data;
  GDestroyNotify free_extra_data;
} GLocalParentFileInfo;
#ifndef G_OS_WIN32
#define GLocalFileStat struct _stati64
#else
#define GLocalFileStat struct stat
#endif
gboolean _g_local_file_has_trash_dir(const char *dirname, dev_t dir_dev);
void _g_local_file_info_get_parent_info(const char *dir, GFileAttributeMatcher *attribute_matcher, GLocalParentFileInfo *parent_info);
void _g_local_file_info_free_parent_info(GLocalParentFileInfo *parent_info);
GFileInfo *_g_local_file_info_get(const char *basename, const char *path, GFileAttributeMatcher *attribute_matcher, GFileQueryInfoFlags flags,
                                  GLocalParentFileInfo *parent_info, GError **error);
GFileInfo *_g_local_file_info_get_from_fd(int fd, const char *attributes, GError **error);
char *_g_local_file_info_create_etag(GLocalFileStat *statbuf);
gboolean _g_local_file_info_set_attribute(char *filename, const char *attribute, GFileAttributeType type, gpointer value_p, GFileQueryInfoFlags flags,
                                          GCancellable *cancellable, GError **error);
gboolean _g_local_file_info_set_attributes(char *filename, GFileInfo *info, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
G_END_DECLS

#endif