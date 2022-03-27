#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_FILE_INFO_H__
#define __G_FILE_INFO_H__

#include "../gobject/gobject.h"
#include "../gobject/gtype.h"
#include "giotypes.h"
#include "gioenums.h"

G_BEGIN_DECLS
#define G_TYPE_FILE_INFO  (g_file_info_get_type ())
#define G_FILE_INFO(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_FILE_INFO, GFileInfo))
#define G_FILE_INFO_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_FILE_INFO, GFileInfoClass))
#define G_IS_FILE_INFO(o)  (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_FILE_INFO))
#define G_IS_FILE_INFO_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_FILE_INFO))
#define G_FILE_INFO_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), G_TYPE_FILE_INFO, GFileInfoClass))
typedef struct _GFileInfoClass   GFileInfoClass;
#define G_FILE_ATTRIBUTE_STANDARD_TYPE "standard::type"
#define G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN "standard::is-hidden"
#define G_FILE_ATTRIBUTE_STANDARD_IS_BACKUP "standard::is-backup"
#define G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK "standard::is-symlink"
#define G_FILE_ATTRIBUTE_STANDARD_IS_VIRTUAL "standard::is-virtual"
#define G_FILE_ATTRIBUTE_STANDARD_NAME "standard::name"
#define G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME "standard::display-name"
#define G_FILE_ATTRIBUTE_STANDARD_EDIT_NAME "standard::edit-name"
#define G_FILE_ATTRIBUTE_STANDARD_COPY_NAME "standard::copy-name"
#define G_FILE_ATTRIBUTE_STANDARD_DESCRIPTION "standard::description"
#define G_FILE_ATTRIBUTE_STANDARD_ICON "standard::icon"
#define G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE "standard::content-type"
#define G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE "standard::fast-content-type"
#define G_FILE_ATTRIBUTE_STANDARD_SIZE "standard::size"
#define G_FILE_ATTRIBUTE_STANDARD_ALLOCATED_SIZE "standard::allocated-size"
#define G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET "standard::symlink-target"
#define G_FILE_ATTRIBUTE_STANDARD_TARGET_URI "standard::target-uri"
#define G_FILE_ATTRIBUTE_STANDARD_SORT_ORDER "standard::sort-order"
#define G_FILE_ATTRIBUTE_ETAG_VALUE "etag::value"
#define G_FILE_ATTRIBUTE_ID_FILE "id::file"
#define G_FILE_ATTRIBUTE_ID_FILESYSTEM "id::filesystem"
#define G_FILE_ATTRIBUTE_ACCESS_CAN_READ "access::can-read"
#define G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE "access::can-write"
#define G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE "access::can-execute"
#define G_FILE_ATTRIBUTE_ACCESS_CAN_DELETE "access::can-delete"
#define G_FILE_ATTRIBUTE_ACCESS_CAN_TRASH "access::can-trash"
#define G_FILE_ATTRIBUTE_ACCESS_CAN_RENAME "access::can-rename"
#define G_FILE_ATTRIBUTE_MOUNTABLE_CAN_MOUNT "mountable::can-mount"
#define G_FILE_ATTRIBUTE_MOUNTABLE_CAN_UNMOUNT "mountable::can-unmount"
#define G_FILE_ATTRIBUTE_MOUNTABLE_CAN_EJECT "mountable::can-eject"
#define G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE "mountable::unix-device"
#define G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE "mountable::unix-device-file"
#define G_FILE_ATTRIBUTE_MOUNTABLE_HAL_UDI "mountable::hal-udi"
#define G_FILE_ATTRIBUTE_MOUNTABLE_CAN_START "mountable::can-start"
#define G_FILE_ATTRIBUTE_MOUNTABLE_CAN_START_DEGRADED "mountable::can-start-degraded"
#define G_FILE_ATTRIBUTE_MOUNTABLE_CAN_STOP "mountable::can-stop"
#define G_FILE_ATTRIBUTE_MOUNTABLE_START_STOP_TYPE "mountable::start-stop-type"
#define G_FILE_ATTRIBUTE_MOUNTABLE_CAN_POLL "mountable::can-poll"
#define G_FILE_ATTRIBUTE_MOUNTABLE_IS_MEDIA_CHECK_AUTOMATIC "mountable::is-media-check-automatic"
#define G_FILE_ATTRIBUTE_TIME_MODIFIED "time::modified"
#define G_FILE_ATTRIBUTE_TIME_MODIFIED_USEC "time::modified-usec"
#define G_FILE_ATTRIBUTE_TIME_ACCESS "time::access"
#define G_FILE_ATTRIBUTE_TIME_ACCESS_USEC "time::access-usec"
#define G_FILE_ATTRIBUTE_TIME_CHANGED "time::changed"
#define G_FILE_ATTRIBUTE_TIME_CHANGED_USEC "time::changed-usec"
#define G_FILE_ATTRIBUTE_TIME_CREATED "time::created"
#define G_FILE_ATTRIBUTE_TIME_CREATED_USEC "time::created-usec"
#define G_FILE_ATTRIBUTE_UNIX_DEVICE "unix::device"
#define G_FILE_ATTRIBUTE_UNIX_INODE "unix::inode"
#define G_FILE_ATTRIBUTE_UNIX_MODE "unix::mode"
#define G_FILE_ATTRIBUTE_UNIX_NLINK "unix::nlink"
#define G_FILE_ATTRIBUTE_UNIX_UID "unix::uid"
#define G_FILE_ATTRIBUTE_UNIX_GID "unix::gid"
#define G_FILE_ATTRIBUTE_UNIX_RDEV "unix::rdev"
#define G_FILE_ATTRIBUTE_UNIX_BLOCK_SIZE "unix::block-size"
#define G_FILE_ATTRIBUTE_UNIX_BLOCKS "unix::blocks"
#define G_FILE_ATTRIBUTE_UNIX_IS_MOUNTPOINT "unix::is-mountpoint"
#define G_FILE_ATTRIBUTE_DOS_IS_ARCHIVE "dos::is-archive"
#define G_FILE_ATTRIBUTE_DOS_IS_SYSTEM "dos::is-system"
#define G_FILE_ATTRIBUTE_OWNER_USER "owner::user"
#define G_FILE_ATTRIBUTE_OWNER_USER_REAL "owner::user-real"
#define G_FILE_ATTRIBUTE_OWNER_GROUP "owner::group"
#define G_FILE_ATTRIBUTE_THUMBNAIL_PATH "thumbnail::path"
#define G_FILE_ATTRIBUTE_THUMBNAILING_FAILED "thumbnail::failed"
#define G_FILE_ATTRIBUTE_PREVIEW_ICON "preview::icon"
#define G_FILE_ATTRIBUTE_FILESYSTEM_SIZE "filesystem::size"
#define G_FILE_ATTRIBUTE_FILESYSTEM_FREE "filesystem::free"
#define G_FILE_ATTRIBUTE_FILESYSTEM_TYPE "filesystem::type"
#define G_FILE_ATTRIBUTE_FILESYSTEM_READONLY "filesystem::readonly"
#define G_FILE_ATTRIBUTE_FILESYSTEM_USE_PREVIEW "filesystem::use-preview"
#define G_FILE_ATTRIBUTE_GVFS_BACKEND "gvfs::backend"
#define G_FILE_ATTRIBUTE_SELINUX_CONTEXT "selinux::context"
#define G_FILE_ATTRIBUTE_TRASH_ITEM_COUNT "trash::item-count"
#define G_FILE_ATTRIBUTE_TRASH_ORIG_PATH "trash::orig-path"
#define G_FILE_ATTRIBUTE_TRASH_DELETION_DATE "trash::deletion-date"
GType g_file_info_get_type(void) G_GNUC_CONST;
GFileInfo *g_file_info_new(void);
GFileInfo *g_file_info_dup(GFileInfo *other);
void g_file_info_copy_into(GFileInfo *src_info, GFileInfo *dest_info);
gboolean g_file_info_has_attribute(GFileInfo *info, const char *attribute);
gboolean g_file_info_has_namespace(GFileInfo *info, const char *name_space);
char **g_file_info_list_attributes(GFileInfo *info, const char *name_space);
gboolean g_file_info_get_attribute_data(GFileInfo *info, const char *attribute, GFileAttributeType *type, gpointer *value_pp, GFileAttributeStatus *status);
GFileAttributeType g_file_info_get_attribute_type(GFileInfo *info, const char *attribute);
void g_file_info_remove_attribute(GFileInfo *info, const char *attribute);
GFileAttributeStatus g_file_info_get_attribute_status(GFileInfo  *info, const char *attribute);
gboolean g_file_info_set_attribute_status(GFileInfo *info, const char *attribute, GFileAttributeStatus status);
char *g_file_info_get_attribute_as_string(GFileInfo *info, const char *attribute);
const char *g_file_info_get_attribute_string(GFileInfo *info, const char *attribute);
const char *g_file_info_get_attribute_byte_string(GFileInfo *info, const char *attribute);
gboolean g_file_info_get_attribute_boolean(GFileInfo *info, const char *attribute);
guint32 g_file_info_get_attribute_uint32(GFileInfo *info, const char *attribute);
gint32 g_file_info_get_attribute_int32(GFileInfo *info, const char *attribute);
guint64 g_file_info_get_attribute_uint64(GFileInfo *info, const char *attribute);
gint64 g_file_info_get_attribute_int64(GFileInfo *info, const char *attribute);
GObject *g_file_info_get_attribute_object(GFileInfo *info, const char *attribute);
char **g_file_info_get_attribute_stringv(GFileInfo *info, const char *attribute);
void g_file_info_set_attribute(GFileInfo *info, const char *attribute, GFileAttributeType type, gpointer value_p);
void g_file_info_set_attribute_string(GFileInfo *info, const char *attribute, const char *attr_value);
void g_file_info_set_attribute_byte_string(GFileInfo *info, const char *attribute, const char *attr_value);
void g_file_info_set_attribute_boolean(GFileInfo *info, const char *attribute, gboolean attr_value);
void g_file_info_set_attribute_uint32(GFileInfo *info, const char *attribute, guint32 attr_value);
void g_file_info_set_attribute_int32(GFileInfo *info, const char *attribute, gint32 attr_value);
void g_file_info_set_attribute_uint64(GFileInfo *info, const char *attribute, guint64 attr_value);
void g_file_info_set_attribute_int64(GFileInfo *info, const char *attribute, gint64 attr_value);
void g_file_info_set_attribute_object(GFileInfo *info, const char *attribute, GObject *attr_value);
void g_file_info_set_attribute_stringv(GFileInfo *info, const char *attribute, char **attr_value);
void g_file_info_clear_status(GFileInfo *info);
GFileType g_file_info_get_file_type(GFileInfo *info);
gboolean g_file_info_get_is_hidden(GFileInfo *info);
gboolean g_file_info_get_is_backup(GFileInfo *info);
gboolean g_file_info_get_is_symlink(GFileInfo *info);
const char *g_file_info_get_name(GFileInfo *info);
const char *g_file_info_get_display_name(GFileInfo *info);
const char *g_file_info_get_edit_name(GFileInfo *info);
GIcon *g_file_info_get_icon(GFileInfo *info);
const char *g_file_info_get_content_type(GFileInfo *info);
goffset g_file_info_get_size(GFileInfo *info);
void g_file_info_get_modification_time(GFileInfo *info, GTimeVal *result);
const char *g_file_info_get_symlink_target(GFileInfo *info);
const char *g_file_info_get_etag(GFileInfo *info);
gint32 g_file_info_get_sort_order(GFileInfo *info);
void g_file_info_set_attribute_mask(GFileInfo *info, GFileAttributeMatcher *mask);
void g_file_info_unset_attribute_mask(GFileInfo *info);
void g_file_info_set_file_type(GFileInfo *info, GFileType type);
void g_file_info_set_is_hidden(GFileInfo *info, gboolean is_hidden);
void g_file_info_set_is_symlink(GFileInfo *info, gboolean is_symlink);
void g_file_info_set_name(GFileInfo *info, const char *name);
void g_file_info_set_display_name(GFileInfo *info, const char *display_name);
void g_file_info_set_edit_name(GFileInfo *info, const char *edit_name);
void g_file_info_set_icon(GFileInfo *info, GIcon *icon);
void g_file_info_set_content_type(GFileInfo *info, const char *content_type);
void g_file_info_set_size(GFileInfo *info, goffset size);
void g_file_info_set_modification_time(GFileInfo *info, GTimeVal *mtime);
void g_file_info_set_symlink_target(GFileInfo *info, const char *symlink_target);
void g_file_info_set_sort_order(GFileInfo *info, gint32 sort_order);
GType g_file_attribute_matcher_get_type(void) G_GNUC_CONST;
GFileAttributeMatcher *g_file_attribute_matcher_new(const char *attributes);
GFileAttributeMatcher *g_file_attribute_matcher_ref(GFileAttributeMatcher *matcher);
void g_file_attribute_matcher_unref(GFileAttributeMatcher *matcher);
gboolean g_file_attribute_matcher_matches(GFileAttributeMatcher *matcher, const char *attribute);
gboolean g_file_attribute_matcher_matches_only(GFileAttributeMatcher *matcher, const char *attribute);
gboolean g_file_attribute_matcher_enumerate_namespace(GFileAttributeMatcher *matcher, const char *ns);
const char *g_file_attribute_matcher_enumerate_next (GFileAttributeMatcher *matcher);
G_END_DECLS

#endif