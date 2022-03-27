#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/glib-object.h"
#include "config.h"
#include "gemblemedicon.h"
#include "gioerror.h"

enum {
  PROP_GICON = 1,
  NUM_PROPERTIES
};
struct _GEmblemedIconPrivate {
  GIcon *icon;
  GList *emblems;
};
static GParamSpec *properties[NUM_PROPERTIES] = { NULL, };
static void g_emblemed_icon_icon_iface_init(GIconIface *iface);
G_DEFINE_TYPE_WITH_CODE(GEmblemedIcon, g_emblemed_icon, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_ICON, g_emblemed_icon_icon_iface_init));
static void g_emblemed_icon_finalize(GObject *object) {
  GEmblemedIcon *emblemed;
  emblemed = G_EMBLEMED_ICON(object);
  g_object_unref(emblemed->priv->icon);
  g_list_foreach(emblemed->priv->emblems, (GFunc)g_object_unref, NULL);
  g_list_free(emblemed->priv->emblems);
  (*G_OBJECT_CLASS(g_emblemed_icon_parent_class)->finalize)(object);
}
static void g_emblemed_icon_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
  GEmblemedIcon *self = G_EMBLEMED_ICON(object);
  switch(property_id) {
      case PROP_GICON: self->priv->icon = g_value_dup_object(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec); break;
  }
}
static void g_emblemed_icon_get_property(GObject  *object, guint property_id, GValue *value, GParamSpec *pspec) {
  GEmblemedIcon *self = G_EMBLEMED_ICON(object);
  switch(property_id) {
      case PROP_GICON: g_value_set_object(value, self->priv->icon); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec); break;
  }
}
static void g_emblemed_icon_class_init(GEmblemedIconClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_emblemed_icon_finalize;
  gobject_class->set_property = g_emblemed_icon_set_property;
  gobject_class->get_property = g_emblemed_icon_get_property;
  properties[PROP_GICON] = g_param_spec_object("gicon","The base GIcon","The GIcon to attach emblems to", G_TYPE_ICON, G_PARAM_READWRITE |
                                               G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, properties);
  g_type_class_add_private(klass, sizeof(GEmblemedIconPrivate));
}
static void g_emblemed_icon_init(GEmblemedIcon *emblemed) {
  emblemed->priv = G_TYPE_INSTANCE_GET_PRIVATE(emblemed, G_TYPE_EMBLEMED_ICON, GEmblemedIconPrivate);
}
GIcon *g_emblemed_icon_new(GIcon *icon, GEmblem *emblem) {
  GEmblemedIcon *emblemed;
  g_return_val_if_fail(G_IS_ICON(icon), NULL);
  g_return_val_if_fail(!G_IS_EMBLEM(icon), NULL);
  emblemed = G_EMBLEMED_ICON(g_object_new(G_TYPE_EMBLEMED_ICON, "gicon", icon, NULL));
  if (emblem != NULL) g_emblemed_icon_add_emblem(emblemed, emblem);
  return G_ICON(emblemed);
}
GIcon *g_emblemed_icon_get_icon(GEmblemedIcon *emblemed) {
  g_return_val_if_fail(G_IS_EMBLEMED_ICON(emblemed), NULL);
  return emblemed->priv->icon;
}
GList * g_emblemed_icon_get_emblems(GEmblemedIcon *emblemed) {
  g_return_val_if_fail(G_IS_EMBLEMED_ICON(emblemed), NULL);
  return emblemed->priv->emblems;
}
void g_emblemed_icon_clear_emblems(GEmblemedIcon *emblemed) {
  g_return_if_fail(G_IS_EMBLEMED_ICON(emblemed));
  if (emblemed->priv->emblems == NULL) return;
  g_list_free_full(emblemed->priv->emblems, g_object_unref);
  emblemed->priv->emblems = NULL;
}
static gint g_emblem_comp(GEmblem *a, GEmblem *b) {
  guint hash_a = g_icon_hash(G_ICON(a));
  guint hash_b = g_icon_hash(G_ICON(b));
  if(hash_a < hash_b) return -1;
  if(hash_a == hash_b) return 0;
  return 1;
}
void g_emblemed_icon_add_emblem(GEmblemedIcon *emblemed, GEmblem *emblem) {
  g_return_if_fail(G_IS_EMBLEMED_ICON(emblemed));
  g_return_if_fail(G_IS_EMBLEM(emblem));
  g_object_ref(emblem);
  emblemed->priv->emblems = g_list_insert_sorted(emblemed->priv->emblems, emblem, (GCompareFunc)g_emblem_comp);
}
static guint g_emblemed_icon_hash(GIcon *icon) {
  GEmblemedIcon *emblemed = G_EMBLEMED_ICON(icon);
  GList *list;
  guint hash = g_icon_hash (emblemed->priv->icon);
  for (list = emblemed->priv->emblems; list != NULL; list = list->next) hash ^= g_icon_hash(G_ICON(list->data));
  return hash;
}
static gboolean g_emblemed_icon_equal(GIcon *icon1, GIcon *icon2) {
  GEmblemedIcon *emblemed1 = G_EMBLEMED_ICON(icon1);
  GEmblemedIcon *emblemed2 = G_EMBLEMED_ICON(icon2);
  GList *list1, *list2;
  if (!g_icon_equal(emblemed1->priv->icon, emblemed2->priv->icon)) return FALSE;
  list1 = emblemed1->priv->emblems;
  list2 = emblemed2->priv->emblems;
  while(list1 && list2) {
      if (!g_icon_equal(G_ICON(list1->data), G_ICON(list2->data))) return FALSE;
      list1 = list1->next;
      list2 = list2->next;
  }
  return list1 == NULL && list2 == NULL;
}
static gboolean g_emblemed_icon_to_tokens(GIcon *icon, GPtrArray *tokens, gint *out_version) {
  GEmblemedIcon *emblemed_icon = G_EMBLEMED_ICON(icon);
  GList *l;
  char *s;
  g_return_val_if_fail(out_version != NULL, FALSE);
  *out_version = 0;
  s = g_icon_to_string(emblemed_icon->priv->icon);
  if (s == NULL) return FALSE;
  g_ptr_array_add(tokens, s);
  for (l = emblemed_icon->priv->emblems; l != NULL; l = l->next) {
      GIcon *emblem_icon = G_ICON(l->data);
      s = g_icon_to_string(emblem_icon);
      if (s == NULL) return FALSE;
      g_ptr_array_add(tokens, s);
  }
  return TRUE;
}
static GIcon *g_emblemed_icon_from_tokens(gchar **tokens, gint num_tokens, gint version, GError **error) {
  GEmblemedIcon *emblemed_icon;
  int n;
  emblemed_icon = NULL;
  if (version != 0) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Can't handle version %d of GEmblemedIcon encoding", version);
      goto fail;
  }
  if (num_tokens < 1) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Malformed number of tokens (%d) in GEmblemedIcon encoding", num_tokens);
      goto fail;
  }
  emblemed_icon = g_object_new(G_TYPE_EMBLEMED_ICON, NULL);
  emblemed_icon->priv->icon = g_icon_new_for_string(tokens[0], error);
  if (emblemed_icon->priv->icon == NULL) goto fail;
  for (n = 1; n < num_tokens; n++) {
      GIcon *emblem;
      emblem = g_icon_new_for_string(tokens[n], error);
      if (emblem == NULL) goto fail;
      if (!G_IS_EMBLEM(emblem)) {
          g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Expected a GEmblem for GEmblemedIcon");
          g_object_unref(emblem);
          goto fail;
      }
      emblemed_icon->priv->emblems = g_list_append(emblemed_icon->priv->emblems, emblem);
  }
  return G_ICON(emblemed_icon);
fail:
  if (emblemed_icon != NULL) g_object_unref(emblemed_icon);
  return NULL;
}
static void g_emblemed_icon_icon_iface_init(GIconIface *iface) {
  iface->hash = g_emblemed_icon_hash;
  iface->equal = g_emblemed_icon_equal;
  iface->to_tokens = g_emblemed_icon_to_tokens;
  iface->from_tokens = g_emblemed_icon_from_tokens;
}