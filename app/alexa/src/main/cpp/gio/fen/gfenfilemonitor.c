#include "../config.h"
#include "../giomodule.h"
#include "gfenfilemonitor.h"
#include "fen-helper.h"

struct _GFenFileMonitor {
    GLocalFileMonitor parent_instance;
    gboolean enabled;
};
static gboolean g_fen_file_monitor_cancel (GFileMonitor* monitor);
#define g_fen_file_monitor_get_type _g_fen_file_monitor_get_type
G_DEFINE_TYPE_WITH_CODE(GFenFileMonitor, g_fen_file_monitor, G_TYPE_LOCAL_FILE_MONITOR,g_io_extension_point_implement (G_LOCAL_FILE_MONITOR_EXTENSION_POINT_NAME, g_define_type_id, "fen", 20))
static void g_fen_file_monitor_finalize(GObject *object) {
    GFenFileMonitor *self = G_FEN_FILE_MONITOR(object);
    if (self->enabled) {
        fen_remove (G_LOCAL_FILE_MONITOR (self)->filename, self, FALSE);
        self->enabled = FALSE;
    }
    if (G_OBJECT_CLASS (g_fen_file_monitor_parent_class)->finalize) (*G_OBJECT_CLASS(g_fen_file_monitor_parent_class)->finalize)(object);
}
static GObject *g_fen_file_monitor_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_properties) {
    GObject *obj = NULL;
    GFenFileMonitorClass *klass;
    GObjectClass *parent_class;
    GFenFileMonitor *self;
    const gchar *filename = NULL;
    klass = G_FEN_FILE_MONITOR_CLASS(g_type_class_peek (G_TYPE_FEN_FILE_MONITOR));
    parent_class = g_fen_file_monitor_parent_class;
    obj = parent_class->constructor(type, n_construct_properties, construct_properties);
    self = G_FEN_FILE_MONITOR(obj);
    filename = G_LOCAL_FILE_MONITOR(obj)->filename;
    g_assert(filename != NULL);
    if (!fen_init()) g_assert_not_reached();
    fen_add (filename, self, FALSE);
    self->enabled = TRUE;
    return obj;
}
static gboolean g_fen_file_monitor_is_supported(void) {
    return fen_init ();
}
static void g_fen_file_monitor_class_init(GFenFileMonitorClass* klass) {
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    GFileMonitorClass *file_monitor_class = G_FILE_MONITOR_CLASS(klass);
    GLocalFileMonitorClass *local_file_monitor_class = G_LOCAL_FILE_MONITOR_CLASS(klass);
    gobject_class->finalize = g_fen_file_monitor_finalize;
    gobject_class->constructor = g_fen_file_monitor_constructor;
    file_monitor_class->cancel = g_fen_file_monitor_cancel;
    local_file_monitor_class->is_supported = g_fen_file_monitor_is_supported;
}
static void g_fen_file_monitor_init(GFenFileMonitor* monitor) {}
static gboolean g_fen_file_monitor_cancel(GFileMonitor* monitor) {
    GFenFileMonitor *self = G_FEN_FILE_MONITOR(monitor);
    if (self->enabled) {
        fen_remove(G_LOCAL_FILE_MONITOR(self)->filename, self, FALSE);
        self->enabled = FALSE;
    }
    if (G_FILE_MONITOR_CLASS(g_fen_file_monitor_parent_class)->cancel) (*G_FILE_MONITOR_CLASS(g_fen_file_monitor_parent_class)->cancel)(monitor);
    return TRUE;
}