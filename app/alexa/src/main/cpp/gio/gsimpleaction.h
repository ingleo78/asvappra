#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_SIMPLE_ACTION_H__
#define __G_SIMPLE_ACTION_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_SIMPLE_ACTION  (g_simple_action_get_type())
#define G_SIMPLE_ACTION(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_SIMPLE_ACTION, GSimpleAction))
#define G_SIMPLE_ACTION_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_SIMPLE_ACTION, GSimpleActionClass))
#define G_IS_SIMPLE_ACTION(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_SIMPLE_ACTION))
#define G_IS_SIMPLE_ACTION_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_SIMPLE_ACTION))
#define G_SIMPLE_ACTION_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_SIMPLE_ACTION, GSimpleActionClass))
typedef struct _GSimpleActionPrivate GSimpleActionPrivate;
typedef struct _GSimpleActionClass GSimpleActionClass;
struct _GSimpleAction {
  GObject parent_instance;
  GSimpleActionPrivate *priv;
};
struct _GSimpleActionClass {
  GObjectClass parent_class;
  void  (*activate)(GSimpleAction *simple, GVariant *parameter);
  gpointer padding[6];
};
GType g_simple_action_get_type(void) G_GNUC_CONST;
GSimpleAction *g_simple_action_new(const gchar *name, const GVariantType *parameter_type);
GSimpleAction *g_simple_action_new_stateful(const gchar *name, const GVariantType *parameter_type, GVariant *state);
void g_simple_action_set_enabled(GSimpleAction *simple, gboolean enabled);
G_END_DECLS

#endif