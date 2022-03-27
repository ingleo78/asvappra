#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "config.h"
#include "gioenumtypes.h"
#include "glocalfilemonitor.h"
#include "giomodule-priv.h"
#include "gioerror.h"

enum {
  PROP_0,
  PROP_FILENAME,
  PROP_FLAGS
};
G_DEFINE_ABSTRACT_TYPE(GLocalFileMonitor, g_local_file_monitor, G_TYPE_FILE_MONITOR);
static void g_local_file_monitor_init(GLocalFileMonitor* local_monitor) {}
static void g_local_file_monitor_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
  switch(property_id) {
      case PROP_FILENAME: case PROP_FLAGS: break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}
static GObject *g_local_file_monitor_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_properties) {
  GObject *obj;
  GLocalFileMonitorClass *klass;
  GObjectClass *parent_class;
  GLocalFileMonitor *local_monitor;
  const gchar *filename = NULL;
  GFileMonitorFlags flags = 0;
  gint i;
  klass = G_LOCAL_FILE_MONITOR_CLASS(g_type_class_peek(G_TYPE_LOCAL_FILE_MONITOR));
  parent_class = G_OBJECT_CLASS(g_type_class_peek_parent(klass));
  obj = parent_class->constructor(type, n_construct_properties, construct_properties);
  local_monitor = G_LOCAL_FILE_MONITOR(obj);
  for (i = 0; i < n_construct_properties; i++) {
      if (strcmp("filename", g_param_spec_get_name(construct_properties[i].pspec)) == 0) {
          g_warn_if_fail(G_VALUE_HOLDS_STRING(construct_properties[i].value));
          filename = g_value_get_string(construct_properties[i].value);
      } else if (strcmp("flags", g_param_spec_get_name(construct_properties[i].pspec)) == 0) {
          g_warn_if_fail(G_VALUE_HOLDS_FLAGS(construct_properties[i].value));
          flags = g_value_get_flags(construct_properties[i].value);
      }
  }
  g_warn_if_fail(filename != NULL);
  local_monitor->filename = g_strdup(filename);
  local_monitor->flags = flags;
  return obj;
}
static void g_local_file_monitor_finalize(GObject *object) {
  GLocalFileMonitor *local_monitor = G_LOCAL_FILE_MONITOR(object);
  if (local_monitor->filename) {
      g_free(local_monitor->filename);
      local_monitor->filename = NULL;
  }
  G_OBJECT_CLASS(g_local_file_monitor_parent_class)->finalize(object);
}
static void g_local_file_monitor_class_init (GLocalFileMonitorClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->set_property = g_local_file_monitor_set_property;
  gobject_class->finalize = g_local_file_monitor_finalize;
  gobject_class->constructor = g_local_file_monitor_constructor;
  g_object_class_install_property(gobject_class,PROP_FILENAME,g_param_spec_string("filename", P_("File name"),
                                  P_("File name to monitor"),NULL,G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE | G_PARAM_STATIC_NAME |
                                  G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB));
  g_object_class_install_property(gobject_class, PROP_FLAGS, g_param_spec_flags("flags", P_("Monitor flags"), P_("Monitor flags"),
						          G_TYPE_FILE_MONITOR_FLAGS, 0, G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
						          G_PARAM_STATIC_BLURB));
}
static gpointer get_default_local_file_monitor(gpointer data) {
  GLocalFileMonitorClass *chosen_class;
  GLocalFileMonitorClass **ret = data;
  GIOExtensionPoint *ep;
  GList *extensions, *l;
  _g_io_modules_ensure_loaded();
  ep = g_io_extension_point_lookup(G_LOCAL_FILE_MONITOR_EXTENSION_POINT_NAME);
  extensions = g_io_extension_point_get_extensions(ep);
  chosen_class = NULL;
  for (l = extensions; l != NULL; l = l->next) {
      GIOExtension *extension = l->data;
      GLocalFileMonitorClass *klass;
      klass = G_LOCAL_FILE_MONITOR_CLASS(g_io_extension_ref_class(extension));
      if (klass->is_supported()) {
          chosen_class = klass;
          break;
	  } else g_type_class_unref(klass);
  }
  if (chosen_class) {
      *ret = chosen_class;
      return (gpointer)G_TYPE_FROM_CLASS(chosen_class);
  } else return (gpointer)G_TYPE_INVALID;
}
GFileMonitor* _g_local_file_monitor_new(const char *pathname, GFileMonitorFlags flags, GError **error) {
  static GOnce once_init = G_ONCE_INIT;
  GTypeClass *type_class;
  GFileMonitor *monitor;
  GType type;
  type_class = NULL;
  g_once(&once_init, get_default_local_file_monitor, &type_class);
  type = (GType)once_init.retval;
  monitor = NULL;
  if (type != G_TYPE_INVALID) monitor = G_FILE_MONITOR(g_object_new(type, "filename", pathname, "flags", flags, NULL));
  else g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Unable to find default local file monitor type"));
  if (type_class) g_type_class_unref(type_class);
  return monitor;
}