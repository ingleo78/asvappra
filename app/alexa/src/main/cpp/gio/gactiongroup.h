#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_ACTION_GROUP_H__
#define __G_ACTION_GROUP_H__

#include "../gobject/gtype.h"
#include "../glib/gvariant.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_ACTION_GROUP  (g_action_group_get_type ())
#define G_ACTION_GROUP(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_ACTION_GROUP, GActionGroup))
#define G_IS_ACTION_GROUP(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_ACTION_GROUP))
#define G_ACTION_GROUP_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), G_TYPE_ACTION_GROUP, GActionGroupInterface))
typedef struct _GActionGroupInterface GActionGroupInterface;
struct _GActionGroupInterface {
  GTypeInterface g_iface;
  gboolean (* has_action)(GActionGroup *action_group, const gchar *action_name);
  gchar **(*list_actions)(GActionGroup *action_group);
  gboolean (* get_action_enabled)(GActionGroup *action_group, const gchar *action_name);
  const GVariantType *(*get_action_parameter_type)(GActionGroup *action_group, const gchar *action_name);
  const GVariantType *(*get_action_state_type)(GActionGroup *action_group, const gchar *action_name);
  GVariant *(*get_action_state_hint)(GActionGroup *action_group, const gchar *action_name);
  GVariant *(* get_action_state)(GActionGroup *action_group, const gchar *action_name);
  void (*change_action_state)(GActionGroup *action_group, const gchar *action_name, GVariant *value);
  void (*activate_action)(GActionGroup *action_group, const gchar *action_name, GVariant *parameter);
  void (*action_added)(GActionGroup *action_group, const gchar *action_name);
  void (*action_removed)(GActionGroup *action_group, const gchar *action_name);
  void (*action_enabled_changed)(GActionGroup *action_group, const gchar *action_name, gboolean enabled);
  void (*action_state_changed)(GActionGroup *action_group, const gchar *action_name, GVariant *value);
};
GType g_action_group_get_type(void) G_GNUC_CONST;
gboolean g_action_group_has_action(GActionGroup *action_group, const gchar *action_name);
gchar **g_action_group_list_actions(GActionGroup *action_group);
const GVariantType *g_action_group_get_action_parameter_type(GActionGroup *action_group, const gchar *action_name);
const GVariantType *g_action_group_get_action_state_type(GActionGroup *action_group, const gchar *action_name);
GVariant *g_action_group_get_action_state_hint(GActionGroup *action_group, const gchar *action_name);
gboolean g_action_group_get_action_enabled(GActionGroup *action_group, const gchar *action_name);
GVariant *g_action_group_get_action_state(GActionGroup *action_group, const gchar *action_name);
void g_action_group_change_action_state(GActionGroup *action_group, const gchar *action_name, GVariant *value);
void g_action_group_activate_action(GActionGroup *action_group, const gchar *action_name, GVariant *parameter);
void g_action_group_action_added(GActionGroup *action_group, const gchar *action_name);
void g_action_group_action_removed(GActionGroup *action_group, const gchar *action_name);
void g_action_group_action_enabled_changed(GActionGroup *action_group, const gchar *action_name, gboolean enabled);
void g_action_group_action_state_changed(GActionGroup *action_group, const gchar *action_name, GVariant *state);
G_END_DECLS

#endif