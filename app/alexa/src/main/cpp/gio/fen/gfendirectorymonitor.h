#ifndef __G_FEN_DIRECTORY_MONITOR_H__
#define __G_FEN_DIRECTORY_MONITOR_H__

#include <string.h>
#include "../../glib/glib-object.h"
#include "../../glib/glib.h"
#include "../../gobject/gtype.h"
#include "../glocaldirectorymonitor.h"
#include "../giomodule.h"

G_BEGIN_DECLS
#define G_TYPE_FEN_DIRECTORY_MONITOR  (_g_fen_directory_monitor_get_type ())
#define G_FEN_DIRECTORY_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_FEN_DIRECTORY_MONITOR, GFenDirectoryMonitor))
#define G_FEN_DIRECTORY_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST ((k), G_TYPE_FEN_DIRECTORY_MONITOR, GFenDirectoryMonitorClass))
#define G_IS_FEN_DIRECTORY_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_FEN_DIRECTORY_MONITOR))
#define G_IS_FEN_DIRECTORY_MONITOR_CLASS(k)	 (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_FEN_DIRECTORY_MONITOR))
typedef struct _GFenDirectoryMonitor GFenDirectoryMonitor;
typedef struct _GFenDirectoryMonitorClass GFenDirectoryMonitorClass;
struct _GFenDirectoryMonitorClass {
  GLocalDirectoryMonitorClass parent_class;
};
GType _g_fen_directory_monitor_get_type (void) G_GNUC_CONST;
G_END_DECLS

#endif