#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include "../glib/glibintl.h"
#define _GNU_SOURCE
#include "config.h"
#include "gfile.h"
#include "gvfs.h"
#include "gioscheduler.h"
#include "gsimpleasyncresult.h"
#include "gfileattribute-priv.h"
#include "gfiledescriptorbased.h"
#include "gpollfilemonitor.h"
#include "gappinfo.h"
#include "gfileinputstream.h"
#include "gfileoutputstream.h"
#include "gcancellable.h"
#include "gasyncresult.h"
#include "gioerror.h"

typedef gboolean (*GFileReadMoreCallback)(const char *file_contents, goffset file_size, gpointer callback_data);
static void g_file_real_query_info_async(GFile *file, const char *attributes, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable,
								         GAsyncReadyCallback callback, gpointer user_data);
static GFileInfo *g_file_real_query_info_finish(GFile *file, GAsyncResult *res, GError **error);
static void g_file_real_query_filesystem_info_async(GFile *file, const char *attributes, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
								                    gpointer user_data);
static GFileInfo *g_file_real_query_filesystem_info_finish(GFile *file, GAsyncResult *res, GError **error);
static void g_file_real_enumerate_children_async(GFile *file, const char *attributes, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable,
								                 GAsyncReadyCallback callback, gpointer user_data);
static GFileEnumerator *g_file_real_enumerate_children_finish(GFile *file, GAsyncResult *res, GError **error);
static void g_file_real_read_async(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
static GFileInputStream *g_file_real_read_finish(GFile *file, GAsyncResult *res, GError **error);
static void g_file_real_append_to_async(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
								        gpointer user_data);
static GFileOutputStream *g_file_real_append_to_finish(GFile *file, GAsyncResult *res, GError **error);
static void g_file_real_create_async(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
								     gpointer user_data);
static GFileOutputStream *g_file_real_create_finish(GFile *file, GAsyncResult *res, GError **error);
static void g_file_real_replace_async(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, int io_priority, GCancellable *cancellable,
                                      GAsyncReadyCallback callback, gpointer user_data);
static GFileOutputStream *g_file_real_replace_finish(GFile *file, GAsyncResult *res, GError **error);
static void g_file_real_open_readwrite_async(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
static GFileIOStream *g_file_real_open_readwrite_finish(GFile *file, GAsyncResult *res, GError **error);
static void g_file_real_create_readwrite_async(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                               gpointer user_data);
static GFileIOStream *g_file_real_create_readwrite_finish(GFile *file, GAsyncResult *res,  GError **error);
static void g_file_real_replace_readwrite_async(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, int io_priority,
                                                GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
static GFileIOStream *g_file_real_replace_readwrite_finish(GFile *file, GAsyncResult *res, GError **error);
static gboolean g_file_real_set_attributes_from_info(GFile *file, GFileInfo *info, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
static void g_file_real_set_display_name_async(GFile *file, const char *display_name, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                               gpointer user_data);
static GFile *g_file_real_set_display_name_finish(GFile *file, GAsyncResult *res, GError **error);
static void g_file_real_set_attributes_async(GFile *file, GFileInfo *info, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable,
                                             GAsyncReadyCallback callback, gpointer user_data);
static gboolean g_file_real_set_attributes_finish(GFile *file, GAsyncResult *res, GFileInfo **info, GError **error);
static void g_file_real_find_enclosing_mount_async(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
static GMount *g_file_real_find_enclosing_mount_finish(GFile *file, GAsyncResult *res, GError **error);
static void g_file_real_copy_async(GFile *source, GFile *destination, GFileCopyFlags flags, int io_priority, GCancellable *cancellable,
								   GFileProgressCallback progress_callback, gpointer progress_callback_data, GAsyncReadyCallback callback, gpointer user_data);
static gboolean g_file_real_copy_finish(GFile *file, GAsyncResult *res, GError **error);
typedef GFileIface GFileInterface;
G_DEFINE_INTERFACE(GFile, g_file, G_TYPE_OBJECT);
static void g_file_default_init(GFileIface *iface) {
  iface->enumerate_children_async = g_file_real_enumerate_children_async;
  iface->enumerate_children_finish = g_file_real_enumerate_children_finish;
  iface->set_display_name_async = g_file_real_set_display_name_async;
  iface->set_display_name_finish = g_file_real_set_display_name_finish;
  iface->query_info_async = g_file_real_query_info_async;
  iface->query_info_finish = g_file_real_query_info_finish;
  iface->query_filesystem_info_async = g_file_real_query_filesystem_info_async;
  iface->query_filesystem_info_finish = g_file_real_query_filesystem_info_finish;
  iface->set_attributes_async = g_file_real_set_attributes_async;
  iface->set_attributes_finish = g_file_real_set_attributes_finish;
  iface->read_async = g_file_real_read_async;
  iface->read_finish = g_file_real_read_finish;
  iface->append_to_async = g_file_real_append_to_async;
  iface->append_to_finish = g_file_real_append_to_finish;
  iface->create_async = g_file_real_create_async;
  iface->create_finish = g_file_real_create_finish;
  iface->replace_async = g_file_real_replace_async;
  iface->replace_finish = g_file_real_replace_finish;
  iface->open_readwrite_async = g_file_real_open_readwrite_async;
  iface->open_readwrite_finish = g_file_real_open_readwrite_finish;
  iface->create_readwrite_async = g_file_real_create_readwrite_async;
  iface->create_readwrite_finish = g_file_real_create_readwrite_finish;
  iface->replace_readwrite_async = g_file_real_replace_readwrite_async;
  iface->replace_readwrite_finish = g_file_real_replace_readwrite_finish;
  iface->find_enclosing_mount_async = g_file_real_find_enclosing_mount_async;
  iface->find_enclosing_mount_finish = g_file_real_find_enclosing_mount_finish;
  iface->set_attributes_from_info = g_file_real_set_attributes_from_info;
  iface->copy_async = g_file_real_copy_async;
  iface->copy_finish = g_file_real_copy_finish;
}
gboolean g_file_is_native(GFile *file) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  iface = G_FILE_GET_IFACE(file);
  return (*iface->is_native)(file);
}
gboolean g_file_has_uri_scheme(GFile *file, const char *uri_scheme) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(uri_scheme != NULL, FALSE);
  iface = G_FILE_GET_IFACE(file);
  return (*iface->has_uri_scheme)(file, uri_scheme);
}
char *g_file_get_uri_scheme(GFile *file) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  iface = G_FILE_GET_IFACE(file);
  return (*iface->get_uri_scheme)(file);
  return NULL;
}
char *g_file_get_basename(GFile *file) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  iface = G_FILE_GET_IFACE(file);
  return (*iface->get_basename)(file);
}
char *g_file_get_path(GFile *file) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  iface = G_FILE_GET_IFACE(file);
  return (*iface->get_path)(file);
}
char *g_file_get_uri(GFile *file) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  iface = G_FILE_GET_IFACE(file);
  return (*iface->get_uri)(file);
}
char *g_file_get_parse_name(GFile *file) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  iface = G_FILE_GET_IFACE(file);
  return (*iface->get_parse_name)(file);
}
GFile *g_file_dup(GFile *file) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  iface = G_FILE_GET_IFACE(file);
  return (*iface->dup)(file);
}
guint g_file_hash(gconstpointer file) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), 0);
  iface = G_FILE_GET_IFACE(file);
  return (*iface->hash)((GFile*)file);
}
gboolean g_file_equal(GFile *file1, GFile *file2) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file1), FALSE);
  g_return_val_if_fail(G_IS_FILE(file2), FALSE);
  if (G_TYPE_FROM_INSTANCE(file1) != G_TYPE_FROM_INSTANCE(file2)) return FALSE;
  iface = G_FILE_GET_IFACE(file1);
  return (*iface->equal)(file1, file2);
}
GFile *g_file_get_parent(GFile *file) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  iface = G_FILE_GET_IFACE(file);
  return (*iface->get_parent)(file);
}
gboolean g_file_has_parent(GFile *file, GFile *parent) {
  GFile *actual_parent;
  gboolean result;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(parent == NULL || G_IS_FILE(parent), FALSE);
  actual_parent = g_file_get_parent(file);
  if (actual_parent != NULL) {
      if (parent != NULL) result = g_file_equal(parent, actual_parent);
      else result = TRUE;
      g_object_unref(actual_parent);
  } else result = FALSE;
  return result;
}
GFile *g_file_get_child(GFile *file, const char *name) {
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(name != NULL, NULL);
  return g_file_resolve_relative_path(file, name);
}
GFile *
g_file_get_child_for_display_name(GFile *file, const char *display_name, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(display_name != NULL, NULL);
  iface = G_FILE_GET_IFACE(file);
  return (*iface->get_child_for_display_name)(file, display_name, error);
}
gboolean g_file_has_prefix(GFile *file, GFile *prefix) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(G_IS_FILE(prefix), FALSE);
  if (G_TYPE_FROM_INSTANCE(file) != G_TYPE_FROM_INSTANCE(prefix)) return FALSE;
  iface = G_FILE_GET_IFACE(file);
  return (*iface->prefix_matches)(prefix, file);
}
char *g_file_get_relative_path(GFile *parent, GFile *descendant) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(parent), NULL);
  g_return_val_if_fail(G_IS_FILE(descendant), NULL);
  if (G_TYPE_FROM_INSTANCE(parent) != G_TYPE_FROM_INSTANCE(descendant)) return NULL;
  iface = G_FILE_GET_IFACE(parent);
  return (*iface->get_relative_path)(parent, descendant);
}
GFile *g_file_resolve_relative_path(GFile *file, const char *relative_path) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(relative_path != NULL, NULL);
  iface = G_FILE_GET_IFACE(file);
  return (*iface->resolve_relative_path)(file, relative_path);
}
GFileEnumerator *g_file_enumerate_children(GFile *file, const char *attributes, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  if (iface->enumerate_children == NULL) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED, "Operation not supported");
      return NULL;
  }
  return (*iface->enumerate_children)(file, attributes, flags, cancellable, error);
}
void g_file_enumerate_children_async(GFile *file, const char *attributes, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable,
				                     GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  (*iface->enumerate_children_async)(file, attributes, flags, io_priority, cancellable, callback, user_data);
}
GFileEnumerator *g_file_enumerate_children_finish(GFile *file, GAsyncResult *res, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->enumerate_children_finish)(file, res, error);
}
gboolean g_file_query_exists(GFile *file, GCancellable *cancellable) {
  GFileInfo *info;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_TYPE, G_FILE_QUERY_INFO_NONE, cancellable, NULL);
  if (info != NULL) {
      g_object_unref(info);
      return TRUE;
  }
  return FALSE;
}
GFileType g_file_query_file_type(GFile *file, GFileQueryInfoFlags flags, GCancellable *cancellable) {
  GFileInfo *info;
  GFileType file_type;
  g_return_val_if_fail(G_IS_FILE(file), G_FILE_TYPE_UNKNOWN);
  info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_TYPE, flags, cancellable, NULL);
  if (info != NULL) {
      file_type = g_file_info_get_file_type(info);
      g_object_unref(info);
  } else file_type = G_FILE_TYPE_UNKNOWN;
  return file_type;
}
GFileInfo *g_file_query_info(GFile *file, const char *attributes, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE (file), NULL);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  if (iface->query_info == NULL) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED, "Operation not supported");
      return NULL;
  }
  return (*iface->query_info)(file, attributes, flags, cancellable, error);
}
void g_file_query_info_async(GFile *file, const char *attributes, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable,
                             GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  (*iface->query_info_async)(file, attributes, flags, io_priority, cancellable, callback, user_data);
}
GFileInfo *g_file_query_info_finish(GFile *file, GAsyncResult *res, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->query_info_finish)(file, res, error);
}
GFileInfo *g_file_query_filesystem_info(GFile *file, const char *attributes, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  if (iface->query_filesystem_info == NULL) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED, "Operation not supported");
      return NULL;
  }
  return (*iface->query_filesystem_info)(file, attributes, cancellable, error);
}
void g_file_query_filesystem_info_async(GFile *file, const char *attributes, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                        gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  (*iface->query_filesystem_info_async)(file, attributes, io_priority, cancellable, callback, user_data);
}
GFileInfo *g_file_query_filesystem_info_finish(GFile *file, GAsyncResult *res, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->query_filesystem_info_finish)(file, res, error);
}
GMount *g_file_find_enclosing_mount(GFile *file, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  if (iface->find_enclosing_mount == NULL){
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND, "Containing mount does not exist");
      return NULL;
  }
  return (*iface->find_enclosing_mount)(file, cancellable, error);
}
void g_file_find_enclosing_mount_async(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  (*iface->find_enclosing_mount_async)(file, io_priority, cancellable, callback, user_data);
}
GMount *g_file_find_enclosing_mount_finish(GFile *file, GAsyncResult *res, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->find_enclosing_mount_finish)(file, res, error);
}
GFileInputStream *g_file_read(GFile *file, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  if (iface->read_fn == NULL) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED, "Operation not supported");
      return NULL;
  }
  return (*iface->read_fn)(file, cancellable, error);
}
GFileOutputStream *g_file_append_to(GFile *file, GFileCreateFlags flags, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  if (iface->append_to == NULL) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED, "Operation not supported");
      return NULL;
  }
  return (*iface->append_to)(file, flags, cancellable, error);
}
GFileOutputStream *g_file_create(GFile *file, GFileCreateFlags flags, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  if (iface->create == NULL) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED, "Operation not supported");
      return NULL;
  }
  return (*iface->create)(file, flags, cancellable, error);
}
GFileOutputStream *g_file_replace(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  if (iface->replace == NULL) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, "Operation not supported");
      return NULL;
  }
  if (etag && *etag == 0) etag = NULL;
  return (*iface->replace)(file, etag, make_backup, flags, cancellable, error);
}
GFileIOStream *g_file_open_readwrite(GFile *file, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  if (iface->open_readwrite == NULL) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED, "Operation not supported");
      return NULL;
  }
  return (*iface->open_readwrite)(file, cancellable, error);
}
GFileIOStream *g_file_create_readwrite(GFile *file, GFileCreateFlags flags, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  if (iface->create_readwrite == NULL) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED, "Operation not supported");
      return NULL;
  }
  return (*iface->create_readwrite)(file, flags, cancellable, error);
}
GFileIOStream *g_file_replace_readwrite(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  if (iface->replace_readwrite == NULL) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED, "Operation not supported");
      return NULL;
  }
  return (*iface->replace_readwrite)(file, etag, make_backup, flags, cancellable, error);
}
void g_file_read_async(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  (*iface->read_async)(file, io_priority, cancellable, callback, user_data);
}
GFileInputStream *g_file_read_finish(GFile *file, GAsyncResult *res, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT (res), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->read_finish)(file, res, error);
}
void g_file_append_to_async(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  (*iface->append_to_async)(file, flags, io_priority, cancellable, callback, user_data);
}
GFileOutputStream *g_file_append_to_finish(GFile *file, GAsyncResult *res, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->append_to_finish)(file, res, error);
}
void g_file_create_async(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  (*iface->create_async)(file, flags, io_priority, cancellable, callback, user_data);
}
GFileOutputStream *g_file_create_finish(GFile *file, GAsyncResult *res, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT (res), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->create_finish)(file, res, error);
}
void g_file_replace_async(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, int io_priority, GCancellable *cancellable,
                          GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  (*iface->replace_async)(file, etag, make_backup, flags, io_priority, cancellable, callback, user_data);
}
GFileOutputStream *g_file_replace_finish(GFile *file, GAsyncResult *res, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->replace_finish)(file, res, error);
}
void g_file_open_readwrite_async(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  (*iface->open_readwrite_async)(file, io_priority, cancellable, callback, user_data);
}
GFileIOStream *g_file_open_readwrite_finish(GFile *file, GAsyncResult *res, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->open_readwrite_finish)(file, res, error);
}
void g_file_create_readwrite_async(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                   gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  (* iface->create_readwrite_async)(file, flags, io_priority, cancellable, callback, user_data);
}
GFileIOStream *g_file_create_readwrite_finish(GFile *file, GAsyncResult *res, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->create_readwrite_finish)(file, res, error);
}
void g_file_replace_readwrite_async(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, int io_priority, GCancellable *cancellable,
                                    GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE (file));
  iface = G_FILE_GET_IFACE(file);
  (*iface->replace_readwrite_async)(file, etag, make_backup, flags, io_priority, cancellable, callback, user_data);
}
GFileIOStream *g_file_replace_readwrite_finish(GFile *file, GAsyncResult *res, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->replace_readwrite_finish)(file, res, error);
}
static gboolean copy_symlink(GFile *destination, GFileCopyFlags flags, GCancellable *cancellable, const char *target, GError **error) {
  GError *my_error;
  gboolean tried_delete;
  GFileInfo *info;
  GFileType file_type;
  tried_delete = FALSE;
retry:
  my_error = NULL;
  if (!g_file_make_symbolic_link(destination, target, cancellable, &my_error)) {
      if (!tried_delete && (flags & G_FILE_COPY_OVERWRITE) && my_error->domain == G_IO_ERROR && my_error->code == G_IO_ERROR_EXISTS) {
          g_error_free(my_error);
          info = g_file_query_info(destination, G_FILE_ATTRIBUTE_STANDARD_TYPE, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, cancellable, &my_error);
          if (info != NULL) {
              file_type = g_file_info_get_file_type(info);
              g_object_unref(info);
              if (file_type == G_FILE_TYPE_DIRECTORY) {
              g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_IS_DIRECTORY, "Can't copy over directory");
              return FALSE;
              }
          }
          if (!g_file_delete(destination, cancellable, error)) return FALSE;
          tried_delete = TRUE;
          goto retry;
      }
      g_propagate_error(error, my_error);
      return FALSE;
  }
  return TRUE;
}
static GInputStream *open_source_for_copy(GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable, GError **error) {
  GError *my_error;
  GInputStream *in;
  GFileInfo *info;
  GFileType file_type;
  my_error = NULL;
  in = (GInputStream *) g_file_read(source, cancellable, &my_error);
  if (in != NULL) return in;
  if (my_error->domain == G_IO_ERROR && my_error->code == G_IO_ERROR_IS_DIRECTORY) {
      g_error_free(my_error);
      my_error = NULL;
      info = g_file_query_info(destination, G_FILE_ATTRIBUTE_STANDARD_TYPE, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, cancellable, &my_error);
      if (info != NULL && g_file_info_has_attribute(info, G_FILE_ATTRIBUTE_STANDARD_TYPE)) {
          file_type = g_file_info_get_file_type(info);
          g_object_unref(info);
          if (flags & G_FILE_COPY_OVERWRITE) {
              if (file_type == G_FILE_TYPE_DIRECTORY) {
              g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_WOULD_MERGE, "Can't copy directory over directory");
              return NULL;
              }
          } else {
              g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_EXISTS, "Target file exists");
              return NULL;
          }
	  } else {
          if (my_error != NULL && !g_error_matches(my_error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
              g_propagate_error(error, my_error);
              return NULL;
          }
          g_clear_error(&my_error);
	  }
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_WOULD_RECURSE,"Can't recursively copy directory");
      return NULL;
  }
  g_propagate_error(error, my_error);
  return NULL;
}
static gboolean should_copy(GFileAttributeInfo *info, gboolean as_move, gboolean skip_perms) {
  if (skip_perms && strcmp(info->name, "unix::mode") == 0) return FALSE;
  if (as_move) return info->flags & G_FILE_ATTRIBUTE_INFO_COPY_WHEN_MOVED;
  return info->flags & G_FILE_ATTRIBUTE_INFO_COPY_WITH_FILE;
}
static char *build_attribute_list_for_copy(GFileAttributeInfoList *attributes, GFileAttributeInfoList *namespaces, gboolean as_move,
                                           gboolean skip_perms) {
  GString *s;
  gboolean first;
  int i;
  first = TRUE;
  s = g_string_new("");
  if (attributes) {
      for (i = 0; i < attributes->n_infos; i++) {
          if (should_copy(&attributes->infos[i], as_move, skip_perms)) {
              if (first) first = FALSE;
              else g_string_append_c(s, ',');
              g_string_append(s, attributes->infos[i].name);
          }
	  }
  }
  if (namespaces) {
      for (i = 0; i < namespaces->n_infos; i++) {
          if (should_copy(&namespaces->infos[i], as_move, FALSE)) {
              if (first) first = FALSE;
              else g_string_append_c(s, ',');
              g_string_append(s, namespaces->infos[i].name);
              g_string_append(s, "::*");
          }
	  }
  }
  return g_string_free(s, FALSE);
}
gboolean g_file_copy_attributes(GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable, GError **error) {
  GFileAttributeInfoList *attributes, *namespaces;
  char *attrs_to_read;
  gboolean res;
  GFileInfo *info;
  gboolean as_move;
  gboolean source_nofollow_symlinks;
  gboolean skip_perms;
  as_move = flags & G_FILE_COPY_ALL_METADATA;
  source_nofollow_symlinks = flags & G_FILE_COPY_NOFOLLOW_SYMLINKS;
  skip_perms = (flags & G_FILE_COPY_TARGET_DEFAULT_PERMS) != 0;
  attributes = g_file_query_settable_attributes(destination, cancellable, NULL);
  namespaces = g_file_query_writable_namespaces(destination, cancellable, NULL);
  if (attributes == NULL && namespaces == NULL) return TRUE;
  attrs_to_read = build_attribute_list_for_copy(attributes, namespaces, as_move, skip_perms);
  info = g_file_query_info(source, attrs_to_read, source_nofollow_symlinks ? G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS : 0, cancellable, NULL);
  g_free(attrs_to_read);
  res = TRUE;
  if (info) {
      res = g_file_set_attributes_from_info(destination, info, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, cancellable, error);
      g_object_unref(info);
  }
  g_file_attribute_info_list_unref(attributes);
  g_file_attribute_info_list_unref(namespaces);
  return res;
}
static gboolean copy_stream_with_progress(GInputStream *in, GOutputStream *out, GFile *source, GCancellable *cancellable, GFileProgressCallback progress_callback,
                                          gpointer progress_callback_data, GError **error) {
  gssize n_read, n_written;
  goffset current_size;
  char buffer[1024*64], *p;
  gboolean res;
  goffset total_size;
  GFileInfo *info;
  total_size = -1;
  if (progress_callback) {
      info = g_file_input_stream_query_info(G_FILE_INPUT_STREAM(in), G_FILE_ATTRIBUTE_STANDARD_SIZE, cancellable, NULL);
      if (info) {
          if (g_file_info_has_attribute(info, G_FILE_ATTRIBUTE_STANDARD_SIZE)) total_size = g_file_info_get_size(info);
          g_object_unref(info);
      }
      if (total_size == -1) {
          info = g_file_query_info(source, G_FILE_ATTRIBUTE_STANDARD_SIZE, G_FILE_QUERY_INFO_NONE, cancellable, NULL);
          if (info) {
              if (g_file_info_has_attribute(info, G_FILE_ATTRIBUTE_STANDARD_SIZE)) total_size = g_file_info_get_size(info);
              g_object_unref(info);
          }
      }
  }
  if (total_size == -1) total_size = 0;
  current_size = 0;
  res = TRUE;
  while(TRUE) {
      n_read = g_input_stream_read(in, buffer, sizeof(buffer), cancellable, error);
      if (n_read == -1) {
          res = FALSE;
          break;
	  }
      if (n_read == 0) break;
      current_size += n_read;
      p = buffer;
      while(n_read > 0) {
          n_written = g_output_stream_write(out, p, n_read, cancellable, error);
          if (n_written == -1) {
              res = FALSE;
              break;
          }
          p += n_written;
          n_read -= n_written;
	  }
      if (!res) break;
      if (progress_callback) progress_callback(current_size, total_size, progress_callback_data);
  }
  if (progress_callback) progress_callback(current_size, total_size, progress_callback_data);
  return res;
}
#ifndef HAVE_SPLICE
static gboolean do_splice(int fd_in, loff_t *off_in, int fd_out, loff_t *off_out, size_t len, long *bytes_transferd, GError **error) {
  long result;
retry:
  result = splice(fd_in, off_in, fd_out, off_out, len, SPLICE_F_MORE);
  if (result == -1) {
      int errsv = errno;
      if (errsv == EINTR) goto retry;
      else if (errsv == ENOSYS || errsv == EINVAL) g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Splice not supported"));
      else g_set_error (error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error splicing file: %s"), g_strerror(errsv));
      return FALSE;
  }
  *bytes_transferd = result;
  return TRUE;
}
static gboolean splice_stream_with_progress(GInputStream *in, GOutputStream *out, GCancellable *cancellable, GFileProgressCallback progress_callback,
                                            gpointer progress_callback_data, GError **error) {
  int buffer[2];
  gboolean res;
  goffset total_size;
  loff_t offset_in;
  loff_t offset_out;
  int fd_in, fd_out;
  fd_in = g_file_descriptor_based_get_fd(G_FILE_DESCRIPTOR_BASED(in));
  fd_out = g_file_descriptor_based_get_fd(G_FILE_DESCRIPTOR_BASED(out));
  if (pipe (buffer) != 0) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, "Pipe creation failed");
      return FALSE;
  }
  total_size = -1;
  if (progress_callback) {
      struct stat sbuf;
      if (fstat (fd_in, &sbuf) == 0) total_size = sbuf.st_size;
  }
  if (total_size == -1) total_size = 0;
  offset_in = offset_out = 0;
  res = FALSE;
  while(TRUE) {
      long n_read;
      long n_written;
      if (g_cancellable_set_error_if_cancelled(cancellable, error)) break;
      if (!do_splice(fd_in, &offset_in, buffer[1], NULL, 1024*64, &n_read, error)) break;
      if (n_read == 0) {
          res = TRUE;
          break;
      }
      while(n_read > 0) {
          if (g_cancellable_set_error_if_cancelled(cancellable, error)) goto out;
          if (!do_splice(buffer[0], NULL, fd_out, &offset_out, n_read, &n_written, error)) goto out;
          n_read -= n_written;
      }
      if (progress_callback) progress_callback(offset_in, total_size, progress_callback_data);
  }
  if (progress_callback) progress_callback(offset_in, total_size, progress_callback_data);
out:
  close(buffer[0]);
  close(buffer[1]);
  return res;
}
#endif
static gboolean file_copy_fallback(GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable  *cancellable, GFileProgressCallback progress_callback,
                                   gpointer progress_callback_data, GError **error) {
  GInputStream *in;
  GOutputStream *out;
  GFileInfo *info;
  const char *target;
  gboolean result;
#ifndef HAVE_SPLICE
  gboolean fallback = TRUE;
#endif
  info = g_file_query_info(source, G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
			               cancellable, error);
  if (info == NULL) return FALSE;
  if ((flags & G_FILE_COPY_NOFOLLOW_SYMLINKS) && g_file_info_get_file_type (info) == G_FILE_TYPE_SYMBOLIC_LINK) {
      target = g_file_info_get_symlink_target(info);
      if (target) {
          if (!copy_symlink(destination, flags, cancellable, target, error)) {
              g_object_unref(info);
              return FALSE;
          }
          g_object_unref(info);
          goto copied_file;
	  }
  	  g_object_unref(info);
  } else if (g_file_info_get_file_type(info) == G_FILE_TYPE_SPECIAL) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"Can't copy special file");
      g_object_unref(info);
      return FALSE;
  } else g_object_unref(info);
  in = open_source_for_copy(source, destination, flags, cancellable, error);
  if (in == NULL) return FALSE;
  if (flags & G_FILE_COPY_OVERWRITE) {
      out = (GOutputStream*)g_file_replace(destination, NULL, flags & G_FILE_COPY_BACKUP, G_FILE_CREATE_REPLACE_DESTINATION, cancellable, error);
  } else out = (GOutputStream*)g_file_create(destination, 0, cancellable, error);
  if (out == NULL) {
      g_object_unref(in);
      return FALSE;
  }
#ifndef HAVE_SPLICE
  if (G_IS_FILE_DESCRIPTOR_BASED(in) && G_IS_FILE_DESCRIPTOR_BASED(out)) {
      GError *splice_err = NULL;
      result = splice_stream_with_progress(in, out, cancellable, progress_callback, progress_callback_data, &splice_err);
      if (result || !g_error_matches(splice_err, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED)) {
          fallback = FALSE;
          if (!result) g_propagate_error(error, splice_err);
      } else g_clear_error(&splice_err);
  }
  if (fallback)
#endif
  result = copy_stream_with_progress(in, out, source, cancellable, progress_callback, progress_callback_data, error);
  g_input_stream_close(in, cancellable, NULL);
  if (!g_output_stream_close(out, cancellable, result ? error : NULL)) result = FALSE;
  g_object_unref(in);
  g_object_unref(out);
  if (result == FALSE) return FALSE;
copied_file:
  g_file_copy_attributes(source, destination, flags, cancellable, NULL);
  return TRUE;
}
gboolean g_file_copy(GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable, GFileProgressCallback progress_callback,
                     gpointer progress_callback_data, GError **error) {
  GFileIface *iface;
  GError *my_error;
  gboolean res;
  g_return_val_if_fail(G_IS_FILE(source), FALSE);
  g_return_val_if_fail(G_IS_FILE(destination), FALSE);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return FALSE;
  iface = G_FILE_GET_IFACE(destination);
  if (iface->copy) {
      my_error = NULL;
      res = (*iface->copy)(source, destination, flags, cancellable, progress_callback, progress_callback_data, &my_error);
      if (res) return TRUE;
      if (my_error->domain != G_IO_ERROR || my_error->code != G_IO_ERROR_NOT_SUPPORTED) {
          g_propagate_error(error, my_error);
	      return FALSE;
	  } else g_clear_error(&my_error);
  }
  if (G_OBJECT_TYPE(source) != G_OBJECT_TYPE(destination)) {
      iface = G_FILE_GET_IFACE(source);
      if (iface->copy) {
          my_error = NULL;
          res = (*iface->copy)(source, destination, flags, cancellable, progress_callback, progress_callback_data, &my_error);
          if (res) return TRUE;
          if (my_error->domain != G_IO_ERROR || my_error->code != G_IO_ERROR_NOT_SUPPORTED) {
              g_propagate_error(error, my_error);
              return FALSE;
          } else g_clear_error(&my_error);
	  }
  }
  return file_copy_fallback(source, destination, flags, cancellable, progress_callback, progress_callback_data, error);
}
void g_file_copy_async(GFile *source, GFile *destination, GFileCopyFlags flags, int io_priority, GCancellable *cancellable, GFileProgressCallback progress_callback,
                       gpointer progress_callback_data, GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(source));
  g_return_if_fail(G_IS_FILE(destination));
  iface = G_FILE_GET_IFACE(source);
  (*iface->copy_async)(source, destination, flags, io_priority, cancellable, progress_callback, progress_callback_data, callback, user_data);
}
gboolean g_file_copy_finish(GFile *file, GAsyncResult *res, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->copy_finish)(file, res, error);
}
gboolean g_file_move(GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable, GFileProgressCallback progress_callback,
                     gpointer progress_callback_data, GError **error) {
  GFileIface *iface;
  GError *my_error;
  gboolean res;
  g_return_val_if_fail(G_IS_FILE(source), FALSE);
  g_return_val_if_fail(G_IS_FILE(destination), FALSE);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return FALSE;
  iface = G_FILE_GET_IFACE(destination);
  if (iface->move) {
      my_error = NULL;
      res = (*iface->move)(source, destination, flags, cancellable, progress_callback, progress_callback_data, &my_error);
      if (res) return TRUE;
      if (my_error->domain != G_IO_ERROR || my_error->code != G_IO_ERROR_NOT_SUPPORTED) {
          g_propagate_error(error, my_error);
          return FALSE;
	  }
  }
  if (G_OBJECT_TYPE(source) != G_OBJECT_TYPE(destination)) {
      iface = G_FILE_GET_IFACE(source);
      if (iface->move) {
          my_error = NULL;
          res = (*iface->move)(source, destination, flags, cancellable, progress_callback, progress_callback_data, &my_error);
          if (res) return TRUE;
          if (my_error->domain != G_IO_ERROR || my_error->code != G_IO_ERROR_NOT_SUPPORTED) {
              g_propagate_error(error, my_error);
              return FALSE;
          }
	  }
  }
  if (flags & G_FILE_COPY_NO_FALLBACK_FOR_MOVE) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      return FALSE;
  }
  flags |= G_FILE_COPY_ALL_METADATA;
  if (!g_file_copy(source, destination, flags, cancellable, progress_callback, progress_callback_data, error)) return FALSE;
  return g_file_delete(source, cancellable, error);
}
gboolean g_file_make_directory(GFile *file, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return FALSE;
  iface = G_FILE_GET_IFACE(file);
  if (iface->make_directory == NULL) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      return FALSE;
  }
  return (*iface->make_directory)(file, cancellable, error);
}
gboolean g_file_make_directory_with_parents(GFile *file, GCancellable *cancellable, GError **error) {
  gboolean result;
  GFile *parent_file, *work_file;
  GList *list = NULL, *l;
  GError *my_error = NULL;
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return FALSE;
  result = g_file_make_directory(file, cancellable, &my_error);
  if (result || my_error->code != G_IO_ERROR_NOT_FOUND) {
      if (my_error) g_propagate_error(error, my_error);
      return result;
  }
  work_file = file;
  while(!result && my_error->code == G_IO_ERROR_NOT_FOUND) {
      g_clear_error(&my_error);
      parent_file = g_file_get_parent(work_file);
      if (parent_file == NULL) break;
      result = g_file_make_directory(parent_file, cancellable, &my_error);
      if (!result && my_error->code == G_IO_ERROR_NOT_FOUND) list = g_list_prepend(list, parent_file);
      work_file = parent_file;
  }
  for (l = list; result && l; l = l->next) result = g_file_make_directory((GFile *) l->data, cancellable, &my_error);
  while(list != NULL) {
      g_object_unref((GFile *) list->data);
      list = g_list_remove(list, list->data);
  }
  if (!result) {
      g_propagate_error(error, my_error);
      return result;
  }
  return g_file_make_directory(file, cancellable, error);
}
gboolean g_file_make_symbolic_link(GFile *file, const char *symlink_value, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(symlink_value != NULL, FALSE);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return FALSE;
  if (*symlink_value == '\0') {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Invalid symlink value given");
      return FALSE;
  }
  iface = G_FILE_GET_IFACE(file);
  if (iface->make_symbolic_link == NULL) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      return FALSE;
  }
  return (*iface->make_symbolic_link)(file, symlink_value, cancellable, error);
}
gboolean g_file_delete(GFile *file, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return FALSE;
  iface = G_FILE_GET_IFACE(file);
  if (iface->delete_file == NULL) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      return FALSE;
  }
  return (*iface->delete_file)(file, cancellable, error);
}
gboolean g_file_trash(GFile *file, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return FALSE;
  iface = G_FILE_GET_IFACE(file);
  if (iface->trash == NULL) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"Trash not supported");
      return FALSE;
  }
  return (*iface->trash)(file, cancellable, error);
}
GFile *g_file_set_display_name(GFile *file, const char *display_name, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(display_name != NULL, NULL);
  if (strchr(display_name, G_DIR_SEPARATOR) != NULL) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,"File names cannot contain '%c'", G_DIR_SEPARATOR);
      return NULL;
  }
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  return (*iface->set_display_name)(file, display_name, cancellable, error);
}
void g_file_set_display_name_async(GFile *file, const char *display_name, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
			                       gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE (file));
  g_return_if_fail(display_name != NULL);
  iface = G_FILE_GET_IFACE(file);
  (*iface->set_display_name_async)(file, display_name, io_priority, cancellable, callback, user_data);
}
GFile *g_file_set_display_name_finish (GFile *file, GAsyncResult *res, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(res), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(res)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->set_display_name_finish)(file, res, error);
}
GFileAttributeInfoList *g_file_query_settable_attributes(GFile *file, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  GError *my_error;
  GFileAttributeInfoList *list;
  g_return_val_if_fail(G_IS_FILE (file), NULL);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  if (iface->query_settable_attributes == NULL) return g_file_attribute_info_list_new();
  my_error = NULL;
  list = (*iface->query_settable_attributes)(file, cancellable, &my_error);
  if (list == NULL) {
      if (my_error->domain == G_IO_ERROR && my_error->code == G_IO_ERROR_NOT_SUPPORTED) {
          list = g_file_attribute_info_list_new();
          g_error_free(my_error);
	  } else g_propagate_error(error, my_error);
  }
  return list;
}
GFileAttributeInfoList *g_file_query_writable_namespaces(GFile *file, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  GError *my_error;
  GFileAttributeInfoList *list;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  if (iface->query_writable_namespaces == NULL) return g_file_attribute_info_list_new();
  my_error = NULL;
  list = (*iface->query_writable_namespaces)(file, cancellable, &my_error);
  if (list == NULL) {
      if (my_error->domain == G_IO_ERROR && my_error->code == G_IO_ERROR_NOT_SUPPORTED) {
          list = g_file_attribute_info_list_new();
          g_error_free(my_error);
	  } else g_propagate_error(error, my_error);
  }
  return list;
}
gboolean g_file_set_attribute(GFile *file, const char *attribute, GFileAttributeType type, gpointer value_p, GFileQueryInfoFlags flags, GCancellable *cancellable,
                              GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(attribute != NULL && *attribute != '\0', FALSE);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return FALSE;
  iface = G_FILE_GET_IFACE(file);
  if (iface->set_attribute == NULL) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      return FALSE;
  }
  return (*iface->set_attribute)(file, attribute, type, value_p, flags, cancellable, error);
}
gboolean g_file_set_attributes_from_info(GFile *file, GFileInfo *info, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(G_IS_FILE_INFO(info), FALSE);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return FALSE;
  g_file_info_clear_status(info);
  iface = G_FILE_GET_IFACE(file);
  return (*iface->set_attributes_from_info)(file, info, flags, cancellable, error);
}
static gboolean g_file_real_set_attributes_from_info(GFile *file, GFileInfo *info, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error) {
  char **attributes;
  int i;
  gboolean res;
  GFileAttributeValue *value;
  res = TRUE;
  attributes = g_file_info_list_attributes(info, NULL);
  for (i = 0; attributes[i] != NULL; i++) {
      value = _g_file_info_get_attribute_value(info, attributes[i]);
      if (value->status != G_FILE_ATTRIBUTE_STATUS_UNSET) continue;
      if (!g_file_set_attribute(file, attributes[i], value->type, _g_file_attribute_value_peek_as_pointer(value), flags, cancellable, error)) {
          value->status = G_FILE_ATTRIBUTE_STATUS_ERROR_SETTING;
          res = FALSE;
          error = NULL;
	  } else value->status = G_FILE_ATTRIBUTE_STATUS_SET;
  }
  g_strfreev(attributes);
  return res;
}
void g_file_set_attributes_async(GFile *file, GFileInfo *info, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                 gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  g_return_if_fail(G_IS_FILE_INFO(info));
  iface = G_FILE_GET_IFACE(file);
  (*iface->set_attributes_async)(file, info, flags, io_priority, cancellable, callback, user_data);
}
gboolean g_file_set_attributes_finish(GFile *file, GAsyncResult *result, GFileInfo **info, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  iface = G_FILE_GET_IFACE(file);
  return (*iface->set_attributes_finish)(file, result, info, error);
}
gboolean g_file_set_attribute_string(GFile *file, const char *attribute, const char *value, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error) {
  return g_file_set_attribute(file, attribute, G_FILE_ATTRIBUTE_TYPE_STRING, (gpointer)value, flags, cancellable, error);
}
gboolean g_file_set_attribute_byte_string(GFile *file, const char *attribute, const char *value, GFileQueryInfoFlags flags, GCancellable *cancellable,
                                          GError **error) {
  return g_file_set_attribute(file, attribute, G_FILE_ATTRIBUTE_TYPE_BYTE_STRING, (gpointer)value, flags, cancellable, error);
}
gboolean g_file_set_attribute_uint32(GFile *file, const char *attribute, guint32 value, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error) {
  return g_file_set_attribute(file, attribute, G_FILE_ATTRIBUTE_TYPE_UINT32, &value, flags, cancellable, error);
}
gboolean g_file_set_attribute_int32(GFile *file, const char *attribute, gint32 value, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error) {
  return g_file_set_attribute(file, attribute, G_FILE_ATTRIBUTE_TYPE_INT32, &value, flags, cancellable, error);
}
gboolean g_file_set_attribute_uint64(GFile *file, const char *attribute, guint64 value, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error) {
  return g_file_set_attribute(file, attribute, G_FILE_ATTRIBUTE_TYPE_UINT64, &value, flags, cancellable, error);
}
gboolean g_file_set_attribute_int64(GFile *file, const char *attribute, gint64 value, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error) {
  return g_file_set_attribute(file, attribute, G_FILE_ATTRIBUTE_TYPE_INT64, &value, flags, cancellable, error);
}
void g_file_mount_mountable(GFile *file, GMountMountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                            gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  if (iface->mount_mountable == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(file), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      return;
  }
  (*iface->mount_mountable)(file, flags, mount_operation, cancellable, callback, user_data);
}
GFile *g_file_mount_mountable_finish(GFile *file, GAsyncResult *result, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->mount_mountable_finish)(file, result, error);
}
void g_file_unmount_mountable(GFile *file, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  if (iface->unmount_mountable == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(file), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      return;
  }
  (*iface->unmount_mountable)(file, flags, cancellable, callback, user_data);
}
gboolean g_file_unmount_mountable_finish(GFile *file, GAsyncResult *result, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->unmount_mountable_finish)(file, result, error);
}
void g_file_unmount_mountable_with_operation(GFile *file, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable,
                                             GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  if (iface->unmount_mountable == NULL && iface->unmount_mountable_with_operation == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(file), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      return;
  }
  if (iface->unmount_mountable_with_operation != NULL) (*iface->unmount_mountable_with_operation)(file, flags, mount_operation, cancellable, callback, user_data);
  else (*iface->unmount_mountable)(file, flags, cancellable, callback, user_data);
}
gboolean g_file_unmount_mountable_with_operation_finish(GFile *file, GAsyncResult *result, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_FILE_GET_IFACE(file);
  if (iface->unmount_mountable_with_operation_finish != NULL) return (*iface->unmount_mountable_with_operation_finish)(file, result, error);
  else return (*iface->unmount_mountable_finish)(file, result, error);
}
void g_file_eject_mountable(GFile *file, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  if (iface->eject_mountable == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(file), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      return;
  }
  (*iface->eject_mountable)(file, flags, cancellable, callback, user_data);
}
gboolean g_file_eject_mountable_finish(GFile *file, GAsyncResult *result, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->eject_mountable_finish)(file, result, error);
}
void g_file_eject_mountable_with_operation(GFile *file, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable,
                                           GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  if (iface->eject_mountable == NULL && iface->eject_mountable_with_operation == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(file), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      return;
  }
  if (iface->eject_mountable_with_operation != NULL) (*iface->eject_mountable_with_operation)(file, flags, mount_operation, cancellable, callback, user_data);
  else (*iface->eject_mountable)(file, flags, cancellable, callback, user_data);
}
gboolean g_file_eject_mountable_with_operation_finish(GFile *file, GAsyncResult *result, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT (result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_FILE_GET_IFACE(file);
  if (iface->eject_mountable_with_operation_finish != NULL) return (*iface->eject_mountable_with_operation_finish)(file, result, error);
  else return (*iface->eject_mountable_finish)(file, result, error);
}
GFileMonitor* g_file_monitor_directory(GFile *file, GFileMonitorFlags flags, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  if (g_cancellable_set_error_if_cancelled (cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  if (iface->monitor_dir == NULL){
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      return NULL;
  }
  return (*iface->monitor_dir)(file, flags, cancellable, error);
}
GFileMonitor* g_file_monitor_file(GFile *file, GFileMonitorFlags flags, GCancellable *cancellable, GError **error) {
  GFileIface *iface;
  GFileMonitor *monitor;
  g_return_val_if_fail(G_IS_FILE(file), NULL);
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return NULL;
  iface = G_FILE_GET_IFACE(file);
  monitor = NULL;
  if (iface->monitor_file) monitor = (*iface->monitor_file)(file, flags, cancellable, NULL);
  if (monitor == NULL) monitor = _g_poll_file_monitor_new(file);
  return monitor;
}
GFileMonitor* g_file_monitor(GFile *file, GFileMonitorFlags flags, GCancellable *cancellable, GError **error) {
  if (g_file_query_file_type(file, 0, cancellable) == G_FILE_TYPE_DIRECTORY) return g_file_monitor_directory(file, flags, cancellable, error);
  else return g_file_monitor_file(file, flags, cancellable, error);
}
typedef struct {
  char *attributes;
  GFileQueryInfoFlags flags;
  GFileInfo *info;
} QueryInfoAsyncData;
static void query_info_data_free(QueryInfoAsyncData *data) {
  if (data->info) g_object_unref(data->info);
  g_free(data->attributes);
  g_free(data);
}
static void query_info_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GError *error = NULL;
  QueryInfoAsyncData *data;
  GFileInfo *info;
  data = g_simple_async_result_get_op_res_gpointer(res);
  info = g_file_query_info(G_FILE(object), data->attributes, data->flags, cancellable, &error);
  if (info == NULL) g_simple_async_result_take_error(res, error);
  else data->info = info;
}
static void g_file_real_query_info_async(GFile *file, const char *attributes, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable,
                                         GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  QueryInfoAsyncData *data;
  data = g_new0(QueryInfoAsyncData, 1);
  data->attributes = g_strdup(attributes);
  data->flags = flags;
  res = g_simple_async_result_new(G_OBJECT(file), callback, user_data, g_file_real_query_info_async);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)query_info_data_free);
  g_simple_async_result_run_in_thread(res, query_info_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static GFileInfo *g_file_real_query_info_finish(GFile *file, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  QueryInfoAsyncData *data;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_real_query_info_async);
  data = g_simple_async_result_get_op_res_gpointer(simple);
  if (data->info) return g_object_ref(data->info);
  return NULL;
}
typedef struct {
  char *attributes;
  GFileInfo *info;
} QueryFilesystemInfoAsyncData;
static void query_filesystem_info_data_free(QueryFilesystemInfoAsyncData *data) {
  if (data->info) g_object_unref(data->info);
  g_free(data->attributes);
  g_free(data);
}
static void query_filesystem_info_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GError *error = NULL;
  QueryFilesystemInfoAsyncData *data;
  GFileInfo *info;
  data = g_simple_async_result_get_op_res_gpointer(res);
  info = g_file_query_filesystem_info(G_FILE(object), data->attributes, cancellable, &error);
  if (info == NULL) g_simple_async_result_take_error(res, error);
  else data->info = info;
}
static void g_file_real_query_filesystem_info_async(GFile *file, const char *attributes, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                                    gpointer user_data) {
  GSimpleAsyncResult *res;
  QueryFilesystemInfoAsyncData *data;
  data = g_new0(QueryFilesystemInfoAsyncData, 1);
  data->attributes = g_strdup(attributes);
  res = g_simple_async_result_new(G_OBJECT(file), callback, user_data, g_file_real_query_filesystem_info_async);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)query_filesystem_info_data_free);
  g_simple_async_result_run_in_thread(res, query_filesystem_info_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static GFileInfo *g_file_real_query_filesystem_info_finish(GFile *file, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  QueryFilesystemInfoAsyncData *data;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_real_query_filesystem_info_async);
  data = g_simple_async_result_get_op_res_gpointer(simple);
  if (data->info) return g_object_ref(data->info);
  return NULL;
}
typedef struct {
  char *attributes;
  GFileQueryInfoFlags flags;
  GFileEnumerator *enumerator;
} EnumerateChildrenAsyncData;
static void enumerate_children_data_free(EnumerateChildrenAsyncData *data) {
  if (data->enumerator) g_object_unref(data->enumerator);
  g_free(data->attributes);
  g_free(data);
}
static void enumerate_children_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GError *error = NULL;
  EnumerateChildrenAsyncData *data;
  GFileEnumerator *enumerator;
  data = g_simple_async_result_get_op_res_gpointer(res);
  enumerator = g_file_enumerate_children(G_FILE(object), data->attributes, data->flags, cancellable, &error);
  if (enumerator == NULL) g_simple_async_result_take_error(res, error);
  else data->enumerator = enumerator;
}
static void g_file_real_enumerate_children_async(GFile *file, const char *attributes, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable,
                                                 GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  EnumerateChildrenAsyncData *data;
  data = g_new0(EnumerateChildrenAsyncData, 1);
  data->attributes = g_strdup(attributes);
  data->flags = flags;
  res = g_simple_async_result_new(G_OBJECT(file), callback, user_data, g_file_real_enumerate_children_async);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)enumerate_children_data_free);
  g_simple_async_result_run_in_thread(res, enumerate_children_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static GFileEnumerator *g_file_real_enumerate_children_finish (GFile *file, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  EnumerateChildrenAsyncData *data;
  g_simple_async_result_get_source_tag(simple) == g_file_real_enumerate_children_async;
  data = g_simple_async_result_get_op_res_gpointer(simple);
  if (data->enumerator) return g_object_ref(data->enumerator);
  return NULL;
}
static void open_read_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GFileIface *iface;
  GFileInputStream *stream;
  GError *error = NULL;
  iface = G_FILE_GET_IFACE(object);
  if (iface->read_fn == NULL) {
      g_set_error_literal(&error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      g_simple_async_result_take_error(res, error);
      return;
  }
  stream = iface->read_fn(G_FILE(object), cancellable, &error);
  if (stream == NULL) g_simple_async_result_take_error(res, error);
  else g_simple_async_result_set_op_res_gpointer(res, stream, g_object_unref);
}
static void g_file_real_read_async(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  res = g_simple_async_result_new(G_OBJECT(file), callback, user_data, g_file_real_read_async);
  g_simple_async_result_run_in_thread(res, open_read_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static GFileInputStream *g_file_real_read_finish(GFile *file, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  gpointer op;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_real_read_async);
  op = g_simple_async_result_get_op_res_gpointer(simple);
  if (op) return g_object_ref(op);
  return NULL;
}
static void append_to_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GFileIface *iface;
  GFileCreateFlags *data;
  GFileOutputStream *stream;
  GError *error = NULL;
  iface = G_FILE_GET_IFACE(object);
  data = g_simple_async_result_get_op_res_gpointer(res);
  stream = iface->append_to(G_FILE(object), *data, cancellable, &error);
  if (stream == NULL) g_simple_async_result_take_error(res, error);
  else g_simple_async_result_set_op_res_gpointer(res, stream, g_object_unref);
}
static void g_file_real_append_to_async(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                        gpointer user_data) {
  GFileCreateFlags *data;
  GSimpleAsyncResult *res;
  data = g_new0(GFileCreateFlags, 1);
  *data = flags;
  res = g_simple_async_result_new(G_OBJECT(file), callback, user_data, g_file_real_append_to_async);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)g_free);
  g_simple_async_result_run_in_thread(res, append_to_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static GFileOutputStream *g_file_real_append_to_finish(GFile *file, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  gpointer op;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_real_append_to_async);
  op = g_simple_async_result_get_op_res_gpointer(simple);
  if (op) return g_object_ref(op);
  return NULL;
}
static void create_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GFileIface *iface;
  GFileCreateFlags *data;
  GFileOutputStream *stream;
  GError *error = NULL;
  iface = G_FILE_GET_IFACE(object);
  data = g_simple_async_result_get_op_res_gpointer(res);
  stream = iface->create(G_FILE(object), *data, cancellable, &error);
  if (stream == NULL) g_simple_async_result_take_error(res, error);
  else g_simple_async_result_set_op_res_gpointer(res, stream, g_object_unref);
}
static void g_file_real_create_async(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                     gpointer user_data) {
  GFileCreateFlags *data;
  GSimpleAsyncResult *res;
  data = g_new0(GFileCreateFlags, 1);
  *data = flags;
  res = g_simple_async_result_new(G_OBJECT(file), callback, user_data, g_file_real_create_async);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)g_free);
  g_simple_async_result_run_in_thread(res, create_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static GFileOutputStream *g_file_real_create_finish(GFile *file, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  gpointer op;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_real_create_async);
  op = g_simple_async_result_get_op_res_gpointer(simple);
  if (op) return g_object_ref(op);
  return NULL;
}
typedef struct {
  GFileOutputStream *stream;
  char *etag;
  gboolean make_backup;
  GFileCreateFlags flags;
} ReplaceAsyncData;
static void replace_async_data_free(ReplaceAsyncData *data) {
  if (data->stream) g_object_unref(data->stream);
  g_free(data->etag);
  g_free(data);
}
static void replace_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GFileIface *iface;
  GFileOutputStream *stream;
  GError *error = NULL;
  ReplaceAsyncData *data;
  iface = G_FILE_GET_IFACE(object);
  data = g_simple_async_result_get_op_res_gpointer(res);
  stream = iface->replace(G_FILE(object), data->etag, data->make_backup, data->flags, cancellable, &error);
  if (stream == NULL) g_simple_async_result_take_error(res, error);
  else data->stream = stream;
}
static void g_file_real_replace_async(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, int io_priority, GCancellable *cancellable,
                                      GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  ReplaceAsyncData *data;
  data = g_new0(ReplaceAsyncData, 1);
  data->etag = g_strdup(etag);
  data->make_backup = make_backup;
  data->flags = flags;
  res = g_simple_async_result_new(G_OBJECT(file), callback, user_data, g_file_real_replace_async);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)replace_async_data_free);
  g_simple_async_result_run_in_thread(res, replace_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static GFileOutputStream *g_file_real_replace_finish(GFile *file, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  ReplaceAsyncData *data;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_real_replace_async);
  data = g_simple_async_result_get_op_res_gpointer(simple);
  if (data->stream) return g_object_ref(data->stream);
  return NULL;
}
static void open_readwrite_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GFileIface *iface;
  GFileIOStream *stream;
  GError *error = NULL;
  iface = G_FILE_GET_IFACE(object);
  if (iface->open_readwrite == NULL) {
      g_set_error_literal(&error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      g_simple_async_result_take_error(res, error);
      return;
  }
  stream = iface->open_readwrite(G_FILE(object), cancellable, &error);
  if (stream == NULL) g_simple_async_result_take_error(res, error);
  else g_simple_async_result_set_op_res_gpointer(res, stream, g_object_unref);
}
static void g_file_real_open_readwrite_async(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  res = g_simple_async_result_new(G_OBJECT(file), callback, user_data, g_file_real_open_readwrite_async);
  g_simple_async_result_run_in_thread(res, open_readwrite_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static GFileIOStream *g_file_real_open_readwrite_finish(GFile *file, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  gpointer op;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_real_open_readwrite_async);
  op = g_simple_async_result_get_op_res_gpointer(simple);
  if (op) return g_object_ref(op);
  return NULL;
}
static void create_readwrite_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GFileIface *iface;
  GFileCreateFlags *data;
  GFileIOStream *stream;
  GError *error = NULL;
  iface = G_FILE_GET_IFACE(object);
  data = g_simple_async_result_get_op_res_gpointer(res);
  if (iface->create_readwrite == NULL) {
      g_set_error_literal(&error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      g_simple_async_result_take_error(res, error);
      return;
  }
  stream = iface->create_readwrite(G_FILE(object), *data, cancellable, &error);
  if (stream == NULL) g_simple_async_result_take_error(res, error);
  else g_simple_async_result_set_op_res_gpointer(res, stream, g_object_unref);
}
static void g_file_real_create_readwrite_async(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                               gpointer user_data) {
  GFileCreateFlags *data;
  GSimpleAsyncResult *res;
  data = g_new0(GFileCreateFlags, 1);
  *data = flags;
  res = g_simple_async_result_new(G_OBJECT(file), callback, user_data, g_file_real_create_readwrite_async);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)g_free);
  g_simple_async_result_run_in_thread(res, create_readwrite_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static GFileIOStream *g_file_real_create_readwrite_finish(GFile *file, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  gpointer op;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_real_create_readwrite_async);
  op = g_simple_async_result_get_op_res_gpointer(simple);
  if (op) return g_object_ref(op);
  return NULL;
}
typedef struct {
  GFileIOStream *stream;
  char *etag;
  gboolean make_backup;
  GFileCreateFlags flags;
} ReplaceRWAsyncData;
static void replace_rw_async_data_free(ReplaceRWAsyncData *data) {
  if (data->stream) g_object_unref(data->stream);
  g_free(data->etag);
  g_free(data);
}
static void replace_readwrite_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GFileIface *iface;
  GFileIOStream *stream;
  GError *error = NULL;
  ReplaceRWAsyncData *data;
  iface = G_FILE_GET_IFACE(object);
  data = g_simple_async_result_get_op_res_gpointer(res);
  stream = iface->replace_readwrite(G_FILE(object), data->etag, data->make_backup, data->flags, cancellable, &error);
  if (stream == NULL) g_simple_async_result_take_error(res, error);
  else data->stream = stream;
}
static void g_file_real_replace_readwrite_async(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, int io_priority,
                                                GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  ReplaceRWAsyncData *data;
  data = g_new0(ReplaceRWAsyncData, 1);
  data->etag = g_strdup(etag);
  data->make_backup = make_backup;
  data->flags = flags;
  res = g_simple_async_result_new(G_OBJECT(file), callback, user_data, g_file_real_replace_readwrite_async);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)replace_rw_async_data_free);
  g_simple_async_result_run_in_thread(res, replace_readwrite_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static GFileIOStream *g_file_real_replace_readwrite_finish(GFile *file, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  ReplaceRWAsyncData *data;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_real_replace_readwrite_async);
  data = g_simple_async_result_get_op_res_gpointer(simple);
  if (data->stream) return g_object_ref(data->stream);
  return NULL;
}
typedef struct {
  char *name;
  GFile *file;
} SetDisplayNameAsyncData;
static void set_display_name_data_free(SetDisplayNameAsyncData *data) {
  g_free (data->name);
  if (data->file) g_object_unref(data->file);
  g_free(data);
}
static void set_display_name_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GError *error = NULL;
  SetDisplayNameAsyncData *data;
  GFile *file;
  data = g_simple_async_result_get_op_res_gpointer(res);
  file = g_file_set_display_name(G_FILE(object), data->name, cancellable, &error);
  if (file == NULL) g_simple_async_result_take_error(res, error);
  else data->file = file;
}
static void g_file_real_set_display_name_async(GFile *file, const char *display_name, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                               gpointer user_data) {
  GSimpleAsyncResult *res;
  SetDisplayNameAsyncData *data;
  data = g_new0(SetDisplayNameAsyncData, 1);
  data->name = g_strdup(display_name);
  res = g_simple_async_result_new(G_OBJECT(file), callback, user_data, g_file_real_set_display_name_async);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)set_display_name_data_free);
  g_simple_async_result_run_in_thread(res, set_display_name_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static GFile *g_file_real_set_display_name_finish(GFile *file, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  SetDisplayNameAsyncData *data;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_real_set_display_name_async);
  data = g_simple_async_result_get_op_res_gpointer(simple);
  if (data->file) return g_object_ref(data->file);
  return NULL;
}
typedef struct {
  GFileQueryInfoFlags flags;
  GFileInfo *info;
  gboolean res;
  GError *error;
} SetInfoAsyncData;
static void set_info_data_free(SetInfoAsyncData *data) {
  if (data->info) g_object_unref(data->info);
  if (data->error) g_error_free(data->error);
  g_free (data);
}
static void set_info_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  SetInfoAsyncData *data;
  data = g_simple_async_result_get_op_res_gpointer(res);
  data->error = NULL;
  data->res = g_file_set_attributes_from_info(G_FILE(object), data->info, data->flags, cancellable, &data->error);
}
static void g_file_real_set_attributes_async(GFile *file, GFileInfo *info, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable,
                                             GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  SetInfoAsyncData *data;
  data = g_new0(SetInfoAsyncData, 1);
  data->info = g_file_info_dup(info);
  data->flags = flags;
  res = g_simple_async_result_new(G_OBJECT(file), callback, user_data, g_file_real_set_attributes_async);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)set_info_data_free);
  g_simple_async_result_run_in_thread(res, set_info_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static gboolean g_file_real_set_attributes_finish(GFile *file, GAsyncResult *res, GFileInfo **info, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  SetInfoAsyncData *data;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_real_set_attributes_async);
  data = g_simple_async_result_get_op_res_gpointer(simple);
  if (info) *info = g_object_ref(data->info);
  if (error != NULL && data->error) *error = g_error_copy(data->error);
  return data->res;
}
static void find_enclosing_mount_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GError *error = NULL;
  GMount *mount;
  mount = g_file_find_enclosing_mount(G_FILE(object), cancellable, &error);
  if (mount == NULL) g_simple_async_result_take_error(res, error);
  else g_simple_async_result_set_op_res_gpointer(res, mount, (GDestroyNotify)g_object_unref);
}
static void g_file_real_find_enclosing_mount_async(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  res = g_simple_async_result_new(G_OBJECT(file), callback, user_data, g_file_real_find_enclosing_mount_async);
  g_simple_async_result_run_in_thread(res, find_enclosing_mount_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static GMount *g_file_real_find_enclosing_mount_finish(GFile *file, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  GMount *mount;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_real_find_enclosing_mount_async);
  mount = g_simple_async_result_get_op_res_gpointer(simple);
  return g_object_ref(mount);
}
typedef struct {
  GFile *source;
  GFile *destination;
  GFileCopyFlags flags;
  GFileProgressCallback progress_cb;
  gpointer progress_cb_data;
  GIOSchedulerJob *job;
} CopyAsyncData;
static void copy_async_data_free(CopyAsyncData *data) {
  g_object_unref(data->source);
  g_object_unref(data->destination);
  g_free(data);
}
typedef struct {
  CopyAsyncData *data;
  goffset current_num_bytes;
  goffset total_num_bytes;
} ProgressData;
static gboolean copy_async_progress_in_main(gpointer user_data) {
  ProgressData *progress = user_data;
  CopyAsyncData *data = progress->data;
  data->progress_cb(progress->current_num_bytes, progress->total_num_bytes, data->progress_cb_data);
  return FALSE;
}
static gboolean mainloop_barrier(gpointer user_data) {
  return FALSE;
}
static void copy_async_progress_callback(goffset current_num_bytes, goffset total_num_bytes, gpointer user_data) {
  CopyAsyncData *data = user_data;
  ProgressData *progress;
  progress = g_new(ProgressData, 1);
  progress->data = data;
  progress->current_num_bytes = current_num_bytes;
  progress->total_num_bytes = total_num_bytes;
  g_io_scheduler_job_send_to_mainloop_async(data->job, copy_async_progress_in_main, progress, g_free);
}
static gboolean copy_async_thread(GIOSchedulerJob *job, GCancellable *cancellable,gpointer user_data) {
  GSimpleAsyncResult *res;
  CopyAsyncData *data;
  gboolean result;
  GError *error;
  res = user_data;
  data = g_simple_async_result_get_op_res_gpointer(res);
  error = NULL;
  data->job = job;
  result = g_file_copy(data->source, data->destination, data->flags, cancellable, (data->progress_cb != NULL) ? copy_async_progress_callback : NULL, data,
                       &error);
  if (data->progress_cb != NULL) g_io_scheduler_job_send_to_mainloop(job, mainloop_barrier, NULL, NULL);
  if (!result) g_simple_async_result_take_error(res, error);
  g_simple_async_result_complete_in_idle(res);
  return FALSE;
}
static void g_file_real_copy_async(GFile *source, GFile *destination, GFileCopyFlags flags, int io_priority, GCancellable *cancellable,
                                   GFileProgressCallback progress_callback, gpointer progress_callback_data, GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  CopyAsyncData *data;
  data = g_new0(CopyAsyncData, 1);
  data->source = g_object_ref(source);
  data->destination = g_object_ref(destination);
  data->flags = flags;
  data->progress_cb = progress_callback;
  data->progress_cb_data = progress_callback_data;
  res = g_simple_async_result_new(G_OBJECT(source), callback, user_data, g_file_real_copy_async);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)copy_async_data_free);
  g_io_scheduler_push_job(copy_async_thread, res, g_object_unref, io_priority, cancellable);
}
static gboolean g_file_real_copy_finish(GFile *file, GAsyncResult *res, GError **error) {
  return TRUE;
}
GFile *g_file_new_for_path(const char *path) {
  g_return_val_if_fail(path != NULL, NULL);
  return g_vfs_get_file_for_path(g_vfs_get_default(), path);
}
GFile *g_file_new_for_uri(const char *uri) {
  g_return_val_if_fail(uri != NULL, NULL);
  return g_vfs_get_file_for_uri(g_vfs_get_default(), uri);
}
GFile *g_file_parse_name(const char *parse_name) {
  g_return_val_if_fail(parse_name != NULL, NULL);
  return g_vfs_parse_name(g_vfs_get_default(), parse_name);
}
static gboolean is_valid_scheme_character(char c) {
  return g_ascii_isalnum(c) || c == '+' || c == '-' || c == '.';
}
static gboolean has_valid_scheme(const char *uri) {
  const char *p;
  p = uri;
  if (!g_ascii_isalpha(*p)) return FALSE;
  do {
      p++;
  } while(is_valid_scheme_character(*p));
  return *p == ':';
}
GFile *g_file_new_for_commandline_arg(const char *arg) {
  GFile *file;
  char *filename;
  char *current_dir;
  g_return_val_if_fail(arg != NULL, NULL);
  if (g_path_is_absolute(arg)) return g_file_new_for_path(arg);
  if (has_valid_scheme(arg)) return g_file_new_for_uri(arg);
  current_dir = g_get_current_dir();
  filename = g_build_filename(current_dir, arg, NULL);
  g_free(current_dir);
  file = g_file_new_for_path(filename);
  g_free(filename);
  return file;
}
void g_file_mount_enclosing_volume(GFile *location, GMountMountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable,
                                   GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(location));
  iface = G_FILE_GET_IFACE(location);
  if (iface->mount_enclosing_volume == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(location), callback, user_data, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,"volume doesn't implement mount");
      return;
  }
  (*iface->mount_enclosing_volume)(location, flags, mount_operation, cancellable, callback, user_data);
}
gboolean g_file_mount_enclosing_volume_finish(GFile *location, GAsyncResult *result, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(location), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_FILE_GET_IFACE(location);
  return (*iface->mount_enclosing_volume_finish)(location, result, error);
}
GAppInfo *g_file_query_default_handler(GFile *file, GCancellable *cancellable, GError **error) {
  char *uri_scheme;
  const char *content_type;
  GAppInfo *appinfo;
  GFileInfo *info;
  char *path;
  uri_scheme = g_file_get_uri_scheme(file);
  if (uri_scheme && uri_scheme[0] != '\0') {
      appinfo = g_app_info_get_default_for_uri_scheme(uri_scheme);
      g_free(uri_scheme);
      if (appinfo != NULL) return appinfo;
  }
  info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE, 0, cancellable, error);
  if (info == NULL) return NULL;
  appinfo = NULL;
  content_type = g_file_info_get_content_type(info);
  if (content_type) {
      path = g_file_get_path(file);
      appinfo = g_app_info_get_default_for_type(content_type,path == NULL);
      g_free(path);
  }
  g_object_unref(info);
  if (appinfo != NULL) return appinfo;
  g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED,"No application is registered as handling this file");
  return NULL;
}
#define GET_CONTENT_BLOCK_SIZE 8192
gboolean g_file_load_contents(GFile *file, GCancellable *cancellable, char **contents, gsize *length, char **etag_out, GError **error) {
  GFileInputStream *in;
  GByteArray *content;
  gsize pos;
  gssize res;
  GFileInfo *info;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(contents != NULL, FALSE);
  in = g_file_read(file, cancellable, error);
  if (in == NULL) return FALSE;
  content = g_byte_array_new();
  pos = 0;
  g_byte_array_set_size(content, pos + GET_CONTENT_BLOCK_SIZE + 1);
  while((res = g_input_stream_read(G_INPUT_STREAM(in),content->data + pos, GET_CONTENT_BLOCK_SIZE, cancellable, error)) > 0) {
      pos += res;
      g_byte_array_set_size(content, pos + GET_CONTENT_BLOCK_SIZE + 1);
  }
  if (etag_out) {
      *etag_out = NULL;
      info = g_file_input_stream_query_info(in, G_FILE_ATTRIBUTE_ETAG_VALUE, cancellable,NULL);
      if (info) {
	  *etag_out = g_strdup(g_file_info_get_etag(info));
	  g_object_unref(info);
	  }
  }
  g_input_stream_close(G_INPUT_STREAM(in), cancellable, NULL);
  g_object_unref(in);
  if (res < 0) {
      g_byte_array_free(content, TRUE);
      return FALSE;
  }
  if (length) *length = pos;
  content->data[pos] = 0;
  *contents = (char*)g_byte_array_free(content,FALSE);
  return TRUE;
}
typedef struct {
  GFile *file;
  GError *error;
  GCancellable *cancellable;
  GFileReadMoreCallback read_more_callback;
  GAsyncReadyCallback callback;
  gpointer user_data;
  GByteArray *content;
  gsize pos;
  char *etag;
} LoadContentsData;
static void load_contents_data_free(LoadContentsData *data) {
  if (data->error) g_error_free(data->error);
  if (data->cancellable) g_object_unref(data->cancellable);
  if (data->content) g_byte_array_free(data->content, TRUE);
  g_free(data->etag);
  g_object_unref(data->file);
  g_free(data);
}
static void load_contents_close_callback(GObject *obj, GAsyncResult *close_res, gpointer user_data) {
  GInputStream *stream = G_INPUT_STREAM(obj);
  LoadContentsData *data = user_data;
  GSimpleAsyncResult *res;
  g_input_stream_close_finish(stream, close_res, NULL);
  g_object_unref(stream);
  res = g_simple_async_result_new(G_OBJECT(data->file), data->callback, data->user_data, g_file_load_contents_async);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)load_contents_data_free);
  g_simple_async_result_complete(res);
  g_object_unref(res);
}
static void load_contents_fstat_callback(GObject *obj, GAsyncResult *stat_res, gpointer user_data) {
  GInputStream *stream = G_INPUT_STREAM(obj);
  LoadContentsData *data = user_data;
  GFileInfo *info;
  info = g_file_input_stream_query_info_finish(G_FILE_INPUT_STREAM(stream), stat_res, NULL);
  if (info) {
      data->etag = g_strdup(g_file_info_get_etag(info));
      g_object_unref(info);
  }
  g_input_stream_close_async(stream, 0, data->cancellable, load_contents_close_callback, data);
}
static void load_contents_read_callback(GObject *obj, GAsyncResult *read_res, gpointer user_data) {
  GInputStream *stream = G_INPUT_STREAM(obj);
  LoadContentsData *data = user_data;
  GError *error = NULL;
  gssize read_size;
  read_size = g_input_stream_read_finish(stream, read_res, &error);
  if (read_size < 0) {
      data->error = error;
      g_input_stream_close_async(stream, 0, data->cancellable, load_contents_close_callback, data);
  } else if (read_size == 0) {
      g_file_input_stream_query_info_async(G_FILE_INPUT_STREAM(stream), G_FILE_ATTRIBUTE_ETAG_VALUE, 0, data->cancellable, load_contents_fstat_callback, data);
  } else if (read_size > 0) {
      data->pos += read_size;
      g_byte_array_set_size(data->content, data->pos + GET_CONTENT_BLOCK_SIZE);
      if (data->read_more_callback && !data->read_more_callback((char*)data->content->data, data->pos, data->user_data)) {
          g_file_input_stream_query_info_async(G_FILE_INPUT_STREAM(stream), G_FILE_ATTRIBUTE_ETAG_VALUE, 0, data->cancellable, load_contents_fstat_callback, data);
      } else g_input_stream_read_async(stream, data->content->data + data->pos, GET_CONTENT_BLOCK_SIZE, 0, data->cancellable, load_contents_read_callback, data);
  }
}
static void load_contents_open_callback(GObject *obj, GAsyncResult *open_res, gpointer user_data) {
  GFile *file = G_FILE (obj);
  GFileInputStream *stream;
  LoadContentsData *data = user_data;
  GError *error = NULL;
  GSimpleAsyncResult *res;
  stream = g_file_read_finish(file, open_res, &error);
  if (stream) {
      g_byte_array_set_size(data->content, data->pos + GET_CONTENT_BLOCK_SIZE);
      g_input_stream_read_async(G_INPUT_STREAM(stream), data->content->data + data->pos, GET_CONTENT_BLOCK_SIZE, 0, data->cancellable, load_contents_read_callback,
                                data);
  } else {
      res = g_simple_async_result_new_take_error(G_OBJECT(data->file), data->callback, data->user_data, error);
      g_simple_async_result_complete(res);
      load_contents_data_free(data);
      g_object_unref(res);
  }
}
void g_file_load_partial_contents_async(GFile *file, GCancellable *cancellable, GFileReadMoreCallback read_more_callback, GAsyncReadyCallback callback,
                                        gpointer user_data) {
  LoadContentsData *data;
  g_return_if_fail(G_IS_FILE(file));
  data = g_new0(LoadContentsData, 1);
  if (cancellable) data->cancellable = g_object_ref(cancellable);
  data->read_more_callback = read_more_callback;
  data->callback = callback;
  data->user_data = user_data;
  data->content = g_byte_array_new();
  data->file = g_object_ref(file);
  g_file_read_async(file, 0, cancellable, load_contents_open_callback, data);
}
gboolean g_file_load_partial_contents_finish(GFile *file, GAsyncResult *res, char **contents, gsize *length, char **etag_out, GError **error) {
  GSimpleAsyncResult *simple;
  LoadContentsData *data;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(G_IS_SIMPLE_ASYNC_RESULT(res), FALSE);
  g_return_val_if_fail(contents != NULL, FALSE);
  simple = G_SIMPLE_ASYNC_RESULT(res);
  if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_load_contents_async);
  data = g_simple_async_result_get_op_res_gpointer(simple);
  if (data->error) {
      g_propagate_error(error, data->error);
      data->error = NULL;
      *contents = NULL;
      if (length) *length = 0;
      return FALSE;
  }
  if (length) *length = data->pos;
  if (etag_out) {
      *etag_out = data->etag;
      data->etag = NULL;
  }
  g_byte_array_set_size(data->content, data->pos + 1);
  data->content->data[data->pos] = 0;
  *contents = (char*)g_byte_array_free(data->content, FALSE);
  data->content = NULL;
  return TRUE;
}
void g_file_load_contents_async(GFile *file, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  g_file_load_partial_contents_async(file, cancellable, NULL, callback, user_data);
}
gboolean g_file_load_contents_finish(GFile *file, GAsyncResult *res, char **contents, gsize *length, char **etag_out, GError **error) {
  return g_file_load_partial_contents_finish(file, res, contents, length, etag_out, error);
}
gboolean g_file_replace_contents(GFile *file, const char *contents, gsize length, const char *etag, gboolean make_backup, GFileCreateFlags flags, char **new_etag,
                                 GCancellable *cancellable, GError **error) {
  GFileOutputStream *out;
  gsize pos, remainder;
  gssize res;
  gboolean ret;
  g_return_val_if_fail(G_IS_FILE (file), FALSE);
  g_return_val_if_fail(contents != NULL, FALSE);
  out = g_file_replace(file, etag, make_backup, flags, cancellable, error);
  if (out == NULL) return FALSE;
  pos = 0;
  remainder = length;
  while(remainder > 0 && (res = g_output_stream_write(G_OUTPUT_STREAM(out),contents + pos,MIN(remainder, GET_CONTENT_BLOCK_SIZE), cancellable,
		error)) > 0) {
      pos += res;
      remainder -= res;
  }
  if (remainder > 0 && res < 0) {
      g_output_stream_close(G_OUTPUT_STREAM(out), cancellable, NULL);
      g_object_unref(out);
      return FALSE;
  }
  ret = g_output_stream_close(G_OUTPUT_STREAM(out), cancellable, error);
  if (new_etag) *new_etag = g_file_output_stream_get_etag(out);
  g_object_unref(out);
  return ret;
}
typedef struct {
  GFile *file;
  GError *error;
  GCancellable *cancellable;
  GAsyncReadyCallback callback;
  gpointer user_data;
  const char *content;
  gsize length;
  gsize pos;
  char *etag;
} ReplaceContentsData;
static void replace_contents_data_free(ReplaceContentsData *data) {
  if (data->error) g_error_free(data->error);
  if (data->cancellable) g_object_unref(data->cancellable);
  g_object_unref(data->file);
  g_free(data->etag);
  g_free(data);
}
static void replace_contents_close_callback (GObject *obj, GAsyncResult *close_res, gpointer user_data) {
  GOutputStream *stream = G_OUTPUT_STREAM(obj);
  ReplaceContentsData *data = user_data;
  GSimpleAsyncResult *res;
  g_output_stream_close_finish(stream, close_res, NULL);
  g_object_unref(stream);
  data->etag = g_file_output_stream_get_etag(G_FILE_OUTPUT_STREAM(stream));
  res = g_simple_async_result_new(G_OBJECT(data->file), data->callback, data->user_data, g_file_replace_contents_async);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)replace_contents_data_free);
  g_simple_async_result_complete(res);
  g_object_unref(res);
}
static void replace_contents_write_callback (GObject *obj, GAsyncResult *read_res, gpointer user_data) {
  GOutputStream *stream = G_OUTPUT_STREAM(obj);
  ReplaceContentsData *data = user_data;
  GError *error = NULL;
  gssize write_size;
  write_size = g_output_stream_write_finish(stream, read_res, &error);
  if (write_size <= 0) {
      if (write_size < 0) data->error = error;
      g_output_stream_close_async(stream, 0, data->cancellable, replace_contents_close_callback, data);
  } else if (write_size > 0) {
      data->pos += write_size;
      if (data->pos >= data->length) g_output_stream_close_async(stream, 0, data->cancellable, replace_contents_close_callback, data);
      else {
          g_output_stream_write_async(stream,data->content + data->pos, data->length - data->pos,0, data->cancellable,
                                      replace_contents_write_callback, data);
      }
  }
}
static void replace_contents_open_callback(GObject *obj, GAsyncResult *open_res, gpointer user_data) {
  GFile *file = G_FILE (obj);
  GFileOutputStream *stream;
  ReplaceContentsData *data = user_data;
  GError *error = NULL;
  GSimpleAsyncResult *res;
  stream = g_file_replace_finish(file, open_res, &error);
  if (stream) {
      g_output_stream_write_async(G_OUTPUT_STREAM(stream),data->content + data->pos,data->length - data->pos,0, data->cancellable,
                                  replace_contents_write_callback, data);
  } else {
      res = g_simple_async_result_new_take_error(G_OBJECT(data->file), data->callback, data->user_data, error);
      g_simple_async_result_complete(res);
      replace_contents_data_free(data);
      g_object_unref(res);
  }
}
void g_file_replace_contents_async(GFile *file, const char *contents, gsize length, const char *etag, gboolean make_backup, GFileCreateFlags flags,
                                   GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  ReplaceContentsData *data;
  g_return_if_fail(G_IS_FILE(file));
  g_return_if_fail(contents != NULL);
  data = g_new0(ReplaceContentsData, 1);
  if (cancellable) data->cancellable = g_object_ref(cancellable);
  data->callback = callback;
  data->user_data = user_data;
  data->content = contents;
  data->length = length;
  data->pos = 0;
  data->file = g_object_ref(file);
  g_file_replace_async(file, etag, make_backup, flags,0, cancellable, replace_contents_open_callback, data);
}
gboolean g_file_replace_contents_finish(GFile *file, GAsyncResult *res, char **new_etag, GError **error) {
  GSimpleAsyncResult *simple;
  ReplaceContentsData *data;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(G_IS_SIMPLE_ASYNC_RESULT(res), FALSE);
  simple = G_SIMPLE_ASYNC_RESULT(res);
  if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_replace_contents_async);
  data = g_simple_async_result_get_op_res_gpointer(simple);
  if (data->error) {
      g_propagate_error(error, data->error);
      data->error = NULL;
      return FALSE;
  }
  if (new_etag) {
      *new_etag = data->etag;
      data->etag = NULL;
  }
  return TRUE;
}
void g_file_start_mountable(GFile *file, GDriveStartFlags flags, GMountOperation *start_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                            gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  if (iface->start_mountable == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(file), callback, user_data, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      return;
  }
  (*iface->start_mountable)(file, flags, start_operation, cancellable, callback, user_data);
}
gboolean g_file_start_mountable_finish(GFile *file, GAsyncResult *result, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE (file), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT (result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_FILE_GET_IFACE(file);
  return (* iface->start_mountable_finish) (file, result, error);
}
void g_file_stop_mountable(GFile *file, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                           gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  if (iface->stop_mountable == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(file), callback, user_data, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      return;
  }
  (*iface->stop_mountable)(file, flags, mount_operation, cancellable, callback, user_data);
}
gboolean g_file_stop_mountable_finish(GFile *file, GAsyncResult *result, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE(file), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT (result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error (simple, error)) return FALSE;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->stop_mountable_finish)(file, result, error);
}
void g_file_poll_mountable(GFile *file, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GFileIface *iface;
  g_return_if_fail(G_IS_FILE(file));
  iface = G_FILE_GET_IFACE(file);
  if (iface->poll_mountable == NULL) {
      g_simple_async_report_error_in_idle(G_OBJECT(file), callback, user_data, G_IO_ERROR,G_IO_ERROR_NOT_SUPPORTED,"Operation not supported");
      return;
  }
  (*iface->poll_mountable)(file, cancellable, callback, user_data);
}
gboolean g_file_poll_mountable_finish (GFile *file, GAsyncResult *result, GError **error) {
  GFileIface *iface;
  g_return_val_if_fail(G_IS_FILE (file), FALSE);
  g_return_val_if_fail(G_IS_ASYNC_RESULT (result), FALSE);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return FALSE;
  }
  iface = G_FILE_GET_IFACE(file);
  return (*iface->poll_mountable_finish)(file, result, error);
}
gboolean g_file_supports_thread_contexts(GFile *file) {
 GFileIface *iface;
 g_return_val_if_fail(G_IS_FILE(file), FALSE);
 iface = G_FILE_GET_IFACE(file);
 return iface->supports_thread_contexts;
}