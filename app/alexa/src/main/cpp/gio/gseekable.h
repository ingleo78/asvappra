#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_SEEKABLE_H__
#define __G_SEEKABLE_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_SEEKABLE  (g_seekable_get_type())
#define G_SEEKABLE(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_SEEKABLE, GSeekable))
#define G_IS_SEEKABLE(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_SEEKABLE))
#define G_SEEKABLE_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_SEEKABLE, GSeekableIface))
typedef struct _GSeekableIface GSeekableIface;
struct _GSeekableIface {
  GTypeInterface g_iface;
  goffset (*tell)(GSeekable *seekable);
  gboolean (*can_seek)(GSeekable *seekable);
  gboolean (*seek)(GSeekable *seekable, goffset offset, GSeekType type, GCancellable *cancellable, GError **error);
  gboolean (*can_truncate)(GSeekable *seekable);
  gboolean (*truncate_fn)(GSeekable *seekable, goffset offset, GCancellable *cancellable, GError **error);
};
GType g_seekable_get_type(void) G_GNUC_CONST;
goffset g_seekable_tell(GSeekable *seekable);
gboolean g_seekable_can_seek(GSeekable *seekable);
gboolean g_seekable_seek(GSeekable *seekable, goffset offset, GSeekType type, GCancellable *cancellable, GError **error);
gboolean g_seekable_can_truncate(GSeekable *seekable);
gboolean g_seekable_truncate(GSeekable *seekable, goffset offset, GCancellable *cancellable, GError **error);
G_END_DECLS

#endif