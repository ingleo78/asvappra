#include "../glib/glib.h"
#include "gsimpleactiongroup.h"
#include "gaction.h"

struct _GSimpleActionGroupPrivate {
  GHashTable *table;
};
static void g_simple_action_group_iface_init (GActionGroupInterface *);
G_DEFINE_TYPE_WITH_CODE(GSimpleActionGroup, g_simple_action_group, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE (G_TYPE_ACTION_GROUP, g_simple_action_group_iface_init));
static gchar **g_simple_action_group_list_actions(GActionGroup *group) {
  GSimpleActionGroup *simple = G_SIMPLE_ACTION_GROUP(group);
  GHashTableIter iter;
  gint n, i = 0;
  gchar **keys;
  gpointer key;
  n = g_hash_table_size(simple->priv->table);
  keys = g_new(gchar *, n + 1);
  g_hash_table_iter_init(&iter, simple->priv->table);
  while (g_hash_table_iter_next(&iter, &key, NULL)) keys[i++] = g_strdup(key);
  g_assert_cmpint(i, ==, n);
  keys[n] = NULL;
  return keys;
}
static gboolean g_simple_action_group_has_action(GActionGroup *group, const gchar *action_name) {
  GSimpleActionGroup *simple = G_SIMPLE_ACTION_GROUP(group);
  return g_hash_table_lookup(simple->priv->table, action_name) != NULL;
}
static const GVariantType *g_simple_action_group_get_parameter_type(GActionGroup *group, const gchar *action_name) {
  GSimpleActionGroup *simple = G_SIMPLE_ACTION_GROUP(group);
  GAction *action;
  action = g_hash_table_lookup(simple->priv->table, action_name);
  if (action == NULL) return NULL;
  return g_action_get_parameter_type(action);
}
static const GVariantType *g_simple_action_group_get_state_type(GActionGroup *group, const gchar *action_name) {
  GSimpleActionGroup *simple = G_SIMPLE_ACTION_GROUP(group);
  GAction *action;
  action = g_hash_table_lookup(simple->priv->table, action_name);
  if (action == NULL) return NULL;
  return g_action_get_state_type(action);
}
static GVariant *g_simple_action_group_get_state_hint (GActionGroup *group, const gchar *action_name) {
  GSimpleActionGroup *simple = G_SIMPLE_ACTION_GROUP(group);
  GAction *action;
  action = g_hash_table_lookup (simple->priv->table, action_name);
  if (action == NULL) return NULL;
  return g_action_get_state_hint(action);
}
static gboolean g_simple_action_group_get_enabled(GActionGroup *group, const gchar *action_name) {
  GSimpleActionGroup *simple = G_SIMPLE_ACTION_GROUP(group);
  GAction *action;
  action = g_hash_table_lookup(simple->priv->table, action_name);
  if (action == NULL) return FALSE;
  return g_action_get_enabled(action);
}
static GVariant *g_simple_action_group_get_state(GActionGroup *group, const gchar *action_name) {
  GSimpleActionGroup *simple = G_SIMPLE_ACTION_GROUP(group);
  GAction *action;
  action = g_hash_table_lookup(simple->priv->table, action_name);
  if (action == NULL) return NULL;
  return g_action_get_state(action);
}
static void g_simple_action_group_set_state(GActionGroup *group, const gchar *action_name, GVariant *value) {
  GSimpleActionGroup *simple = G_SIMPLE_ACTION_GROUP(group);
  GAction *action;
  action = g_hash_table_lookup(simple->priv->table, action_name);
  if (action == NULL) return;
  g_action_set_state(action, value);
}
static void g_simple_action_group_activate(GActionGroup *group, const gchar *action_name, GVariant *parameter) {
  GSimpleActionGroup *simple = G_SIMPLE_ACTION_GROUP(group);
  GAction *action;
  action = g_hash_table_lookup(simple->priv->table, action_name);
  if (action == NULL) return;
  g_action_activate(action, parameter);
}
static void action_enabled_notify(GAction *action, GParamSpec *pspec, gpointer user_data) {
  g_action_group_action_enabled_changed(user_data, g_action_get_name(action), g_action_get_enabled (action));
}
static void action_state_notify(GAction *action, GParamSpec *pspec, gpointer user_data) {
  GVariant *value;
  value = g_action_get_state(action);
  g_action_group_action_state_changed(user_data, g_action_get_name(action), value);
  g_variant_unref(value);
}
static void g_simple_action_group_disconnect(gpointer key, gpointer value, gpointer user_data) {
  g_signal_handlers_disconnect_by_func(value, action_enabled_notify, user_data);
  g_signal_handlers_disconnect_by_func(value, action_state_notify, user_data);
}
static void g_simple_action_group_finalize(GObject *object) {
  GSimpleActionGroup *simple = G_SIMPLE_ACTION_GROUP(object);
  g_hash_table_foreach(simple->priv->table, g_simple_action_group_disconnect, simple);
  g_hash_table_unref(simple->priv->table);
  G_OBJECT_CLASS(g_simple_action_group_parent_class)->finalize(object);
}
static void g_simple_action_group_init(GSimpleActionGroup *simple) {
  simple->priv = G_TYPE_INSTANCE_GET_PRIVATE(simple, G_TYPE_SIMPLE_ACTION_GROUP, GSimpleActionGroupPrivate);
  simple->priv->table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
}
static void g_simple_action_group_class_init(GSimpleActionGroupClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  object_class->finalize = g_simple_action_group_finalize;
  g_type_class_add_private(class, sizeof(GSimpleActionGroupPrivate));
}
static void g_simple_action_group_iface_init(GActionGroupInterface *iface) {
  iface->list_actions = g_simple_action_group_list_actions;
  iface->has_action = g_simple_action_group_has_action;
  iface->get_action_parameter_type = g_simple_action_group_get_parameter_type;
  iface->get_action_state_type = g_simple_action_group_get_state_type;
  iface->get_action_state_hint = g_simple_action_group_get_state_hint;
  iface->get_action_enabled = g_simple_action_group_get_enabled;
  iface->get_action_state = g_simple_action_group_get_state;
  iface->change_action_state = g_simple_action_group_set_state;
  iface->activate_action = g_simple_action_group_activate;
}
GSimpleActionGroup *g_simple_action_group_new(void) {
  return g_object_new(G_TYPE_SIMPLE_ACTION_GROUP, NULL);
}
GAction *g_simple_action_group_lookup(GSimpleActionGroup *simple, const gchar *action_name) {
  g_return_val_if_fail(G_IS_SIMPLE_ACTION_GROUP (simple), NULL);
  return g_hash_table_lookup(simple->priv->table, action_name);
}
void g_simple_action_group_insert(GSimpleActionGroup *simple, GAction *action) {
  const gchar *action_name;
  GAction *old_action;
  g_return_if_fail(G_IS_SIMPLE_ACTION_GROUP(simple));
  g_return_if_fail(G_IS_ACTION(action));
  action_name = g_action_get_name(action);
  old_action = g_hash_table_lookup(simple->priv->table, action_name);
  if (old_action != action) {
      if (old_action != NULL) {
          g_action_group_action_removed(G_ACTION_GROUP(simple), action_name);
          g_simple_action_group_disconnect(NULL, old_action, simple);
      }
      g_signal_connect(action, "notify::enabled",G_CALLBACK(action_enabled_notify), simple);
      if (g_action_get_state_type(action) != NULL) g_signal_connect(action, "notify::state",G_CALLBACK(action_state_notify), simple);
      g_hash_table_insert(simple->priv->table, g_strdup(action_name), g_object_ref(action));
      g_action_group_action_added(G_ACTION_GROUP(simple), action_name);
    }
}
void g_simple_action_group_remove(GSimpleActionGroup *simple, const gchar *action_name) {
  GAction *action;
  g_return_if_fail(G_IS_SIMPLE_ACTION_GROUP(simple));
  action = g_hash_table_lookup(simple->priv->table, action_name);
  if (action != NULL) {
      g_action_group_action_removed(G_ACTION_GROUP(simple), action_name);
      g_simple_action_group_disconnect(NULL, action, simple);
      g_hash_table_remove(simple->priv->table, action_name);
  }
}