#ifndef __G_WIN32_DIRECTORY_MONITOR_H__
#define __G_WIN32_DIRECTORY_MONITOR_H__

#include <stdlib.h>
#include <string.h>
#include "../../glib/glib.h"
#include "../../glib/glib-object.h"
#include "../../gobject/gtype.h"
#include "../gio.h"
#include "../glocaldirectorymonitor.h"
#include "../giomodule.h"
#include "../giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_WIN32_DIRECTORY_MONITOR (g_win32_directory_monitor_get_type ())
#define G_WIN32_DIRECTORY_MONITOR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_WIN32_DIRECTORY_MONITOR, GWin32DirectoryMonitor))
#define G_WIN32_DIRECTORY_MONITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), G_TYPE_WIN32_DIRECTORY_MONITOR, GWin32DirectoryMonitorClass))
#define G_IS_WIN32_DIRECTORY_MONITOR(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_WIN32_DIRECTORY_MONITOR))
#define G_IS_WIN32_DIRECTORY_MONITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), G_TYPE_WIN32_DIRECTORY_MONITOR))
#define G_WIN32_DIRECTORY_MONITOR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), G_TYPE_WIN32_DIRECTORY_MONITOR, GWin32DirectoryMonitorClass))
typedef struct _GWin32DirectoryMonitor GWin32DirectoryMonitor;
typedef struct _GWin32DirectoryMonitorClass GWin32DirectoryMonitorClass;
typedef struct _GWin32DirectoryMonitorPrivate GWin32DirectoryMonitorPrivate;
struct _GWin32DirectoryMonitor {
  GLocalDirectoryMonitor parent_instance;
  GWin32DirectoryMonitorPrivate *priv;
};
struct _GWin32DirectoryMonitorClass {
  GLocalDirectoryMonitorClass parent_class;
};
GType g_win32_directory_monitor_get_type(void);
void g_win32_directory_monitor_register(GIOModule *module);
G_END_DECLS

#endif