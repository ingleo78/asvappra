#ifndef __G_FILE_DESCRIPTOR_BASED_H__
#define __G_FILE_DESCRIPTOR_BASED_H__

#include "../glib/glib.h"
#include "../gobject/gtype.h"
#include "gio.h"
#include "gioenums.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_FILE_DESCRIPTOR_BASED  (g_file_descriptor_based_get_type())
#define G_FILE_DESCRIPTOR_BASED(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_FILE_DESCRIPTOR_BASED, GFileDescriptorBased))
#define G_IS_FILE_DESCRIPTOR_BASED(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_FILE_DESCRIPTOR_BASED))
#define G_FILE_DESCRIPTOR_BASED_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_FILE_DESCRIPTOR_BASED, GFileDescriptorBasedIface))
typedef struct _GFileDescriptorBasedIface   GFileDescriptorBasedIface;
struct _GFileDescriptorBasedIface {
  GTypeInterface g_iface;
  int (*get_fd)(GFileDescriptorBased *fd_based);
};
GType g_file_descriptor_based_get_type(void) G_GNUC_CONST;
int g_file_descriptor_based_get_fd(GFileDescriptorBased *fd_based);
G_END_DECLS

#endif