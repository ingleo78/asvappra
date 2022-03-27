#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_FILE_H__
#define __G_FILE_H__

#include "../gobject/gtype.h"
#include "giotypes.h"
#include "gioenums.h"

G_BEGIN_DECLS
#define G_TYPE_FILE  (g_file_get_type())
#define G_FILE(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_FILE, GFile))
#define G_IS_FILE(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_FILE))
#define G_FILE_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_FILE, GFileIface))
#if 0
typedef struct _GFile GFile;
#endif
typedef struct _GFileIface GFileIface;
struct _GFileIface {
  GTypeInterface g_iface;
  GFile *(*dup)(GFile *file);
  guint (*hash)(GFile *file);
  gboolean (*equal)(GFile *file1, GFile *file2);
  gboolean (*is_native)(GFile *file);
  gboolean (*has_uri_scheme)(GFile *file, const char *uri_scheme);
  char *(*get_uri_scheme)(GFile *file);
  char *(*get_basename)(GFile *file);
  char *(*get_path)(GFile *file);
  char *(*get_uri)(GFile *file);
  char *(*get_parse_name)(GFile *file);
  GFile *(*get_parent)(GFile *file);
  gboolean (*prefix_matches)(GFile *prefix, GFile *file);
  char *(*get_relative_path)(GFile *parent, GFile *descendant);
  GFile *(*resolve_relative_path)(GFile *file, const char *relative_path);
  GFile *(*get_child_for_display_name)(GFile *file, const char *display_name, GError **error);
  GFileEnumerator *(*enumerate_children)(GFile *file, const char *attributes, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
  void (*enumerate_children_async)(GFile *file, const char *attributes, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable,
                                   GAsyncReadyCallback callback, gpointer user_data);
  GFileEnumerator *(*enumerate_children_finish)(GFile *file, GAsyncResult *res, GError **error);
  GFileInfo *(*query_info)(GFile *file, const char *attributes, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
  void (*query_info_async)(GFile *file, const char *attributes, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                           gpointer user_data);
  GFileInfo *(*query_info_finish)(GFile *file, GAsyncResult *res, GError **error);
  GFileInfo *(*query_filesystem_info)(GFile *file, const char *attributes, GCancellable *cancellable, GError **error);
  void (*query_filesystem_info_async)(GFile *file, const char *attributes, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  GFileInfo *(*query_filesystem_info_finish)(GFile *file, GAsyncResult *res, GError **error);
  GMount *(*find_enclosing_mount)(GFile *file, GCancellable *cancellable, GError **error);
  void (*find_enclosing_mount_async)(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  GMount *(*find_enclosing_mount_finish)(GFile *file, GAsyncResult *res, GError **error);
  GFile *(*set_display_name)(GFile *file, const char *display_name, GCancellable *cancellable, GError **error);
  void (*set_display_name_async)(GFile *file, const char *display_name, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  GFile *(*set_display_name_finish)(GFile *file, GAsyncResult *res, GError **error);
  GFileAttributeInfoList *(*query_settable_attributes)(GFile *file, GCancellable *cancellable, GError **error);
  void (*_query_settable_attributes_async)(void);
  void (*_query_settable_attributes_finish)(void);
  GFileAttributeInfoList *(*query_writable_namespaces)(GFile *file, GCancellable *cancellable, GError **error);
  void (*_query_writable_namespaces_async)(void);
  void (*_query_writable_namespaces_finish)(void);
  gboolean (*set_attribute)(GFile *file, const char *attribute, GFileAttributeType type, gpointer value_p, GFileQueryInfoFlags flags, GCancellable *cancellable,
                            GError **error);
  gboolean (*set_attributes_from_info)(GFile *file, GFileInfo *info, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
  void (*set_attributes_async)(GFile *file, GFileInfo *info, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                               gpointer user_data);
  gboolean (*set_attributes_finish)(GFile *file, GAsyncResult *result, GFileInfo **info, GError **error);
  GFileInputStream *(*read_fn)(GFile *file, GCancellable *cancellable, GError **error);
  void (*read_async)(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  GFileInputStream *(*read_finish)(GFile *file, GAsyncResult *res, GError **error);
  GFileOutputStream *(*append_to)(GFile *file, GFileCreateFlags flags, GCancellable *cancellable, GError **error);
  void (*append_to_async)(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  GFileOutputStream *(*append_to_finish)(GFile *file, GAsyncResult *res, GError **error);
  GFileOutputStream *(*create)(GFile *file, GFileCreateFlags flags, GCancellable *cancellable, GError **error);
  void (*create_async)(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  GFileOutputStream *(*create_finish)(GFile *file, GAsyncResult *res, GError **error);
  GFileOutputStream *(*replace)(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, GCancellable *cancellable, GError **error);
  void (*replace_async)(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, int io_priority, GCancellable *cancellable,
                        GAsyncReadyCallback callback, gpointer user_data);
  GFileOutputStream *(*replace_finish)(GFile *file, GAsyncResult *res, GError **error);
  gboolean (*delete_file)(GFile *file, GCancellable *cancellable, GError **error);
  void (*_delete_file_async)(void);
  void (*_delete_file_finish)(void);
  gboolean (*trash)(GFile *file, GCancellable *cancellable, GError **error);
  void (*_trash_async)(void);
  void (*_trash_finish)(void);
  gboolean (*make_directory)(GFile *file, GCancellable *cancellable, GError **error);
  void (*_make_directory_async)(void);
  void (*_make_directory_finish)(void);
  gboolean (*make_symbolic_link)(GFile *file, const char *symlink_value, GCancellable *cancellable, GError **error);
  void (*_make_symbolic_link_async)(void);
  void (*_make_symbolic_link_finish) (void);
  gboolean (*copy)(GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable, GFileProgressCallback progress_callback,
                   gpointer progress_callback_data, GError **error);
  void (*copy_async)(GFile *source, GFile *destination, GFileCopyFlags flags, int io_priority, GCancellable *cancellable, GFileProgressCallback progress_callback,
                     gpointer progress_callback_data, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*copy_finish)(GFile *file, GAsyncResult *res, GError **error);
  gboolean (*move)(GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable, GFileProgressCallback progress_callback,
                   gpointer progress_callback_data, GError **error);
  void (*_move_async)(void);
  void (*_move_finish)(void);
  void (*mount_mountable)(GFile *file, GMountMountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                          gpointer user_data);
  GFile *(*mount_mountable_finish)(GFile *file, GAsyncResult *result, GError **error);
  void (*unmount_mountable)(GFile *file, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*unmount_mountable_finish)(GFile *file, GAsyncResult *result, GError **error);
  void (*eject_mountable)(GFile *file, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer  user_data);
  gboolean (*eject_mountable_finish)(GFile *file, GAsyncResult *result, GError **error);
  void (*mount_enclosing_volume)(GFile *location, GMountMountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable,GAsyncReadyCallback callback,
                                 gpointer user_data);
  gboolean (*mount_enclosing_volume_finish)(GFile *location, GAsyncResult *result, GError **error);
  GFileMonitor *(*monitor_dir)(GFile *file, GFileMonitorFlags flags, GCancellable *cancellable, GError **error);
  GFileMonitor *(*monitor_file)(GFile *file, GFileMonitorFlags flags, GCancellable *cancellable, GError **error);
  GFileIOStream *(*open_readwrite)(GFile *file, GCancellable *cancellable, GError **error);
  void (*open_readwrite_async)(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  GFileIOStream *(*open_readwrite_finish)(GFile *file, GAsyncResult *res, GError **error);
  GFileIOStream *(*create_readwrite)(GFile *file, GFileCreateFlags flags, GCancellable *cancellable, GError **error);
  void (*create_readwrite_async)(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  GFileIOStream *(*create_readwrite_finish)(GFile *file, GAsyncResult *res, GError **error);
  GFileIOStream *(*replace_readwrite)(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, GCancellable *cancellable, GError **error);
  void (*replace_readwrite_async)(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, int io_priority, GCancellable *cancellable,
                                  GAsyncReadyCallback callback, gpointer user_data);
  GFileIOStream *(*replace_readwrite_finish)(GFile *file, GAsyncResult *res, GError **error);
  void (*start_mountable)(GFile *file, GDriveStartFlags flags, GMountOperation *start_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                          gpointer user_data);
  gboolean (*start_mountable_finish)(GFile *file, GAsyncResult *result, GError **error);
  void (*stop_mountable)(GFile *file, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
                         gpointer user_data);
  gboolean (*stop_mountable_finish)(GFile *file, GAsyncResult *result, GError **error);
  gboolean supports_thread_contexts;
  void (*unmount_mountable_with_operation)(GFile *file, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable,
                                           GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*unmount_mountable_with_operation_finish)(GFile *file, GAsyncResult *result, GError **error);
  void (*eject_mountable_with_operation)(GFile *file, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable,
                                         GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*eject_mountable_with_operation_finish)(GFile *file, GAsyncResult *result, GError **error);
  void (*poll_mountable)(GFile *file, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
  gboolean (*poll_mountable_finish)(GFile *file, GAsyncResult *result, GError **error);
};
GType g_file_get_type(void) G_GNUC_CONST;
GFile *g_file_new_for_path(const char *path);
GFile *g_file_new_for_uri(const char *uri);
GFile *g_file_new_for_commandline_arg(const char *arg);
GFile *g_file_parse_name(const char *parse_name);
GFile *g_file_dup(GFile *file);
guint g_file_hash(gconstpointer file);
gboolean g_file_equal(GFile *file1, GFile *file2);
char *g_file_get_basename(GFile *file);
char *g_file_get_path(GFile *file);
char *g_file_get_uri(GFile *file);
char *g_file_get_parse_name(GFile *file);
GFile *g_file_get_parent(GFile *file);
gboolean g_file_has_parent(GFile *file, GFile *parent);
GFile *g_file_get_child(GFile *file, const char *name);
GFile *g_file_get_child_for_display_name(GFile *file, const char *display_name, GError **error);
gboolean g_file_has_prefix(GFile *file, GFile *prefix);
char *g_file_get_relative_path(GFile *parent, GFile *descendant);
GFile *g_file_resolve_relative_path(GFile *file, const char *relative_path);
gboolean g_file_is_native(GFile *file);
gboolean g_file_has_uri_scheme(GFile *file, const char *uri_scheme);
char *g_file_get_uri_scheme(GFile *file);
GFileInputStream *g_file_read(GFile *file, GCancellable *cancellable, GError **error);
void g_file_read_async(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GFileInputStream *g_file_read_finish(GFile *file, GAsyncResult *res, GError **error);
GFileOutputStream *g_file_append_to(GFile *file, GFileCreateFlags flags, GCancellable *cancellable, GError **error);
GFileOutputStream *g_file_create(GFile *file, GFileCreateFlags flags, GCancellable *cancellable, GError **error);
GFileOutputStream *g_file_replace(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, GCancellable *cancellable, GError **error);
void g_file_append_to_async(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GFileOutputStream *g_file_append_to_finish(GFile *file, GAsyncResult *res, GError **error);
void g_file_create_async(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GFileOutputStream *g_file_create_finish(GFile *file, GAsyncResult *res, GError **error);
void g_file_replace_async(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, int io_priority, GCancellable *cancellable,
                          GAsyncReadyCallback callback, gpointer user_data);
GFileOutputStream *g_file_replace_finish(GFile *file, GAsyncResult *res, GError **error);
GFileIOStream *g_file_open_readwrite(GFile *file, GCancellable *cancellable, GError **error);
void g_file_open_readwrite_async(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GFileIOStream *g_file_open_readwrite_finish(GFile *file, GAsyncResult *res, GError **error);
GFileIOStream *g_file_create_readwrite(GFile *file, GFileCreateFlags flags, GCancellable *cancellable, GError **error);
void g_file_create_readwrite_async(GFile *file, GFileCreateFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GFileIOStream *g_file_create_readwrite_finish(GFile *file, GAsyncResult *res, GError **error);
GFileIOStream *g_file_replace_readwrite(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, GCancellable *cancellable, GError **error);
void g_file_replace_readwrite_async(GFile *file, const char *etag, gboolean make_backup, GFileCreateFlags flags, int io_priority, GCancellable *cancellable,
                                    GAsyncReadyCallback callback, gpointer user_data);
GFileIOStream *g_file_replace_readwrite_finish(GFile *file, GAsyncResult *res, GError **error);
gboolean g_file_query_exists(GFile *file, GCancellable *cancellable);
GFileType g_file_query_file_type(GFile *file, GFileQueryInfoFlags flags, GCancellable *cancellable);
GFileInfo *g_file_query_info(GFile *file, const char *attributes, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
void g_file_query_info_async(GFile *file, const char *attributes, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
							 gpointer user_data);
GFileInfo *g_file_query_info_finish(GFile *file, GAsyncResult *res, GError **error);
GFileInfo *g_file_query_filesystem_info(GFile *file, const char *attributes, GCancellable *cancellable, GError **error);
void g_file_query_filesystem_info_async(GFile *file, const char *attributes, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
							            gpointer user_data);
GFileInfo *g_file_query_filesystem_info_finish(GFile *file, GAsyncResult *res, GError **error);
GMount *g_file_find_enclosing_mount(GFile *file, GCancellable *cancellable, GError **error);
void g_file_find_enclosing_mount_async(GFile *file, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GMount *g_file_find_enclosing_mount_finish(GFile *file, GAsyncResult *res, GError **error);
GFileEnumerator *g_file_enumerate_children(GFile *file, const char *attributes, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
void g_file_enumerate_children_async(GFile *file, const char *attributes, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable,
							         GAsyncReadyCallback callback, gpointer user_data);
GFileEnumerator *g_file_enumerate_children_finish(GFile *file, GAsyncResult *res, GError **error);
GFile *g_file_set_display_name(GFile *file, const char *display_name, GCancellable *cancellable, GError **error);
void g_file_set_display_name_async(GFile *file, const char *display_name, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
							       gpointer user_data);
GFile *g_file_set_display_name_finish(GFile *file, GAsyncResult *res, GError **error);
gboolean g_file_delete(GFile *file, GCancellable *cancellable, GError **error);
gboolean g_file_trash(GFile *file, GCancellable *cancellable, GError **error);
gboolean g_file_copy(GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable, GFileProgressCallback progress_callback,
					 gpointer progress_callback_data, GError **error);
void g_file_copy_async(GFile *source, GFile *destination, GFileCopyFlags flags, int io_priority, GCancellable *cancellable, GFileProgressCallback progress_callback,
					   gpointer progress_callback_data, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_file_copy_finish(GFile *file, GAsyncResult *res, GError **error);
gboolean g_file_move(GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable, GFileProgressCallback progress_callback,
					 gpointer progress_callback_data, GError **error);
gboolean g_file_make_directory(GFile *file, GCancellable *cancellable, GError **error);
gboolean g_file_make_directory_with_parents(GFile *file, GCancellable *cancellable, GError **error);
gboolean g_file_make_symbolic_link(GFile *file, const char *symlink_value, GCancellable *cancellable, GError **error);
GFileAttributeInfoList *g_file_query_settable_attributes(GFile *file, GCancellable *cancellable, GError **error);
GFileAttributeInfoList *g_file_query_writable_namespaces(GFile *file, GCancellable *cancellable, GError **error);
gboolean g_file_set_attribute(GFile *file, const char *attribute, GFileAttributeType type, gpointer value_p, GFileQueryInfoFlags flags, GCancellable *cancellable,
							  GError **error);
gboolean g_file_set_attributes_from_info(GFile *file, GFileInfo *info, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
void g_file_set_attributes_async(GFile *file, GFileInfo *info, GFileQueryInfoFlags flags, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
							     gpointer user_data);
gboolean g_file_set_attributes_finish(GFile *file, GAsyncResult *result, GFileInfo **info, GError **error);
gboolean g_file_set_attribute_string(GFile *file, const char *attribute, const char *value, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
gboolean g_file_set_attribute_byte_string(GFile *file, const char *attribute, const char *value, GFileQueryInfoFlags flags, GCancellable *cancellable,
							              GError **error);
gboolean g_file_set_attribute_uint32(GFile *file, const char *attribute, guint32 value, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
gboolean g_file_set_attribute_int32(GFile *file, const char *attribute, gint32 value, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
gboolean g_file_set_attribute_uint64(GFile *file, const char *attribute, guint64 value, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
gboolean g_file_set_attribute_int64(GFile *file, const char *attribute, gint64 value, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
void g_file_mount_enclosing_volume(GFile *location, GMountMountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
							       gpointer user_data);
gboolean g_file_mount_enclosing_volume_finish(GFile *location, GAsyncResult *result, GError **error);
void g_file_mount_mountable(GFile *file, GMountMountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
							gpointer user_data);
GFile *g_file_mount_mountable_finish(GFile *file, GAsyncResult *result, GError **error);
#ifndef G_DISABLE_DEPRECATED
void g_file_unmount_mountable(GFile *file, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_file_unmount_mountable_finish(GFile *file, GAsyncResult *result, GError **error);
#endif
void g_file_unmount_mountable_with_operation(GFile *file, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable,
                                             GAsyncReadyCallback callback, gpointer user_data);
gboolean g_file_unmount_mountable_with_operation_finish(GFile *file, GAsyncResult *result, GError **error);
#ifndef G_DISABLE_DEPRECATED
void g_file_eject_mountable(GFile *file, GMountUnmountFlags flags, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_file_eject_mountable_finish(GFile *file, GAsyncResult *result, GError **error);
#endif
void g_file_eject_mountable_with_operation(GFile *file, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable,
                                           GAsyncReadyCallback callback, gpointer user_data);
gboolean g_file_eject_mountable_with_operation_finish(GFile *file, GAsyncResult *result, GError **error);
gboolean g_file_copy_attributes(GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable, GError **error);
GFileMonitor* g_file_monitor_directory(GFile *file, GFileMonitorFlags flags, GCancellable *cancellable, GError **error);
GFileMonitor* g_file_monitor_file(GFile *file, GFileMonitorFlags flags, GCancellable *cancellable, GError **error);
GFileMonitor* g_file_monitor(GFile *file, GFileMonitorFlags flags, GCancellable *cancellable, GError **error);
void g_file_start_mountable(GFile *file, GDriveStartFlags flags, GMountOperation *start_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
							gpointer user_data);
gboolean g_file_start_mountable_finish(GFile *file, GAsyncResult *result, GError **error);
void g_file_stop_mountable(GFile *file, GMountUnmountFlags flags, GMountOperation *mount_operation, GCancellable *cancellable, GAsyncReadyCallback callback,
						   gpointer user_data);
gboolean g_file_stop_mountable_finish(GFile *file, GAsyncResult *result, GError **error);
void g_file_poll_mountable(GFile *file, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_file_poll_mountable_finish(GFile *file, GAsyncResult *result, GError **error);
GAppInfo *g_file_query_default_handler(GFile *file, GCancellable *cancellable, GError **error);
gboolean g_file_load_contents(GFile *file, GCancellable *cancellable, char **contents, gsize *length, char **etag_out, GError **error);
void g_file_load_contents_async(GFile *file, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean g_file_load_contents_finish(GFile *file, GAsyncResult *res, char **contents, gsize *length, char **etag_out, GError **error);
void g_file_load_partial_contents_async(GFile *file, GCancellable *cancellable, GFileReadMoreCallback read_more_callback, GAsyncReadyCallback callback,
					                    gpointer user_data);
gboolean g_file_load_partial_contents_finish(GFile *file, GAsyncResult *res, char **contents, gsize *length, char **etag_out, GError **error);
gboolean g_file_replace_contents(GFile *file, const char *contents, gsize length, const char *etag, gboolean make_backup, GFileCreateFlags flags, char **new_etag,
					             GCancellable *cancellable, GError **error);
void g_file_replace_contents_async(GFile *file, const char *contents, gsize length, const char *etag, gboolean make_backup, GFileCreateFlags flags,
					               GCancellable *cancellable, GAsyncReadyCallback callback, gpointer  user_data);
gboolean g_file_replace_contents_finish(GFile *file, GAsyncResult *res, char **new_etag, GError **error);
gboolean g_file_supports_thread_contexts(GFile *file);
G_END_DECLS

#endif