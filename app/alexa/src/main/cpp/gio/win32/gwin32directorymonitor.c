#define _WIN32_WINNT 0x0400
#include "../config.h"
#include "../gregistrysettingsbackend.h"
#include "gwin32directorymonitor.h"
#include "winhttp.h"

G_DEFINE_TYPE_WITH_CODE(GWin32DirectoryMonitor, g_win32_directory_monitor, G_TYPE_LOCAL_DIRECTORY_MONITOR,
			       g_io_extension_point_implement(G_LOCAL_DIRECTORY_MONITOR_EXTENSION_POINT_NAME, g_define_type_id, "readdirectorychanges", 20))
struct _GWin32DirectoryMonitorPrivate {
  //OVERLAPPED overlapped;
  DWORD buffer_allocated_bytes;
  gchar *file_notify_buffer;
  DWORD buffer_filled_bytes;
  HANDLE hDirectory;
  GFileMonitor *self;
};
static void g_win32_directory_monitor_finalize(GObject *base);
static gboolean g_win32_directory_monitor_cancel(GFileMonitor *base);
static GObject *g_win32_directory_monitor_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_properties);
static gboolean g_win32_directory_monitor_is_supported(void) {
  return TRUE;
}
static void g_win32_directory_monitor_finalize(GObject *base) {
  /*GWin32DirectoryMonitor *self;
  self = G_WIN32_DIRECTORY_MONITOR(base);
  if (self->priv->hDirectory == INVALID_HANDLE_VALUE) {
      g_free(self->priv->file_notify_buffer);
      self->priv->file_notify_buffer = NULL;
      g_free(self->priv);
  } else self->priv->self = NULL;
  if (G_OBJECT_CLASS(g_win32_directory_monitor_parent_class)->finalize) (*G_OBJECT_CLASS(g_win32_directory_monitor_parent_class)->finalize)(base);*/
}
static gboolean g_win32_directory_monitor_cancel(GFileMonitor *base) {
  /*GWin32DirectoryMonitor *self;
  self = G_WIN32_DIRECTORY_MONITOR (base);
  if (self->priv->hDirectory != INVALID_HANDLE_VALUE) CloseHandle (self->priv->hDirectory);
  if (G_FILE_MONITOR_CLASS(g_win32_directory_monitor_parent_class)->cancel) (*G_FILE_MONITOR_CLASS(g_win32_directory_monitor_parent_class)->cancel)(base);*/
  return TRUE;
}
/*static CALLBACK void CALLBACK g_win32_directory_monitor_callback(DWORD error, DWORD nBytes, LPOVERLAPPED lpOverlapped) {
  gulong offset;
  PFILE_NOTIFY_INFORMATION pfile_notify_walker;
  glong file_name_len;
  gchar *file_name;
  gchar *path;
  GFile *file;
  GWin32DirectoryMonitorPrivate *priv = (GWin32DirectoryMonitorPrivate*)lpOverlapped;
  static GFileMonitorEvent events[] = {
      0, 
      G_FILE_MONITOR_EVENT_CREATED,
      G_FILE_MONITOR_EVENT_DELETED,
      G_FILE_MONITOR_EVENT_CHANGED,
      G_FILE_MONITOR_EVENT_DELETED,
      G_FILE_MONITOR_EVENT_CREATED
  };
  if (priv->self == NULL || g_file_monitor_is_cancelled (priv->self) || priv->file_notify_buffer == NULL) {
      g_free(priv->file_notify_buffer);
      g_free(priv);
      return;
  }
  offset = 0;
  do {
      pfile_notify_walker = (PFILE_NOTIFY_INFORMATION)(priv->file_notify_buffer + offset);
      if (pfile_notify_walker->Action > 0) {
          file_name = g_utf16_to_utf8 (pfile_notify_walker->FileName, pfile_notify_walker->FileNameLength / sizeof(WCHAR), NULL, &file_name_len, NULL);
          path = g_build_filename(G_LOCAL_DIRECTORY_MONITOR (priv->self)->dirname, file_name, NULL);
          file = g_file_new_for_path (path);
          g_file_monitor_emit_event (priv->self, file, NULL, events [pfile_notify_walker->Action]);
          g_object_unref (file);
          g_free (path);
          g_free (file_name);
      }
      offset += pfile_notify_walker->NextEntryOffset;
  } while (pfile_notify_walker->NextEntryOffset);
  ReadDirectoryChangesW(priv->hDirectory, (gpointer)priv->file_notify_buffer, priv->buffer_allocated_bytes, FALSE, FILE_NOTIFY_CHANGE_FILE_NAME |
			            FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE, &priv->buffer_filled_bytes,
			            &priv->overlapped, g_win32_directory_monitor_callback);
}*/
static GObject *g_win32_directory_monitor_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_properties) {
  /*GObject *obj;
  GWin32DirectoryMonitorClass *klass;
  GObjectClass *parent_class;
  GWin32DirectoryMonitor *self;
  wchar_t *wdirname;
  gboolean result;
  klass = G_WIN32_DIRECTORY_MONITOR_CLASS (g_type_class_peek (G_TYPE_WIN32_DIRECTORY_MONITOR));
  parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
  obj = parent_class->constructor (type, n_construct_properties, construct_properties);
  self = G_WIN32_DIRECTORY_MONITOR (obj);
  wdirname = g_utf8_to_utf16 (G_LOCAL_DIRECTORY_MONITOR (obj)->dirname, -1, NULL, NULL, NULL);
  self->priv->hDirectory = CreateFileW(wdirname, FILE_LIST_DIRECTORY, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
					                   FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
  g_free (wdirname);
  if (self->priv->hDirectory == INVALID_HANDLE_VALUE) return obj;
  result = ReadDirectoryChangesW(self->priv->hDirectory, (gpointer)self->priv->file_notify_buffer, self->priv->buffer_allocated_bytes, FALSE,
				                 FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE,
				                 &self->priv->buffer_filled_bytes, &self->priv->overlapped, g_win32_directory_monitor_callback);
  return obj;*/
  return NULL;
}
static void g_win32_directory_monitor_class_init(GWin32DirectoryMonitorClass *klass) {
  /*g_win32_directory_monitor_parent_class = g_type_class_peek_parent(klass);
  G_OBJECT_CLASS(klass)->constructor = g_win32_directory_monitor_constructor;
  G_OBJECT_CLASS(klass)->finalize = g_win32_directory_monitor_finalize;
  G_FILE_MONITOR_CLASS(klass)->cancel = g_win32_directory_monitor_cancel;
  G_LOCAL_DIRECTORY_MONITOR_CLASS(klass)->mount_notify = FALSE;
  G_LOCAL_DIRECTORY_MONITOR_CLASS(klass)->is_supported = g_win32_directory_monitor_is_supported;*/
}
static void g_win32_directory_monitor_init (GWin32DirectoryMonitor *self) {
  /*self->priv = (GWin32DirectoryMonitorPrivate*)g_new0 (GWin32DirectoryMonitorPrivate, 1);
  g_assert(self->priv != 0);
  self->priv->buffer_allocated_bytes = 32768;
  self->priv->file_notify_buffer = g_new0(gchar, self->priv->buffer_allocated_bytes);
  g_assert(self->priv->file_notify_buffer);
  self->priv->self = G_FILE_MONITOR(self);*/
}