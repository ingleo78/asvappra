#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_FILE_ICON_H__
#define __G_FILE_ICON_H__

#include "../gobject/gtype.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_FILE_ICON  (g_file_icon_get_type())
#define G_FILE_ICON(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_FILE_ICON, GFileIcon))
#define G_FILE_ICON_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_FILE_ICON, GFileIconClass))
#define G_IS_FILE_ICON(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_FILE_ICON))
#define G_IS_FILE_ICON_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_FILE_ICON))
#define G_FILE_ICON_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_FILE_ICON, GFileIconClass))
typedef struct _GFileIconClass GFileIconClass;
GType g_file_icon_get_type(void) G_GNUC_CONST;
GIcon *g_file_icon_new(GFile *file);
GFile *g_file_icon_get_file(GFileIcon *icon);
G_END_DECLS

#endif