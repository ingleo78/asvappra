#include "../glib/glibintl.h"
#include "config.h"
#include "gfiledescriptorbased.h"

typedef GFileDescriptorBasedIface GFileDescriptorBasedInterface;
G_DEFINE_INTERFACE(GFileDescriptorBased, g_file_descriptor_based, G_TYPE_OBJECT);
static void g_file_descriptor_based_default_init (GFileDescriptorBasedInterface *iface) {}
int g_file_descriptor_based_get_fd(GFileDescriptorBased *fd_based) {
  GFileDescriptorBasedIface *iface;
  g_return_val_if_fail (G_IS_FILE_DESCRIPTOR_BASED (fd_based), 0);
  iface = G_FILE_DESCRIPTOR_BASED_GET_IFACE (fd_based);
  return (* iface->get_fd) (fd_based);
}