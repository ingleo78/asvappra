#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "config.h"
#include "gseekable.h"

typedef GSeekableIface GSeekableInterface;
G_DEFINE_INTERFACE(GSeekable, g_seekable, G_TYPE_OBJECT);
static void g_seekable_default_init(GSeekableInterface *iface) {}
goffset g_seekable_tell(GSeekable *seekable) {
  GSeekableIface *iface;
  g_return_val_if_fail(G_IS_SEEKABLE(seekable), 0);
  iface = G_SEEKABLE_GET_IFACE(seekable);
  return (*iface->tell)(seekable);
}
gboolean g_seekable_can_seek(GSeekable *seekable) {
  GSeekableIface *iface;
  g_return_val_if_fail(G_IS_SEEKABLE(seekable), FALSE);
  iface = G_SEEKABLE_GET_IFACE(seekable);
  return (*iface->can_seek)(seekable);
}
gboolean g_seekable_seek(GSeekable *seekable, goffset offset, GSeekType type, GCancellable *cancellable, GError **error) {
  GSeekableIface *iface;
  g_return_val_if_fail(G_IS_SEEKABLE(seekable), FALSE);
  iface = G_SEEKABLE_GET_IFACE(seekable);
  return (*iface->seek)(seekable, offset, type, cancellable, error);
}
gboolean g_seekable_can_truncate(GSeekable *seekable) {
  GSeekableIface *iface;
  g_return_val_if_fail(G_IS_SEEKABLE(seekable), FALSE);
  iface = G_SEEKABLE_GET_IFACE(seekable);
  return (*iface->can_truncate)(seekable);
}
gboolean g_seekable_truncate(GSeekable *seekable, goffset offset, GCancellable *cancellable, GError **error) {
  GSeekableIface *iface;
  g_return_val_if_fail(G_IS_SEEKABLE(seekable), FALSE);
  iface = G_SEEKABLE_GET_IFACE(seekable);
  return (*iface->truncate_fn)(seekable, offset, cancellable, error);
}