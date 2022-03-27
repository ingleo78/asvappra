#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_SIMPLE_ACTION_GROUP_H__
#define __G_SIMPLE_ACTION_GROUP_H__

#include "../gobject/gobject.h"
#include "gactiongroup.h"

G_BEGIN_DECLS
#define G_TYPE_SIMPLE_ACTION_GROUP  (g_simple_action_group_get_type())
#define G_SIMPLE_ACTION_GROUP(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_SIMPLE_ACTION_GROUP, GSimpleActionGroup))
#define G_SIMPLE_ACTION_GROUP_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_SIMPLE_ACTION_GROUP, GSimpleActionGroupClass))
#define G_IS_SIMPLE_ACTION_GROUP(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_SIMPLE_ACTION_GROUP))
#define G_IS_SIMPLE_ACTION_GROUP_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_SIMPLE_ACTION_GROUP))
#define G_SIMPLE_ACTION_GROUP_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_SIMPLE_ACTION_GROUP, GSimpleActionGroupClass))
typedef struct _GSimpleActionGroupPrivate GSimpleActionGroupPrivate;
typedef struct _GSimpleActionGroupClass GSimpleActionGroupClass;
struct _GSimpleActionGroup {
  GObject parent_instance;
  GSimpleActionGroupPrivate *priv;
};
struct _GSimpleActionGroupClass {
  GObjectClass parent_class;
  gpointer padding[12];
};
GType g_simple_action_group_get_type(void) G_GNUC_CONST;
GSimpleActionGroup *g_simple_action_group_new(void);
GAction *g_simple_action_group_lookup(GSimpleActionGroup *simple, const gchar *action_name);
void g_simple_action_group_insert(GSimpleActionGroup *simple, GAction *action);
void g_simple_action_group_remove(GSimpleActionGroup *simple, const gchar *action_name);
G_END_DECLS

#endif