#include <string.h>
#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gwin32volumemonitor.h"
#include "gwin32mount.h"
#include "gmount.h"
#include "giomodule.h"
#include "gregistrysettingsbackend.h"

#define _WIN32_WINNT 0x0500
struct _GWin32VolumeMonitor {
  GNativeVolumeMonitor parent;
  GList *volumes;
  GList *mounts;
};
#define g_win32_volume_monitor_get_type _g_win32_volume_monitor_get_type
G_DEFINE_TYPE_WITH_CODE(GWin32VolumeMonitor, g_win32_volume_monitor, G_TYPE_NATIVE_VOLUME_MONITOR,g_io_extension_point_implement(G_NATIVE_VOLUME_MONITOR_EXTENSION_POINT_NAME,
						g_define_type_id, "win32", 0));
static void g_win32_volume_monitor_finalize(GObject *object) {
  GWin32VolumeMonitor *monitor;
  monitor = G_WIN32_VOLUME_MONITOR (object);
  if (G_OBJECT_CLASS(g_win32_volume_monitor_parent_class)->finalize) (*G_OBJECT_CLASS(g_win32_volume_monitor_parent_class)->finalize)(object);
}
static guint32 get_viewable_logical_drives(void) {
  /*guint viewable_drives = GetLogicalDrives();
  HKEY key;
  DWORD var_type = REG_DWORD;
  DWORD no_drives_size = 4;
  DWORD no_drives;
  gboolean hklm_present = FALSE;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", 0, KEY_READ, &key) == ERROR_SUCCESS) {
      if (RegQueryValueEx(key, "NoDrives", NULL, &var_type, (LPBYTE)&no_drives, &no_drives_size) == ERROR_SUCCESS) {
          viewable_drives = viewable_drives & ~no_drives;
          hklm_present = TRUE;
      }
      RegCloseKey(key);
  }
  if (!hklm_present) {
      if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", 0, KEY_READ, &key) == ERROR_SUCCESS) {
          if (RegQueryValueEx(key, "NoDrives", NULL, &var_type, (LPBYTE)&no_drives, &no_drives_size) == ERROR_SUCCESS) {
              viewable_drives = viewable_drives & ~no_drives;
          }
          RegCloseKey(key);
	  }
  }
  return viewable_drives; */
  return 0;
}
static GList *get_mounts(GVolumeMonitor *volume_monitor) {
  GWin32VolumeMonitor *monitor;
  DWORD drives;
  gchar drive[4] = "A:\\";
  GList *list = NULL;
  monitor = G_WIN32_VOLUME_MONITOR(volume_monitor);
  drives = get_viewable_logical_drives();
  if (!drives) g_warning("get_viewable_logical_drives failed.");
  while (drives && drive[0] <= 'Z') {
      if (drives & 1) list = g_list_prepend(list, _g_win32_mount_new(volume_monitor, drive, NULL));
      drives >>= 1;
      drive[0]++;
  }
  return list;
}
static GList *get_volumes(GVolumeMonitor *volume_monitor) {
  GWin32VolumeMonitor *monitor;
  GList *l = NULL;
  monitor = G_WIN32_VOLUME_MONITOR(volume_monitor);
  return l;
}
static GList *get_connected_drives(GVolumeMonitor *volume_monitor) {
  /*GWin32VolumeMonitor *monitor;
  HANDLE find_handle;
  BOOL found;
  wchar_t wc_name[MAX_PATH+1];
  GList *list = NULL;
  monitor = G_WIN32_VOLUME_MONITOR(volume_monitor);
#if 0
  find_handle = FindFirstVolumeW(wc_name, MAX_PATH);
  found = (find_handle != INVALID_HANDLE_VALUE);
  while(found) {
      wchar_t wc_dev_name[MAX_PATH+1];
      guint trailing = wcslen(wc_name) - 1;
      wc_name[trailing] = L'\0';
      if (QueryDosDeviceW(&wc_name[4], wc_dev_name, MAX_PATH)) {
          gchar *name = g_utf16_to_utf8(wc_dev_name, -1, NULL, NULL, NULL);
          g_print("%s\n", name);
          g_free(name);
	  }
      found = FindNextVolumeW(find_handle, wc_name, MAX_PATH);
  }
  if (find_handle != INVALID_HANDLE_VALUE) FindVolumeClose(find_handle);
#endif
  return list;*/
  return NULL;
}
static GVolume *get_volume_for_uuid(GVolumeMonitor *volume_monitor, const char *uuid) {
  return NULL;
}
static GMount *get_mount_for_uuid(GVolumeMonitor *volume_monitor, const char *uuid) {
  return NULL;
}
static gboolean is_supported(void) {
  return TRUE;
}
static GMount *get_mount_for_mount_path(const char *mount_path, GCancellable *cancellable) {
  GWin32Mount *mount;
  mount = _g_win32_mount_new(NULL, mount_path, NULL);
  return G_MOUNT(mount);
}
static void g_win32_volume_monitor_class_init(GWin32VolumeMonitorClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GVolumeMonitorClass *monitor_class = G_VOLUME_MONITOR_CLASS(klass);
  GNativeVolumeMonitorClass *native_class = G_NATIVE_VOLUME_MONITOR_CLASS(klass);
  gobject_class->finalize = g_win32_volume_monitor_finalize;
  monitor_class->get_mounts = get_mounts;
  monitor_class->get_volumes = get_volumes;
  monitor_class->get_connected_drives = get_connected_drives;
  monitor_class->get_volume_for_uuid = get_volume_for_uuid;
  monitor_class->get_mount_for_uuid = get_mount_for_uuid;
  monitor_class->is_supported = is_supported;
  native_class->get_mount_for_mount_path = get_mount_for_mount_path;
}
static void g_win32_volume_monitor_init(GWin32VolumeMonitor *win32_monitor) {
#if 0
  unix_monitor->mount_monitor = g_win32_mount_monitor_new();
  g_signal_connect(win32_monitor->mount_monitor, "mounts-changed", G_CALLBACK(mounts_changed), win32_monitor);
  g_signal_connect(win32_monitor->mount_monitor, "mountpoints-changed", G_CALLBACK(mountpoints_changed), win32_monitor);
  update_volumes(win32_monitor);
  update_mounts(win32_monitor);
#endif
}