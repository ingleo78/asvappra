#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_EMBLEMED_ICON_H__
#define __G_EMBLEMED_ICON_H__

#include "gicon.h"
#include "gemblem.h"

G_BEGIN_DECLS
#define G_TYPE_EMBLEMED_ICON  (g_emblemed_icon_get_type())
#define G_EMBLEMED_ICON(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_EMBLEMED_ICON, GEmblemedIcon))
#define G_EMBLEMED_ICON_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_EMBLEMED_ICON, GEmblemedIconClass))
#define G_IS_EMBLEMED_ICON(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_EMBLEMED_ICON))
#define G_IS_EMBLEMED_ICON_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_EMBLEMED_ICON))
#define G_EMBLEMED_ICON_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_EMBLEMED_ICON, GEmblemedIconClass))
typedef struct _GEmblemedIcon GEmblemedIcon;
typedef struct _GEmblemedIconClass GEmblemedIconClass;
typedef struct _GEmblemedIconPrivate GEmblemedIconPrivate;
struct _GEmblemedIcon {
  GObject parent_instance;
  GEmblemedIconPrivate *priv;
};
struct _GEmblemedIconClass {
  GObjectClass parent_class;
};
GType g_emblemed_icon_get_type(void) G_GNUC_CONST;
GIcon *g_emblemed_icon_new(GIcon *icon, GEmblem *emblem);
GIcon *g_emblemed_icon_get_icon(GEmblemedIcon *emblemed);
GList *g_emblemed_icon_get_emblems(GEmblemedIcon *emblemed);
void g_emblemed_icon_add_emblem(GEmblemedIcon *emblemed, GEmblem *emblem);
void g_emblemed_icon_clear_emblems(GEmblemedIcon *emblemed);
G_END_DECLS

#endif