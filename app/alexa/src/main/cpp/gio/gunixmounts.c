#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/statvfs.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include "../glib/gstdio.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gio.h"
#include "gunixmounts.h"
#include "gfile.h"
#include "gfilemonitor.h"
#include "gthemedicon.h"

static const char *_resolve_dev_root(void);
typedef enum {
  G_UNIX_MOUNT_TYPE_UNKNOWN,
  G_UNIX_MOUNT_TYPE_FLOPPY,
  G_UNIX_MOUNT_TYPE_CDROM,
  G_UNIX_MOUNT_TYPE_NFS,
  G_UNIX_MOUNT_TYPE_ZIP,
  G_UNIX_MOUNT_TYPE_JAZ,
  G_UNIX_MOUNT_TYPE_MEMSTICK,
  G_UNIX_MOUNT_TYPE_CF,
  G_UNIX_MOUNT_TYPE_SM,
  G_UNIX_MOUNT_TYPE_SDMMC,
  G_UNIX_MOUNT_TYPE_IPOD,
  G_UNIX_MOUNT_TYPE_CAMERA,
  G_UNIX_MOUNT_TYPE_HD
} GUnixMountType;
struct _GUnixMountEntry {
  char *mount_path;
  char *device_path;
  char *filesystem_type;
  gboolean is_read_only;
  gboolean is_system_internal;
};
struct _GUnixMountPoint {
  char *mount_path;
  char *device_path;
  char *filesystem_type;
  gboolean is_read_only;
  gboolean is_user_mountable;
  gboolean is_loopback;
};
enum {
  MOUNTS_CHANGED,
  MOUNTPOINTS_CHANGED,
  LAST_SIGNAL
};
static guint signals[LAST_SIGNAL];
struct _GUnixMountMonitor {
  GObject parent;
  GFileMonitor *fstab_monitor;
  GFileMonitor *mtab_monitor;
};
struct _GUnixMountMonitorClass {
  GObjectClass parent_class;
};
static GUnixMountMonitor *the_mount_monitor = NULL;
static GList *_g_get_unix_mounts(void);
static GList *_g_get_unix_mount_points(void);
G_DEFINE_TYPE(GUnixMountMonitor, g_unix_mount_monitor, G_TYPE_OBJECT);
#define MOUNT_POLL_INTERVAL 4000
#ifndef HAVE_SYS_MNTTAB_H
#define MNTOPT_RO	"ro"
#endif
#ifndef HAVE_MNTENT_H
#include <mntent.h>
#elif defined(HAVE_SYS_MNTTAB_H)
#include <sys/mnttab.h>
#endif
#ifdef HAVE_SYS_VFSTAB_H
#include <sys/vfstab.h>
#endif
#if defined(HAVE_SYS_MNTCTL_H) && defined(HAVE_SYS_VMOUNT_H) && defined(HAVE_SYS_VFS_H)
#include <sys/mntctl.h>
#include <sys/vfs.h>
#include <sys/vmount.h>
#include <fshelp.h>
#endif
#if defined(HAVE_GETMNTINFO) && defined(HAVE_FSTAB_H) && defined(HAVE_SYS_MOUNT_H)
#include <sys/ucred.h>
#include <sys/mount.h>
#include <fstab.h>
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#endif
#ifndef HAVE_SETMNTENT
#define setmntent(f,m) fopen(f,m)
#endif
#ifndef HAVE_ENDMNTENT
#define endmntent(f) fclose(f)
#endif
static gboolean is_in(const char *value, const char *set[]) {
  int i;
  for (i = 0; set[i] != NULL; i++) {
      if (strcmp (set[i], value) == 0) return TRUE;
  }
  return FALSE;
}
gboolean g_unix_is_mount_path_system_internal(const char *mount_path) {
  const char *ignore_mountpoints[] = {
      "/", "/bin", "/boot", "/dev", "/etc", "/home", "/lib", "/lib64", "/media", "/mnt", "/opt", "/root", "/sbin", "/srv", "/tmp", "/usr", "/usr/local", "/var",
      "/var/log/audit", "/var/tmp", "/proc", "/sbin", "/net", "/sys", NULL
  };
  if (is_in(mount_path, ignore_mountpoints)) return TRUE;
  if (g_str_has_prefix(mount_path, "/dev/") || g_str_has_prefix(mount_path, "/proc/") || g_str_has_prefix(mount_path, "/sys/")) return TRUE;
  if (g_str_has_suffix(mount_path, "/.gvfs")) return TRUE;
  return FALSE;
}
static gboolean guess_system_internal(const char *mountpoint, const char *fs, const char *device) {
  const char *ignore_fs[] = {
      "auto", "autofs", "devfs", "devpts", "ecryptfs", "kernfs", "linprocfs", "proc", "procfs", "ptyfs", "rootfs", "selinuxfs", "sysfs", "tmpfs", "usbfs",
      "nfsd", "rpc_pipefs", "zfs", NULL
  };
  const char *ignore_devices[] = { "none", "sunrpc", "devpts", "nfsd", "/dev/loop", "/dev/vn", NULL };
  if (is_in(fs, ignore_fs)) return TRUE;
  if (is_in(device, ignore_devices)) return TRUE;
  if (g_unix_is_mount_path_system_internal(mountpoint)) return TRUE;
  return FALSE;
}
#ifndef HAVE_MNTENT_H
static char *get_mtab_read_file(void) {
#ifdef _PATH_MOUNTED
#ifdef __linux__
  return "/proc/mounts";
#else
  return _PATH_MOUNTED;
#endif
#else	
  return "/etc/mtab";
#endif
}
static char *get_mtab_monitor_file(void) {
#ifdef _PATH_MOUNTED
  return _PATH_MOUNTED;
#else	
  return "/etc/mtab";
#endif
}
#ifndef HAVE_GETMNTENT_R
G_LOCK_DEFINE_STATIC(getmntent);
#endif
static GList *_g_get_unix_mounts(void) {
#ifdef HAVE_GETMNTENT_R
  struct mntent ent;
  char buf[1024];
#endif
  struct mntent *mntent;
  FILE *file;
  char *read_file;
  GUnixMountEntry *mount_entry;
  GHashTable *mounts_hash;
  GList *return_list;
  read_file = get_mtab_read_file();
  file = setmntent(read_file, "r");
  if (file == NULL) return NULL;
  return_list = NULL;
  mounts_hash = g_hash_table_new(g_str_hash, g_str_equal);
#ifdef HAVE_GETMNTENT_R
  while((mntent = getmntent_r(file, &ent, buf, sizeof(buf))) != NULL)
#else
  G_LOCK(getmntent);
  while((mntent = getmntent(file)) != NULL)
#endif
  {
      if (mntent->mnt_fsname != NULL && mntent->mnt_fsname[0] == '/' && g_hash_table_lookup(mounts_hash, mntent->mnt_fsname)) continue;
      mount_entry = g_new0(GUnixMountEntry, 1);
      mount_entry->mount_path = g_strdup(mntent->mnt_dir);
      if (strcmp(mntent->mnt_fsname, "/dev/root") == 0) mount_entry->device_path = g_strdup(_resolve_dev_root());
      else mount_entry->device_path = g_strdup(mntent->mnt_fsname);
      mount_entry->filesystem_type = g_strdup(mntent->mnt_type);
#if defined (HAVE_HASMNTOPT)
      if (hasmntopt(mntent, MNTOPT_RO) != NULL) mount_entry->is_read_only = TRUE;
#endif
      mount_entry->is_system_internal = guess_system_internal(mount_entry->mount_path, mount_entry->filesystem_type, mount_entry->device_path);
      g_hash_table_insert(mounts_hash, mount_entry->device_path, mount_entry->device_path);
      return_list = g_list_prepend(return_list, mount_entry);
  }
  g_hash_table_destroy(mounts_hash);
  endmntent(file);
#ifndef HAVE_GETMNTENT_R
  G_UNLOCK(getmntent);
#endif
  return g_list_reverse(return_list);
}
#elif defined (HAVE_SYS_MNTTAB_H)
G_LOCK_DEFINE_STATIC(getmntent);
static char *get_mtab_read_file(void) {
#ifdef _PATH_MOUNTED
  return _PATH_MOUNTED;
#else	
  return "/etc/mnttab";
#endif
}
static char *get_mtab_monitor_file(void) {
  return get_mtab_read_file();
}
static GList *_g_get_unix_mounts(void) {
  struct mnttab mntent;
  FILE *file;
  char *read_file;
  GUnixMountEntry *mount_entry;
  GList *return_list;
  read_file = get_mtab_read_file ();
  file = setmntent(read_file, "r");
  if (file == NULL) return NULL;
  return_list = NULL;
  G_LOCK (getmntent);
  while(! getmntent(file, &mntent)) {
      mount_entry = g_new0(GUnixMountEntry, 1);
      mount_entry->mount_path = g_strdup(mntent.mnt_mountp);
      mount_entry->device_path = g_strdup(mntent.mnt_special);
      mount_entry->filesystem_type = g_strdup(mntent.mnt_fstype);
#if defined (HAVE_HASMNTOPT)
      if (hasmntopt(&mntent, MNTOPT_RO) != NULL) mount_entry->is_read_only = TRUE;
#endif
      mount_entry->is_system_internal = guess_system_internal(mount_entry->mount_path, mount_entry->filesystem_type, mount_entry->device_path);
      return_list = g_list_prepend(return_list, mount_entry);
  }
  endmntent(file);
  G_UNLOCK(getmntent);
  return g_list_reverse(return_list);
}
#elif defined(HAVE_SYS_MNTCTL_H) && defined(HAVE_SYS_VMOUNT_H) && defined(HAVE_SYS_VFS_H)
static char *get_mtab_monitor_file(void) {
  return NULL;
}
static GList *_g_get_unix_mounts(void) {
  struct vfs_ent *fs_info;
  struct vmount *vmount_info;
  int vmount_number;
  unsigned int vmount_size;
  int current;
  GList *return_list;
  if (mntctl(MCTL_QUERY, sizeof (vmount_size), &vmount_size) != 0) {
      g_warning("Unable to know the number of mounted volumes\n");
      return NULL;
  }
  vmount_info = (struct vmount*)g_malloc(vmount_size);
  vmount_number = mntctl(MCTL_QUERY, vmount_size, vmount_info);
  if (vmount_info->vmt_revision != VMT_REVISION) g_warning("Bad vmount structure revision number, want %d, got %d\n", VMT_REVISION, vmount_info->vmt_revision);
  if (vmount_number < 0) {
      g_warning("Unable to recover mounted volumes information\n");
      g_free(vmount_info);
      return NULL;
  }
  return_list = NULL;
  while(vmount_number > 0) {
      mount_entry = g_new0(GUnixMountEntry, 1);
      mount_entry->device_path = g_strdup(vmt2dataptr(vmount_info, VMT_OBJECT));
      mount_entry->mount_path = g_strdup(vmt2dataptr(vmount_info, VMT_STUB));
      mount_entry->is_read_only = (vmount_info->vmt_flags & MNT_READONLY) ? 1 : 0;
      fs_info = getvfsbytype(vmount_info->vmt_gfstype);
      if (fs_info == NULL) mount_entry->filesystem_type = g_strdup("unknown");
      else mount_entry->filesystem_type = g_strdup(fs_info->vfsent_name);
      mount_entry->is_system_internal = guess_system_internal(mount_entry->mount_path, mount_entry->filesystem_type, mount_entry->device_path);
      return_list = g_list_prepend(return_list, mount_entry);
      vmount_info = (struct vmount*)((char*)vmount_info + vmount_info->vmt_length);
      vmount_number--;
  }
  g_free(vmount_info);
  return g_list_reverse(return_list);
}
#elif defined(HAVE_GETMNTINFO) && defined(HAVE_FSTAB_H) && defined(HAVE_SYS_MOUNT_H)
static char *get_mtab_monitor_file(void) {
  return NULL;
}
static GList *_g_get_unix_mounts(void) {
  struct statfs *mntent = NULL;
  int num_mounts, i;
  GUnixMountEntry *mount_entry;
  GList *return_list;
  if ((num_mounts = getmntinfo(&mntent, MNT_NOWAIT)) == 0) return NULL;
  return_list = NULL;
  for (i = 0; i < num_mounts; i++) {
      mount_entry = g_new0(GUnixMountEntry, 1);
      mount_entry->mount_path = g_strdup(mntent[i].f_mntonname);
      mount_entry->device_path = g_strdup(mntent[i].f_mntfromname);
      mount_entry->filesystem_type = g_strdup(mntent[i].f_fstypename);
      if (mntent[i].f_flags & MNT_RDONLY) mount_entry->is_read_only = TRUE;
      mount_entry->is_system_internal = guess_system_internal(mount_entry->mount_path, mount_entry->filesystem_type, mount_entry->device_path);
      return_list = g_list_prepend(return_list, mount_entry);
  }
  return g_list_reverse(return_list);
}
#elif defined(__INTERIX)
static char *get_mtab_monitor_file(void) {
  return NULL;
}
static GList *_g_get_unix_mounts(void) {
  DIR *dirp;
  GList* return_list = NULL;
  char filename[9 + NAME_MAX];
  dirp = opendir("/dev/fs");
  if (!dirp) {
      g_warning("unable to read /dev/fs!");
      return NULL;
  }
  while(1) {
      struct statvfs statbuf;
      struct dirent entry;
      struct dirent* result;
      if (readdir_r(dirp, &entry, &result) || result == NULL) break;
      strcpy(filename, "/dev/fs/");
      strcat(filename, entry.d_name);
      if (statvfs(filename, &statbuf) == 0) {
          GUnixMountEntry* mount_entry = g_new0(GUnixMountEntry, 1);
          mount_entry->mount_path = g_strdup(statbuf.f_mntonname);
          mount_entry->device_path = g_strdup(statbuf.f_mntfromname);
          mount_entry->filesystem_type = g_strdup(statbuf.f_fstypename);
          if (statbuf.f_flag & ST_RDONLY) mount_entry->is_read_only = TRUE;
          return_list = g_list_prepend(return_list, mount_entry);
      }
  }
  return_list = g_list_reverse(return_list);
  closedir(dirp);
  return return_list;
}
#else
#error No _g_get_unix_mounts() implementation for system
#endif
static char *get_fstab_file(void) {
#if defined(HAVE_SYS_MNTCTL_H) && defined(HAVE_SYS_VMOUNT_H) && defined(HAVE_SYS_VFS_H)
  return "/etc/filesystems";
#elif defined(_PATH_MNTTAB)
  return _PATH_MNTTAB;
#elif defined(VFSTAB)
  return VFSTAB;
#else
  return "/etc/fstab";
#endif
}
#ifndef HAVE_MNTENT_H
static GList *_g_get_unix_mount_points(void) {
#ifdef HAVE_GETMNTENT_R
  struct mntent ent;
  char buf[1024];
#endif
  struct mntent *mntent;
  FILE *file;
  char *read_file;
  GUnixMountPoint *mount_entry;
  GList *return_list;
  read_file = get_fstab_file();
  file = setmntent (read_file, "r");
  if (file == NULL) return NULL;
  return_list = NULL;
#ifdef HAVE_GETMNTENT_R
  while((mntent = getmntent_r(file, &ent, buf, sizeof(buf))) != NULL)
#else
  G_LOCK(getmntent);
  while((mntent = getmntent(file)) != NULL)
#endif
  {
      if ((strcmp(mntent->mnt_dir, "ignore") == 0) || (strcmp(mntent->mnt_dir, "swap") == 0)) continue;
      mount_entry = g_new0(GUnixMountPoint, 1);
      mount_entry->mount_path = g_strdup(mntent->mnt_dir);
      if (strcmp(mntent->mnt_fsname, "/dev/root") == 0) mount_entry->device_path = g_strdup(_resolve_dev_root());
      else mount_entry->device_path = g_strdup(mntent->mnt_fsname);
      mount_entry->filesystem_type = g_strdup(mntent->mnt_type);
#ifdef HAVE_HASMNTOPT
      if (hasmntopt(mntent, MNTOPT_RO) != NULL) mount_entry->is_read_only = TRUE;
      if (hasmntopt(mntent, "loop") != NULL) mount_entry->is_loopback = TRUE;
#endif
      if ((mntent->mnt_type != NULL && strcmp("supermount", mntent->mnt_type) == 0)
      #ifdef HAVE_HASMNTOPT
	      || (hasmntopt(mntent, "user") != NULL && hasmntopt(mntent, "user") != hasmntopt(mntent, "user_xattr")) || hasmntopt(mntent, "pamconsole") != NULL
	      || hasmntopt(mntent, "users") != NULL || hasmntopt(mntent, "owner") != NULL
      #endif
	     ) {
          mount_entry->is_user_mountable = TRUE;
      }
      return_list = g_list_prepend(return_list, mount_entry);
  }
  endmntent(file);
#ifndef HAVE_GETMNTENT_R
  G_UNLOCK(getmntent);
#endif
  return g_list_reverse(return_list);
}
#elif defined (HAVE_SYS_MNTTAB_H)
static GList *_g_get_unix_mount_points(void) {
  struct mnttab mntent;
  FILE *file;
  char *read_file;
  GUnixMountPoint *mount_entry;
  GList *return_list;
  read_file = get_fstab_file();
  file = setmntent(read_file, "r");
  if (file == NULL) return NULL;
  return_list = NULL;
  G_LOCK(getmntent);
  while(! getmntent (file, &mntent)) {
      if ((strcmp(mntent.mnt_mountp, "ignore") == 0) || (strcmp(mntent.mnt_mountp, "swap") == 0)) continue;
      mount_entry = g_new0(GUnixMountPoint, 1);
      mount_entry->mount_path = g_strdup(mntent.mnt_mountp);
      mount_entry->device_path = g_strdup(mntent.mnt_special);
      mount_entry->filesystem_type = g_strdup(mntent.mnt_fstype);
#ifdef HAVE_HASMNTOPT
      if (hasmntopt(&mntent, MNTOPT_RO) != NULL) mount_entry->is_read_only = TRUE;
      if (hasmntopt(&mntent, "lofs") != NULL) mount_entry->is_loopback = TRUE;
#endif
      if ((mntent.mnt_fstype != NULL)
      #ifdef HAVE_HASMNTOPT
	      || (hasmntopt(&mntent, "user") != NULL && hasmntopt(&mntent, "user") != hasmntopt(&mntent, "user_xattr"))
	      || hasmntopt(&mntent, "pamconsole") != NULL || hasmntopt(&mntent, "users") != NULL || hasmntopt(&mntent, "owner") != NULL
      #endif
	     ) {
          mount_entry->is_user_mountable = TRUE;
      }
      return_list = g_list_prepend(return_list, mount_entry);
  }
  endmntent(file);
  G_UNLOCK(getmntent);
  return g_list_reverse(return_list);
}
#elif defined(HAVE_SYS_MNTCTL_H) && defined(HAVE_SYS_VMOUNT_H) && defined(HAVE_SYS_VFS_H)
static int aix_fs_getc(FILE *fd) {
  int c;
  while((c = getc(fd)) == '*') {
      while(((c = getc(fd)) != '\n') && (c != EOF));
  }
}
static int aix_fs_ignorespace(FILE *fd) {
  int c;
  while((c = aix_fs_getc(fd)) != EOF) {
      if (!g_ascii_isspace(c)) {
          ungetc(c,fd);
          return c;
	  }
  }
  return EOF;
}
static int aix_fs_getword(FILE *fd, char *word) {
  int c;
  aix_fs_ignorespace(fd);
  while(((c = aix_fs_getc(fd)) != EOF) && !g_ascii_isspace(c)) {
      if (c == '"') {
          while(((c = aix_fs_getc (fd)) != EOF) && (c != '"')) *word++ = c;
          else *word++ = c;
	  }
  }
  *word = 0;
  return c;
}
typedef struct {
  char mnt_mount[PATH_MAX];
  char mnt_special[PATH_MAX];
  char mnt_fstype[16];
  char mnt_options[128];
} AixMountTableEntry;
static int aix_fs_get (FILE *fd, AixMountTableEntry *prop) {
  static char word[PATH_MAX] = { 0 };
  char value[PATH_MAX];
  if (word[0] == 0) {
      if (aix_fs_getword(fd, word) == EOF) return EOF;
  }
  word[strlen(word) - 1] = 0;
  strcpy (prop->mnt_mount, word);
  while(aix_fs_getword(fd, word) != EOF) {
      if (word[strlen(word) - 1] == ':') return 0;
      aix_fs_getword(fd, value);
      aix_fs_getword(fd, value);
      if (strcmp(word, "dev") == 0) strcpy(prop->mnt_special, value);
      else if (strcmp(word, "vfs") == 0) strcpy(prop->mnt_fstype, value);
      else if (strcmp(word, "options") == 0) strcpy(prop->mnt_options, value);
  }
  return 0;
}
static GList *_g_get_unix_mount_points(void) {
  struct mntent *mntent;
  FILE *file;
  char *read_file;
  GUnixMountPoint *mount_entry;
  AixMountTableEntry mntent;
  GList *return_list;
  read_file = get_fstab_file();
  file = setmntent(read_file, "r");
  if (file == NULL) return NULL;
  return_list = NULL;
  while (!aix_fs_get(file, &mntent)) {
      if (strcmp("cdrfs", mntent.mnt_fstype) == 0) {
	  mount_entry = g_new0(GUnixMountPoint, 1);
	  mount_entry->mount_path = g_strdup(mntent.mnt_mount);
	  mount_entry->device_path = g_strdup(mntent.mnt_special);
	  mount_entry->filesystem_type = g_strdup(mntent.mnt_fstype);
	  mount_entry->is_read_only = TRUE;
	  mount_entry->is_user_mountable = TRUE;
	  return_list = g_list_prepend(return_list, mount_entry);
	  }
  }
  endmntent(file);
  return g_list_reverse(return_list);
}
#elif defined(HAVE_GETMNTINFO) && defined(HAVE_FSTAB_H) && defined(HAVE_SYS_MOUNT_H)
static GList *_g_get_unix_mount_points(void) {
  struct fstab *fstab = NULL;
  GUnixMountPoint *mount_entry;
  GList *return_list;
#ifdef HAVE_SYS_SYSCTL_H
  int usermnt = 0;
  size_t len = sizeof(usermnt);
  struct stat sb;
#endif
  if (!setfsent()) return NULL;
  return_list = NULL;
#ifdef HAVE_SYS_SYSCTL_H
#if defined(HAVE_SYSCTLBYNAME)
  sysctlbyname("vfs.usermount", &usermnt, &len, NULL, 0);
#elif defined(CTL_VFS) && defined(VFS_USERMOUNT)
  {
      int mib[2];
      mib[0] = CTL_VFS;
      mib[1] = VFS_USERMOUNT;
      sysctl(mib, 2, &usermnt, &len, NULL, 0);
  }
#elif defined(CTL_KERN) && defined(KERN_USERMOUNT)
  {
      int mib[2];
      mib[0] = CTL_KERN;
      mib[1] = KERN_USERMOUNT;
      sysctl(mib, 2, &usermnt, &len, NULL, 0);
  }
#endif
#endif
  while((fstab = getfsent()) != NULL) {
      if (strcmp(fstab->fs_vfstype, "swap") == 0) continue;
      mount_entry = g_new0(GUnixMountPoint, 1);
      mount_entry->mount_path = g_strdup(fstab->fs_file);
      mount_entry->device_path = g_strdup(fstab->fs_spec);
      mount_entry->filesystem_type = g_strdup(fstab->fs_vfstype);
      if (strcmp(fstab->fs_type, "ro") == 0) mount_entry->is_read_only = TRUE;
  #ifdef HAVE_SYS_SYSCTL_H
      if (usermnt != 0) {
          uid_t uid = getuid();
          if (stat(fstab->fs_file, &sb) == 0) {
              if (uid == 0 || sb.st_uid == uid) mount_entry->is_user_mountable = TRUE;
          }
	  }
  #endif
      return_list = g_list_prepend(return_list, mount_entry);
  }
  endfsent();
  return g_list_reverse(return_list);
}
#elif defined(__INTERIX)
static GList *_g_get_unix_mount_points(void) {
  return _g_get_unix_mounts();
}
#else
#error No g_get_mount_table() implementation for system
#endif
static guint64 get_mounts_timestamp(void) {
  const char *monitor_file;
  struct stat buf;
  monitor_file = get_mtab_monitor_file();
  if (monitor_file) {
      if (stat (monitor_file, &buf) == 0) return (guint64)buf.st_mtime;
  }
  return 0;
}
static guint64 get_mount_points_timestamp(void) {
  const char *monitor_file;
  struct stat buf;
  monitor_file = get_fstab_file();
  if (monitor_file) {
      if (stat (monitor_file, &buf) == 0) return (guint64)buf.st_mtime;
  }
  return 0;
}
GList *g_unix_mounts_get(guint64 *time_read) {
  if (time_read) *time_read = get_mounts_timestamp();
  return _g_get_unix_mounts();
}
GUnixMountEntry *g_unix_mount_at(const char *mount_path, guint64 *time_read) {
  GList *mounts, *l;
  GUnixMountEntry *mount_entry, *found;
  mounts = g_unix_mounts_get(time_read);
  found = NULL;
  for (l = mounts; l != NULL; l = l->next) {
      mount_entry = l->data;
      if (!found && strcmp(mount_path, mount_entry->mount_path) == 0) found = mount_entry;
      else g_unix_mount_free(mount_entry);
  }
  g_list_free(mounts);
  return found;
}
GList *g_unix_mount_points_get(guint64 *time_read) {
  if (time_read) *time_read = get_mount_points_timestamp();
  return _g_get_unix_mount_points();
}
gboolean g_unix_mounts_changed_since(guint64 time) {
  return get_mounts_timestamp() != time;
}
gboolean g_unix_mount_points_changed_since(guint64 time) {
  return get_mount_points_timestamp() != time;
}
static void g_unix_mount_monitor_finalize(GObject *object) {
  GUnixMountMonitor *monitor;
  monitor = G_UNIX_MOUNT_MONITOR(object);
  if (monitor->fstab_monitor) {
      g_file_monitor_cancel(monitor->fstab_monitor);
      g_object_unref(monitor->fstab_monitor);
  }
  if (monitor->mtab_monitor) {
      g_file_monitor_cancel(monitor->mtab_monitor);
      g_object_unref(monitor->mtab_monitor);
  }
  the_mount_monitor = NULL;
  G_OBJECT_CLASS(g_unix_mount_monitor_parent_class)->finalize(object);
}
static void g_unix_mount_monitor_class_init(GUnixMountMonitorClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_unix_mount_monitor_finalize;
  signals[MOUNTS_CHANGED] = g_signal_new("mounts-changed", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
  signals[MOUNTPOINTS_CHANGED] = g_signal_new("mountpoints-changed", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID,
		                                      G_TYPE_NONE, 0);
}
static void fstab_file_changed(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type, gpointer user_data) {
  GUnixMountMonitor *mount_monitor;
  if (event_type != G_FILE_MONITOR_EVENT_CHANGED && event_type != G_FILE_MONITOR_EVENT_CREATED && event_type != G_FILE_MONITOR_EVENT_DELETED) return;
  mount_monitor = user_data;
  g_signal_emit (mount_monitor, signals[MOUNTPOINTS_CHANGED], 0);
}
static void mtab_file_changed(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type, gpointer user_data) {
  GUnixMountMonitor *mount_monitor;
  if (event_type != G_FILE_MONITOR_EVENT_CHANGED && event_type != G_FILE_MONITOR_EVENT_CREATED && event_type != G_FILE_MONITOR_EVENT_DELETED) return;
  mount_monitor = user_data;
  g_signal_emit(mount_monitor, signals[MOUNTS_CHANGED], 0);
}
static void g_unix_mount_monitor_init(GUnixMountMonitor *monitor) {
  GFile *file;
  if (get_fstab_file() != NULL) {
      file = g_file_new_for_path(get_fstab_file());
      monitor->fstab_monitor = g_file_monitor_file(file, 0, NULL, NULL);
      g_object_unref(file);
      g_signal_connect(monitor->fstab_monitor, "changed", (GCallback)fstab_file_changed, monitor);
  }
  if (get_mtab_monitor_file() != NULL) {
      file = g_file_new_for_path(get_mtab_monitor_file());
      monitor->mtab_monitor = g_file_monitor_file(file, 0, NULL, NULL);
      g_object_unref(file);
      g_signal_connect(monitor->mtab_monitor, "changed", (GCallback)mtab_file_changed, monitor);
  }
}
void g_unix_mount_monitor_set_rate_limit(GUnixMountMonitor *mount_monitor, gint limit_msec) {
  g_return_if_fail(G_IS_UNIX_MOUNT_MONITOR(mount_monitor));
  if (mount_monitor->fstab_monitor != NULL) g_file_monitor_set_rate_limit(mount_monitor->fstab_monitor, limit_msec);
  if (mount_monitor->mtab_monitor != NULL) g_file_monitor_set_rate_limit(mount_monitor->mtab_monitor, limit_msec);
}
GUnixMountMonitor * g_unix_mount_monitor_new(void) {
  if (the_mount_monitor == NULL) {
      the_mount_monitor = g_object_new(G_TYPE_UNIX_MOUNT_MONITOR, NULL);
      return the_mount_monitor;
  }
  return g_object_ref (the_mount_monitor);
}
void g_unix_mount_free(GUnixMountEntry *mount_entry) {
  g_return_if_fail(mount_entry != NULL);
  g_free(mount_entry->mount_path);
  g_free(mount_entry->device_path);
  g_free(mount_entry->filesystem_type);
  g_free(mount_entry);
}
void g_unix_mount_point_free(GUnixMountPoint *mount_point) {
  g_return_if_fail(mount_point != NULL);
  g_free(mount_point->mount_path);
  g_free(mount_point->device_path);
  g_free(mount_point->filesystem_type);
  g_free(mount_point);
}
gint g_unix_mount_compare(GUnixMountEntry *mount1, GUnixMountEntry *mount2) {
  int res;
  g_return_val_if_fail(mount1 != NULL && mount2 != NULL, 0);
  res = g_strcmp0(mount1->mount_path, mount2->mount_path);
  if (res != 0) return res;
  res = g_strcmp0(mount1->device_path, mount2->device_path);
  if (res != 0) return res;
  res = g_strcmp0(mount1->filesystem_type, mount2->filesystem_type);
  if (res != 0) return res;
  res =  mount1->is_read_only - mount2->is_read_only;
  if (res != 0) return res;
  return 0;
}
const gchar *g_unix_mount_get_mount_path(GUnixMountEntry *mount_entry) {
  g_return_val_if_fail(mount_entry != NULL, NULL);
  return mount_entry->mount_path;
}
const gchar *g_unix_mount_get_device_path(GUnixMountEntry *mount_entry) {
  g_return_val_if_fail(mount_entry != NULL, NULL);
  return mount_entry->device_path;
}
const gchar *g_unix_mount_get_fs_type(GUnixMountEntry *mount_entry) {
  g_return_val_if_fail(mount_entry != NULL, NULL);
  return mount_entry->filesystem_type;
}
gboolean g_unix_mount_is_readonly(GUnixMountEntry *mount_entry) {
  g_return_val_if_fail(mount_entry != NULL, FALSE);
  return mount_entry->is_read_only;
}
gboolean g_unix_mount_is_system_internal(GUnixMountEntry *mount_entry) {
  g_return_val_if_fail(mount_entry != NULL, FALSE);
  return mount_entry->is_system_internal;
}
gint g_unix_mount_point_compare(GUnixMountPoint *mount1, GUnixMountPoint *mount2) {
  int res;
  g_return_val_if_fail(mount1 != NULL && mount2 != NULL, 0);
  res = g_strcmp0(mount1->mount_path, mount2->mount_path);
  if (res != 0) return res;
  res = g_strcmp0(mount1->device_path, mount2->device_path);
  if (res != 0) return res;
  res = g_strcmp0(mount1->filesystem_type, mount2->filesystem_type);
  if (res != 0) return res;
  res =  mount1->is_read_only - mount2->is_read_only;
  if (res != 0) return res;
  res = mount1->is_user_mountable - mount2->is_user_mountable;
  if (res != 0) return res;
  res = mount1->is_loopback - mount2->is_loopback;
  if (res != 0) return res;
  return 0;
}
const gchar *g_unix_mount_point_get_mount_path(GUnixMountPoint *mount_point) {
  g_return_val_if_fail(mount_point != NULL, NULL);
  return mount_point->mount_path;
}
const gchar *g_unix_mount_point_get_device_path(GUnixMountPoint *mount_point) {
  g_return_val_if_fail(mount_point != NULL, NULL);
  return mount_point->device_path;
}
const gchar *g_unix_mount_point_get_fs_type(GUnixMountPoint *mount_point) {
  g_return_val_if_fail(mount_point != NULL, NULL);
  return mount_point->filesystem_type;
}
gboolean g_unix_mount_point_is_readonly(GUnixMountPoint *mount_point) {
  g_return_val_if_fail(mount_point != NULL, FALSE);
  return mount_point->is_read_only;
}
gboolean g_unix_mount_point_is_user_mountable(GUnixMountPoint *mount_point) {
  g_return_val_if_fail(mount_point != NULL, FALSE);
  return mount_point->is_user_mountable;
}
gboolean g_unix_mount_point_is_loopback(GUnixMountPoint *mount_point) {
  g_return_val_if_fail(mount_point != NULL, FALSE);
  return mount_point->is_loopback;
}
static GUnixMountType guess_mount_type(const char *mount_path, const char *device_path, const char *filesystem_type) {
  GUnixMountType type;
  char *basename;
  type = G_UNIX_MOUNT_TYPE_UNKNOWN;
  if ((strcmp(filesystem_type, "udf") == 0) || (strcmp(filesystem_type, "iso9660") == 0) || (strcmp(filesystem_type, "cd9660") == 0)) type = G_UNIX_MOUNT_TYPE_CDROM;
  else if ((strcmp (filesystem_type, "nfs") == 0) || (strcmp (filesystem_type, "nfs4") == 0)) type = G_UNIX_MOUNT_TYPE_NFS;
  else if (g_str_has_prefix(device_path, "/vol/dev/diskette/") || g_str_has_prefix(device_path, "/dev/fd") || g_str_has_prefix(device_path, "/dev/floppy")) {
      type = G_UNIX_MOUNT_TYPE_FLOPPY;
  } else if (g_str_has_prefix(device_path, "/dev/cdrom") || g_str_has_prefix(device_path, "/dev/acd") || g_str_has_prefix(device_path, "/dev/cd")) {
      type = G_UNIX_MOUNT_TYPE_CDROM;
  } else if (g_str_has_prefix(device_path, "/vol/")) {
      const char *name = mount_path + strlen("/");
      if (g_str_has_prefix(name, "cdrom")) type = G_UNIX_MOUNT_TYPE_CDROM;
      else if (g_str_has_prefix(name, "floppy") || g_str_has_prefix (device_path, "/vol/dev/diskette/")) type = G_UNIX_MOUNT_TYPE_FLOPPY;
      else if (g_str_has_prefix(name, "rmdisk")) type = G_UNIX_MOUNT_TYPE_ZIP;
      else if (g_str_has_prefix(name, "jaz")) type = G_UNIX_MOUNT_TYPE_JAZ;
      else if (g_str_has_prefix(name, "memstick")) type = G_UNIX_MOUNT_TYPE_MEMSTICK;
  } else {
      basename = g_path_get_basename(mount_path);
      if (g_str_has_prefix(basename, "cdr") || g_str_has_prefix(basename, "cdwriter") || g_str_has_prefix(basename, "burn") ||
          g_str_has_prefix(basename, "dvdr")) {
          type = G_UNIX_MOUNT_TYPE_CDROM;
      } else if (g_str_has_prefix(basename, "floppy")) type = G_UNIX_MOUNT_TYPE_FLOPPY;
      else if (g_str_has_prefix(basename, "zip")) type = G_UNIX_MOUNT_TYPE_ZIP;
      else if (g_str_has_prefix(basename, "jaz")) type = G_UNIX_MOUNT_TYPE_JAZ;
      else if (g_str_has_prefix(basename, "camera")) type = G_UNIX_MOUNT_TYPE_CAMERA;
      else if (g_str_has_prefix(basename, "memstick") || g_str_has_prefix(basename, "memory_stick") || g_str_has_prefix(basename, "ram")) {
          type = G_UNIX_MOUNT_TYPE_MEMSTICK;
      } else if (g_str_has_prefix(basename, "compact_flash")) type = G_UNIX_MOUNT_TYPE_CF;
      else if (g_str_has_prefix(basename, "smart_media")) type = G_UNIX_MOUNT_TYPE_SM;
      else if (g_str_has_prefix(basename, "sd_mmc")) type = G_UNIX_MOUNT_TYPE_SDMMC;
      else if (g_str_has_prefix(basename, "ipod")) type = G_UNIX_MOUNT_TYPE_IPOD;
      g_free(basename);
  }
  if (type == G_UNIX_MOUNT_TYPE_UNKNOWN) type = G_UNIX_MOUNT_TYPE_HD;
  return type;
}
static GUnixMountType g_unix_mount_guess_type(GUnixMountEntry *mount_entry) {
  g_return_val_if_fail(mount_entry != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);
  g_return_val_if_fail(mount_entry->mount_path != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);
  g_return_val_if_fail(mount_entry->device_path != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);
  g_return_val_if_fail(mount_entry->filesystem_type != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);
  return guess_mount_type(mount_entry->mount_path, mount_entry->device_path, mount_entry->filesystem_type);
}
static GUnixMountType g_unix_mount_point_guess_type(GUnixMountPoint *mount_point) {
  g_return_val_if_fail(mount_point != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);
  g_return_val_if_fail(mount_point->mount_path != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);
  g_return_val_if_fail(mount_point->device_path != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);
  g_return_val_if_fail(mount_point->filesystem_type != NULL, G_UNIX_MOUNT_TYPE_UNKNOWN);
  return guess_mount_type(mount_point->mount_path, mount_point->device_path, mount_point->filesystem_type);
}
static const char *type_to_icon(GUnixMountType type, gboolean is_mount_point) {
  const char *icon_name;
  switch(type) {
      case G_UNIX_MOUNT_TYPE_HD:
          if (is_mount_point) icon_name = "drive-removable-media";
          else icon_name = "drive-harddisk";
          break;
      case G_UNIX_MOUNT_TYPE_FLOPPY: case G_UNIX_MOUNT_TYPE_ZIP: case G_UNIX_MOUNT_TYPE_JAZ:
          if (is_mount_point) icon_name = "drive-removable-media";
          else icon_name = "media-floppy";
          break;
      case G_UNIX_MOUNT_TYPE_CDROM:
          if (is_mount_point) icon_name = "drive-optical";
          else icon_name = "media-optical";
          break;
      case G_UNIX_MOUNT_TYPE_NFS:
          if (is_mount_point) icon_name = "drive-removable-media";
          else icon_name = "drive-harddisk";
          break;
      case G_UNIX_MOUNT_TYPE_MEMSTICK:
          if (is_mount_point) icon_name = "drive-removable-media";
          else icon_name = "media-flash";
          break;
      case G_UNIX_MOUNT_TYPE_CAMERA:
          if (is_mount_point) icon_name = "drive-removable-media";
          else icon_name = "camera-photo";
          break;
      case G_UNIX_MOUNT_TYPE_IPOD:
          if (is_mount_point) icon_name = "drive-removable-media";
          else icon_name = "multimedia-player";
          break;
      default:
          if (is_mount_point) icon_name = "drive-removable-media";
          else icon_name = "drive-harddisk";
  }
  return icon_name;
}
gchar *g_unix_mount_guess_name(GUnixMountEntry *mount_entry) {
  char *name;
  if (strcmp(mount_entry->mount_path, "/") == 0) name = g_strdup(_("Filesystem root"));
  else name = g_filename_display_basename(mount_entry->mount_path);
  return name;
}
GIcon *g_unix_mount_guess_icon(GUnixMountEntry *mount_entry) {
  return g_themed_icon_new_with_default_fallbacks(type_to_icon(g_unix_mount_guess_type(mount_entry), FALSE));
}
gchar *g_unix_mount_point_guess_name(GUnixMountPoint *mount_point) {
  char *name;
  if (strcmp(mount_point->mount_path, "/") == 0) name = g_strdup(_("Filesystem root"));
  else name = g_filename_display_basename(mount_point->mount_path);
  return name;
}
GIcon *g_unix_mount_point_guess_icon(GUnixMountPoint *mount_point) {
  return g_themed_icon_new_with_default_fallbacks(type_to_icon (g_unix_mount_point_guess_type (mount_point), TRUE));
}
gboolean g_unix_mount_guess_can_eject(GUnixMountEntry *mount_entry) {
  GUnixMountType guessed_type;
  guessed_type = g_unix_mount_guess_type(mount_entry);
  if (guessed_type == G_UNIX_MOUNT_TYPE_IPOD || guessed_type == G_UNIX_MOUNT_TYPE_CDROM) return TRUE;
  return FALSE;
}
gboolean g_unix_mount_guess_should_display(GUnixMountEntry *mount_entry) {
  const char *mount_path;
  if (g_unix_mount_is_system_internal(mount_entry)) return FALSE;
  mount_path = mount_entry->mount_path;
  if (mount_path != NULL) {
      if (g_strstr_len(mount_path, -1, "/.") != NULL) return FALSE;
      if (g_str_has_prefix(mount_path, "/media/")) {
          char *path;
          path = g_path_get_dirname(mount_path);
          if (g_str_has_prefix(path, "/media/")) {
              if (g_access(path, R_OK|X_OK) != 0) {
                  g_free(path);
                  return FALSE;
              }
          }
          g_free(path);
          if (mount_entry->device_path && mount_entry->device_path[0] == '/') {
             struct stat st;
             if (g_stat(mount_entry->device_path, &st) == 0 && S_ISBLK(st.st_mode) && g_access (mount_path, R_OK | X_OK) != 0) return FALSE;
          }
          return TRUE;
      }
      if (g_str_has_prefix(mount_path, g_get_home_dir()) && mount_path[strlen (g_get_home_dir())] == G_DIR_SEPARATOR) return TRUE;
  }
  return FALSE;
}
gboolean g_unix_mount_point_guess_can_eject(GUnixMountPoint *mount_point) {
  GUnixMountType guessed_type;
  guessed_type = g_unix_mount_point_guess_type(mount_point);
  if (guessed_type == G_UNIX_MOUNT_TYPE_IPOD || guessed_type == G_UNIX_MOUNT_TYPE_CDROM) return TRUE;
  return FALSE;
}
static void _canonicalize_filename(gchar *filename) {
  gchar *p, *q;
  gboolean last_was_slash = FALSE;
  p = filename;
  q = filename;
  while(*p) {
      if (*p == G_DIR_SEPARATOR) {
          if (!last_was_slash) *q++ = G_DIR_SEPARATOR;
          last_was_slash = TRUE;
      } else {
          if (last_was_slash && *p == '.') {
              if (*(p + 1) == G_DIR_SEPARATOR || *(p + 1) == '\0') {
                  if (*(p + 1) == '\0') break;
                  p += 1;
              } else if (*(p + 1) == '.' && (*(p + 2) == G_DIR_SEPARATOR || *(p + 2) == '\0')) {
                  if (q > filename + 1) {
                      q--;
                      while(q > filename + 1 && *(q - 1) != G_DIR_SEPARATOR) q--;
                  }
                  if (*(p + 2) == '\0') break;
                  p += 2;
              } else {
                  *q++ = *p;
                  last_was_slash = FALSE;
              }
          } else {
              *q++ = *p;
              last_was_slash = FALSE;
          }
      }
      p++;
  }
  if (q > filename + 1 && *(q - 1) == G_DIR_SEPARATOR) q--;
  *q = '\0';
}
static char *_resolve_symlink(const char *file) {
  GError *error;
  char *dir;
  char *link;
  char *f;
  char *f1;
  f = g_strdup(file);
  while(g_file_test (f, G_FILE_TEST_IS_SYMLINK)) {
      link = g_file_read_link(f, &error);
      if (link == NULL) {
          g_error_free(error);
          g_free(f);
          f = NULL;
          goto out;
      }
      dir = g_path_get_dirname(f);
      f1 = g_strdup_printf("%s/%s", dir, link);
      g_free(dir);
      g_free(link);
      g_free(f);
      f = f1;
  }
out:
  if (f != NULL) _canonicalize_filename (f);
  return f;
}
#ifdef HAVE_MNTENT_H
static const char *_resolve_dev_root(void) {
  static gboolean have_real_dev_root = FALSE;
  static char real_dev_root[256];
  struct stat statbuf;
  if (have_real_dev_root) goto found;
  have_real_dev_root = TRUE;
  if (stat("/dev/root", &statbuf) == 0)
      if (! S_ISLNK(statbuf.st_mode)) {
          dev_t root_dev = statbuf.st_dev;
          FILE *f;
          char buf[1024];
          f = fopen("/etc/mtab", "r");
          if (f != NULL) {
              struct mntent *entp;
          #ifdef HAVE_GETMNTENT_R
              struct mntent ent;
              while((entp = getmntent_r(f, &ent, buf, sizeof(buf))) != NULL) {
          #else
              G_LOCK(getmntent);
          #endif
              while((entp = getmntent(f)) != NULL) {
                  if (stat(entp->mnt_fsname, &statbuf) == 0 && statbuf.st_dev == root_dev) {
                      strncpy(real_dev_root, entp->mnt_fsname, sizeof(real_dev_root) - 1);
                      real_dev_root[sizeof(real_dev_root) - 1] = '\0';
                      fclose(f);
                      goto found;
                  }
              }
          #ifndef HAVE_GETMNTENT_R
              endmntent(f);
              G_UNLOCK(getmntent);
              }
          #endif
          } else {
              char *resolved;
              resolved = _resolve_symlink("/dev/root");
              if (resolved != NULL) {
                  strncpy (real_dev_root, resolved, sizeof(real_dev_root) - 1);
                  real_dev_root[sizeof(real_dev_root) - 1] = '\0';
                  g_free(resolved);
                  goto found;
              }
          }
      }
  strcpy(real_dev_root, "/dev/root");
found:
  return real_dev_root;
}
#endif