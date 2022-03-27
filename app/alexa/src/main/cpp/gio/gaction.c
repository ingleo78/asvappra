#include "../gobject/gparamspecs.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gaction.h"

G_DEFINE_INTERFACE(GAction, g_action, G_TYPE_OBJECT);
void g_action_default_init(GActionInterface *iface) {
  g_object_interface_install_property(iface,g_param_spec_string("name", P_("Action Name"), P_("The name used to invoke the action"),
                                      NULL,G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_interface_install_property(iface, g_param_spec_boxed("parameter-type", P_("Parameter Type"), P_("The type of GVariant passed to activate()"),
                                      G_TYPE_VARIANT_TYPE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_interface_install_property(iface,g_param_spec_boolean("enabled", P_("Enabled"), P_("If the action can be activated"), TRUE,
                                      G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_interface_install_property(iface, g_param_spec_boxed("state-type", P_("State Type"), P_("The type of the state kept by the action"),
                                      G_TYPE_VARIANT_TYPE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_interface_install_property(iface,g_param_spec_variant("state", P_("State"), P_("The state the action is in"), G_VARIANT_TYPE_ANY,
                                      NULL,G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
void g_action_set_state(GAction *action, GVariant *value) {
  const GVariantType *state_type;
  g_return_if_fail(G_IS_ACTION (action));
  g_return_if_fail(value != NULL);
  state_type = g_action_get_state_type(action);
  g_return_if_fail(state_type != NULL);
  g_return_if_fail(g_variant_is_of_type(value, state_type));
  g_variant_ref_sink(value);
  G_ACTION_GET_IFACE(action)->set_state(action, value);
  g_variant_unref(value);
}
GVariant *g_action_get_state(GAction *action) {
  g_return_val_if_fail(G_IS_ACTION(action), NULL);
  return G_ACTION_GET_IFACE(action)->get_state(action);
}
const gchar *g_action_get_name(GAction *action) {
  g_return_val_if_fail(G_IS_ACTION(action), NULL);
  return G_ACTION_GET_IFACE(action)->get_name(action);
}
const GVariantType *g_action_get_parameter_type(GAction *action) {
  g_return_val_if_fail(G_IS_ACTION(action), NULL);
  return G_ACTION_GET_IFACE(action)->get_parameter_type(action);
}
const GVariantType *g_action_get_state_type(GAction *action) {
  g_return_val_if_fail(G_IS_ACTION(action), NULL);
  return G_ACTION_GET_IFACE(action)->get_state_type(action);
}
GVariant *g_action_get_state_hint(GAction *action) {
  g_return_val_if_fail(G_IS_ACTION(action), NULL);
  return G_ACTION_GET_IFACE(action)->get_state_hint(action);
}
gboolean g_action_get_enabled (GAction *action) {
  g_return_val_if_fail(G_IS_ACTION(action), FALSE);
  return G_ACTION_GET_IFACE(action)->get_enabled(action);
}
void g_action_activate(GAction  *action, GVariant *parameter) {
  g_return_if_fail(G_IS_ACTION(action));
  if (parameter != NULL) g_variant_ref_sink(parameter);
  G_ACTION_GET_IFACE (action)->activate(action, parameter);
  if (parameter != NULL) g_variant_unref(parameter);
}