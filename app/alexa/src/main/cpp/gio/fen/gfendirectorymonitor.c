#include "../config.h"
#include "../giomodule.h"
#include "gfendirectorymonitor.h"
#include "fen-helper.h"

struct _GFenDirectoryMonitor {
    GLocalDirectoryMonitor parent_instance;
    gboolean enabled;
};
static gboolean g_fen_directory_monitor_cancel (GFileMonitor* monitor);
#define g_fen_directory_monitor_get_type _g_fen_directory_monitor_get_type
//G_DEFINE_TYPE_WITH_CODE(GFenDirectoryMonitor, g_fen_directory_monitor, G_TYPE_LOCAL_DIRECTORY_MONITOR, g_io_extension_point_implement(G_LOCAL_DIRECTORY_MONITOR_EXTENSION_POINT_NAME, g_define_type_id,"fen", 20))
static  void g_fen_directory_monitor_finalize (GObject *object) {
    GFenDirectoryMonitor *self = G_FEN_DIRECTORY_MONITOR (object);
    if (self->enabled) {
        fen_remove (G_LOCAL_DIRECTORY_MONITOR (self)->dirname, self, TRUE);
        self->enabled = FALSE;
    }
    //if (G_OBJECT_CLASS (g_fen_directory_monitor_parent_class)->finalize) (*G_OBJECT_CLASS (g_fen_directory_monitor_parent_class)->finalize) (object);
}
static GObject *g_fen_directory_monitor_constructor (GType type, guint n_construct_properties, GObjectConstructParam *construct_properties) {
    GObject *obj = NULL;
    /*GFenDirectoryMonitorClass *klass;
    GObjectClass *parent_class;
    GFenDirectoryMonitor *self;
    const gchar *dirname = NULL;
    klass = G_FEN_DIRECTORY_MONITOR_CLASS(g_type_class_peek(G_TYPE_FEN_DIRECTORY_MONITOR));
    parent_class = g_fen_directory_monitor_parent_class;
    obj = parent_class->constructor(type, n_construct_properties, construct_properties);
    self = G_FEN_DIRECTORY_MONITOR(obj);
    dirname = G_LOCAL_DIRECTORY_MONITOR(self)->dirname;
    g_assert(dirname != NULL);
    if (!fen_init()) g_assert_not_reached();
    fen_add(dirname, self, TRUE);
    self->enabled = TRUE;*/
    return obj;
}
static gboolean g_fen_directory_monitor_is_supported(void) {
    return fen_init();
}
static void g_fen_directory_monitor_class_init(GFenDirectoryMonitorClass* klass) {
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    GFileMonitorClass *directory_monitor_class = G_FILE_MONITOR_CLASS(klass);
    GLocalDirectoryMonitorClass *local_directory_monitor_class = G_LOCAL_DIRECTORY_MONITOR_CLASS(klass);
    gobject_class->finalize = g_fen_directory_monitor_finalize;
    gobject_class->constructor = g_fen_directory_monitor_constructor;
    directory_monitor_class->cancel = g_fen_directory_monitor_cancel;
    local_directory_monitor_class->mount_notify = TRUE;
    local_directory_monitor_class->is_supported = g_fen_directory_monitor_is_supported;
}
static void g_fen_directory_monitor_init(GFenDirectoryMonitor* monitor) {}
static gboolean g_fen_directory_monitor_cancel(GFileMonitor* monitor) {
    GFenDirectoryMonitor *self = G_FEN_DIRECTORY_MONITOR(monitor);
    if (self->enabled) {
        fen_remove(G_LOCAL_DIRECTORY_MONITOR(self)->dirname, self, TRUE);
        self->enabled = FALSE;
    }
    //if (G_FILE_MONITOR_CLASS(g_fen_directory_monitor_parent_class)->cancel) (*G_FILE_MONITOR_CLASS (g_fen_directory_monitor_parent_class)->cancel)(monitor);
    return TRUE;
}
