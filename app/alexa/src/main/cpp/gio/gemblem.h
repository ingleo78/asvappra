#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_EMBLEM_H__
#define __G_EMBLEM_H__

#include "../gobject/gobject.h"
#include "gioenums.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_EMBLEM  (g_emblem_get_type())
#define G_EMBLEM(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_EMBLEM, GEmblem))
#define G_EMBLEM_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_EMBLEM, GEmblemClass))
#define G_IS_EMBLEM(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_EMBLEM))
#define G_IS_EMBLEM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_EMBLEM))
#define G_EMBLEM_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_EMBLEM, GEmblemClass))
typedef struct _GEmblem GEmblem;
typedef struct _GEmblemClass GEmblemClass;
GType g_emblem_get_type(void) G_GNUC_CONST;
GEmblem *g_emblem_new(GIcon *icon);
GEmblem *g_emblem_new_with_origin(GIcon *icon, GEmblemOrigin  origin);
GIcon *g_emblem_get_icon(GEmblem *emblem);
GEmblemOrigin g_emblem_get_origin(GEmblem *emblem);
G_END_DECLS

#endif