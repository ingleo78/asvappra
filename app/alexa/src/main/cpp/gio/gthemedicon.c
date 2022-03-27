#include <string.h>
#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "../gobject/gboxed.h"
#include "../glib/glib-object.h"
#include "config.h"
#include "gthemedicon.h"
#include "gicon.h"
#include "gioerror.h"

static void g_themed_icon_icon_iface_init (GIconIface *iface);
struct _GThemedIcon {
  GObject parent_instance;
  char **names;
  gboolean use_default_fallbacks;
};
struct _GThemedIconClass {
  GObjectClass parent_class;
};
enum {
  PROP_0,
  PROP_NAME,
  PROP_NAMES,
  PROP_USE_DEFAULT_FALLBACKS
};
G_DEFINE_TYPE_WITH_CODE(GThemedIcon, g_themed_icon, G_TYPE_OBJECT, G_IMPLEMENT_INTERFACE(G_TYPE_ICON, g_themed_icon_icon_iface_init));
static void g_themed_icon_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GThemedIcon *icon = G_THEMED_ICON(object);
  switch(prop_id) {
      case PROP_NAMES: g_value_set_boxed(value, icon->names); break;
      case PROP_USE_DEFAULT_FALLBACKS: g_value_set_boolean(value, icon->use_default_fallbacks); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_themed_icon_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GThemedIcon *icon = G_THEMED_ICON (object);
  gchar **names;
  const gchar *name;
  switch(prop_id) {
      case PROP_NAME:
          name = g_value_get_string(value);
          if (!name) break;
          if (icon->names) g_strfreev(icon->names);
          icon->names = g_new(char *, 2);
          icon->names[0] = g_strdup(name);
          icon->names[1] = NULL;
          break;
      case PROP_NAMES:
          names = g_value_dup_boxed(value);
          if (!names) break;
          if (icon->names) g_strfreev(icon->names);
          icon->names = names;
          break;
      case PROP_USE_DEFAULT_FALLBACKS: icon->use_default_fallbacks = g_value_get_boolean(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_themed_icon_constructed(GObject *object) {
  GThemedIcon *themed = G_THEMED_ICON(object);
  g_return_if_fail(themed->names != NULL && themed->names[0] != NULL);
  if (themed->use_default_fallbacks) {
      int i = 0, dashes = 0;
      const char *p;
      char *dashp;
      char *last;
      p = themed->names[0];
      while(*p) {
          if (*p == '-') dashes++;
          p++;
      }
      last = g_strdup(themed->names[0]);
      g_strfreev(themed->names);
      themed->names = g_new(char *, dashes + 1 + 1);
      themed->names[i++] = last;
      while((dashp = strrchr(last, '-')) != NULL) themed->names[i++] = last = g_strndup(last, dashp - last);
      themed->names[i++] = NULL;
  }
}
static void g_themed_icon_finalize(GObject *object) {
  GThemedIcon *themed;
  themed = G_THEMED_ICON(object);
  g_strfreev(themed->names);
  G_OBJECT_CLASS(g_themed_icon_parent_class)->finalize(object);
}
static void g_themed_icon_class_init(GThemedIconClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_themed_icon_finalize;
  gobject_class->constructed = g_themed_icon_constructed;
  gobject_class->set_property = g_themed_icon_set_property;
  gobject_class->get_property = g_themed_icon_get_property;
  g_object_class_install_property(gobject_class,PROP_NAME,g_param_spec_string("name", P_("name"), P_("The name of the icon"),
                                  NULL,G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class, PROP_NAMES, g_param_spec_boxed("names", P_("names"), P_("An array containing the icon names"),
                                  G_TYPE_STRV, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));
  g_object_class_install_property(gobject_class,PROP_USE_DEFAULT_FALLBACKS,g_param_spec_boolean("use-default-fallbacks", P_("use default"
                                  " fallbacks"), P_("Whether to use default fallbacks found by shortening the name at '-' characters. Ignores names after "
                                  "the first if multiple names are given."), FALSE,G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
                                  G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));
}
static void g_themed_icon_init(GThemedIcon *themed) {
  themed->names = NULL;
}
GIcon *g_themed_icon_new(const char *iconname) {
  g_return_val_if_fail(iconname != NULL, NULL);
  return G_ICON(g_object_new(G_TYPE_THEMED_ICON, "name", iconname, NULL));
}
GIcon *g_themed_icon_new_from_names(char **iconnames, int len) {
  GIcon *icon;
  g_return_val_if_fail(iconnames != NULL, NULL);
  if (len >= 0) {
      char **names;
      int i;
      names = g_new(char*, len + 1);
      for (i = 0; i < len; i++) names[i] = iconnames[i];
      names[i] = NULL;
      icon = G_ICON(g_object_new(G_TYPE_THEMED_ICON, "names", names, NULL));
      g_free(names);
  } else icon = G_ICON(g_object_new(G_TYPE_THEMED_ICON, "names", iconnames, NULL));
  return icon;
}
GIcon *g_themed_icon_new_with_default_fallbacks(const char *iconname) {
  g_return_val_if_fail(iconname != NULL, NULL);
  return G_ICON(g_object_new(G_TYPE_THEMED_ICON, "name", iconname, "use-default-fallbacks", TRUE, NULL));
}
const char * const *g_themed_icon_get_names(GThemedIcon *icon) {
  g_return_val_if_fail(G_IS_THEMED_ICON(icon), NULL);
  return (const char * const *)icon->names;
}
void g_themed_icon_append_name(GThemedIcon *icon, const char *iconname) {
  guint num_names;
  g_return_if_fail(G_IS_THEMED_ICON(icon));
  g_return_if_fail(iconname != NULL);
  num_names = g_strv_length(icon->names);
  icon->names = g_realloc(icon->names, sizeof(char*) * (num_names + 2));
  icon->names[num_names] = g_strdup(iconname);
  icon->names[num_names + 1] = NULL;
  g_object_notify(G_OBJECT(icon), "names");
}
void g_themed_icon_prepend_name(GThemedIcon *icon, const char *iconname) {
  guint num_names;
  gchar **names;
  gint i;
  g_return_if_fail(G_IS_THEMED_ICON(icon));
  g_return_if_fail(iconname != NULL);
  num_names = g_strv_length(icon->names);
  names = g_new(char*, num_names + 2);
  for (i = 0; icon->names[i]; i++) names[i + 1] = icon->names[i];
  names[0] = g_strdup(iconname);
  names[num_names + 1] = NULL;
  g_free(icon->names);
  icon->names = names;
  g_object_notify(G_OBJECT(icon), "names");
}
static guint g_themed_icon_hash(GIcon *icon) {
  GThemedIcon *themed = G_THEMED_ICON(icon);
  guint hash;
  int i;
  hash = 0;
  for (i = 0; themed->names[i] != NULL; i++) hash ^= g_str_hash(themed->names[i]);
  return hash;
}
static gboolean g_themed_icon_equal(GIcon *icon1, GIcon *icon2) {
  GThemedIcon *themed1 = G_THEMED_ICON(icon1);
  GThemedIcon *themed2 = G_THEMED_ICON(icon2);
  int i;
  for (i = 0; themed1->names[i] != NULL && themed2->names[i] != NULL; i++) {
      if (!g_str_equal(themed1->names[i], themed2->names[i])) return FALSE;
  }
  return themed1->names[i] == NULL && themed2->names[i] == NULL;
}
static gboolean g_themed_icon_to_tokens(GIcon *icon, GPtrArray *tokens, gint *out_version) {
  GThemedIcon *themed_icon = G_THEMED_ICON(icon);
  int n;
  g_return_val_if_fail(out_version != NULL, FALSE);
  *out_version = 0;
  for (n = 0; themed_icon->names[n] != NULL; n++) g_ptr_array_add(tokens, g_strdup(themed_icon->names[n]));
  return TRUE;
}
static GIcon *g_themed_icon_from_tokens(gchar **tokens, gint num_tokens, gint version, GError **error) {
  GIcon *icon;
  gchar **names;
  int n;
  icon = NULL;
  if (version != 0) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT, _("Can't handle version %d of GThemedIcon encoding"), version);
      goto out;
  }
  names = g_new0(gchar *, num_tokens + 1);
  for (n = 0; n < num_tokens; n++) names[n] = tokens[n];
  names[n] = NULL;
  icon = g_themed_icon_new_from_names(names, num_tokens);
  g_free(names);
out:
  return icon;
}
static void g_themed_icon_icon_iface_init(GIconIface *iface) {
  iface->hash = g_themed_icon_hash;
  iface->equal = g_themed_icon_equal;
  iface->to_tokens = g_themed_icon_to_tokens;
  iface->from_tokens = g_themed_icon_from_tokens;
}