#ifndef __G_WINHTTP_FILE_H__
#define __G_WINHTTP_FILE_H__

#include "../../gobject/gobject.h"
#include "../giotypes.h"
#include "gwinhttpvfs.h"

G_BEGIN_DECLS
#define G_TYPE_WINHTTP_FILE  (_g_winhttp_file_get_type ())
#define G_WINHTTP_FILE(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_WINHTTP_FILE, GWinHttpFile))
#define G_WINHTTP_FILE_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_WINHTTP_FILE, GWinHttpFileClass))
#define G_IS_WINHTTP_FILE(o)  (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_WINHTTP_FILE))
#define G_IS_WINHTTP_FILE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_WINHTTP_FILE))
#define G_WINHTTP_FILE_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), G_TYPE_WINHTTP_FILE, GWinHttpFileClass))
typedef struct _GWinHttpFile GWinHttpFile;
typedef struct _GWinHttpFileClass GWinHttpFileClass;
struct _GWinHttpFile {
  GObject parent_instance;
  GWinHttpVfs *vfs;
  URL_COMPONENTS url;
};
struct _GWinHttpFileClass {
  GObjectClass parent_class;
};
GType _g_winhttp_file_get_type (void) G_GNUC_CONST;
GFile * _g_winhttp_file_new (GWinHttpVfs *vfs, const char *uri);
G_END_DECLS

#endif