#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/gutils.h"
#include "config.h"
#include "gvfs.h"
#include "glocalvfs.h"
#include "giomodule-priv.h"

G_DEFINE_TYPE(GVfs, g_vfs, G_TYPE_OBJECT);
static void g_vfs_class_init(GVfsClass *klass) {}
static void g_vfs_init(GVfs *vfs) {}
gboolean g_vfs_is_active(GVfs *vfs) {
  GVfsClass *class;
  g_return_val_if_fail(G_IS_VFS(vfs), FALSE);
  class = G_VFS_GET_CLASS(vfs);
  return (*class->is_active)(vfs);
}
GFile *g_vfs_get_file_for_path(GVfs *vfs, const char *path) {
  GVfsClass *class;
  g_return_val_if_fail(G_IS_VFS(vfs), NULL);
  g_return_val_if_fail(path != NULL, NULL);
  class = G_VFS_GET_CLASS (vfs);
  return (*class->get_file_for_path)(vfs, path);
}
GFile *g_vfs_get_file_for_uri(GVfs *vfs, const char *uri) {
  GVfsClass *class;
  g_return_val_if_fail(G_IS_VFS(vfs), NULL);
  g_return_val_if_fail(uri != NULL, NULL);
  class = G_VFS_GET_CLASS(vfs);
  return (*class->get_file_for_uri)(vfs, uri);
}
const gchar * const *g_vfs_get_supported_uri_schemes(GVfs *vfs) {
  GVfsClass *class;
  g_return_val_if_fail(G_IS_VFS(vfs), NULL);
  class = G_VFS_GET_CLASS(vfs);
  return (*class->get_supported_uri_schemes)(vfs);
}
GFile *g_vfs_parse_name(GVfs *vfs, const char *parse_name) {
  GVfsClass *class;
  g_return_val_if_fail(G_IS_VFS(vfs), NULL);
  g_return_val_if_fail(parse_name != NULL, NULL);
  class = G_VFS_GET_CLASS(vfs);
  return (*class->parse_name)(vfs, parse_name);
}
static gpointer get_default_vfs(gpointer arg) {
  const char *use_this;
  GVfs *vfs;
  GList *l;
  GIOExtensionPoint *ep;
  GIOExtension *extension;
  use_this = g_getenv("GIO_USE_VFS");
  _g_io_modules_ensure_loaded();
  ep = g_io_extension_point_lookup(G_VFS_EXTENSION_POINT_NAME);
  if (use_this) {
      extension = g_io_extension_point_get_extension_by_name(ep, use_this);
      if (extension) {
          vfs = g_object_new(g_io_extension_get_type(extension), NULL);
          if (g_vfs_is_active(vfs)) return vfs;
          g_object_unref(vfs);
	  }
  }
  for (l = g_io_extension_point_get_extensions(ep); l != NULL; l = l->next) {
      extension = l->data;
      vfs = g_object_new(g_io_extension_get_type(extension), NULL);
      if (g_vfs_is_active(vfs)) return vfs;
      g_object_unref(vfs);
  }
  return NULL;
}
GVfs *g_vfs_get_default(void) {
  static GOnce once_init = G_ONCE_INIT;
  return g_once(&once_init, get_default_vfs, NULL);
}
GVfs *g_vfs_get_local(void) {
  static gsize vfs = 0;
  if (g_once_init_enter(&vfs)) g_once_init_leave(&vfs, (gsize)_g_local_vfs_new());
  return G_VFS(vfs);
}