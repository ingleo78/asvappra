#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_ICON_H__
#define __G_ICON_H__

#include "../glib/garray.h"
#include "../gobject/gtype.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_ICON  (g_icon_get_type())
#define G_ICON(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), G_TYPE_ICON, GIcon))
#define G_IS_ICON(obj)	(G_TYPE_CHECK_INSTANCE_TYPE((obj), G_TYPE_ICON))
#define G_ICON_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_ICON, GIconIface))
typedef struct _GIconIface GIconIface;
struct _GIconIface {
  GTypeInterface g_iface;
  guint (*hash)(GIcon *icon);
  gboolean (*equal)(GIcon *icon1, GIcon *icon2);
  gboolean (*to_tokens)(GIcon *icon, GPtrArray *tokens, gint *out_version);
  GIcon *(*from_tokens)(gchar **tokens, gint num_tokens, gint version, GError **error);
};
GType g_icon_get_type(void) G_GNUC_CONST;
guint g_icon_hash(gconstpointer icon);
gboolean g_icon_equal(GIcon *icon1, GIcon *icon2);
gchar *g_icon_to_string(GIcon *icon);
GIcon *g_icon_new_for_string(const gchar *str, GError **error);
G_END_DECLS

#endif