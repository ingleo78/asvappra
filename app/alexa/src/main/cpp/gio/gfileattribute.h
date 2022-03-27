#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_FILE_ATTRIBUTE_H__
#define __G_FILE_ATTRIBUTE_H__

#include "../gobject/gtype.h"
#include "giotypes.h"
#include "gioenums.h"

G_BEGIN_DECLS
struct _GFileAttributeInfo {
  char *name;
  GFileAttributeType type;
  GFileAttributeInfoFlags flags;
};
struct _GFileAttributeInfoList {
  GFileAttributeInfo *infos;
  int n_infos;
};
GType g_file_attribute_info_list_get_type(void);
GFileAttributeInfoList *g_file_attribute_info_list_new(void);
GFileAttributeInfoList *g_file_attribute_info_list_ref(GFileAttributeInfoList *list);
void g_file_attribute_info_list_unref(GFileAttributeInfoList *list);
GFileAttributeInfoList *g_file_attribute_info_list_dup(GFileAttributeInfoList *list);
const GFileAttributeInfo *g_file_attribute_info_list_lookup(GFileAttributeInfoList *list, const char *name);
void g_file_attribute_info_list_add(GFileAttributeInfoList *list, const char *name, GFileAttributeType type, GFileAttributeInfoFlags flags);
G_END_DECLS

#endif