#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_SRV_TARGET_H__
#define __G_SRV_TARGET_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
GType g_srv_target_get_type(void) G_GNUC_CONST;
#define G_TYPE_SRV_TARGET  (g_srv_target_get_type())
GSrvTarget *g_srv_target_new(const gchar *hostname, guint16 port, guint16 priority, guint16 weight);
GSrvTarget *g_srv_target_copy(GSrvTarget *target);
void g_srv_target_free(GSrvTarget *target);
const gchar *g_srv_target_get_hostname(GSrvTarget *target);
guint16 g_srv_target_get_port(GSrvTarget *target);
guint16 g_srv_target_get_priority(GSrvTarget *target);
guint16 g_srv_target_get_weight(GSrvTarget *target);
GList *g_srv_target_list_sort(GList *targets);
G_END_DECLS

#endif