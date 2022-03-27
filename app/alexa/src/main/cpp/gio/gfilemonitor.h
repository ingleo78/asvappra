#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_FILE_MONITOR_H__
#define __G_FILE_MONITOR_H__

#include "../gobject/gobject.h"
#include "giotypes.h"
#include "gioenums.h"

G_BEGIN_DECLS
#define G_TYPE_FILE_MONITOR  (g_file_monitor_get_type ())
#define G_FILE_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_FILE_MONITOR, GFileMonitor))
#define G_FILE_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_FILE_MONITOR, GFileMonitorClass))
#define G_IS_FILE_MONITOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_FILE_MONITOR))
#define G_IS_FILE_MONITOR_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_FILE_MONITOR))
#define G_FILE_MONITOR_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), G_TYPE_FILE_MONITOR, GFileMonitorClass))
typedef struct _GFileMonitorClass GFileMonitorClass;
typedef struct _GFileMonitorPrivate	GFileMonitorPrivate;
struct _GFileMonitor {
  GObject parent_instance;
  GFileMonitorPrivate *priv;
};
struct _GFileMonitorClass {
  GObjectClass parent_class;
  void (*changed)(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type);
  gboolean (*cancel) (GFileMonitor *monitor);
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
GType g_file_monitor_get_type(void) G_GNUC_CONST;
gboolean g_file_monitor_cancel(GFileMonitor *monitor);
gboolean g_file_monitor_is_cancelled(GFileMonitor *monitor);
void g_file_monitor_set_rate_limit(GFileMonitor *monitor, gint limit_msecs);
void g_file_monitor_emit_event(GFileMonitor *monitor, GFile *child, GFile *other_file, GFileMonitorEvent event_type);
G_END_DECLS

#endif