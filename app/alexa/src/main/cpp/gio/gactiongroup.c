#include "../glib/glibintl.h"
#include "config.h"
#include "gactiongroup.h"
#include "gaction.h"
#include "../gobject/gsignal.h"

G_DEFINE_INTERFACE (GActionGroup, g_action_group, G_TYPE_OBJECT)
enum {
  SIGNAL_ACTION_ADDED,
  SIGNAL_ACTION_REMOVED,
  SIGNAL_ACTION_ENABLED_CHANGED,
  SIGNAL_ACTION_STATE_CHANGED,
  NR_SIGNALS
};
static guint g_action_group_signals[NR_SIGNALS];
static void g_action_group_default_init(GActionGroupInterface *class) {
  g_action_group_signals[SIGNAL_ACTION_ADDED] = g_signal_new(I_("action-added"), G_TYPE_ACTION_GROUP, G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                                                             G_STRUCT_OFFSET(GActionGroupInterface, action_added), NULL, NULL, g_cclosure_marshal_VOID__STRING,
                                                             G_TYPE_NONE, 1, G_TYPE_STRING);
  g_action_group_signals[SIGNAL_ACTION_REMOVED] = g_signal_new(I_("action-removed"), G_TYPE_ACTION_GROUP, G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                                                               G_STRUCT_OFFSET(GActionGroupInterface, action_removed), NULL, NULL, g_cclosure_marshal_VOID__STRING,
                                                               G_TYPE_NONE, 1, G_TYPE_STRING);
  g_action_group_signals[SIGNAL_ACTION_ENABLED_CHANGED] = g_signal_new(I_("action-enabled-changed"), G_TYPE_ACTION_GROUP, G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                                                                       G_STRUCT_OFFSET(GActionGroupInterface, action_enabled_changed), NULL, NULL,
                                                                       NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_BOOLEAN);
  g_action_group_signals[SIGNAL_ACTION_STATE_CHANGED] = g_signal_new(I_("action-state-changed"), G_TYPE_ACTION_GROUP, G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                                                                     G_STRUCT_OFFSET (GActionGroupInterface, action_state_changed), NULL, NULL,
                                                                     NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_VARIANT);
}
gchar **g_action_group_list_actions (GActionGroup *action_group) {
  g_return_val_if_fail(G_IS_ACTION_GROUP(action_group), NULL);
  return G_ACTION_GROUP_GET_IFACE(action_group)->list_actions (action_group);
}
gboolean g_action_group_has_action(GActionGroup *action_group, const gchar *action_name) {
  g_return_val_if_fail(G_IS_ACTION_GROUP(action_group), FALSE);
  return G_ACTION_GROUP_GET_IFACE(action_group)->has_action(action_group, action_name);
}
const GVariantType *g_action_group_get_action_parameter_type(GActionGroup *action_group, const gchar  *action_name) {
  g_return_val_if_fail(G_IS_ACTION_GROUP (action_group), NULL);
  return G_ACTION_GROUP_GET_IFACE (action_group)->get_action_parameter_type (action_group, action_name);
}
const GVariantType *g_action_group_get_action_state_type(GActionGroup *action_group, const gchar  *action_name) {
  g_return_val_if_fail(G_IS_ACTION_GROUP(action_group), NULL);
  return G_ACTION_GROUP_GET_IFACE(action_group)->get_action_state_type(action_group, action_name);
}
GVariant *g_action_group_get_action_state_hint(GActionGroup *action_group, const gchar  *action_name) {
  g_return_val_if_fail(G_IS_ACTION_GROUP(action_group), NULL);
  return G_ACTION_GROUP_GET_IFACE(action_group)->get_action_state_hint(action_group, action_name);
}
gboolean g_action_group_get_action_enabled(GActionGroup *action_group, const gchar *action_name) {
  g_return_val_if_fail(G_IS_ACTION_GROUP(action_group), FALSE);
  return G_ACTION_GROUP_GET_IFACE(action_group)->get_action_enabled(action_group, action_name);
}
GVariant *g_action_group_get_action_state(GActionGroup *action_group, const gchar *action_name) {
  g_return_val_if_fail(G_IS_ACTION_GROUP(action_group), NULL);
  return G_ACTION_GROUP_GET_IFACE(action_group)->get_action_state(action_group, action_name);
}
void g_action_group_change_action_state(GActionGroup *action_group, const gchar *action_name, GVariant *value) {
  g_return_if_fail(G_IS_ACTION_GROUP(action_group));
  g_return_if_fail(action_name != NULL);
  g_return_if_fail(value != NULL);
  G_ACTION_GROUP_GET_IFACE(action_group)->change_action_state(action_group, action_name, value);
}
void g_action_group_activate_action(GActionGroup *action_group, const gchar  *action_name, GVariant *parameter) {
  g_return_if_fail(G_IS_ACTION_GROUP(action_group));
  g_return_if_fail(action_name != NULL);
  G_ACTION_GROUP_GET_IFACE(action_group)->activate_action(action_group, action_name, parameter);
}
void g_action_group_action_added(GActionGroup *action_group, const gchar *action_name) {
  g_return_if_fail(G_IS_ACTION_GROUP(action_group));
  g_return_if_fail(action_name != NULL);
  g_signal_emit(action_group, g_action_group_signals[SIGNAL_ACTION_ADDED], g_quark_try_string(action_name), action_name);
}
void
g_action_group_action_removed(GActionGroup *action_group, const gchar *action_name) {
  g_return_if_fail(G_IS_ACTION_GROUP(action_group));
  g_return_if_fail(action_name != NULL);
  g_signal_emit(action_group, g_action_group_signals[SIGNAL_ACTION_REMOVED], g_quark_try_string(action_name), action_name);
}
void g_action_group_action_enabled_changed(GActionGroup *action_group, const gchar *action_name, gboolean enabled) {
  g_return_if_fail(G_IS_ACTION_GROUP (action_group));
  g_return_if_fail(action_name != NULL);
  enabled = !!enabled;
  g_signal_emit(action_group, g_action_group_signals[SIGNAL_ACTION_ENABLED_CHANGED], g_quark_try_string(action_name), action_name, enabled);
}
void g_action_group_action_state_changed(GActionGroup *action_group, const gchar *action_name, GVariant *state) {
  g_return_if_fail(G_IS_ACTION_GROUP(action_group));
  g_return_if_fail(action_name != NULL);
  g_signal_emit(action_group, g_action_group_signals[SIGNAL_ACTION_STATE_CHANGED], g_quark_try_string(action_name), action_name, state);
}